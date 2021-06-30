/**
 * @file board.c
 * @desc Board specific functions and data for SAMD21 MCU board
 * @author Samuel Dewan
 * @date 2020-09-05
 * Last Author:
 * Last Edited On:
 */

#include "board.h"

#include "global.h"

#include "variant.h"
#include "target.h"

#include "adc.h"
#include "sercom-uart.h"
#include "sercom-spi.h"
#include "sercom-i2c.h"
#include "usb.h"
#include "usb-cdc.h"
#include "wdt.h"
#include "sdspi.h"

// MARK: Hardware Resources from Config File
#ifdef ENABLE_SPI0
struct sercom_spi_desc_t spi0_g;
#endif
#ifdef ENABLE_I2C0
struct sercom_i2c_desc_t i2c0_g;
#endif

#ifdef ENABLE_UART0
struct sercom_uart_desc_t uart0_g;
#endif
#ifdef ENABLE_UART1
struct sercom_uart_desc_t uart1_g;
#endif
#ifdef ENABLE_UART2
struct sercom_uart_desc_t uart2_g;
#endif
#ifdef ENABLE_UART3
struct sercom_uart_desc_t uart3_g;
#endif

#ifdef ENABLE_IO_EXPANDER
struct mcp23s17_desc_t io_expander_g;
#ifndef ENABLE_SPI0
#error IO expander is enabled, but SPI0 is not.
#endif
#endif

#ifdef ENABLE_SDSPI
struct sdspi_desc_t sdspi_g;
#endif


// MARK: Functions
static inline void init_io (void)
{
#ifdef ENABLE_SPI0
    // SPI
    PORT_IOBUS->Group[1].PMUX[6].bit.PMUXE = 0x2;     // MOSI (Pad 0)
    PORT_IOBUS->Group[1].PINCFG[12].bit.PMUXEN = 0b1;
    PORT_IOBUS->Group[1].PMUX[6].bit.PMUXO = 0x2;     // SCK (Pad 1)
    PORT_IOBUS->Group[1].PINCFG[13].bit.PMUXEN = 0b1;
    PORT_IOBUS->Group[1].PMUX[7].bit.PMUXE = 0x2;     // MISO (Pad 2)
    PORT_IOBUS->Group[1].PINCFG[14].bit.PMUXEN = 0b1;
#endif

#ifdef ENABLE_I2C0
    // I2C
    PORT_IOBUS->Group[1].PMUX[8].bit.PMUXE = 0x2;     // SDA (Pad 0)
    PORT_IOBUS->Group[1].PINCFG[16].bit.PMUXEN = 0b1;
    PORT_IOBUS->Group[1].PMUX[8].bit.PMUXO = 0x2;     // SCL (Pad 1)
    PORT_IOBUS->Group[1].PINCFG[17].bit.PMUXEN = 0b1;
#endif

#ifdef ENABLE_UART0
    // UART 0
    PORT_IOBUS->Group[0].PMUX[2].bit.PMUXE = 0x3;     // TX Sercom 0 Pad 0
    PORT_IOBUS->Group[0].PINCFG[4].bit.PMUXEN = 0b1;
    PORT_IOBUS->Group[0].PMUX[2].bit.PMUXO = 0x3;     // RX Sercom 0 Pad 1
    PORT_IOBUS->Group[0].PINCFG[5].bit.PMUXEN = 0b1;
#endif

#ifdef ENABLE_UART1
    // UART 1
    PORT_IOBUS->Group[0].PMUX[8].bit.PMUXE = 0x2;     // TX Sercom 1 Pad 0
    PORT_IOBUS->Group[0].PINCFG[16].bit.PMUXEN = 0b1;
    PORT_IOBUS->Group[0].PMUX[8].bit.PMUXO = 0x2;     // RX Sercom 1 Pad 1
    PORT_IOBUS->Group[0].PINCFG[17].bit.PMUXEN = 0b1;
#endif

#ifdef ENABLE_UART2
    // UART 2
    PORT_IOBUS->Group[0].PMUX[6].bit.PMUXE = 0x2;     // TX Sercom 2 Pad 0
    PORT_IOBUS->Group[0].PINCFG[12].bit.PMUXEN = 0b1;
    PORT_IOBUS->Group[0].PMUX[6].bit.PMUXO = 0x2;     // RX Sercom 2 Pad 1
    PORT_IOBUS->Group[0].PINCFG[13].bit.PMUXEN = 0b1;
#endif

#ifdef ENABLE_UART3
    // UART 3
    PORT_IOBUS->Group[0].PMUX[11].bit.PMUXE = 0x2;    // TX Sercom 3 Pad 0
    PORT_IOBUS->Group[0].PINCFG[22].bit.PMUXEN = 0b1;
    PORT_IOBUS->Group[0].PMUX[11].bit.PMUXO = 0x2;    // RX Sercom 3 Pad 1
    PORT_IOBUS->Group[0].PINCFG[23].bit.PMUXEN = 0b1;
#endif

#ifdef ENABLE_USB
    // USB
    PORT_IOBUS->Group[0].PMUX[12].bit.PMUXE = 0x6;     // D-
    PORT_IOBUS->Group[0].PINCFG[24].bit.PMUXEN = 0b1;
    PORT_IOBUS->Group[0].PMUX[12].bit.PMUXO = 0x6;     // D+
    PORT_IOBUS->Group[0].PINCFG[25].bit.PMUXEN = 0b1;
#endif

#ifdef ENABLE_IO_EXPANDER
    // IO Expander CS pin
    PORT_IOBUS->Group[IO_EXPANDER_CS_PIN_GROUP].DIRSET.reg = IO_EXPANDER_CS_PIN_MASK;
    PORT_IOBUS->Group[IO_EXPANDER_CS_PIN_GROUP].OUTSET.reg = IO_EXPANDER_CS_PIN_MASK;
#endif

#ifdef ENABLE_SDSPI
    // SD CS pin
    PORT_IOBUS->Group[SDSPI_CS_PIN_GROUP].DIRSET.reg = SDSPI_CS_PIN_MASK;
    PORT_IOBUS->Group[SDSPI_CS_PIN_GROUP].OUTSET.reg = SDSPI_CS_PIN_MASK;
#endif
}

