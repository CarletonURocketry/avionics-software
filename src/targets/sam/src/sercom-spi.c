/**
 * @file sercom-spi.c
 * @desc SERCOM SPI mode driver which allows interrupt or DMA driven transfers.
 * @author Samuel Dewan
 * @date 2019-01-02
 * Last Author:
 * Last Edited On:
 */

#include "sercom-spi.h"

#include "sercom-tools.h"
#include "dma.h"


#define SERCOM_SPI_BAUD_FALLBACK    1000000UL


static const uint8_t spi_dummy_byte = 0xFF;
// Address where uneeded input data can be dumped
static uint8_t spi_sink;


static void sercom_spi_isr_dre (Sercom *sercom, uint8_t inst_num, void *state);
static void sercom_spi_isr_txc (Sercom *sercom, uint8_t inst_num, void *state);
static void sercom_spi_isr_rxc (Sercom *sercom, uint8_t inst_num, void *state);
static void sercom_spi_tx_dma_callback (uint8_t chan, void *state);
static void sercom_spi_rx_dma_callback (uint8_t chan, void *state);


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
    enable_bus_clock(sercom_get_bus_clk(instance_num));

    /* Select the core clock for the SERCOM instance */
    set_perph_generic_clock(sercom_get_gclk(instance_num), core_clock_mask);

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

    /* Configure interrupts */
    sercom_handlers[instance_num] = (struct sercom_handler_t) {
        .dre_handler = sercom_spi_isr_dre,
        .txc_handler = sercom_spi_isr_txc,
        .rxc_handler = sercom_spi_isr_rxc,
        .state = (void*)descriptor
    };
    
    sercom_enable_interupts(instance_num, (SERCOM_SPI_INTFLAG_DRE |
                                           SERCOM_SPI_INTFLAG_TXC |
                                           SERCOM_SPI_INTFLAG_RXC));

    /* Setup Descriptor */
    descriptor->sercom = sercom;
    descriptor->sercom_instnum = instance_num;
    descriptor->core_frequency = core_freq;
    init_transaction_queue(&descriptor->queue, descriptor->transactions,
                           SERCOM_SPI_TRANSACTION_QUEUE_LENGTH,
                           descriptor->states,
                           sizeof(struct sercom_spi_transaction_t));
    descriptor->in_session = 0;
    descriptor->service_lock = 0;


    // Configure DMA
    if ((tx_dma_channel >= 0) && (tx_dma_channel < DMAC_CH_NUM)) {
        descriptor->tx_dma_chan = (uint8_t)tx_dma_channel;
        descriptor->tx_use_dma = 0b1;

        dma_callbacks[tx_dma_channel] = (struct dma_callback_t) {
            .callback = sercom_spi_tx_dma_callback,
            .state = (void*)descriptor
        };
    }
    if ((rx_dma_channel >= 0) && (rx_dma_channel < DMAC_CH_NUM)) {
        descriptor->rx_dma_chan = (uint8_t)rx_dma_channel;
        descriptor->rx_use_dma = 0b1;

        dma_callbacks[rx_dma_channel] = (struct dma_callback_t) {
            .callback = sercom_spi_rx_dma_callback,
            .state = (void*)descriptor
        };
    }
}

static inline void init_transaction(struct transaction_t *t, uint32_t baudrate,
                                    uint8_t cs_pin_group, uint32_t cs_pin_mask,
                                    uint8_t const *out_buffer,
                                    uint16_t out_length, uint8_t * in_buffer,
                                    uint16_t in_length, uint8_t multi_part,
                                    sercom_spi_transaction_cb_t callback,
                                    void *context)
{
    struct sercom_spi_transaction_t *state =
                                    (struct sercom_spi_transaction_t*)t->state;

    state->callback = callback;
    state->context = context;
    state->out_buffer = out_buffer;
    state->in_buffer = in_buffer;
    state->out_length = out_length;
    state->in_length = in_length;
    state->baudrate = baudrate;
    state->cs_pin_group = cs_pin_group;
    state->cs_pin_mask = cs_pin_mask;
    state->rx_started = 0;
    state->multi_part = multi_part;
    state->session = 0;
    state->simultaneous = 0;

    state->bytes_out = 0;
    state->bytes_in = 0;

    transaction_queue_set_valid(t);
}

