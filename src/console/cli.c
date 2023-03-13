/**
 * @file cli.c
 * @desc Provides a CLI on a console for debugging purposes.
 * @author Samuel Dewan
 * @date 2019-01-21
 * Last Author: Samuel Dewan
 * Last Edited On: 2019-08-25
 */

#include "cli.h"

#include <string.h>

/** String to clear the screen of a VT100 and bring the cursor home. */
static const char *cli_clear_str = "\x1B[2J\x1B[H";

static const char *cli_unknown_str_0 = "Unknown command \"";
static const char *cli_unknown_str_1 = "\"\n";


static void cli_help (uint8_t argc, char **argv, struct console_desc_t *console,
                      struct cli_desc_t *cli)
{
    if (argc > 2) {
        console_send_str(console, "Use \"help\" to list all commands or "
                                  "\"help <command>\" to get information on a "
                                  "specific command.\n");
        return;
    } else if (argc == 2) {
        // Get help for a specific command
        for (const struct cli_func_desc_t *cmd = cli->functions;
                    cmd->func != NULL; cmd++) {
            if (!strcasecmp(argv[1], cmd->name)) {
                console_send_str(console, cmd->help_string);
                console_send_str(console, "\n");
                return;
            }
        }

        console_send_str(console, cli_unknown_str_0);
        console_send_str(console, argv[1]);
        console_send_str(console, cli_unknown_str_1);
    } else {
        console_send_str(console, "Use \"help <command>\" to get information "
                                  "on a specific command.\n");
    }
    
    console_send_str(console, "\nAvailable Commands:\n");
    for (const struct cli_func_desc_t *cmd = cli->functions;
                cmd->func != NULL; cmd++) {
        console_send_str(console, cmd->name);
        console_send_str(console, "\n");
    }
}

static void cli_line_callback (char *line, struct console_desc_t *console,
                               void *context)
{
    struct cli_desc_t *cli = (struct cli_desc_t*)context;
    
    /* Count the number of space separated tokens in the line */
    uint8_t num_args = 1;
    for (uint16_t i = 0; i < strlen(line); i++) {
        num_args += (line[i] == ' ');
    }
    
    /* Create a an array of pointers to each token */
    // Array is one larger to leave room for NULL pointer
    char *args[num_args + 1];
    for (uint16_t i = 0; (args[i] = strsep(&line, " ")) != NULL; i++);
   
    /* Call the specified function */
    uint8_t command_found = 0;
    if (args[0] == NULL) {
        // Empty line
        command_found = 1;
    } else if (!strcasecmp(args[0], "help")) {
        // Print the help string for a function
        cli_help(num_args, args, console, cli);
        command_found = 1;
    } else if (!strcasecmp(args[0], "clear")) {
        // Clear the screen
        console_send_str(console, cli_clear_str);
        command_found = 1;
    } else {
        // Search function list
        for (const struct cli_func_desc_t *cmd = cli->functions;
                    cmd->func != NULL; cmd++) {
            if (!strcasecmp(args[0], cmd->name)) {
                cmd->func(num_args, args, console);

                command_found = 1;
                break;
            }
        }
    }
    
    /* If the command was not found and is not empty, print an angry message. */
    if (!command_found) {
        console_send_str(console, cli_unknown_str_0);
        console_send_str(console, args[0]);
        console_send_str(console, cli_unknown_str_1);
    }
    
    /* Print the prompt. */
    console_send_str(console, cli->prompt);
    return;
}

static void cli_init_callback (struct console_desc_t *console, void *context)
{
    struct cli_desc_t *cli = (struct cli_desc_t*)context;
    
    console_send_str(console, cli_clear_str);
    console_send_str(console, cli->prompt);
}

void init_cli (struct cli_desc_t *cli, struct console_desc_t *console,
               const char *prompt,
               const struct cli_func_desc_t *const functions)
{
    cli->prompt = prompt;
    cli->functions = functions;
    
    console_set_line_callback(console, cli_line_callback, (void*)cli);
    console_set_init_callback(console, cli_init_callback, (void*)cli);
}
