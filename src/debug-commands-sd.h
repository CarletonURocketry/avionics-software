/**
 * @file debug-commands-sd.h
 * @desc Commands for debugging SD card
 * @author Samuel Dewan
 * @date 2021-06-01
 * Last Author:
 * Last Edited On:
 */

#ifndef debug_commands_sd_h
#define debug_commands_sd_h

#include <stdint.h>
#include "cli.h"


#define DEBUG_SDSPI_NAME  "sdspi"
#define DEBUG_SDSPI_HELP  "Get information about or interact with SD card "\
                          "connected with SPI."

extern void debug_sdspi (uint8_t argc, char **argv,
                         struct console_desc_t *console);

#endif /* debug_commands_sd_h */
