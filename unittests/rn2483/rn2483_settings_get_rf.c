#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_settings_get_rf() gets the power level, spreading factor, coding
 *  rate and bandwidth values from a radio settings structure.
 */

int main (int argc, char **argv)
{
    // Get rf values in a settings structure.
    {
        struct rn2483_lora_settings_t settings;
        settings.power = 12;
        settings.spreading_factor = RN2483_SF_SF11;
        settings.coding_rate = RN2483_CR_4_7;
        settings.bandwidth = RN2483_BW_500;

        int8_t power;
        enum rn2483_sf spreading_factor;
        enum rn2483_cr coding_rate;
        enum rn2483_bw bandwidth;
        rn2483_settings_get_rf(&settings, &power, &spreading_factor,
                               &coding_rate, &bandwidth);

        // Check that the values where fetched correctly
        ut_assert(power == 12);
        ut_assert(spreading_factor == RN2483_SF_SF11);
        ut_assert(coding_rate == RN2483_CR_4_7);
        ut_assert(bandwidth == RN2483_BW_500);
    }

    return UT_PASS;
}
