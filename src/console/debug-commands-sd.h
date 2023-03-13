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


#define DEBUG_SDSPI_NAME    "sdspi"
#define DEBUG_SDSPI_HELP    "Get information about or interact with SD card "\
                            "connected with SPI."

extern void debug_sdspi (uint8_t argc, char **argv,
                         struct console_desc_t *console);


#define DEBUG_SDHC_NAME "sdhc"
#define DEBUG_SDHC_HELP "Get information about or interact with SD card "\
                        "connected through the SD Host Controller."
extern void debug_sdhc (uint8_t argc, char **argv,
                        struct console_desc_t *console);


#define DEBUG_MBR_NAME  "mbr"
#define DEBUG_MBR_HELP  "Read or create Master Boot Record on SD card."
extern void debug_mbr (uint8_t argc, char **argv,
                       struct console_desc_t *console);


#define DEBUG_FORMAT_NAME  "format"
#define DEBUG_FORMAT_HELP  "Format a CU InSpace partition on SD card.\nUsage: "\
                           "format <partition number>"
extern void debug_format (uint8_t argc, char **argv,
                          struct console_desc_t *console);


#define DEBUG_LOGGING_NAME  "logging"
#define DEBUG_LOGGING_HELP  "Control logging service.\nUsage: "\
                            "logging [info/pause/resume]"
extern void debug_logging (uint8_t argc, char **argv,
                           struct console_desc_t *console);

#endif /* debug_commands_sd_h */
