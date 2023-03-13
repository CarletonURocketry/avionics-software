/**
 * @file radio-antmgr.h
 * @desc Radio antenna manger, monitors link quality and selects best antenna.
 * @author Samuel Dewan
 * @date 2020-03-24
 * Last Author:
 * Last Edited On:
 */

#ifndef radio_antmgr_h
#define radio_antmgr_h

#include "global.h"
#include "sky13414.h"
#include "radio-types.h"

#define ANTMGR_ANT_1_MASK   (1 << 0)
#define ANTMGR_ANT_2_MASK   (1 << 1)
#define ANTMGR_ANT_3_MASK   (1 << 2)
#define ANTMGR_ANT_4_MASK   (1 << 3)


extern void init_radio_antmgr(struct radio_instance_desc *inst,
                              const struct radio_antenna_info *info);

extern void radio_antmgr_service(struct radio_instance_desc *inst);

extern void radio_antmgr_set_fixed(struct radio_instance_desc *inst,
                                   const struct radio_antenna_info *info);

extern uint8_t radio_antmgr_get_current_antenna(
                                        const struct radio_instance_desc *inst);

extern void radio_antmgr_metadata_cb(struct radio_transport_desc *transport,
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
extern void radio_antmgr_rx_loss_cb(struct radio_transport_desc *transport,
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
extern void radio_antmgr_tx_loss_cb(struct radio_transport_desc *transport,
                                    struct radio_instance_desc *tx_radio,
                                    uint8_t antenna_num, int8_t tx_loss,
                                    int8_t remote_snr);

#endif /* radio_antmgr_h */
