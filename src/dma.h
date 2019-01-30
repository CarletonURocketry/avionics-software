/**
 * @file dma.h
 * @desc Abstraction layer for configuring DMAC
 * @author Samuel Dewan
 * @date 2018-12-30
 * Last Author:
 * Last Edited On:
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
    uint16_t orig_tail;
    uint8_t valid:1;
};


/**
 *  Initilizes the DMAC to enable DMA transfers and CRC calculations.
 */
extern void init_dmac(void);

/**
 *  Transfer all of the data in a circular buffer to a static address.
 *
 *  @param tran A circual buffer transfer descriptor which provides memeory for
 *              the second DMA transfer descriptor if nessesary and holds state.
 *  @param chan The DMA channel to be used.
 *  @param buffer The circular buffer from which data should be read.
 *  @param dest The address of the destination register.
 *  @param trigger The trigger which should be used to control the transfer.
 *  @param priority The priority level of the transfer.
 *
 *  @return 0 on success.
 */
extern int8_t dma_start_circular_buffer_to_static(
                                            struct dma_circ_transfer_t *tran,
                                            uint8_t chan,
                                            struct circular_buffer_t *buffer,
                                            uint8_t *dest, uint8_t trigger,
                                            uint8_t priority);

/**
 *  Transfer a buffer of data to a static address (peripheral).
 *
 *  @param chan The DMA channel to be used.
 *  @param buffer The data to be transfered.
 *  @param length The number of bytes which should be transfered.
 *  @param dest The address of the destination register.
 *  @param trigger The trigger which should be used to control the transfer.
 *  @param priority The priority level of the transfer.
 */
extern void dma_start_buffer_to_static(uint8_t chan, const uint8_t *buffer,
                                       uint16_t length, uint8_t *dest,
                                       uint8_t trigger, uint8_t priority);

/**
 *  Transfer data from a static address to a buffer.
 *
 *  @param chan The DMA channel to be used.
 *  @param buffer The buffer where data should be placed.
 *  @param length The number of bytes which should be transfered.
 *  @param source The address of the source register.
 *  @param trigger The trigger which should be used to control the transfer.
 *  @param priority The priority level of the transfer.
 */
extern void dma_start_static_to_buffer(uint8_t chan, uint8_t *buffer,
                                       uint16_t length, const uint8_t *source,
                                       uint8_t trigger, uint8_t priority);

/**
 *  Repeatedly transfer a static value to a static address (peripheral).
 *
 *  @param chan The DMA channel to be used.
 *  @param source Pointer to the byte to be transfered.
 *  @param length The number of times which the byte should be transfered.
 *  @param dest The address of the destination register.
 *  @param trigger The trigger which should be used to control the transfer.
 *  @param priority The priority level of the transfer.
 */
extern void dma_start_static_to_static(uint8_t chan, const uint8_t *source,
                                       uint16_t length, uint8_t *dest,
                                       uint8_t trigger, uint8_t priority);


/**
 *  Check if there is a transfer ongoing on a given channel.
 *
 *  @param chan The DMA channel to be checked.
 *
 *  @return 1 if the chanel is active, 0 otherwise.
 */
inline static uint8_t dma_chan_is_active(uint8_t chan)
{
    DMAC->CHID.bit.ID = chan;
    return DMAC->CHINTENSET.bit.TCMPL ||
           (DMAC->ACTIVE.bit.ID == chan && DMAC->ACTIVE.bit.ABUSY);
}




extern uint16_t crc_calc_crc16_sync(void);

extern void crc_calc_crc16_async(void);

extern void crc_calc_crc16_dma(void);

extern uint32_t crc_calc_crc32_sync(void);

extern void crc_calc_crc32_async(void);

extern void crc_calc_crc32_dma(void);

extern uint32_t crc_get_async_result_32(void);

static inline uint16_t crc_get_async_result_16(void)
{
    return (uint16_t)crc_get_async_result_32();
}

extern void crc_service(void);

#endif /* dma_h */
