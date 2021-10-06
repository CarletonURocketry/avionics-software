//file name: adc-same54.h
//author   : Arsalan Syed

#ifndef adc_same54_h
#define adc_same54_h

#include "global.h"


/**
* Checks enabled ADCs to see if they are done scanning through all the channels
* that they have been assigned. If they are, then set the ADCs so that they
* start scanning those channels again.
*/
extern void adc_service(void);


/**
 *  Initilize and start automatic ADC sampling at a fixed period.
 *
 *  @param clock_mask Bitmask for the Generic Clock Generator to be used by the
 *                    ADC and, if applicable, the TC
 *  @param clock_freq Frequency of the Generic Clock Generator
 *  @param channel_mask Mask for desired ADC channels, must not be 0
 *  @param sweep_period The time in milliseconds between sweeps, if 0 sweeps
 *                      sweeps will happen as fast as possible
 *  @param max_source_impedance Maximum impedence of source, see figure 37-5
 *                              in SAMD21 datasheet
 *  @param dma_chan The DMA channel to be used, if negative DMA will not be used
 *                  and the ADC result will be read via an interrupt
 *  @param adcSel Which ADC should be initialized (0 or 1)
 *
 *  @return 0 if ADC initilized successfully
 */
extern int init_adc (uint32_t clock_mask, uint32_t clock_freq,
                     uint32_t channel_mask, uint32_t sweep_period,
                     uint32_t max_source_impedance, int8_t dma_chan,
                     uint8_t adcSel);

/**
 * Read internal temperature sensor, return value in degrees celcius.
 *
 * @param adcSel Which ADC should be used to read the temperature (0 or 1)
 *
 * @return Temperature
 */
extern int16_t adc_get_temp (uint8_t adcSel);

/**
* return the latest reading from a select ADC channel. (the ADC continually
* cycles through its channels, reading values, and updating the internal
* buffer that the values are stored in as it moves along)
*
* @param channel a 8 bit value that represents the channel that should be read.
*                The notation is slightly unconventional because the 7th bit
                 defines which ADC to read from. For example:
                 if channel = 0b1000 0010
                 that means, read from ADC 1, channel 2
                 if channel = 0b1000 0100
                 that means read from ADC 1, channel 4
                 if channel = 0b0000 1000
                 that means read from ADC 0, channel 8

*/
extern uint16_t adc_get_value (uint8_t channel);

/**
* get the most recent ADC reading of the battery's voltage.
*/
extern int16_t adc_get_bat_vcc (void);

/**
*get the most recent ADC reading of the internal bandgap filter's voltage
*/
extern int16_t adc_get_bandgap_vcc (void);

/**
* get the most recent ADC reading of the Digital to analog converter
*/
extern int16_t adc_get_DAC_val (void);

#endif /* adc_same54_h */
