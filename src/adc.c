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
    uint32_t sweep_period;
    
    uint16_t adc_in_buffer_pins[20];
    uint16_t adc_in_buffer_internal[5];
    
    union {
        uint8_t dma_chan;
        struct {
            uint8_t chan_num;
            uint8_t last_chan;
        };
    };
    
    uint8_t use_dma:1;
} adc_state_g;

struct pin_t {
    uint8_t num:5;
    uint8_t port:1;
};

static const struct pin_t adc_pins[] = {
    {.port = 0, .num = 2},  // AIN[0]
    {.port = 0, .num = 3},  // AIN[1]
    {.port = 1, .num = 8},  // AIN[2]
    {.port = 1, .num = 9},  // AIN[3]
    {.port = 0, .num = 4},  // AIN[4]
    {.port = 0, .num = 5},  // AIN[5]
    {.port = 0, .num = 6},  // AIN[6]
    {.port = 0, .num = 7},  // AIN[7]
    {.port = 1, .num = 0},  // AIN[8]
    {.port = 1, .num = 1},  // AIN[9]
    {.port = 1, .num = 2},  // AIN[10]
    {.port = 1, .num = 3},  // AIN[11]
    {.port = 1, .num = 4},  // AIN[12]
    {.port = 1, .num = 5},  // AIN[13]
    {.port = 1, .num = 6},  // AIN[14]
    {.port = 1, .num = 7},  // AIN[15]
    {.port = 0, .num = 8},  // AIN[16]
    {.port = 0, .num = 9},  // AIN[17]
    {.port = 0, .num = 10}, // AIN[18]
    {.port = 0, .num = 11}  // AIN[19]
};

