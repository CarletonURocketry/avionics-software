/**
 * @file main.c
 * @desc Where it all begins
 * @author Samuel Dewan
 * @date 2018-12-17
 * Last Author:
 * Last Edited On:
 */

#include "global.h"

#include "dma.h"
#include "sercom-uart.h"
#include "sercom-spi.h"

//MARK: Constants

// MARK: Function prototypes
static void main_loop(void);

// MARK: Variable Definitions
volatile uint32_t millis;

static uint32_t lastLed_g;


// Stores 2 ^ TRACE_BUFFER_MAGNITUDE_PACKETS packets.
// 4 -> 16 packets
#define TRACE_BUFFER_MAGNITUDE_PACKETS 4
// Size in uint32_t. Two per packet.
#define TRACE_BUFFER_SIZE (1 << (TRACE_BUFFER_MAGNITUDE_PACKETS + 1))
// Size in bytes. 8 bytes per packet.
#define TRACE_BUFFER_SIZE_BYTES (TRACE_BUFFER_SIZE << 3)
__attribute__((__aligned__(TRACE_BUFFER_SIZE_BYTES))) uint32_t mtb[TRACE_BUFFER_SIZE];


// MARK: Function Definitions
static void init_main_clock(void)
{
    // Set 1 Flash Wait State for 48MHz, cf tables 20.9 and 35.27 in SAMD21 Datasheet
    NVMCTRL->CTRLB.bit.RWS = NVMCTRL_CTRLB_RWS_HALF_Val;
    // Turn on the digital interface clock
    PM->APBAMASK.reg |= PM_APBAMASK_GCLK;
    
    // Enable XOSC32K clock (External on-board 32.768Hz oscillator)
    SYSCTRL->XOSC32K.reg = SYSCTRL_XOSC32K_STARTUP( 0x6u ) | // cf table 15.10 of product datasheet in chapter 15.8.6
            SYSCTRL_XOSC32K_XTALEN | SYSCTRL_XOSC32K_EN32K;
    SYSCTRL->XOSC32K.bit.ENABLE = 1; // separate call, as described in chapter 15.6.3
    while (!(SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_XOSC32KRDY)); // Wait for oscillator stabilization
    
    
    // Software reset the module to ensure it is re-initialized correctly
    GCLK->CTRL.reg = GCLK_CTRL_SWRST;
    // Due to synchronization, there is a delay from writing CTRL.SWRST until the reset is complete.
    // CTRL.SWRST and STATUS.SYNCBUSY will both be cleared when the reset is complete, as described in chapter 13.8.1
    while ((GCLK->CTRL.reg & GCLK_CTRL_SWRST) && (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)); // Wait for reset to complete
    
    // Put XOSC32K as source of Generic Clock Generator 1
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(1u) ; // Generic Clock Generator 1
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY); // Wait for synchronization
    
    
    // Write Generic Clock Generator 1 configuration
    GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(1u) | // Generic Clock Generator 1
                        GCLK_GENCTRL_SRC_XOSC32K | // Selected source is External 32KHz Oscillator
    //                      GCLK_GENCTRL_OE | // Output clock to a pin for tests
                        GCLK_GENCTRL_GENEN ;
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY); // Wait for synchronization
    
    // Set Generic Clock Generator 1 as source for Generic Clock Multiplexer 0 (DFLL48M reference)
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(0u) | // Generic Clock Multiplexer 0
            GCLK_CLKCTRL_GEN_GCLK1 | // Generic Clock Generator 1 is source
            GCLK_CLKCTRL_CLKEN;
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY); // Wait for synchronization
    
    // Enable DFLL48M clock
    // DFLL Configuration in Closed Loop mode, cf product datasheet chapter 15.6.7.1 - Closed-Loop Operation
    
    // Remove the OnDemand mode, Bug http://avr32.icgroup.norway.atmel.com/bugzilla/show_bug.cgi?id=9905
    SYSCTRL->DFLLCTRL.bit.ONDEMAND = 0;
    while (!(SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY));  // Wait for synchronization
    
    SYSCTRL->DFLLMUL.reg = SYSCTRL_DFLLMUL_CSTEP( 31 ) | // Coarse step is 31, half of the max value
    SYSCTRL_DFLLMUL_FSTEP( 511 ) | // Fine step is 511, half of the max value
    SYSCTRL_DFLLMUL_MUL((48000000/32768)) ; // External 32KHz is the reference
    while (!(SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY));  // Wait for synchronization
    
    // Write full configuration to DFLL control register
    SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_MODE | // Enable the closed loop mode
            SYSCTRL_DFLLCTRL_WAITLOCK |
            SYSCTRL_DFLLCTRL_QLDIS; // Disable Quick lock
    while (!(SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY));  // Wait for synchronization
    
    
    // Enable the DFLL
    SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_ENABLE;
    // Wait for locks flags
    while ((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKC) == 0 || (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKF) == 0);
    while ((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0); // Wait for synchronization
    
    
    // Switch Generic Clock Generator 0 to DFLL48M. CPU will run at 48MHz.
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(0u); // Generic Clock Generator 0
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY); // Wait for synchronization
    
    // Write Generic Clock Generator 0 configuration
    GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(0u) | // Generic Clock Generator 0
    GCLK_GENCTRL_SRC_DFLL48M | // Selected source is DFLL 48MHz
    //                      GCLK_GENCTRL_OE | // Output clock to a pin for tests
            GCLK_GENCTRL_IDC | // Set 50/50 duty cycle
            GCLK_GENCTRL_GENEN ;
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY); // Wait for synchronization
    
    
    // Modify PRESCaler value of OSC8M to have 8MHz
    SYSCTRL->OSC8M.bit.PRESC = SYSCTRL_OSC8M_PRESC_1_Val;
    SYSCTRL->OSC8M.bit.ONDEMAND = 0;
    
    // Put OSC8M as source for Generic Clock Generator 3
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(3u); // Generic Clock Generator 3
    
    // Write Generic Clock Generator 3 configuration
    GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(3u) | // Generic Clock Generator 3
            GCLK_GENCTRL_SRC_OSC8M | // Selected source is RC OSC 8MHz (already enabled at reset)
    //                      GCLK_GENCTRL_OE | // Output clock to a pin for tests
            GCLK_GENCTRL_GENEN ;
    
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY); // Wait for synchronization
}

