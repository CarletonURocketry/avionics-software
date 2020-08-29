#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_get_input() gets the current value for a radio module gpio pin that
 *  is configured as a digital input.
 */

int main (int argc, char **argv)
{
    // Get the value for the gpio 8 pin.
    {
        static struct rn2483_desc_t radio_descriptor;
        radio_descriptor.pins[RN2483_GPIO8].mode = RN2483_PIN_MODE_INPUT;
        radio_descriptor.pins[RN2483_GPIO8].value = 0;

        uint8_t ret = rn2483_get_input(&radio_descriptor, RN2483_GPIO8);

        ut_assert(ret == 0);
    }

    // Get the value for the gpio 0 pin.
    {
        static struct rn2483_desc_t radio_descriptor;
        radio_descriptor.pins[RN2483_GPIO0].mode = RN2483_PIN_MODE_INPUT;
        radio_descriptor.pins[RN2483_GPIO0].value = 1;

        uint8_t ret = rn2483_get_input(&radio_descriptor, RN2483_GPIO0);

        ut_assert(ret == 1);
    }

    return UT_PASS;
}
