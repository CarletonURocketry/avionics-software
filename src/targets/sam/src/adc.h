/**
 * @file adc.h
 * @desc ADC Driver
 * @author Samuel Dewan
 * @date 2019-04-19
 * Last Author:
 * Last Edited On:
 */

#ifndef adc_h
#define adc_h

#include "global.h"

#if defined(SAMD2x)
#include "adc-samd21.h"
#elif defined(SAMx5x)
#include "adc-same54.h"
#endif

/**
 *  Function to be called in each iteration of the main loop.
 */
extern void adc_service (void);

/**
 *  Get the measured value for an ADC channel.
 *
 *  @param channel The channel number of which the value should be retrieved,
 *                 see board.h for a list of channel that can be read from,
 *                 starting from ANALOG_A0.
 *
 *  @return The measured value of the channel, ranging from 0 to 65535
 */
extern uint16_t adc_get_value (uint8_t channel);

/**
 *  Get the measured value for an ADC channel in millivolts.
 *
 *  @param channel The channel number of which the value should be retrieved
 *
 *  @return The measured value of the channel in millivolts, from 0 to 1000
 */
extern uint16_t adc_get_value_millivolts (uint8_t channel);

/**
 *  Get the measured value for an ADC channel in nanovolts.
 *
 *  @param channel The channel number of which the value should be retrieved
 *
 *  @return The measured value of the channel in nanovolts, from 0 to 1000000000
 */
extern uint32_t adc_get_value_nanovolts (uint8_t channel);

/**
 *  Get the temperature from the internal temperature sensor without
 *  compensating for internal reference voltage.
 *
 *  @return The measured temperature in hundredths of a degree celsius, will
 *          return INT16_MIN if ADC temperature sensor channel is not enabled
 */
extern int16_t adc_get_temp_course (void);

/**
 *  Get the temperature from the internal temperature sensor after
 *  compensating for internal reference voltage.
 *
 *  @return The measured temperature in hundredths of a degree celsius, will
 *          return INT16_MIN if ADC temperature sensor channel is not enabled
 */
extern int16_t adc_get_temp_fine (void);

/**
 *  Get the measured core voltage.
 *
 *  @return The core voltage in millivolts, will return INT16_MIN if ADC core
 *          voltage channel is not enabled
 */
extern int16_t adc_get_core_vcc (void);

/**
 *  Get the measured IO voltage.
 *
 *  @return The IO voltage in millivolts, will return INT16_MIN if ADC IO
 *          voltage channel is not enabled
 */
extern int16_t adc_get_io_vcc (void);

/**
 *  Get the last time at which an ADC sweep was completed.
 *
 *  @return The millis value when the last sweep completed
 */
extern uint32_t adc_get_last_sweep_time (void);

/**
 *  Get the mask of enabled ADC channels.
 *
 *  @return The channel mask
 */
extern uint64_t adc_get_channel_mask (void);

#endif /* adc_h */
