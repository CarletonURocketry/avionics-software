//TODO: update document so that tab = 4 spaces (as per CUINSPACE coding guidelines)
//TODO: update driver so that it doesn't need to scan through all channels (only the ones we're interested in!)
//TODO: implement ability to have interrupt driven operation if no DMA channel
//      is provided


#include <stdint.h>
#include "adc.h"
#include "dma.h"

#define NUM_OF_ADC_PIN_SRS            16
#define NUM_OF_ADC_INTERNAL_SRS       7
#define CHANNEL_RANGE (NUM_OF_ADC_PIN_SRS + NUM_OF_ADC_INTERNAL_SRS)

#define ADC_IRQ_PRIORITY                     4
#define ADC0_DMA_RES_TO_BUFFER_PRIORITY      1
#define ADC0_DMA_BUFFER_TO_DSEQDATA_PRIORITY 0
#define ADC1_DMA_RES_TO_BUFFER_PRIORITY      1
#define ADC1_DMA_BUFFER_TO_DSEQDATA_PRIORITY 0

#define ADC_SWEEP_PERIOD                     500 // milliseconds

//global variables to keep track of which DMA_Descriptors we're using.
//ex. ADCx_DMA_Desc_Results_to_buffer[0] == 10
//This means we're using the 10th descriptor, and the 10th channel to move
//results from the res register to the internal memory buffer
//
//
//DMA descriptor that describes how the DMA should move information from the
//results register of the ADC into their respective locations in main memory
uint8_t ADCx_DMA_desc_results_to_buffer[2] = {-1, -1};
//
//DMA descriptor that will describe how the DMA should move information from
//the main memory into the DSEQ_DATA register associated with the ADC. everytime
//the ADC is done making a measurement the DSEQ_DATA is moved into the ADC's
//configuration registers, which give the ADC a new target to get a Measurement
//from.
uint8_t ADCx_DMA_desc_buffer_to_DSEQ_DATA[2] = {-1, -1};

//these are all the descriptors that describe how the DMA channels will behave
//descriptor 1 is associated with channel 1, desc2 -> ch2 etc.
extern DmacDescriptor dmacDescriptors_g[DMAC_CH_NUM];

struct {
    /*these will hold the latest readings from
      the input pins (internal and external)
    */
    uint16_t adc_input_buffer[2][NUM_OF_ADC_PIN_SRS + NUM_OF_ADC_INTERNAL_SRS];
    uint32_t  sweep_period;
    uint32_t  last_sweep_time;

} adc_state_g;


struct pin_t {
    uint8_t num:5;
    uint8_t port:2;

};

struct adc_dseq_source {
    ADC_INPUTCTRL_Type INPUTCTRL;
    ADC_CTRLB_Type CTRLB;
};

const struct adc_dseq_source ADC_measurement_sources[] = {
    //external sources
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN0 },
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN1 },
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN2 },
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN3 },
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN4 },
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN5 },
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN6 },
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN7 },
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN8 },
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN9 },
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN10 },
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN11 },
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN12 },
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN13 },
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN14 },
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_AIN15 },

    //internal sources
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_SCALEDCOREVCC },
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_SCALEDVBAT },
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_SCALEDIOVCC },
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_BANDGAP },
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_PTAT },  //temperature sensor (not reliable)
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_CTAT },  //another temperature sensor (not reliable)

    //final write to ADC->INPUTCTRL needs to set DSEQSTOP so that we stop using the
    //DMA after ADC_INPUTCTRL_MUXPOS_DAC is read from
    { .INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_DAC | (1 << ADC_INPUTCTRL_DSEQSTOP_Pos) }

 };



