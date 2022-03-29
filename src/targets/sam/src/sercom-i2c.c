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
#define I2C_DMA_THRESHOLD   3
// The maximum length for an I2C DMA transaction
#define I2C_DMA_MAX  255

// Target frequencies and high to low ratios for various modes
// ratio = fraction of time spend with SCL high
#define I2C_FREQ_STANDARD       100000UL
#define I2C_RATIO_STANDARD      0.5f
#define I2C_RISE_STANDARD       0.0000003f // (300 ns worst case)
#define I2C_FREQ_FAST           400000UL
#define I2C_RATIO_FAST          0.33f
#define I2C_RISE_FAST           0.0000003f // (300 ns worst case)
#define I2C_FREQ_FAST_PLUS      1000000UL
#define I2C_RATIO_FAST_PLUS     0.33f
#define I2C_RISE_FAST_PLUS      0.0000001f // (100 ns worst case)
#define I2C_FREQ_HIGH_SPEED     3400000UL
#define I2C_RATIO_HIGH_SPEED    0.33f
#define I2C_RISE_HIGH_SPEED     0.00000004f // (40 ns worst case)


static void sercom_i2c_isr_mb (Sercom *sercom, uint8_t inst_num, void *state);
static void sercom_i2c_isr_sb (Sercom *sercom, uint8_t inst_num, void *state);
static void sercom_i2c_isr_error (Sercom *sercom, uint8_t inst_num,
                                  void *state);
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
    uint8_t const instance_num = sercom_get_inst_num(sercom);
    
    /* Enable the APBC clock for the SERCOM instance */
    enable_bus_clock(sercom_get_bus_clk(instance_num));
    
    /* Select the core clock for the SERCOM instance */
    set_perph_generic_clock(sercom_get_gclk(instance_num), core_clock_mask);
    
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
            // Ideally, Tlow = Thigh = 5 microseconds
            // BAUD should equal 235
            sercom->I2CM.BAUD.bit.BAUD = I2C_BAUD_HIGH(I2C_FREQ_STANDARD,
                                                       core_freq,
                                                       I2C_RISE_STANDARD,
                                                       I2C_RATIO_STANDARD);
            sercom->I2CM.BAUD.bit.BAUDLOW = 0;
            break;
    }
    
    /* Configure interrupts */
    sercom_handlers[instance_num] = (struct sercom_handler_t) {
        .mb_handler = sercom_i2c_isr_mb,
        .sb_handler = sercom_i2c_isr_sb,
        .misc_handler = sercom_i2c_isr_error,
        .state = (void*)descriptor
    };
    descriptor->sercom_instnum = instance_num;

    sercom_enable_interrupts(instance_num, (SERCOM_I2CM_INTFLAG_MB |
                                            SERCOM_I2CM_INTFLAG_SB |
                                            SERCOM_I2CM_INTFLAG_ERROR));
    
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
                                 uint8_t const* out_buffer, uint16_t out_length,
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
    state->dma_out = (i2c_inst->use_dma && (out_length >= I2C_DMA_THRESHOLD) &&
                      (out_length <= I2C_DMA_MAX));
    
    state->generic.in_buffer = in_buffer;
    state->generic.in_length = in_length;
    state->generic.bytes_in = 0;
    state->dma_in = (i2c_inst->use_dma && (in_length >= I2C_DMA_THRESHOLD) &&
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
    state->dma_out = (i2c_inst->use_dma && (length >= I2C_DMA_THRESHOLD) &&
                      (length <= I2C_DMA_MAX));
    state->dma_in = 0;
    
    state->dev_address = dev_address << 1;
    state->type = I2C_TRANSACTION_REG_WRITE;
    state->state = I2C_STATE_PENDING;

    state->reg.callback = NULL;
    state->reg.callback_context = NULL;
    
    transaction_queue_set_valid(t);
    
    sercom_i2c_service(i2c_inst);
    return 0;
}

uint8_t sercom_i2c_start_reg_read(struct sercom_i2c_desc_t *i2c_inst,
                                  uint8_t *trans_id, uint8_t dev_address,
                                  uint8_t register_address, uint8_t *data,
                                  uint16_t length)
{
    return sercom_i2c_start_reg_read_with_cb(i2c_inst, trans_id, dev_address,
                                             register_address, data, length,
                                             NULL, NULL);
}

