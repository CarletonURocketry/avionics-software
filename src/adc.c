/**
 * @file adc.c
 * @desc ADC Driver
 * @author Samuel Dewan
 * @date 2019-04-19
 * Last Author:
 * Last Edited On:
 */

#include "adc.h"

#include "dma.h"
#include "evsys.h"
#include "tc.h"

#define ADC_IRQ_PRIORITY    3

#define ADC_DMA_PRIORITY    0

// Max ADC clock freq, see datasheet section 7.11.4
#define ADC_CLOCK_MAX 2100000UL


static void adc_dma_callback (uint8_t chan, void *state);


#define ADC_RANGE_A_FIRST       ADC_INPUTCTRL_MUXPOS_PIN0_Val
#define ADC_RANGE_A_LAST        ADC_INPUTCTRL_MUXPOS_PIN9_Val
#define ADC_RANGE_A_MASK        0x3FF
#define ADC_RANGE_A_BIT         29

#define ADC_RANGE_B_FIRST       ADC_INPUTCTRL_MUXPOS_PIN10_Val
#define ADC_RANGE_B_LAST        ADC_INPUTCTRL_MUXPOS_PIN19_Val
#define ADC_RANGE_B_MASK        0xFFC00
#define ADC_RANGE_B_BIT         30

#define ADC_RANGE_INT_FIRST     ADC_INPUTCTRL_MUXPOS_TEMP_Val
#define ADC_RANGE_INT_LAST      ADC_INPUTCTRL_MUXPOS_DAC_Val
#define ADC_RANGE_INT_MASK      0x1F000000
#define ADC_RANGE_INT_BIT       31


enum adc_scan_range {
    /* Scan from channels 1 to 10 */
    ADC_RANGE_A,
    /* Scan from channel 11 to 19 */
    ADC_RANGE_B,
    /* Scan of internal channels */
    ADC_RANGE_INTERNAL
};

struct {
    uint32_t channel_mask;
    uint32_t last_sweep_time;
    
    union {
        uint32_t sweep_period;
        Tc *tc;
    };
    
    uint16_t adc_in_buffer_pins[20];
    uint16_t adc_in_buffer_internal[5];
    
    union {
        uint8_t dma_chan;
        struct {
            uint8_t chan_num;
            uint8_t last_chan;
        };
    };
    
    uint8_t event_chan:4;
    uint8_t use_tc:1;
    uint8_t use_dma:1;
} adc_state_g;


/**
 *  Configure the ADC and supporting peripherals to be ready for the next scan.
 *
 *  @param range The scan range to configure
 *
 *  @return The next range which will be scanned
 */
static enum adc_scan_range adc_conf_scan (void)
{
    uint32_t scan_mask;
    enum adc_scan_range range;
    
    /* Find the next range */
    if (ADC->INPUTCTRL.bit.MUXPOS <= ADC_RANGE_A_LAST) {
        // Last scan was over range A, next scan over B
        scan_mask = adc_state_g.channel_mask & ADC_RANGE_B_MASK;
        range = ADC_RANGE_B;
    } else if (ADC->INPUTCTRL.bit.MUXPOS <= ADC_RANGE_B_LAST) {
        // Last scan was over range B, next scan over Internal
        scan_mask = adc_state_g.channel_mask & ADC_RANGE_INT_MASK;
        range = ADC_RANGE_INTERNAL;
    } else {
        // Last scan was over internal inputs, next scan over A
        scan_mask = adc_state_g.channel_mask & ADC_RANGE_A_MASK;
        range = ADC_RANGE_B;
    }
    
    /* Skip any ranges without enabled channels */
    while (!scan_mask) {
        if (scan_mask == (adc_state_g.channel_mask & ADC_RANGE_INT_MASK)) {
            scan_mask = adc_state_g.channel_mask & ADC_RANGE_A_MASK;
            range = ADC_RANGE_A;
        } else if (scan_mask == (adc_state_g.channel_mask & ADC_RANGE_A_MASK)) {
            scan_mask = adc_state_g.channel_mask & ADC_RANGE_B_MASK;
            range = ADC_RANGE_B;
        } else if (scan_mask == (adc_state_g.channel_mask & ADC_RANGE_B_MASK)) {
            scan_mask = adc_state_g.channel_mask & ADC_RANGE_INT_MASK;
            range = ADC_RANGE_INTERNAL;
        }
    }
    
