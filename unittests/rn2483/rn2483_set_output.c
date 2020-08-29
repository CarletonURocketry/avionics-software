#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_set_output() sets the output value for a radio gpio pin.
 */

static struct rn2483_desc_t radio_descriptor;

#include "rn2483_service_stubs.c"

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
    // Set the value of gpio pin 0 to 1.
    {
        reset();

        rn2483_set_output(&radio_descriptor, RN2483_GPIO0, 1);

        // Check that the pin's value has been set
        ut_assert(radio_descriptor.pins[RN2483_GPIO0].value == 1);
        // Check that the pin's value has been marked dirty
        ut_assert(radio_descriptor.pins[RN2483_GPIO0].value_dirty == 1);
        // Check that sercom_uart_has_line was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that the idle state was executed right away
        ut_assert(executed_state == RN2483_IDLE);
    }

    // Set the value of gpio pin 5 to 0 when it is already 0.
    {
        reset();

        rn2483_set_output(&radio_descriptor, RN2483_GPIO5, 0);

        // Check that the pin's value has not chagned
        ut_assert(radio_descriptor.pins[RN2483_GPIO5].value == 0);
        // Check that the pin's value has not been marked dirty
        ut_assert(radio_descriptor.pins[RN2483_GPIO5].value_dirty == 0);
        // Check that sercom_uart_has_line was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that no state was executed right away
        ut_assert(executed_state == (enum rn2483_state)-1);
    }

    return UT_PASS;
}
