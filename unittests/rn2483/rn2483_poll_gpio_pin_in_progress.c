#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_poll_gpio_pin_in_progress() checks whether a radio modile gpio pin is
 *  being polled.
 */

int main (int argc, char **argv)
{
    // Check if the TEST0 pin is being polled when it is not marked as dirty.
    {
        static struct rn2483_desc_t radio_descriptor;
        radio_descriptor.pins[RN2483_TEST0].value_dirty = 0;

        uint8_t ret = rn2483_poll_gpio_pin_in_progress(&radio_descriptor,
                                                       RN2483_TEST0);

        ut_assert(ret == 0);
    }

    // Check if the TEST0 pin is being polled when it is marked as dirty.
    {
        static struct rn2483_desc_t radio_descriptor;
        radio_descriptor.pins[RN2483_TEST0].value_dirty = 1;

        uint8_t ret = rn2483_poll_gpio_pin_in_progress(&radio_descriptor,
                                                       RN2483_TEST0);

        ut_assert(ret == 1);
    }

    return UT_PASS;
}
