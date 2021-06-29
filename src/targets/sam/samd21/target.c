/**
 * @file target.c
 * @desc Target specific functions for SAMD21
 * @author Samuel Dewan
 * @date 2020-08-30
 * Last Author:
 * Last Edited On:
 */

#include "target.h"

#include "board.h"
#include "dma.h"

// Micro Trace Buffer
#ifdef ENABLE_MTB
// Stores 2 ^ TRACE_BUFFER_MAGNITUDE_PACKETS packets.
// 4 -> 16 packets
#define TRACE_BUFFER_MAGNITUDE_PACKETS 4
// Size in uint32_t. Two per packet.
#define TRACE_BUFFER_SIZE (1 << (TRACE_BUFFER_MAGNITUDE_PACKETS + 1))
// Size in bytes. 8 bytes per packet.
#define TRACE_BUFFER_SIZE_BYTES (TRACE_BUFFER_SIZE << 3)
__attribute__((__aligned__(TRACE_BUFFER_SIZE_BYTES))) uint32_t mtb[TRACE_BUFFER_SIZE];
#endif

volatile uint32_t millis;

/* Initialization functions */

static void init_clocks (void)
{
    /* Configure a single flash wait state, good for 2.7-3.3v operation at
        48MHz */
    // See section 37.12 of datasheet (NVM Characteristics)
    NVMCTRL->CTRLB.bit.RWS = NVMCTRL_CTRLB_RWS_HALF_Val;

    // Ensure that interface clock for generic clock controller is enabled
    PM->APBAMASK.reg |= PM_APBAMASK_GCLK;

    /* Enable external 32.768 KHz oscillator */
    // 1000092μs (32768 OSCULP32K cycles) startup time, enable crystal,
    // enable 32.768 KHz output
    SYSCTRL->XOSC32K.reg = (SYSCTRL_XOSC32K_STARTUP( 0x5 ) |
                            SYSCTRL_XOSC32K_XTALEN | SYSCTRL_XOSC32K_EN32K);
    // Enable oscillator (note: enabled must not be set with other bits)
    SYSCTRL->XOSC32K.bit.ENABLE = 1;
    // Wait for oscillator stabilization (about 1 second)
    while (!(SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_XOSC32KRDY));

    /* Reset Generic Clock Controller */
    GCLK->CTRL.reg = GCLK_CTRL_SWRST;
    // Wait for reset to complete
    while (GCLK->CTRL.bit.SWRST);

    /* Configure Generic Clock Generator 1 */
    // Set division factor to 0 (no division)
    GCLK->GENDIV.reg = GCLK_GENDIV_DIV(0) | GCLK_GENDIV_ID(1);
    // Wait for synchronization
    while (GCLK->STATUS.bit.SYNCBUSY);
    // Write Generic Clock Generator 1 configuration
    // source from XOSC32K and enable
    GCLK->GENCTRL.reg = (GCLK_GENCTRL_ID(1) | GCLK_GENCTRL_SRC_XOSC32K |
                         GCLK_GENCTRL_GENEN);
    // Wait for synchronization
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

    /* Configure Generic Clock Multiplexer for DFLL48M to use Generic Clock
        Generator 1 */
    GCLK->CLKCTRL.reg = (GCLK_CLKCTRL_ID_DFLL48 | GCLK_CLKCTRL_GEN_GCLK1 |
                         GCLK_CLKCTRL_CLKEN);
    // Wait for synchronization
    while (GCLK->STATUS.bit.SYNCBUSY);

    /* Enable DFLL48M */
    // Disable On Demand mode before configuration
    //      see silicon errata section 1.2.1 - Write Access to DFLL Register
    SYSCTRL->DFLLCTRL.bit.ONDEMAND = 0;
    // Wait for DFLL48M to be ready
    while (!SYSCTRL->PCLKSR.bit.DFLLRDY);

    /* Configure the DFLL48M */
    // Configuration for DFLL in Closed Loop mode
    //      see datasheet section 17.6.7.1.2 - Closed-Loop Operation

    // Set course and fine steps to one quarter of their max values (16 and 256)
    // Configure DFLL multiplication value to generate 48 MHz clock from 32.768
    // KHz input
    SYSCTRL->DFLLMUL.reg = (SYSCTRL_DFLLMUL_CSTEP( 16 ) |
                            SYSCTRL_DFLLMUL_FSTEP( 256 ) |
                            SYSCTRL_DFLLMUL_MUL(48000000 / 32768));
    // Wait for DFLL48M to be ready
    while (!SYSCTRL->PCLKSR.bit.DFLLRDY);

    // Enabled closed loop mode, wait for lock and disable quick lock
    SYSCTRL->DFLLCTRL.reg |= (SYSCTRL_DFLLCTRL_MODE |
                              SYSCTRL_DFLLCTRL_WAITLOCK |
                              SYSCTRL_DFLLCTRL_QLDIS);
    // Wait for DFLL48M to be ready
    while (!SYSCTRL->PCLKSR.bit.DFLLRDY);

    // Enable DFLL48M
    SYSCTRL->DFLLCTRL.bit.ENABLE = 1;
    // Wait for locks
    while (!SYSCTRL->PCLKSR.bit.DFLLLCKC || ! SYSCTRL->PCLKSR.bit.DFLLLCKF);
    // Wait for DFLL48M to be ready
    while (!SYSCTRL->PCLKSR.bit.DFLLRDY);

    /* Configure Generic Clock Generator 0 */
    // Source from DFLL48M to run CPU at 48 MHz
    // Set division factor to 0 (no division)
    GCLK->GENDIV.reg = GCLK_GENDIV_DIV(0) | GCLK_GENDIV_ID(0);
    // Wait for synchronization
    while (GCLK->STATUS.bit.SYNCBUSY);
    // Write Generic Clock Generator 0 configuration
    // source from DFLL48M, enable improved (50/50) duty cycle and enable
    GCLK->GENCTRL.reg = (GCLK_GENCTRL_ID(0) | GCLK_GENCTRL_SRC_DFLL48M |
                         GCLK_GENCTRL_IDC | GCLK_GENCTRL_GENEN);
    // Wait for synchronization
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

    /* Configure OSC8M */
    // Set Prescaler to generate 8 MHz
    SYSCTRL->OSC8M.bit.PRESC = SYSCTRL_OSC8M_PRESC_0_Val;

    /* Configure Generic Clock Generator 3 */
    // Set division factor to 0 (no division)
    GCLK->GENDIV.reg = GCLK_GENDIV_DIV(0) | GCLK_GENDIV_ID(3);
    // Wait for synchronization
    while (GCLK->STATUS.bit.SYNCBUSY);
    // Write Generic Clock Generator 3 configuration
    // source from OSC8M, and enable
    GCLK->GENCTRL.reg = (GCLK_GENCTRL_ID(3) | GCLK_GENCTRL_SRC_OSC8M |
                         GCLK_GENCTRL_GENEN);
    // Wait for synchronization
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

    /* Configure Generic Clock Generator 7 */
    // Set division factor to 2 (divide by 2^2 = 4)
    GCLK->GENDIV.reg = GCLK_GENDIV_DIV(2) | GCLK_GENDIV_ID(7);
    // Wait for synchronization
    while (GCLK->STATUS.bit.SYNCBUSY);
    // Write Generic Clock Generator 7 configuration
    // source from OSCULP32K, and enable
    GCLK->GENCTRL.reg = (GCLK_GENCTRL_ID(7) | GCLK_GENCTRL_SRC_OSCULP32K |
                         GCLK_GENCTRL_GENEN);
    // Wait for synchronization
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);
}