uint8_t sercom_i2c_start_reg_read_with_cb(struct sercom_i2c_desc_t *i2c_inst,
                                          uint8_t *trans_id,
                                          uint8_t dev_address,
                                          uint8_t register_address,
                                          uint8_t *data, uint16_t length,
                                          sercom_i2c_transaction_cb_t callback,
                                          void *context)
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
    state->dma_in = (i2c_inst->use_dma && (length >= I2C_DMA_THRESHOLD) &&
                     (length <= I2C_DMA_MAX));

    state->dev_address = dev_address << 1;
    state->type = I2C_TRANSACTION_REG_READ;
    state->state = I2C_STATE_PENDING;

    state->reg.callback = callback;
    state->reg.callback_context = context;

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
    
    state->dev_address = 1; // Skip address 0 (general call address)
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

uint8_t sercom_i2c_device_available(struct sercom_i2c_desc_t *i2c_inst,
                                    uint8_t trans_id, uint8_t address)
{
    struct transaction_t *t = transaction_queue_get(&i2c_inst->queue, trans_id);
    struct sercom_i2c_transaction_t *state =
                                (struct sercom_i2c_transaction_t*)t->state;
    
    return ((address < 64) ?
            !!(state->scan.results[0] & ((uint64_t)1 << address)) :
            !!(state->scan.results[1] & ((uint64_t)1 << (address - 64))));
}







static inline void sercom_i2c_begin_generic (
                                        struct sercom_i2c_desc_t *i2c_inst,
                                        struct sercom_i2c_transaction_t *state)
{
    int const in_only = state->generic.out_length == 0;
    int const dma_out_only = (state->generic.in_length == 0) && state->dma_out;
    int const dma_in_only = in_only && state->dma_in;
    uint8_t const addr = state->dev_address | in_only;

    state->state = in_only ? I2C_STATE_RX : I2C_STATE_TX;

    if (dma_out_only || dma_in_only) {
        /* Start transaction with DMA */
        uint8_t const len = (uint8_t)(in_only ? state->generic.in_length :
                                                state->generic.out_length);

        uint8_t const dma_trig = sercom_get_dma_tx_trigger(
                                                    i2c_inst->sercom_instnum);
        if (in_only) {
            dma_config_transfer(i2c_inst->dma_chan, DMA_WIDTH_BYTE,
                                &i2c_inst->sercom->I2CM.DATA.reg, 0,
                                state->generic.in_buffer, 1, len, dma_trig,
                                SERCOM_DMA_RX_PRIORITY, NULL);
            // Set up to ACK bytes as we receive them
            i2c_inst->sercom->I2CM.CTRLB.bit.ACKACT = 0;
        } else {
            dma_config_transfer(i2c_inst->dma_chan, DMA_WIDTH_BYTE,
                                state->generic.out_buffer, 1,
                                &i2c_inst->sercom->I2CM.DATA.reg, 0, len,
                                dma_trig, SERCOM_DMA_TX_PRIORITY, NULL);
        }
        // Enable error interrupt
        i2c_inst->sercom->I2CM.INTENSET.reg = SERCOM_I2CM_INTENSET_ERROR;
        // Write ADDR to start I2C transaction
        i2c_inst->sercom->I2CM.ADDR.reg = (SERCOM_I2CM_ADDR_LEN(len) |
                                           SERCOM_I2CM_ADDR_LENEN |
                                           SERCOM_I2CM_ADDR_ADDR(addr));
    } else {
        /* Start transaction interrupt driven */
        // Enable master on bus and slave on bus interrupts
        i2c_inst->sercom->I2CM.INTENSET.reg = (SERCOM_I2CM_INTENSET_MB |
                                               SERCOM_I2CM_INTENSET_SB);
        // Write ADDR to start I2C transaction
        i2c_inst->sercom->I2CM.ADDR.reg = SERCOM_I2CM_ADDR_ADDR(addr);
    }
}

static inline void sercom_i2c_begin_register (
                                        struct sercom_i2c_desc_t *i2c_inst,
                                        struct sercom_i2c_transaction_t *state)
{
    uint8_t const addr = state->dev_address | 0;
    
