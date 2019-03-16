/**
 * @file sercom-i2c.c
 * @desc SERCOM I2C master mode driver with DMA support.
 * @author Samuel Dewan
 * @date 2019-01-25
 * Last Author:
 * Last Edited On:
 */

#include "sercom-i2c.h"

#include "sercom-tools.h"
#include "dma.h"

#include "transaction-queue.h"

// The minimum length for a transaction to use DMA
#define I2C_DMA_THREASHOLD  0
// The maximum length for an I2C DMA transaction
#define I2C_DMA_MAX  255

// Target frequencies and high to low ratios for various modes
// ratio = fraction of time spend with SCL high
#define I2C_FREQ_STANDARD       100000UL
#define I2C_RATIO_STANDARD      0.5
#define I2C_RISE_STANDARD       0.0000003 // (300 ns worst case)
#define I2C_FREQ_FAST           400000UL
#define I2C_RATIO_FAST          0.33
#define I2C_RISE_FAST           0.0000003 // (300 ns worst case)
#define I2C_FREQ_FAST_PLUS      1000000UL
#define I2C_RATIO_FAST_PLUS     0.33
#define I2C_RISE_FAST_PLUS      0.0000001 // (100 ns worst case)
#define I2C_FREQ_HIGH_SPEED     3400000UL
#define I2C_RATIO_HIGH_SPEED    0.33
#define I2C_RISE_HIGH_SPEED     0.00000004 // (40 ns worst case)


static void sercom_i2c_isr (Sercom *sercom, uint8_t inst_num, void *state);
static void sercom_i2c_dma_callback (uint8_t chan, void *state);

// Macros to calculate baud register values at compile time
#define I2C_BAUD_FOR_FREQ(f_scl, f_gclk, t_rise) ((uint8_t)((f_gclk / f_scl) - (10 + (f_gclk * t_rise))))
#define I2C_BAUD_HIGH(f_scl, f_gclk, t_rise, ratio) ((uint8_t)(I2C_BAUD_FOR_FREQ(f_scl, f_gclk, t_rise) * ratio))
#define I2C_BAUD_LOW(f_scl, f_gclk, t_rise, ratio) ((uint8_t)(I2C_BAUD_FOR_FREQ(f_scl, f_gclk, t_rise) - (I2C_BAUD_FOR_FREQ(f_scl, f_gclk, t_rise) * ratio)))

#define I2C_BAUD_FOR_FREQ_HS(f_scl, f_gclk) (uint8_t)((f_gclk / f_scl) - 2)
#define I2C_BAUD_HIGH_HS(f_scl, f_gclk, ratio) ((uint8_t)(I2C_BAUD_FOR_FREQ_HS(f_scl, f_gclk) * ratio))
#define I2C_BAUD_LOW_HS(f_scl, f_gclk, ratio) ((uint8_t)(I2C_BAUD_FOR_FREQ_HS(f_scl, f_gclk) - (I2C_BAUD_FOR_FREQ_HS(f_scl, f_gclk) * ratio)))

void init_sercom_i2c(struct sercom_i2c_desc_t *descriptor, Sercom *sercom,
                     uint32_t core_freq, uint32_t core_clock_mask,
                     enum i2c_mode mode, int8_t dma_channel)
{
    uint8_t instance_num = sercom_get_inst_num(sercom);
    
    /* Enable the APBC clock for the SERCOM instance */
    PM->APBCMASK.reg |= sercom_get_pm_apb_mask(instance_num);
    
    /* Select the core clock for the SERCOM instance */
    GCLK->CLKCTRL.reg = (GCLK_CLKCTRL_CLKEN |
                         core_clock_mask |
                         sercom_get_clk_id_mask(instance_num));
    // Wait for synchronization
    while (GCLK->STATUS.bit.SYNCBUSY);
    
    /* Reset SERCOM instance */
    sercom->I2CM.CTRLA.bit.SWRST = 0b1;
    // Wait for reset to complete
    while (sercom->I2CM.SYNCBUSY.bit.SWRST);
    
    /* Write CTRLA */
    uint8_t speed = 0x0;
    if (mode == I2C_MODE_FAST_PLUS) {
        speed = 0x1;
    } else if (mode == I2C_MODE_HIGH_SPEED) {
        speed = 0x2;
    }
    
