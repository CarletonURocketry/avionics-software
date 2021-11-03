//file name: adc-same54.h
//author   : Arsalan Syed

#include "global.h"

#ifndef adc_same54_h
#define adc_same54_h

#define ADC0_EXTERNAL_ANALOG_INPUT_0_MASK                                 (1<<0)
#define ADC0_EXTERNAL_ANALOG_INPUT_1_MASK                                 (1<<1)
#define ADC0_EXTERNAL_ANALOG_INPUT_2_MASK                                 (1<<2)
#define ADC0_EXTERNAL_ANALOG_INPUT_3_MASK                                 (1<<3)
#define ADC0_EXTERNAL_ANALOG_INPUT_4_MASK                                 (1<<4)
#define ADC0_EXTERNAL_ANALOG_INPUT_5_MASK                                 (1<<5)
#define ADC0_EXTERNAL_ANALOG_INPUT_6_MASK                                 (1<<6)
#define ADC0_EXTERNAL_ANALOG_INPUT_7_MASK                                 (1<<7)
#define ADC0_EXTERNAL_ANALOG_INPUT_8_MASK                                 (1<<8)
#define ADC0_EXTERNAL_ANALOG_INPUT_9_MASK                                 (1<<9)
#define ADC0_EXTERNAL_ANALOG_INPUT_10_MASK                               (1<<10)
#define ADC0_EXTERNAL_ANALOG_INPUT_11_MASK                               (1<<11)
#define ADC0_EXTERNAL_ANALOG_INPUT_12_MASK                               (1<<12)
#define ADC0_EXTERNAL_ANALOG_INPUT_13_MASK                               (1<<13)
#define ADC0_EXTERNAL_ANALOG_INPUT_14_MASK                               (1<<14)
#define ADC0_EXTERNAL_ANALOG_INPUT_15_MASK                               (1<<15)

#define ADC1_EXTERNAL_ANALOG_INPUT_0_MASK                                (1<<16)
#define ADC1_EXTERNAL_ANALOG_INPUT_1_MASK                                (1<<17)
#define ADC1_EXTERNAL_ANALOG_INPUT_2_MASK                                (1<<18)
#define ADC1_EXTERNAL_ANALOG_INPUT_3_MASK                                (1<<19)
#define ADC1_EXTERNAL_ANALOG_INPUT_4_MASK                                (1<<20)
#define ADC1_EXTERNAL_ANALOG_INPUT_5_MASK                                (1<<21)
#define ADC1_EXTERNAL_ANALOG_INPUT_6_MASK                                (1<<22)
#define ADC1_EXTERNAL_ANALOG_INPUT_7_MASK                                (1<<23)
#define ADC1_EXTERNAL_ANALOG_INPUT_8_MASK                                (1<<24)
#define ADC1_EXTERNAL_ANALOG_INPUT_9_MASK                                (1<<25)
#define ADC1_EXTERNAL_ANALOG_INPUT_10_MASK                               (1<<26)
#define ADC1_EXTERNAL_ANALOG_INPUT_11_MASK                               (1<<27)
#define ADC1_EXTERNAL_ANALOG_INPUT_12_MASK                               (1<<28)
#define ADC1_EXTERNAL_ANALOG_INPUT_13_MASK                               (1<<29)
#define ADC1_EXTERNAL_ANALOG_INPUT_14_MASK                               (1<<30)
#define ADC1_EXTERNAL_ANALOG_INPUT_15_MASK                               (1<<31)

/*note these are categorized ad ADCX because either ADC0 or ADC1 can read
  these sources. By default, we use ADC0, but that may change in future
  iterations.*/
#define ADCX_INTERNAL_INPUT_SCALED_CORE_VCC_MASK                         (1<<32)
#define ADCX_INTERNAL_INPUT_SCALED_VBATT_SUPPLY_MASK                     (1<<33)
#define ADCX_INTERNAL_INPUT_SCALED_CORE_VCC_MASK                         (1<<34)
#define ADCX_INTERNAL_INPUT_SCALED_IOVCC_MASK                            (1<<35)
#define ADCX_INTERNAL_INPUT_BANDGAP_MASK                                 (1<<36)
#define ADCX_INTERNAL_INPUT_TEMP_SENSOR_PTAT_MASK                        (1<<37)
#define ADCX_INTERNAL_INPUT_TEMP_SENSOR_CTAT_MASK                        (1<<38)
#define ADCX_INTERNAL_INPUT_DAC_MASK                                     (1<<39)

