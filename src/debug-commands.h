/**
 * @file debug-commands.h
 * @desc Functions to be run from CLI for debugging purposes.
 * @author Samuel Dewan
 * @date 2019-09-22
 * Last Author:
 * Last Edited On:
 */


#ifndef debug_commands_h
#define debug_commands_h

#include "cli.h"

extern const uint8_t debug_commands_num_funcs;
extern const struct cli_func_desc_t debug_commands_funcs[];



// MARK: Debug CLI helper functions

extern void debug_print_fixed_point (struct console_desc_t *console,
                                     int32_t value, uint8_t decimal_places);

extern void debug_print_byte_with_pad (struct console_desc_t *console,
                                       const char *line_start, uint8_t byte,
                                       const char *line_end);


#endif /* debug_commands_h */