    sercom->I2CM.CTRLA.reg = (SERCOM_I2CM_CTRLA_INACTOUT(0x3) |
                              SERCOM_I2CM_CTRLA_SDAHOLD(0x2) |
                              SERCOM_I2CM_CTRLA_SPEED(speed) |
                              SERCOM_I2CM_CTRLA_MODE_I2C_MASTER);
    
    /* Enable Smart Operation */
    sercom->I2CM.CTRLB.reg = (SERCOM_I2CM_CTRLB_SMEN);
    while (sercom->I2CM.SYNCBUSY.bit.SYSOP);
    
    /* Set Baud Rate */
    switch (mode) {
        case I2C_MODE_FAST:
            // Fast mode: 400 kHz
            // BAUD should equal 36
            // LBAUD should equal 74
            sercom->I2CM.BAUD.bit.BAUD = I2C_BAUD_HIGH(I2C_FREQ_FAST, core_freq,
                                                       I2C_RISE_FAST,
                                                       I2C_RATIO_FAST);
            sercom->I2CM.BAUD.bit.BAUDLOW = I2C_BAUD_LOW(I2C_FREQ_FAST,
                                                         core_freq,
                                                         I2C_RISE_FAST,
                                                         I2C_RATIO_FAST);
            break;
        case I2C_MODE_FAST_PLUS:
            // Fast mode plus: 1 MHz
            // BAUD should equal 12
            // LBAUD should equal 26
            sercom->I2CM.BAUD.bit.BAUD = I2C_BAUD_HIGH(I2C_FREQ_FAST_PLUS,
                                                       core_freq,
                                                       I2C_RISE_FAST_PLUS,
                                                       I2C_RATIO_FAST_PLUS);
            sercom->I2CM.BAUD.bit.BAUDLOW = I2C_BAUD_LOW(I2C_FREQ_FAST_PLUS,
                                                         core_freq,
                                                         I2C_RISE_FAST_PLUS,
                                                         I2C_RATIO_FAST_PLUS);
            break;
        case I2C_MODE_HIGH_SPEED:
            // High speed mode: 3.4 MHz
            // BAUD should equal 4
            // LBAUD should equal 8
            sercom->I2CM.BAUD.bit.BAUD = I2C_BAUD_HIGH_HS(I2C_FREQ_FAST_PLUS,
                                                          core_freq,
                                                          I2C_RATIO_FAST_PLUS);
            sercom->I2CM.BAUD.bit.BAUDLOW = I2C_BAUD_LOW_HS(I2C_FREQ_FAST_PLUS,
                                                            core_freq,
                                                            I2C_RATIO_FAST_PLUS);
            break;
        default:
            // Standard mode: 100 kHz
            // Ideally, Tlow = Thigh = 5 micoseconds
            // BAUD should equal 235
            sercom->I2CM.BAUD.bit.BAUD = I2C_BAUD_HIGH(I2C_FREQ_STANDARD,
                                                       core_freq,
                                                       I2C_RISE_STANDARD,
                                                       I2C_RATIO_STANDARD);
            sercom->I2CM.BAUD.bit.BAUDLOW = 0;
            break;
    }
    
    /* Configure interupts */
    sercom_handlers[instance_num] = (struct sercom_handler_t) {
        .handler = sercom_i2c_isr,
        .state = (void*)descriptor
    };
    descriptor->sercom_instnum = instance_num;
    NVIC_EnableIRQ(sercom_get_irq_num(instance_num));
    
    /* Setup Descriptor */
    descriptor->sercom = sercom;
    
    init_transaction_queue(&descriptor->queue, descriptor->transactions,
                           SERCOM_I2C_TRANSACTION_QUEUE_LENGTH,
                           descriptor->states,
                           sizeof(struct sercom_i2c_transaction_t));
    
    /* Configure DMA */
    if ((dma_channel >= 0) && (dma_channel < DMAC_CH_NUM)) {
        descriptor->dma_chan = (uint8_t)dma_channel;
        descriptor->use_dma = 0b1;
        
        dma_callbacks[dma_channel] = (struct dma_callback_t) {
            .callback = sercom_i2c_dma_callback,
            .state = (void*)descriptor
        };
    }
    
    /* Enable SERCOM Instance */
    sercom->I2CM.CTRLA.bit.ENABLE = 0b1;
    while (sercom->I2CM.SYNCBUSY.bit.ENABLE);
    
