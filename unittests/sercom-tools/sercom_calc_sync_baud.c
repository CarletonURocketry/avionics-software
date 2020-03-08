#include <unittest.h>
#include SOURCE_C

/*
 *  sercom_calc_sync_baud() is a helper function which calculates the baud
 *  register value for synchronous baud rate generation.
 *
 *  Baud rate generation is described in section 25.6.2.3 of the SAMD21
 *  datasheet.
 */


int main (int argc, char **argv)
{
    // Calculate register value for a rate of 12 mbaud with a clock frequency of
    // 48 MHz. This should result in a value of 1.
    {
        uint8_t baud;
        uint8_t ret = sercom_calc_sync_baud(12000000, 48000000, &baud);

        ut_assert(ret == 0);
        ut_assert(baud == 1); // ceil((48000000/(2*12000000))-1)
    }

    // Calculate register value for a rate of 1 mbaud with a clock frequency of
    // 48 MHz. This should result in a value of 23.
    {
        uint8_t baud;
        uint8_t ret = sercom_calc_sync_baud(1000000, 48000000, &baud);

        ut_assert(ret == 0);
        ut_assert(baud == 23); // ceil((48000000/(2*1000000))-1)
    }

    // Calculate register value for a rate of 7.5 mbaud with a clock frequency
    // of 48 MHz. This should result in a value of 3. A baud register value of 3
    // actually results in a baud rate of 6 mbaud, which is not the closest
    // supported baud rate but the function should provide the next lowest
    // supported rate.
    {
        uint8_t baud;
        uint8_t ret = sercom_calc_sync_baud(7500000, 48000000, &baud);

        ut_assert(ret == 0);
        ut_assert(baud == 3); // ceil((48000000/(2*7500000))-1)
    }

    // Calculate register value for a rate of 200 kbaud with a clock frequency
    // of 16 MHz. This should result in a value of 39.
    {
        uint8_t baud;
        uint8_t ret = sercom_calc_sync_baud(200000, 16000000, &baud);

        ut_assert(ret == 0);
        ut_assert(baud == 39); // ceil((16000000/(2*200000))-1)
    }

    // Calculate register value for a rate of 4 mbaud with a clock frequency of
    // 8 MHz. This should result in a value of 0.
    {
        uint8_t baud;
        uint8_t ret = sercom_calc_sync_baud(4000000, 8000000, &baud);

        ut_assert(ret == 0);
        ut_assert(baud == 0); // ceil((8000000/(2*4000000))-1)
    }

    // Calculate register value for a rate of 6 mbaud with a clock frequency of
    // 8 MHz. The function should fail as this baud rate is too high for the
    // clock frequency.
    {
        uint8_t baud;
        uint8_t ret = sercom_calc_sync_baud(6000000, 8000000, &baud);

        ut_assert(ret != 0);
    }

    return UT_PASS;
}