    if ((state->type == I2C_TRANSACTION_REG_WRITE) && state->dma_out) {
        /* Start transaction with DMA */
        state->state = I2C_STATE_TX;
        uint8_t len = (uint8_t)(state->reg.data_length);

        // Configure second DMA descriptor for transfer
        // Second descriptor transfers data being written to register
        dma_config_desc(&i2c_inst->dma_desc, DMA_WIDTH_BYTE, state->reg.buffer,
                        1, (volatile uint8_t*)&i2c_inst->sercom->I2CM.DATA.reg,
                        0, len, NULL);
        // Configure first DMA descriptor and enable DMA channel
        // First descriptor transfers register address
        dma_config_transfer(i2c_inst->dma_chan, DMA_WIDTH_BYTE,
                            &state->reg.register_address, 1,
                            (volatile uint8_t*)&i2c_inst->sercom->I2CM.DATA.reg,
                            0, 1,
                            sercom_get_dma_tx_trigger(i2c_inst->sercom_instnum),
                            SERCOM_DMA_TX_PRIORITY, &i2c_inst->dma_desc);
        // Enable error interrupt
        i2c_inst->sercom->I2CM.INTENSET.reg = SERCOM_I2CM_INTENSET_ERROR;
        // Write ADDR to start I2C transaction
        i2c_inst->sercom->I2CM.ADDR.reg = (SERCOM_I2CM_ADDR_LEN(len + 1) |
                                           SERCOM_I2CM_ADDR_LENEN |
                                           SERCOM_I2CM_ADDR_ADDR(addr));
    } else {
        /* Start transaction interrupt driven */
        state->state = I2C_STATE_REG_ADDR;
        // Enable master on bus and slave on bus interrupts
        i2c_inst->sercom->I2CM.INTENSET.reg = (SERCOM_I2CM_INTENSET_MB |
                                               SERCOM_I2CM_INTENSET_SB);
        // Write ADDR to start I2C transaction
        i2c_inst->sercom->I2CM.ADDR.reg = SERCOM_I2CM_ADDR_ADDR(addr);
    }
}

static inline void sercom_i2c_end_transaction (
                                            struct sercom_i2c_desc_t *i2c_inst,
                                            struct transaction_t *t)
{
    struct sercom_i2c_transaction_t *s =
                                    (struct sercom_i2c_transaction_t*)t->state;

    // Mark transaction as done and not active
    transaction_queue_set_done(t);
    
    // Disable MB and SM interrupts
    i2c_inst->sercom->I2CM.INTENCLR.reg = (SERCOM_I2CM_INTENCLR_MB |
                                           SERCOM_I2CM_INTENCLR_SB |
                                           SERCOM_I2CM_INTENCLR_ERROR);

    // Run callback if there is one
    if (((s->type == I2C_TRANSACTION_REG_READ) ||
         (s->type == I2C_TRANSACTION_REG_WRITE)) &&
        (s->reg.callback != NULL)) {
        enum i2c_transaction_state const state = s->state;
        void *const context = s->reg.callback_context;
        transaction_queue_invalidate(t);
        s->reg.callback(state, context);
    }
    
    // Run the I2C service to start the next transaction if there is one
    sercom_i2c_service(i2c_inst);
}

static inline void sercom_i2c_begin_in_dma (
                                        struct sercom_i2c_desc_t *i2c_inst,
                                        struct sercom_i2c_transaction_t *state)
{
    // Transaction must be generic or reg read
    uint8_t const reg = (state->type == I2C_TRANSACTION_REG_READ);
    uint8_t const len = (uint8_t)(reg ? state->reg.data_length :
                                        state->generic.in_length);
    uint8_t *const buffer = reg ? state->reg.buffer : state->generic.in_buffer;
    
    // Begin reading bytes with DMA
    dma_config_transfer(i2c_inst->dma_chan, DMA_WIDTH_BYTE,
                        &i2c_inst->sercom->I2CM.DATA.reg, 0, buffer, 1, len,
                        sercom_get_dma_rx_trigger(i2c_inst->sercom_instnum),
                        SERCOM_DMA_RX_PRIORITY, NULL);
    // Disable MB and SM interrupts
    i2c_inst->sercom->I2CM.INTENCLR.reg = (SERCOM_I2CM_INTENCLR_MB |
                                           SERCOM_I2CM_INTENCLR_SB);
    // Enable error interrupt
    i2c_inst->sercom->I2CM.INTENSET.reg = SERCOM_I2CM_INTENSET_ERROR;
    // Set up to ACK bytes as we receive them
    i2c_inst->sercom->I2CM.CTRLB.bit.ACKACT = 0;
    // Write ADDR to start I2C transaction
    uint8_t const addr = state->dev_address | 1;
    i2c_inst->sercom->I2CM.ADDR.reg = (SERCOM_I2CM_ADDR_LEN(len) |
                                       SERCOM_I2CM_ADDR_LENEN |
                                       SERCOM_I2CM_ADDR_ADDR(addr));
}

