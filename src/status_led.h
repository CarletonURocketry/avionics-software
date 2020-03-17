/**
 * @file status_led.h
 * @desc Control the status led
 * @author Quinn Parrott
 * @date 2020-02-01
 * Last Author:
 * Last Edited On:
 */

#ifndef status_led_h
#define status_led_h

#include "global.h"


#define LED_OFF 0
#define LED_RED 1
#define LED_GREEN 2
#define LED_BLUE 4
#define LED_YELLOW (LED_RED | LED_GREEN)
#define LED_CYAN (LED_GREEN | LED_BLUE)
#define LED_MAGENTA (LED_BLUE | LED_RED)
#define LED_WHITE (LED_RED | LED_GREEN | LED_BLUE)

// Something when wrong at boot
#define STATUS_STARTUP_ERROR LED_OFF
// Everything is running fine
#define STATUS_OK LED_GREEN
// General error
#define STATUS_ERROR LED_RED


/**
 *  Set the error LED
 *
 *  @param error One of the constants defined in status_led.h
 *  @return 0 if successful, non-zero otherwise
 */
extern void status_set(uint8_t error);

#endif /* status_led_h */
