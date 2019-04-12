/**
 * @file wdt.c
 * @desc Watchdog timer and brown out dector driver
 * @author Samuel Dewan
 * @date 2019-04-10
 * Last Author:
 * Last Edited On:
 */

#include "wdt.h"


uint8_t init_wdt(uint32_t core_clock_mask, uint8_t timeout,
                 uint8_t early_warning)
{
    // Ensure that WDT is disabled before changing configuration registers
    WDT->CTRL.bit.ENABLE = 0;
    
    // Check that timeout is valid
    if (timeout < 3 || timeout > 14) {
        return 1;
    }
    
    // Configure time out
    WDT->CONFIG.bit.PER = timeout = 3;
    // Wait for synchronization
    while (WDT->STATUS.bit.SYNCBUSY);
    
    if (early_warning < 3 || early_warning > 14) {
        // Configure early warning
        WDT->EWCTRL.bit.EWOFFSET = early_warning - 3;
        WDT->INTENSET.bit.EW = 1;
    } else if (early_warning != 0) {
        // Early warning is invalid
        return 1;
    }
    
    // Enable WDT
    WDT->CTRL.reg = WDT_CTRL_ENABLE;
    // Wait for synchronization
    while (WDT->STATUS.bit.SYNCBUSY);
    
    return 0;
}

uint8_t init_wdt_window(uint32_t core_clock_mask, uint8_t closed, uint8_t open,
                        uint8_t early_warning)
{
    // Ensure that WDT is disabled before changing configuration registers
    WDT->CTRL.bit.ENABLE = 0;
    
    // Check that window is valid
    if ((open < 3 || open > 14) || (closed < 3 || closed > 14)) {
        return 1;
    }
    // Configure window
    WDT->CONFIG.reg = WDT_CONFIG_PER(open) | WDT_CONFIG_WINDOW(closed);
    // Wait for synchronization
    while (WDT->STATUS.bit.SYNCBUSY);
    
    if (early_warning < 3 || early_warning > 14) {
        // Configure early warning
        WDT->EWCTRL.bit.EWOFFSET = early_warning - 3;
        WDT->INTENSET.bit.EW = 1;
    } else if (early_warning != 0) {
        // Early warning is invalid
        return 1;
    }
    
    // Enable WDT with window mode
    WDT->CTRL.reg = WDT_CTRL_ENABLE | WDT_CTRL_WEN;
    // Wait for synchronization
    while (WDT->STATUS.bit.SYNCBUSY);
    
    return 0;
}




void init_bod33_continuous(enum bod33_level level, uint8_t hysteresis)
{
    // Wait for syncronization
    while (SYSCTRL->PCLKSR.bit.B33SRDY);
    // Configure BOD33
    SYSCTRL->BOD33.reg = (SYSCTRL_BOD33_LEVEL(level) |
                          SYSCTRL_BOD33_RUNSTDBY |
                          SYSCTRL_BOD33_ACTION_RESET |
                          (!!hysteresis << SYSCTRL_BOD33_HYST_Pos) |
                          SYSCTRL_BOD33_ENABLE);
}

void init_bod33_sampling(enum bod33_level level, uint8_t hysteresis,
                         uint8_t prescaler)
{
    // Wait for syncronization
    while (SYSCTRL->PCLKSR.bit.B33SRDY);
    // Configure BOD33
    SYSCTRL->BOD33.reg = (SYSCTRL_BOD33_LEVEL(level) |
                          SYSCTRL_BOD33_PSEL(prescaler) |
                          SYSCTRL_BOD33_CEN |
                          SYSCTRL_BOD33_MODE |
                          SYSCTRL_BOD33_RUNSTDBY |
                          SYSCTRL_BOD33_ACTION_RESET |
                          (!!hysteresis << SYSCTRL_BOD33_HYST_Pos) |
                          SYSCTRL_BOD33_ENABLE);
}
