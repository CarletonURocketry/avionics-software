#include <adc.h>

#include <stdint.h>

#define CHANNEL_RANGE 16
#define ADC_IRQ_PRIORITY 4


 struct {
   //these will hold the latest readings from the input pins (internal and external)
    uint16_t adc_in_buffer_pins[2][16];
    uint16_t adc_in_buffer_internal[7];

    union {
        uint8_t dma_chan;
        struct {
            uint8_t chan_num;
            uint8_t last_chan;
        };
    };

    uint8_t use_dma:1;
} adc_state_g;



typedef struct pin_t {

  uint8_t num:5;
  uint8_t port:2;

} pin_t;



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
static const int ADC_Descriptor[] = {
  1,2,3,5
};



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



int init_adc(uint32_t clock_mask, uint32_t clock_freq,
             uint32_t channel_mask, uint32_t sweep_period,
             uint32_t max_source_impedance, int8_t dma_chan,
             uint8_t adcSel){

  uint8_t generator_n = 1;

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
  //enable conversions triggered by end of sequencing
  ADCx->DSEQCTRL.bit.AUTOSTART = 0x1;

  //indicate which register(s) we want to update
  ADCx->DSEQCTRL.bit.INPUTCTRL = 1;

}else{
  //enable interrupt that tells us when results are ready to be ready
  ADCx->ADC_INTENSET.bit.RESRDY = 1;

  //enable interrupt in NVIC
//  NVIC_SetPriority(ADC_IRQn, ADC_IRQ_PRIORITY);
//  NVIC_EnableIRQ(ADC_IRQn);
}

    return 0;
}

//dma_start_buffer_to_static_word
//more like dma_configure

static inline void adc_start_scan(void){

}

/*
void adc_service(uint8_t ADC_sel){


    //enable the ADC//
    ADC->CTRLA.bit.ENABLE = 0b1;
    //wait for synchronization;
    while(ADC->SYNCBUSY.bit.ENABLE == 1);


  //enable the ADC

  //wait for synchronization

  //start next scan

}
 */


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

    //enable the ADC
    ADCx->CTRLA.bit.ENABLE = 0x1;

    //wait for Synchronization
    while (ADCx->SYNCBUSY.bit.ENABLE == 0x1);

    //start the ADC
    ADCx->SWTRIG.bit.START = 0x1;

    //wait for result
    while(ADCx->INTFLAG.bit.RESRDY == 0x0);

    //read the result
    uint16_t result = ADCx->RESULT.reg;

    //disable the ADC
    ADCx->CTRLA.bit.ENABLE = 0x0;

    //wait for Synchronization
    while (ADCx->SYNCBUSY.bit.ENABLE == 0x1);

    return result;

}

static float convert_to_dec(uint val){
  //decimal parts of a numbers are given as 4 bit values for SAME54. They
  //need to be converted to an actual decimal that can be used.
  //ex. val = 16, output = 0.16
  //ex  val = 8,  output = 0.8
  val = float(val);
  if(val <= 10)
    return val/10.0;
  else if (val <= 100)
    return val/100.0;

}

static int16_t adc_get_temp (uint8_t adcSel){
    //NOTE: READ table 54-24 for timing...according to page 1455

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
    float  TL = TL_Calibration_Val_Int_Part + convert_to_dec(TL_Calibration_Val_Dec_Part);

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

    uint32_t temp_denominator = (VCL * TP)
                              - (VCH * TP)
                              - (VPL * TC)
                              + (VPH * TC);

    uint16_t temperature = (uint16_t)(temp_numerator/temp_denominator);

    return temperature;
}

extern uint16_t adc_get_value (uint8_t adcSel, uint8_t channel){
  //get the latest value of the selected channels

  //channel number must be between 0 and ADC_INPUTCTRL_MUXPOS_PTC_Val, adcSel must be 0 or 1
  if(channel < 0 || (adcSel > 1 || adcSel < 0 || channel > ADC_INPUTCTRL_MUXPOS_PTC_Val)
    return 0;

  //user is attemping to retrieve value from input pins
  if(channel <= ADC_INPUTCTRL_MUXPOS_AIN15_Val ){
    return adc_state_g.adc_in_buffer_pins[adcSel][channel];
  }
  //user is attemping to retrieve value from internal source
  else{
    return adc_state_g.adc_in_buffer_internal[channel];
  }

}

extern uint16_t adc_get_value_millivolts (uint8_t adcSel, uint8_t channel){
//get the voltage of the selected channel in millivolts
  uint16_t temp_val = adc_get_value(adcSel, channel);
  return (1000 * (uint32_t)temp_val/65535);
}

extern uint16_t adc_get_value_nanovolts (uint8_t adcSel channel){
//get the value of the channel in nanovolts
  uint16_t temp_val = adc_get_value(adcSel, channel);
  return (1000000000 * (uint64_t)temp_val/65535);
}

extern int32_t adc_get_core_vcc (void){
//get the voltage value of the core.
//this value needs to be multiplied by 4 since the value we get from the adc is
//scaled by 1/4. The returned value must be stored in a 32 bit word because
//the value retrieved could be 0xFFFF, which, when multiplied by 4 would overflow
  return 4 * (uint32_t)(adc_get_value(0, ADC_INPUTCTRL_MUXPOS_SCALEDCOREVCC));
}

extern int32_t adc_get_io_vcc (void){
  //get the voltage value of the input/output.
  //this value needs to be multiplied by 4 since the value we get from the adc is
  //scaled by 1/4. The returned value must be stored in a 32 bit word because
  //the value retrieved could be 0xFFFF, which, when multiplied by 4 would overflow
  return 4 * (uint32_t)(adc_get_value(0, ADC_INPUTCTRL_MUXPOS_SCALEDIOVCC));

}

extern int16_t adc_get_bat_vcc (void){
  //get the latest voltage value of VBAT
 return 4 * (uint32_t)(adc_get_value(0, ADC_INPUTCTRL_MUXPOS_SCALEDVBAT));
}


extern int16_t adc_get_bandgap_vcc (void){
//get the latest voltage value of the badgap
  return adc_get_value(0, ADC_INPUTCTRL_MUXPOS_BANDGAP);
}

extern int16_t adc_get_DAC_val (void){
//get the latest voltage value of the Digital-to-A
  return adc_get_value(0, ADC_INPUTCTRL_MUXPOS_DAC);
}




//questions for Sam:
  //the generic clock is asynchronous to the bus clock - it will need to be
  //synchronized to write certain registers - what's going on here
  //what prescaler should we use for the adc clock
  //which generator should we use for the clock..source, etc
  //can we configure the adc ref and sample size separately?
  //selecting the prescaler??
