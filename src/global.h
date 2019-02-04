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
#include "samd21/include/compiler.h"

#include <stddef.h> // NULL


#define VERSION_STRING "CU InSpace 2019 Avionics Software\n"
#define BUILD_STRING "Built "__DATE__" at "__TIME__" with arm-none-eabi-gcc "__VERSION__"\n"


#define DEBUG_LED_GROUP_NUM 1
//#define DEBUG_LED_MASK       PORT_PB30  // Xplained Pro
#define DEBUG_LED_MASK       PORT_PB15  // CU InSpace Board

/**
 *  Number of milliseconds ellapsed since system was reset
 */
extern volatile uint32_t millis;


#endif /* global_h */