/*these are used by the user to specify which channel they want to read
  ex. ADC_get_value(ADC_CHAN_0)*/
#define ADC_CHAN_0                             ADC0_EXTERNAL_ANALOG_INPUT_0_MASK
#define ADC_CHAN_1                             ADC0_EXTERNAL_ANALOG_INPUT_1_MASK
#define ADC_CHAN_2                             ADC0_EXTERNAL_ANALOG_INPUT_2_MASK
#define ADC_CHAN_3                             ADC0_EXTERNAL_ANALOG_INPUT_3_MASK
#define ADC_CHAN_4                             ADC0_EXTERNAL_ANALOG_INPUT_4_MASK
#define ADC_CHAN_5                             ADC0_EXTERNAL_ANALOG_INPUT_5_MASK
#define ADC_CHAN_6                             ADC0_EXTERNAL_ANALOG_INPUT_6_MASK
#define ADC_CHAN_7                             ADC0_EXTERNAL_ANALOG_INPUT_7_MASK
#define ADC_CHAN_8                             ADC0_EXTERNAL_ANALOG_INPUT_8_MASK
#define ADC_CHAN_9                             ADC0_EXTERNAL_ANALOG_INPUT_9_MASK
#define ADC_CHAN_10                           ADC0_EXTERNAL_ANALOG_INPUT_10_MASK
#define ADC_CHAN_11                           ADC0_EXTERNAL_ANALOG_INPUT_11_MASK
#define ADC_CHAN_12                           ADC0_EXTERNAL_ANALOG_INPUT_12_MASK
#define ADC_CHAN_13                           ADC0_EXTERNAL_ANALOG_INPUT_13_MASK
#define ADC_CHAN_14                           ADC0_EXTERNAL_ANALOG_INPUT_14_MASK
#define ADC_CHAN_15                           ADC0_EXTERNAL_ANALOG_INPUT_15_MASK
#define ADC_CHAN_16                            ADC1_EXTERNAL_ANALOG_INPUT_0_MASK
#define ADC_CHAN_17                            ADC1_EXTERNAL_ANALOG_INPUT_1_MASK
#define ADC_CHAN_18                            ADC1_EXTERNAL_ANALOG_INPUT_2_MASK
#define ADC_CHAN_19                            ADC1_EXTERNAL_ANALOG_INPUT_3_MASK
#define ADC_CHAN_20                            ADC1_EXTERNAL_ANALOG_INPUT_4_MASK
#define ADC_CHAN_21                            ADC1_EXTERNAL_ANALOG_INPUT_5_MASK
#define ADC_CHAN_22                            ADC1_EXTERNAL_ANALOG_INPUT_6_MASK
#define ADC_CHAN_23                            ADC1_EXTERNAL_ANALOG_INPUT_7_MASK
#define ADC_CHAN_24                            ADC1_EXTERNAL_ANALOG_INPUT_8_MASK
#define ADC_CHAN_25                            ADC1_EXTERNAL_ANALOG_INPUT_9_MASK
#define ADC_CHAN_26                           ADC1_EXTERNAL_ANALOG_INPUT_10_MASK
#define ADC_CHAN_27                           ADC1_EXTERNAL_ANALOG_INPUT_11_MASK
#define ADC_CHAN_28                           ADC1_EXTERNAL_ANALOG_INPUT_12_MASK
#define ADC_CHAN_29                           ADC1_EXTERNAL_ANALOG_INPUT_13_MASK
#define ADC_CHAN_30                           ADC1_EXTERNAL_ANALOG_INPUT_14_MASK
#define ADC_CHAN_31                           ADC1_EXTERNAL_ANALOG_INPUT_15_MASK
#define ADC_CHAN_CORE_VCC                    ADCX_INTERNAL_INPUT_SCALED_CORE_VCC
#define ADC_CHAN_SCALED_VBATT_SUPPLY     ADCX_INTERNAL_INPUT_SCALED_VBATT_SUPPLY
#define ADC_CHAN_SCALED_CORE_VCC             ADCX_INTERNAL_INPUT_SCALED_CORE_VCC
#define ADC_CHAN_SCALED_IOVCC                   ADCX_INTERNAL_INPUT_SCALED_IOVCC
#define ADC_CHAN_BANDGAP                             ADCX_INTERNAL_INPUT_BANDGAP
#define ADC_CHAN_TEMP_SENSOR_PTAT           ADCX_INTERNAL_INPUT_TEMP_SENSOR_PTAT
#define ADC_CHAN_TEMP_SENSOR_PTAT           ADCX_INTERNAL_INPUT_TEMP_SENSOR_CTAT
#define ADC_CHAN_DAC                                     ADCX_INTERNAL_INPUT_DAC


