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

static const enum peripheral_bus_clock tc_bus_clocks[] = {
#ifdef TC0
    PERPH_BUS_CLK_TC0_APB,
#endif
#ifdef TC1
    PERPH_BUS_CLK_TC1_APB,
#endif
#ifdef TC2
    PERPH_BUS_CLK_TC2_APB,
#endif
#ifdef TC3
    PERPH_BUS_CLK_TC3_APB,
#endif
#ifdef TC4
    PERPH_BUS_CLK_TC4_APB,
#endif
#ifdef TC5
    PERPH_BUS_CLK_TC5_APB,
#endif
#ifdef TC6
    PERPH_BUS_CLK_TC6_APB,
#endif
#ifdef TC7
    PERPH_BUS_CLK_TC7_APB,
#endif
};

static const enum peripheral_generic_clock tc_glcks[] = {
#ifdef TC0
    PERPH_GCLK_TC0_TC1,
#endif
#ifdef TC1
    PERPH_GCLK_TC0_TC1,
#endif
#ifdef TC2
    PERPH_GCLK_TC2_TC3,
#endif
#ifdef TC3
#if defined(SAMD2x)
    PERPH_GCLK_TCC2_TC3,
#elif defined(SAMx5x)
    PERPH_GCLK_TC2_TC3,
#endif
#endif
#ifdef TC4
    PERPH_GCLK_TC4_TC5,
#endif
#ifdef TC5
    PERPH_GCLK_TC4_TC5,
#endif
#ifdef TC6
    PERPH_GCLK_TC6_TC7,
#endif
#ifdef TC7
    PERPH_GCLK_TC6_TC7,
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
    enable_bus_clock(tc_bus_clocks[inst_num]);

    /* Configure generic clock for TC instance */
    set_perph_generic_clock(tc_glcks[inst_num], clock_mask);
    
    /* Reset TC */
    tc->COUNT16.CTRLA.bit.SWRST = 1;
    // Wait for reset to complete
#if defined(SAMD2x)
    while (tc->COUNT16.CTRLA.bit.SWRST | tc->COUNT16.STATUS.bit.SYNCBUSY);
#elif defined(SAMx5x)
    while (tc->COUNT16.CTRLA.bit.SWRST | tc->COUNT16.SYNCBUSY.bit.SWRST);
#endif
    
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
#if defined(SAMD2x)
                             TC_CTRLA_WAVEGEN_MFRQ |
#endif
                             TC_CTRLA_MODE_COUNT16);
#if defined(SAMD2x)
    // Wait for synchronization
    while (tc->COUNT16.STATUS.bit.SYNCBUSY);
#endif

#if defined(SAMx5x)
    tc->COUNT16.WAVE.reg = TC_WAVE_WAVEGEN_MFRQ;
#endif
    
    /* Configure TOP */
    tc->COUNT16.CC[0].reg = top;
    
    /* Configure events */
    tc->COUNT16.EVCTRL.reg = TC_EVCTRL_OVFEO;
    
    /* Enable timer */
    tc->COUNT16.CTRLA.bit.ENABLE = 1;
    // Wait for synchronization
#if defined(SAMD2x)
    while (tc->COUNT16.STATUS.bit.SYNCBUSY);
#elif defined(SAMx5x)
    while (tc->COUNT16.SYNCBUSY.bit.ENABLE);
#endif
    
    return 0;
}

uint8_t tc_get_evsys_gen_ovf_id (Tc *tc)
{
    return tc_evsys_gen_ovf_ids[tc_get_inst_num(tc)];
}
