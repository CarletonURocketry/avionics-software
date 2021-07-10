/**
 * @file dma.c
 * @desc Abstraction layer for configuring DMAC
 * @author Samuel Dewan
 * @date 2018-12-30
 * Last Author: Samuel Dewan
 * Last Edited On: 2019-04-27
 */

#include "dma.h"

#define DMA_IRQ_PRIORITY    2


static DmacDescriptor dmacDescriptors_g[DMAC_CH_NUM] __attribute__((aligned(16)));
static DmacDescriptor dmacWriteBack_g[DMAC_CH_NUM] __attribute__((aligned(16)));


struct dma_callback_t dma_callbacks[DMAC_CH_NUM];
static struct dma_circ_transfer_t *dmaCircBufferTransfers[DMAC_CH_NUM];


void init_dmac(void)
{
    /* Reset the DMAC */
    DMAC->CTRL.bit.SWRST = 0b1;
    // Wait for reset to complete
    while (DMAC->CTRL.bit.SWRST);
    
    /* Write base addresses to registers */
    DMAC->BASEADDR.reg = (uint32_t)&dmacDescriptors_g;
    DMAC->WRBADDR.reg = (uint32_t)&dmacWriteBack_g;
    
    /* Set arbitration scheme for all priority levels to round robin */
    DMAC->PRICTRL0.reg = (DMAC_PRICTRL0_RRLVLEN0 | DMAC_PRICTRL0_RRLVLEN1 |
                          DMAC_PRICTRL0_RRLVLEN2 | DMAC_PRICTRL0_RRLVLEN3);
    
    /* Enable all priority levels */
    DMAC->CTRL.reg = (DMAC_CTRL_LVLEN0 | DMAC_CTRL_LVLEN1 | DMAC_CTRL_LVLEN2 |
                      DMAC_CTRL_LVLEN3);
    
    /* Enable DMAC interrupts */
#if defined(SAMD2x)
    NVIC_SetPriority (DMAC_IRQn, DMA_IRQ_PRIORITY);
    NVIC_EnableIRQ(DMAC_IRQn);
#elif defined(SAMx5x)
    // The first 4 DMA channels have their own interrupts. We just don't use
    // those channels to make things easy. In the future we could use those
    // channels for slightly higher performance stuff with a custom DMA driver.
    NVIC_SetPriority (DMAC_4_IRQn, DMA_IRQ_PRIORITY);
    NVIC_EnableIRQ(DMAC_4_IRQn);
#endif

    /* Enable DMA module */
    DMAC->CTRL.bit.DMAENABLE = 0b1;
}


/**
 *  Configure the channel registers for a DMA transfer.
 *
 *  @param chan The channel number for the channel to configure
 *  @param trigger The trigger number for the channel
 *  @param priority The priority level for the channel
 */
static void dma_config_channel(uint8_t chan, uint8_t trigger, uint8_t priority)
{
#if defined(SAMD2x)
    /* Select DMA channel to configure */
    DMAC->CHID.reg = chan;

    /* Reset DMA channel */
    DMAC->CHCTRLA.bit.SWRST = 0b1;
    // Wait for reset to complete
    while (DMAC->CHCTRLA.bit.SWRST);

    /* Configure DMA Channel */
    // Require one trigger per beat, select trigger source and select priority
    DMAC->CHCTRLB.reg = (DMAC_CHCTRLB_TRIGACT_BEAT |
                         DMAC_CHCTRLB_TRIGSRC(trigger) |
                         DMAC_CHCTRLB_LVL(priority));
    // Enable transfer complete interupt
    DMAC->CHINTENSET.bit.TCMPL = 0b1;
#elif defined(SAMx5x)
    /* Reset DMA channel */
    DMAC->Channel[chan].CHCTRLA.bit.SWRST = 0b1;
    // Wait for reset to complete
    while (DMAC->Channel[chan].CHCTRLA.bit.SWRST);

    /* Configure DMA Channel */
    // Require one trigger per beat, select trigger source and select priority
    DMAC->Channel[chan].CHCTRLA.reg = (DMAC_CHCTRLA_TRIGSRC(trigger) |
                                       DMAC_CHCTRLA_TRIGACT_BURST |
                                       DMAC_CHCTRLA_BURSTLEN_SINGLE);
    // Configure priority
    DMAC->Channel[chan].CHPRILVL.bit.PRILVL = priority;
    // Enable transfer complete interupt
    DMAC->Channel[chan].CHINTENSET.bit.TCMPL = 0b1;
#endif
}

/**
 *  Enable a DMA channel.
 *
 *  @param chan The channel number for the channel to enable
 */
static inline void dma_enable_channel(uint8_t chan)
{
#if defined(SAMD2x)
    DMAC->CHCTRLA.bit.ENABLE = 0b1;
#elif defined(SAMx5x)
    DMAC->Channel[chan].CHCTRLA.bit.ENABLE = 0b1;
#endif
}

