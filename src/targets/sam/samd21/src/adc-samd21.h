/**
 * @file adc.c
 * @desc ADC Driver functions for SAMD21
 * @author Samuel Dewan
 * @date 2021-11-09
 * Last Author:
 * Last Edited On:
 */


#ifndef adc_samd21_h
#define adc_samd21_h

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
 *  @param dma_chan The DMA channel to be used, if negative DMA will not be used
 *                  and the ADC result will be read via an interrupt
 *  @param max_source_impedance Maximum impedence of source, see figure 37-5
 *                              in SAMD21 datasheet
 *
 *  @return 0 if ADC initilized successfully
 */
extern uint8_t init_adc (uint32_t clock_mask, uint32_t clock_freq,
                         uint32_t channel_mask, uint32_t sweep_period,
                         uint32_t max_source_impedance, int8_t dma_chan);

#endif /* adc_samd21_h */
