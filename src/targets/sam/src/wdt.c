/**
 * @file wdt.c
 * @desc Watchdog timer driver
 * @author Samuel Dewan
 * @date 2021-06-26
 * Last Author:
 * Last Edited On:
 */

#include "wdt.h"


uint8_t init_wdt(uint32_t core_clock_mask, uint8_t timeout,
                 uint8_t early_warning)
{
#ifndef SAMx5x
    // Configure generic clock for watchdog timer
    set_perph_generic_clock(PERPH_GCLK_WDT, core_clock_mask);
#endif

    // Ensure that WDT is disabled before changing configuration registers
#if defined(SAMD2x)
    WDT->CTRL.bit.ENABLE = 0;
#elif defined(SAMx5x)
    WDT->CTRLA.bit.ENABLE = 0;
#endif
    
    // Check that timeout is valid
    if (timeout < 3 || timeout > 14) {
        return 1;
    }
    
    // Configure time out
    WDT->CONFIG.bit.PER = timeout - 3;
#if defined(SAMD2x)
    // Wait for synchronization
    while (WDT->STATUS.bit.SYNCBUSY);
#endif
    
    if (early_warning < 3 || early_warning > 14) {
        // Configure early warning
        WDT->EWCTRL.bit.EWOFFSET = early_warning - 3;
        WDT->INTENSET.bit.EW = 1;
    } else if (early_warning != 0) {
        // Early warning is invalid
        return 1;
    }
    
    // Enable WDT
#if defined(SAMD2x)
    WDT->CTRL.reg = WDT_CTRL_ENABLE;
    // Wait for synchronization
    while (WDT->STATUS.bit.SYNCBUSY);
#elif defined(SAMx5x)
    WDT->CTRLA.reg = WDT_CTRLA_ENABLE;
    // Wait for synchronization
    while (WDT->SYNCBUSY.bit.ENABLE);
#endif
    
    return 0;
}

uint8_t init_wdt_window(uint32_t core_clock_mask, uint8_t closed, uint8_t open,
                        uint8_t early_warning)
{
#ifndef SAMx5x
    // Configure generic clock for watchdog timer
    set_perph_generic_clock(PERPH_GCLK_WDT, core_clock_mask);
#endif

    // Ensure that WDT is disabled before changing configuration registers
#if defined(SAMD2x)
    WDT->CTRL.bit.ENABLE = 0;
#elif defined(SAMx5x)
    WDT->CTRLA.bit.ENABLE = 0;
#endif
    
    // Check that window is valid
    if ((open < 3 || open > 14) || (closed < 3 || closed > 14)) {
        return 1;
    }
    // Configure window
    WDT->CONFIG.reg = WDT_CONFIG_PER(open - 3) | WDT_CONFIG_WINDOW(closed - 3);
#if defined(SAMD2x)
    // Wait for synchronization
    while (WDT->STATUS.bit.SYNCBUSY);
#endif
    
    if (early_warning < 3 || early_warning > 14) {
        // Configure early warning
        WDT->EWCTRL.bit.EWOFFSET = early_warning - 3;
        WDT->INTENSET.bit.EW = 1;
    } else if (early_warning != 0) {
        // Early warning is invalid
        return 1;
    }
    
    // Enable WDT with window mode
#if defined(SAMD2x)
    WDT->CTRL.reg = WDT_CTRL_ENABLE | WDT_CTRL_WEN;
    // Wait for synchronization
    while (WDT->STATUS.bit.SYNCBUSY);
#elif defined(SAMx5x)
    WDT->CTRLA.reg = WDT_CTRLA_ENABLE | WDT_CTRLA_WEN;
    // Wait for synchronization
    while (WDT->SYNCBUSY.bit.ENABLE);
#endif
    
    return 0;
}
