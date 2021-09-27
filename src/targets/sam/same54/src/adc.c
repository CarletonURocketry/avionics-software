//TODO: update document so that tab = 4 spaces (as per CUINSPACE coding guidelines)

#include <adc.h>

#include <stdint.h>

#define CHANNEL_RANGE 16
#define ADC_IRQ_PRIORITY 4


 struct {
   //these will hold the latest readings from the input pins (internal and external)
    uint16_t adc_input_buffer[2][16 + 7]; //16 external pins, 7 internal measurement sources

    union {
        uint8_t dma_chan;
        struct {
            uint8_t chan_num;
            uint8_t last_chan;
        };
    };

    uint8_t use_dma:1;
} adc_state_g;

DmacDescriptor ADC_DMA_Descriptors[4];

enum ADC_DMA_descriptor_names{
    //DMA descriptor that describes how the DMA should move information from the
    //results register of the ADC into their respective locations in main memory
    ADCx_DMA_results_to_buffer_desc,

    //DMA descriptor that will describe how the DMA should move information from
    //the main memory into the DSEQ_DATA register associated with the ADC. everytime
    //the ADC is done making a measurement the DSEQ_DATA is moved into the ADC's
    //configuration registers, which give the ADC a new target to get a Measurement
    //from.
    ADCx_DMA_buffer_to_DSEQ_DATA_desc,
    };

typedef struct{

  uint8_t num:5;
  uint8_t port:2;

} pin_t;

uint32_t ADC_measurement_sources[16 + 7] = { //16 input pins, 7 internal measurement sources
  //external sources
  ADC_INPUTCTRL_MUXPOS_AIN0,
  ADC_INPUTCTRL_MUXPOS_AIN1,
  ADC_INPUTCTRL_MUXPOS_AIN3,
  ADC_INPUTCTRL_MUXPOS_AIN4,
  ADC_INPUTCTRL_MUXPOS_AIN5,
  ADC_INPUTCTRL_MUXPOS_AIN6,
  ADC_INPUTCTRL_MUXPOS_AIN7,
  ADC_INPUTCTRL_MUXPOS_AIN8,
  ADC_INPUTCTRL_MUXPOS_AIN9,
  ADC_INPUTCTRL_MUXPOS_AIN10,
  ADC_INPUTCTRL_MUXPOS_AIN11,
  ADC_INPUTCTRL_MUXPOS_AIN12,
  ADC_INPUTCTRL_MUXPOS_AIN13,
  ADC_INPUTCTRL_MUXPOS_AIN14,
  ADC_INPUTCTRL_MUXPOS_AIN15,

  //internal sources
  ADC_INPUTCTRL_MUXPOS_SCALEDCOREVCC,
  ADC_INPUTCTRL_MUXPOS_SCALEDVBAT,
  ADC_INPUTCTRL_MUXPOS_SCALEDIOVCC,
  ADC_INPUTCTRL_MUXPOS_BANDGAP,
  ADC_INPUTCTRL_MUXPOS_PTAT,  //temperature sensor (not reliable)
  ADC_INPUTCTRL_MUXPOS_CTAT,  //another temperature sensor (not reliable)

  //final write to ADC->INPUTCTRL needs to set DSEQSTOP so that we stop using the
  //DMA after ADC_INPUTCTRL_MUXPOS_DAC is read from
  ADC_INPUTCTRL_MUXPOS_DAC | (1 << ADC_INPUTCTRL_DSEQSTOP_Pos)

 };





