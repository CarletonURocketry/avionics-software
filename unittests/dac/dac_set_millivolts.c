#include <unittest.h>
#include "common.c"

#include <string.h>

/*
 *  dac_set_millivolts() takes a value in millivolts and calculates the correct
 *  setting for the DACs data register in order to output that voltage.
 *
 *  The DAC is described in section 35 of the SAMD21 datasheet.
 */

static void reset (void)
{
    memset(&dac, 0, sizeof(dac));
}


int main (int argc, char **argv)
{
    // Set DAC value to 0 V with 1 V reference. Should result in DATA register
    // value of 0.
    {
        reset();
        dac.CTRLB.bit.REFSEL = DAC_CTRLB_REFSEL_INT1V_Val;
        dac_set_millivolts(0, (uint16_t)0);

        ut_assert(dac.DATA.reg == (uint16_t)0);
    }

    // Set DAC value to 0 V with 3.3 V reference. Should result in DATA register
    // value of 0.
    {
        reset();
        dac.CTRLB.bit.REFSEL = DAC_CTRLB_REFSEL_AVCC_Val;
        dac_set_millivolts(0, (uint16_t)0);

        ut_assert(dac.DATA.reg == (uint16_t)0);
    }

    // Set DAC value to 0.85 V with 1 V reference. Should result in DATA
    // register value of 55704 (floor((0.85*((2^16 - 1)/1)))).
    {
        reset();
        dac.CTRLB.bit.REFSEL = DAC_CTRLB_REFSEL_INT1V_Val;
        dac_set_millivolts(0, (uint16_t)850);

        ut_assert(dac.DATA.reg == (uint16_t)55704);
    }

    // Set DAC value to 0.85 V with 3.3 V reference. Should result in DATA
    // register value of 16880 (floor((0.85*((2^16 - 1)/1)))).
    {
        reset();
        dac.CTRLB.bit.REFSEL = DAC_CTRLB_REFSEL_AVCC_Val;
        dac_set_millivolts(0, (uint16_t)850);

        ut_assert(dac.DATA.reg == (uint16_t)16880);
    }

    // Set DAC value to 2.3 V with 1 V reference. Should result in DATA
    // register value of UINT16_MAX.
    {
        reset();
        dac.CTRLB.bit.REFSEL = DAC_CTRLB_REFSEL_INT1V_Val;
        dac_set_millivolts(0, (uint16_t)2300);

        ut_assert(dac.DATA.reg == UINT16_MAX);
    }

    // Set DAC value to 5.7 V with 3.3 V reference. Should result in DATA
    // register value of UINT16_MAX.
    {
        reset();
        dac.CTRLB.bit.REFSEL = DAC_CTRLB_REFSEL_AVCC_Val;
        dac_set_millivolts(0, (uint16_t)5700);

        ut_assert(dac.DATA.reg == UINT16_MAX);
    }

    return UT_PASS;
}
