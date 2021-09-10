//file name: adc.h
//author   : Arsalan Syed

#ifndef adc_same54_h
#define adc_same54_h

#include "global.h"

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
 * Read internal temperature sensor.
 *
 * @param adcSel Which ADC should be used to read the temperature (0 or 1)
 *
 * @return Temperature
 */
extern int16_t adc_get_temp (uint8_t adcSel);

#endif /* adc_same54_h */