void sercom_i2c_service (struct sercom_i2c_desc_t *i2c_inst)
{
    /* Acquire service function lock */
    if (i2c_inst->service_lock) {
        // Could not acquire lock, service is already being run
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
                // Begin reading bytes interrupt driven
                i2c_inst->sercom->I2CM.INTENSET.reg = (SERCOM_I2CM_INTENSET_MB |
                                                       SERCOM_I2CM_INTENSET_SB);
                s->state = I2C_STATE_RX;
                uint8_t const addr = s->dev_address | 1;
                i2c_inst->sercom->I2CM.ADDR.reg = SERCOM_I2CM_ADDR_ADDR(addr);
            }
        } else if ((i2c_inst->sercom->I2CM.STATUS.bit.BUSSTATE == 0x1) &&
                    s->state == I2C_STATE_WAIT_FOR_DONE) {
            // The I2C bus has returned to idle, we can let the CPU sleep again
            allow_sleep();

            // End transaction
            s->state = I2C_STATE_DONE;

            t->done = 1;
            t->active = 0;

            // Disable MB and SM interrupts
            i2c_inst->sercom->I2CM.INTENCLR.reg = (SERCOM_I2CM_INTENCLR_MB |
                                                   SERCOM_I2CM_INTENCLR_SB |
                                                   SERCOM_I2CM_INTENCLR_ERROR);
        }
        
        i2c_inst->service_lock = 0;
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
        i2c_inst->service_lock = 0;
        return;
    }

    // No transaction in progress, check if one needs to be started
    struct transaction_t *t = transaction_queue_next(&i2c_inst->queue);
    if (t == NULL) {
        // No pending transactions
        i2c_inst->service_lock = 0;
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
                i2c_inst->sercom->I2CM.INTENSET.reg = SERCOM_I2CM_INTENSET_MB;
                uint8_t const addr = s->dev_address << 1;
                i2c_inst->sercom->I2CM.ADDR.reg = SERCOM_I2CM_ADDR_ADDR(addr);
                break;
            default:
                break;
        }
    } else {
        // There is a pending transaction but the bus is not idle... eek
        i2c_inst->wait_for_idle = 1;
        // Keep checking if the bus has become idle as often as possible
        //inhibit_sleep();
    }

    i2c_inst->service_lock = 0;
}



void sercom_i2c_isr_mb (Sercom *sercom, uint8_t inst_num, void *state)
{
    struct sercom_i2c_desc_t *i2c_inst = (struct sercom_i2c_desc_t*)state;
    struct transaction_t *t = transaction_queue_get_active(&i2c_inst->queue);
    struct sercom_i2c_transaction_t *s =
                                    (struct sercom_i2c_transaction_t*)t->state;
    
    /* Master on Bus */
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
            s->scan.results[s->dev_address > 63] |= ((uint64_t)1 << (
                                                        (s->dev_address < 64) ?
                                                        (s->dev_address) :
                                                        (s->dev_address - 64)));
        }

        s->dev_address++;
        if (s->dev_address < 128) {
            // Send next address
            uint8_t const addr = s->dev_address << 1;
            i2c_inst->sercom->I2CM.ADDR.reg = SERCOM_I2CM_ADDR_ADDR(addr);
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
    } else if (s->type == I2C_TRANSACTION_GENERIC) {
        if (s->generic.bytes_out == s->generic.out_length) {
            // All bytes have been sent
            if (s->generic.in_length) {
                // There are bytes to be received, sent repeated start
                if (s->dma_in) {
                    // Begin reading bytes with DMA
                    sercom_i2c_begin_in_dma(i2c_inst, s);
                } else {
                    // Begin reading bytes interrupt driven
                    s->state = I2C_STATE_RX;
                    uint8_t const ad = s->dev_address | 1;
                    i2c_inst->sercom->I2CM.ADDR.reg = SERCOM_I2CM_ADDR_ADDR(ad);
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
            uint8_t const n = s->generic.out_buffer[s->generic.bytes_out++];
            i2c_inst->sercom->I2CM.DATA.reg = n;
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
                uint8_t const n = s->generic.out_buffer[s->reg.position++];
                i2c_inst->sercom->I2CM.DATA.reg = n;
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
                // Begin reading bytes interrupt driven
                uint8_t const addr = s->dev_address | 1;
                i2c_inst->sercom->I2CM.ADDR.reg = SERCOM_I2CM_ADDR_ADDR(addr);
            }
        } else {
            // Send register address
            i2c_inst->sercom->I2CM.DATA.reg = s->reg.register_address;
            s->state = I2C_STATE_RX;
        }
    }

    // Clear master on bus interrupt and error interrupt
    i2c_inst->sercom->I2CM.INTFLAG.reg |= (SERCOM_I2CM_INTFLAG_MB |
                                           SERCOM_I2CM_INTFLAG_ERROR);
}