void init_target(void)
{
    init_clocks();

    // Enable SysTick for an interrupt every millisecond
    SysTick_Config(48000);
    // Give SysTick interrupt highest priority
    NVIC_SetPriority(SysTick_IRQn, 0);

    // Load ADC factory calibration values
    // ADC Bias Calibration
    uint32_t bias = (*((uint32_t *) ADC_FUSES_BIASCAL_ADDR) &
                     ADC_FUSES_BIASCAL_Msk) >> ADC_FUSES_BIASCAL_Pos;
    // ADC Linearity bits 4:0
    uint32_t linearity = (*((uint32_t *) ADC_FUSES_LINEARITY_0_ADDR) &
                        ADC_FUSES_LINEARITY_0_Msk) >> ADC_FUSES_LINEARITY_0_Pos;
    // ADC Linearity bits 7:5
    linearity |= ((*((uint32_t *) ADC_FUSES_LINEARITY_1_ADDR) &
                 ADC_FUSES_LINEARITY_1_Msk) >> ADC_FUSES_LINEARITY_1_Pos) << 5;
    ADC->CALIB.reg = (ADC_CALIB_BIAS_CAL(bias) |
                      ADC_CALIB_LINEARITY_CAL(linearity));

    // Disable automatic NVM write operations
    NVMCTRL->CTRLB.bit.MANW = 1;

#ifdef ENABLE_MTB
    // Enable Micro Trace Buffer (MTB) as described here:
    //      https://github.com/adafruit/gdb-micro-trace-buffer
    MTB->POSITION.reg = ((uint32_t) (mtb - REG_MTB_BASE)) & 0xFFFFFFF8;
    MTB->FLOW.reg = (((uint32_t) mtb - REG_MTB_BASE) +
                     TRACE_BUFFER_SIZE_BYTES) & 0xFFFFFFF8;
    MTB->MASTER.reg = 0x80000000 + (TRACE_BUFFER_MAGNITUDE_PACKETS - 1);
#endif

    // Init DMA
    init_dmac();
}


