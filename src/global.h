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

#define DONT_USE_CMSIS_INIT
#include "samd21/include/samd21j18a.h"
#include "samd21/include/compiler.h"

#include <stddef.h> // NULL


#define VERSION_STRING "CU InSpace 2019 Avionics Software\n"
#define BUILD_STRING "Built "__DATE__" at "__TIME__" with arm-none-eabi-gcc "__VERSION__"\n"

/**
 *  Number of milliseconds elapsed since system was reset
 */
extern volatile uint32_t millis;

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
    inhibit_sleep_g++;
}

/**
 *  Allows the main loop to enter sleep
 */
static inline void allow_sleep (void)
{
    inhibit_sleep_g--;
}

#endif /* global_h */