static void adc_set_pmux (uint8_t channel)
{
    struct pin_t pin = adc_pins[channel];
    
    if ((pin.num % 2) == 0) {
        PORT->Group[pin.port].PMUX[pin.num / 2].bit.PMUXE = 0x1;
    } else {
        PORT->Group[pin.port].PMUX[pin.num / 2].bit.PMUXO = 0x1;
    }
    PORT->Group[pin.port].PINCFG[pin.num].bit.PMUXEN = 0b1;
}

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
        // Last scan was over range A
        if (adc_state_g.channel_mask & ADC_RANGE_B_MASK) {
            // Next scan over B
            scan_mask = adc_state_g.channel_mask & ADC_RANGE_B_MASK;
            range = ADC_RANGE_B;
        } else if (adc_state_g.channel_mask & ADC_RANGE_INT_MASK) {
            // Next scan over Internal
            scan_mask = adc_state_g.channel_mask & ADC_RANGE_INT_MASK;
            range = ADC_RANGE_INTERNAL;
        } else {
            // Next scan over A
            scan_mask = adc_state_g.channel_mask & ADC_RANGE_A_MASK;
            range = ADC_RANGE_A;
        }
    } else if (ADC->INPUTCTRL.bit.MUXPOS <= ADC_RANGE_B_LAST) {
        // Last scan was over range B
        if (adc_state_g.channel_mask & ADC_RANGE_INT_MASK) {
            // Next scan over Internal
            scan_mask = adc_state_g.channel_mask & ADC_RANGE_INT_MASK;
            range = ADC_RANGE_INTERNAL;
        } else if (adc_state_g.channel_mask & ADC_RANGE_A_MASK) {
            // Next scan over A
            scan_mask = adc_state_g.channel_mask & ADC_RANGE_A_MASK;
            range = ADC_RANGE_A;
        } else {
            // Next scan over B
            scan_mask = adc_state_g.channel_mask & ADC_RANGE_B_MASK;
            range = ADC_RANGE_B;
        }
    } else {
        // Last scan was over internal inputs
        if (adc_state_g.channel_mask & ADC_RANGE_A_MASK) {
            // Next scan over A
            scan_mask = adc_state_g.channel_mask & ADC_RANGE_A_MASK;
            range = ADC_RANGE_A;
        } else if (adc_state_g.channel_mask & ADC_RANGE_B_MASK) {
            // Next scan over B
            scan_mask = adc_state_g.channel_mask & ADC_RANGE_B_MASK;
            range = ADC_RANGE_B;
        } else {
            // Next scan over Internal
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
        uint16_t *buffer = ((scan_mask & ADC_RANGE_INT_MASK) ?
                            (adc_state_g.adc_in_buffer_internal +
                             (first - ADC_RANGE_INT_FIRST)) :
                            (adc_state_g.adc_in_buffer_pins + first));
    
        dma_start_static_to_buffer_hword(adc_state_g.dma_chan, buffer,
                                         (1 + last - first),
                                         ((volatile const uint16_t*)
                                          (&ADC->RESULT.reg)),
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
    /** Start free running input scan */
    ADC->CTRLB.bit.FREERUN = 0b1;
}

uint8_t init_adc (uint32_t clock_mask, uint32_t clock_freq,
                  uint32_t channel_mask, uint32_t sweep_period,
                  uint32_t max_source_impedance, int8_t dma_chan)
{
    if (!channel_mask) {
        // Give up if no channels are enabled
        return 1;
    }
    
    /* Configure all enabled pins as analog inputs */
    for (uint8_t i = 0; i < ADC_RANGE_B_LAST; i++) {
        if (channel_mask & (1 << i)) {
            adc_set_pmux(i);
        }
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
    
    /* Use internal 1.0 V reference */
    ADC->REFCTRL.reg = ADC_REFCTRL_REFSEL_INT1V;
    
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
    
    // Set prescaler, 16 bit output
    ADC->CTRLB.reg = (ADC_CTRLB_PRESCALER(prescaler) |
                      ADC_CTRLB_RESSEL_16BIT);
    // Wait for synchronization
    while (ADC->STATUS.bit.SYNCBUSY);
    
    /* Calculate sample time for target source impedance */
    // See SAMD21 datasheet sections 37.11.4.3 and 33.8.4 for details
    // The constants used in this formula assume 16 bit accuracy
    
    // Add R_SAMPLE to target maximum source impedence and multiply by
    // 2 * C_SAMPLE * (n + 1) * ln(2), where n = 16 is the accuracy
    // Since C_SAMPLE is so small, we scale the constant up by 10e15
    uint64_t samplen = (uint64_t)(max_source_impedance + 3500) * 82485;
    // Divide by the ADC clock frequecy
    samplen *= clock_freq / (1 << (prescaler + 2));
    
    // Divide by 10e15 to remove scaling factor and or with 0x3F to ensure that
    // the result is at most 63
    // The 64 bit constant for 10e15 is written as a product of two 32 bit
    // constants to avoid a extraneous warning
    ADC->SAMPCTRL.bit.SAMPLEN = (samplen / (1000000000UL * 1000000UL)) & 0x3F;
    
    /* Enable bandgap and temparature references if requested */
    uint8_t bg = !!(channel_mask & (1 << ADC_INPUTCTRL_MUXPOS_BANDGAP_Val));
    uint8_t ts = !!(channel_mask & (1 << ADC_INPUTCTRL_MUXPOS_TEMP_Val));
    SYSCTRL->VREF.reg |= ((bg << SYSCTRL_VREF_BGOUTEN_Pos) |
                          (ts << SYSCTRL_VREF_TSEN_Pos));
    
    /* Configure initial state */
    adc_state_g.channel_mask = channel_mask;
    adc_state_g.sweep_period = sweep_period;
    
    // Set bits in mask to indicate which ranges are used
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
    
    // Set up for first sweep
    adc_conf_scan();
    
    return 0;
}

void adc_service (void)
{
    if ((millis - adc_state_g.last_sweep_time) > adc_state_g.sweep_period) {
        /* Enable ADC */
        ADC->CTRLA.bit.ENABLE = 1;
        // Wait for synchronization
        while (ADC->STATUS.bit.SYNCBUSY);
        
        // Start next scan
        adc_state_g.last_sweep_time = millis;
        adc_start_scan();
    }
}

uint16_t adc_get_value (uint8_t channel)
{
    if (channel >= ADC_RANGE_INT_FIRST) {
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
    int64_t temp_r = (((uint64_t)room_temp_val_int * 100000000UL) +
                      ((uint64_t)room_temp_val_dec * 10000000UL));
    
    // Hot temperature (in nanodegrees celsius)
    uint8_t hot_temp_val_int = (uint8_t)(((*((uint32_t*)
                                        NVMCTRL_FUSES_HOT_TEMP_VAL_INT_ADDR)) &
                                          NVMCTRL_FUSES_HOT_TEMP_VAL_INT_Msk) >>
                                         NVMCTRL_FUSES_HOT_TEMP_VAL_INT_Pos);
    uint8_t hot_temp_val_dec = (uint8_t)(((*((uint32_t*)
                                        NVMCTRL_FUSES_HOT_TEMP_VAL_DEC_ADDR)) &
                                          NVMCTRL_FUSES_HOT_TEMP_VAL_DEC_Msk) >>
                                         NVMCTRL_FUSES_HOT_TEMP_VAL_DEC_Pos);
    int64_t temp_h = (((uint64_t)hot_temp_val_int * 100000000UL) +
                      ((uint64_t)hot_temp_val_dec * 10000000UL));
    
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
    
    /* Compute coefficients to convert ADC values to nanovolts */
    uint32_t adc_r_co = ((100000 * (uint32_t)int1v_r) + 2048) / 4095;
    uint32_t adc_h_co = ((100000 * (uint32_t)int1v_h) + 2048) / 4095;
    uint32_t adc_m_course_co = ((100000 * (uint32_t)1000) + 32768) / 65535;
    
    /* Compute voltages (in nanovolts) */
    uint32_t v_adc_r = adc_r_val * adc_r_co;
    uint32_t v_adc_h = adc_h_val * adc_h_co;
    uint32_t v_adc_m_course = adc_m_val * adc_m_course_co;
    
    /* Compute course temp (in nanodegrees celsius) */
    int32_t denominator = v_adc_h - v_adc_r;
    int32_t delta_v_course = v_adc_m_course - v_adc_r;
    int64_t delta_t = temp_h - temp_r;
    
    int64_t numerator_course = (int64_t)delta_v_course * delta_t;
    int64_t temp_c = temp_r + (numerator_course / denominator);
    
    if (!fine) {
        // Return course value
        return (int16_t)(temp_c / 1000000);
    }
    
    /* Estimate 1 V reference actual voltage (in nanovolts) */
    int16_t delta_int1v = int1v_h - int1v_r;
    int32_t delta_t_s = delta_t / 100000;
    int32_t int1v_r_s = 100000 * int1v_r;
    int32_t int1v_m = int1v_r_s + (delta_int1v * (temp_c - temp_r) / delta_t_s);

    /* Compute new coefficient for measured temp ADC value */
    uint32_t adc_m_fine_co = (int1v_m + 32768) / 65535;
    
    /* Compute new measured ACD voltage (in nanovolts) */
    uint32_t v_adc_m_fine = adc_m_val * adc_m_fine_co;
    
    /* Compute fine temp (in nanodegrees celsius) */
    int32_t delta_v_fine = v_adc_m_fine - v_adc_r;
    int64_t numerator_fine = (int64_t)delta_v_fine * delta_t;
    int64_t temp_f = temp_r + (numerator_fine / denominator);
    
    // Return fine value
    return (int16_t)(temp_f / 1000000);
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

#ifdef ENABLE_ADC
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
#endif


static void adc_dma_callback (uint8_t chan, void *state)
{
    // Stop freerunning mode
    ADC->CTRLB.bit.FREERUN = 0;
    ADC->SWTRIG.bit.FLUSH = 1;
    
    // Configure the next sweep
    enum adc_scan_range next_range = adc_conf_scan();

    if (((next_range == ADC_RANGE_B) && (adc_state_g.channel_mask & ADC_RANGE_A_MASK)) ||
        ((next_range == ADC_RANGE_INTERNAL) && (adc_state_g.channel_mask & (~ADC_RANGE_INT_MASK)))) {
        // The next range is B and range A is selected or the next scan range is
        // internal and A or B is selected
        adc_start_scan();
    } else {
        // Full set of sweeps is complete
        adc_state_g.last_sweep_time = millis;
        
        if (adc_state_g.sweep_period == 0) {
            // Next sweep should be started right away
            adc_start_scan();
        } else {
            // Disable the ADC
            ADC->CTRLA.bit.ENABLE = 0;
        }
    }
}
