/**
 * @file debug-commands-analog.h
 * @desc Commands for debugging CLI
 * @author Samuel Dewan
 * @date 2020-08-11
 * Last Author:
 * Last Edited On:
 */

#ifndef debug_commands_analog_h
#define debug_commands_analog_h

#include <stdint.h>
#include "cli.h"


#define DEBUG_TEMP_NAME "temp"
#define DEBUG_TEMP_HELP "Read internal temperature sensor and the NVM "\
                        "temperature log row."

extern void debug_temp (uint8_t argc, char **argv,
                        struct console_desc_t *console);


#define DEBUG_ANALOG_NAME   "analog"
#define DEBUG_ANALOG_HELP   "Print values of analog inputs.\nUsage: analog "\
                            "[pin numbering]\nPin numbering should be one of "\
                            "internal or header."

extern void debug_analog (uint8_t argc, char **argv,
                          struct console_desc_t *console);


#define DEBUG_ADC_INIT_NAME "adc-init"
#define DEBUG_ADC_INIT_HELP "Initialize ADC"

extern void debug_adc_init (uint8_t argc, char **argv,
                            struct console_desc_t *console);


#define DEBUG_ADC_READ_NAME "adc-read"
#define DEBUG_ADC_READ_HELP "Read ADC\n"\
                            "Usage: adc-read <scan start> [scan end]"

extern void debug_adc_read (uint8_t argc, char **argv,
                            struct console_desc_t *console);


#define DEBUG_DAC_NAME  "dac"
#define DEBUG_DAC_HELP  "Control DAC.\nUsage: dac [value] [volts/raw]"

extern void debug_dac (uint8_t argc, char **argv,
                       struct console_desc_t *console);


#endif /* debug_commands_analog_h */
