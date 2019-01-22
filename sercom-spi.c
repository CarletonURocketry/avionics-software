/**
 * @file sercom-spi.c
 * @desc SERCOM SPI mode driver which allows interupt or DMA driven transfers.
 * @author Samuel Dewan
 * @date 2019-01-02
 * Last Author:
 * Last Edited On:
 */

#include "sercom-spi.h"

#include "sercom-tools.h"
#include "dma.h"


#define SERCOM_SPI_BAUD_FALLBACK    1000000UL


static const uint8_t spi_dummy_byte = 0;


static void sercom_spi_isr (Sercom *sercom, uint8_t inst_num, void *state);
static void sercom_spi_dma_callback (uint8_t chan, void *state);


/**
 *  Start any pending transactions.
 *
 *  @param spi_inst The SPI instance for which the service should be run.
 */
static void sercom_spi_service (struct sercom_spi_desc_t *spi_inst);


void init_sercom_spi(struct sercom_spi_desc_t *descriptor,
                     Sercom *sercom, uint32_t core_freq,
                     uint32_t core_clock_mask, int8_t tx_dma_channel,
                     int8_t rx_dma_channel)
{
    uint8_t instance_num = sercom_get_inst_num(sercom);
    
    /* Enable the APBC clock for the SERCOM instance */
    PM->APBCMASK.reg |= sercom_get_pm_apb_mask(instance_num);
    
    /* Select the core clock for the SERCOM instance */
    GCLK->CLKCTRL.reg = (GCLK_CLKCTRL_CLKEN | core_clock_mask |
                         sercom_get_clk_id_mask(instance_num));
    // Wait for synchronization
    while (GCLK->STATUS.bit.SYNCBUSY);
    
    /* Reset SERCOM instance */
    sercom->SPI.CTRLA.bit.SWRST = 0b1;
    // Wait for reset to complete
    while (sercom->SPI.SYNCBUSY.bit.SWRST);
    
    /* Configure CTRL Reg A */
    // MISO on pad 2, MOSI on pad 0, SCK on pad 1, run in standby, master mode
    sercom->SPI.CTRLA.reg = (SERCOM_SPI_CTRLA_DIPO(0x2) |
                             SERCOM_SPI_CTRLA_DOPO(0x0) |
                             SERCOM_SPI_CTRLA_RUNSTDBY |
                             SERCOM_SPI_CTRLA_MODE_SPI_MASTER);
    
    /* Configure CTRL Reg B */
    // Set 8 bit characters
    sercom->SPI.CTRLB.reg = (SERCOM_SPI_CTRLB_CHSIZE(0x0));
    // Wait for synchronization
    while (sercom->SPI.SYNCBUSY.bit.CTRLB);
    
    /* Configure interupts */
    sercom_handlers[instance_num] = (struct sercom_handler_t) {
        .handler = sercom_spi_isr,
        .state = (void*)descriptor
    };
    
    NVIC_EnableIRQ(sercom_get_irq_num(instance_num));
    
    /* Setup Descriptor */
    descriptor->sercom = sercom;
    descriptor->sercom_instnum = instance_num;
    descriptor->core_frequency = core_freq;
    init_transaction_queue(&descriptor->queue, descriptor->transactions,
                           SERCOM_SPI_TRANSACTION_QUEUE_LENGTH,
                           descriptor->states,
                           sizeof(struct sercom_spi_transaction_t));
   
    
    // Configure DMA
    if ((tx_dma_channel >= 0) && (tx_dma_channel < DMAC_CH_NUM)) {
        descriptor->tx_dma_chan = (uint8_t)tx_dma_channel;
        descriptor->tx_use_dma = 0b1;
        
        dma_callbacks[tx_dma_channel] = (struct dma_callback_t) {
            .callback = sercom_spi_dma_callback,
            .state = (void*)descriptor
        };
    }
    if ((rx_dma_channel >= 0) && (rx_dma_channel < DMAC_CH_NUM)) {
        descriptor->rx_dma_chan = (uint8_t)rx_dma_channel;
        descriptor->rx_use_dma = 0b1;
        
        dma_callbacks[rx_dma_channel] = (struct dma_callback_t) {
            .callback = sercom_spi_dma_callback,
            .state = (void*)descriptor
        };
    }
}