static const pin_t adc_pins[2][16] = {

  /*NOTE: although both ADC0 and ADC1 have inputs with the same names, that
  * does not mean that they both read from the same pin. Ex. AIN0 for ADC0 is
  * connected to pin 2(port A ) and AIN0 for ADC1 is connected to pin 8(port B).
  *
  * on the other hand, both ADCs are connected to the same internal sources
  * ex. there's only two internal temprature sensors, and they're both connected
  * to both ADCs
  */

  //this array contains information for each pin that the *ADC0* uses as inputs.
  {
  {.port = 0, .num = 2}, // ADC1/AIN[0]
  {.port = 0, .num = 3}, // ADC1/AIN[1]
  {.port = 1, .num = 8}, // ADC1/AIN[2]
  {.port = 1, .num = 9}, // ADC1/AIN[3]
  {.port = 0, .num = 4}, // ADC1/AIN[4]
  {.port = 0, .num = 5}, // ADC1/AIN[5]
  {.port = 0, .num = 6}, // ADC1/AIN[6]
  {.port = 0, .num = 7}, // ADC1/AIN[7]
  {.port = 0, .num = 8}, // ADC1/AIN[8]
  {.port = 0, .num = 9}, // ADC1/AIN[9]
  {.port = 0, .num = 10},//ADC1/AIN[10]
  {.port = 0, .num = 11},//ADC1/AIN[11]
  {.port = 1, .num = 0}, //ADC1/AIN[12]
  {.port = 1, .num = 1}, //ADC1/AIN[13]
  {.port = 1, .num = 2}, //ADC1/AIN[14]
  {.port = 1, .num = 3}, //ADC1/AIN[15]
},

  //this array contains information for each pin that the *ADC1* can use as inputs
  {
  {.port = 1, .num = 8}, // ADC1/AIN[0]
  {.port = 1, .num = 9}, // ADC1/AIN[1]
  {.port = 0, .num = 8}, // ADC1/AIN[2]
  {.port = 0, .num = 9}, // ADC1/AIN[3]
  {.port = 2, .num = 2}, // ADC1/AIN[4]
  {.port = 2, .num = 3}, // ADC1/AIN[5]
  {.port = 1, .num = 4}, // ADC1/AIN[6]
  {.port = 1, .num = 5}, // ADC1/AIN[7]
  {.port = 1, .num = 6}, // ADC1/AIN[8]
  {.port = 1, .num = 7}, // ADC1/AIN[9]
  {.port = 2, .num = 0}, //ADC1/AIN[10]
  {.port = 2, .num = 1}, //ADC1/AIN[11]
  {.port = 2, .num = 30},//ADC1/AIN[12]
  {.port = 2, .num = 31},//ADC1/AIN[13]
  {.port = 3, .num = 0}, //ADC1/AIN[14]
  {.port = 3, .num = 1}, //ADC1/AIN[15]
}


};

/*
  specifies which pin the ADC should read, read from left to right
*/
//static const int ADC_Descriptor[] = {
//  1,2,3,5
//};



static void adcx_set_pmux(uint8_t channel, uint8_t adc_sel){
  //select the pin from the pin_t struct defined above
  struct pin_t pin = adc_pins[adc_sel][ channel];

  //set the alternative function for the pin (in this case, input for ADC)
  if(pin.num % 2 == 0){
    PORT->Group[pin.port].PMUX[pin.num / 2].bit.PMUXE = 0x1 ;
    }
  else {
    PORT->Group[pin.port].PMUX[pin.num / 2].bit.PMUXO = 0x1;
      }

  //enable the alternative function (input for the ADC)
  PORT->Group[pin.port].PINCFG[pin.num].bit.PMUXEN = 0x1;
}


void adcx_service(void){
  //check to see if if the ADCs are done scanning through all their channels,
  //and if so, then reset the configuration so that they scan through the
  //channels again.

  //check both ADCs
  for(int adcSel = 0; adcSel <= 1; adcSel++){
    Adc* ADCx = (adcSel == 1)? ADC1: ADC0;

    //DMA is still sequencing, therefore, the configuration doesn't need to be reset
    if(ADCx->INPUTCTRL.bit.DSEQSTOP == 0)
        return;
    else{ //DMA sequencing has halted, therefore, need to reset configuration

        //set the destination address to be the start of the adc_input_buffer array
        ADC_DMA_Descriptors[adcSel*2 + ADCx_DMA_results_to_buffer_desc]->DSTADDR.reg = adc_state_g.adc_input_buffer[adcSel];

        //set the source address to point to the start of the list of measurement sources again
        ADC_DMA_Descriptors[adcSel*2 + ADCx_DMA_buffer_to_DSEQ_DATA_desc]->SRCADDR.reg = &ADC_measurement_sources;

        //stop the stop on DMA sequencing...meaning, start DMA sequencing!
        ADCx->INPUTCTRL.bit.DSEQSTOP = 0;

    }
  }

}

