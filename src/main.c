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

#include "wdt.h"
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
static void test_ext_int(union gpio_pin_t pin, uint8_t value);

// MARK: Variable Definitions
volatile uint32_t millis;
volatile uint8_t inhibit_sleep_g;

static uint32_t lastLed_g;

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
static void init_clocks (void)
{
    /* Configure a single flash wait state, good for 2.7-3.3v operation at 48MHz */
    // See section 37.12 of datasheet (NVM Characteristics)
    NVMCTRL->CTRLB.bit.RWS = NVMCTRL_CTRLB_RWS_HALF_Val;
    
    // Ensure that interface clock for generic clock controler is enabled
    PM->APBAMASK.reg |= PM_APBAMASK_GCLK;
    
    /* Enable external 32.768 KHz oscillator */
    // 1000092μs (32768 OSCULP32K cycles) startupt time, enable crystal,
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
    
    /* Configure Generic Clock Multiplexer for DFLL48M to use Generic Clock Generator 1 */
    GCLK->CLKCTRL.reg = (GCLK_CLKCTRL_ID_DFLL48 | GCLK_CLKCTRL_GEN_GCLK1 |
                         GCLK_CLKCTRL_CLKEN);
    // Wait for synchronization
    while (GCLK->STATUS.bit.SYNCBUSY);
    
    /* Enable DFLL48M */
    // Disable On Demand mode before configuration
    //      see silicon erata section 1.2.1 - Write Access to DFLL Register
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
    PORT->Group[0].PMUX[2].bit.PMUXE = 0x3;
    PORT->Group[0].PINCFG[4].bit.PMUXEN = 0b1;
    PORT->Group[0].PMUX[2].bit.PMUXO = 0x3;
    PORT->Group[0].PINCFG[5].bit.PMUXEN = 0b1;
    
    // UART 1
    PORT->Group[0].PMUX[8].bit.PMUXE = 0x3;
    PORT->Group[0].PINCFG[16].bit.PMUXEN = 0b1;
    PORT->Group[0].PMUX[8].bit.PMUXO = 0x3;
    PORT->Group[0].PINCFG[17].bit.PMUXEN = 0b1;
    
    // UART 2
    PORT->Group[0].PMUX[6].bit.PMUXE = 0x3;
    PORT->Group[0].PINCFG[12].bit.PMUXEN = 0b1;
    PORT->Group[0].PMUX[6].bit.PMUXO = 0x3;
    PORT->Group[0].PINCFG[13].bit.PMUXEN = 0b1;
    
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
    init_clocks();
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
#ifdef SPI_SERCOM_INST
    
#ifndef SPI_RX_DMA_CHAN
#define SPI_RX_DMA_CHAN -1
#endif
    
#ifndef SPI_TX_DMA_CHAN
#define SPI_TX_DMA_CHAN -1
#endif
    init_sercom_spi(&spi_g, SPI_SERCOM_INST, F_CPU, GCLK_CLKCTRL_GEN_GCLK0,
                    SPI_TX_DMA_CHAN, SPI_RX_DMA_CHAN);
#endif
    
    // Init I2C
#ifdef I2C_SERCOM_INST
    
#ifndef I2C_DMA_CHAN
#define I2C_DMA_CHAN -1
#endif
    
#ifndef I2C_SPEED
#define I2C_SPEED I2C_MODE_STANDARD
#endif
    init_sercom_i2c(&i2c_g, I2C_SERCOM_INST, F_CPU, GCLK_CLKCTRL_GEN_GCLK0,
                    I2C_SPEED, I2C_DMA_CHAN);
#endif
    
    // Init UART 0
#ifdef UART0_SERCOM_INST
#ifndef UART0_DMA_CHAN
#define UART0_DMA_CHAN -1
#endif
    init_sercom_uart(&uart0_g, UART0_SERCOM_INST, UART0_BAUD, F_CPU,
                     GCLK_CLKCTRL_GEN_GCLK0, UART0_DMA_CHAN, UART0_ECHO);
#endif
    
    // Init UART 1
#ifdef UART1_SERCOM_INST
#ifndef UART1_DMA_CHAN
#define UART1_DMA_CHAN -1
#endif
    init_sercom_uart(&uart1_g, UART1_SERCOM_INST, UART1_BAUD, F_CPU,
                     GCLK_CLKCTRL_GEN_GCLK0, UART1_DMA_CHAN, UART1_ECHO);
#endif
    
    // Init UART 2
#ifdef UART2_SERCOM_INST
#ifndef UART2_DMA_CHAN
#define UART2_DMA_CHAN -1
#endif
    init_sercom_uart(&uart2_g, UART2_SERCOM_INST, UART2_BAUD, F_CPU,
                     GCLK_CLKCTRL_GEN_GCLK0, UART2_DMA_CHAN, UART2_ECHO);
#endif
    
    // Init UART 3
#ifdef UART3_SERCOM_INST
#ifndef UART3_DMA_CHAN
#define UART3_DMA_CHAN -1
#endif
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
    
    // GPIO
#ifdef ENABLE_IO_EXPANDER
    init_mcp23s17(&io_expander_g, 0, &spi_g, 100, IO_EXPANDER_CS_PIN_MASK,
                  IO_EXPANDER_CS_PIN_GROUP);
    init_gpio(GCLK_CLKCTRL_GEN_GCLK0, &io_expander_g, IO_EXPANDER_INT_PIN);
#else
    init_gpio(GCLK_CLKCTRL_GEN_GCLK0, NULL, 0);
#endif
    
    gpio_set_pin_mode(DEBUG0_LED_PIN, GPIO_PIN_OUTPUT_TOTEM);
    gpio_set_pin_mode(DEBUG1_LED_PIN, GPIO_PIN_OUTPUT_TOTEM);
    gpio_set_pin_mode(STAT_R_LED_PIN, GPIO_PIN_OUTPUT_TOTEM);
    gpio_set_pin_mode(STAT_G_LED_PIN, GPIO_PIN_OUTPUT_TOTEM);
    
    gpio_set_pin_mode(MCP23S17_PIN_FOR(MCP23S17_PORT_B, 7), GPIO_PIN_INPUT);
    gpio_set_pull(MCP23S17_PIN_FOR(MCP23S17_PORT_B, 7), GPIO_PULL_HIGH);

    gpio_enable_interupt(MCP23S17_PIN_FOR(MCP23S17_PORT_B, 7),
                         GPIO_INTERRUPT_FALLING_EDGE, 0, test_ext_int);
    
//    gpio_set_pin_mode(GPIO_PIN_FOR(PIN_PB30), GPIO_PIN_INPUT);
//    gpio_set_pull(GPIO_PIN_FOR(PIN_PB30), GPIO_PULL_HIGH);
//
//    gpio_enable_interupt(GPIO_PIN_FOR(PIN_PB30), GPIO_INTERRUPT_LOW, 1,
//                         test_ext_int);
    
    gpio_set_output(STAT_G_LED_PIN, 1);
    
    // Start Watchdog Timer
    init_wdt(GCLK_CLKCTRL_GEN_GCLK7, 14, 0);
    
    // Main Loop
    for (;;) {
        // Pat the watchdog
        wdt_pat();
        // Run the main loop
        main_loop();
        // Sleep if sleep is not inhibited
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
    
    if ((millis - lastLed_g) >= period) {
        lastLed_g = millis;
        gpio_toggle_output(DEBUG0_LED_PIN);
    }
    
    static uint32_t last_stat;
    if (((millis - last_stat) >= STAT_PERIOD)) {
        last_stat = millis;
        gpio_toggle_output(STAT_R_LED_PIN);
        gpio_toggle_output(STAT_G_LED_PIN);
    }
    gpio_set_output(DEBUG1_LED_PIN, gpio_get_input(MCP23S17_PIN_FOR(MCP23S17_PORT_B, 7)));
    
#ifdef ENABLE_CONSOLE
    console_service(&console_g);
#endif
    
#ifdef I2C_SERCOM_INST
    //sercom_i2c_service(&i2c_g);
#endif
    
#ifdef ENABLE_IO_EXPANDER
    mcp23s17_service(&io_expander_g);
#endif
}


static void test_ext_int(union gpio_pin_t pin, uint8_t value)
{
    
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