uint8_t sercom_spi_start(struct sercom_spi_desc_t *spi_inst,
                         uint8_t *trans_id, uint32_t baudrate,
                         uint8_t cs_pin_group, uint32_t cs_pin_mask,
                         uint8_t *out_buffer, uint16_t out_length,
                         uint8_t * in_buffer, uint16_t in_length)
{
    struct transaction_t *t = transaction_queue_add(&spi_inst->queue);
    if (t == NULL) {
        return 1;
    }
    *trans_id = t->transaction_id;
    
    struct sercom_spi_transaction_t *state =
                                (struct sercom_spi_transaction_t*)t->state;
    
    state->out_buffer = out_buffer;
    state->in_buffer = in_buffer;
    state->out_length = out_length;
    state->in_length = in_length;
    state->baudrate = baudrate;
    state->cs_pin_group = cs_pin_group;
    state->cs_pin_mask = cs_pin_mask;
    state->rx_started = 0;
    
    state->bytes_out = 0;
    state->bytes_in = 0;
    
    
    transaction_queue_set_valid(t);
    
    sercom_spi_service(spi_inst);
    return 0;
}

uint8_t sercom_spi_transaction_done(struct sercom_spi_desc_t *spi_inst,
                                    uint8_t trans_id)
{
    return transaction_queue_is_done(
                            transaction_queue_get(&spi_inst->queue, trans_id));
}

uint8_t sercom_spi_clear_transaction(struct sercom_spi_desc_t *spi_inst,
                                     uint8_t trans_id)
{
    return transaction_queue_invalidate(
                            transaction_queue_get(&spi_inst->queue, trans_id));
}



static void sercom_spi_service (struct sercom_spi_desc_t *spi_inst)
{
    if (transaction_queue_head_active(&spi_inst->queue)) {
        // There is already a transaction in progress
        return;
    }
    
    struct transaction_t *t = transaction_queue_next(&spi_inst->queue);
    if (t == NULL) {
        // No pending transactions
        return;
    } else {
        // Start the next transaction
        struct sercom_spi_transaction_t *s =
                                    (struct sercom_spi_transaction_t*)t->state;
        
        /* Mark transaction as active */
        t->active = 1;
        
        /* Set baudrate */
        uint8_t baud_error = sercom_calc_sync_baud(s->baudrate,
                                            spi_inst->core_frequency,
                                            &(spi_inst->sercom->SPI.BAUD.reg));
        if (baud_error) {
            // Fallback to safe baud value
            sercom_calc_sync_baud(SERCOM_SPI_BAUD_FALLBACK,
                                  spi_inst->core_frequency,
                                  &(spi_inst->sercom->SPI.BAUD.reg));
        }
        
        /* Enable SERCOM instance */
        spi_inst->sercom->SPI.CTRLA.bit.ENABLE = 0b1;
        
        /* Assert CS line */
        PORT->Group[s->cs_pin_group].OUTCLR.reg = s->cs_pin_mask;
        
        /* Wait for SERCOM instance to be enabled */
        while (spi_inst->sercom->SPI.SYNCBUSY.bit.ENABLE);
        
        /* Begin transmition */
        if (spi_inst->tx_use_dma && s->out_length) {
            // Use DMA to transmit out buffer
            dma_start_buffer_to_static(spi_inst->tx_dma_chan, s->out_buffer,
                            s->out_length,(uint8_t*)&spi_inst->sercom->SPI.DATA,
                            sercom_get_dma_tx_trigger(spi_inst->sercom_instnum),
                            SERCOM_DMA_TX_PRIORITY);
        } else if (spi_inst->tx_use_dma) {
            // Simulate DMA transmition of out buffer having completed
            sercom_spi_dma_callback(spi_inst->tx_dma_chan, (void*)spi_inst);
        } else {
            // Use interupts
            spi_inst->sercom->SPI.INTENSET.bit.DRE = 0b1;
        }
    }
}

static inline void sercom_spi_end_transaction (struct sercom_spi_desc_t *spi_inst,
                                               struct transaction_t *t)
{
    struct sercom_spi_transaction_t *s =
                                    (struct sercom_spi_transaction_t*)t->state;
    
    t->done = 1;
    t->active = 0;
    
    // Disable DRE and RXC interutps
    spi_inst->sercom->SPI.INTENCLR.reg = (SERCOM_SPI_INTENCLR_DRE |
                                SERCOM_SPI_INTENCLR_RXC);
    
    // Disable Reciever and SERCOM
    spi_inst->sercom->SPI.CTRLB.bit.RXEN = 0b0;
    spi_inst->sercom->SPI.CTRLA.bit.ENABLE = 0b0;
    
    // Deassert the CS pin
    PORT->Group[s->cs_pin_group].OUTSET.reg = s->cs_pin_mask;
    