uint8_t sercom_spi_start(struct sercom_spi_desc_t *spi_inst,
                         uint8_t *trans_id, uint32_t baudrate,
                         uint8_t cs_pin_group, uint32_t cs_pin_mask,
                         uint8_t *out_buffer, uint16_t out_length,
                         uint8_t *in_buffer, uint16_t in_length)
{
    return sercom_spi_start_with_cb(spi_inst, trans_id, baudrate, cs_pin_group,
                                    cs_pin_mask, out_buffer, out_length,
                                    in_buffer, in_length, NULL, NULL);
}

uint8_t sercom_spi_start_with_cb(struct sercom_spi_desc_t *spi_inst,
                                 uint8_t *trans_id, uint32_t baudrate,
                                 uint8_t cs_pin_group, uint32_t cs_pin_mask,
                                 uint8_t *out_buffer, uint16_t out_length,
                                 uint8_t * in_buffer, uint16_t in_length,
                                 sercom_spi_transaction_cb_t callback,
                                 void *context)
{
    // Try to get a transaction queue entry
    struct transaction_t *t = transaction_queue_add(&spi_inst->queue);
    if (t == NULL) {
        return 1;
    }

    // Initialize the transaction state
    init_transaction(t, baudrate, cs_pin_group, cs_pin_mask, out_buffer,
                     out_length, in_buffer, in_length, 0, callback, context);
    *trans_id = t->transaction_id;

    // Run the service to start a transaction if possible
    sercom_spi_service(spi_inst);
    return 0;
}

uint8_t sercom_spi_start_multi_part(struct sercom_spi_desc_t *spi_inst,
                                    struct sercom_spi_transaction_part_t *parts,
                                    uint8_t num_parts, uint8_t cs_pin_group,
                                    uint32_t cs_pin_mask)
{
    // Try to get a transaction queue entry for each part
    struct transaction_t *transactions[num_parts];
    for (uint8_t i = 0; i < num_parts; i++) {
        transactions[i] = transaction_queue_add(&spi_inst->queue);
        if (transactions[i] == NULL) {
            return 1;
        }
    }

    // Initialize each transaction state
    for (uint8_t i = 0; i < num_parts; i++) {
        init_transaction(transactions[i], parts[i].baudrate, cs_pin_group,
                         cs_pin_mask, parts[i].out_buffer, parts[i].out_length,
                         parts[i].in_buffer, parts[i].in_length,
                         i != (num_parts - 1), NULL, NULL);
        parts[i].transaction_id = transactions[i]->transaction_id;
    }

    // Run the service to start a transaction if possible
    sercom_spi_service(spi_inst);
    return 0;
}

uint8_t sercom_spi_transaction_done(struct sercom_spi_desc_t *spi_inst,
                                    uint8_t trans_id)
{
    // We run the SPI service here because we could theoretically get stalled if
    // the interrupt that signals the end of a transaction happens while the
    // main loop is in the exact wrong place in the sercom_spi_service function
    // and a transaction is pending.
    // If such a stall has happened we will get the next transaction started
    // here.
    sercom_spi_service(spi_inst);
    
    return transaction_queue_is_done(
                            transaction_queue_get(&spi_inst->queue, trans_id));
}

uint8_t sercom_spi_clear_transaction(struct sercom_spi_desc_t *spi_inst,
                                     uint8_t trans_id)
{
    struct transaction_t *t = transaction_queue_get(&spi_inst->queue, trans_id);
    struct sercom_spi_transaction_t *s =
                                    (struct sercom_spi_transaction_t*)t->state;

    if (s->session) {
        // Cannot clear a session, need to use sercom_spi_end_session() instead
        return 1;
    }

    return transaction_queue_invalidate(t);
}

