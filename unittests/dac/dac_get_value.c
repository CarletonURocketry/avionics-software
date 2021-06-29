#include <unittest.h>
#include "common.c"

#include <string.h>

/*
 *  dac_get_value() is a function which returns the current value of the DAC's
 *  DATA register.
 *
 *  The DAC is described in section 35 of the SAMD21 datasheet.
 */

static void reset (void)
{
    memset(&dac, 0, sizeof(dac));
}


int main (int argc, char **argv)
{
    // Set the DAC's output register to a value of 0.
    {
        reset();
        dac.DATA.reg = 0x0;
        uint16_t ret = dac_get_value(0);

        ut_assert(ret == 0x0);
    }

    // Set the DAC's output register to a value of 0x8000.
    {
        reset();
        dac.DATA.reg = 0x8000;
        uint16_t ret = dac_get_value(0);

        ut_assert(ret == 0x8000);
    }

    // Set the DAC's output register to a value of 0xFFFF.
    {
        reset();
        dac.DATA.reg = 0xFFFF;
        uint16_t ret = dac_get_value(0);

        ut_assert(ret == 0xFFFF);
    }

    return UT_PASS;
}