static const struct pin_t adc_pins[2][16] = {

    /*NOTE: although both ADC0 and ADC1 have inputs with the same names, that
    * does not mean that they both read from the same pin. Ex. AIN0 for ADC0 is
    * connected to pin 2(port A) and AIN0 for ADC1 is connected to pin 8(port B)
    *
    * on the other hand, both ADCs are connected to the same internal sources
    * ex. there's only two internal temprature sensors, and they're both
    *connected to both ADCs
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
       {.port = 1, .num = 8}, // ADC1/AIN[0
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


void adc_service(void){
    //check to see if if the ADCs are done scanning through all their channels,
    //and if so, then reset the configuration so that they scan through the
    //channels again.

    //check to see if it is time to read the ADC value. If not, then return.
    if(millis <  (adc_state_g.last_sweep_time + adc_state_g.sweep_period)) {
        return;
    }

    //check both ADCs
    for(int adcSel = 0; adcSel <= 1; adcSel++){
        Adc* ADCx = (adcSel == 1)? ADC1: ADC0;


        //if the ADC hasn't been enabled, then don't service it
        if(ADCx->CTRLA.bit.ENABLE == 0x0){
            continue;
        }

        /*DMA is still sequencing, therefore, the configuration
        *doesn't need to be reset
        */
        if(ADCx->INPUTCTRL.bit.DSEQSTOP == 0){
            continue;

        }else{
            //DMA sequencing has halted, therefore, need to reset configuration
            //matching DMA channel with it's predetermined priority
            uint8_t results_to_buffer_priority

            if(adcSel == 1){
                result_to_buffer_priority = ADC1_DMA_RES_TO_BUFFER_PRIORITY;
            }else{
                result_to_buffer_priority = ADC0_DMA_RES_TO_BUFFER_PRIORITY;
            }


            //matching DMA transfer descriptor with trigger source
            uint8_t trig = (adcSel == 1) ? ADC1_DMAC_ID_RESRDY :
                                           ADC0_DMAC_ID_RESRDY;

            dma_config_transfer(ADCx_DMA_desc_results_to_buffer[adcSel],
                            //channel to be configured
                            DMA_WIDTH_HALF_WORD,
                            //width of dma transcation (16 bits)
                            &(ADCx->RESULT.reg),
                            //source to read from
                            0,
                            //should the source be incremented? 0 = No
                            &(adc_state_g.adc_input_buffer[adcSel]),
                            //destination to write data
                            1,
                            //should the destination be incremented? 1 = yes
                            NUM_OF_ADC_INTERNAL_SRS + NUM_OF_ADC_PIN_SRS,
                            //number of beats to send
                            trig,
                            //trigger_source
                            results_to_buffer_priority,
                            //priority
                            NULL);
                            //the next descriptor to link to this one



            //matching DMA channel with its priority
            uint8_t priority;
            if (adcSel== 1) {
                priority = ADC1_DMA_BUFFER_TO_DSEQDATA_PRIORITY;
            }else {
                priority = ADC0_DMA_BUFFER_TO_DSEQDATA_PRIORITY;
            }


            //matching DMA channel with its trigger source
            trig = (adcSel) ? ADC1_DMAC_ID_SEQ : ADC0_DMAC_ID_SEQ;

            dma_config_transfer(ADCx_DMA_desc_buffer_to_DSEQ_DATA[adcSel],
                                //channel to be configured
                                DMA_WIDTH_WORD,
                                /*width of dma transcation:
                                *!DSEQ_DATA only accepts 32 bit access!
                                */
                                &(ADC_measurement_sources),
                                //source to read from
                                1,
                                //should the source be incremented? 1 = yes
                                &(ADCx->DSEQDATA.reg),
                                //destination to write data
                                0,
                                //should the destination be incremented? 0 = No
                                NUM_OF_ADC_INTERNAL_SRS + NUM_OF_ADC_PIN_SRS,
                                //number of beats to send per transaction
                                trig,
                                //trigger source
                                priority,
                                //priority
                                NULL);
                                //the address of the next descriptor to link.


            //set the new time at which the sweep began.
            adc_state_g.last_sweep_time = millis;

            //stop the stop on DMA sequencing...meaning, start DMA sequencing!
            ADCx->INPUTCTRL.bit.DSEQSTOP = 0;
        }
    }
}



