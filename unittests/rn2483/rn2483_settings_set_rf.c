#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_settings_set_rf() sets the power level, spreading factor, coding
 *  rate and bandwidth values in a radio settings structure.
 */

int main (int argc, char **argv)
{
    // Set rf values in a settings structure.
    {
        struct rn2483_lora_settings_t settings;
        settings.power = 0;
        settings.spreading_factor = RN2483_SF_SF7;
        settings.coding_rate = RN2483_CR_4_5;
        settings.bandwidth = RN2483_BW_125;

        rn2483_settings_set_rf(&settings, 10, RN2483_SF_SF9, RN2483_CR_4_7,
                               RN2483_BW_250);

        // Check that the values where set correctly
        ut_assert(settings.power == 10);
        ut_assert(settings.spreading_factor == RN2483_SF_SF9);
        ut_assert(settings.coding_rate == RN2483_CR_4_7);
        ut_assert(settings.bandwidth == RN2483_BW_250);
    }

    // Try to set rf values with a power that is above the maximum
    {
        struct rn2483_lora_settings_t settings;
        settings.power = 0;
        settings.spreading_factor = RN2483_SF_SF7;
        settings.coding_rate = RN2483_CR_4_5;
        settings.bandwidth = RN2483_BW_125;

        rn2483_settings_set_rf(&settings, 25, RN2483_SF_SF10, RN2483_CR_4_8,
                               RN2483_BW_500);

        // Check that the power was set to the maximum value
        ut_assert(settings.power == 14);
        // Check that the other values where set correctly
        ut_assert(settings.spreading_factor == RN2483_SF_SF10);
        ut_assert(settings.coding_rate == RN2483_CR_4_8);
        ut_assert(settings.bandwidth == RN2483_BW_500);
    }

    // Try to set frequency to a value below the minimum.
    {
        struct rn2483_lora_settings_t settings;
        settings.power = 0;
        settings.spreading_factor = RN2483_SF_SF7;
        settings.coding_rate = RN2483_CR_4_5;
        settings.bandwidth = RN2483_BW_125;

        rn2483_settings_set_rf(&settings, -54, RN2483_SF_SF12, RN2483_CR_4_6,
                               RN2483_BW_250);

        // Check that the power was set to the minimum value
        ut_assert(settings.power == -3);
        // Check that the other values where set correctly
        ut_assert(settings.spreading_factor == RN2483_SF_SF12);
        ut_assert(settings.coding_rate == RN2483_CR_4_6);
        ut_assert(settings.bandwidth == RN2483_BW_250);
    }

    return UT_PASS;
}
