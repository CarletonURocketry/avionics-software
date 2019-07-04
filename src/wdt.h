/**
 * @file wdt.h
 * @desc Watchdog timer and brown out dector driver
 * @author Samuel Dewan
 * @date 2019-04-10
 * Last Author:
 * Last Edited On:
 */

#ifndef wdt_h
#define wdt_h

#include "global.h"

/**
 *  Initilize Watchdog Timer.
 *
 *  @param core_clock_mask The mask for the Generic Clock Generator to be used
 *                         for the Watchdog Timer
 *  @param timeout The time out value for the WDT, the timer will count to
 *                 2^(timeout - 3) clock cycles before reset, timeout must be at
 *                 least 3 and at most 14
 *  @param early_warning Amount of time from the start of the watchdog timeout
 *                       period to the early warning interupt, timer will count
 *                       to 2^(timeout - 3) clock cycles before interrupt,
 *                       early_warning must be at least 3 and at most 14, if the
 *                       value is 0 the interrupt will be disabled
 *  @return 0 if the WDT was successfully started
 */
extern uint8_t init_wdt(uint32_t core_clock_mask, uint8_t timeout,
                     uint8_t early_warning);

/**
 *  Initilize Watchdog Timer in window mode.
 *
 *  @param core_clock_mask The mask for the Generic Clock Generator to be used
 *                         for the Watchdog Timer
 *  @param closed The window closed time for the WDT, the timer will count to
 *                2^(closed - 3) clock cycles before the window opens, closed
 *                value must be at east 3 and at most 14
 *  @param open The window open time for the WDT, the timer will count to
 *              2^(open - 3) clock cycles before reset, closed value must be at
 *              least 3 and at most 14
 *  @param early_warning Amount of time from the start of the watchdog timeout
 *                       period to the early warning interupt, timer will count
 *                       to 2^(early_warning - 3) clock cycles before interrupt,
 *                       early_warning must be at least 3 and at most 14, if the
 *                       value is 0 the interrupt will be disabled
 *  @return 0 if the WDT was successfully started
 */
extern uint8_t init_wdt_window(uint32_t core_clock_mask, uint8_t closed,
                            uint8_t open, uint8_t early_warning);

/**
 *  Pat the Watchdog Timer
 *
 *  @note If the watchdog timer is being syncronized (probably because it is
 *        currently in the process of being patted) it will not be cleared
 */
static inline void wdt_pat(void)
{
    if (!WDT->STATUS.bit.SYNCBUSY) {
        WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
    }
}


/**
 *  BOD33 level values
 *  See datasheet section 37.11.3 - Brown-Out Detectors Characteristics
 */
enum bod33_level {
    /** 1.64 V negative, 1.715 V positive */
    BOD33_LVL_1640 = 6,
    /** 1.675 V negative, 1.750 V positive */
    BOD33_LVL_1675 = 7,
    /** 2.77 V negative, 2.84 V positive */
    BOD33_LVL_2770 = 39,
    /** 3.07 V negative, 3.2 V positive */
    BOD33_LVL_3070 = 48
};

/**
 *  Enable the 3.3 V brown out detector in continuous mode.
 *
 *  @param level The voltage level below which the CPU should be reset
 *  @param hysteresis If non-zero, hysteresis will be enabled
 */
extern void init_bod33_continuous(enum bod33_level level, uint8_t hysteresis);

/**
 *  Enable the 3.3 V brown out detector in sampling mode.
 *
 *  @param level The voltage level below which the CPU should be reset
 *  @param hysteresis If non-zero, hysteresis will be enabled
 *  @param prescaler Division factor for sampling clock
 */
extern void init_bod33_sampling(enum bod33_level level, uint8_t hysteresis,
                                uint8_t prescaler);

#endif /* wdt_h */
