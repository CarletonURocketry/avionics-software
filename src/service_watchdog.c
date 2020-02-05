/**
 * @file service_watchdog.c
 * @desc Manage services and watch for errors
 * @author Quinn Parrott
 * @date 2020-02-01
 * Last Author:
 * Last Edited On:
 */

#include "service_watchdog.h"

#include "config.h"
#include "wdt.h"


static uint32_t lastLed_g;

void service_loop(service_t *services, uint8_t services_size) {

    uint8_t service_index = 0;

    // Start Watchdog Timer
    init_wdt(GCLK_CLKCTRL_GEN_GCLK7, 14, 0);

    // Main Loop
    for (;;) {

        // Pat the watchdog
        wdt_pat();

        // TODO: Move heartbeat into it's own service
        static uint32_t period = 1000;

        if ((millis - lastLed_g) >= period) {
            lastLed_g = millis;
            gpio_toggle_output(DEBUG0_LED_PIN);
        }

        service_t current_service = services[service_index];

        current_service.call(current_service.storage);

        // Increment to the next service
        service_index = (service_index + 1) % services_size;

        // Sleep if sleep is not inhibited
        if (!inhibit_sleep_g) {
            __WFI();
        }
    }
}