int init_adc(uint32_t clock_mask, uint32_t clock_freq,
             uint32_t channel_mask, uint32_t sweep_period,
             uint32_t max_source_impedance, int8_t DMA_res_to_buff_chan,
             int8_t DMA_buff_to_DMASEQ_chan, uint8_t adcSel){

    adc_state_g.sweep_period = MS_TO_MILLIS(1000);

    if (!channel_mask){
        //give up if no channels are enabled
        return 1;
    }

    //configure all enabled channels as analog inputs
    for(int i = 0; i < NUM_OF_ADC_PIN_SRS + NUM_OF_ADC_INTERNAL_SRS;i++){
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
    if((DMA_res_to_buff_chan >=0)  &&
       (DMA_res_to_buff_chan < DMAC_CH_NUM) &&
       (DMA_buff_to_DMASEQ_chan >=0) &&
       (DMA_buff_to_DMASEQ_chan < DMAC_CH_NUM)){


           init_ADCx_DMA(DMA_res_to_buff_chan, DMA_buff_to_DMASEQ_chan);
       }



}else{
    //enable interrupt that tells us when results are ready to be read
    ADCx->INTENSET.bit.RESRDY = 1;

    //enable interrupt in NVIC
    //NVIC_SetPriority(ADC_IRQn, ADC_IRQ_PRIORITY);
    //NVIC_EnableIRQ(ADC_IRQn);
}

    //enable the ADC
    ADCx->CTRLA.bit.ENABLE = 0x1;

    //wait for Synchronization
    while (ADCx->SYNCBUSY.bit.ENABLE == 0x1);

    return 0;


}


void init_ADCx_DMA(uint8_t DMA_res_to_buff_chan,
                   uint8_t DMA_buff_to_DMASEQ_chan,
                   uint8_t adcSel) {

    /*capturing the identifying number of the channel that the DMA will use
    * for transferring results of ADC scans into the buffer. We need this
    * channel number for adc_service().
    */
    ADCx_DMA_desc_results_to_buffer[adcSel] = DMA_res_to_buff_chan;

    /*capturing the identifying number of the channel that the DMA will use
    * for transferring DMASEQ data into the DMASEQ register. We need this
    * channel number for adc_service().
    */
    ADCx_DMA_desc_buffer_to_DSEQ_DATA[adcSel] = DMA_buff_to_DMASEQ_chan;

    //matching DMA channel with it's predetermined priority
    uint8_t results_to_buffer_priority;

     if(adcSel == 1){
         results_to_buffer_priority = ADC1_DMA_RES_TO_BUFFER_PRIORITY;
     } else {
         ADC0_DMA_RES_TO_BUFFER_PRIORITY;
     }


    //matching DMA transfer descriptor with trigger source
    uint8_t trig = (adcSel == 1)? ADC1_DMAC_ID_RESRDY : ADC0_DMAC_ID_RESRDY;


    dma_config_transfer(DMA_res_to_buff_chan,
                        //channel to be configured
                        DMA_WIDTH_HALF_WORD,
                        //width of dma transcation (16 bits)
                        &(ADCx->RESULT.reg),
                        //source to read from
                        0,
                        //should the source be incremented? 0 = No
                        &(adc_state_g.adc_input_buffer[adcSel]),
                        //destination to write data
                        1,
                        //should the destination be incremented? 1 = yes
                        NUM_OF_ADC_INTERNAL_SRS + NUM_OF_ADC_PIN_SRS,
                        //number of beats to send per transaction
                        trig,
                        //trigger_source
                        results_to_buffer_priority,
                        //priority
                        NULL);
                        //the next descriptor to link



    //matching DMA channel with its priority
    uint8_t priority = (adcSel== 1)? ADC1_DMA_BUFFER_TO_DSEQDATA_PRIORITY :
                                     ADC0_DMA_BUFFER_TO_DSEQDATA_PRIORITY;

    //matching DMA channel with its trigger source
    trig = (adcSel) ? ADC1_DMAC_ID_SEQ : ADC0_DMAC_ID_SEQ;

    dma_config_transfer(DMA_buff_to_DMASEQ_chan,
                        //channel to be configured
                        DMA_WIDTH_WORD,
                        /*width of dma transcation (32 bits):
                        * DSEQ_DATA only accepts 32 bit access
                        */
                        &(ADC_measurement_sources),
                        //source to read from
                        1,
                        //should the source be incremented? 1 = yes
                        &(ADCx->DSEQDATA.reg),
                        //destination to write data
                        0,
                        //should the destination be incremented? 0 = No
                        NUM_OF_ADC_INTERNAL_SRS + NUM_OF_ADC_PIN_SRS,
                        //number of beats to send per transaction
                        trig,
                        //trigger source
                        priority,
                        //priority
                        NULL);
                        //the next descriptor to link



    /*enable auto_start on the ADC so that it starts a new scan right after
    the DMA has moved the results out of the results register and into
    the internal buffer*/
    ADCx->DSEQCTRL.bit.AUTOSTART = 0x1;

    /*DMA sequencing should only update the 'input control' registers.
    This also enables DMASequencing!*/
    ADCx->DSEQCTRL.bit.INPUTCTRL = 0x1;

    //ADCx->SWTRIG.reg = ADC_SWTRIG_START;

    /* ADC will begin measuring sources after the first DMA descriptor is
    // moved to DMA_SEQ_DATA.
    // After that, autostart will take care of the rest
    */
    adc_state_g.last_sweep_time = millis;


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
    if(val <= 10){
        return val/10.0f;
    } else {
        return val/100.0f;
    }
}

