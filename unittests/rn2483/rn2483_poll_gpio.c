#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_poll_gpio() marks all of the radio module GPIO pins that have been
 *  explicitely configured as
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
}

int main (int argc, char **argv)
{
    // Run poll function when there are no pins that have been explicitly set as
    // inputs.
    {
        reset();
        radio_descriptor.state = RN2483_IDLE;

        rn2483_poll_gpio(&radio_descriptor);

        // Check that no pins where marked as dirty
        for (enum rn2483_pin pin = 0; pin < RN2483_NUM_PINS; pin++) {
            ut_assert(radio_descriptor.pins[pin].value_dirty == 0);
        }
        // Check that the driver state was not changed
        ut_assert(radio_descriptor.state == RN2483_IDLE);
        // Checl that the sercom_uart_has_line function was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that we ran the idle state right away
        ut_assert(executed_state == RN2483_IDLE);
    }

    // Run poll function when pin 0 is an explicit input and pin 1 is an
    // explicit ananlog input. Driver is currently in the process of a reception
    // that must be cancled.
    {
        reset();
        radio_descriptor.state = RN2483_RECEIVE;
        radio_descriptor.pins[0].mode = RN2483_PIN_MODE_INPUT;
        radio_descriptor.pins[0].mode_explicit = 1;
        radio_descriptor.pins[1].mode = RN2483_PIN_MODE_ANALOG;
        radio_descriptor.pins[1].mode_explicit = 1;

        rn2483_poll_gpio(&radio_descriptor);

        // Check that pins 0 and 1 have been marked as dirty
        ut_assert(radio_descriptor.pins[0].value_dirty == 1);
        ut_assert(radio_descriptor.pins[1].value_dirty == 1);
        // Check that none of the other pins where marked as dirty
        for (enum rn2483_pin pin = 2; pin < RN2483_NUM_PINS; pin++) {
            ut_assert(radio_descriptor.pins[pin].value_dirty == 0);
        }
        // Check that the driver state was changed tp receive abost
        ut_assert(radio_descriptor.state == RN2483_RECEIVE_ABORT);
        // Checl that the sercom_uart_has_line function was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that we ran the receive abort state right away
        ut_assert(executed_state == RN2483_RECEIVE_ABORT);
    }

    // Run poll function when pis 5 and 6 are an explicit inputs and pins 7 and
    // 8 are explicit outputs. Driver is currently in the process of a reception
    // that must be cancled.
    {
        reset();
        radio_descriptor.state = RN2483_RECEIVE;
        radio_descriptor.pins[5].mode = RN2483_PIN_MODE_INPUT;
        radio_descriptor.pins[5].mode_explicit = 1;
        radio_descriptor.pins[6].mode = RN2483_PIN_MODE_INPUT;
        radio_descriptor.pins[6].mode_explicit = 1;
        radio_descriptor.pins[7].mode = RN2483_PIN_MODE_OUTPUT;
        radio_descriptor.pins[7].mode_explicit = 1;
        radio_descriptor.pins[8].mode = RN2483_PIN_MODE_OUTPUT;
        radio_descriptor.pins[8].mode_explicit = 1;

        rn2483_poll_gpio(&radio_descriptor);

        // Check that none of pins 0 through 4 have been marked dirty
        for (enum rn2483_pin pin = 0; pin < 5; pin++) {
            ut_assert(radio_descriptor.pins[pin].value_dirty == 0);
        }
        // Check that pins 5 and 6 have been marked as dirty
        ut_assert(radio_descriptor.pins[5].value_dirty == 1);
        ut_assert(radio_descriptor.pins[6].value_dirty == 1);
        // Check that pins 7 and 8 are not marked dirty
        ut_assert(radio_descriptor.pins[7].value_dirty == 0);
        ut_assert(radio_descriptor.pins[8].value_dirty == 0);
        // Check that none of the other pins where marked as dirty
        for (enum rn2483_pin pin = 9; pin < RN2483_NUM_PINS; pin++) {
            ut_assert(radio_descriptor.pins[pin].value_dirty == 0);
        }
        // Check that the driver state was changed tp receive abost
        ut_assert(radio_descriptor.state == RN2483_RECEIVE_ABORT);
        // Checl that the sercom_uart_has_line function was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that we ran the receive abort state right away
        ut_assert(executed_state == RN2483_RECEIVE_ABORT);
    }

    return UT_PASS;
}
