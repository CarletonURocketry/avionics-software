/**
 * @file debug-commmands.c
 * @desc Functions to be run from CLI for debuging purposes.
 * @author Samuel Dewan
 * @date 2019-09-22
 * Last Author: Samuel Dewan
 * Last Edited On: 2019-09-23
 */

#include "debug-commands.h"


#define DEBUG_VERSION_NAME  "version"
#define DEBUG_VERSION_HELP  "Get software version information."
static void debug_version (uint8_t argc, char **argv,
                           struct console_desc_t *console)
{
    return;
}


const uint8_t debug_commands_num_funcs = 1;
const struct cli_func_desc_t debug_commands_funcs[] = {
    {.func = debug_version, .name = DEBUG_VERSION_NAME, .help_string = DEBUG_VERSION_HELP}
};