uint8_t sercom_spi_start_session(struct sercom_spi_desc_t *spi_inst,
                                 uint8_t *trans_id, uint32_t baudrate,
                                 uint8_t cs_pin_group, uint32_t cs_pin_mask)
{
    // Try to get a transaction queue entry
    struct transaction_t *t = transaction_queue_add(&spi_inst->queue);
    if (t == NULL) {
        return 1;
    }

    struct sercom_spi_transaction_t *const state =
                                    (struct sercom_spi_transaction_t*)t->state;

    // Zero out all of the elements that are specific to individual transactions
    state->callback = NULL;
    state->context = NULL;
    state->out_buffer = NULL;
    state->in_buffer = NULL;
    state->out_length = 0;
    state->in_length = 0;
    state->bytes_in = 0;
    state->bytes_out = 0;
    state->rx_started = 0;
    state->simultaneous = 0;

    // Initialize elements that are constant for all transaction in the session
    state->baudrate = baudrate;
    state->cs_pin_mask = cs_pin_mask;
    state->cs_pin_group = cs_pin_group;
    state->multi_part = 0;
    state->session = 1;

    // Mark the transaction as done since this session does not yet have valid
    // transaction in it
    transaction_queue_set_done(t);

    // Mark transaction as valid
    transaction_queue_set_valid(t);

    // Store the transaction ID
    *trans_id = t->transaction_id;

    // Run the service to start a transaction if possible
    sercom_spi_service(spi_inst);
    return 0;
}

uint8_t sercom_spi_start_session_transaction(struct sercom_spi_desc_t *spi_inst,
                                             uint8_t trans_id,
                                             uint8_t const *out_buffer,
                                             uint16_t out_length,
                                             uint8_t * in_buffer,
                                             uint16_t in_length)
{
    // Get the transaction structure for the session transaction
    struct transaction_t *t = transaction_queue_get(&spi_inst->queue, trans_id);

    if (t == NULL) {
        // Session does not exist
        return 1;
    }

    // Get the current state for the session transaction
    struct sercom_spi_transaction_t *const s =
                                    (struct sercom_spi_transaction_t*)t->state;

    if (!s->session) {
        // Transaction is not a session
        return 1;
    }

    if (!t->done) {
        // There is already a transaction ongoing or ready to start in this
        // session
        return 1;
    }

    // Configure the state for this transaction
    s->callback = NULL;
    s->context = NULL;
    s->out_buffer = out_buffer;
    s->in_buffer = in_buffer;
    s->out_length = out_length;
    s->in_length = in_length;
    s->bytes_in = 0;
    s->bytes_out = 0;
    s->rx_started = 0;
    s->simultaneous = 0;

    // Mark this transaction as not being done yet so that it can be started
    t->done = 0;

    // Run the service to start the transaction if possible
    sercom_spi_service(spi_inst);
    return 0;
}

uint8_t sercom_spi_start_simultaneous_session_transaction(
                                            struct sercom_spi_desc_t *spi_inst,
                                            uint8_t trans_id,
                                            uint8_t const *out_buffer,
                                            uint8_t * in_buffer,
                                            uint16_t length)
{
    // Get the transaction structure for the session transaction
    struct transaction_t *t = transaction_queue_get(&spi_inst->queue, trans_id);

    if (t == NULL) {
        // Session does not exist
        return 1;
    }

    // Get the current state for the session transaction
    struct sercom_spi_transaction_t *const s =
    (struct sercom_spi_transaction_t*)t->state;

    if (!s->session) {
        // Transaction is not a session
        return 1;
    }

    if (!t->done) {
        // There is already a transaction ongoing or ready to start in this
        // session
        return 1;
    }

    // Configure the state for this transaction
    s->callback = NULL;
    s->context = NULL;
    s->out_buffer = out_buffer;
    s->in_buffer = in_buffer;
    s->out_length = 0;
    s->in_length = length;
    s->bytes_in = 0;
    s->bytes_out = 0;
    s->rx_started = 0;
    s->simultaneous = 1;

    // Mark this transaction as not being done yet so that it can be started
    t->done = 0;

    // Run the service to start the transaction if possible
    sercom_spi_service(spi_inst);
    return 0;
}

