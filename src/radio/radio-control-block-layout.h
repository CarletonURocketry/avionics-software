/**
 * @file radio-control-block-layout.h
 * @desc Helpers for parsing control blocks
 * @author Samuel Dewan
 * @date 2020-08-02
 * Last Author:
 * Last Edited On:
 */

#ifndef radio_control_block_layout_h
#define radio_control_block_layout_h

#include "radio-packet-layout.h"

// MARK: Signal Report

#define RADIO_BLOCK_SIG_REPORT_LENGHT   (RADIO_BLOCK_HEADER_LENGTH + 4)

/**
 *  Get the signal to noise ratio from a signal report block.
 *
 *  @param block Pointer to the signal report block to be parsed
 *
 *  @return The SNR from the signal report block
 */
__attribute__((const))
static inline int8_t radio_block_sig_report_snr(const uint8_t *block)
{
    return block[RADIO_BLOCK_HEADER_LENGTH + 0];
}

/**
 *  Get the received signal strength indicator from a signal report block.
 *
 *  @param block Pointer to the signal report block to be parsed
 *
 *  @return The RSSI from the signal report block
 */
__attribute__((const))
static inline int8_t radio_block_sig_report_rssi(const uint8_t *block)
{
    return block[RADIO_BLOCK_HEADER_LENGTH + 1];
}

/**
 *  Get the radio from a transmit report block.
 *
 *  @param block Pointer to the signal report block to be parsed
 *
 *  @return The radio from the signal report block
 */
__attribute__((const))
static inline uint8_t radio_block_sig_report_radio(const uint8_t *block)
{
    return block[RADIO_BLOCK_HEADER_LENGTH + 2] & 0x3;
}

/**
 *  Get the transmit power from a signal report block.
 *
 *  @param block Pointer to the signal report block to be parsed
 *
 *  @return The transmit power from the signal report block
 */
__attribute__((const))
static inline int8_t radio_block_sig_report_tx_power(const uint8_t *block)
{
    return ((int8_t)block[RADIO_BLOCK_HEADER_LENGTH + 2]) >> 2;
}

/**
 *  Get whether a signal report block contains a request for a reply.
 *
 *  @param block Pointer to the signal report block to be parsed
 *
 *  @return A non-zero value if a reply is requested, zero otherwise
 */
__attribute__((const))
static inline int radio_block_sig_report_req(const uint8_t *block)
{
    return block[RADIO_BLOCK_HEADER_LENGTH + 3] & 0x80;
}

/**
 *  Marshal the payload of a signal report block.
 *
 *  @param block Pointer to the block for which the payload should be created
 *  @param snr The SNR value to place in the block payload
 *  @param rssi The RSSI value to place in the block payload
 *  @param radio The radio number for the radio that the request was sent on
 *  @param tx_power The tx_power value to place in the block payload
 *  @param request Whether the block should include a request for a reply
 */
static inline void radio_block_marshal_sig_report(uint8_t *block, int8_t snr,
                                                  int8_t rssi, uint8_t radio,
                                                  int8_t tx_power, int request)
{
    block[RADIO_BLOCK_HEADER_LENGTH + 0] = snr;
    block[RADIO_BLOCK_HEADER_LENGTH + 1] = rssi;
    block[RADIO_BLOCK_HEADER_LENGTH + 2] = ((uint8_t)tx_power) << 2;
    block[RADIO_BLOCK_HEADER_LENGTH + 2] |= radio & 0x3;
    block[RADIO_BLOCK_HEADER_LENGTH + 3] = request ? 0x80 : 0;
}

/**
 *  Set the tx radio number in a signal report block.
 *
 *  @param block The signal report block in which the radio number should be set
 *  @param tx_radio_num The new radio number
 */
static inline void radio_block_sig_report_set_tx_radio(uint8_t *block,
                                                       uint8_t tx_radio_num)
{
    block[RADIO_BLOCK_HEADER_LENGTH + 2] &= ~0x3;
    block[RADIO_BLOCK_HEADER_LENGTH + 2] |= tx_radio_num & 0x3;
}

#endif /* radio_control_block_layout_h */