    /* Find first and last chan for scan */
    uint8_t first = __builtin_ctz(scan_mask);
    uint8_t last = 31 - __builtin_clz(scan_mask);
    
    /* Configure scan */
    ADC->INPUTCTRL.reg = (ADC_INPUTCTRL_MUXPOS(first) |
                          ADC_INPUTCTRL_MUXNEG_GND |
                          ADC_INPUTCTRL_INPUTSCAN(last - first) |
                          ADC_INPUTCTRL_INPUTOFFSET(0) |
                          ADC_INPUTCTRL_GAIN_1X);
    // Wait for synchronization
    while (ADC->STATUS.bit.SYNCBUSY);
    
    /* Configure DMA or interrupts */
    if (adc_state_g.use_dma) {
        uint16_t *buffer = ((scan_mask ==
                             (adc_state_g.channel_mask & ADC_RANGE_INT_MASK)) ?
                            (adc_state_g.adc_in_buffer_internal +
                             (first - ADC_RANGE_INT_FIRST)) :
                            (adc_state_g.adc_in_buffer_pins + first));
    
        dma_start_static_to_buffer_hword(adc_state_g.dma_chan, buffer,
                                         (1 + last - first),
                                         (uint16_t*)(&ADC->RESULT.reg),
                                         ADC_DMAC_ID_RESRDY, ADC_DMA_PRIORITY);
    } else {
        adc_state_g.chan_num = first;
        adc_state_g.last_chan = last;
    }
    
    return range;
}

/**
 *  Start the next ADC scan.
 */
static inline void adc_start_scan (void)
{
    /** Trigger ADC scan */
    ADC->SWTRIG.bit.START = 0b1;
    // Wait for synchronization
    while (ADC->STATUS.bit.SYNCBUSY);
}

