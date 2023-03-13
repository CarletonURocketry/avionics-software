/**
 * @file debug-commmands-general.h
 * @desc Commands for debugging CLI
 * @author Samuel Dewan
 * @date 2020-08-11
 * Last Author:
 * Last Edited On:
 */

#ifndef debug_commands_general_h
#define debug_commands_general_h

#include <stdint.h>
#include "cli.h"



#define DEBUG_VERSION_NAME  "version"
#define DEBUG_VERSION_HELP  "Get software version information."

extern void debug_version (uint8_t argc, char **argv,
                           struct console_desc_t *console);


#define DEBUG_DID_NAME  "did"
#define DEBUG_DID_HELP  "Get device identification information."

extern void debug_did (uint8_t argc, char **argv,
                       struct console_desc_t *console);


#define DEBUG_RCAUSE_NAME   "rcause"
#define DEBUG_RCAUSE_HELP   "Get reset cause."

extern void debug_rcause (uint8_t argc, char **argv,
                          struct console_desc_t *console);


#define DEBUG_I2C_SCAN_NAME  "i2c-scan"
#define DEBUG_I2C_SCAN_HELP  "Scan for devices on the I2C bus."

extern void debug_i2c_scan (uint8_t argc, char **argv,
                            struct console_desc_t *console);


#define DEBUG_IO_EXP_REGS_NAME  "io-exp-regs"
#define DEBUG_IO_EXP_REGS_HELP  "Read MCP23S17 registers.\n"\
                                "Usage: io-exp-regs [address]"

extern void debug_io_exp_regs (uint8_t argc, char **argv,
                               struct console_desc_t *console);


#define DEBUG_GPIO_NAME "gpio"
#define DEBUG_GPIO_HELP "Control gpio pins."\
                        "\nUsage: gpio mode <pin> <input/output/strong/pull>"\
                        "\n       gpio pull <pin> <high/low/none>"\
                        "\n       gpio out <pin> <high/low>"\
                        "\n       gpio status <pin>"\
                        "\n       gpio in <pin>"\
                        "\n       gpio poll <pin>"\
                        "\n<pin> = g<0 to 23>           : header gpio pin"\
                        "\n<pin> = p<a/b><0 to 31>      : internal pin"\
                        "\n<pin> = e<a/b><0 to 7>       : IO expander pin"\
                        "\n<pin> = r<radio #>.<0 to 17> : RN2483 pin"

extern void debug_gpio (uint8_t argc, char **argv,
                        struct console_desc_t *console);

#endif /* debug_commands_general_h */