    /* Force Bus State to IDLE */
    // If we don't force it, it will wait until it sees a stop condition to be
    // sure that the bus is idle. Since we are the only master on the bus, we
    // will wait forever for a stop condition to occur.
    sercom->I2CM.STATUS.bit.BUSSTATE = 0x1;
    while (sercom->I2CM.SYNCBUSY.bit.SYSOP);
    // Make sure that wait for idle bit is cleared
    descriptor->wait_for_idle = 0;
}

uint8_t sercom_i2c_start_generic(struct sercom_i2c_desc_t *i2c_inst,
                                 uint8_t *trans_id, uint8_t dev_address,
                                 uint8_t *out_buffer, uint16_t out_length,
                                 uint8_t *in_buffer, uint16_t in_length)
{
    struct transaction_t *t = transaction_queue_add(&i2c_inst->queue);
    if (t == NULL) {
        return 1;
    }
    *trans_id = t->transaction_id;
    
    struct sercom_i2c_transaction_t *state =
                                (struct sercom_i2c_transaction_t*)t->state;
    
    state->generic.out_buffer = out_buffer;
    state->generic.out_length = out_length;
    state->generic.bytes_out = 0;
    state->dma_out = (i2c_inst->use_dma && (out_length >= I2C_DMA_THREASHOLD) &&
                      (out_length <= I2C_DMA_MAX));
    
    state->generic.in_buffer = in_buffer;
    state->generic.in_length = in_length;
    state->generic.bytes_in = 0;
    state->dma_in = (i2c_inst->use_dma && (in_length >= I2C_DMA_THREASHOLD) &&
                      (in_length <= I2C_DMA_MAX));
    
    state->dev_address = dev_address << 1;
    state->type = I2C_TRANSACTION_GENERIC;
    state->state = I2C_STATE_PENDING;
    
    transaction_queue_set_valid(t);
    
    sercom_i2c_service(i2c_inst);
    return 0;
}

uint8_t sercom_i2c_start_reg_write(struct sercom_i2c_desc_t *i2c_inst,
                                   uint8_t *trans_id, uint8_t dev_address,
                                   uint8_t register_address, uint8_t *data,
                                   uint16_t length)
{
    struct transaction_t *t = transaction_queue_add(&i2c_inst->queue);
    if (t == NULL) {
        return 1;
    }
    *trans_id = t->transaction_id;
    
    struct sercom_i2c_transaction_t *state =
                                (struct sercom_i2c_transaction_t*)t->state;
    
    state->reg.buffer = data;
    state->reg.data_length = length;
    state->reg.register_address = register_address;
    state->reg.position = 0;
    state->dma_out = (i2c_inst->use_dma && (length >= I2C_DMA_THREASHOLD) &&
                      (length < I2C_DMA_MAX));
    state->dma_in = 0;
    
    state->dev_address = dev_address << 1;
    state->type = I2C_TRANSACTION_REG_WRITE;
    state->state = I2C_STATE_PENDING;
    
    transaction_queue_set_valid(t);
    
    sercom_i2c_service(i2c_inst);
    return 0;
}

uint8_t sercom_i2c_start_reg_read(struct sercom_i2c_desc_t *i2c_inst,
                                  uint8_t *trans_id, uint8_t dev_address,
                                  uint8_t register_address, uint8_t *data,
                                  uint16_t length)
{
    struct transaction_t *t = transaction_queue_add(&i2c_inst->queue);
    if (t == NULL) {
        return 1;
    }
    *trans_id = t->transaction_id;
    
    struct sercom_i2c_transaction_t *state =
                                (struct sercom_i2c_transaction_t*)t->state;
    
    state->reg.buffer = data;
    state->reg.data_length = length;
    state->reg.register_address = register_address;
    state->reg.position = 0;
    state->dma_out = 0;
    state->dma_in = (i2c_inst->use_dma && (length >= I2C_DMA_THREASHOLD) &&
                     (length <= I2C_DMA_MAX));
    
    state->dev_address = dev_address << 1;
    state->type = I2C_TRANSACTION_REG_READ;
    state->state = I2C_STATE_PENDING;
    
    transaction_queue_set_valid(t);
    
    sercom_i2c_service(i2c_inst);
    return 0;
}

