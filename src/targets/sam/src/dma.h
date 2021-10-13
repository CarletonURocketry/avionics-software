/**
 * @file dma.h
 * @desc Abstraction layer for configuring DMAC
 * @author Samuel Dewan
 * @date 2018-12-30
 * Last Author: Samuel Dewan
 * Last Edited On: 2019-04-27
 */

#ifndef dma_h
#define dma_h

#include "global.h"
#include "circular-buffer.h"

/**
 *  Callbacks for when a DMA channel is finished.
 */
extern struct dma_callback_t {
    void (*callback)(uint8_t, void*);
    void *state;
} dma_callbacks[DMAC_CH_NUM];


/**
 *  Description of a DMA transfer from a circular buffer
 */
struct dma_circ_transfer_t {
    DmacDescriptor second_descriptor;
    struct circular_buffer_t *buffer;
    uint16_t length;
    uint8_t valid:1;
};


/**
 *  Initializes the DMAC to enable DMA transfers and CRC calculations.
 */
extern void init_dmac(void);

/**
 *  DMA transfer beat size.
 */
enum dma_width {
    DMA_WIDTH_BYTE = DMAC_BTCTRL_BEATSIZE_BYTE_Val,
    DMA_WIDTH_HALF_WORD = DMAC_BTCTRL_BEATSIZE_HWORD_Val,
    DMA_WIDTH_WORD = DMAC_BTCTRL_BEATSIZE_WORD_Val
};

/**
 *  Configure a DMA descriptor. Can be used to build a transfer with multiple
 *  descriptors.
 *
 *  @note The source and destination addresses must be aligned to the beat size.
 *
 *  @param desc The DMA descriptor to be configured
 *  @param beatsize The number of bytes to transfer in each beat of this
 *                  transfer
 *  @param source The address from which data should be copied
 *  @param increment_source Whether the source address should be incremented
 *  @param destination The address to which data should be copied
 *  @param increment_destination Whether the destination address should be
 *                               incremented
 *  @param length The number of beats which should be transferred
 *  @param next DmacDescriptor to chain onto end of transaction
 */
extern void dma_config_desc(DmacDescriptor *desc, enum dma_width beatsize,
                            const volatile void *source, int increment_source,
                            volatile void *destination,
                            int increment_destination, uint16_t length,
                            DmacDescriptor *next);

/**
 *  Configure a DMA transfer and enable the channel.
 *
 *  @note The source and destination addresses must be aligned to the beat size.
 *
 *  @param chan The DMA channel to be used.
 *  @param beatsize The number of bytes to transfer in each beat of this
 *                  transfer
 *  @param source The address from which data should be copied
 *  @param increment_source Whether the source address should be incremented
 *  @param destination The address to which data should be copied
 *  @param increment_destination Whether the destination address should be
 *                               incremented
 *  @param length The number of beats which should be transferred
 *  @param trigger The trigger which should be used to control the transfer
 *  @param priority The priority level of the transfer
 *  @param next DmacDescriptor to chain onto end of transfer
 */
extern void dma_config_transfer(uint8_t chan, enum dma_width beatsize,
                                const volatile void *source,
                                int increment_source,
                                volatile void *destination,
                                int increment_destination, uint16_t length,
                                uint8_t trigger, uint8_t priority,
                                DmacDescriptor *next);

/**
 *  Transfer all of the data in a circular buffer to a static address. Uses a
 *  one byte block size.
 *
 *  @param tran A circular buffer transfer descriptor which provides memory for
 *              the second DMA transfer descriptor if necessary and holds state.
 *  @param chan The DMA channel to be used.
 *  @param buffer The circular buffer from which data should be read.
 *  @param dest The address of the destination register.
 *  @param trigger The trigger which should be used to control the transfer.
 *  @param priority The priority level of the transfer.
 *
 *  @return 0 on success.
 */
extern int8_t dma_config_circular_buffer_to_static(
                                            struct dma_circ_transfer_t *tran,
                                            uint8_t chan,
                                            struct circular_buffer_t *buffer,
                                            volatile uint8_t *dest,
                                            uint8_t trigger, uint8_t priority);

/**
 *  Cancel an ongoing DMA transaction.
 *
 *  @param chan The DMA for which the transaction should be canceled.
 */
extern void dma_abort_transfer(uint8_t chan);


/**
 *  Check if there is a transfer ongoing on a given channel.
 *
 *  @param chan The DMA channel to be checked.
 *
 *  @return 1 if the channel is active, 0 otherwise.
 */
inline static uint8_t dma_chan_is_active(uint8_t chan)
{
#if defined(SAMD2x)
    DMAC->CHID.bit.ID = chan;
    return DMAC->CHINTENSET.bit.TCMPL ||
                    (DMAC->ACTIVE.bit.ID == chan && DMAC->ACTIVE.bit.ABUSY);
#elif defined(SAMx5x)
    return DMAC->Channel[chan].CHINTENSET.bit.TCMPL ||
                    (DMAC->ACTIVE.bit.ID == chan && DMAC->ACTIVE.bit.ABUSY);
#endif
}




extern uint16_t crc_calc_crc16(void);

extern void crc_calc_crc32(void);


#endif /* dma_h */
