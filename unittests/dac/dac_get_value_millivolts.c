#include <unittest.h>
#include "common.c"

#include <string.h>

/*
 *  dac_get_value_millivolts() returns the current output value of the DAC in
 *  millivolt.
 *
 *  The DAC is described in section 35 of the SAMD21 datasheet.
 */

static void reset (void)
{
    memset(&dac, 0, sizeof(dac));
}


int main (int argc, char **argv)
{
    // Get DAC value of 0 V with 1 V reference.
    {
        reset();
        dac.CTRLB.bit.REFSEL = DAC_CTRLB_REFSEL_INT1V_Val;
        dac.DATA.reg = (uint16_t)0;
        uint16_t ret = dac_get_value_millivolts();

        ut_assert(ret == 0);
    }

    // Get DAC value of 0 V with 3.3 V reference. 
    {
        reset();
        dac.CTRLB.bit.REFSEL = DAC_CTRLB_REFSEL_AVCC_Val;
        dac.DATA.reg = (uint16_t)0;
        uint16_t ret = dac_get_value_millivolts();

        ut_assert(ret == 0);
    }

    // Get DAC value of 0.849 V with 1 V reference.
    {
        reset();
        dac.CTRLB.bit.REFSEL = DAC_CTRLB_REFSEL_INT1V_Val;
        dac.DATA.reg = (uint16_t)55704;
        uint16_t ret = dac_get_value_millivolts();

        ut_assert(ret == 849);
    }

    // Get DAC value of 0.849 V with 3.3 V reference.
    {
        reset();
        dac.CTRLB.bit.REFSEL = DAC_CTRLB_REFSEL_AVCC_Val;
        dac.DATA.reg = (uint16_t)16880;
        uint16_t ret = dac_get_value_millivolts();

        ut_assert(ret == 849);
    }

    // Get DAC value of 1 V with 1 V reference.
    {
        reset();
        dac.CTRLB.bit.REFSEL = DAC_CTRLB_REFSEL_INT1V_Val;
        dac.DATA.reg = (uint16_t)UINT16_MAX;
        uint16_t ret = dac_get_value_millivolts();

        ut_assert(ret == 1000);
    }

    // Get DAC value of 3.3 V with 3.3 V reference.
    {
        reset();
        dac.CTRLB.bit.REFSEL = DAC_CTRLB_REFSEL_AVCC_Val;
        dac.DATA.reg = (uint16_t)UINT16_MAX;
        uint16_t ret = dac_get_value_millivolts();

        ut_assert(ret == 3300);
    }

    return UT_PASS;
}