uint8_t sercom_i2c_start_scan(struct sercom_i2c_desc_t *i2c_inst,
                              uint8_t *trans_id)
{
    struct transaction_t *t = transaction_queue_add(&i2c_inst->queue);
    if (t == NULL) {
        return 1;
    }
    *trans_id = t->transaction_id;
    
    struct sercom_i2c_transaction_t *state =
                                (struct sercom_i2c_transaction_t*)t->state;
    
    state->scan.results[0] = 0;
    state->scan.results[1] = 0;
    
    state->dev_address = 2; // Skip address 0 (general call address)
    state->type = I2C_TRANSACTION_SCAN;
    state->state = I2C_STATE_PENDING;
    state->dma_out = 0;
    state->dma_in = 0;
    
    transaction_queue_set_valid(t);
    
    sercom_i2c_service(i2c_inst);
    return 0;
}

uint8_t sercom_i2c_transaction_done(struct sercom_i2c_desc_t *i2c_inst,
                                    uint8_t trans_id)
{
    return transaction_queue_is_done(
                            transaction_queue_get(&i2c_inst->queue, trans_id));
}

enum i2c_transaction_state sercom_i2c_transaction_state(
                                            struct sercom_i2c_desc_t *i2c_inst,
                                            uint8_t trans_id)
{
    struct transaction_t *t = transaction_queue_get(&i2c_inst->queue, trans_id);
    return ((struct sercom_i2c_transaction_t*) t->state)->state;
}

uint8_t sercom_i2c_clear_transaction(struct sercom_i2c_desc_t *i2c_inst,
                                     uint8_t trans_id)
{
    return transaction_queue_invalidate(
                            transaction_queue_get(&i2c_inst->queue, trans_id));
}

uint8_t sercom_i2c_device_avaliable(struct sercom_i2c_desc_t *i2c_inst,
                                    uint8_t trans_id, uint8_t address)
{
    struct transaction_t *t = transaction_queue_get(&i2c_inst->queue, trans_id);
    struct sercom_i2c_transaction_t *state =
                                (struct sercom_i2c_transaction_t*)t->state;
    
    return (address < 64) ? !!(state->scan.results[0] & (1 << address)) :
                            !!(state->scan.results[1] & (1 << (address - 64)));
}







static inline void sercom_i2c_begin_generic (
                                        struct sercom_i2c_desc_t *i2c_inst,
                                        struct sercom_i2c_transaction_t *state)
{
    state->state = (state->generic.out_length) ? I2C_STATE_TX : I2C_STATE_RX;
    
    if ((state->generic.out_length && state->dma_out) ||
        (!state->generic.out_length && state->dma_in)) {
        /* Start transaction with DMA */
        
        uint8_t len = (uint8_t)((state->generic.out_length) ?
                                state->generic.out_length :
                                state->generic.in_length);
        
        if (state->generic.out_length) {
            dma_start_buffer_to_static(i2c_inst->dma_chan,
                            state->generic.out_buffer, len,
                            &i2c_inst->sercom->I2CM.DATA.reg,
                            sercom_get_dma_tx_trigger(i2c_inst->sercom_instnum),
                            SERCOM_DMA_TX_PRIORITY);
        } else {
            dma_start_static_to_buffer(i2c_inst->dma_chan,
                            state->generic.in_buffer, len,
                            &i2c_inst->sercom->I2CM.DATA.reg,
                            sercom_get_dma_rx_trigger(i2c_inst->sercom_instnum),
                            SERCOM_DMA_RX_PRIORITY);
        }
        // Write ADDR to start I2C transaction
        i2c_inst->sercom->I2CM.ADDR.reg = (
                                    SERCOM_I2CM_ADDR_LEN(len) |
                                    SERCOM_I2CM_ADDR_LENEN |
                                    SERCOM_I2CM_ADDR_ADDR(state->dev_address)
                                           );
    } else {
        /* Start transaction interupt driven */
        i2c_inst->sercom->I2CM.INTENSET.reg = (SERCOM_I2CM_INTENCLR_MB |
                                               SERCOM_I2CM_INTENCLR_SB);
        i2c_inst->sercom->I2CM.ADDR.bit.ADDR = state->dev_address |
                                                !(state->generic.out_length);
    }
}

static inline void sercom_i2c_begin_register (
                                        struct sercom_i2c_desc_t *i2c_inst,
                                        struct sercom_i2c_transaction_t *state)
{
    state->state = I2C_STATE_TX;
    
