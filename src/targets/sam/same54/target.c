/**
 * @file target.c
 * @desc Target specific functions for SAME54
 * @author Samuel Dewan
 * @date 2021-03-08
 * Last Author:
 * Last Edited On:
 */

#include "target.h"

#include "board.h"
#include "dma.h"

/* Initialization functions */

static void init_clocks (void)
{
    /* Ensure that interface clocks for GLCK, OSCCTRL and OSC32KCTL are
     enabled (they should be by default at reset) */
    MCLK->APBAMASK.reg |= (MCLK_APBAMASK_GCLK | MCLK_APBAMASK_OSCCTRL |
                           MCLK_APBAMASK_OSC32KCTRL);


    /* Enable the 32.768 KHz external oscillator */
    // High gain (for ESR up to 90 kΩ), 1 second startup time, 32.768 KHz output
    // enabled.
    OSC32KCTRL->XOSC32K.reg = (OSC32KCTRL_XOSC32K_CGM_HS |
                               OSC32KCTRL_XOSC32K_STARTUP(0x3) |
                               OSC32KCTRL_XOSC32K_EN32K |
                               OSC32KCTRL_XOSC32K_XTALEN);
    // Enable automatic switching to OSCULP32K if XOSC32K fails
    OSC32KCTRL->CFDCTRL.reg = (OSC32KCTRL_CFDCTRL_SWBACK |
                               OSC32KCTRL_CFDCTRL_CFDEN);
    // Source RTC clock from XOSC32K 32.768 KHz output
    OSC32KCTRL->RTCCTRL.reg = OSC32KCTRL_RTCCTRL_RTCSEL_XOSC32K;
    // Enable XOSC32K
    OSC32KCTRL->XOSC32K.bit.ENABLE = 1;
    // Wait for oscillator to be ready
    while (!(OSC32KCTRL->STATUS.reg & OSC32KCTRL_STATUS_XOSC32KRDY));


    /* Reset Generic Clock Controller */
    GCLK->CTRLA.reg = GCLK_CTRLA_SWRST;
    // Wait for reset to complete
    while (GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_SWRST);


    /* Configure Generic Clock Generator 3 with XOSC32K as source */
    // Do not divide, XOSC32K as source
    GCLK->GENCTRL[3].reg = (GCLK_GENCTRL_SRC_XOSC32K | GCLK_GENCTRL_GENEN);
    // Wait for generic clock generator 3 to be ready
    while (GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL3);


    /* Configure TC0 */
    // We need TC0 to wait for the FDPLLs to lock due to silicon errata 2.13.1
    // Enable bus clock for TC0
    MCLK->APBAMASK.bit.TC0_ = 1;
    // Configure generic clock for TC0 to use GCLK_GEN3 (XOSC32K)
    GCLK->PCHCTRL[TC0_GCLK_ID].reg = (GCLK_PCHCTRL_CHEN |
                                      GCLK_PCHCTRL_GEN_GCLK3);
    // Reset TC0
    TC0->COUNT16.CTRLA.bit.SWRST = 1;
    while (TC0->COUNT16.SYNCBUSY.bit.SWRST);
    // Configure TC0 in 16 bit mode for one shot operation counting from 0 to
    // CC0. Configure to count to 387 for ~10 ms delay.
    TC0->COUNT16.CTRLA.reg = TC_CTRLA_MODE_COUNT16;
    TC0->COUNT16.CTRLBSET.reg = TC_CTRLBSET_ONESHOT;
    while (TC0->COUNT16.SYNCBUSY.bit.CTRLB);
    TC0->COUNT16.WAVE.reg = TC_WAVE_WAVEGEN_MFRQ;
    TC0->COUNT16.CC[0].reg = 328;
    while (TC0->COUNT16.SYNCBUSY.bit.CC0);


    /* Configure the FDPLLx internal lock timer clock to use GCLK_GEN3
       (XOSC32K) */
    // Note: this peripheral clock channel serves several peripherals:
    //       FDPLL0 32KHz clock for internal lock timer, FDPLL1 32KHz clock for
    //       internal lock timer, SDHC0 Slow, SDHC1 Slow, SERCOM[0..7] Slow
    GCLK->PCHCTRL[3].reg = (GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN_GCLK3);


    /* Configure DPLL0 to generate 120 MHz clock with XOSC32K as reference */
    // Set Loop Divider Ratio
    // Fdplln = Fckr * (LDR + 1 + (LDRFRAC/32))
    OSCCTRL->Dpll[0].DPLLRATIO.reg = (OSCCTRL_DPLLRATIO_LDR(3661) |
                                      OSCCTRL_DPLLRATIO_LDRFRAC(3));
    // Wait for synchronization of ratio register
    while (OSCCTRL->Dpll[0].DPLLSYNCBUSY.reg & OSCCTRL_DPLLSYNCBUSY_DPLLRATIO);
    // Reference clock is XOSC32K, enable wake up fast and lock bypass to work
    // around silicon errata.
    OSCCTRL->Dpll[0].DPLLCTRLB.reg = (OSCCTRL_DPLLCTRLB_REFCLK_XOSC32 |
                                      OSCCTRL_DPLLCTRLB_WUF |
                                      OSCCTRL_DPLLCTRLB_LBYPASS);
    // Enable DPLL0
    OSCCTRL->Dpll[0].DPLLCTRLA.reg = OSCCTRL_DPLLCTRLA_ENABLE;
    // Wait for DPLL0 to be ready
    while (!(OSCCTRL->Dpll[0].DPLLSTATUS.reg & OSCCTRL_DPLLSTATUS_CLKRDY));
    // Wait an extra 10 ms with TC0 to make sure that DPLL is locked
    TC0->COUNT16.CTRLA.bit.ENABLE = 1;
    while (!TC0->COUNT16.INTFLAG.bit.OVF);
    TC0->COUNT16.INTFLAG.reg = TC_INTFLAG_OVF;
    TC0->COUNT16.CTRLA.bit.ENABLE = 0;
    while (TC0->COUNT16.SYNCBUSY.bit.ENABLE);
    TC0->COUNT16.COUNT.reg = 0;
    while (TC0->COUNT16.SYNCBUSY.bit.COUNT);


    /* Switch Generic Clock Generator 0 source to DPLL0 (this is the CPU
       clock) */
    // Do not divide, DPLL0 as source
    GCLK->GENCTRL[0].reg = (GCLK_GENCTRL_SRC_DPLL0 | GCLK_GENCTRL_GENEN);
    // Wait for generic clock generator 0 to be ready
    while (GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL0);


    /* Configure DFLL48M input clock source to use GCLK_GEN3 (XOSC32K) */
    GCLK->PCHCTRL[0].reg = (GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN_GCLK3);


    /* Reconfigure DFLL48M in closed loop mode */
    // Disable DFLL48M
    OSCCTRL->DFLLCTRLA.bit.ENABLE = 0;
    // Wait for DFLL48M to be disabled
    while (OSCCTRL->DFLLSYNC.reg & OSCCTRL_DFLLSYNC_ENABLE);
    // Put DFLL48M in closed loop mode
    OSCCTRL->DFLLCTRLB.bit.MODE = 1;
    // Set course and fine steps to one quarter of their max values (16 and 256)
    // Configure DFLL multiplication value to generate 48 MHz clock from 32.768
    // KHz input
    OSCCTRL->DFLLMUL.reg = (OSCCTRL_DFLLMUL_CSTEP(16) |
                            OSCCTRL_DFLLMUL_FSTEP(256) |
                            OSCCTRL_DFLLMUL_MUL(48000000 / 32768));
    // Wait for synchronization of DFLLMUL register
    while (OSCCTRL->DFLLSYNC.reg & OSCCTRL_DFLLSYNC_DFLLMUL);
    // Enable DFLL48M and clear on-demand bit
    OSCCTRL->DFLLCTRLA.reg = OSCCTRL_DFLLCTRLA_ENABLE;
    // Wait for locks
    while (!OSCCTRL->STATUS.bit.DFLLLCKC || !OSCCTRL->STATUS.bit.DFLLLCKF);
    // Wait for DFLL48M to be ready
    while (!OSCCTRL->STATUS.bit.DFLLRDY);


    /* Configure Generic Clock Generator 2 with DFLL as source */
    // Do not divide, DFLL as source
    GCLK->GENCTRL[2].reg = (GCLK_GENCTRL_SRC_DFLL | GCLK_GENCTRL_GENEN);
    // Wait for generic clock generator 2 to be ready
    while (GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL2);


#ifdef ENABLE_XOSC0
    // XOSC0 is enabled, use it to provide a 12 MHz clock on GCLK_GEN 4.

    /* Enable XOSC0 */
    // Clock failure detector prescaler set to 4, start up time of 244 µs,
    // clock failure detection is enabled, automatic loop control enabled,
    // current multiplier and reference set according to Table 28-7 for a 12 MHz
    // crystal, on demand enabled, crystal pads connected.
    OSCCTRL->XOSCCTRL[0].reg = (OSCCTRL_XOSCCTRL_CFDPRESC(2) |
                                OSCCTRL_XOSCCTRL_STARTUP(0x3) |
                                OSCCTRL_XOSCCTRL_CFDEN |
                                OSCCTRL_XOSCCTRL_ENALC |
                                OSCCTRL_XOSCCTRL_IMULT(4) |
                                OSCCTRL_XOSCCTRL_IPTAT(3) |
                                OSCCTRL_XOSCCTRL_ONDEMAND |
                                OSCCTRL_XOSCCTRL_XTALEN);
    // Enable XOSC0
    OSCCTRL->XOSCCTRL[0].bit.ENABLE = 1;
    // We don't wait for the clock to be running because it is on demand, it
    // won't start until something requests it.


    /* Configure Generic Clock Generator 4 with XOSC0 as source */
    // Do not divide, XOSC0 as source
    GCLK->GENCTRL[4].reg = (GCLK_GENCTRL_SRC_XOSC0 | GCLK_GENCTRL_GENEN);
    // Wait for generic clock generator 4 to be ready
    while (GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL4);
#else
    // XOSC0 is not enabled, provide a 12 MHz clock on GCLK_GEN 4 by dividing
    // DFLL48M by 4.

    /* Configure Generic Clock Generator 4 with DFLL48M as source div by 4 */
    // Divide by 4, DFLL48M as source
    GCLK->GENCTRL[4].reg = (GCLK_GENCTRL_DIV(4) | GCLK_GENCTRL_SRC_DFLL |
                            GCLK_GENCTRL_GENEN);
    // Wait for generic clock generator 4 to be ready
    while (GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL4);
#endif


    /* Configure DPLL1 to generate 100 MHz clock with XOSC32K as reference */
    // Set Loop Divider Ratio
    // Fdplln = Fckr * (LDR + 1 + (LDRFRAC/32))
    OSCCTRL->Dpll[1].DPLLRATIO.reg = (OSCCTRL_DPLLRATIO_LDR(3050) |
                                      OSCCTRL_DPLLRATIO_LDRFRAC(24));
    // Wait for synchronization of ratio register
    while (OSCCTRL->Dpll[1].DPLLSYNCBUSY.reg & OSCCTRL_DPLLSYNCBUSY_DPLLRATIO);
    // Reference clock is XOSC32K, enable wake up fast and lock bypass to work
    // around silicon errata.
    OSCCTRL->Dpll[1].DPLLCTRLB.reg = (OSCCTRL_DPLLCTRLB_REFCLK_XOSC32 |
                                      OSCCTRL_DPLLCTRLB_WUF |
                                      OSCCTRL_DPLLCTRLB_LBYPASS);
    // Enable DPLL1
    OSCCTRL->Dpll[1].DPLLCTRLA.reg = OSCCTRL_DPLLCTRLA_ENABLE;
    // Wait for DPLL1 to be ready
    while (!(OSCCTRL->Dpll[1].DPLLSTATUS.reg & OSCCTRL_DPLLSTATUS_CLKRDY));
    // Wait an extra 10 ms with TC0 to make sure that DPLL is locked
    TC0->COUNT16.CTRLA.bit.ENABLE = 1;
    while (!TC0->COUNT16.INTFLAG.bit.OVF);
    TC0->COUNT16.INTFLAG.reg = TC_INTFLAG_OVF;
    TC0->COUNT16.CTRLA.bit.ENABLE = 0;
    while (TC0->COUNT16.SYNCBUSY.bit.ENABLE);
    TC0->COUNT16.COUNT.reg = 0;
    while (TC0->COUNT16.SYNCBUSY.bit.COUNT);


    /* Configure Generic Clock Generator 5 with DPLL1 as source */
    // Do not divide, DPLL1 as source
    GCLK->GENCTRL[5].reg = (GCLK_GENCTRL_SRC_DPLL1 | GCLK_GENCTRL_GENEN);
    // Wait for generic clock generator 5 to be ready
    while (GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL5);


    /* Clock output for debugging */
//    // Configure generic clock generator 1 with clock to be outputted
//    GCLK->GENCTRL[1].reg = (GCLK_GENCTRL_SRC_DPLL1 | GCLK_GENCTRL_GENEN |
//                            GCLK_GENCTRL_OE | GCLK_GENCTRL_DIV(4));
//    // Wait for generic clock generator 1 to be ready
//    while (GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL1);
//    // Setup output pin
//    PORT->Group[1].PMUX[11].bit.PMUXO = 12;
//    PORT->Group[1].PINCFG[23].bit.PMUXEN = 1;


    /* Reset TC0 */
    TC0->COUNT16.CTRLA.bit.SWRST = 1;
    while (TC0->COUNT16.SYNCBUSY.bit.SWRST);
    // Disable generic clock for TC0
    GCLK->PCHCTRL[TC0_GCLK_ID].reg = 0;
    // Disable bus clock for TC0
    MCLK->APBAMASK.bit.TC0_ = 0;

}