void dma_config_desc(DmacDescriptor *desc, enum dma_width beatsize,
                     const volatile void *source, int increment_source,
                     volatile void *destination, int increment_destination,
                     uint16_t length, DmacDescriptor *next)
{
    /* Configure transfer descriptor */
    // Set beatsize, mark descriptor as valid and enable interrupt after block
    desc->BTCTRL.reg = (DMAC_BTCTRL_BEATSIZE(beatsize) |
                        DMAC_BTCTRL_VALID |
                        ((next == NULL) ? DMAC_BTCTRL_BLOCKACT_INT :
                                          DMAC_BTCTRL_BLOCKACT_NOACT));
    // Configure source and destination address incrementing
    desc->BTCTRL.bit.SRCINC = !!increment_source;
    desc->BTCTRL.bit.DSTINC = !!increment_destination;

    // Set source and destination addresses
    uint32_t const inc = length * (beatsize + 1);
    uint32_t const source_inc = increment_source ? inc : 0;
    desc->SRCADDR.reg = (uint32_t)source + source_inc;
    uint32_t const dest_inc = increment_destination ? inc : 0;
    desc->DSTADDR.reg = (uint32_t)destination + dest_inc;

    // Select block transfer count
    desc->BTCNT.reg = length;

    // Set next descriptor address
    desc->DESCADDR.reg = (uint32_t)next;
}

void dma_config_transfer(uint8_t chan, enum dma_width beatsize,
                         const volatile void *source, int increment_source,
                         volatile void *destination, int increment_destination,
                         uint16_t length, uint8_t trigger, uint8_t priority,
                         DmacDescriptor *next)
{
    /* Configure DMA channel */
    dma_config_channel(chan, trigger, priority);

    /* Configure transfer descriptor */
    dma_config_desc(&dmacDescriptors_g[chan], beatsize, source,
                    increment_source, destination, increment_destination,
                    length, next);

    /* Enable channel */
    dma_enable_channel(chan);
}



int8_t dma_config_circular_buffer_to_static(struct dma_circ_transfer_t *tran,
                                            uint8_t chan,
                                            struct circular_buffer_t *buffer,
                                            volatile uint8_t *dest,
                                            uint8_t trigger, uint8_t priority)
{
    if (buffer->head == buffer->tail) {
        return 1;
    }

    /* Configure DMA channel */
    dma_config_channel(chan, trigger, priority);

    /* Configure transfer descriptor(s) */
    // Ensure that the step size setting does not apply to source address,
    // enable incrementing of source address, set beat size to one byte and mark
    // descriptor as valid
    dmacDescriptors_g[chan].BTCTRL.reg = (DMAC_BTCTRL_STEPSEL_DST |
                                          DMAC_BTCTRL_SRCINC |
                                          DMAC_BTCTRL_BEATSIZE_BYTE |
                                          DMAC_BTCTRL_VALID |
                                          DMAC_BTCTRL_BLOCKACT_NOACT);

    // Destination addresses
    dmacDescriptors_g[chan].DSTADDR.reg = (uint32_t)dest;

    if (buffer->tail > buffer->head) {
        // Select block transfer count
        dmacDescriptors_g[chan].BTCNT.reg = (buffer->tail - buffer->head);

        // Source address
        dmacDescriptors_g[chan].SRCADDR.reg = (uint32_t)(buffer->buffer
                                                         + buffer->tail);

        // Set next descriptor address
        dmacDescriptors_g[chan].DESCADDR.reg = 0x0;

        // Enable interupt on block completion
        dmacDescriptors_g[chan].BTCTRL.bit.BLOCKACT =
                                                DMAC_BTCTRL_BLOCKACT_INT_Val;
    } else {
        // Select block transfer count
        dmacDescriptors_g[chan].BTCNT.reg = (buffer->capacity - buffer->head);

        // Source address
        dmacDescriptors_g[chan].SRCADDR.reg = (uint32_t)(buffer->buffer
                                                         + buffer->capacity);

        if (buffer->tail == 0) {
            // Set next descriptor address
            dmacDescriptors_g[chan].DESCADDR.reg = 0x0;
        } else {
            // Set next descriptor address
            dmacDescriptors_g[chan].DESCADDR.reg =
            (uint32_t)(&tran->second_descriptor);

            /* Configure second descriptor */
            // Ensure that the step size setting does not apply to source
            // address, enable incrementing of source address, set beatsize to
            // one byte and mark descriptor as valid
            tran->second_descriptor.BTCTRL.reg = (DMAC_BTCTRL_STEPSEL_DST |
                                                  DMAC_BTCTRL_SRCINC |
                                                  DMAC_BTCTRL_BEATSIZE_BYTE |
                                                  DMAC_BTCTRL_VALID |
                                                  DMAC_BTCTRL_BLOCKACT_INT);
            // Set source and destination addresses
            tran->second_descriptor.SRCADDR.reg = (uint32_t)(buffer->buffer
                                                             + buffer->tail);
            tran->second_descriptor.DSTADDR.reg = (uint32_t)dest;

            // Select block transfer count
            tran->second_descriptor.BTCNT.reg = buffer->tail;

            // Set next descriptor address
            tran->second_descriptor.DESCADDR.reg = 0x0;
        }

    }

    /* Set up transfer descriptor */
    tran->buffer = buffer;
    tran->length = buffer->length;
    tran->valid = 0b1;
    dmaCircBufferTransfers[chan] = tran;

    /* Enable channel */
    dma_enable_channel(chan);

    return 0;
}


