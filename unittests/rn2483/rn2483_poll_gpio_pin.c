#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_poll_gpio_pin() indicates that the value for a GPIO pin should be
 *  updated.
 */

#include "rn2483_service_stubs.c"

static struct rn2483_desc_t radio_descriptor;

static void reset (void)
{
    sercom_uart_has_line_call_count = 0;
    executed_state = (enum rn2483_state)-1;
    radio_descriptor.waiting_for_line = 0;
    sercom_uart_has_line_retval = 0;
    // Reset all pins
    for (enum rn2483_pin pin = 0; pin < RN2483_NUM_PINS; pin++) {
        radio_descriptor.pins[pin].raw =
                            RN2483_PIN_DESC_MODE(RN2483_PIN_MODE_OUTPUT);
    }
    radio_descriptor.state = RN2483_IDLE;
}

int main (int argc, char **argv)
{
    // Poll GPIO pin 8.
    {
        reset();

        rn2483_poll_gpio_pin(&radio_descriptor, RN2483_GPIO8);

        // Check that the dirty bit was set properly
        ut_assert(radio_descriptor.pins[RN2483_GPIO8].value_dirty == 1);
        // Check that sercom_uart_has_line was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that the idle state was executed right away
        ut_assert(executed_state == RN2483_IDLE);
    }

    return UT_PASS;
}