uint8_t init_adc (uint32_t clock_mask, uint32_t clock_freq,
                  uint32_t channel_mask, uint32_t sweep_period, int8_t dma_chan,
                  Tc *tc, int8_t event_chan)
{
    if (!channel_mask) {
        // Give up if no channels are enabled
        return 1;
    }
    
    /* Enable the APBC clock for the ADC */
    PM->APBCMASK.reg |= PM_APBCMASK_ADC;
    
    /* Select the core clock for the ADC */
    GCLK->CLKCTRL.reg = (GCLK_CLKCTRL_CLKEN | clock_mask | GCLK_CLKCTRL_ID_ADC);
    // Wait for synchronization
    while (GCLK->STATUS.bit.SYNCBUSY);
    
    /* Reset ADC */
    ADC->CTRLA.bit.SWRST = 1;
    // Wait for reset to complete
    while (ADC->CTRLA.bit.SWRST | ADC->STATUS.bit.SYNCBUSY);
    
    /* Use internal 1.0 V reference with reference buffer offset compensation */
    ADC->REFCTRL.reg = ADC_REFCTRL_REFCOMP | ADC_REFCTRL_REFSEL_INT1V;
    
    /* 256x oversampling and decimation for 16 bit effective resolution */
    ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_256 | ADC_AVGCTRL_ADJRES(0);
    
    /* Configure Control B register */
    // Calculate prescaler value
    // Division factor (rounded up to ensure maximum frequency is not exceeded)
    //      div = 1 + ((clock_freq - 1) / ADC_CLOCK_MAX);
    // Find (log2 of the next highest power of two) - 2
    //      prescaler = (32 - __builtin_clz(div - 1)) - 2;
    uint32_t prescaler = 30 - __builtin_clz((clock_freq - 1) / ADC_CLOCK_MAX);
    // Make sure prescaler is at most 512
    // (the ADC clock may exceed the maximum value if the generic clock is
    // faster than 1 GHz, but at that point the ADC clock will be far from the
    // only thing wrong)
    prescaler &= 0x1FF;
    
    // Set prescaler, 16 bit output, freerunning mode
    ADC->CTRLB.reg = (ADC_CTRLB_PRESCALER(prescaler) |
                      ADC_CTRLB_RESSEL_16BIT |
                      ADC_CTRLB_FREERUN);
    // Wait for synchronization
    while (ADC->STATUS.bit.SYNCBUSY);
    
    /* Enable bandgap and temparature references if requested */
    uint8_t bg = !!(channel_mask & (1 << ADC_INPUTCTRL_MUXPOS_BANDGAP_Val));
    uint8_t ts = !!(channel_mask & (1 << ADC_INPUTCTRL_MUXPOS_TEMP_Val));
    SYSCTRL->VREF.reg |= ((bg << SYSCTRL_VREF_BGOUTEN_Pos) |
                          (ts << SYSCTRL_VREF_TSEN_Pos));
    
    /* Configure initial state */
    adc_state_g.channel_mask = channel_mask;
    adc_state_g.sweep_period = sweep_period;
    
    // Set in mask to indicate which ranges are used
    channel_mask |= ((!!(channel_mask & ADC_RANGE_A_MASK) << ADC_RANGE_A_BIT) |
                     (!!(channel_mask & ADC_RANGE_B_MASK) << ADC_RANGE_B_BIT) |
                     (!!(channel_mask & ADC_RANGE_INT_MASK) << ADC_RANGE_INT_BIT)
                    );
    
    /* Configure DMA or interrupts */
    if ((dma_chan >= 0) && (dma_chan < DMAC_CH_NUM)) {
        // Use DMA
        adc_state_g.use_dma = 1;
        adc_state_g.dma_chan = dma_chan;
        
        dma_callbacks[dma_chan] = (struct dma_callback_t) {
            .callback = adc_dma_callback,
            .state = NULL
        };
    } else {
        // Enable interrupt when result is ready to be read
        ADC->INTENSET.bit.RESRDY = 1;
        // Enable ADC interrupt in NVIC
        NVIC_SetPriority(ADC_IRQn, ADC_IRQ_PRIORITY);
        NVIC_EnableIRQ(ADC_IRQn);
    }
    
    /* Configure timer and event system (if requested) */
    if (sweep_period && (tc != NULL)) {
        adc_state_g.use_tc = 1;
        adc_state_g.tc = tc;
        adc_state_g.event_chan = event_chan;
        
        // Initilize TC to provide events at the sweep period
        uint8_t r = init_tc_periodic_event(tc, sweep_period, clock_mask,
                                           clock_freq);
        if (r) {
            // TC could not be initilized
            return 1;
        }
        
        // Configure the ADC event user MUX to accept events from the correct
        // channel
        evsys_configure_user_mux(EVSYS_ID_USER_ADC_START, event_chan);
        // Configure the EVSYS channel to source events from the TC
        evsys_configure_channel(event_chan, tc_get_evsys_gen_ovf_id(tc),
                                clock_mask, EVSYS_PATH_ASYNCHRONOUS,
                                EVSYS_EDGE_RISING);
        // Configure ADC to start sweep on event
        ADC->EVCTRL.bit.STARTEI = 1;
    }
    
    // Set up for first sweep
    adc_conf_scan();
    
    /* Enable ADC */
    ADC->CTRLA.bit.ENABLE = 1;
    // Wait for synchronization
    while (ADC->STATUS.bit.SYNCBUSY);
    
    return 0;
}

void adc_service (void)
{
    if ((!adc_state_g.use_tc) &&
        ((millis - adc_state_g.last_sweep_time) > adc_state_g.sweep_period)) {
        adc_state_g.last_sweep_time = millis;
        // Start next scan
        adc_start_scan();
    }
}

uint16_t adc_get_value (uint8_t channel)
{
    if (adc_state_g.chan_num >= ADC_RANGE_INT_FIRST) {
        return adc_state_g.adc_in_buffer_internal[channel - ADC_RANGE_INT_FIRST];
    } else {
        return adc_state_g.adc_in_buffer_pins[channel];
    }
}

uint16_t adc_get_value_millivolts (uint8_t channel)
{
    uint16_t adc_m = adc_get_value(channel);
    return (1000 * (uint32_t)adc_m) / 65535;
}

uint32_t adc_get_value_nanovolts (uint8_t channel)
{
    uint16_t adc_m = adc_get_value(channel);
    return (uint32_t)((1000000000 * (uint64_t)adc_m) / 65535);
}

