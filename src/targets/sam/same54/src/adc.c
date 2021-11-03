//TODO: update document so that tab = 4 spaces (as per CUINSPACE coding guidelines)

//TODO: implement ability to have interrupt driven operation if no DMA channel
//      is provided
//TODO: allow user to decide which registers should be updated by DSEQ


#include <stdint.h>
#include "adc.h"
#include "dma.h"
#include "adc-same54.h"

//---------------------------start of settings--------------------------------//
   //time between measurements of all targets (in milliseconds)
#define ADC_SWEEP_PERIOD                                                     500

   //should ADC0 or ADC1 read the internal sources? 0 = ADC0, 1 = ADC1
#define ADC_INTERNAL_SRC_READER                                                0

//---------------------------end of settings----------------------------------//

#define NUM_OF_ADC_PIN_SRCS                                                   16
#define NUM_OF_ADC_INTERNAL_SRCS                                               7
#define ADCX_NUM_OF_CHANS       (NUM_OF_ADC_PIN_SRCS + NUM_OF_ADC_INTERNAL_SRCS)
#define ADC_TOTAL_NUM_CHANS                               (2* ADCX_NUM_OF_CHANS)

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

/*an array for keeping track of the index within
  adc_state_g.adc_input_buffer that a channel's reading is stored in. For
  example, if channel 1 (adc0 analog input 1) is the only channel that is
  designated to be read then channel_results_storage[1] = 0 because
  adc_state_g.adc_input_buffer[0][0] is where the reading would be stored
 * */
int8_t adc_map_channel_to_storage_locaction[ADC_TOTAL_NUM_CHANS]; //chans 0 -39

struct {
    //which channels to read from
    uint64_t  channel_mask;

    //how many channels we're using for ADC0 and AND1
    uint8_t   chan_count[2];

    //how long to wait between scanning all targets
    uint32_t  sweep_period;

    //when was the last time we read  from all our targets?
    uint32_t  last_sweep_time;

    //tells us which ADCs are turned on (ex 0x01 = ADC0 is on, 0x02 - ADC1 is on
    uint8_t   adc_in_use_mask:2 ;

    /*these will hold the latest readings from
      the input pins (internal and external)
    */
    uint16_t adc_input_buffer[ADCX_NUM_OF_CHANS];
} adc_state_g;


struct pin_t {
    uint8_t num:5;
    uint8_t port:2;

};

struct adc_dseq_source {
    ADC_INPUTCTRL_Type INPUTCTRL;
    ADC_CTRLB_Type     CTRLB;
    ADC_REFCTRL_Type   REFCTRL;
    uint8_t            RESERVED;
    ADC_AVGCTRL_Type   AVRGCTRL;
    ADC_SAMPCTRL_Type  SAMPCTRL;
};

struct adc_dseq_source selected_measurement_srcs[2][ADC_TOTAL_NUM_CHANS];