uint8_t sercom_spi_session_active(struct sercom_spi_desc_t *spi_inst,
                                  uint8_t trans_id)
{
    if (!spi_inst->in_session) {
        return 0;
    }

    struct transaction_t *t = transaction_queue_get_head(&spi_inst->queue);
    return t->transaction_id == trans_id;
}

uint8_t sercom_spi_end_session(struct sercom_spi_desc_t *spi_inst,
                               uint8_t trans_id)
{
    // Acquire service function lock, we are going to mess with the head of the
    // transaction queue in ways that could go badly if the service function is
    // run from an interrupt at the same time.
    if (spi_inst->service_lock) {
        // Could not acquire lock, service is already being run
        return 1;
    } else {
        spi_inst->service_lock = 1;
    }

    // Check if the session we are ending is currently ongoing
    uint8_t const is_active = sercom_spi_session_active(spi_inst, trans_id);

    struct transaction_t *const trans = transaction_queue_get(&spi_inst->queue,
                                                              trans_id);
    struct sercom_spi_transaction_t *const s =
                                (struct sercom_spi_transaction_t*)trans->state;

    uint8_t const ret = transaction_queue_invalidate(trans);

    if (ret != 0) {
        // Could not invalidate transaction
        spi_inst->service_lock = 0;
        return ret;
    }

    if (is_active) {
        // We just ended the current session
        spi_inst->in_session = 0;
        // De-assert CS line
        PORT->Group[s->cs_pin_group].OUTSET.reg = s->cs_pin_mask;
        // We might be able to start another transaction that was queued after
        // the session
        sercom_spi_service(spi_inst);
    }

    spi_inst->service_lock = 0;
    return 0;
}



