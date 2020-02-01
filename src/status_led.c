/**
 * @file status_led.c
 * @desc Control the status led
 * @author Quinn Parrott
 * @date 2020-02-01
 * Last Author:
 * Last Edited On:
 */

#include "status_led.h"

#include "config.h"
#include "gpio.h"


void status_set(uint8_t error) {

    #ifdef pins_rev_a_h
    // Revision A of the board is not full RGB. It only has
    // a bi-directional red green LED.
    if (error != STATUS_OK) {
        gpio_set_output(STAT_G_LED_PIN, 0);
        gpio_set_output(STAT_R_LED_PIN, 1);
    } else {
        gpio_set_output(STAT_R_LED_PIN, 0);
        gpio_set_output(STAT_G_LED_PIN, 1);
    }
    #endif

    #ifdef pins_rev_b_h
    // In status_led.h the LED
    gpio_set_output(STAT_R_LED_PIN, error & 1);
    gpio_set_output(STAT_G_LED_PIN, (error >> 1) & 1);
    gpio_set_output(STAT_B_LED_PIN, (error >> 2) & 1);
    #endif

    if (error != STATUS_OK) {
        gpio_set_output(DEBUG1_LED_PIN, 1);
    } else {
        gpio_set_output(DEBUG1_LED_PIN, 0);
    }

}
