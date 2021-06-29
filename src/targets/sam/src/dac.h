/**
 * @file dac.h
 * @desc DAC Driver
 * @author Samuel Dewan
 * @date 2019-08-28
 * Last Author:
 * Last Edited On:
 */

#ifndef dac_h
#define dac_h

#include "global.h"

#if defined(SAMD2x)
#define DAC_NUM_CHANNELS    1
#elif defined(SAMx5x)
#define DAC_NUM_CHANNELS    DAC_CHANNEL_SIZE
#else
#define DAC_NUM_CHANNELS    0
#endif

enum dac_reference {
    /* Internal 1.0 volt reference */
    DAC_REF_1V,
    /* Analog VCC */
    DAC_REF_AVCC
};

/**
 *  Initilize DAC.
 *
 *  @param clock_mask Bitmask for the Generic Clock Generator to be used by the
 *                    DAC
 *  @param reference Voltage reference to be used by DAC
 *  @param channel_mask Bitmask to select channels to enable
 *  @param enable_int_output Whether the internal output should be enabled
 *  @param enable_ext_output Whether the external output should be enabled
 */
extern void init_dac (uint32_t clock_mask, enum dac_reference reference,
                      uint8_t channel_mask, uint8_t enable_int_output,
                      uint8_t enable_ext_output);

/**
 *  Set the DACs output voltage.
 *
 *  @param chan The channel to set the output voltage for
 *  @param value The value to be output by the DAC
 *
 *  Vout = (value / (2^16 - 1)) * Vref
 *
 *  @note Though a 16 bit value is used, DAC only has 10 bit precision
 */
extern void dac_set (uint8_t chan, uint16_t value);

/**
 *  Set the DACs output voltage in millivolts.
 *
 *  @param chan The channel to set the output voltage for
 *  @param millivolts The voltage to be output by the DAC
 *
 *  @note If the target voltage is greater than the reference voltage the
 *        reference voltage will be output
 */
extern void dac_set_millivolts (uint8_t chan, uint16_t millivolts);

/**
 *  Get the current value of the DAC.
 *
 *  @param chan The channel to get the output voltage for
 *
 *  @return The current value of the DAC, ranges from 0 to 2^16 - 1
 */
extern uint16_t dac_get_value (uint8_t chan);

/**
 *  Get the current value of the DAC in millivolts.
 *
 *  @param chan The channel to get the output voltage for
 *
 *  @return the current output voltage of the DAC in millivolts
 */
extern uint16_t dac_get_value_millivolts (uint8_t chan);

#endif /* dac_h */
