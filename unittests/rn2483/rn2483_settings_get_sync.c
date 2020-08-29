#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_settings_get_sync() gets whether a crc should be send, whether the I
 *  and Q streams should be inverted, the sync word and the preamble length from
 *  a radio settings structure.
 */

int main (int argc, char **argv)
{
    // Get syncronization values in a settings structure.
    {
        struct rn2483_lora_settings_t settings;
        settings.crc = 1;
        settings.invert_qi = 0;
        settings.sync_byte = 0x12;
        settings.preamble_length = 8000;

        uint8_t crc;
        uint8_t invert_qi;
        uint8_t sync_byte;
        uint16_t preamble_length;
        rn2483_settings_get_sync(&settings, &crc, &invert_qi, &sync_byte,
                                 &preamble_length);

        // Check that the values where fetched correctly
        ut_assert(crc == 1);
        ut_assert(invert_qi == 0);
        ut_assert(sync_byte == 0x12);
        ut_assert(preamble_length == 8000);
    }

    return UT_PASS;
}
