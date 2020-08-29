#include <unittest.h>
#include SOURCE_C


/*
 *  init_rn2483() initializes a new instance of the rn2483 driver.
 */


int main (int argc, char **argv)
{
    // Initialize a driver instance (only one case because there are no
    // branches in this function)
    {
        struct sercom_uart_desc_t uart;
        struct rn2483_desc_t radio_descriptor;
        struct rn2483_lora_settings_t settings;

        init_rn2483(&radio_descriptor, &uart, &settings);

        // Check that uart descriptor and settings struct where stored properly
        ut_assert(radio_descriptor.uart == &uart);
        ut_assert(radio_descriptor.settings == &settings);
        // Check that GPIO pins are properly initialized
        for (unsigned int pin = 0; pin < RN2483_NUM_PINS; pin++) {
            ut_assert(radio_descriptor.pins[pin].mode == RN2483_PIN_MODE_INPUT);
            ut_assert(radio_descriptor.pins[pin].mode_dirty);
        }
        // Check that state is set properly
        ut_assert(radio_descriptor.state == RN2483_RESET);
        // Check that other misc. values are set properly
        ut_assert(radio_descriptor.waiting_for_line == 0);
        ut_assert(radio_descriptor.cmd_ready == 0);
        ut_assert(radio_descriptor.position == 0);
        ut_assert(radio_descriptor.reset_try_count == 0);
    }

    return UT_PASS;
}