int init_adc(uint32_t clock_mask, uint32_t clock_freq,
             uint32_t channel_mask, uint32_t sweep_period,
             uint32_t max_source_impedance, int8_t* dma_chans,
             uint8_t adcSel){


  if (!channel_mask){
    //give up if no channels are enabled
    return 1;
  }

  //configure all enabled channels as analog inputs
  for(int i = 0; i < CHANNEL_RANGE; i++){
    if(channel_mask & (0x1<<i)){
      adcx_set_pmux(i, adcSel);
    }
  }

//----create a pointer to the ADC that we're configuring---//
Adc* ADCx = (adcSel == 1)? ADC1: ADC0;

#if 0
//----setting up the core clock and the bus clock-----//
    //max sampling rate = 1MSPS
    //sampling rate = CLK_ADC/ (N_sampling + offset + N_data)
      //N_sampling = sampling duration in clk_adc cycles
      //offset = offset compensation in clck_adc cycles
      //N_data = bit resolution

      // therefore, at 12 bit resolution, 0 offset, and sampling time =1 clck_adc
      //CLK_ADC = 13 MHz

      //since minimum prescaler for ADC is 2, we need a clock source that
      //runs at 26 MHz or slower...

      //we will use a 48 MHz source (DFLL48M) and divide by 4 to get 12 MHz.
      //this frequency is sourced by the DFFL in open-loop mode


    //generator side of code starts here//

    //select the generator to use
    uint8_t generator_n = 0x1;

    //pick the source for the generator we want to use for the core clock
    uint8_t src = GCLK_GENCTRL_SRC_DFLL_Val;

    //run the DFFL in open-loop mode
    OSCCTRL->DFLLCTRLB.bit.MODE |= 0x1;

    //enable the clock Source to run only when requested by the ADC
    OSCCTRL->DFLLCTRLA.bit.EMABLE = (uint8_t)(0xFF);

    //select how we want to divide the frequency of the generator's clock freq
    //if using the second generator, max div = 2^16, else max div = 2^8
    uint8_t div =  0x0; //don't divide

    //This decides if we want to divide by DIV or 2^DIV
    uint8_t divsel = 0x0; //don't divide

    //enable the generator
    uint8_t genen = 0x01;

    uint32_t generatorConf = 0x0|
                             (src)|
                             (div << 16)|
                             (divsel <<12)|
                             (genen <<8);

    GCLK->GENCTRL[generator_n].reg = generatorConf;

    //wait for synchronization
    while(GCLK->SYNCBUSY.bit.GENCTRL & (uint16_t)(1<< (generator_n) + 2));
#endif

    //generator side of code ends here /peripheral side of code starts here---//


    // specify the clock generator we want to use for the core clock
    GCLK->PCHCTRL[40 + adcSel].reg = GCLK_PCHCTRL_CHEN | clock_mask;


    //enable the APB clock
    if(adcSel == 0){
        MCLK->APBDMASK.bit.ADC0_= 0x1;}
    else {
        MCLK->APBDMASK.bit.ADC1_= 0x1;
    }

    // Reset ADC
    ADCx->CTRLA.bit.SWRST = 1;
    while (ADCx->CTRLA.bit.SWRST || ADCx->SYNCBUSY.bit.SWRST);


#if 0
    // Disabling prescaller selection for now, should be fine as long as we use
    // a slow enough generic clock


    //select the prescaler for the ADC clock
    //prescaler should maximize the freq of the ADC core clock without exceeding
    //the limit, which is: (ref: )
    // therefore: prescaler = clock_freq/ADC_CLOCK_MAX
    //
    //some prescaler values and the code that specifies them:
    // | prescaler | Code        |
    // |  2        |  0x0        |
    // |  4        |  0x1        |
    // |  8        |  0x2        |
    // |  n        |  log2(n) -1 |

    //therefore, we want to use log2(n) -1 to calculate the code we need to use
    //to specify the prescaler that we want (prescaler is n)

    //in C, log2(n) = 31 - __builtin_clz(n)
    //therefore, prescaler = 31 - __builtin_clz(n) -1
    // prescaler = 30 - __builtin_clz(n)

    //we want to make sure the prescaler does not exceed 256 (max division)
    //therefore prescaler = prescaler & 0x7;
    uint8_t prescaler = 30 - __builtin_clz(clock_freq/ADC_CLOCK_MAX);
    prescaler &= 0x7;

    ADCx->CTRLA.bit.PRESCALER = prescaler;
#endif


//----setting up the reference value----//
    //we decide to create some latency for the reference to reach it's final value
    ADCx->REFCTRL.bit.REFCOMP = 0x1;

    //wait for Synchronization
    while(ADCx->SYNCBUSY.bit.REFCTRL == 0b1);

    //We select the internal bandgap voltage as the reference
    ADCx->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTREF;

    //wait for Synchronization
    while(ADCx->SYNCBUSY.bit.REFCTRL == 0b1);

    //set the internal bandgap voltage to 1v
    SUPC->VREF.bit.SEL &= 0;

    //enable the output of the voltage reference from the SUPC
    SUPC->VREF.bit.VREFOE = 0x1;


    //Sampling Time Configuration//
    //by setting REFCOMP we can only sample for one clock cycle
    ADCx->SAMPCTRL.bit.SAMPLEN = 0x1;
    //wait for Synchronization
    while(ADCx->SYNCBUSY.bit.SAMPCTRL == 0x1);

//----setting up the ADC Resolution----//
    //hwe want to accumolate 1024 samples, and take the average value.
    ADCx->AVGCTRL.bit.SAMPLENUM = 0xA;
    //wait for Synchronization
    while(ADCx->SYNCBUSY.bit.AVGCTRL == 0x1);

    //how many right shifts do we need to make? 4 for >= 16 samples
    ADCx->AVGCTRL.bit.ADJRES = 0x04;
    //wait for Synchronization
    while(ADCx->SYNCBUSY.bit.AVGCTRL == 0x1);

    //setting output resolution to 16 bits
    ADCx->CTRLB.bit.RESSEL = 0x1;
    //wait for Synchronization
    while(ADCx->SYNCBUSY.bit.CTRLB == 0x1);

  //setting collection to be in single-ended mode//
    ADCx->INPUTCTRL.bit.DIFFMODE = 0x0;
    //wait for synchronization
    while(ADCx->SYNCBUSY.bit.INPUTCTRL == 0x1);

//----Setting up DMA or interrupt----//
if((dma_chan >=0) && (dma_chan < DMAC_CH_NUM)){

  //the DMAcDescriptors need to be configured

  dma_config_desc(ADC_DMA_descriptor_names[ADCx_DMA_results_to_buffer_desc],      //descriptor to be configured
                  dma_width[DMA_WIDTH_HALF_WORD],                                 //width of dma transcation (16 bits)
                  ADC0->RESULT.reg,                                               //source to read from
                  0,                                                              //should the source be incremented? 0 = No
                  adc_state_g.adc_input_buffer[adcSel],                           //destination to write data
                  1,                                                              //should the destination be incremented? 1 = yes
                  1,                                                              //number of beats to send per transaction
                  NULL);                                                          //the next descriptor (used for burst and block transfers)


dma_config_desc(ADC_DMA_descriptor_names[ADC0_DMA_buffer_to_DSEQ_DATA_desc],      //descriptor to be configured
                 dma_width[DMA_WIDTH_WORD],                                       //width of dma transcation (32 bits) <- DSEQ_DATA only accepts 32 bit access
                 &ADC_measurement_sources,                                        //source to read from
                 0,                                                               //should the source be incremented? 1 = yes
                 ADCx->DSEQDATA.reg,                                              //destination to write data
                 0,                                                               //should the destination be incremented? 0 = No
                 1,                                                               //number of beats to send per transaction
                 NULL);                                                           //the next descriptor (used for burst and block transfers)


/*configure and enable the DMA channels that will be using the descriptors configured above*/

  enum trigger_source{
    ADCx_RESRDY = (0x44 + adcSel*2),
    ADCx_DSEQ
  };


 /*note the priorities! The same task on different ADCs should have the same priority
   so that preemption doesn't occur! */
  dma_config_transfer(dma_chans[0],                                                                 //channel that is to be used
                      ADC_DMA_descriptors[ADCx_DMA_results_to_buffer_desc]->BTCRL.bit.BEATSIZE,     //beatsize
                      ADC_DMA_Descriptors[ADCx_DMA_results_to_buffer_desc]->SRCADDR.reg,            //source (where will data be read from)
                      ADC_DMA_Descriptors[ADCx_DMA_results_to_buffer_desc]->BTCTRL.bit.SRCINC,      //should the source be incremented everytime?
                      ADC_DMA_Descriptors[ADCx_DMA_results_to_buffer_desc]->DSTADDR.reg,            //destination (where to send data)
                      ADC_DMA_Descriptors[ADCx_DMA_results_to_buffer_desc]->BTCTRL.bit.DSTINC,      //should the destination be incremented?
                      ADCx_RESRDY                                                                   // trigger source: what starts the DMA seq?
                      0,                                                                            // priority
                      NULL);                                                                        // next descriptor


  dma_config_transfer(dma_chans[2],                                                                 //channel that is to be used
                      ADC_DMA_descriptors[ADC0_DMA_buffer_to_DSEQ_DATA_desc]->BTCRL.bit.BEATSIZE,   //beatsize
                      ADC_DMA_Descriptors[ADC0_DMA_buffer_to_DSEQ_DATA_desc]->SRCADDR.reg.,         //source (where will data be read from)
                      ADC_DMA_Descriptors[ADC0_DMA_buffer_to_DSEQ_DATA_desc]->BTCTRL.bit.SRCINC,    //should the source be incremented everytime?
                      ADC_DMA_Descriptors[ADC0_DMA_buffer_to_DSEQ_DATA_desc]->DSTADDR.reg,          //destination (where to send data)
                      ADC_DMA_Descriptors[ADC0_DMA_buffer_to_DSEQ_DATA_desc]->BTCTRL.bit.DSTINC,    //should the destination be incremented?
                      ADCx_DSEQ,                                                                    // trigger source: what starts the DMA seq?
                      1,                                                                            // priority
                      NULL);                                                                        // next descriptor




    ADCx->DSEQCTRL.bit.INPUTCTRL = 0x1;      //DMA sequencing should only update the 'input control' registers. This also enables DMASequencing


    ADCx->DSEQCTRL.bit.AUTOSTART = 0x1;  //enable auto_start on the ADC (so that it starts scanning right after it gets new tasks from the DMA)



}else{
  //enable interrupt that tells us when results are ready to be ready
  ADCx->INTENSET.bit.RESRDY = 1;

  //enable interrupt in NVIC
//  NVIC_SetPriority(ADC_IRQn, ADC_IRQ_PRIORITY);
//  NVIC_EnableIRQ(ADC_IRQn);
}

    //enable the ADC
    ADCx->CTRLA.bit.ENABLE = 0x1;

    //wait for Synchronization
    while (ADCx->SYNCBUSY.bit.ENABLE == 0x1);

    return 0;
}