    if ((state->type == I2C_TRANSACTION_REG_WRITE) && state->dma_out) {
        /* Start transaction with DMA */
        uint8_t len = (uint8_t)(state->reg.data_length);
        
        dma_start_double_buffer_to_static(i2c_inst->dma_chan,
                            &state->reg.register_address, 1,
                            state->reg.buffer, len, &i2c_inst->dma_desc,
                            &i2c_inst->sercom->I2CM.DATA.reg,
                            sercom_get_dma_tx_trigger(i2c_inst->sercom_instnum),
                            SERCOM_DMA_TX_PRIORITY);
        
        // Write ADDR to start I2C transaction
        i2c_inst->sercom->I2CM.ADDR.reg = (
                                    SERCOM_I2CM_ADDR_LEN(len) |
                                    SERCOM_I2CM_ADDR_LENEN |
                                    SERCOM_I2CM_ADDR_ADDR(state->dev_address)
                                           );
    } else {
        /* Start transaction interupt driven */
        i2c_inst->sercom->I2CM.INTENSET.reg = (SERCOM_I2CM_INTENCLR_MB |
                                               SERCOM_I2CM_INTENCLR_SB);
        i2c_inst->sercom->I2CM.ADDR.bit.ADDR = state->dev_address | 0;
    }
}

static inline void sercom_i2c_end_transaction (
                                               struct sercom_i2c_desc_t *i2c_inst,
                                               struct transaction_t *t)
{
    t->done = 1;
    t->active = 0;
    
    // Disable MB and SM interutps
    i2c_inst->sercom->I2CM.INTENCLR.reg = (SERCOM_I2CM_INTENCLR_MB |
                                           SERCOM_I2CM_INTENCLR_SB |
                                           SERCOM_I2CM_INTENCLR_ERROR);
    
    // Run the I2C service to start the next transaction if there is one
    sercom_i2c_service(i2c_inst);
}

static inline void sercom_i2c_begin_in_dma (
                                            struct sercom_i2c_desc_t *i2c_inst,
                                            struct sercom_i2c_transaction_t *state)
{
    // Transaction must be generic or reg read
    uint8_t r = (state->type == I2C_TRANSACTION_REG_READ);
    uint8_t len = (uint8_t)(r ? state->reg.data_length :
                            state->generic.in_length);
    
    // Begin reading bytes with DMA
    dma_start_static_to_buffer(i2c_inst->dma_chan,
                               (r ? state->reg.buffer :
                                state->generic.in_buffer),
                               len, &i2c_inst->sercom->I2CM.DATA.reg,
                               sercom_get_dma_rx_trigger(i2c_inst->sercom_instnum),
                               SERCOM_DMA_RX_PRIORITY);
    // Write ADDR to start I2C transaction
    i2c_inst->sercom->I2CM.ADDR.reg = (
                                       SERCOM_I2CM_ADDR_LEN(len) |
                                       SERCOM_I2CM_ADDR_LENEN |
                                       SERCOM_I2CM_ADDR_ADDR(state->dev_address)
                                       );
}