void init_board(void)
{
    init_io();

    // Init SPI
#ifdef ENABLE_SPI0

#ifndef SPI0_RX_DMA_CHAN
#define SPI0_RX_DMA_CHAN -1
#endif

#ifndef SPI0_TX_DMA_CHAN
#define SPI0_TX_DMA_CHAN -1
#endif
    init_sercom_spi(&spi0_g, SPI0_SERCOM_INST, F_CPU, SAMD21_CLK_MSK_48MHZ,
                    SPI0_TX_DMA_CHAN, SPI0_RX_DMA_CHAN);
#endif

    // Init I2C
#ifdef ENABLE_I2C0

#ifndef I2C0_DMA_CHAN
#define I2C0_DMA_CHAN -1
#endif

#ifndef I2C0_SPEED
#define I2C0_SPEED I2C_MODE_STANDARD
#endif
    init_sercom_i2c(&i2c0_g, I2C0_SERCOM_INST, F_CPU, SAMD21_CLK_MSK_48MHZ,
                    I2C0_SPEED, I2C0_DMA_CHAN);
#endif

    // Init UART 0
#ifdef ENABLE_UART0
#ifndef UART0_DMA_CHAN
#define UART0_DMA_CHAN -1
#endif
    init_sercom_uart(&uart0_g, UART0_SERCOM_INST, UART0_BAUD, F_CPU,
                     SAMD21_CLK_MSK_48MHZ, UART0_DMA_CHAN, UART0_ECHO,
                     UART0_TX_PIN_GROUP, UART0_TX_PIN_NUM);
#endif

    // Init UART 1
#ifdef ENABLE_UART1
#ifndef UART1_DMA_CHAN
#define UART1_DMA_CHAN -1
#endif
    init_sercom_uart(&uart1_g, UART1_SERCOM_INST, UART1_BAUD, F_CPU,
                     SAMD21_CLK_MSK_48MHZ, UART1_DMA_CHAN, UART1_ECHO,
                     UART1_TX_PIN_GROUP, UART1_TX_PIN_NUM);
#endif

    // Init UART 2
#ifdef ENABLE_UART2
#ifndef UART2_DMA_CHAN
#define UART2_DMA_CHAN -1
#endif
    init_sercom_uart(&uart2_g, UART2_SERCOM_INST, UART2_BAUD, F_CPU,
                     SAMD21_CLK_MSK_48MHZ, UART2_DMA_CHAN, UART2_ECHO,
                     UART2_TX_PIN_GROUP, UART2_TX_PIN_NUM);
#endif

    // Init UART 3
#ifdef ENABLE_UART3
#ifndef UART3_DMA_CHAN
#define UART3_DMA_CHAN -1
#endif
    init_sercom_uart(&uart3_g, UART3_SERCOM_INST, UART3_BAUD, F_CPU,
                     SAMD21_CLK_MSK_48MHZ, UART3_DMA_CHAN, UART3_ECHO,
                     UART3_TX_PIN_GROUP, UART3_TX_PIN_NUM);
#endif

    // Init ADC
#ifdef ENABLE_ADC
#ifndef ADC_DMA_CHAN
#define ADC_DMA_CHAN -1
#endif
#ifndef ADC_TC
#define ADC_TC NULL
#endif
#ifndef ADC_EVENT_CHAN
#define ADC_EVENT_CHAN -1
#endif
    uint32_t chan_mask = (EXTERNAL_ANALOG_MASK |
                          (1 << ADC_INPUTCTRL_MUXPOS_TEMP_Val) |
                          (1 << ADC_INPUTCTRL_MUXPOS_SCALEDCOREVCC) |
                          (1 << ADC_INPUTCTRL_MUXPOS_SCALEDIOVCC));
    init_adc(SAMD21_CLK_MSK_8MHZ, 8000000UL, chan_mask, ADC_PERIOD,
             ADC_SOURCE_IMPEDANCE, ADC_DMA_CHAN);
#endif

    // Init USB
#ifdef ENABLE_USB
    init_usb(SAMD21_CLK_MSK_48MHZ, USB_SPEED_FULL,
             &usb_cdc_enable_config_callback, &usb_cdc_disable_config_callback,
             &usb_cdc_class_request_callback,
             ((const struct usb_configuration_descriptor*)
              &usb_cdc_config_descriptor));
    usb_attach();
#endif

    // GPIO
#ifdef ENABLE_IO_EXPANDER
    init_mcp23s17(&io_expander_g, 0, &spi0_g, 100, IO_EXPANDER_CS_PIN_MASK,
                  IO_EXPANDER_CS_PIN_GROUP);
#ifdef ENABLE_LORA
    init_gpio(SAMD21_CLK_MSK_48MHZ, &io_expander_g, IO_EXPANDER_INT_PIN,
              radios_g);
#else
    init_gpio(SAMD21_CLK_MSK_48MHZ, &io_expander_g, IO_EXPANDER_INT_PIN,
              NULL);
#endif
#else
#ifdef ENABLE_LORA
    init_gpio(SAMD21_CLK_MSK_48MHZ, NULL, 0, radios_g);
#else
    init_gpio(SAMD21_CLK_MSK_48MHZ, NULL, 0, NULL);
#endif
#endif

    gpio_set_pin_mode(DEBUG0_LED_PIN, GPIO_PIN_OUTPUT_STRONG);
#ifdef DEBUG1_LED_PIN
    gpio_set_pin_mode(DEBUG1_LED_PIN, GPIO_PIN_OUTPUT_STRONG);
#endif
#ifdef STAT_R_LED_PIN
    gpio_set_pin_mode(STAT_R_LED_PIN, GPIO_PIN_OUTPUT_STRONG);
#endif
#ifdef STAT_G_LED_PIN
    gpio_set_pin_mode(STAT_G_LED_PIN, GPIO_PIN_OUTPUT_STRONG);
    gpio_set_output(STAT_G_LED_PIN, 1);
#endif
#ifdef STAT_B_LED_PIN
    gpio_set_pin_mode(STAT_B_LED_PIN, GPIO_PIN_OUTPUT_STRONG);
#endif

#ifdef ENABLE_SDSPI
    init_sdpsi(&sdspi_g, &spi0_g, SDSPI_CS_PIN_MASK, SDSPI_CS_PIN_GROUP,
               SDSPI_DETECT_PIN);
#endif

#ifdef ENABLE_WATCHDOG
    init_wdt(SAMD21_CLK_MSK_8KHZ, 14, 0);
#endif
}