//dma_start_buffer_to_static_word
//more like dma_configure

static inline void adc_start_scan(void){

}




static uint16_t adcx_start_single_scan(uint8_t target, uint8_t adcSel){
    //---confirm that the ADC that we're using has been initialized----//

    //----decide which ADC to use----//
    Adc* ADCx = (adcSel == 0)? ADC0 : ADC1;

    //set the input to the ADC
    ADCx->INPUTCTRL.bit.MUXPOS = target;

    //set ADC to run in single conversion mode
    ADCx->CTRLB.bit.FREERUN =0x0;

    //wait for synchronization
    while (ADCx->SYNCBUSY.bit.CTRLB == 0x1);

    //start the ADC
    ADCx->SWTRIG.bit.START = 0x1;

    //wait for result
    while(ADCx->INTFLAG.bit.RESRDY == 0x0);

    //read the result
    uint16_t result = ADCx->RESULT.reg;

    return result;

}

static float convert_to_dec(uint8_t val){
  //decimal parts of a numbers are given as 4 bit values for SAME54. They
  //need to be converted to an actual decimal that can be used.
  //ex. val = 16, output = 0.16
  //ex  val = 8,  output = 0.8
  //note: this function is used for the decimal part of the NVM Calibration
  //values for the temperature sensors. These decimal parts are all 4 bits
  //long, and so, there is no way that 'val' could be more than 15.
  val = (float)val;
  if(val <= 10)
    return val/10.0f;
  else
    return val/100.0f;
}