void sercom_i2c_service (struct sercom_i2c_desc_t *i2c_inst)
{
    /* Acquire service function lock */
    if (i2c_inst->service_lock) {
        // Could not accuire lock, service is already being run
        return;
    } else {
        i2c_inst->service_lock = 1;
    }
    
    if (transaction_queue_head_active(&i2c_inst->queue)) {
        // There is already a transaction in progress
        struct transaction_t *t =
                                transaction_queue_get_active(&i2c_inst->queue);
        struct sercom_i2c_transaction_t *s =
                                    (struct sercom_i2c_transaction_t*)t->state;
        
        if ((i2c_inst->sercom->I2CM.STATUS.bit.BUSSTATE == 0x1) &&
                s->state == I2C_STATE_WAIT_FOR_RX) {
            // The I2C bus has returned to idle, we can let the CPU sleep again
            allow_sleep();
            
            // Start reception
            if (s->dma_in) {
                // Begin reading bytes with DMA
                sercom_i2c_begin_in_dma(i2c_inst, s);
            } else {
                // Begin reading bytes interupt driven
                i2c_inst->sercom->I2CM.INTENSET.reg = (
                                                    SERCOM_I2CM_INTENCLR_MB |
                                                    SERCOM_I2CM_INTENCLR_SB);
                s->state = I2C_STATE_RX;
                i2c_inst->sercom->I2CM.ADDR.bit.ADDR = (s->dev_address |
                                                        1);
            }
        } else if ((i2c_inst->sercom->I2CM.STATUS.bit.BUSSTATE == 0x1) &&
                    s->state == I2C_STATE_WAIT_FOR_DONE) {
            // The I2C bus has returned to idle, we can let the CPU sleep again
            allow_sleep();
            
            // End transaction
            s->state = I2C_STATE_DONE;
            
            t->done = 1;
            t->active = 0;
            
            // Disable MB and SM interutps
            i2c_inst->sercom->I2CM.INTENCLR.reg = (SERCOM_I2CM_INTENCLR_MB |
                                                   SERCOM_I2CM_INTENCLR_SB |
                                                   SERCOM_I2CM_INTENCLR_ERROR);
        }
        
        return;
    }
    
    // If we are waiting for idle and the bus is now idle, we should stop
    // waiting for idle.
    if (i2c_inst->wait_for_idle &&
        i2c_inst->sercom->I2CM.STATUS.bit.BUSSTATE == 0x1) {
        i2c_inst->wait_for_idle = 0;
    } else if (i2c_inst->wait_for_idle) {
        // The bus still isn't idle, return now so that we don't start
        // doubly waiting for idle.
        return;
    }
    
    // No transaction in progress, check if one needs to be started
    struct transaction_t *t = transaction_queue_next(&i2c_inst->queue);
    if (t == NULL) {
        // No pending transactions
        return;
    } else if (i2c_inst->sercom->I2CM.STATUS.bit.BUSSTATE == 0x1) {
        // Start the next transaction
        struct sercom_i2c_transaction_t *s =
                            (struct sercom_i2c_transaction_t*)t->state;
        
        /* Mark transaction as active */
        t->active = 1;
        
        /* Begin transaction */
        switch (s->type) {
            case I2C_TRANSACTION_GENERIC:
                sercom_i2c_begin_generic(i2c_inst, s);
                break;
            case I2C_TRANSACTION_REG_WRITE:
            case I2C_TRANSACTION_REG_READ:
                sercom_i2c_begin_register(i2c_inst, s);
                break;
            case I2C_TRANSACTION_SCAN:
                // Start by sending first address
                i2c_inst->sercom->I2CM.INTENSET.reg =
                                                (SERCOM_I2CM_INTENCLR_MB |
                                                 SERCOM_I2CM_INTENCLR_SB);
                i2c_inst->sercom->I2CM.ADDR.bit.ADDR = s->dev_address;
                s->dev_address += 2;
                break;
            default:
                break;
        }
    } else {
        // There is a pending transaction but the bus is not idle... eek
        i2c_inst->wait_for_idle = 1;
        // Keep checking if the bus has become idle as often as possible
        inhibit_sleep();
    }
    
    i2c_inst->service_lock = 0;
}