/* Clock control functions */
void enable_bus_clock(enum peripheral_bus_clock clock)
{
    switch (PERPH_BUS_CLK_BUS(clock)) {
        case PERPH_BUS_CLK_BUS_AHB:
            PM->AHBMASK.reg |= (1 << PERPH_BUS_CLK_BIT(clock));
            break;
        case PERPH_BUS_CLK_BUS_APBA:
            PM->APBAMASK.reg |= (1 << PERPH_BUS_CLK_BIT(clock));
            break;
        case PERPH_BUS_CLK_BUS_APBB:
            PM->APBBMASK.reg |= (1 << PERPH_BUS_CLK_BIT(clock));
            break;
        case PERPH_BUS_CLK_BUS_APBC:
            PM->APBCMASK.reg |= (1 << PERPH_BUS_CLK_BIT(clock));
            break;
        default:
            break;
    }
}

void disable_bus_clock(enum peripheral_bus_clock clock)
{
    switch (PERPH_BUS_CLK_BUS(clock)) {
        case PERPH_BUS_CLK_BUS_AHB:
            PM->AHBMASK.reg &= ~(1 << PERPH_BUS_CLK_BIT(clock));
            break;
        case PERPH_BUS_CLK_BUS_APBA:
            PM->APBAMASK.reg &= ~(1 << PERPH_BUS_CLK_BIT(clock));
            break;
        case PERPH_BUS_CLK_BUS_APBB:
            PM->APBBMASK.reg &= ~(1 << PERPH_BUS_CLK_BIT(clock));
            break;
        case PERPH_BUS_CLK_BUS_APBC:
            PM->APBCMASK.reg &= ~(1 << PERPH_BUS_CLK_BIT(clock));
            break;
        default:
            break;
    }
}

void set_perph_generic_clock(enum peripheral_generic_clock channel,
                             uint32_t clock_mask)
{
    GCLK->CLKCTRL.reg = (GCLK_CLKCTRL_CLKEN | clock_mask |
                         GCLK_CLKCTRL_ID(channel));
}


/* Interrupt Service Routines */
RAMFUNC void SysTick_Handler(void)
{
    millis++;
}

void HardFault_Handler(void)
{
#ifdef ENABLE_MTB
    // Turn off the micro trace buffer so we don't fill it up in the infinite
    // loop below.
    MTB->MASTER.reg = 0x00000000;
#endif

    uint8_t port = DEBUG0_LED_PIN.internal.port;
    uint32_t mask = (1 << DEBUG0_LED_PIN.internal.pin);

    PORT->Group[port].DIRSET.reg = mask;

    for (;;) {
        PORT->Group[port].OUTSET.reg = mask;
        uint64_t n = 1000000;
        while (n--) {
            asm volatile ("");
        }
        PORT->Group[port].OUTCLR.reg = mask;
        n = 10000000;
        while (n--) {
            asm volatile ("");
        }
    }
}
