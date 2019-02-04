/**
 * @file debug-commmands.c
 * @desc Functions to be run from CLI for debuging purposes.
 * @author Samuel Dewan
 * @date 2019-09-22
 * Last Author: Samuel Dewan
 * Last Edited On: 2019-09-23
 */

#include "debug-commands.h"

#include <stdlib.h>

#include "global.h"


#define DEBUG_VERSION_NAME  "version"
#define DEBUG_VERSION_HELP  "Get software version information."

static void debug_version (uint8_t argc, char **argv,
                           struct console_desc_t *console)
{
    console_send_str(console, VERSION_STRING);
    console_send_str(console, BUILD_STRING);
}



#define DEBUG_DID_NAME  "did"
#define DEBUG_DID_HELP  "Get device identification information."

static void debug_did (uint8_t argc, char **argv,
                       struct console_desc_t *console)
{
    char str[16];
    
    str[0] = '0';
    str[1] = 'x';
    
    console_send_str(console, "Device Identification: ");
    utoa(DSU->DID.reg, str + 2, 16);
    console_send_str(console, str);
    
    console_send_str(console, "\n\nPROCESSOR: ");
    utoa(DSU->DID.bit.PROCESSOR, str + 2, 16);
    console_send_str(console, str);
    
    console_send_str(console, "\nFAMILY: ");
    utoa(DSU->DID.bit.FAMILY, str + 2, 16);
    console_send_str(console, str);
    
    console_send_str(console, "\nSERIES: ");
    utoa(DSU->DID.bit.SERIES, str + 2, 16);
    console_send_str(console, str);
    
    console_send_str(console, "\nDIE: ");
    utoa(DSU->DID.bit.DIE, str + 2, 16);
    console_send_str(console, str);
    
    console_send_str(console, "\nREVISION: ");
    utoa(DSU->DID.bit.REVISION, str + 2, 16);
    console_send_str(console, str);
    str[2] = ' ';
    str[3] = '(';
    str[4] = (char)(DSU->DID.bit.REVISION + 'A');
    str[5] = ')';
    str[6] = 0;
    console_send_str(console, str + 2);
    
    console_send_str(console, "\nDEVSEL: ");
    utoa(DSU->DID.bit.DEVSEL, str + 2, 16);
    console_send_str(console, str);
    
    console_send_str(console, "\n");
}


const uint8_t debug_commands_num_funcs = 2;
const struct cli_func_desc_t debug_commands_funcs[] = {
    {.func = debug_version, .name = DEBUG_VERSION_NAME, .help_string = DEBUG_VERSION_HELP},
    {.func = debug_did, .name = DEBUG_DID_NAME, .help_string = DEBUG_DID_HELP}
};