void sercom_i2c_isr (Sercom *sercom, uint8_t inst_num, void *state)
{
    struct sercom_i2c_desc_t *i2c_inst = (struct sercom_i2c_desc_t*)state;
    struct transaction_t *t = transaction_queue_get_active(&i2c_inst->queue);
    struct sercom_i2c_transaction_t *s =
                                    (struct sercom_i2c_transaction_t*)t->state;
    
    // Master on Bus
    if (sercom->I2CM.INTFLAG.bit.MB) {
        if (i2c_inst->sercom->I2CM.STATUS.bit.BUSERR) {
            /* Bus error */
            s->state = I2C_STATE_BUS_ERROR;
            sercom_i2c_end_transaction(i2c_inst, t);
        } else if (i2c_inst->sercom->I2CM.STATUS.bit.ARBLOST) {
            /* Lost arbitration */
            s->state = I2C_STATE_ARBITRATION_LOST;
            sercom_i2c_end_transaction(i2c_inst, t);
        } else if (s->type == I2C_TRANSACTION_SCAN) {
            if (!i2c_inst->sercom->I2CM.STATUS.bit.RXNACK) {
                /* Slave did ACK address */
                s->scan.results[s->dev_address > 63] |= (1 << (
                                                        (s->dev_address < 64) ?
                                                        (s->dev_address) :
                                                        (s->dev_address - 64)));
            }
            
            s->dev_address += 2;
            if (s->dev_address != 0) {
                // Send next address
                i2c_inst->sercom->I2CM.ADDR.bit.ADDR = s->dev_address;
            } else {
                // Scan complete
                i2c_inst->sercom->I2CM.CTRLB.bit.CMD = 0x3;
                while (i2c_inst->sercom->I2CM.SYNCBUSY.bit.SYSOP);
                s->state = I2C_STATE_DONE;
                sercom_i2c_end_transaction(i2c_inst, t);
            }
        } else if (i2c_inst->sercom->I2CM.STATUS.bit.RXNACK) {
            /* Slave did not ACK address or data */
            s->state = I2C_STATE_SLAVE_NACK;
            sercom_i2c_end_transaction(i2c_inst, t);
            sercom_i2c_end_transaction(i2c_inst, t);
        } else if (s->type == I2C_TRANSACTION_GENERIC) {
            if (s->generic.bytes_out == s->generic.out_length) {
                // All bytes have been sent
                if (s->generic.in_length) {
                    // There are bytes to be received, sent repeated start
                    if (s->dma_in) {
                        // Begin reading bytes with DMA
                        sercom_i2c_begin_in_dma(i2c_inst, s);
                    } else {
                        // Begin reading bytes interupt driven
                        s->state = I2C_STATE_RX;
                        i2c_inst->sercom->I2CM.ADDR.bit.ADDR = (s->dev_address |
                                                                1);
                    }
                } else {
                    // There are no bytes to be received, send stop condition
                    i2c_inst->sercom->I2CM.CTRLB.bit.CMD = 0x3;
                    while (i2c_inst->sercom->I2CM.SYNCBUSY.bit.SYSOP);
                    s->state = I2C_STATE_DONE;
                    sercom_i2c_end_transaction(i2c_inst, t);
                }
            } else {
                // Send next byte
                i2c_inst->sercom->I2CM.DATA.reg =
                                s->generic.out_buffer[s->generic.bytes_out++];
            }
        } else if (s->type == I2C_TRANSACTION_REG_WRITE) {
            if (s->state == I2C_STATE_TX) {
                // Sending data
                if (s->reg.position == s->reg.data_length) {
                    // All bytes have been sent, send stop condition
                    i2c_inst->sercom->I2CM.CTRLB.bit.CMD = 0x3;
                    while (i2c_inst->sercom->I2CM.SYNCBUSY.bit.SYSOP);
                    s->state = I2C_STATE_DONE;
                    sercom_i2c_end_transaction(i2c_inst, t);
                } else {
                    // Send next byte
                    i2c_inst->sercom->I2CM.DATA.reg =
                                s->generic.out_buffer[s->generic.bytes_out++];
                }
            } else {
                // Send register address
                i2c_inst->sercom->I2CM.DATA.reg = s->reg.register_address;
                s->state = I2C_STATE_TX;
            }
        } else if (s->type == I2C_TRANSACTION_REG_READ) {
            if (s->state == I2C_STATE_RX) {
                // Start receiving data, send repeated start
                if (s->dma_in) {
                    // Begin reading bytes with DMA
                    sercom_i2c_begin_in_dma(i2c_inst, s);
                } else {
                    // Begin reading bytes interupt driven
                    i2c_inst->sercom->I2CM.ADDR.bit.ADDR = s->dev_address | 1;
                }
            } else {
                // Send register address
                i2c_inst->sercom->I2CM.DATA.reg = s->reg.register_address;
                s->state = I2C_STATE_RX;
            }
        }
        
        // Clear master on bus interupt and error interupt
        i2c_inst->sercom->I2CM.INTFLAG.reg |= (SERCOM_I2CM_INTFLAG_MB |
                                               SERCOM_I2CM_INTFLAG_ERROR);
    }
    
    // Slave on Bus
    if (sercom->I2CM.INTFLAG.bit.SB) {
        uint8_t generic_last = (s->type == I2C_TRANSACTION_GENERIC) &&
                            (s->generic.bytes_in == (s->generic.in_length - 1));
        uint8_t reg_last = (s->type == I2C_TRANSACTION_REG_READ) &&
                                (s->reg.position == (s->reg.data_length - 1));
        
        if (generic_last || reg_last) {
            // The last byte has been received, send NACK next
            i2c_inst->sercom->I2CM.CTRLB.bit.ACKACT = 1;
            while (i2c_inst->sercom->I2CM.SYNCBUSY.bit.SYSOP);
            // Send stop condition after byte read
            i2c_inst->sercom->I2CM.CTRLB.bit.CMD = 0x3;
            while (i2c_inst->sercom->I2CM.SYNCBUSY.bit.SYSOP);
            
            // Read last byte
            if (s->type == I2C_TRANSACTION_GENERIC) {
                s->generic.in_buffer[s->generic.bytes_in] =
                                            i2c_inst->sercom->I2CM.DATA.reg;
            } else if (s->type == I2C_TRANSACTION_REG_READ) {
                s->reg.buffer[s->reg.position] =
                                            i2c_inst->sercom->I2CM.DATA.reg;
            }
            
            // Transaction done
            s->state = I2C_STATE_DONE;
            sercom_i2c_end_transaction(i2c_inst, t);
        } else {
            // A byte has been received, send ACK
            i2c_inst->sercom->I2CM.CTRLB.bit.ACKACT = 0;
            while (i2c_inst->sercom->I2CM.SYNCBUSY.bit.SYSOP);
            
            // Read byte
            if (s->type == I2C_TRANSACTION_GENERIC) {
                s->generic.in_buffer[s->generic.bytes_in++] =
                                            i2c_inst->sercom->I2CM.DATA.reg;
            } else if (s->type == I2C_TRANSACTION_REG_READ) {
                s->reg.buffer[s->reg.position++] =
                                            i2c_inst->sercom->I2CM.DATA.reg;
            }
            
            // Receive next byte
            i2c_inst->sercom->I2CM.CTRLB.bit.CMD = 0x2;
            while (i2c_inst->sercom->I2CM.SYNCBUSY.bit.SYSOP);
        }
        
        // Clear slave on bus interupt
        i2c_inst->sercom->I2CM.INTFLAG.reg |= (SERCOM_I2CM_INTFLAG_SB);
    }
    
    // Error
    if (sercom->I2CM.INTFLAG.bit.ERROR) {
        /* An error has occured during a DMA driven transaction */
        // Abort DMA transaction
        dma_abort_transaction(i2c_inst->dma_chan);
        
        // Record error
        if (i2c_inst->sercom->I2CM.STATUS.bit.BUSERR) {
            /* Bus error */
            s->state = I2C_STATE_BUS_ERROR;
        } else if (i2c_inst->sercom->I2CM.STATUS.bit.ARBLOST) {
            /* Lost arbitration */
            s->state = I2C_STATE_ARBITRATION_LOST;
        } else if (i2c_inst->sercom->I2CM.STATUS.bit.LENERR) {
            /* Slave NACKed early */
            s->state = I2C_STATE_SLAVE_NACK;
        }
        
        // End I2C transaction
        sercom_i2c_end_transaction(i2c_inst, t);
    }
}