#define NUM_OF_ADC_PIN_SRCS                                                   16
#define NUM_OF_ADC_INTERNAL_SRCS                                               7
#define ADCX_NUM_OF_CHANS        (NUM_OF_ADC_PIN_SRCS + NUM_OF_ADC_INTERNAL_SRS)
#define TOTAL_NUM_CHANS                                   (2* ADCX_NUM_OF_CHANS)

//all external analog pins associated with ADC0 and ADC1 (32 in total)
#define ADC_EXTERNAL_ANALOG_MASK                                      0xffffffff
/*all internal sources that the adc designated with reading internal soures
 *must read*/
#define INTERNAL_CHANNEL_MASK    0xffff << (32 + (16 * ADC_INTERNAL_SRC_READER))

#define SAM_E5X_PORT_A                                                         0
#define SAM_E5X_PORT_B                                                         1
#define SAM_E5X_PORT_C                                                         2
#define SAM_E5X_PORT_D                                                         3

#define ADC_IRQ_PRIORITY                                                       4
#define ADC0_DMA_RES_TO_BUFFER_PRIORITY                                        1
#define ADC0_DMA_BUFFER_TO_DSEQDATA_PRIORITY                                   0
#define ADC1_DMA_RES_TO_BUFFER_PRIORITY                                        1
#define ADC1_DMA_BUFFER_TO_DSEQDATA_PRIORITY                                   0


#define ADC_CHAN(a, c)          (c + (a * 16))
#define ADC0_CHAN(c)            ADC_CHAN(0, c)
#define ADC1_CHAN(c)            ADC_CHAN(1, c)



#define ADC_CHAN_GET_PMUX(c)    ((c <= 16) ? c : (c - 16))
#define ADC_INTERNAL_MASK(adc, chan) (1 << ((chan - 0x18) + 32 + (adc * 16)))


#define AVGCTRL_SETTING (ADC_AVGCTRL_SAMPLENUM_1024 | ADC_AVGCTRL_ADJRES(0))


const struct adc_dseq_source ADC_measurement_srcs[] = {
    //external sources
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN0,
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN1,
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN2,
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN3,
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN4,
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN5,
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN6,
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN7,
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN8,
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN9,
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN10,
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN11,
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN12,
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN13,
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN14,
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN15,
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    //internal sources
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_SCALEDCOREVCC,
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_SCALEDVBAT,
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_SCALEDIOVCC,
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_BANDGAP,
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_PTAT, //temp sensor (not reliable)
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_CTAT, //temp sensor (not reliable)
      .AVRGCTRL.reg = AVGCTRL_SETTING },

    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_DAC,
      .AVRGCTRL.reg = AVGCTRL_SETTING }

 };


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
 *  @param DMA_res_to_buff_chan The DMA channel to be used for storing results,
 *                              if negative DMA will not be used and the ADC
 *                              result will be read via an interrupt
 *  @param DMA_buff_to_DMASEQ_chan The DMA channel to be used for sequencing,
 *                                 if negative DMA will not be used and the ADC
 *                                 result will be read via an interrupt
 *  @param adcSel Which ADC should be initialized (0 or 1)
 *
 *  @return 0 if ADC initilized successfully
 */
 extern int init_adc(uint32_t clock_mask, uint32_t clock_freq,
                     uint64_t channel_mask, uint32_t sweep_period,
                     uint32_t max_source_impedance, int8_t DMA_res_to_buff_chan,
                     int8_t DMA_buff_to_DMASEQ_chan);
  

extern int init_adc_helper(uint32_t clock_mask, uint32_t clock_freq,
                    uint32_t max_source_impedance, int8_t DMA_res_to_buff_chan,
                    int8_t DMA_buff_to_DMASEQ_chan, uint8_t adcSel)

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
