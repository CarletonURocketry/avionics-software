/**
 * @file debug-commands-radio.h
 * @desc Commands for debugging CLI
 * @author Samuel Dewan
 * @date 2020-08-11
 * Last Author:
 * Last Edited On:
 */

#ifndef debug_commands_radio_h
#define debug_commands_radio_h

#include <stdint.h>
#include "cli.h"


#define DEBUG_LORA_VERSION_NAME  "lora-version"
#define DEBUG_LORA_VERSION_HELP  "Get version string from LoRa radio module"

extern void debug_lora_version (uint8_t argc, char **argv,
                                struct console_desc_t *console);


#define DEBUG_RADIO_INFO_NAME   "radio-info"
#define DEBUG_RADIO_INFO_HELP   "Get information about attached radios"

extern void debug_radio_info (uint8_t argc, char **argv,
                              struct console_desc_t *console);


#define DEBUG_RADIO_RX_NAME "radio-rx"
#define DEBUG_RADIO_RX_HELP "Print received packets from radio transport"

extern void debug_radio_rx (uint8_t argc, char **argv,
                            struct console_desc_t *console);


#define DEBUG_RADIO_TX_NAME "radio-tx"
#define DEBUG_RADIO_TX_HELP "Send a debug message block"

extern void debug_radio_tx (uint8_t argc, char **argv,
                            struct console_desc_t *console);

#endif /* debug_commands_radio_h */