    // Run the SPI service to start the next transaction if there is one
    sercom_spi_service(spi_inst);
}

static inline void sercom_spi_start_reception (struct sercom_spi_desc_t *spi_inst,
                                               struct transaction_t *t)
{
    struct sercom_spi_transaction_t *s =
                                    (struct sercom_spi_transaction_t*)t->state;
    
    // Enable reception
    spi_inst->sercom->SPI.CTRLB.bit.RXEN = 0b1;
    
    if (spi_inst->rx_use_dma) {
        // Start DMA transaction to receive data
        dma_start_static_to_buffer(spi_inst->rx_dma_chan, s->in_buffer,
                        s->in_length, (uint8_t*)&spi_inst->sercom->SPI.DATA,
                        sercom_get_dma_rx_trigger(spi_inst->sercom_instnum),
                        SERCOM_DMA_RX_PRIORITY);
    } else {
        // Enable receive complete interupt
        spi_inst->sercom->SPI.INTENSET.bit.RXC = 0b1;
    }
    
    s->rx_started = 1;
}

static void sercom_spi_isr (Sercom *sercom, uint8_t inst_num, void *state)
{
    struct sercom_spi_desc_t *spi_inst = (struct sercom_spi_desc_t*)state;
    struct transaction_t *t = transaction_queue_get_active(&spi_inst->queue);
    struct sercom_spi_transaction_t *s =
                                    (struct sercom_spi_transaction_t*)t->state;
    
    // TX
    if (sercom->SPI.INTENSET.bit.DRE && sercom->SPI.INTFLAG.bit.DRE) {
        if (s->bytes_out < s->out_length) {
            // Send next byte
            sercom->SPI.DATA.reg = s->out_buffer[s->bytes_out];
            s->bytes_out++;
        } else if (!s->in_length) {
            // Transaction is complete
            sercom_spi_end_transaction(spi_inst, t);
        }  else if ((spi_inst->rx_use_dma && (s->bytes_in < s->in_length)) ||
                    (s->bytes_in < (s->in_length - 1))) {
            if (!s->rx_started) {
                // Need to enable receiver
                sercom_spi_start_reception(spi_inst, t);
            }
            // Send dummy byte
            sercom->SPI.DATA.reg = spi_dummy_byte;
            // If DMA is being used for reception, the bytes in count must be
            // inceremented here
            if (spi_inst->rx_use_dma) {
                s->bytes_in++;
            }
        } else {
            // No more bytes to be sent, disable data register empty interupt
            sercom->SPI.INTENCLR.bit.DRE = 0b0;
        }
    }
    
    // RX
    if (sercom->USART.INTFLAG.bit.RXC) {
        s->in_buffer[s->bytes_in] = sercom->SPI.DATA.reg;
        s->bytes_in++;
        
        if (s->bytes_in == s->in_length) {
            // Transaction done
            sercom_spi_end_transaction(spi_inst, t);
        } else if (!spi_inst->rx_use_dma) {
            // For some reason the RXC interupt seems to get disabled every time
            // the interupt service routine runs. Not clear why this happens, it
            // is not mentioned in the datasheet.
            sercom->USART.INTENSET.bit.RXC = 0b1;
        }
    }
}

static void sercom_spi_dma_callback (uint8_t chan, void *state)
{
    struct sercom_spi_desc_t *spi_inst = (struct sercom_spi_desc_t*)state;
    struct transaction_t *t = transaction_queue_get_active(&spi_inst->queue);
    struct sercom_spi_transaction_t *s =
                                    (struct sercom_spi_transaction_t*)t->state;
    
    if (spi_inst->tx_use_dma && chan == spi_inst->tx_dma_chan &&
                                !s->rx_started) {
        // TX stage is complete
        if (s->in_length) {
            // Start RX stage
            sercom_spi_start_reception(spi_inst, t);
            // Start DMA transaction to write dummy bytes
            dma_start_static_to_static(spi_inst->tx_dma_chan, &spi_dummy_byte,
                            s->in_length, (uint8_t*)&spi_inst->sercom->SPI.DATA,
                            sercom_get_dma_tx_trigger(spi_inst->sercom_instnum),
                            SERCOM_DMA_TX_PRIORITY);
        } else {
            // Transaction is complete
            sercom_spi_end_transaction(spi_inst, t);
        }
    } else if (spi_inst->rx_use_dma && chan == spi_inst->rx_dma_chan) {
        // Transaction is complete
        sercom_spi_end_transaction(spi_inst, t);
    }
}
