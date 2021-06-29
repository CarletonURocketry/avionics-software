/**
 * @file main.c
 * @author Samuel Dewan
 * @date 2020-09-04
 * Last Author:
 * Last Edited On:
 */

#include "global.h"

#include "target.h"
#include "board.h"
#include "variant.h"

// MARK: Function prototypes
static void main_loop(void);

// MARK: Variable Definitions
volatile uint8_t inhibit_sleep_g;

// MARK: Function Definitions
int main(void)
{
    init_target();
    init_board();
    init_variant();

    // Main Loop
    for (;;) {
        // Run the main loop
        main_loop();

        // Sleep if sleep is not inhibited
        if (!inhibit_sleep_g) {
            __WFI();
        }
    }

    return 0; // never reached
}

static void main_loop (void)
{
    board_service();
    variant_service();
}
