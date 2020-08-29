#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_poll_gpio_in_progress() check if the polling of any inputs is in
 *  progress.
 */

static struct rn2483_desc_t radio_descriptor;

static void reset (void)
{
    // Reset all pins
    for (enum rn2483_pin pin = 0; pin < RN2483_NUM_PINS; pin++) {
        radio_descriptor.pins[pin].raw =
                                    RN2483_PIN_DESC_MODE(RN2483_PIN_MODE_INPUT);
    }
}

int main (int argc, char **argv)
{
    // Test where all pins are either digital or analog inputs but none are
    // marked as dirty. Should return 0 (false).
    {
        reset();
        // Make a couple pins ananlog inputs
        radio_descriptor.pins[0].mode = RN2483_PIN_MODE_ANALOG;
        radio_descriptor.pins[7].mode = RN2483_PIN_MODE_ANALOG;

        uint8_t ret = rn2483_poll_gpio_in_progress(&radio_descriptor);

        ut_assert(ret == 0);
    }

    // Test where some pins are outputs and some of the outputs are marked as
    // dirty. Should return 0 (false).
    {
        reset();
        // Make a couple pins outputs
        radio_descriptor.pins[4].mode = RN2483_PIN_MODE_OUTPUT;
        radio_descriptor.pins[10].mode = RN2483_PIN_MODE_OUTPUT;
        // Make a couple pins outputs with dirty values
        radio_descriptor.pins[1].mode = RN2483_PIN_MODE_OUTPUT;
        radio_descriptor.pins[1].value_dirty = 1;
        radio_descriptor.pins[7].mode = RN2483_PIN_MODE_OUTPUT;
        radio_descriptor.pins[7].value_dirty = 1;

        uint8_t ret = rn2483_poll_gpio_in_progress(&radio_descriptor);

        ut_assert(ret == 0);
    }

    // Test where there is an input with a dirty value. Should return 1 (true).
    {
        reset();

        radio_descriptor.pins[3].mode = RN2483_PIN_MODE_INPUT;
        radio_descriptor.pins[3].value_dirty = 1;

        uint8_t ret = rn2483_poll_gpio_in_progress(&radio_descriptor);

        ut_assert(ret == 1);
    }

    // Test where there is an analog input with a dirty value. Should return 1
    // (true).
    {
        reset();

        radio_descriptor.pins[9].mode = RN2483_PIN_MODE_ANALOG;
        radio_descriptor.pins[9].value_dirty = 1;

        uint8_t ret = rn2483_poll_gpio_in_progress(&radio_descriptor);

        ut_assert(ret == 1);
    }

    return UT_PASS;
}
