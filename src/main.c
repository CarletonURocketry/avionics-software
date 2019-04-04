/**
 * @file main.c
 * @desc Where it all begins
 * @author Samuel Dewan
 * @date 2018-12-17
 * Last Author:
 * Last Edited On:
 */

#include "global.h"
#include "config.h"

#include "gpio.h"

#include "dma.h"
#include "sercom-uart.h"
#include "sercom-spi.h"
#include "sercom-i2c.h"

#ifdef ID_USB
#include "usb/usb.h"
#else
#undef ENABLE_USB
#endif

#include "console.h"
#include "cli.h"
#include "debug-commands.h"

//MARK: Constants

// MARK: Function prototypes
static void main_loop(void);

// MARK: Variable Definitions
volatile uint32_t millis;
volatile uint8_t inhibit_sleep_g;

static uint32_t lastLed_g;
static uint8_t stat_transaction_id;

// MARK: Hardware Resources from Config File
#ifdef SPI_SERCOM_INST
struct sercom_spi_desc_t spi_g;
#endif
#ifdef I2C_SERCOM_INST
struct sercom_i2c_desc_t i2c_g;
#endif

#ifdef UART0_SERCOM_INST
struct sercom_uart_desc_t uart0_g;
#endif
#ifdef UART1_SERCOM_INST
struct sercom_uart_desc_t uart1_g;
#endif
#ifdef UART2_SERCOM_INST
struct sercom_uart_desc_t uart2_g;
#endif
#ifdef UART3_SERCOM_INST
struct sercom_uart_desc_t uart3_g;
#endif

#ifdef ENABLE_IO_EXPANDER
struct mcp23s17_desc_t io_expander_g;
#endif

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


struct console_desc_t console_g;
struct cli_desc_t cli_g;


static inline void init_io (void)
{
    // Debug LED
    PORT->Group[DEBUG_LED_GROUP_NUM].DIRSET.reg = DEBUG_LED_MASK;
    //PORT->Group[DEBUG_LED_GROUP_NUM].PINCFG[30].bit.DRVSTR = 0b1;
    PORT->Group[DEBUG_LED_GROUP_NUM].PINCFG[15].bit.DRVSTR = 0b1;
    
    // SPI
    PORT->Group[1].PMUX[6].bit.PMUXE = 0x2;     // MOSI (Pad 0)
    PORT->Group[1].PINCFG[12].bit.PMUXEN = 0b1;
    PORT->Group[1].PMUX[6].bit.PMUXO = 0x2;     // SCK (Pad 1)
    PORT->Group[1].PINCFG[13].bit.PMUXEN = 0b1;
    PORT->Group[1].PMUX[7].bit.PMUXE = 0x2;     // MISO (Pad 2)
    PORT->Group[1].PINCFG[14].bit.PMUXEN = 0b1;
    
    // I2C
    PORT->Group[1].PMUX[8].bit.PMUXE = 0x2;     // SDA (Pad 0)
    PORT->Group[1].PINCFG[16].bit.PMUXEN = 0b1;
    PORT->Group[1].PMUX[8].bit.PMUXO = 0x2;     // SCL (Pad 1)
    PORT->Group[1].PINCFG[17].bit.PMUXEN = 0b1;
    
    // UART 0
    
    // UART 1
    
    // UART 2
    
    // UART 3
    PORT->Group[0].PMUX[11].bit.PMUXE = 0x2;
    PORT->Group[0].PINCFG[22].bit.PMUXEN = 0b1;
    PORT->Group[0].PMUX[11].bit.PMUXO = 0x2;
    PORT->Group[0].PINCFG[23].bit.PMUXEN = 0b1;
    
    // USB
    PORT->Group[0].PMUX[12].bit.PMUXE = 0x6;     // D-
    PORT->Group[0].PINCFG[24].bit.PMUXEN = 0b1;
    PORT->Group[0].PMUX[12].bit.PMUXO = 0x6;     // D+
    PORT->Group[0].PINCFG[25].bit.PMUXEN = 0b1;
    
    // IO Expander CS pin
    PORT->Group[0].DIRSET.reg = PORT_PA28;
    PORT->Group[0].OUTSET.reg = PORT_PA28;
}

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
    
    init_io();
    
    // Init DMA
#ifdef ENABLE_DMA
    init_dmac();
#endif

    // Init SPI
#ifndef SPI_RX_DMA_CHAN
#define SPI_RX_DMA_CHAN -1
#endif

#ifndef SPI_TX_DMA_CHAN
#define SPI_TX_DMA_CHAN -1
#endif
    
#ifdef SPI_SERCOM_INST
     init_sercom_spi(&spi_g, SPI_SERCOM_INST, F_CPU, GCLK_CLKCTRL_GEN_GCLK0,
                     SPI_TX_DMA_CHAN, SPI_RX_DMA_CHAN);
#endif
    
    // Init I2C
#ifndef I2C_DMA_CHAN
#define I2C_DMA_CHAN -1
#endif
    
#ifndef I2C_SPEED
#define I2C_SPEED I2C_MODE_STANDARD
#endif
    