static void sercom_spi_service (struct sercom_spi_desc_t *spi_inst)
{
    if (transaction_queue_head_active(&spi_inst->queue)) {
        // There is already a transaction in progress
        return;
    }

    /* Acquire service function lock */
    if (spi_inst->service_lock) {
        // Could not acquire lock, service is already being run
        return;
    } else {
        // Note that an interrupt could happen between when we check the service
        // lock and when we set the service lock. We don't care about this
        // because the entire service function will have run through in the ISR
        // before we set the service lock bit. This lock is not to protect
        // against multiple concurrent threads, it is just to keep the service
        // function from being started in an ISR if it is already in the middle
        // of being run in the main loop.
        spi_inst->service_lock = 1;
    }

    struct transaction_t *t = NULL;

    if (spi_inst->in_session) {
        // The head of the transaction queue is an active session
        t = transaction_queue_get_head(&spi_inst->queue);
    } else {
        // Get the next transaction to be started
        t = transaction_queue_next(&spi_inst->queue);

        if (t == NULL) {
            // No pending transactions
            goto done;
        }
    }

    struct sercom_spi_transaction_t *const s =
                                    (struct sercom_spi_transaction_t*)t->state;

    if (s->session && !spi_inst->in_session) {
        // We are entering a new session
        spi_inst->in_session = 1;
    }

    if (spi_inst->in_session && t->done) {
        // There is nothing to do for this session right now
        goto done;
    }

    /* Start the next transaction */
    // Mark transaction as active
    t->active = 1;

    // Set baudrate
    uint8_t const baud_error = sercom_calc_sync_baud(s->baudrate,
                                            spi_inst->core_frequency,
                                            &(spi_inst->sercom->SPI.BAUD.reg));
    if (baud_error) {
        // Fall back to safe baud value
        sercom_calc_sync_baud(SERCOM_SPI_BAUD_FALLBACK,
                              spi_inst->core_frequency,
                              &(spi_inst->sercom->SPI.BAUD.reg));
    }

    // Enable SERCOM instance
    spi_inst->sercom->SPI.CTRLA.bit.ENABLE = 0b1;

    // Wait for SERCOM instance to be enabled
    while (spi_inst->sercom->SPI.SYNCBUSY.bit.ENABLE);

    // Assert CS line
    PORT->Group[s->cs_pin_group].OUTCLR.reg = s->cs_pin_mask;

    // Begin transmission
    int const dma_tx = spi_inst->tx_use_dma && (s->out_length != 0);
    int const dma_rx = spi_inst->rx_use_dma && (s->in_length != 0);

    if (dma_tx && dma_rx) {
        // Use DMA for entire transaction with input and output stages

        // Enable reception
        spi_inst->sercom->SPI.CTRLB.bit.RXEN = 0b1;
        s->rx_started = 1;

        /* RX */
        // Configure second descriptor of RX DMA transfer. The second part
        // copies data to the receive buffer during the in stage of the
        // transaction.
        dma_config_desc(&spi_inst->rx_dma_desc, DMA_WIDTH_BYTE,
                        (volatile uint8_t*)&spi_inst->sercom->SPI.DATA, 0,
                        s->in_buffer, 1, s->in_length, NULL);
        // Configure first descriptor of RX DMA transfer and enable DMA channel.
        // The first part receives invalid bytes during the out stage of the
        // transaction.
        dma_config_transfer(spi_inst->rx_dma_chan, DMA_WIDTH_BYTE,
                            (volatile uint8_t*)&spi_inst->sercom->SPI.DATA, 0,
                            &spi_sink, 0, s->out_length,
                            sercom_get_dma_rx_trigger(spi_inst->sercom_instnum),
                            SERCOM_DMA_RX_PRIORITY, &spi_inst->rx_dma_desc);

        /* TX */
        // Configure second descriptor of TX DMA transfer. The second part sends
        // dummy bytes during the in stage of the transaction.
        dma_config_desc(&spi_inst->tx_dma_desc, DMA_WIDTH_BYTE, &spi_dummy_byte,
                        0, (volatile uint8_t*)&spi_inst->sercom->SPI.DATA, 0,
                        s->in_length, NULL);
        // Configure first descriptor of TX DMA transfer and enable DMA channel.
        // The first part sends the out stage of the transaction.
        dma_config_transfer(spi_inst->tx_dma_chan, DMA_WIDTH_BYTE,
                            s->out_buffer, 1,
                            (volatile uint8_t*)&spi_inst->sercom->SPI.DATA, 0,
                            s->out_length,
                            sercom_get_dma_tx_trigger(spi_inst->sercom_instnum),
                            SERCOM_DMA_TX_PRIORITY, &spi_inst->tx_dma_desc);
    } else if (dma_tx) {
        // Use DMA to transmit out buffer
        dma_config_transfer(spi_inst->tx_dma_chan, DMA_WIDTH_BYTE,
                            s->out_buffer, 1,
                            (volatile uint8_t*)&spi_inst->sercom->SPI.DATA, 0,
                            s->out_length,
                            sercom_get_dma_tx_trigger(spi_inst->sercom_instnum),
                            SERCOM_DMA_TX_PRIORITY, NULL);
    } else {
        // We are using interrupt driven transmission and/or this transaction
        // does not have any out stage. Either way, the DRE interrupt will do
        // the right thing.
        spi_inst->sercom->SPI.INTENSET.bit.DRE = 0b1;
    }

done:
    spi_inst->service_lock = 0;
}

static inline void sercom_spi_end_transaction (
                                            struct sercom_spi_desc_t *spi_inst,
                                            struct transaction_t *t)
{
    struct sercom_spi_transaction_t *s =
                                    (struct sercom_spi_transaction_t*)t->state;

    // Disable DRE and RXC interrupts
    spi_inst->sercom->SPI.INTENCLR.reg = (SERCOM_SPI_INTENCLR_DRE |
                                          SERCOM_SPI_INTENCLR_RXC);

    // Mark transaction as done and not active
    transaction_queue_set_done(t);

    // Deassert the CS pin if there are no further parts to this transaction
    if (!s->multi_part && !s->session) {
        PORT->Group[s->cs_pin_group].OUTSET.reg = s->cs_pin_mask;
    }

