/**
 * @file radio-chanmgr.h
 * @desc Radio channel manger, monitors link quality and tracks when radio
 *       settings should be changed.
 * @author Samuel Dewan
 * @date 2020-03-24
 * Last Author:
 * Last Edited On:
 */

#ifndef radio_chanmgr_h
#define radio_chanmgr_h

#include "global.h"

#include "radio-types.h"


/**
 *  Initialize the a channel manager for a radio transport.
 *
 *  @param inst The radio transport instance
 */
extern void init_radio_chanmgr(struct radio_transport_desc *inst);

/**
 *  Service to be called regularly for a radio channel manager instance.
 *
 *  @param inst The radio transport instance
 */
extern void radio_chanmgr_service(struct radio_transport_desc *inst);

/**
 *  Get the best radio to transmit on.
 *
 *  @param inst The radio transport instance
 *
 *  @return RN2483 driver descriptor for best transmit radio
 */
extern struct radio_instance_desc *radio_chanmgr_get_tx_radio(
                                            struct radio_transport_desc *inst);

/**
 *  Function called whenever a packet is received.
 *
 *  @param transport The radio transport instance
 *  @param radio The radio instance on which the packet was received
 *  @param antenna_num Number of the antenna on which the packet was received
 *  @param snr Signal to noise ratio of received packet
 *  @param rssi Received signal strength for received packet
 */
extern void radio_chanmgr_metadata_cb(struct radio_transport_desc *transport,
                                      struct radio_instance_desc *radio,
                                      uint8_t antenna_num, int8_t snr,
                                      int8_t rssi);

/**
 *  Callback for when an RX power loss estimate is available from a received
 *  signal report block.
 *
 *  @param transport The radio transport instance
 *  @param radio The radio instance on which the signal report was received
 *  @param antenna_num Number of the antenna on which the signal report was
 *                     received
 *  @param rx_loss The signal loss for the packet containing the signal report
 *                 request
 */
extern void radio_chanmgr_rx_loss_cb(struct radio_transport_desc *transport,
                                     struct radio_instance_desc *radio,
                                     uint8_t antenna_num, int8_t rx_loss);

/**
 *  Callback for when an TX power loss estimate is available from a received
 *  signal report response block.
 *
 *  @param transport The radio transport instance
 *  @param tx_radio The radio instance on which the signal report request was
 *                  sent
 *  @param antenna_num Number of the antenna on which the signal report request
 *                     was sent
 *  @param tx_loss The signal loss for the request that prompted this response
 *  @param remote_snr Signal to noise ratio reported by sender of signal report
 */
extern void radio_chanmgr_tx_loss_cb(struct radio_transport_desc *transport,
                                     struct radio_instance_desc *tx_radio,
                                     uint8_t antenna_num, int8_t tx_loss,
                                     int8_t remote_snr);


#endif /* radio_chanmgr_h */
