#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_get_analog() gets the current value for a radio module gpio pin that
 *  is configured as an analog input.
 */

int main (int argc, char **argv)
{
    // Get the analog value for the gpio 10 pin.
    {
        static struct rn2483_desc_t radio_descriptor;
        radio_descriptor.pins[RN2483_GPIO10].mode = RN2483_PIN_MODE_ANALOG;
        radio_descriptor.pins[RN2483_GPIO10].value = 0x3AA;

        uint16_t ret = rn2483_get_analog(&radio_descriptor, RN2483_GPIO10);

        ut_assert(ret == 0x3AA);
    }

    // Get the analog value for the gpio 13 pin.
    {
        static struct rn2483_desc_t radio_descriptor;
        radio_descriptor.pins[RN2483_GPIO13].mode = RN2483_PIN_MODE_ANALOG;
        radio_descriptor.pins[RN2483_GPIO13].value = 0x6B;

        uint16_t ret = rn2483_get_analog(&radio_descriptor, RN2483_GPIO13);

        ut_assert(ret == 0x6B);
    }

    // Get the analog value for the gpio 4 pin wich is not configured as an
    // analog input.
    {
        static struct rn2483_desc_t radio_descriptor;
        radio_descriptor.pins[RN2483_GPIO4].mode = RN2483_PIN_MODE_OUTPUT;
        radio_descriptor.pins[RN2483_GPIO4].value = 0;

        uint16_t ret = rn2483_get_analog(&radio_descriptor, RN2483_GPIO4);

        ut_assert(ret == 0xFFFF);
    }

    return UT_PASS;
}