void sercom_i2c_dma_callback (uint8_t chan, void *state)
{
    struct sercom_i2c_desc_t *i2c_inst = (struct sercom_i2c_desc_t*)state;
    struct transaction_t *t = transaction_queue_get_active(&i2c_inst->queue);
    struct sercom_i2c_transaction_t *s =
                                    (struct sercom_i2c_transaction_t*)t->state;
    
    switch (s->type) {
        case I2C_TRANSACTION_GENERIC:
            if (s->state == I2C_STATE_TX) {
                // TX Complete
                if (s->generic.bytes_in) {
                    // Wait for bus to become idle so that we can start recieve
                    // stage
                    s->state = I2C_STATE_WAIT_FOR_RX;
                } else {
                    // Wait for bus to become idle so that we can end
                    s->state = I2C_STATE_WAIT_FOR_DONE;
                }
                // Inhibit sleep as the delay before the bus is idle may be much
                // less than the time that the CPU normaly spends sleeping
                inhibit_sleep();
                break;
            }
            // RX Complete, transaction is done
            // fall through
        case I2C_TRANSACTION_REG_READ:
            // Transaction is complete
            s->state = I2C_STATE_DONE;
            sercom_i2c_end_transaction(i2c_inst, t);
            break;
        case I2C_TRANSACTION_REG_WRITE:
            // Need to wait for the bus to become idle before ending transaction
            s->state = I2C_STATE_WAIT_FOR_DONE;
            // Inhibit sleep as the delay before the bus is idle may be much
            // less than the time that the CPU normaly spends sleeping
            inhibit_sleep();
            break;
        default:
            break;
    }
}
