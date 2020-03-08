#include <unittest.h>
#include "common.c"

#include <string.h>

/*
 *  dac_set() is a function which sets the DAC's output register.
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
        dac.DATA.reg = 0xAAAA; // Set data to something that is not 0
        dac_set((uint16_t)0);

        ut_assert(dac.DATA.reg == (uint16_t)0);
    }

    // Set the DAC's output register to a value of 0x8000.
    {
        reset();
        dac_set((uint16_t)0x8000);

        ut_assert(dac.DATA.reg == (uint16_t)0x8000);
    }

    // Set the DAC's output register to a value of 0xFFFF.
    {
        reset();
        dac_set((uint16_t)0xFFFF);

        ut_assert(dac.DATA.reg == (uint16_t)0xFFFF);
    }

    return UT_PASS;
}
