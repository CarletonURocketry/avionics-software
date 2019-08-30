/**
 * @file dac.c
 * @desc DAC Driver
 * @author Samuel Dewan
 * @date 2019-08-28
 * Last Author:
 * Last Edited On:
 */

#include "dac.h"

#include "config.h"

void init_dac (uint32_t clock_mask, enum dac_reference reference,
               uint8_t enable_int_output, uint8_t enable_ext_output)
{
    /* Enable the APBC clock for the DAC */
    PM->APBCMASK.reg |= PM_APBCMASK_DAC;
    
    /* Select the core clock for the DAC */
    GCLK->CLKCTRL.reg = (GCLK_CLKCTRL_CLKEN | clock_mask | GCLK_CLKCTRL_ID_DAC);
    // Wait for synchronization
    while (GCLK->STATUS.bit.SYNCBUSY);
    
    /* Set pin multiplex for DAC pin */
    uint8_t port = DAC_OUT.internal.port;
    uint8_t pin = DAC_OUT.internal.pin;
    PORT->Group[port].PMUX[pin / 2].bit.PMUXE = 0x1;
    PORT->Group[port].PINCFG[pin].bit.PMUXEN = 0b1;
    
    /* Reset DAC */
    DAC->CTRLA.bit.SWRST = 1;
    // Wait for reset to complete
    while (DAC->CTRLA.bit.SWRST | DAC->STATUS.bit.SYNCBUSY);
    
    /* Configure DAC */
    DAC->CTRLB.reg = (((reference == DAC_REF_1V) ? DAC_CTRLB_REFSEL_INT1V :
                       DAC_CTRLB_REFSEL_AVCC) |
                      DAC_CTRLB_LEFTADJ |
                      (enable_int_output ? DAC_CTRLB_IOEN : 0) |
                      (enable_ext_output ? DAC_CTRLB_EOEN : 0));
    // Wait for synchronization
    while (DAC->CTRLA.bit.SWRST | DAC->STATUS.bit.SYNCBUSY);
    
    /* Enable DAC */
    DAC->CTRLA.bit.ENABLE = 1;
    // Wait for synchronization
    while (DAC->CTRLA.bit.SWRST | DAC->STATUS.bit.SYNCBUSY);
}

void dac_set (uint16_t value)
{
    DAC->DATA.reg = value;
}

void dac_set_millivolts (uint16_t millivolts)
{
    uint32_t value = (uint32_t)millivolts * 65535;
    value /= (DAC->CTRLB.bit.REFSEL == DAC_CTRLB_REFSEL_INT1V_Val) ? 1000 : 3300;
    
    DAC->DATA.reg = (uint16_t)((value > UINT16_MAX) ? UINT16_MAX : value);
}

uint16_t dac_get_value (void)
{
    return DAC->DATA.reg;
}

uint16_t dac_get_value_millivolts (void)
{
    uint32_t r = (DAC->CTRLB.bit.REFSEL == DAC_CTRLB_REFSEL_INT1V_Val) ? 1000 : 3300;
    return (uint16_t)(((uint32_t)DAC->DATA.reg * r) / 65535);
}
