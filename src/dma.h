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
 *  Transfer all of the data in a circular buffer to a static address.
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
extern int8_t dma_start_circular_buffer_to_static(
                                            struct dma_circ_transfer_t *tran,
                                            uint8_t chan,
                                            struct circular_buffer_t *buffer,
                                            volatile uint8_t *dest,
                                            uint8_t trigger, uint8_t priority);

/**
 *  Transfer a buffer of data to a static address (peripheral).
 *
 *  @param chan The DMA channel to be used.
 *  @param buffer The data to be transferred.
 *  @param length The number of bytes which should be transferred.
 *  @param dest The address of the destination register.
 *  @param trigger The trigger which should be used to control the transfer.
 *  @param priority The priority level of the transfer.
 */
extern void dma_start_buffer_to_static(uint8_t chan, const uint8_t *buffer,
                                       uint16_t length, volatile uint8_t *dest,
                                       uint8_t trigger, uint8_t priority);

/**
 *  Transfer two buffers of data to a static address (peripheral).
 *
 *  @param chan The DMA channel to be used.
 *  @param buffer1 The data to be transferred.
 *  @param length1 The number of bytes which should be transferred.
 *  @param buffer2 The data to be transferred.
 *  @param length2 The number of bytes which should be transferred.
 *  @param descriptor Memory to be used for second DMA descriptor.
 *  @param dest The address of the destination register.
 *  @param trigger The trigger which should be used to control the transfer.
 *  @param priority The priority level of the transfer.
 */
extern void dma_start_double_buffer_to_static(uint8_t chan,
                                              const uint8_t *buffer1,
                                              uint16_t length1,
                                              const uint8_t *buffer2,
                                              uint16_t length2,
                                              DmacDescriptor *descriptor,
                                              volatile uint8_t *dest,
                                              uint8_t trigger,
                                              uint8_t priority);

/**
 *  Transfer data from a static address to a buffer.
 *
 *  @param chan The DMA channel to be used.
 *  @param buffer The buffer where data should be placed.
 *  @param length The number of bytes which should be transferred.
 *  @param source The address of the source register.
 *  @param trigger The trigger which should be used to control the transfer.
 *  @param priority The priority level of the transfer.
 */
extern void dma_start_static_to_buffer(uint8_t chan, uint8_t *buffer,
                                       uint16_t length,
                                       const volatile uint8_t *source,
                                       uint8_t trigger, uint8_t priority);

/**
 *  The same as dma_start_static_to_buffer but with a two byte beat size.
 */
extern void dma_start_static_to_buffer_hword(uint8_t chan, uint16_t *buffer,
                                             uint16_t length,
                                             const volatile uint16_t *source,
                                             uint8_t trigger, uint8_t priority);

/**
 *  Repeatedly transfer a static value to a static address (peripheral).
 *
 *  @param chan The DMA channel to be used.
 *  @param source Pointer to the byte to be transferred.
 *  @param length The number of times which the byte should be transferred.
 *  @param dest The address of the destination register.
 *  @param trigger The trigger which should be used to control the transfer.
 *  @param priority The priority level of the transfer.
 */
extern void dma_start_static_to_static(uint8_t chan,
                                       const volatile uint8_t *source,
                                       uint16_t length, volatile uint8_t *dest,
                                       uint8_t trigger, uint8_t priority);

/**
 *  Cancel an ongoing DMA transaction.
 *
 *  @param chan The DMA for which the transaction should be canceled.
 */
extern void dma_abort_transaction(uint8_t chan);


/**
 *  Check if there is a transfer ongoing on a given channel.
 *
 *  @param chan The DMA channel to be checked.
 *
 *  @return 1 if the channel is active, 0 otherwise.
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
