#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_settings_get_freq() returns the frequency value from a radio settings
 *  structure.
 */

int main (int argc, char **argv)
{
    {
        struct rn2483_lora_settings_t settings;
        settings.freq = 433050000UL;

        uint32_t ret = rn2483_settings_get_freq(&settings);

        ut_assert(ret == 433050000UL);
    }

    return UT_PASS;
}
