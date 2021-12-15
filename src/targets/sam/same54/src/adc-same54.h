//file name: adc-same54.h
//author   : Arsalan Syed

#include "global.h"

#ifndef adc_same54_h
#define adc_same54_h


#define NUM_OF_ADC_PIN_SRCS                                                   16
#define NUM_OF_ADC_INTERNAL_SRCS                                               8
#define ADCX_NUM_OF_CHANS       (NUM_OF_ADC_PIN_SRCS + NUM_OF_ADC_INTERNAL_SRCS)
#define TOTAL_NUM_CHANS                                   (2* ADCX_NUM_OF_CHANS)

#define SAM_E5X_PORT_A                                                         0
#define SAM_E5X_PORT_B                                                         1
#define SAM_E5X_PORT_C                                                         2
#define SAM_E5X_PORT_D                                                         3

#define ADC_IRQ_PRIORITY                                                       4
#define ADC0_DMA_RES_TO_BUFFER_PRIORITY                                        1
#define ADC0_DMA_BUFFER_TO_DSEQDATA_PRIORITY                                   0
#define ADC1_DMA_RES_TO_BUFFER_PRIORITY                                        1
#define ADC1_DMA_BUFFER_TO_DSEQDATA_PRIORITY                                   0


#define ADC_CHAN(a, c)                                             (c + (a * 16))
#define ADC0_CHAN(c)                                               ADC_CHAN(0, c)
#define ADC1_CHAN(c)                                               ADC_CHAN(1, c)

#define ADC_INTERNAL_MASK(adc, chan)    (UINT64_C(1) << ((chan - 0x18) + 32 + (adc * 16)))

#define AVGCTRL_SETTING (ADC_AVGCTRL_SAMPLENUM_1024 | ADC_AVGCTRL_ADJRES(0))

typedef struct {
        uint32_t TLI: 8;
        uint32_t TLD: 4;
        uint32_t THI: 8;
        uint32_t THD: 4;
        uint32_t : 16; //reserved
        uint32_t VPL: 12;
        uint32_t VPH: 12;
        uint32_t VCL: 12;
        uint32_t VCH: 12;
} NVM_cal_val;
 

/**
 *  Initilize and start automatic ADC sampling at a fixed period.
 *
 *  @param clock_mask Bitmask for the Generic Clock Generator to be used by the
 *                    ADC and, if applicable, the TC
 * 
 *  @param clock_freq Frequency of the Generic Clock Generator
 * 
 *  @param channel_mask Mask for desired ADC channels, must not be 0
 * 
 *  @param sweep_period The time in milliseconds between sweeps, if 0 sweeps
 *                      sweeps will happen as fast as possible
 * 
 *  @param max_source_impedance Maximum impedence of source
 * 
 *  @param DMA_res_to_buff_chan A two member array specifying the DMA channels 
 *                              to be used for storing results, if negative DMA
 *                              will not be used and the ADC result will be read
 *                              via an interrupt
 * 
 *  @param DMA_buff_to_DMASEQ_chan a two member array that specifies the The DMA
 *                                 channels to be used for sequencing, if
 *                                 negative DMA will not be used and the ADC
 *                                 result will be read via an interrupt
 *  @param adcSel Which ADC should be initialized (0 or 1)
 *
 *  @return 0 if ADC initilized successfully
 */
 extern int init_adc(uint32_t clock_mask, uint32_t clock_freq,
                     uint64_t channel_mask, uint32_t sweep_period,
                     uint32_t max_source_impedance,
                     const int8_t* DMA_res_to_buff_chan,
                     const int8_t* DMA_buff_to_DMASEQ_chan);
  

/**
 * Read internal temperature sensor, return value in degrees celcius.
 *
 * @param adcSel Which ADC should be used to read the temperature (0 or 1)
 *
 * @return Temperature
 */


extern int16_t adc_get_temp (uint8_t adcSel);


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
