/**
 * @file dac.c
 * @desc DAC Driver
 * @author Samuel Dewan
 * @date 2019-08-28
 * Last Author:
 * Last Edited On:
 */

#include "dac.h"

#include "board.h"
#include "gpio.h"

static void configure_dac_pin(union gpio_pin_t pin)
{
    uint8_t const port = pin.internal.port;
    uint8_t const pin_num = pin.internal.pin;

    if (pin_num & 1) {
        // Odd numbered pin
        PORT->Group[port].PMUX[pin_num / 2].bit.PMUXO = 0x1;
    } else {
        // Even numbered pin
        PORT->Group[port].PMUX[pin_num / 2].bit.PMUXE = 0x1;
    }
    PORT->Group[port].PINCFG[pin_num].bit.PMUXEN = 0b1;
}

void init_dac (uint32_t clock_mask, enum dac_reference reference,
               uint8_t channel_mask, uint8_t enable_int_output,
               uint8_t enable_ext_output)
{
    /* Enable the APBC clock for the DAC */
    enable_bus_clock(PERPH_BUS_CLK_DAC_APB);
    /* Select the core clock for the DAC */
    set_perph_generic_clock(PERPH_GCLK_DAC, clock_mask);
    
    /* Set pin multiplex for DAC pin */
#if defined(SAMD2x)
    if (channel_mask & (1 << 0)) {
        configure_dac_pin(DAC_OUT);
    }
#elif defined(SAMx5x)
    if (channel_mask & (1 << 0)) {
        configure_dac_pin(DAC_OUT0);
    }
    if (channel_mask & (1 << 1)) {
        configure_dac_pin(DAC_OUT1);
    }
#endif
    
    /* Reset DAC */
    DAC->CTRLA.bit.SWRST = 1;
    // Wait for reset to complete
#if defined(SAMD2x)
    while (DAC->CTRLA.bit.SWRST | DAC->STATUS.bit.SYNCBUSY);
#elif defined(SAMx5x)
    while (DAC->CTRLA.bit.SWRST | DAC->SYNCBUSY.bit.SWRST);
#endif
    
    /* Configure DAC */
#if defined(SAMD2x)
    DAC->CTRLB.reg = (((reference == DAC_REF_1V) ? DAC_CTRLB_REFSEL_INT1V :
                       DAC_CTRLB_REFSEL_AVCC) |
                      DAC_CTRLB_LEFTADJ |
                      (enable_int_output ? DAC_CTRLB_IOEN : 0) |
                      (enable_ext_output ? DAC_CTRLB_EOEN : 0));
#elif defined(SAMx5x)
    DAC->CTRLB.reg = ((reference == DAC_REF_1V) ? DAC_CTRLB_REFSEL_INTREF :
                                                  DAC_CTRLB_REFSEL_VDDANA);
    
    for (uint8_t i = 0; i < DAC_CHANNEL_SIZE; i++) {
        if (channel_mask & (1 << i)) {
            // Configure DAC with left adjusted output, current control
            // configured for 12 MHz clock and a refresh every 15 cycles of
            // the 32.768 KHz oscillator
            DAC->DACCTRL[i].reg = (DAC_DACCTRL_LEFTADJ |
                                   DAC_DACCTRL_ENABLE |
                                   DAC_DACCTRL_CCTRL_CC12M |
                                   DAC_DACCTRL_REFRESH(15));
        }
    }
#endif
    
    /* Enable DAC */
    DAC->CTRLA.bit.ENABLE = 1;
    // Wait for synchronization
#if defined(SAMD2x)
    while (DAC->STATUS.bit.SYNCBUSY);
#elif defined(SAMx5x)
    while (DAC->SYNCBUSY.bit.ENABLE);
#endif

#if defined(SAMx5x)
    for (uint8_t i = 0; i < DAC_CHANNEL_SIZE; i++) {
        if (channel_mask & (1 << i)) {
            // Wait for DAC channel to be ready
            while (!(DAC->STATUS.reg & (1 << i)));

            // Set DAC value to 0, otherwise it will start slowly drifting
            // upwards.
            DAC->DATA[i].reg = 0;
        }
    }
#endif
}

void dac_set (uint8_t chan, uint16_t value)
{
    if (chan >= DAC_NUM_CHANNELS) {
        return;
    }
#if defined(SAMD2x)
    DAC->DATA.reg = value;
#elif defined(SAMx5x)
    DAC->DATA[chan].reg = value;
#endif
}

void dac_set_millivolts (uint8_t chan, uint16_t millivolts)
{
    uint32_t value = (uint32_t)millivolts * 65535;
#if defined(SAMD2x)
    value /= (DAC->CTRLB.bit.REFSEL == DAC_CTRLB_REFSEL_INT1V_Val) ? 1000 : 3300;
#elif defined(SAMx5x)
    value /= (DAC->CTRLB.bit.REFSEL == DAC_CTRLB_REFSEL_INTREF_Val) ? 1000 : 3300;
#endif
    value = (value > UINT16_MAX) ? UINT16_MAX : value;

    dac_set(chan, (uint16_t)value);
}

uint16_t dac_get_value (uint8_t chan)
{
    if (chan >= DAC_NUM_CHANNELS) {
        return 0;
    }
#if defined(SAMD2x)
    return DAC->DATA.reg;
#elif defined(SAMx5x)
    return DAC->DATA[chan].reg;
#endif
}

uint16_t dac_get_value_millivolts (uint8_t chan)
{
#if defined(SAMD2x)
    uint32_t r = (DAC->CTRLB.bit.REFSEL == DAC_CTRLB_REFSEL_INT1V_Val) ? 1000 : 3300;
#elif defined(SAMx5x)
    uint32_t r = (DAC->CTRLB.bit.REFSEL == DAC_CTRLB_REFSEL_INTREF_Val) ? 1000 : 3300;
#endif
    return (uint16_t)(((uint32_t)dac_get_value(chan) * r) / 65535);
}