    // Disable Receiver and SERCOM
    spi_inst->sercom->SPI.CTRLB.bit.RXEN = 0b0;
    spi_inst->sercom->SPI.CTRLA.bit.ENABLE = 0b0;

    // Check if there is a callback for this transaction
    if (s->callback) {
        // Automatically clear transction
        transaction_queue_invalidate(t);
        // Call callback
        s->callback(s->context);
    }

    // Run the SPI service to start the next transaction if there is one
    sercom_spi_service(spi_inst);
}

static inline void sercom_spi_start_reception (
                                            struct sercom_spi_desc_t *spi_inst,
                                            struct transaction_t *t)
{
    struct sercom_spi_transaction_t *s =
                                    (struct sercom_spi_transaction_t*)t->state;

    // Enable reception
    spi_inst->sercom->SPI.CTRLB.bit.RXEN = 0b1;

    if (spi_inst->rx_use_dma) {
        // Start DMA transaction to receive data
        dma_config_transfer(spi_inst->rx_dma_chan, DMA_WIDTH_BYTE,
                            (volatile uint8_t*)&spi_inst->sercom->SPI.DATA, 0,
                            s->in_buffer, 1, s->in_length,
                            sercom_get_dma_rx_trigger(spi_inst->sercom_instnum),
                            SERCOM_DMA_RX_PRIORITY, NULL);
    } else {
        // Enable receive complete interrupt
        spi_inst->sercom->SPI.INTENSET.bit.RXC = 0b1;
    }

    s->rx_started = 1;

    if (spi_inst->tx_use_dma && !s->simultaneous) {
        // Start DMA transaction to write dummy bytes
        dma_config_transfer(spi_inst->tx_dma_chan, DMA_WIDTH_BYTE,
                            &spi_dummy_byte, 0,
                            (volatile uint8_t*)&spi_inst->sercom->SPI.DATA, 0,
                            s->in_length,
                            sercom_get_dma_tx_trigger(spi_inst->sercom_instnum),
                            SERCOM_DMA_TX_PRIORITY, NULL);
    } else if (spi_inst->tx_use_dma) {
        // Start DMA transaction to write non-dummy bytes
        dma_config_transfer(spi_inst->tx_dma_chan, DMA_WIDTH_BYTE,
                            s->out_buffer, 1,
                            (volatile uint8_t*)&spi_inst->sercom->SPI.DATA, 0,
                            s->in_length,
                            sercom_get_dma_tx_trigger(spi_inst->sercom_instnum),
                            SERCOM_DMA_TX_PRIORITY, NULL);
    } else {
        s->dummy_bytes_out = 0;
        // Re-enable the data register empty interrupt
        spi_inst->sercom->SPI.INTENSET.bit.DRE = 0b1;
    }
}

static void sercom_spi_isr_dre (Sercom *sercom, uint8_t inst_num, void *state)
{
    struct sercom_spi_desc_t *spi_inst = (struct sercom_spi_desc_t*)state;
    struct transaction_t *t = transaction_queue_get_active(&spi_inst->queue);
    if (t == NULL) {
        return;
    }
    struct sercom_spi_transaction_t *s =
                                    (struct sercom_spi_transaction_t*)t->state;

    uint8_t virtual_txc = 0;

    /* Data Register Empty */
    if ((!s->rx_started && (s->bytes_out < s->out_length)) ||
        (s->simultaneous && s->rx_started && (s->bytes_in < s->in_length))) {
        // Send next byte
        sercom->SPI.DATA.reg = s->out_buffer[s->bytes_out];
        s->bytes_out++;
    } else if (s->in_length == 0) {
        // Transaction is complete
        sercom->SPI.INTENSET.bit.TXC = 0b1;
    } else if (!s->rx_started) {
        /* Let the transmission end and start reception in the TXC ISR */
        // Disable DRE ISR so that we don't get stuck in it infinitely
        sercom->SPI.INTENCLR.bit.DRE = 0b1;
        if (s->bytes_out != 0) {
            // Enable TXC ISR
            sercom->SPI.INTENSET.bit.TXC = 0b1;
        } else {
            // Since we didn't actually send any bytes the TXC interrupt
            // will not be asserted. Use a flag to jump into TCX
            // handling code later in this call to the ISR.
            virtual_txc = 1;
        }
    } else if (s->dummy_bytes_out < s->in_length) {
        // Send dummy byte
        sercom->SPI.DATA.reg = spi_dummy_byte;
        s->dummy_bytes_out++;
        // If DMA is being used for reception, the bytes in count must
        // be incremented here
        if (spi_inst->rx_use_dma) {
            s->bytes_in++;
        }
    } else {
        // No more bytes to be sent, disable data register empty interrupt
        sercom->SPI.INTENCLR.bit.DRE = 0b1;
    }

    // Transmit Complete
    if (virtual_txc) {
        sercom_spi_isr_txc(sercom, inst_num, state);
    }

    if (!spi_inst->rx_use_dma) {
        // For some reason the RXC interrupt seems to get disabled every
        // time the interrupt service routine runs. Not clear why this
        // happens, it is not mentioned in the datasheet.
        sercom->SPI.INTENSET.bit.RXC = 0b1;
    }
}