static int16_t adc_get_temp (uint8_t fine)
{
    /* Check that temperature sensor is enabled */
    if (!(adc_state_g.channel_mask & ADC_INPUTCTRL_MUXPOS_TEMP)) {
        return INT16_MIN;
    }
    
    /* Read values from Temperature Log Row */
    // Room temperature (in hundred nanodegrees celsius)
    uint8_t room_temp_val_int = (uint8_t)(((*((uint32_t*)
                                        NVMCTRL_FUSES_ROOM_TEMP_VAL_INT_ADDR)) &
                                        NVMCTRL_FUSES_ROOM_TEMP_VAL_INT_Msk) >>
                                          NVMCTRL_FUSES_ROOM_TEMP_VAL_INT_Pos);
    uint8_t room_temp_val_dec = (uint8_t)(((*((uint32_t*)
                                        NVMCTRL_FUSES_ROOM_TEMP_VAL_DEC_ADDR)) &
                                        NVMCTRL_FUSES_ROOM_TEMP_VAL_DEC_Msk) >>
                                          NVMCTRL_FUSES_ROOM_TEMP_VAL_DEC_Pos);
    int32_t temp_r = ((room_temp_val_int * 10000000) +
                      (room_temp_val_dec * 1000000));
    
    // Hot temperature (in nanodegrees celsius)
    uint8_t hot_temp_val_int = (uint8_t)(((*((uint32_t*)
                                        NVMCTRL_FUSES_HOT_TEMP_VAL_INT_ADDR)) &
                                          NVMCTRL_FUSES_HOT_TEMP_VAL_INT_Msk) >>
                                         NVMCTRL_FUSES_HOT_TEMP_VAL_INT_Pos);
    uint8_t hot_temp_val_dec = (uint8_t)(((*((uint32_t*)
                                        NVMCTRL_FUSES_HOT_TEMP_VAL_DEC_ADDR)) &
                                          NVMCTRL_FUSES_HOT_TEMP_VAL_DEC_Msk) >>
                                         NVMCTRL_FUSES_HOT_TEMP_VAL_DEC_Pos);
    int32_t temp_h = ((hot_temp_val_int * 10000000) +
                      (hot_temp_val_dec * 1000000));
    
    // 1 V reference actual voltage for room temperature measurment
    int16_t int1v_r = 1000 - ((int8_t)(((*((uint32_t*)
                                           NVMCTRL_FUSES_ROOM_INT1V_VAL_ADDR)) &
                                            NVMCTRL_FUSES_ROOM_INT1V_VAL_Msk) >>
                                            NVMCTRL_FUSES_ROOM_INT1V_VAL_Pos));
    
    // 1 V reference actual voltage for hot temperature measurment
    int16_t int1v_h = 1000 - ((int8_t)(((*((uint32_t*)
                                          NVMCTRL_FUSES_HOT_INT1V_VAL_ADDR)) &
                                            NVMCTRL_FUSES_HOT_INT1V_VAL_Msk) >>
                                            NVMCTRL_FUSES_HOT_INT1V_VAL_Pos));
    
    // ADC value for room temperature measurment
    uint16_t adc_r_val = (uint16_t)(((*((uint32_t*)
                                           NVMCTRL_FUSES_ROOM_ADC_VAL_ADDR)) &
                                        NVMCTRL_FUSES_ROOM_ADC_VAL_Msk) >>
                                       NVMCTRL_FUSES_ROOM_ADC_VAL_Pos);
    
    // ADC value for hot temperature measurment
    uint16_t adc_h_val = (uint16_t)(((*((uint32_t*)
                                          NVMCTRL_FUSES_HOT_ADC_VAL_ADDR)) &
                                       NVMCTRL_FUSES_HOT_ADC_VAL_Msk) >>
                                      NVMCTRL_FUSES_HOT_ADC_VAL_Pos);
    
    /* Get measured ADC value */
    uint16_t adc_m_val = adc_state_g.adc_in_buffer_internal[
                        ADC_INPUTCTRL_MUXPOS_TEMP_Val - ADC_RANGE_INT_FIRST];
    
    /* Compute coefficients to convert ADC values to hundred nanovolts */
    uint32_t adc_r_co = ((10000 * int1v_r) + 2048) / 4095;
    uint32_t adc_h_co = ((10000 * int1v_h) + 2048) / 4095;
    uint32_t adc_m_course_co = ((10000 * 1000) + 32768) / 65535;
    
    /* Compute voltages (in hundred nanovolts) */
    uint32_t v_adc_r = adc_r_val * adc_r_co;
    uint32_t v_adc_h = adc_h_val * adc_h_co;
    uint32_t v_adc_m_course = adc_m_val * adc_m_course_co;
    
    /* Compute course temp (in hundred nanodegrees celsius) */
    int32_t denominator = v_adc_h - v_adc_r;
    int32_t delta_v_course = v_adc_m_course - v_adc_r;
    int32_t delta_t = temp_h - temp_r;

    int64_t numerator_course = (int64_t)delta_v_course * delta_t;
    int32_t temp_c = temp_r + (numerator_course / denominator);
    
    if (!fine) {
        // Return course value
        return (int16_t)(temp_c / 100000);
    }
    
    /* Estimate 1 V reference actual voltage (in hundred nanovolts) */
    int16_t delta_int1v = int1v_h - int1v_r;
    int16_t int1v_m = (delta_int1v * (temp_c - temp_r)) / delta_t;
    
    /* Compute new coefficient for measured temp ADC value */
    uint32_t adc_m_fine_co = ((10000 * int1v_m) + 32768) / 65535;
    
    /* Compute new measured ACD voltage (in hundred nanovolts) */
    uint32_t v_adc_m_fine = adc_m_val * adc_m_fine_co;
    
    /* Compute fine temp (in hundred nanodegrees celsius) */
    int32_t delta_v_fine = v_adc_m_fine - v_adc_r;
    int64_t numerator_fine = (int64_t)delta_v_fine * delta_t;
    int32_t temp_f = temp_r + (numerator_fine / denominator);
    
    // Return fine value
    return temp_f;
}