void dma_abort_transfer(uint8_t chan)
{
    // Disable DMA channel, if transaction is in progress it will be aborted
    // gracefully.
#if defined(SAMD2x)
    DMAC->CHID.bit.ID = chan;
    DMAC->CHCTRLA.bit.ENABLE = 0;
#elif defined(SAMx5x)
    DMAC->Channel[chan].CHCTRLA.bit.ENABLE = 0;
#endif
}

#if defined(SAMD2x)
void DMAC_Handler (void)
#elif defined(SAMx5x)
void DMAC_4_Handler (void)
#endif
{
#if defined(SAMD2x)
    // Save the currently selected channel in case an interupt happens during
    // channel configuration
    uint8_t old_chan = DMAC->CHID.bit.ID;
#endif

    // Iterate through all of the channels with pending interupts by selecting
    // the lowest channel with an interupt from the interupt pending register
    // until there are no channels with pending interupts left.
    while (DMAC->INTPEND.reg & (DMAC_INTPEND_SUSP | DMAC_INTPEND_TCMPL |
                                DMAC_INTPEND_TERR)) {
        // Select the lowest channel with an interupt
#if defined(SAMD2x)
        DMAC->CHID.bit.ID = DMAC->INTPEND.bit.ID;

        if (DMAC->CHINTFLAG.bit.SUSP) {
            // Clear interupt
            DMAC->CHINTENCLR.bit.SUSP = 0b1;
        }

        if (DMAC->CHINTFLAG.bit.TCMPL) {
            if (dmaCircBufferTransfers[DMAC->CHID.bit.ID]->valid) {
                // A circular buffer DMA transfer has finished
                // The head of the buffer must be moved
                circular_buffer_move_head(
                            dmaCircBufferTransfers[DMAC->CHID.bit.ID]->buffer,
                            dmaCircBufferTransfers[DMAC->CHID.bit.ID]->length);
                // The transaction is done now, so we need to mark it invalid
                dmaCircBufferTransfers[DMAC->CHID.bit.ID]->valid = 0b0;
            }

            struct dma_callback_t *c = dma_callbacks + DMAC->CHID.bit.ID;

            // Clear interupt
            DMAC->CHINTENCLR.bit.TCMPL = 0b1;

            // Disable channel
            DMAC->CHCTRLA.bit.ENABLE = 0b0;

            if (c->callback != NULL) {
                c->callback(DMAC->CHID.bit.ID, c->state);
            }
        }

        if (DMAC->CHINTFLAG.bit.TERR) {
            // Clear interupt
            DMAC->CHINTENCLR.bit.TERR = 0b1;
        }
#elif defined(SAMx5x)
        uint8_t const chan = DMAC->INTPEND.bit.ID;

        if (DMAC->Channel[chan].CHINTFLAG.bit.SUSP) {
            // Clear interupt
            DMAC->Channel[chan].CHINTENCLR.bit.SUSP = 0b1;;
        }

        if (DMAC->Channel[chan].CHINTFLAG.bit.TCMPL) {
            if (dmaCircBufferTransfers[chan]->valid) {
                // A circular buffer DMA transfer has finished
                // The head of the buffer must be moved
                circular_buffer_move_head(dmaCircBufferTransfers[chan]->buffer,
                                          dmaCircBufferTransfers[chan]->length);
                // The transaction is done now, so we need to mark it invalid
                dmaCircBufferTransfers[chan]->valid = 0b0;
            }

            struct dma_callback_t *c = dma_callbacks + chan;

            // Clear interupt
            DMAC->Channel[chan].CHINTENCLR.bit.TCMPL = 0b1;

            // Disable channel
            DMAC->Channel[chan].CHCTRLA.bit.ENABLE = 0b0;

            if (c->callback != NULL) {
                c->callback(chan, c->state);
            }
        }

        if (DMAC->Channel[chan].CHINTFLAG.bit.TERR) {
            // Clear interupt
            DMAC->Channel[chan].CHINTENCLR.bit.TERR = 0b1;
        }
#endif
    }

#if defined(SAMD2x)
    // Restore previously selected channel
    DMAC->CHID.bit.ID = old_chan;
#endif
}