static void sercom_spi_isr_txc (Sercom *sercom, uint8_t inst_num, void *state)
{
    struct sercom_spi_desc_t *spi_inst = (struct sercom_spi_desc_t*)state;
    struct transaction_t *t = transaction_queue_get_active(&spi_inst->queue);
    if (t == NULL) {
        return;
    }
    struct sercom_spi_transaction_t *s =
                                    (struct sercom_spi_transaction_t*)t->state;


    /* Transmit Complete */
    sercom->SPI.INTENCLR.bit.TXC = 0b1;
    if (s->in_length) {
        // Need to enable receiver
        sercom_spi_start_reception(spi_inst, t);
    } else {
        // Transaction is complete
        sercom_spi_end_transaction(spi_inst, t);
    }
}

static void sercom_spi_isr_rxc (Sercom *sercom, uint8_t inst_num, void *state)
{
    struct sercom_spi_desc_t *spi_inst = (struct sercom_spi_desc_t*)state;
    struct transaction_t *t = transaction_queue_get_active(&spi_inst->queue);
    if (t == NULL) {
        return;
    }
    struct sercom_spi_transaction_t *s =
                                    (struct sercom_spi_transaction_t*)t->state;

    /* Receive Complete */
    // Get the received byte
    s->in_buffer[s->bytes_in] = sercom->SPI.DATA.reg;
    s->bytes_in++;

    if (s->bytes_in == s->in_length) {
        // Transaction done
        sercom_spi_end_transaction(spi_inst, t);
    } else if (!spi_inst->rx_use_dma) {
        // For some reason the RXC interrupt seems to get disabled every
        // time the interrupt service routine runs. Not clear why this
        // happens, it is not mentioned in the datasheet.
        sercom->SPI.INTENSET.bit.RXC = 0b1;
    }
}

static void sercom_spi_tx_dma_callback (uint8_t chan, void *state)
{
    struct sercom_spi_desc_t *spi_inst = (struct sercom_spi_desc_t*)state;
    struct transaction_t *t = transaction_queue_get_active(&spi_inst->queue);
    
    if (spi_inst->tx_use_dma && (chan == spi_inst->tx_dma_chan) &&
        t == NULL) {
        // TX transaction for RX stage completed after RX transaction, ignore
        return;
    }
    
    struct sercom_spi_transaction_t *s =
                                    (struct sercom_spi_transaction_t*)t->state;

    if (spi_inst->tx_use_dma && !s->rx_started) {
        // TX stage is complete
        spi_inst->sercom->SPI.INTENSET.bit.TXC = 0b1;
    }
}

static void sercom_spi_rx_dma_callback (uint8_t chan, void *state)
{
    struct sercom_spi_desc_t *spi_inst = (struct sercom_spi_desc_t*)state;
    struct transaction_t *t = transaction_queue_get_active(&spi_inst->queue);

    if (spi_inst->rx_use_dma) {
        // Transaction is complete
        sercom_spi_end_transaction(spi_inst, t);
    }
}
