/**
 * @file global.h
 * @desc Variables and constants which are used globaly
 * @author Samuel Dewan
 * @date 2018-12-27
 * Last Author:
 * Last Edited On:
 */

#ifndef global_h
#define global_h

#define DONT_USE_CMSIS_INIT
#include "samd21/include/samd21j18a.h"

#include <stddef.h> // NULL


#define DEBUG_LED_GROUP_NUM 1
#define DEBUG_LED_MASK       PORT_PB30  // Xplained Pro
/* #define DEBUG_LED_MASK       PORT_PB15  // CU InSpace Board */

/**
 *  Number of milliseconds ellapsed since system was reset
 */
extern volatile uint32_t millis;




// Disable/enable IRQ functions which allow restoration of IRQ state as sugested
// by Stackoverflow user "Notlikethat".
// http://stackoverflow.com/questions/27998059/atomic-block-for-reading-vs-arm-systicks

static inline int disable_irq(void) {
    int primask;
    asm volatile("mrs %0, PRIMASK\n"
                 "cpsid i\n" : "=r"(primask));
    return primask & 1;
}

static inline void enable_irq(int primask) {
    if (primask) {
        asm volatile("cpsie i\n");
    }
}


#endif /* global_h */