static const struct pin_t adc_pins[2][16] = {

    /*NOTE: although both ADC0 and ADC1 have inputs with the same names, that
    * does not mean that they both read from the same pin. Ex. AIN0 for ADC0 is
    * connected to pin 2(port A) and AIN0 for ADC1 is connected to pin 8(port B)
    *
    * on the other hand, both ADCs are connected to the same internal sources
    * ex. there's only two internal temprature sensors, and they're both
    *connected to both ADCs
    */

      //this array contains information for pins that *ADC0* uses as inputs
    {
        {.port = SAM_E5X_PORT_A, .num = 2}, // ADC0/AIN[0]
        {.port = SAM_E5X_PORT_A, .num = 3}, // ADC0/AIN[1]
        {.port = SAM_E5X_PORT_B, .num = 8}, // ADC0/AIN[2]
        {.port = SAM_E5X_PORT_B, .num = 9}, // ADC0/AIN[3]
        {.port = SAM_E5X_PORT_A, .num = 4}, // ADC0/AIN[4]
        {.port = SAM_E5X_PORT_A, .num = 5}, // ADC0/AIN[5]
        {.port = SAM_E5X_PORT_A, .num = 6}, // ADC0/AIN[6]
        {.port = SAM_E5X_PORT_A, .num = 7}, // ADC0/AIN[7]
        {.port = SAM_E5X_PORT_A, .num = 8}, // ADC0/AIN[8]
        {.port = SAM_E5X_PORT_A, .num = 9}, // ADC0/AIN[9]
        {.port = SAM_E5X_PORT_A, .num = 10},//ADC0/AIN[10]
        {.port = SAM_E5X_PORT_A, .num = 11},//ADC0/AIN[11]
        {.port = SAM_E5X_PORT_B, .num = 0}, //ADC0/AIN[12]
        {.port = SAM_E5X_PORT_B, .num = 1}, //ADC0/AIN[13]
        {.port = SAM_E5X_PORT_B, .num = 2}, //ADC0/AIN[14]
        {.port = SAM_E5X_PORT_B, .num = 3}, //ADC0/AIN[15]
    },

      //this array contains information for pins that *ADC1* can use as inputs
    {
       {.port = SAM_E5X_PORT_B, .num = 8}, // ADC1/AIN[0]
       {.port = SAM_E5X_PORT_B, .num = 9}, // ADC1/AIN[1]
       {.port = SAM_E5X_PORT_A, .num = 8}, // ADC1/AIN[2]
       {.port = SAM_E5X_PORT_A, .num = 9}, // ADC1/AIN[3]
       {.port = SAM_E5X_PORT_C, .num = 2}, // ADC1/AIN[4]
       {.port = SAM_E5X_PORT_C, .num = 3}, // ADC1/AIN[5]
       {.port = SAM_E5X_PORT_B, .num = 4}, // ADC1/AIN[6]
       {.port = SAM_E5X_PORT_B, .num = 5}, // ADC1/AIN[7]
       {.port = SAM_E5X_PORT_B, .num = 6}, // ADC1/AIN[8]
       {.port = SAM_E5X_PORT_B, .num = 7}, // ADC1/AIN[9]
       {.port = SAM_E5X_PORT_C, .num = 0}, //ADC1/AIN[10]
       {.port = SAM_E5X_PORT_C, .num = 1}, //ADC1/AIN[11]
       {.port = SAM_E5X_PORT_C, .num = 30},//ADC1/AIN[12]
       {.port = SAM_E5X_PORT_C, .num = 31},//ADC1/AIN[13]
       {.port = SAM_E5X_PORT_D, .num = 0}, //ADC1/AIN[14]
       {.port = SAM_E5X_PORT_D, .num = 1}, //ADC1/AIN[15]
    }


};

