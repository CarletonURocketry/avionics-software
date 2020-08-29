#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_settings_set_freq() sets the frequency in a radio settings structure.
 */

int main (int argc, char **argv)
{
    // Set frequency to a valid value.
    {
        struct rn2483_lora_settings_t settings;
        settings.freq = 0UL;

        rn2483_settings_set_freq(&settings, 433050000UL);

        // Check that the frequency was set to the correct value
        ut_assert(settings.freq == 433050000UL);
    }

    // Try to set frequency to a value above the maximum.
    {
        struct rn2483_lora_settings_t settings;
        settings.freq = 0UL;

        rn2483_settings_set_freq(&settings, 500000000UL);

        // Check that the frequency was set to the maximum value
        ut_assert(settings.freq == RN2483_FREQ_MAX);
    }

    // Try to set frequency to a value below the minimum.
    {
        struct rn2483_lora_settings_t settings;
        settings.freq = 0UL;

        rn2483_settings_set_freq(&settings, 1000UL);

        // Check that the frequency was set to the minimum value
        ut_assert(settings.freq == RN2483_FREQ_MIN);
    }

    return UT_PASS;
}
