#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_settings_set_sync() sets whether a crc should be send, whether the I
 *  and Q streams should be inverted, the sync word and the preamble length in a
 *  radio settings structure.
 */

int main (int argc, char **argv)
{
    // Set syncronization values in a settings structure.
    {
        struct rn2483_lora_settings_t settings;
        settings.crc = 0;
        settings.invert_qi = 0;
        settings.sync_byte = 0;
        settings.preamble_length = 0;

        rn2483_settings_set_sync(&settings, 1, 1, 0xAA, 10);

        // Check that the values where set correctly
        ut_assert(settings.crc == 1);
        ut_assert(settings.invert_qi == 1);
        ut_assert(settings.sync_byte == 0xAA);
        ut_assert(settings.preamble_length == 10);
    }

    return UT_PASS;
}