int16_t adc_get_temp (uint8_t adcSel){
    /*NOTE: according to the errata, section 2.23.1:
    * 'Both internal temperature sensors, TSENSP and TSENSC, are not supported and should not be used.'
    * therefore, this reading will either be inaccurate/wildly incorrect.
    */

    //----enable the temperature sensors----//
    //If ONDEMAND == 0 then you cannot enable the bandgap reference and the
    //temperature sensors at the same time.
    SUPC->VREF.bit.ONDEMAND = 0x1;

    //enable the temperature sensors
    SUPC->VREF.bit.TSEN = 0x1;

    //----get the value measured by each temperature sensor----//.. These should be removed after verifying that the function works
    //read TSENSP
    uint16_t TP = adcx_start_single_scan(ADC_INPUTCTRL_MUXPOS_PTAT_Val, adcSel);
    //read TSENSC
    uint16_t TC = adcx_start_single_scan(ADC_INPUTCTRL_MUXPOS_CTAT_Val, adcSel);

    //----Calculate the temperature----//
    //the formula for calculating the temperature based on the readings of the two
    //temperature sensors can be found on the datasheet,
    //in section 45.6.3.1: Device Temperature Measurement


    //extracting TL Calibration value
    uint32_t TL_Calibration_Val_Int_part = ((*(uint32_t *)FUSES_ROOM_TEMP_VAL_INT_ADDR)  & FUSES_ROOM_TEMP_VAL_INT_Msk) >> FUSES_ROOM_TEMP_VAL_INT_Pos;
    uint32_t TL_Calibration_Val_Dec_part = ((*(uint32_t *)FUSES_ROOM_TEMP_VAL_DEC_ADDR)  & FUSES_ROOM_TEMP_VAL_DEC_Msk) >> FUSES_ROOM_TEMP_VAL_DEC_Pos;
    float  TL = TL_Calibration_Val_Int_part + convert_to_dec(TL_Calibration_Val_Dec_part);

    //extracting the TH calbiration value
    uint32_t TH_Calibration_Val_Int_Part = (*(uint32_t *)FUSES_HOT_TEMP_VAL_INT_ADDR & FUSES_HOT_TEMP_VAL_INT_Msk) >> FUSES_HOT_TEMP_VAL_INT_Pos;
    uint32_t TH_Calibration_Val_Dec_Part = (*(uint32_t *)FUSES_HOT_TEMP_VAL_DEC_ADDR & FUSES_HOT_TEMP_VAL_DEC_Msk) >> FUSES_HOT_TEMP_VAL_DEC_Pos;
    float TH = TH_Calibration_Val_Int_Part + convert_to_dec(TH_Calibration_Val_Dec_Part);

    //extracting VPL
    uint16_t VPL = (*(uint32_t *)FUSES_ROOM_ADC_VAL_PTAT_ADDR & FUSES_ROOM_ADC_VAL_PTAT_Msk) >> FUSES_ROOM_ADC_VAL_PTAT_Pos;

    //extracting PVH
    uint16_t VPH = (*(uint32_t *)FUSES_HOT_ADC_VAL_PTAT_ADDR & FUSES_HOT_ADC_VAL_PTAT_Msk) >> FUSES_HOT_ADC_VAL_PTAT_Pos;

    //extracting VCL
    uint16_t VCL = (*(uint32_t *)FUSES_ROOM_ADC_VAL_CTAT_ADDR & FUSES_ROOM_ADC_VAL_CTAT_Msk) >> FUSES_ROOM_ADC_VAL_CTAT_Pos;

    //extracting VCH
    uint16_t VCH = (*(uint32_t *)FUSES_HOT_ADC_VAL_CTAT_ADDR & FUSES_HOT_ADC_VAL_CTAT_Msk) >> FUSES_HOT_ADC_VAL_CTAT_Pos;


    uint32_t temp_numerator = (TL * VPH * TC)
                            - (VPL * TH * TC)
                            - (TL * VCH * TP)
                            + (TH * VCL * TP);

    uint16_t temp_denominator = (VCL * TP)
                              - (VCH * TP)
                              - (VPL * TC)
                              + (VPH * TC);

    uint16_t temperature = temp_numerator/temp_denominator;

    return temperature;
}