#ifdef I2C_SERCOM_INST
    init_sercom_i2c(&i2c_g, I2C_SERCOM_INST, F_CPU, GCLK_CLKCTRL_GEN_GCLK0,
                    I2C_SPEED, I2C_DMA_CHAN);
#endif
    
    // Init UART 0
#ifndef UART0_DMA_CHAN
#define UART0_DMA_CHAN -1
#endif
    
#ifdef UART0_SERCOM_INST
    init_sercom_uart(&uart0_g, UART0_SERCOM_INST, UART0_BAUD, F_CPU,
                     GCLK_CLKCTRL_GEN_GCLK0, UART0_DMA_CHAN, UART0_ECHO);
#endif
    
    // Init UART 1
#ifndef UART1_DMA_CHAN
#define UART1_DMA_CHAN -1
#endif
    
#ifdef UART1_SERCOM_INST
    init_sercom_uart(&uart1_g, UART1_SERCOM_INST, UART1_BAUD, F_CPU,
                     GCLK_CLKCTRL_GEN_GCLK0, UART1_DMA_CHAN, UART1_ECHO);
#endif
    
    // Init UART 2
#ifndef UART2_DMA_CHAN
#define UART2_DMA_CHAN -1
#endif
    
#ifdef UART2_SERCOM_INST
    init_sercom_uart(&uart2_g, UART2_SERCOM_INST, UART2_BAUD, F_CPU,
                     GCLK_CLKCTRL_GEN_GCLK0, UART2_DMA_CHAN, UART2_ECHO);
#endif
    
    // Init UART 3
#ifndef UART3_DMA_CHAN
#define UART3_DMA_CHAN -1
#endif
    
#ifdef UART3_SERCOM_INST
    init_sercom_uart(&uart3_g, UART3_SERCOM_INST, UART3_BAUD, F_CPU,
                     GCLK_CLKCTRL_GEN_GCLK0, UART3_DMA_CHAN, UART3_ECHO);
#endif
    
    // Init USB
#ifdef ENABLE_USB
    usb_init();
    usb_set_speed(USB_SPEED_FULL);
    usb_attach();
#endif
    
    // Console
#ifdef ENABLE_CONSOLE
#ifdef CONSOLE_UART
    init_console(&console_g, &CONSOLE_UART, '\r');
#elif defined ENABLE_USB
    init_console(&console_g, NULL, '\r');
#endif
#endif
    
    // Debug CLI
#ifdef ENABLE_DEBUG_CLI
    init_cli(&cli_g, &console_g, "> ", debug_commands_funcs,
             debug_commands_num_funcs);
#endif
    
    // IO Expander
#ifdef ENABLE_IO_EXPANDER
    init_mcp23s17(&io_expander_g, 0, &spi_g, 0, PORT_PA28, 0);
#endif
    
    // GPIO
#ifdef ENABLE_IO_EXPANDER
    init_gpio(GCLK_CLKCTRL_GEN_GCLK0, &io_expander_g, PIN_PA27);
#else
    init_gpio(GCLK_CLKCTRL_GEN_GCLK0, NULL, 0);
#endif
    
    //gpio_set_pin_mode(DEBUG_LED_PIN, GPIO_PIN_OUTPUT_STRONG);
    
    
    // SPI Test
    uint8_t message_io_dir[] = {0b01000000, 0x00, 0x0};
    sercom_spi_start(&spi_g, &stat_transaction_id, 8000000UL, 0, PORT_PA28,
                     message_io_dir, 3, NULL, 0);
    while (!sercom_spi_transaction_done(&spi_g, stat_transaction_id));
    sercom_spi_clear_transaction(&spi_g, stat_transaction_id);
    
    
    // Main Loop
    for (;;) {
        main_loop();
        if (!inhibit_sleep_g) {
            __WFI();
        }
    }
    
	return 0; // never reached
}

#define STAT_PERIOD 1500

static void main_loop ()
{
    static uint32_t period = 1000;
    
    period = 1000;

    if ((millis - lastLed_g) >= period) {
        lastLed_g = millis;
        PORT->Group[DEBUG_LED_GROUP_NUM].OUTTGL.reg = DEBUG_LED_MASK;
    }
    
    static uint32_t last_stat;
    static uint8_t stat_buffer[] ={0b01000000, 0x12, 0b11000000};
    
    if (((millis - last_stat) >= STAT_PERIOD) &&
        sercom_spi_transaction_done(&spi_g, stat_transaction_id)) {
        last_stat = millis;
        sercom_spi_clear_transaction(&spi_g, stat_transaction_id);
        
        stat_buffer[2] ^= 0xE0;
        sercom_spi_start(&spi_g, &stat_transaction_id, 8000000UL, 0, PORT_PA28,
                         stat_buffer, 3, NULL, 0);
    }
        
#ifdef ENABLE_CONSOLE
    console_service(&console_g);
#endif
    
#ifdef I2C_SERCOM_INST
    //sercom_i2c_service(&i2c_g);
#endif
}




/* Interupt Service Routines */
RAMFUNC void SysTick_Handler(void)
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