#define STAT_PERIOD MS_TO_MILLIS(1500)

void board_service(void)
{
#ifdef ENABLE_WATCHDOG
    // Pat the watchdog
    wdt_pat();
#endif

    static uint32_t last_debug_blink_time = 0;
    if ((millis - last_debug_blink_time) >= DEBUG_BLINK_PERIOD) {
        last_debug_blink_time = millis;
        gpio_toggle_output(DEBUG0_LED_PIN);
    }

#ifdef STAT_R_LED_PIN
#ifdef STAT_G_LED_PIN
    static uint32_t last_stat_blink_time = 0;
    if (((millis - last_stat_blink_time) >= STAT_PERIOD)) {
        last_stat_blink_time = millis;
        gpio_toggle_output(STAT_R_LED_PIN);
        gpio_toggle_output(STAT_G_LED_PIN);
    }
#endif
#endif

#ifdef ENABLE_I2C0
    sercom_i2c_service(&i2c0_g);
#endif

#ifdef ENABLE_UART0
    sercom_uart_service(&uart0_g);
#endif

#ifdef ENABLE_UART1
    sercom_uart_service(&uart1_g);
#endif

#ifdef ENABLE_UART2
    sercom_uart_service(&uart2_g);
#endif

#ifdef ENABLE_UART3
    sercom_uart_service(&uart3_g);
#endif

#ifdef ENABLE_IO_EXPANDER
    mcp23s17_service(&io_expander_g);
#endif

#ifdef ENABLE_ADC
    adc_service();
#endif

#ifdef ENABLE_SDSPI
    sdspi_service(&sdspi_g);
#endif
}
