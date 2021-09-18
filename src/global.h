/**
 * @file global.h
 * @desc Variables and constants which are used globally
 * @author Samuel Dewan
 * @date 2018-12-27
 * Last Author:
 * Last Edited On:
 */

#ifndef global_h
#define global_h

// Include target.h for millis and MCU specific headers
#include "target.h"

#include <stdint.h>
#include <stddef.h> // NULL


#define VERSION_STRING "CU InSpace 2021 Avionics Software\n"
#define BUILD_STRING "Built "__DATE__" at "__TIME__" with arm-none-eabi-gcc "\
                     __VERSION__"\n"

// Initalization functions
extern void init_target(void);
extern void init_board(void);
extern void init_variant(void);

// Service functions
extern void board_service(void);
extern void variant_service(void);

// Clock control functions (defined in target.c)
extern void enable_bus_clock(enum peripheral_bus_clock clock);
extern void disable_bus_clock(enum peripheral_bus_clock clock);
extern void set_perph_generic_clock(enum peripheral_generic_clock channel,
                                    uint32_t clock_mask);

/**
 *  Variable which can be incremented by any thread to prevent the main loop
 *  from entering sleep.
 */
extern volatile uint8_t inhibit_sleep_g;

/**
 *  Prevents the main loop from entering sleep
 */
static inline void inhibit_sleep (void)
{
    __disable_irq();
    inhibit_sleep_g++;
    __enable_irq();
}

/**
 *  Allows the main loop to enter sleep
 */
static inline void allow_sleep (void)
{
    __disable_irq();
    inhibit_sleep_g--;
    __enable_irq();
}

#endif /* global_h */
