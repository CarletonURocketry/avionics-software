#include <unittest.h>
#include SOURCE_H

/*
 *  Tests for sercom_get_clk_id_mask() and sercom_get_pm_apb_mask(). These
 *  functions provide mask values for a given SERCOM instance.
 */


int main (int argc, char **argv)
{
    /* sercom_get_clk_id_mask() */
    // Get CLK mask for SERCOM0.
    {
        uint32_t mask = sercom_get_clk_id_mask(0);

        ut_assert(mask == GCLK_CLKCTRL_ID_SERCOM0_CORE);
    }

    // Get CLK mask for SERCOM1.
    {
        uint32_t mask = sercom_get_clk_id_mask(1);

        ut_assert(mask == GCLK_CLKCTRL_ID_SERCOM1_CORE);
    }

    // Get CLK mask for SERCOM2.
    {
        uint32_t mask = sercom_get_clk_id_mask(2);

        ut_assert(mask == GCLK_CLKCTRL_ID_SERCOM2_CORE);
    }

    // Get CLK mask for SERCOM3.
    {
        uint32_t mask = sercom_get_clk_id_mask(3);

        ut_assert(mask == GCLK_CLKCTRL_ID_SERCOM3_CORE);
    }

    // Get CLK mask for SERCOM4.
    {
        uint32_t mask = sercom_get_clk_id_mask(4);

        ut_assert(mask == GCLK_CLKCTRL_ID_SERCOM4_CORE);
    }

    // Get CLK mask for SERCOM5.
    {
        uint32_t mask = sercom_get_clk_id_mask(5);

        ut_assert(mask == GCLK_CLKCTRL_ID_SERCOM5_CORE);
    }

    // Get CLK mask for something that is not a valid SERCOM id. Returned value
    // should be 0;
    {
        uint32_t mask = sercom_get_clk_id_mask(64);

        ut_assert(mask == 0);
    }

    /* sercom_get_pm_apb_mask() */
    // Get APBC mask for SERCOM0.
    {
        uint32_t mask = sercom_get_pm_apb_mask(0);

        ut_assert(mask == PM_APBCMASK_SERCOM0);
    }

    // Get APBC mask for SERCOM1.
    {
        uint32_t mask = sercom_get_pm_apb_mask(1);

        ut_assert(mask == PM_APBCMASK_SERCOM1);
    }

    // Get APBC mask for SERCOM2.
    {
        uint32_t mask = sercom_get_pm_apb_mask(2);

        ut_assert(mask == PM_APBCMASK_SERCOM2);
    }

    // Get APBC mask for SERCOM3.
    {
        uint32_t mask = sercom_get_pm_apb_mask(3);

        ut_assert(mask == PM_APBCMASK_SERCOM3);
    }

    // Get APBC mask for SERCOM4.
    {
        uint32_t mask = sercom_get_pm_apb_mask(4);

        ut_assert(mask == PM_APBCMASK_SERCOM4);
    }

    // Get APBC mask for SERCOM5.
    {
        uint32_t mask = sercom_get_pm_apb_mask(5);

        ut_assert(mask == PM_APBCMASK_SERCOM5);
    }

    // Get APBC mask for something that is not a valid SERCOM id. Returned value
    // should be 0;
    {
        uint32_t mask = sercom_get_pm_apb_mask(64);

        ut_assert(mask == 0);
    }

    return UT_PASS;
}