uint16_t adc_get_value (uint8_t channel){
  //get the latest value of the selected channels
  uint8_t  adcSel = !!(channel & (1 << 7));
    channel &= ~(1 << 7);

  //channel number must be between 0 and ADC_INPUTCTRL_MUXPOS_PTC_Val, adcSel must be 0 or 1
  if((adcSel > 1) || channel > ADC_INPUTCTRL_MUXPOS_PTC_Val || channel < 0)
        return 0;

  //user is attempting to access value from input pins
  if(channel <= ADC_INPUTCTRL_MUXPOS_AIN15)
    return adc_state_g.adc_input_buffer[adcSel][channel];

  //user is attemping to access value from internal measurement sources
  else if(channel > ADC_INPUTCTRL_MUXPOS_AIN15)
    return adc_state_g.adc_input_buffer[adcSel][channel - 8];
}

uint16_t adc_get_value_millivolts (uint8_t channel){
//get the voltage of the selected channel in millivolts
  uint16_t temp_val = adc_get_value(channel);
  return ((1000 * (uint32_t)temp_val) / 65535);
}

uint32_t adc_get_value_nanovolts (uint8_t channel){
//get the value of the channel in nanovolts
  uint16_t temp_val = adc_get_value(channel);
  return ((1000000000 * (uint64_t)temp_val) / 65535);
}

