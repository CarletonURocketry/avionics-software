/**
 * @file cli.h
 * @desc Provides a CLI on a console for debugging purposes.
 * @author Samuel Dewan
 * @date 2019-09-21
 * Last Author:
 * Last Edited On:
 */

#ifndef cli_h
#define cli_h

#include "global.h"
#include "console.h"

/**
 *  Descriptor for a function which can be called via a CLI.
 */
struct cli_func_desc_t {
    void (*func)(uint8_t, char**, struct console_desc_t*);
    const char *name;
    const char *help_string;
};

/**
 *  Descriptor for a CLI instance.
 */
struct cli_desc_t {
    const char *prompt;
    const struct cli_func_desc_t *functions;
};

/**
 *  Initialize a command line interface on a console.
 *
 *  @param cli The CLI to be initialized
 *  @param console The console on which the CLI should operate.
 *  @param functions An array of function descriptors which can be used via this
 *                   CLI
 */
extern void init_cli (struct cli_desc_t *cli,  struct console_desc_t *console,
                      const char *prompt,
                      const struct cli_func_desc_t *const functions);

#endif /* cli_h */
