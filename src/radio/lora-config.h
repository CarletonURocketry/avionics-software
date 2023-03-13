/**
 * @file lora-config.h
 * @desc Configuration for LoRa radio
 * @author Samuel Dewan
 * @date 2020-03-25
 * Last Author:
 * Last Edited On:
 */

#ifndef lora_config_h
#define lora_config_h

#include "src/radio/rn2483.h"

//
//
//  Misc.
//
//

#define RADIO_SIG_REPORT_PERIOD MS_TO_MILLIS(5000)

//
//
// Callsign
//
//

// This placeholder value needs to be replaced before use!!
#define LORA_CALLSIGN   "AAAAAA"

//
//
// Antenna switch settings
//
//

#define ANTENNA_SWITCH_V1   RN2483_GPIO0
#define ANTENNA_SWITCH_V2   RN2483_GPIO1
#define ANTENNA_SWITCH_V3   RN2483_GPIO2

//
//
//  Default LoRa Settings
//
//

//  Centre frequency in hertz, from 433050000 to 434790000
#define LORA_FREQ 433050000
/*  Power level in dBm, from -3 to 14 */
#define LORA_POWER  14
/*  LoRa spreading factor */
#define LORA_SPREADING_FACTOR RN2483_SF_SF9
/*  LoRa coding rate */
#define LORA_CODING_RATE RN2483_CR_4_7
/*  Bandwidth */
#define LORA_BANDWIDTH RN2483_BW_500
/*  Preamble Length, from 0 to 65535 */
#define LORA_PRLEN 6
/*  Whether a CRC should be added to the data */
#define LORA_CRC 1
/*  Whether the I and Q streams should be inverted */
#define LORA_INVERT_IQ 0
/*  Sync word */
#define LORA_SYNC_WORD 0x43


//
//
//  Search Mode LoRa Settings
//
//

// Search frequencies
#define LORA_SEARCH_FREQ_0  433050000
#define LORA_SEARCH_FREQ_1  433267500
#define LORA_SEARCH_FREQ_2  433485000
#define LORA_SEARCH_FREQ_3  433702500
#define LORA_SEARCH_FREQ_4  433920000
#define LORA_SEARCH_FREQ_5  434137500
#define LORA_SEARCH_FREQ_6  434355000
#define LORA_SEARCH_FREQ_7  434572500

/*  Power level in dBm, from -3 to 14 */
#define LORA_SEARCH_POWER  14
/*  LoRa spreading factor */
#define LORA_SEARCH_SPREADING_FACTOR RN2483_SF_SF9
/*  LoRa coding rate */
#define LORA_SEARCH_CODING_RATE RN2483_CR_4_8
/*  Bandwidth */
#define LORA_SEARCH_BANDWIDTH RN2483_BW_125
/*  Preamble Length, from 0 to 65535 */
#define LORA_SEARCH_PRLEN 6


#endif /* lora_config_h */