int16_t adc_get_core_vcc (void){
//get the voltage value of the core.
//this value needs to be multiplied by 4 since the value we get from the adc is
//scaled by 1/4. The returned value must be stored in a 32 bit word because
//the value retrieved could be 0xFFFF, which, when multiplied by 4 would overflow
      uint16_t const val = adc_get_value(ADC_INPUTCTRL_MUXPOS_SCALEDCOREVCC);
      return (int16_t)((4000 * (uint32_t)val) / 65535);
}

int16_t adc_get_io_vcc (void){
  //get the voltage value of the input/output.
  //this value needs to be multiplied by 4 since the value we get from the adc is
  //scaled by 1/4. The returned value must be stored in a 32 bit word because
  //the value retrieved could be 0xFFFF, which, when multiplied by 4 would overflow
      uint16_t const val = adc_get_value(ADC_INPUTCTRL_MUXPOS_SCALEDIOVCC);
      return (int16_t)((4000 * (uint32_t)val) / 65535);
}

int16_t adc_get_bat_vcc (void){
  //get the latest voltage value of VBAT
    uint16_t const val = adc_get_value(ADC_INPUTCTRL_MUXPOS_SCALEDVBAT);
    return (int16_t)((4000 * (uint32_t)val) / 65535);
}


int16_t adc_get_bandgap_vcc (void){
//get the latest voltage value of the badgap
  return adc_get_value_millivolts(ADC_INPUTCTRL_MUXPOS_BANDGAP);
}

int16_t adc_get_DAC_val (void){
//get the latest voltage value of the Digital-to-A
  return adc_get_value_millivolts(ADC_INPUTCTRL_MUXPOS_DAC);
}




//questions for Sam:
  //the generic clock is asynchronous to the bus clock - it will need to be
  //synchronized to write certain registers - what's going on here
  //what prescaler should we use for the adc clock
  //which generator should we use for the clock..source, etc
  //can we configure the adc ref and sample size separately?
  //selecting the prescaler??
