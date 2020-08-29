#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_set_pin_mode() sets the mode for a radio module pin.
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
                                    RN2483_PIN_DESC_MODE(RN2483_PIN_MODE_INPUT);
    }
    radio_descriptor.state = RN2483_IDLE;
}

int main (int argc, char **argv)
{
    // Make GPIO 6 an output.
    {
        reset();
        radio_descriptor.pins[RN2483_GPIO6].mode = RN2483_PIN_MODE_INPUT;
        radio_descriptor.pins[RN2483_GPIO6].value = 1;

        uint8_t ret = rn2483_set_pin_mode(&radio_descriptor, RN2483_GPIO6,
                                          RN2483_PIN_MODE_OUTPUT);

        // Check that the return value indicates success
        ut_assert(ret == 0);
        // Check that the pin's mode has been updated
        ut_assert(radio_descriptor.pins[RN2483_GPIO6].mode ==
                                                        RN2483_PIN_MODE_OUTPUT);
        // Check that the pin has been marked as having an explicitly set mode
        ut_assert(radio_descriptor.pins[RN2483_GPIO6].mode_explicit == 1);
        // Check that the pin's mode and value have been marked dirty
        ut_assert(radio_descriptor.pins[RN2483_GPIO6].mode_dirty == 1);
        ut_assert(radio_descriptor.pins[RN2483_GPIO6].value_dirty == 1);
        // Check that the pin's value has been cleared
        ut_assert(radio_descriptor.pins[RN2483_GPIO6].value == 0);
        // Check that sercom_uart_has_line was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that the idle state was executed right away
        ut_assert(executed_state == RN2483_IDLE);
    }

    // Make GPIO 2 an input when it already is one.
    {
        reset();
        radio_descriptor.pins[RN2483_GPIO2].mode = RN2483_PIN_MODE_INPUT;
        radio_descriptor.pins[RN2483_GPIO2].value = 1;

        uint8_t ret = rn2483_set_pin_mode(&radio_descriptor, RN2483_GPIO2,
                                          RN2483_PIN_MODE_INPUT);

        // Check that the return value indicates success
        ut_assert(ret == 0);
        // Check that the pin's mode is correct
        ut_assert(radio_descriptor.pins[RN2483_GPIO2].mode ==
                                                        RN2483_PIN_MODE_INPUT);
        // Check that the pin has been marked as having an explicitly set mode
        ut_assert(radio_descriptor.pins[RN2483_GPIO2].mode_explicit == 1);
        // Check that the pin's mode and value have not been marked dirty
        ut_assert(radio_descriptor.pins[RN2483_GPIO2].mode_dirty == 0);
        ut_assert(radio_descriptor.pins[RN2483_GPIO2].value_dirty == 0);
        // Check that the pin's value has not been cleared
        ut_assert(radio_descriptor.pins[RN2483_GPIO2].value == 1);
        // Check that sercom_uart_has_line was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that no state was executed right away
        ut_assert(executed_state == (enum rn2483_state)-1);
    }

    // Make GPIO 12 an analog input.
    {
        reset();
        radio_descriptor.pins[RN2483_GPIO12].mode = RN2483_PIN_MODE_INPUT;
        radio_descriptor.pins[RN2483_GPIO12].value = 1;

        uint8_t ret = rn2483_set_pin_mode(&radio_descriptor, RN2483_GPIO12,
                                          RN2483_PIN_MODE_ANALOG);

        // Check that the return value indicates success
        ut_assert(ret == 0);
        // Check that the pin's mode has been updated
        ut_assert(radio_descriptor.pins[RN2483_GPIO12].mode ==
                                                        RN2483_PIN_MODE_ANALOG);
        // Check that the pin has been marked as having an explicitly set mode
        ut_assert(radio_descriptor.pins[RN2483_GPIO12].mode_explicit == 1);
        // Check that the pin's mode and value have been marked dirty
        ut_assert(radio_descriptor.pins[RN2483_GPIO12].mode_dirty == 1);
        ut_assert(radio_descriptor.pins[RN2483_GPIO12].value_dirty == 1);
        // Check that the pin's value has been cleared
        ut_assert(radio_descriptor.pins[RN2483_GPIO12].value == 0);
        // Check that sercom_uart_has_line was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that the idle state was executed right away
        ut_assert(executed_state == RN2483_IDLE);
    }

    // Try to make thhe UART CTS pin (which does not support analog input) an
    // ananlog input.
    {
        reset();
        radio_descriptor.pins[RN2483_UART_CTS].mode = RN2483_PIN_MODE_OUTPUT;
        radio_descriptor.pins[RN2483_UART_CTS].value = 1;

        uint8_t ret = rn2483_set_pin_mode(&radio_descriptor, RN2483_UART_CTS,
                                          RN2483_PIN_MODE_ANALOG);

        // Check that the return value indicates failure
        ut_assert(ret != 0);
        // Check that the pin's mode has not been updated
        ut_assert(radio_descriptor.pins[RN2483_UART_CTS].mode ==
                                                        RN2483_PIN_MODE_OUTPUT);
        // Check that the pin has not been marked as having an explicitly set
        // mode
        ut_assert(radio_descriptor.pins[RN2483_UART_CTS].mode_explicit == 0);
        // Check that the pin's mode and value have not been marked dirty
        ut_assert(radio_descriptor.pins[RN2483_UART_CTS].mode_dirty == 0);
        ut_assert(radio_descriptor.pins[RN2483_UART_CTS].value_dirty == 0);
        // Check that the pin's value has not been cleared
        ut_assert(radio_descriptor.pins[RN2483_UART_CTS].value == 1);
        // Check that sercom_uart_has_line was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that no state was executed right away
        ut_assert(executed_state == (enum rn2483_state)-1);
    }

    return UT_PASS;
}