void init_target(void)
{
    /* Wait for voltage to rise to 3.3 volts */
    // Ensure that the interface clock for the SUPC is enabled
    MCLK->APBAMASK.reg |= MCLK_APBAMASK_SUPC;
    // Disable BOD33 to make sure that we don't get anything funny happening
    // while we configure it
    SUPC->BOD33.bit.ENABLE = 0;
    while (!SUPC->STATUS.bit.B33SRDY);
    // Configure BOD33 to have no action and enable it
    SUPC->BOD33.reg &= ~SUPC_BOD33_ACTION_Msk;
    SUPC->BOD33.reg |= (SUPC_BOD33_ACTION_NONE |
                        SUPC_BOD33_ENABLE);
    while (!SUPC->STATUS.bit.B33SRDY);
    // Wait for supply voltage to rise
    while (SUPC->STATUS.bit.BOD33DET);


    /* Configure clocks */
    // We start up running at 48 MHz from DFLL48M in open loop mode
    init_clocks();
    // Now we should be running at 120 MHz


    /* Configure sleep mode */
    // Ensure that the interface clock for the PM is enabled
    MCLK->APBAMASK.reg |= MCLK_APBAMASK_PM;
    // Set sleep mode to IDLE2 (CPU, AHBx and APBx clocks are disabled)
    PM->SLEEPCFG.reg = PM_SLEEPCFG_SLEEPMODE_IDLE2;


    /* Configure BOD33 to switch backup domain to backup power */
    // Disable BOD33 to make sure that we don't get anything funny happening
    // while we configure it
    SUPC->BOD33.bit.ENABLE = 0;
    while (!SUPC->STATUS.bit.B33SRDY);
    // Clear BOD33 action
    SUPC->BOD33.reg &= ~SUPC_BOD33_ACTION_Msk;
    // Enable BOD33 and configure it to switch backup domain to battery power
    SUPC->BOD33.reg |= (SUPC_BOD33_ACTION_BKUP |
                        SUPC_BOD33_ENABLE);
    while (!SUPC->STATUS.bit.B33SRDY);


    /* Enable Cache */
    if (!CMCC->SR.bit.CSTS) {
        CMCC->CTRL.reg = CMCC_CTRL_CEN;
    }


    /* Configure RTC to be used as millis  */
    // Ensure that the interface clock for the RTC is enabled
    MCLK->APBAMASK.reg |= MCLK_APBAMASK_RTC;
    // Store the backup register values (since they will be cleared when we
    // reset the RTC)
    uint32_t bk_vals[8];
    for (int i = 0; i < 8; i++) {
        bk_vals[i] = RTC->MODE0.BKUP[i].reg;
    }
    // Rest RTC
    RTC->MODE0.CTRLA.bit.SWRST = 1;
    while (RTC->MODE0.CTRLA.bit.SWRST);
    // Restore the backup register values
    for (int i = 0; i < 8; i++) {
        RTC->MODE0.BKUP[i].reg = bk_vals[i];
    }
    // Configure RTC with count register synchronization enabled and a prescaler
    // of 32 in 32 bit counter mode.
    RTC->MODE0.CTRLA.reg = (RTC_MODE0_CTRLA_COUNTSYNC |
                            RTC_MODE0_CTRLA_PRESCALER_DIV32 |
                            RTC_MODE0_CTRLA_MODE_COUNT32);
    // Wait for write synchronization of COUNTSYNC bit
    while (RTC->MODE0.SYNCBUSY.bit.COUNTSYNC);
    // Use compare registers as general purpose registers
    RTC->MODE0.CTRLB.reg = (RTC_MODE0_CTRLB_GP2EN | RTC_MODE0_CTRLB_GP0EN);
    // Enable RTC
    RTC->MODE0.CTRLA.bit.ENABLE = 1;
    // Wait for write synchronization of ENABLE bit
    while (RTC->MODE0.SYNCBUSY.bit.ENABLE);
    // Wait for synchronization of the count register to complete and read from
    // it. The first value synchronized to the register will not be valid, so we
    // need to discard it.
    while (RTC->MODE0.SYNCBUSY.bit.COUNT);
    volatile uint32_t first_count = RTC->MODE0.COUNT.reg;
    (void)first_count; // Convince compiler that this unused variable is ok


    /* Load ADC factory calibration values */
    // ADC0 Bias Calibration
    uint32_t adc_bias_ref = ((*((uint32_t *)ADC0_FUSES_BIASREFBUF_ADDR) &
                              ADC0_FUSES_BIASREFBUF_Msk) >>
                             ADC0_FUSES_BIASREFBUF_Pos);
    uint32_t adc_bias_r2r = ((*((uint32_t *)ADC0_FUSES_BIASR2R_ADDR) &
                              ADC0_FUSES_BIASR2R_Msk) >>
                             ADC0_FUSES_BIASR2R_Pos);
    uint32_t adc_bias_comp = ((*((uint32_t *)ADC0_FUSES_BIASCOMP_ADDR) &
                               ADC0_FUSES_BIASCOMP_Msk) >>
                              ADC0_FUSES_BIASCOMP_Pos);
    ADC0->CALIB.reg = (ADC_CALIB_BIASREFBUF(adc_bias_ref) |
                       ADC_CALIB_BIASR2R(adc_bias_r2r) |
                       ADC_CALIB_BIASCOMP(adc_bias_comp));
    // ADC1 Bias Calibration
    adc_bias_ref = ((*((uint32_t *)ADC1_FUSES_BIASREFBUF_ADDR) &
                     ADC1_FUSES_BIASREFBUF_Msk) >> ADC1_FUSES_BIASREFBUF_Pos);
    adc_bias_r2r = ((*((uint32_t *)ADC1_FUSES_BIASR2R_ADDR) &
                     ADC1_FUSES_BIASR2R_Msk) >> ADC1_FUSES_BIASR2R_Pos);
    adc_bias_comp = ((*((uint32_t *)ADC1_FUSES_BIASCOMP_ADDR) &
                      ADC1_FUSES_BIASCOMP_Msk) >> ADC1_FUSES_BIASCOMP_Pos);
    ADC1->CALIB.reg = (ADC_CALIB_BIASREFBUF(adc_bias_ref) |
                       ADC_CALIB_BIASR2R(adc_bias_r2r) |
                       ADC_CALIB_BIASCOMP(adc_bias_comp));

    // Init DMA
    init_dmac();

    // Enable SysTick for an interrupt every 10 milliseconds, this is just to
    // make sure that we wake the main loop occasionally, not for time keeping
    SysTick_Config(12000);
    // Give SysTick interrupt lowest priority
    NVIC_SetPriority(SysTick_IRQn, 7);
}

