/**
 * @file tc.c
 * @desc Timer Counter Driver
 * @author Samuel Dewan
 * @date 2019-04-27
 * Last Author:
 * Last Edited On:
 */

#include "tc.h"

#include <stdlib.h>

static const uint8_t tc_apb_masks[] = {
#ifdef TC0
    PM_APBCMASK_TC0_Pos,
#endif
#ifdef TC1
    PM_APBCMASK_TC1_Pos,
#endif
#ifdef TC2
    PM_APBCMASK_TC2_Pos,
#endif
#ifdef TC3
    PM_APBCMASK_TC3_Pos,
#endif
#ifdef TC4
    PM_APBCMASK_TC4_Pos,
#endif
#ifdef TC5
    PM_APBCMASK_TC5_Pos,
#endif
#ifdef TC6
    PM_APBCMASK_TC6_Pos,
#endif
#ifdef TC7
    PM_APBCMASK_TC7_Pos,
#endif
};

static const uint8_t tc_clk_ids[] = {
#ifdef TC0
    TC0_GCLK_ID,
#endif
#ifdef TC1
    TC1_GCLK_ID,
#endif
#ifdef TC2
    TC2_GCLK_ID,
#endif
#ifdef TC3
    TC3_GCLK_ID,
#endif
#ifdef TC4
    TC4_GCLK_ID,
#endif
#ifdef TC5
    TC5_GCLK_ID,
#endif
#ifdef TC6
    TC6_GCLK_ID,
#endif
#ifdef TC7
    TC7_GCLK_ID,
#endif
};

static const uint8_t tc_evsys_gen_ovf_ids[] = {
#ifdef TC0
    EVSYS_ID_GEN_TC0_OVF,
#endif
#ifdef TC1
    EVSYS_ID_GEN_TC1_OVF,
#endif
#ifdef TC2
    EVSYS_ID_GEN_TC2_OVF,
#endif
#ifdef TC3
    EVSYS_ID_GEN_TC3_OVF,
#endif
#ifdef TC4
    EVSYS_ID_GEN_TC4_OVF,
#endif
#ifdef TC5
    EVSYS_ID_GEN_TC5_OVF,
#endif
#ifdef TC6
    EVSYS_ID_GEN_TC6_OVF,
#endif
#ifdef TC7
    EVSYS_ID_GEN_TC7_OVF,
#endif
};

#define TC_NUM_PRESCALER_VALUES 8
static const uint16_t tc_prescaler_values[] = {1, 2, 4, 8, 16, 64, 256, 1024};


static int8_t tc_get_inst_num (Tc *const inst)
{
    Tc *const tc_insts[TC_INST_NUM] = TC_INSTS;
    
    for (int i = 0; i < TC_INST_NUM; i++) {
        if ((uintptr_t)tc_insts[i] == (uintptr_t)inst) {
            return i;
        }
    }
    
    return -1;
}


uint8_t init_tc_periodic_event (Tc *tc, uint32_t period, uint32_t clock_mask,
                                uint32_t clock_freq)
{
    uint8_t inst_num = tc_get_inst_num(tc);
    
    /* Enable TC instance interface clock */
    PM->APBCMASK.reg |= (1 << tc_apb_masks[inst_num]);
    
    /* Configure generic clock for TC instance */
    GCLK->CLKCTRL.reg = (GCLK_CLKCTRL_CLKEN | clock_mask |
                         GCLK_CLKCTRL_ID(tc_clk_ids[inst_num]));
    // Wait for synchronization
    while (GCLK->STATUS.bit.SYNCBUSY);
    
    /* Reset TC */
    tc->COUNT16.CTRLA.bit.SWRST = 1;
    // Wait for reset to complete
    while (tc->COUNT16.CTRLA.bit.SWRST | tc->COUNT16.STATUS.bit.SYNCBUSY);
    
    /* Find prescaler and top values */
    uint8_t prescaler = 0xFF;
    uint16_t top;
    uint32_t min_error = UINT32_MAX;
    
    // Iterate through prescaler options and calculate period error for each
    for (int8_t i = 7; i >= 0; i--) {
        // Calcualte top, perform calculations in uint64_t to avoid losing precision
        uint64_t temp = ((uint64_t)clock_freq << 32) / (tc_prescaler_values[i] * 1000);
        uint32_t t = (uint32_t)((temp * period) >> 32);
        if ((t - 1) > UINT16_MAX) {
            // Top is too high, all further prescalers will also be too small
            break;
        }
        // Find the actual period
        int32_t p = ((uint64_t)tc_prescaler_values[i] * 1000 * t) / clock_freq;
        // Find the difference between the actual period and the target period
        uint32_t error = abs(p - (int32_t)period);
        
        if (error == 0) {
            // We won't get better than this, all done
            top = (uint16_t)(t - 1);
            prescaler = i;
            break;
        } else if (error < min_error) {
            // This is the best so far, but it could be better
            min_error = error;
            top = (uint16_t)(t - 1);
            prescaler = i;
        }
    }
    
    if (prescaler == 0xFF) {
        // Prescaler was not set, it is not possible to get the desired period
        // with the provided clock
        return 1;
    }
    
    /* Write CTRLA */
    tc->COUNT16.CTRLA.reg = (TC_CTRLA_PRESCSYNC_RESYNC |
                             TC_CTRLA_PRESCALER(prescaler) |
                             TC_CTRLA_WAVEGEN_MFRQ |
                             TC_CTRLA_MODE_COUNT16);
    // Wait for synchronization
    while (tc->COUNT16.STATUS.bit.SYNCBUSY);
    
    /* Configure TOP */
    tc->COUNT16.CC[0].reg = top;
    
    /* Configure events */
    tc->COUNT16.EVCTRL.reg = TC_EVCTRL_OVFEO;
    
    /* Enable timer */
    tc->COUNT16.CTRLA.bit.ENABLE = 1;
    // Wait for synchronization
    while (tc->COUNT16.STATUS.bit.SYNCBUSY);
    
    return 0;
}

uint8_t tc_get_evsys_gen_ovf_id (Tc *tc)
{
    return tc_evsys_gen_ovf_ids[tc_get_inst_num(tc)];
}