static struct sercom_uart_desc_t console_g;
static struct sercom_spi_desc_t spi_g;

int main(void)
{
    init_main_clock();
    SysTick_Config(48000); // Enable SysTick for an interupt every millisecond
    
    // Load ADC factory calibration values
    // ADC Bias Calibration
    uint32_t bias = (*((uint32_t *) ADC_FUSES_BIASCAL_ADDR) & ADC_FUSES_BIASCAL_Msk) >> ADC_FUSES_BIASCAL_Pos;
    // ADC Linearity bits 4:0
    uint32_t linearity = (*((uint32_t *) ADC_FUSES_LINEARITY_0_ADDR) & ADC_FUSES_LINEARITY_0_Msk) >> ADC_FUSES_LINEARITY_0_Pos;
    // ADC Linearity bits 7:5
    linearity |= ((*((uint32_t *) ADC_FUSES_LINEARITY_1_ADDR) & ADC_FUSES_LINEARITY_1_Msk) >> ADC_FUSES_LINEARITY_1_Pos) << 5;
    ADC->CALIB.reg = ADC_CALIB_BIAS_CAL(bias) | ADC_CALIB_LINEARITY_CAL(linearity);
    
    // Disable automatic NVM write operations
    NVMCTRL->CTRLB.bit.MANW = 1;
    
    // Enable Micro Trace Buffer (MTB) as described here:
    //      https://github.com/adafruit/gdb-micro-trace-buffer
    MTB->POSITION.reg = ((uint32_t) (mtb - REG_MTB_BASE)) & 0xFFFFFFF8;
    MTB->FLOW.reg = (((uint32_t) mtb - REG_MTB_BASE) + TRACE_BUFFER_SIZE_BYTES) & 0xFFFFFFF8;
    MTB->MASTER.reg = 0x80000000 + (TRACE_BUFFER_MAGNITUDE_PACKETS - 1);
    
    
    
    // Setup PORT pins
//    PORT->Group[0].PINCFG[15].bit.PULLEN = 0b1;
//    PORT->Group[0].PINCFG[15].bit.INEN = 0b1;
//    PORT->Group[0].OUTSET.reg = PORT_PA15;
    
    PORT->Group[DEBUG_LED_GROUP_NUM].DIRSET.reg = DEBUG_LED_MASK;
    //PORT->Group[DEBUG_LED_GROUP_NUM].PINCFG[30].bit.DRVSTR = 0b1;
    /* PORT->Group[DEBUG_LED_GROUP_NUM].PINCFG[15].bit.DRVSTR = 0b1; */

    // Init DMA
    init_dmac();
    
    // USART Test
    // Set USART pins to the proper pinmux
    PORT->Group[0].PMUX[11].bit.PMUXE = 0x2;
    PORT->Group[0].PINCFG[22].bit.PMUXEN = 0b1;
    PORT->Group[0].PMUX[11].bit.PMUXO = 0x2;
    PORT->Group[0].PINCFG[23].bit.PMUXEN = 0b1;

    init_sercom_uart(&console_g, SERCOM3, 115200UL, F_CPU,
                        GCLK_CLKCTRL_GEN_GCLK0, 0, 1);
    sercom_uart_put_string(&console_g, "\x1B[2J\x1B[HHello Console!\n");
    sercom_uart_put_string_blocking(&console_g, "!@#$%^&*()_+-=~`[]\{}|;':\",./"
        "<>?\tqwertyuiopasdfghjklzxcvbnm\tQWERTYUIOPASDFGHJKLZXCVBNM 1234567890"
        "\n");

    // SPI Test
    PORT->Group[1].PMUX[6].bit.PMUXE = 0x2;
    PORT->Group[1].PINCFG[12].bit.PMUXEN = 0b1;
    PORT->Group[1].PMUX[6].bit.PMUXO = 0x2;
    PORT->Group[1].PINCFG[13].bit.PMUXEN = 0b1;
    PORT->Group[1].PMUX[7].bit.PMUXE = 0x2;
    PORT->Group[1].PINCFG[14].bit.PMUXEN = 0b1;
    
    // Setup IO Expander CS pin
    PORT->Group[0].DIRSET.reg = PORT_PA28;
    PORT->Group[0].OUTSET.reg = PORT_PA28;

    init_sercom_spi(&spi_g, SERCOM4, F_CPU, GCLK_CLKCTRL_GEN_GCLK0, -1, -1);

    uint8_t message_io_dir[] ={0b01000000, 0x00, 0x0};
    uint8_t message_port[] ={0b01000000, 0x12, 0x1};
    uint8_t id;

    sercom_spi_start(&spi_g, &id, 1000000UL, 0, PORT_PA28, message_io_dir, 3, NULL, 0);
    while (!sercom_spi_transaction_done(&spi_g, id));
    sercom_spi_start(&spi_g, &id, 1000000UL, 0, PORT_PA28, message_port, 3, NULL, 0);

    // Main Loop
    for (;;) {
        main_loop();
    }
    
	return 0; // never reached
}

static uint32_t period = 1000;

static void main_loop ()
{
    if (PORT->Group[0].IN.reg & PORT_PA15) {
        period = 1000;
    } else {
        //period = 100;
    }

    if ((millis - lastLed_g) >= period) {
        lastLed_g = millis;
        PORT->Group[DEBUG_LED_GROUP_NUM].OUTTGL.reg = DEBUG_LED_MASK;
    }
}




/* Interupt Service Routines */
void SysTick_Handler(void)
{
    millis++;
}

void HardFault_Handler(void)
{
    // Turn off the micro trace buffer so we don't fill it up in the infinite
    // loop below.
    MTB->MASTER.reg = 0x00000000;
    
    for (;;) {
        PORT->Group[DEBUG_LED_GROUP_NUM].OUTSET.reg = DEBUG_LED_MASK;
        uint64_t n = 1000000;
        while (n--) {
            asm volatile ("");
        }
        PORT->Group[DEBUG_LED_GROUP_NUM].OUTCLR.reg = DEBUG_LED_MASK;
        n = 10000000;
        while (n--) {
            asm volatile ("");
        }
    }
}