//TODO: this is broken
uint8_t adc_ain_chan_get_adc(uint8_t channel){
    /*note: this function only gives accurate results 
     *after init_adc() has been called*/

    if(channel < 0 || channel > ADC_TOTAL_NUM_CHANS){
        return 2; //error
    }

    //trying to access internal channel
    if(channel > 31){
        if(adc_state_g.channel_mask & (1 <<channel)){
            return 0; //ADC0
            }

        if(adc_state_g.channel_mask & (1<<chanel + 16)){
            return 1; //ADC1
            }
        return 3; //error: chan not activated

    if(channel > 16){
        return 1; //ADC1
    }

    return 0; //ADC0
}

uint8_t adc_chan_get_storage_key(uint8_t chan){
    /*adc_state_g.adc_input_buffer[] is where we store all readings from
     *the adc. This function returns the index within adc_input_buffer
      that we've saved a particular channel's readings in*/
    return adc_map_channel_to_storage_locaction[chan];
}

void adc_map_chan_to_storage_location(uint_t chan, uint8_t location){
    /*adc_state_g.adc_input_buffer[] is where we store all readings from
     *the adc. This function stores the index within adc_input_buffer
      that we store the reading. Note this only works if the DMA is 
      configured. See how this function is used in the DMA_init() 
      function.
      
      ex.
      adc_map_channel_to_storage_location[ chan = 1, location = 4];
      
      now we know that the readings from channel 1 are stored in 
      index 4 of adc_state_g.adc_input_buffer[]
      
    */
    adc_map_channel_to_storage_location[channel] = location;
}

static void adcx_set_pmux(struct pin_t pin){
    
    //set the alternative function for the pin (in this case, input for ADC)
    if(pin.num % 2 == 0){
        PORT->Group[pin.port].PMUX[pin.num / 2].bit.PMUXE = 0x1 ;
    }else {
        PORT->Group[pin.port].PMUX[pin.num / 2].bit.PMUXO = 0x1;
    }

    //enable the alternative function (input for the ADC)
    PORT->Group[pin.port].PINCFG[pin.num].bit.PMUXEN = 0x1;
}



static void init_ADCx_DMA(uint8_t DMA_res_to_buff_chan,
                          uint8_t DMA_buff_to_DMASEQ_chan,
                          uint8_t adcSel) {


    /*determine which ADC the DMA should be configured for*/
    Adc* ADCx = (adcSel == 1)? ADC1: ADC0;

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
        results_to_buffer_priority = ADC0_DMA_RES_TO_BUFFER_PRIORITY;
    }

/*-- creating a list of measurement sources that the the DMA will transfer----//
//---into the DMASEQDATA register of the ADC after each transaction. In other-//
//---words, we're creating a list of measurement targets for the ADC to-------//
//---automatically read from -------------------------------------------------*/

     /* initialize: all channel's values are stored
      * at index -1 within adc_state_g.input_buffer[] */
     for(int i = 0; i < ADC_TOTAL_NUM_CHANS; i++){
         adc_chan_get_storage_key[i] = -1;
        }

     uint64_t channel_mask_temp = adc_state_g.channel_mask;

     /* calculating the value that needs to be written into the INPUTCTRL 
      * register to signal that we want to read from AIN[0]. We start with 
      * this input becuase all other inputs AIN[x] are signalled value x*/
     uint8_t i = 16 * adcSel;

     /*check 16 channels because each ADC module only has 16 analog inputs*/
     uint8_t stop_index = i + 16;

     /*next available index free for storage in adc_state_g.adc_input_buffer*/
     uint8_t j = 0;
    
     /*the first 16 bits in the channel mask represent the analog inputs for
      *ADC0 and the next 16 represent the analog inputs for ADC1. Therefore,
      *based on the ADC module we're setting up, we need to pay attention to
      *a different group of 16 bits in the channel mask*/
     uint32_t adc_external_analog_mask = 0xffff << (adcSel * 16);

     while((channel_mask_temp & adc_external_analog_mask) && (i < stop_index)){

         if(channel_mask_temp & (1 <<i)){

             /*add the measurement source
              *note i =  analog input i for ADC[adcSel]
              *ex.  i = 0, then we select AIN0 for either ADC0 or ADC1*/
             selected_measurement_srcs[adcSel][j].INPUTCTRL.reg = 1%16;

             /*ACGCTRL must be specified because of errata*/
             selected_measurement_srcs[adcSel][j].AVGCTRL.reg = AVGCTRL_SETTING;

             /*j is the index where the results for this channel will be
              * stored adc_state_g.adc_input_buffer. We therefore store this
              * index at adc_chan_get_storage_key [<channel number>] */
             adc_map_chan_to_storage_location(i) = j;

             channel_mask_temp!= ~(1<<i);

             adc_state_g.chan_count[adcSel] ++;

             j++;

             }

        i++;
    }

    /*done adding external channels. Now add internal channels */
    
    /*adc external channels to turn on are specified by the following bits
     *for adc0: bits 32 - 47
     *for adc1: bits 48 - 63
     *note: there are only 8 internal channels to read from, so the internal
     *pin mask only uses bits 32 - 39 for ADC0 and bits 48 - 55 for ADC1
      
    *below we are calculating the first bit to inspect in the channel_mask
    *for determining which internal channels to activate */
    i = 32 + (16 * adcSel);

    //don't delete. This is necessary for the while loop below
    uint8_t start_index = i;

    //there are only 8 internal measurement sources to read from
    uint8_t stop_index  = i + 8;

    /*bits 32 - 47 represent the internal inputs for ADC0 and bits 48 - 63 
     *represent the internal inputs for ADC1. Therefore, depending on the 
     *ADC module that we're setting up we need to look at different bits
     *in the channel mask to find out which channels to read from*/
    uint64_t internal_channel_mask = oxffff << (32 + (adcSel * 16);

    while((channel_mask_temp & internal_channel_mask) && (i < stop_index)){

        if(channel_mask_temp & (1 <<i)){

            /*add the measurement source. Note that 0x18 specifies the first 
             *internal measurement source, which is the scaledCoreVcc value*/
            selected_measurement_srcs[adcSel][j].INPUTCTRL = (i % start_index) + 0x18;

            //AVGCTRL must be specified becasue of errata
            selected_measurement_srcs[adcSel][j].AVGCTRL.reg = AVGCTRL_SETTING;

            /*j is the index where the results for this channel will be
             * stored in adc_state_g.adc_input_buffer.*/

            /*since internal channels are not specific to either ADC, they 
             *they don't have their own channel number like external pins do
             * for example ain[1] is channel 1 for ADC0 and channel 17 for 
             * ADC1. CORE_VCC however, can be read by adc0 or adc1 (in the 
             * former case, it would be specified by bit 32 in channel_mask
             * in the later case, bit 48 in the channel mask. However, it's 
             * still technically a single channel that either ADC can access
             * therefore, we store the results in a common channel -chan 32
             
             * this makes the logic for retreiving the reading easier*/
            uint8_t int_chan = i - (16 * adcSel);

            adc_map_chan_to_storage_location(int_chan) = j;

            channel_mask_temp!= ~(1<<i);

            j++;

            adc_state_g.chan_count[adcSel] ++;

            }

           i++;
       }

    }

    /*add a stop command to the lst input source given to the ADC because after this input
     *there are no more input targets for it to read from*/
    selected_measurement_srcs[adcSel][j-1].INPUTCTRL.reg | (1 << ADC_INPUTCTRL_DSEQSTOP_Pos);


//------------------------Done with specifying measurement sources------------------------///
//----------------------------------------------------------------------------------------///

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
                        adc_state_g.chan_count[adcSel],
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
                        &(ADC_measurement_srcs),
                        //source to read from
                        1,
                        //should the source be incremented? 1 = yes
                        &(ADCx->DSEQDATA.reg),
                        //destination to write data
                        0,
                        //should the destination be incremented? 0 = No
                        adc_state_g.chan_count[adcSel] * 2,
                        //number of beats to send per transaction
                        trig,
                        //trigger source
                        priority,
                        //priority
                        NULL);
                        //the next descriptor to link


    // Enable auto_start on the ADC so that it starts a new scan right after the
    // DMA has moved the results out of the results register and into the
    // internal buffer.
    // DMA sequencing should update INPUTCTRL. Also updates AVGCTL because of
    // errata.
    // This also enables DMASequencing!
    ADCx->DSEQCTRL.reg = (ADC_DSEQCTRL_AUTOSTART |
                          ADC_DSEQCTRL_INPUTCTRL);// |
                          //ADC_DSEQCTRL_AVGCTRL);

    ADCx->SWTRIG.reg = ADC_SWTRIG_FLUSH;

    /* ADC will begin measuring sources after the first DMA descriptor is
     // moved to DMA_SEQ_DATA.
     // After that, autostart will take care of the rest
     */
    adc_state_g.last_sweep_time = millis;


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
            uint8_t results_to_buffer_priority;

            if(adcSel == 1){
                results_to_buffer_priority = ADC1_DMA_RES_TO_BUFFER_PRIORITY;
            }else{
                results_to_buffer_priority = ADC0_DMA_RES_TO_BUFFER_PRIORITY;
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
                            adc_state_g.chan_count,
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
                                &(ADC_measurement_srcs),
                                //source to read from
                                1,
                                //should the source be incremented? 1 = yes
                                &(ADCx->DSEQDATA.reg),
                                //destination to write data
                                0,
                                //should the destination be incremented? 0 = No
                                adc_state_g.chan_count * 2,
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

static void configure_and_count_analog_inputs(uint64_t channel_mask){

    static uint8_t already_configured = 0;

    if(already_configured){
        return;
    }


    while (channel_mask & ADC_EXTERNAL_ANALOG_MASK) {

        /*__builtin_ctzl counts the number of trailing zeros, and gives us the
          number designation of the channel that the user has specified should
          be read from.
          ex. channel_mask = 0b00000000000000000000000000000100
              __builtin_ctzl(channel_mask) -> 2

              therefore, channel 2 was designated by the user as a channel that
              the ADC should read
              */
        int const chan = __builtin_ctzl(channel_mask);

        //based on the channel, which ADC do we need to activate?
        int const adc_num = adc_ain_chan_get_adc(chan);

        /*based on the channel number, what ADC Positive mux value must be
          configured? */
        int const chan_pmux = ADC_CHAN_GET_PMUX(chan);

        /*select the port and pin number associated with the channel that
          was selected*/
        adcx_set_pmux(adc_pins[adc_num][chan_pmux]);

        /*zero out the channel that we just activated so we can move onto the
          next one*/
        channel_mask &= ~(1 << chan);


    }

    already_configured = 1;
    return chan_count;
}



uint64_t get_balanced_channel_mask(uint64_t unbalanced_channel_mask){
    
    uint64_t balanced_channel_mask = unbalanced_channel_mask;

    //find the difference in load between ADCs
    uint8_t diff = adc_state_g.chan_count[0] - adc_state_g.chan_count[1];

    uint8_t adc0_has_int_chans_to_donate = balanced_channel_mask & (0xffff << 32);

    while(diff > 0 && adc0_has_int_chans_to_donate){
        //find first available internal channel to move from adc0 to adc1
        uint8_t int_chan_temp = __builtin_ctzl(balanced_channel_mask & 0xffff <<32);

        //disable the channel for ADC0
        balanced_channel_mask != ~(1<< int_chan_temp);

        //enable the channel for ADC1
        balanced_channel_mask |=  (1<< (int_chan_temp + 16));

        //update
        adc0_has_int_chans_to_donate = balanced_channel_mask & (0xffff << 32);

        adc_state_g.chan_count[0]--;
        adc_state_g.chan_count[1]++;

    }
}


int init_adc(uint32_t clock_mask, uint32_t clock_freq,
             uint64_t channel_mask, uint32_t sweep_period,
             uint32_t max_source_impedance, int8_t DMA_res_to_buff_chan,
             int8_t DMA_buff_to_DMASEQ_chan){

    if (!channel_mask){
        //give up if no channels are enabled
        return 1;
    }

    adc_state_g.sweep_period = MS_TO_MILLIS(sweep_period);
    adc_state_g.chan_count[0] = 0;
    adc_state_g.chan_count[1] = 0;

    //count num of channels 
    for(int i =0 ; i < 64 < i++){
        if(channel_mask & (1 << i)){
            //note, all internal channels are assigned to ADC0 initially
            if(i > 31 || i < 16){
                adc_state_g.chan_count[0]++;
            }
            else{ 
                adc_state_g.chan_count[1]++;
            }

        }
    }

    //balance the channel load on both ADCs
    adc_state_g.channel_mask = get_balanced_channel_mask(channel_mask);

    //determine which ADC module, ADC0 or ADC1 or both, should be turned on----//
    if((channel_mask & ADC0_EXTERNAL_ANALOG_MASK) || 
      ((channel_mask & INTERNAL_CHANNEL_MASK) && (ADC_INTERNAL_SRC_READER == 0)))
        {
            init_adc_helper(clock_mask, clock_freq, max_source_impedance,
                            DMA_res_to_buff_chan, DMA_buff_to_DMASEQ_chan, 0 /*adcSel = ADC0*/)
        }

    if((channel_mask & ADC1_EXTERNAL_ANALOG_MASK) || 
      ((channel_mask & INTERNAL_CHANNEL_MASK) && (ADC_INTERNAL_SRC_READER == 1)))
        {
            init_adc_helper(clock_mask, clock_freq, max_source_impedance,DMA_res_to_buff_chan,
                            DMA_buff_to_DMASEQ_chan, 1 /*adcSel = ADC0*/)
        }

}

int init_adc_helper(uint32_t clock_mask, uint32_t clock_freq,
                    uint32_t max_source_impedance, int8_t DMA_res_to_buff_chan,
                    int8_t DMA_buff_to_DMASEQ_chan, uint8_t adcSel){


    //----create a pointer to the ADC that we're configuring---//
    Adc* ADCx = (adcSel == 1)? ADC1: ADC0;

    adc_state_g.chan_count[adcSel] = 0;

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

//----configure voltage reference and temperature sensor----//
    SUPC->VREF.reg = (SUPC_VREF_SEL_1V0 | SUPC_VREF_ONDEMAND | SUPC_VREF_TSEN);

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

    //how want to accumolate 1024 samples, and take the average value.
    // No right shifts for 16 bit resolution
    ADCx->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_1024 | ADC_AVGCTRL_ADJRES(0);
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


        init_ADCx_DMA(DMA_res_to_buff_chan, DMA_buff_to_DMASEQ_chan, adcSel);
    } else {
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

uint16_t adc_get_value (uint8_t channel){

    //channel number must be between 0 and total adc channels
    if((channel < 0) || (channel > ADC_TOTAL_NUM_CHANS)){
        return 0;
    }

    uint8_t storage_index_key = adc_chan_get_storage_key(channel);
    return adc_state_g.adc_input_buffer[storage_index_key];
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
      uint16_t const val = adc_get_value(ADC_CHAN_CORE_VCC);
      return (int16_t)((4000 * (uint32_t)val) / 65535);
}

int16_t adc_get_io_vcc (void){
  //get the voltage value of the input/output.
  //this value needs to be multiplied by 4 since the value we get from the adc is
  //scaled by 1/4. The returned value must be stored in a 32 bit word because
  //the value retrieved could be 0xFFFF, which, when multiplied by 4 would overflow
      uint16_t const val = adc_get_value(ADC_CHAN_SCALED_IOVCC);
      return (int16_t)((4000 * (uint32_t)val) / 65535);
}

int16_t adc_get_bat_vcc (void){
  //get the latest voltage value of VBAT
    uint16_t const val = adc_get_value(ADC_CHAN_SCALED_VBATT_SUPPLY);
    return (int16_t)((4000 * (uint32_t)val) / 65535);
}


int16_t adc_get_bandgap_vcc (void){
//get the latest voltage value of the badgap
  return adc_get_value_millivolts(ADC_CHAN_BANDGAP);
}

int16_t adc_get_DAC_val (void){
//get the latest voltage value of the Digital-to-A
  return adc_get_value_millivolts(ADC_CHAN_DAC);
}


uint32_t adc_get_last_sweep_time (void)
{
    return adc_state_g.last_sweep_time;
}

uint64_t adc_get_channel_mask (void)
{
    return adc_state_g.channel_mask;
}
