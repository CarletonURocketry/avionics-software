#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_get_pin_mode() fetches the current mode of a radio module gpio pin.
 */

int main (int argc, char **argv)
{
    // Get the mode for the gpio 3 pin which is an input.
    {
        static struct rn2483_desc_t radio_descriptor;
        radio_descriptor.pins[RN2483_GPIO3].mode = RN2483_PIN_MODE_INPUT;

        enum rn2483_pin_mode ret = rn2483_get_pin_mode(&radio_descriptor,
                                                       RN2483_GPIO3);

        ut_assert(ret == RN2483_PIN_MODE_INPUT);
    }

    // Get the mode for the UART RTS pin which is an ouput.
    {
        static struct rn2483_desc_t radio_descriptor;
        radio_descriptor.pins[RN2483_UART_RTS].mode = RN2483_PIN_MODE_OUTPUT;

        enum rn2483_pin_mode ret = rn2483_get_pin_mode(&radio_descriptor,
                                                       RN2483_UART_RTS);

        ut_assert(ret == RN2483_PIN_MODE_OUTPUT);
    }

    return UT_PASS;
}