int16_t adc_get_temp (uint8_t adcSel){
    /*NOTE: according to the errata, section 2.23.1:
    * 'Both internal temperature sensors, TSENSP and TSENSC, are not supported
    * and should not be used.therefore, this reading will either be
    * inaccurate/wildly incorrect
    */

    //----enable the temperature sensors----//
    //If ONDEMAND == 0 then you cannot enable the bandgap reference and the
    //temperature sensors at the same time.
    SUPC->VREF.bit.ONDEMAND = 0x1;

    //enable the temperature sensors
    SUPC->VREF.bit.TSEN = 0x1;

    //----get the value measured by each temperature sensor----//.
    //read TSENSP
    uint16_t TP = adcx_start_single_scan(ADC_INPUTCTRL_MUXPOS_PTAT_Val, adcSel);
    //read TSENSC
    uint16_t TC = adcx_start_single_scan(ADC_INPUTCTRL_MUXPOS_CTAT_Val, adcSel);

    //----Calculate the temperature----//
    //the formula for calculating the temperature based on the readings of the
    // two temperature sensors can be found on the datasheet,
    // in section 45.6.3.1: Device Temperature Measurement

    uint32_t TL_int;

    //extracting TL Calibration value
    TL_int = ((*(uint32_t *)FUSES_ROOM_TEMP_VAL_INT_ADDR)  & FUSES_ROOM_TEMP_VAL_INT_Msk);
    TL_int = TL_int >> FUSES_ROOM_TEMP_VAL_INT_Pos;
    uint32_t TL_Calibration_Val_Dec_part = ((*(uint32_t *)FUSES_ROOM_TEMP_VAL_DEC_ADDR)  & FUSES_ROOM_TEMP_VAL_DEC_Msk) >> FUSES_ROOM_TEMP_VAL_DEC_Pos;
    float  TL = TL_int + convert_to_dec(TL_Calibration_Val_Dec_part);

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

uint16_t adc_get_value (uint8_t channel)
{
  //get the latest value of the selected channels
  uint8_t  adcSel = !!(channel & (1 << 7));
    channel &= ~(1 << 7);

  //channel number must be between 0 and ADC_INPUTCTRL_MUXPOS_PTC_Val, adcSel must be 0 or 1
    if((adcSel > 1) || channel > ADC_INPUTCTRL_MUXPOS_PTC_Val) {
        return 0;
    }

    if(channel <= ADC_INPUTCTRL_MUXPOS_AIN15) {
        //user is attempting to access value from input pins
        return adc_state_g.adc_input_buffer[adcSel][channel];
    } else if(channel > ADC_INPUTCTRL_MUXPOS_AIN15) {
        //user is attemping to access value from internal measurement sources
        return adc_state_g.adc_input_buffer[adcSel][channel - 8];
    }

    return 0;
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
