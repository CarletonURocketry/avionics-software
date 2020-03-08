#include <unittest.h>
#include SOURCE_C


/*
 *  sercom_calc_async_baud() is a helper function which calculates the proper
 *  baud register setting at the highest possible sample rate for a given
 *  baudrate and sercom clock frequency.
 *
 *  Baud rate generation is described in section 25.6.2.3 of the SAMD21
 *  datasheet.
 */


int main (int argc, char **argv)
{
    // Calculate values for a rate of 115.2 kbaud with a clock frequency of
    // 48 MHz. This should work fine at 16 times oversampling and result in a
    // baud value of 63019.
    {
        uint16_t baud;
        uint8_t sampr;
        uint8_t ret = sercom_calc_async_baud(115200, 48000000, &baud, &sampr);

        ut_assert(ret == 0);
        ut_assert(sampr == 0x0); // 16 times sample rate
        ut_assert(baud == 63019); // floor(65536*(1 - (16 * (115200/48000000))))
    }

    // Calculate values for a rate of 9.6 kbaud with a clock frequency of
    // 48 MHz. This should work fine at 16 times oversampling and result in a
    // baud value of 65326.
    {
        uint16_t baud;
        uint8_t sampr;
        uint8_t ret = sercom_calc_async_baud(9600, 48000000, &baud, &sampr);

        ut_assert(ret == 0);
        ut_assert(sampr == 0x0); // 16 times sample rate
        ut_assert(baud == 65326); // floor(65536*(1 - (16 * (9600/48000000))))
    }

    // Calculate values for a rate of 9.6 kbaud with a clock frequency of 8 MHz.
    // This should work fine at 16 times oversampling and result in a baud
    // register value of 64277.
    {
        uint16_t baud;
        uint8_t sampr;
        uint8_t ret = sercom_calc_async_baud(9600, 8000000, &baud, &sampr);

        ut_assert(ret == 0);
        ut_assert(sampr == 0x0); // 16 times sample rate
        ut_assert(baud == 64277); // floor(65536*(1 - (16 * (9600/8000000))))
    }

    // Calculate values for a rate of 1.5 mbaud with a clock frequency of
    // 16 MHz. This should work at 8 times oversampling and result in a baud
    // value of 16384
    {
        uint16_t baud;
        uint8_t sampr;
        uint8_t ret = sercom_calc_async_baud(1500000, 16000000, &baud, &sampr);

        ut_assert(ret == 0);
        ut_assert(sampr == 0x2); // 8 times sample rate
        ut_assert(baud == 16384); // floor(65536*(1 - (8 * (1500000/16000000))))
    }

    // Calculate values for a rate of 250 kbaud with a clock frequency of 1 MHz.
    // This works with 3 times oversampling and should result in a baud value of
    // 16384.
    {
        uint16_t baud;
        uint8_t sampr;
        uint8_t ret = sercom_calc_async_baud(250000, 1000000, &baud, &sampr);

        ut_assert(ret == 0);
        ut_assert(sampr == 0x4); // 3 times sample rate
        ut_assert(baud == 16384); // floor(65536*(1 - (3 * (250000/1000000))))
    }

    // Try to calculate values for a rate of 8 mbaud at a clock frequency of
    // 12 MHz. This should not be possible.
    {
        uint16_t baud;
        uint8_t sampr;
        uint8_t ret = sercom_calc_async_baud(8000000, 12000000, &baud, &sampr);

        ut_assert(ret != 0);
    }

    return UT_PASS;
}