/* Clock control functions */
void enable_bus_clock(enum peripheral_bus_clock clock)
{
    switch (PERPH_BUS_CLK_BUS(clock)) {
        case PERPH_BUS_CLK_BUS_AHB:
            MCLK->AHBMASK.reg |= (1 << PERPH_BUS_CLK_BIT(clock));
            break;
        case PERPH_BUS_CLK_BUS_APBA:
            MCLK->APBAMASK.reg |= (1 << PERPH_BUS_CLK_BIT(clock));
            break;
        case PERPH_BUS_CLK_BUS_APBB:
            MCLK->APBBMASK.reg |= (1 << PERPH_BUS_CLK_BIT(clock));
            break;
        case PERPH_BUS_CLK_BUS_APBC:
            MCLK->APBCMASK.reg |= (1 << PERPH_BUS_CLK_BIT(clock));
            break;
        case PERPH_BUS_CLK_BUS_APBD:
            MCLK->APBDMASK.reg |= (1 << PERPH_BUS_CLK_BIT(clock));
            break;
        default:
            break;
    }
}

void disable_bus_clock(enum peripheral_bus_clock clock)
{
    switch (PERPH_BUS_CLK_BUS(clock)) {
        case PERPH_BUS_CLK_BUS_AHB:
            MCLK->AHBMASK.reg &= ~(1 << PERPH_BUS_CLK_BIT(clock));
            break;
        case PERPH_BUS_CLK_BUS_APBA:
            MCLK->APBAMASK.reg &= ~(1 << PERPH_BUS_CLK_BIT(clock));
            break;
        case PERPH_BUS_CLK_BUS_APBB:
            MCLK->APBBMASK.reg &= ~(1 << PERPH_BUS_CLK_BIT(clock));
            break;
        case PERPH_BUS_CLK_BUS_APBC:
            MCLK->APBCMASK.reg &= ~(1 << PERPH_BUS_CLK_BIT(clock));
            break;
        case PERPH_BUS_CLK_BUS_APBD:
            MCLK->APBDMASK.reg &= ~(1 << PERPH_BUS_CLK_BIT(clock));
            break;
        default:
            break;
    }
}

void set_perph_generic_clock(enum peripheral_generic_clock channel,
                             uint32_t clock_mask)
{
    do {
        GCLK->PCHCTRL[channel].reg = GCLK_PCHCTRL_CHEN | clock_mask;
    } while(!GCLK->PCHCTRL[channel].bit.CHEN);
}


void SysTick_Handler(void)
{
    return;
}

void HardFault_Handler(void)
{
    uint8_t port = DEBUG0_LED_PIN.internal.port;
    uint32_t mask = (1 << DEBUG0_LED_PIN.internal.pin);

    PORT->Group[port].DIRSET.reg = mask;

    for (;;) {
        PORT->Group[port].OUTSET.reg = mask;
        uint64_t n = 3000000;
        while (n--) {
            asm volatile ("");
        }
        PORT->Group[port].OUTCLR.reg = mask;
        n = 30000000;
        while (n--) {
            asm volatile ("");
        }
    }
}
