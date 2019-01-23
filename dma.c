/**
 * @file dma.c
 * @desc Abstraction layer for configuring DMAC
 * @author Samuel Dewan
 * @date 2018-12-30
 * Last Author:
 * Last Edited On:
 */

#include "dma.h"

#include "interupts.h"


static DmacDescriptor dmacDescriptors_g[DMAC_CH_NUM];
static DmacDescriptor dmacWriteBack_g[DMAC_CH_NUM];


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
    
    /* Enable DMAC interupts */
    NVIC_EnableIRQ(DMAC_IRQn);

    /* Enable DMA module */
    DMAC->CTRL.bit.DMAENABLE = 0b1;
}



int8_t dma_start_circular_buffer_to_static(struct dma_circ_transfer_t *tran,
                                        uint8_t chan,
                                        struct circular_buffer_t *buffer,
                                        uint8_t *dest, uint8_t trigger,
                                        uint8_t priority)
{
    if (buffer->head == buffer->tail) {
        return 1;
    }
    
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
    
    /* Configure transfer descriptor(s) */
    // Ensure that the step size setting does not apply to source address,
    // enable incrementing of source address, set beatsize to one byte and mark
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
    }   else {
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
    tran->orig_tail = buffer->tail;
    tran->valid = 0b1;
    dmaCircBufferTransfers[chan] = tran;
    
    /* Enable channel */
    DMAC->CHCTRLA.bit.ENABLE = 0b1;
    
    return 0;
}

void dma_start_buffer_to_static(uint8_t chan, const uint8_t *buffer,
                                uint16_t length, uint8_t *dest, uint8_t trigger,
                                uint8_t priority)
{
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
    
    /* Configure transfer descriptor */
    // Ensure that the step size setting does not apply to source address,
    // enable incrementing of source address, set beatsize to one byte and mark
    // descriptor as valid
    dmacDescriptors_g[chan].BTCTRL.reg = (DMAC_BTCTRL_STEPSEL_DST |
                                          DMAC_BTCTRL_SRCINC |
                                          DMAC_BTCTRL_BEATSIZE_BYTE |
                                          DMAC_BTCTRL_VALID |
                                          DMAC_BTCTRL_BLOCKACT_INT);
    
    // Set source and destination addresses
    dmacDescriptors_g[chan].SRCADDR.reg = (uint32_t)buffer + length;
    dmacDescriptors_g[chan].DSTADDR.reg = (uint32_t)dest;
    
    // Select block transfer count
    dmacDescriptors_g[chan].BTCNT.reg = length;
    
    // Set next descriptor address
    dmacDescriptors_g[chan].DESCADDR.reg = 0x0;
    
    /* Enable channel */
    DMAC->CHCTRLA.bit.ENABLE = 0b1;
}

void dma_start_static_to_buffer(uint8_t chan, uint8_t *buffer, uint16_t length,
                                const uint8_t *source, uint8_t trigger,
                                uint8_t priority)
{
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
    
    /* Configure transfer descriptor */
    // Ensure that the step size setting does not apply to destination address,
    // enable incrementing of destination address, set beatsize to one byte and
    // mark descriptor as valid
    dmacDescriptors_g[chan].BTCTRL.reg = (DMAC_BTCTRL_STEPSEL_SRC |
                                          DMAC_BTCTRL_DSTINC |
                                          DMAC_BTCTRL_BEATSIZE_BYTE |
                                          DMAC_BTCTRL_VALID |
                                          DMAC_BTCTRL_BLOCKACT_INT);
    
    // Set source and destination addresses
    dmacDescriptors_g[chan].SRCADDR.reg = (uint32_t)source;
    dmacDescriptors_g[chan].DSTADDR.reg = (uint32_t)buffer + length;
    
    // Select block transfer count
    dmacDescriptors_g[chan].BTCNT.reg = length;
    
    // Set next descriptor address
    dmacDescriptors_g[chan].DESCADDR.reg = 0x0;
    
    /* Enable channel */
    DMAC->CHCTRLA.bit.ENABLE = 0b1;
}

void dma_start_static_to_static(uint8_t chan, const uint8_t *source,
                                uint16_t length, uint8_t *dest, uint8_t trigger,
                                uint8_t priority)
{
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
    
    /* Configure transfer descriptor */
    // Set beatsize to one byte and mark descriptor as valid
    dmacDescriptors_g[chan].BTCTRL.reg = (DMAC_BTCTRL_BEATSIZE_BYTE |
                                          DMAC_BTCTRL_VALID |
                                          DMAC_BTCTRL_BLOCKACT_INT);
    
    // Set source and destination addresses
    dmacDescriptors_g[chan].SRCADDR.reg = (uint32_t)source;
    dmacDescriptors_g[chan].DSTADDR.reg = (uint32_t)dest;
    
    // Select block transfer count
    dmacDescriptors_g[chan].BTCNT.reg = length;
    
    // Set next descriptor address
    dmacDescriptors_g[chan].DESCADDR.reg = 0x0;
    
    /* Enable channel */
    DMAC->CHCTRLA.bit.ENABLE = 0b1;
}

void dmac_handler (void)
{
    // Save the currently selected channel in case an interupt happens during
    // channel configuration
    uint8_t old_chan = DMAC->CHID.bit.ID;
    
    // Iterate through all of the channels whith pending interupts by selecting
    // the lowest channel with an interupt from the interupt pending register
    // until there are no channels with pending interupts left.
    while (DMAC->INTPEND.reg & (DMAC_INTPEND_SUSP | DMAC_INTPEND_TCMPL |
                                DMAC_INTPEND_TERR)) {
        // Select the lowest channel with an interupt
        DMAC->CHID.bit.ID = DMAC->INTPEND.bit.ID;
        
        if (DMAC->CHINTFLAG.bit.SUSP) {
            // Clear interupt
            DMAC->CHINTENCLR.bit.SUSP = 0b1;
        }
        
        if (DMAC->CHINTFLAG.bit.TCMPL) {
            if (dmaCircBufferTransfers[DMAC->CHID.bit.ID]->valid) {
                // A circular buffer DMA transfer has finished
                // The head of the buffer must be moved
                dmaCircBufferTransfers[DMAC->CHID.bit.ID]->buffer->head =
                        dmaCircBufferTransfers[DMAC->CHID.bit.ID]->orig_tail;
                // The transaction is done now, so we need to mark it invalid
                dmaCircBufferTransfers[DMAC->CHID.bit.ID]->valid = 0b0;
            }
            
            struct dma_callback_t *c = dma_callbacks + DMAC->CHID.bit.ID;
            
            // Clear interupt
            DMAC->CHINTENCLR.bit.TCMPL = 0b1;
            
            if (c->callback != NULL) {
                c->callback(DMAC->CHID.bit.ID, c->state);
            }
        }
        
        if (DMAC->CHINTFLAG.bit.TERR) {
            // Clear interupt
            DMAC->CHINTENCLR.bit.TERR = 0b1;
        }
    }
    
    // Restore previously selected channel
    DMAC->CHID.bit.ID = old_chan;
}