int16_t adc_get_temp_course (void)
{
    return adc_get_temp(0);
}

int16_t adc_get_temp_fine (void)
{
    return adc_get_temp(1);
}

int16_t adc_get_core_vcc (void)
{
    if (!(adc_state_g.channel_mask & ADC_INPUTCTRL_MUXPOS_SCALEDCOREVCC_Val)) {
        return INT16_MIN;
    }
    
    uint16_t adc_m = adc_state_g.adc_in_buffer_internal[
                ADC_INPUTCTRL_MUXPOS_SCALEDCOREVCC_Val - ADC_RANGE_INT_FIRST];
    return (uint16_t)((4000 * (uint32_t)adc_m) / 65535);
}

int16_t adc_get_io_vcc (void)
{
    if (!(adc_state_g.channel_mask & ADC_INPUTCTRL_MUXPOS_SCALEDIOVCC_Val)) {
        return INT16_MIN;
    }
    
    uint16_t adc_m = adc_state_g.adc_in_buffer_internal[
                    ADC_INPUTCTRL_MUXPOS_SCALEDIOVCC_Val - ADC_RANGE_INT_FIRST];
    return (uint16_t)((4000 * (uint32_t)adc_m) / 65535);
}

uint32_t adc_get_last_sweep_time (void)
{
    return adc_state_g.last_sweep_time;
}

uint32_t adc_get_channel_mask (void)
{
    return adc_state_g.channel_mask;
}


void ADC_Handler (void)
{
    // Store result
    if (adc_state_g.chan_num >= ADC_RANGE_INT_FIRST) {
        uint8_t index = adc_state_g.chan_num - ADC_RANGE_INT_FIRST;
        adc_state_g.adc_in_buffer_internal[index] = ADC->RESULT.reg;
        adc_state_g.chan_num++;
    } else {
        uint8_t index = adc_state_g.chan_num++;
        adc_state_g.adc_in_buffer_pins[index] = ADC->RESULT.reg;
    }
    
    // If we have reached the end of a sweep, run the DMA callback function
    if (adc_state_g.chan_num > adc_state_g.last_chan) {
        adc_dma_callback(255, NULL);
    }
}

static void adc_dma_callback (uint8_t chan, void *state)
{
    // Configure the next sweep
    enum adc_scan_range next_range = adc_conf_scan();

    if (((next_range == ADC_RANGE_B) && (adc_state_g.channel_mask & ADC_RANGE_A_MASK)) ||
        ((next_range == ADC_RANGE_INTERNAL) && (adc_state_g.channel_mask && (~ADC_RANGE_INT_MASK)))) {
        // The next range is B and range A is selected or the next scan range is
        // internal and A or B is selected
        adc_start_scan();
    } else {
        // Full set of sweeps is complete
        adc_state_g.last_sweep_time = millis;
        
        if (adc_state_g.sweep_period == 0) {
            // Next sweep should be started right away
            adc_start_scan();
        }
    }
}
