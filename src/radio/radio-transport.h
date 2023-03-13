/**
 * @file radio-transport.h
 * @desc Transport layer for radio stack
 * @author Samuel Dewan
 * @date 2020-03-24
 * Last Author:
 * Last Edited On:
 */

#ifndef radio_transport_h
#define radio_transport_h

#include "global.h"
#include "rn2483.h"

#include "radio-types.h"
#include "radio-chanmgr.h"
#include "radio-antmgr.h"


#define RADIO_FIXED_ANTENNA_VAL(ant) (1 & (ant << 1))
#define RADIO_ANTENNA_IS_FIXED(val) (val & 1)
#define RADIO_GET_FIXED_ATENNA_NUM(val) (val >> 1)


/**
 *  Initialize a radio transport.
 *
 *  @param inst Pointer to radio transport descriptor structure for instance
 *  @param radios Array of radio descriptions of available radios
 *  @param radio_uarts Array of pointers to sercom_uart descriptors for uarts
 *                     to which radios are connected
 *  @param radio_antennas Array of descriptions antenna configurations
 *  @param search_role The role that this radio should take when falling back
 *                     into search mode
 *  @param address The device address that should be used when sending packets
 *                 and determining if packets should be parsed
 */
extern void init_radio_transport(struct radio_transport_desc *inst,
                                 struct radio_instance_desc *const *radios,
                                 struct sercom_uart_desc_t *const *radio_uarts,
                                 const struct radio_antenna_info *radio_antennas,
                                 enum radio_search_role search_role,
                                 enum radio_packet_device_address address);

/**
 *  Service function for radio transport to be called in each iteration of the
 *  main loop.
 *
 *  @param inst Radio transport instance
 */
extern void radio_transport_service(struct radio_transport_desc *inst);


/**
 *  Send a block.
 *
 *  @param inst Radio transport instance
 *  @param block The block to be sent
 *  @param block_length Number of bytes to be sent
 *  @param slack_time The amount of time before the block must be sent, zero if
 *                    the block must be sent as soon as possible
 *  @param time_to_live The amount of time before the block is no longer valid
 *                      and should not be sent, zero if the block should be sent
 *                      under any circumstance
 *
 *  @return Zero if the block was queued for transmission, a non-zero value if
 *          the block could not be queued and will not be sent
 */
extern int radio_send_block(struct radio_transport_desc *inst,
                            const uint8_t *block, uint8_t block_length,
                            uint16_t slack_time, uint16_t time_to_live);

/**
 *  Send a block using the priority queue. Blocks queued on the priority queue
 *  will always be sent as soon as possible without waiting to aggregate more
 *  blocks into the packet.
 *
 *  @note The priority queue packet buffer may not be a full 128 bytes long.
 *
 *  @param inst Radio transport instance
 *  @param block The block to be sent
 *  @param block_length Number of bytes to be sent
 *
 *  @return Zero if the block was queued for transmission, a non-zero value if
 *          the block could not be queued and will not be sent
 */
extern int radio_send_block_priority(struct radio_transport_desc *inst,
                                     const uint8_t *block,
                                     uint8_t block_length);

/**
 *  Set a callback function to be called whenever a packet is received.
 *
 *  @param inst Radio transport instance
 *  @param callback The callback function
 */
static inline void radio_set_logging_callback(struct radio_transport_desc *inst,
                                              radio_rx_packet_cb callback)
{
    inst->logging_callback = callback;
}

/**
 *  Set a callback function to be called whenever a valid packet is received.
 *
 *  @param inst Radio transport instance
 *  @param callback The callback function
 */
static inline void radio_set_ground_callback(struct radio_transport_desc *inst,
                                              radio_rx_packet_cb callback)
{
    inst->ground_callback = callback;
}





#endif /* radio_transport_h */