void sercom_i2c_isr_sb (Sercom *sercom, uint8_t inst_num, void *state)
{
    struct sercom_i2c_desc_t *i2c_inst = (struct sercom_i2c_desc_t*)state;
    struct transaction_t *t = transaction_queue_get_active(&i2c_inst->queue);
    struct sercom_i2c_transaction_t *s =
                                    (struct sercom_i2c_transaction_t*)t->state;

    /* Slave on Bus */
    uint8_t generic_last = ((s->type == I2C_TRANSACTION_GENERIC) &&
                        (s->generic.bytes_in == (s->generic.in_length - 1)));
    uint8_t reg_last = ((s->type == I2C_TRANSACTION_REG_READ) &&
                        (s->reg.position == (s->reg.data_length - 1)));

    if (generic_last || reg_last) {
        // The last byte has been received, send NACK next
        i2c_inst->sercom->I2CM.CTRLB.bit.ACKACT = 1;
        // Send stop condition after byte read
        i2c_inst->sercom->I2CM.CTRLB.bit.CMD = 0x3;
        while (i2c_inst->sercom->I2CM.SYNCBUSY.bit.SYSOP);

        // Read last byte
        uint8_t const in = i2c_inst->sercom->I2CM.DATA.reg;
        if (s->type == I2C_TRANSACTION_GENERIC) {
            s->generic.in_buffer[s->generic.bytes_in] = in;
        } else if (s->type == I2C_TRANSACTION_REG_READ) {
            s->reg.buffer[s->reg.position] = in;
        }

        // Transaction done
        s->state = I2C_STATE_DONE;
        sercom_i2c_end_transaction(i2c_inst, t);
    } else {
        // A byte has been received, send ACK
        i2c_inst->sercom->I2CM.CTRLB.bit.ACKACT = 0;

        // Read byte
        uint8_t const in = i2c_inst->sercom->I2CM.DATA.reg;
        if (s->type == I2C_TRANSACTION_GENERIC) {
            s->generic.in_buffer[s->generic.bytes_in++] = in;
        } else if (s->type == I2C_TRANSACTION_REG_READ) {
            s->reg.buffer[s->reg.position++] = in;
        }

        // Receive next byte
        i2c_inst->sercom->I2CM.CTRLB.bit.CMD = 0x2;
        while (i2c_inst->sercom->I2CM.SYNCBUSY.bit.SYSOP);
    }

    // Clear slave on bus interrupt
    i2c_inst->sercom->I2CM.INTFLAG.reg |= (SERCOM_I2CM_INTFLAG_SB);
}

void sercom_i2c_isr_error (Sercom *sercom, uint8_t inst_num, void *state)
{
    struct sercom_i2c_desc_t *i2c_inst = (struct sercom_i2c_desc_t*)state;
    struct transaction_t *t = transaction_queue_get_active(&i2c_inst->queue);
    struct sercom_i2c_transaction_t *s =
                                    (struct sercom_i2c_transaction_t*)t->state;

    /* An error has occurred during a DMA driven transaction */
    // Abort DMA transaction
    dma_abort_transfer(i2c_inst->dma_chan);

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
                    // Wait for bus to become idle so that we can start receive
                    // stage
                    s->state = I2C_STATE_WAIT_FOR_RX;
                } else {
                    // Wait for bus to become idle so that we can end
                    s->state = I2C_STATE_WAIT_FOR_DONE;
                }
                // Inhibit sleep as the delay before the bus is idle may be much
                // less than the time that the CPU normally spends sleeping
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
            // less than the time that the CPU normally spends sleeping
            inhibit_sleep();
            break;
        default:
            break;
    }
}
