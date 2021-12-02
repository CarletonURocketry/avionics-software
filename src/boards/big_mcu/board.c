/**
 * @file board.c
 * @desc Board specific functions and data for SAME54 MCU board
 * @author Samuel Dewan
 * @date 2021-06-05
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
#include "sdhc.h"

#include "kx134-1211.h"

// MARK: Hardware Resources from Config File
#ifdef ENABLE_SPI0
struct sercom_spi_desc_t spi0_g;
#endif
#ifdef ENABLE_SPI1
struct sercom_spi_desc_t spi1_g;
#endif
#ifdef ENABLE_I2C0
struct sercom_i2c_desc_t i2c0_g;
#endif
#ifdef ENABLE_I2C1
struct sercom_i2c_desc_t i2c1_g;
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

#ifdef ENABLE_SDHC0
struct sdhc_desc_t sdhc0_g;
#endif

#ifdef ENABLE_KX134_1211
struct kx134_1211_desc_t kx134_g;
#endif


// MARK: Functions
static inline void init_io (void)
{
#ifdef ENABLE_SPI0
    // SPI 0 (SERCOM6)
    PORT->Group[2].PMUX[2].bit.PMUXE = 0x2;     // PC04: MOSI (Pad 0)
    PORT->Group[2].PINCFG[4].bit.PMUXEN = 1;
    PORT->Group[2].PMUX[2].bit.PMUXO = 0x2;     // PC05: SCK (Pad 1)
    PORT->Group[2].PINCFG[5].bit.PMUXEN = 1;
    PORT->Group[2].PMUX[3].bit.PMUXE = 0x2;     // PC06: MISO (Pad 2)
    PORT->Group[2].PINCFG[6].bit.PMUXEN = 1;
#endif

#ifdef ENABLE_SPI1
    // SPI 1 (SERCOM5)
    PORT->Group[1].PMUX[8].bit.PMUXE = 0x2;     // PB16: MOSI (Pad 0)
    PORT->Group[1].PINCFG[16].bit.PMUXEN = 1;
    PORT->Group[1].PMUX[8].bit.PMUXO = 0x2;     // PB17: SCK (Pad 1)
    PORT->Group[1].PINCFG[17].bit.PMUXEN = 1;
    PORT->Group[1].PMUX[9].bit.PMUXE = 0x2;     // PB18: MISO (Pad 2)
    PORT->Group[1].PINCFG[18].bit.PMUXEN = 1;
#endif

#ifdef ENABLE_I2C0
    // I2C 0 (SERCOM7)
    PORT->Group[3].PMUX[4].bit.PMUXE = 0x2;     // PD08: SDA (Pad 0)
    PORT->Group[3].PINCFG[8].bit.PMUXEN = 0b1;
    PORT->Group[3].PMUX[4].bit.PMUXO = 0x2;     // PD09: SCL (Pad 1)
    PORT->Group[3].PINCFG[9].bit.PMUXEN = 0b1;
#endif

#ifdef ENABLE_I2C1
    // I2C 1 (SERCOM2)
    PORT->Group[0].PMUX[6].bit.PMUXE = 0x2;     // PA12: SDA (Pad 0)
    PORT->Group[0].PINCFG[12].bit.PMUXEN = 0b1;
    PORT->Group[0].PMUX[6].bit.PMUXO = 0x2;     // PA13: SCL (Pad 1)
    PORT->Group[0].PINCFG[13].bit.PMUXEN = 0b1;
#endif

#ifdef ENABLE_UART0
    // UART 0 (SERCOM1)
    PORT->Group[0].PMUX[8].bit.PMUXE = 0x2;     // PA16: TX (Pad 0)
    PORT->Group[0].PINCFG[16].bit.PMUXEN = 0b1;
    PORT->Group[0].PMUX[8].bit.PMUXO = 0x2;     // PA17: RX (Pad 1)
    PORT->Group[0].PINCFG[17].bit.PMUXEN = 0b1;
#endif

#ifdef ENABLE_UART1
    // UART 1 (SERCOM0)
    PORT->Group[2].PMUX[8].bit.PMUXO = 0x3;     // PC17: TX (Pad 0)
    PORT->Group[2].PINCFG[17].bit.PMUXEN = 0b1;
    PORT->Group[2].PMUX[8].bit.PMUXE = 0x3;     // PC16: RX (Pad 1)
    PORT->Group[2].PINCFG[16].bit.PMUXEN = 0b1;
#endif

#ifdef ENABLE_UART2
    // UART 2 (SERCOM3)
    PORT->Group[2].PMUX[11].bit.PMUXO = 0x3;    // PC23: TX (Pad 0)
    PORT->Group[2].PINCFG[23].bit.PMUXEN = 0b1;
    PORT->Group[2].PMUX[11].bit.PMUXE = 0x3;    // PC22: RX (Pad 1)
    PORT->Group[2].PINCFG[22].bit.PMUXEN = 0b1;
#endif

#ifdef ENABLE_UART3
    // UART 3 (SERCOM4)
    PORT->Group[1].PMUX[13].bit.PMUXO = 0x3;    // PB27: TX (Pad 0)
    PORT->Group[1].PINCFG[27].bit.PMUXEN = 0b1;
    PORT->Group[1].PMUX[13].bit.PMUXE = 0x3;    // PB26: RX (Pad 1)
    PORT->Group[1].PINCFG[26].bit.PMUXEN = 0b1;
#endif

#ifdef ENABLE_CAN0
    // CAN 0
    PORT->Group[0].PMUX[11].bit.PMUXE = 0x8;    // PA22: TX
    PORT->Group[0].PINCFG[22].bit.PMUXEN = 0b1;
    PORT->Group[0].PMUX[11].bit.PMUXO = 0x8;    // PA23: RX
    PORT->Group[0].PINCFG[23].bit.PMUXEN = 0b1;
#endif

#ifdef ENABLE_CAN1
    // CAN 1
    PORT->Group[1].PMUX[7].bit.PMUXE = 0x7;     // PB14: TX
    PORT->Group[1].PINCFG[14].bit.PMUXEN = 0b1;
    PORT->Group[1].PMUX[7].bit.PMUXO = 0x7;     // PB15: RX
    PORT->Group[1].PINCFG[15].bit.PMUXEN = 0b1;
#endif

#ifdef ENABLE_USB
    // USB
    PORT->Group[0].PMUX[12].bit.PMUXE = 0x7;     // PA24: D-
    PORT->Group[0].PINCFG[24].bit.PMUXEN = 0b1;
    PORT->Group[0].PMUX[12].bit.PMUXO = 0x7;     // PA25: D+
    PORT->Group[0].PINCFG[25].bit.PMUXEN = 0b1;
#endif

#ifdef ENABLE_SDHC0
    // SDHC 0
    PORT->Group[0].PMUX[4].bit.PMUXE = 0x8;      // PA08: SDCMD
    PORT->Group[0].PINCFG[8].bit.PMUXEN = 0b1;
    PORT->Group[0].PMUX[4].bit.PMUXO = 0x8;      // PA09: SDDAT0
    PORT->Group[0].PINCFG[9].bit.PMUXEN = 0b1;
    PORT->Group[0].PMUX[5].bit.PMUXO = 0x8;      // PA10: SDDAT1
    PORT->Group[0].PINCFG[10].bit.PMUXEN = 0b1;
    PORT->Group[0].PMUX[5].bit.PMUXO = 0x8;      // PA11: SDDAT2
    PORT->Group[0].PINCFG[11].bit.PMUXEN = 0b1;
    PORT->Group[1].PMUX[5].bit.PMUXO = 0x8;      // PB10: SDDAT3
    PORT->Group[1].PINCFG[10].bit.PMUXEN = 0b1;
    PORT->Group[1].PMUX[5].bit.PMUXO = 0x8;      // PB11: SDCK
    PORT->Group[1].PINCFG[11].bit.PMUXEN = 0b1;
    PORT->Group[1].PMUX[6].bit.PMUXE = 0x8;      // PB12: SDCD
    PORT->Group[1].PINCFG[12].bit.PMUXEN = 0b1;
#endif

#ifdef KX134_1211_CS_PIN_MASK
    PORT->Group[KX134_1211_CS_PIN_GROUP].DIRSET.reg = KX134_1211_CS_PIN_MASK;
    PORT->Group[KX134_1211_CS_PIN_GROUP].OUTSET.reg = KX134_1211_CS_PIN_MASK;
#endif
}

void init_board(void)
{
    init_io();

    // Init SPI0
#ifdef ENABLE_SPI0

#ifndef SPI0_RX_DMA_CHAN
#define SPI0_RX_DMA_CHAN -1
#endif

#ifndef SPI0_TX_DMA_CHAN
#define SPI0_TX_DMA_CHAN -1
#endif
    init_sercom_spi(&spi0_g, SPI0_SERCOM_INST, 48000000UL, SAME54_CLK_MSK_48MHZ,
                    SPI0_TX_DMA_CHAN, SPI0_RX_DMA_CHAN);
#endif

    // Init SPI1
#ifdef ENABLE_SPI1

#ifndef SPI1_RX_DMA_CHAN
#define SPI1_RX_DMA_CHAN -1
#endif

#ifndef SPI1_TX_DMA_CHAN
#define SPI1_TX_DMA_CHAN -1
#endif
    init_sercom_spi(&spi1_g, SPI1_SERCOM_INST, 100000000UL,
                    SAME54_CLK_MSK_100MHZ, SPI1_TX_DMA_CHAN, SPI1_RX_DMA_CHAN);
#endif

    // Init I2C0
#ifdef ENABLE_I2C0

#ifndef I2C0_DMA_CHAN
#define I2C0_DMA_CHAN -1
#endif

#ifndef I2C0_SPEED
#define I2C0_SPEED I2C_MODE_STANDARD
#endif
    init_sercom_i2c(&i2c0_g, I2C0_SERCOM_INST, 48000000UL, SAME54_CLK_MSK_48MHZ,
                    I2C0_SPEED, I2C0_DMA_CHAN);
#endif

    // Init I2C1
#ifdef ENABLE_I2C1

#ifndef I2C1_DMA_CHAN
#define I2C1_DMA_CHAN -1
#endif

#ifndef I2C1_SPEED
#define I2C1_SPEED I2C_MODE_STANDARD
#endif
    init_sercom_i2c(&i2c1_g, I2C1_SERCOM_INST, 48000000UL, SAME54_CLK_MSK_48MHZ,
                    I2C1_SPEED, I2C1_DMA_CHAN);
#endif

    // Init UART 0
#ifdef ENABLE_UART0
#ifndef UART0_DMA_CHAN
#define UART0_DMA_CHAN -1
#endif
    init_sercom_uart(&uart0_g, UART0_SERCOM_INST, UART0_BAUD, 48000000UL,
                     SAME54_CLK_MSK_48MHZ, UART0_DMA_CHAN, UART0_ECHO,
                     UART0_TX_PIN_GROUP, UART0_TX_PIN_NUM);
#endif

    // Init UART 1
#ifdef ENABLE_UART1
#ifndef UART1_DMA_CHAN
#define UART1_DMA_CHAN -1
#endif
    init_sercom_uart(&uart1_g, UART1_SERCOM_INST, UART1_BAUD, 48000000UL,
                     SAME54_CLK_MSK_48MHZ, UART1_DMA_CHAN, UART1_ECHO,
                     UART1_TX_PIN_GROUP, UART1_TX_PIN_NUM);
#endif

    // Init UART 2
#ifdef ENABLE_UART2
#ifndef UART2_DMA_CHAN
#define UART2_DMA_CHAN -1
#endif
    init_sercom_uart(&uart2_g, UART2_SERCOM_INST, UART2_BAUD, 48000000UL,
                     SAME54_CLK_MSK_48MHZ, UART2_DMA_CHAN, UART2_ECHO,
                     UART2_TX_PIN_GROUP, UART2_TX_PIN_NUM);
#endif

    // Init UART 3
#ifdef ENABLE_UART3
#ifndef UART3_DMA_CHAN
#define UART3_DMA_CHAN -1
#endif
    init_sercom_uart(&uart3_g, UART3_SERCOM_INST, UART3_BAUD, 48000000UL,
                     SAME54_CLK_MSK_48MHZ, UART3_DMA_CHAN, UART3_ECHO,
                     UART3_TX_PIN_GROUP, UART3_TX_PIN_NUM);
#endif

    // Init ADC
#ifdef ENABLE_ADC
#ifndef ADC_DMA_RESULT_CHAN
#define ADC_DMA_RESULT_CHAN -1
#endif
#ifndef ADC_DMA_SEQUENCE_CHAN
#define ADC_DMA_SEQUENCE_CHAN -1
#endif
    const int8_t adc_dma_result_chans[2] = { ADC0_DMA_RESULT_CHAN,
                                             ADC1_DMA_RESULT_CHAN };
    const int8_t adc_dma_sequence_chans[2] = { ADC0_DMA_SEQUENCE_CHAN,
                                               ADC1_DMA_SEQUENCE_CHAN };

    init_adc(SAME54_CLK_MSK_12MHZ, 12000000UL,
             EXTERNAL_ANALOG_MASK | INTERNAL_ANALOG_MASK, ADC_PERIOD,
             ADC_SOURCE_IMPEDANCE, adc_dma_result_chans,
             adc_dma_sequence_chans);
#endif

    // Init USB
#ifdef ENABLE_USB
    init_usb(SAME54_CLK_MSK_48MHZ, USB_SPEED_FULL,
             &usb_cdc_enable_config_callback, &usb_cdc_disable_config_callback,
             &usb_cdc_class_request_callback,
             ((const struct usb_configuration_descriptor*)
              &usb_cdc_config_descriptor));
    usb_attach();
#endif

    // GPIO
#ifdef ENABLE_LORA
    init_gpio(SAME54_CLK_MSK_48MHZ, NULL, 0, radios_g);
#else
    init_gpio(SAME54_CLK_MSK_48MHZ, NULL, 0, NULL);
#endif

    gpio_set_pin_mode(DEBUG0_LED_PIN, GPIO_PIN_OUTPUT_STRONG);
    gpio_set_pin_mode(DEBUG1_LED_PIN, GPIO_PIN_OUTPUT_STRONG);

    gpio_set_pin_mode(STAT_R_LED_PIN, GPIO_PIN_OUTPUT_STRONG);
    gpio_set_pin_mode(STAT_G_LED_PIN, GPIO_PIN_OUTPUT_STRONG);
    gpio_set_output(STAT_G_LED_PIN, 1);
    gpio_set_pin_mode(STAT_B_LED_PIN, GPIO_PIN_OUTPUT_STRONG);

    gpio_set_pin_mode(SD_ACTIVE_LED_PIN, GPIO_PIN_OUTPUT_STRONG);

    // SDHC
#ifdef ENABLE_SDHC0
    init_sdhc(&sdhc0_g, SDHC0, 100000000UL, SAME54_CLK_MSK_100MHZ);
#endif

    // KX134-1211 Accelerometer
#ifdef ENABLE_KX134_1211
    init_kx134_1211(&kx134_g, &spi1_g, KX134_1211_CS_PIN_GROUP,
                    KX134_1211_CS_PIN_MASK, KX134_1211_INT1_PIN,
                    KX134_1211_INT2_PIN, KX134_1211_RANGE,
                    KX134_1211_LOW_PASS_ROLLOFF, KX134_1211_ODR,
                    KX134_1211_RES);
#endif


    // WDT
#ifdef ENABLE_WATCHDOG
    // 2 seconds, no early warning interrupt
    init_wdt(0, 11, 0);
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

    static uint32_t last_stat_blink_time = 0;
    if (((millis - last_stat_blink_time) >= STAT_PERIOD)) {
        last_stat_blink_time = millis;
        gpio_toggle_output(STAT_R_LED_PIN);
        gpio_toggle_output(STAT_G_LED_PIN);
    }

#ifdef ENABLE_I2C0
    sercom_i2c_service(&i2c0_g);
#endif

#ifdef ENABLE_I2C1
    sercom_i2c_service(&i2c1_g);
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

#ifdef ENABLE_ADC
    //adc_service();
#endif

#ifdef ENABLE_SDHC0
    sdhc_service(&sdhc0_g);
#endif

#ifdef ENABLE_KX134_1211
    kx134_1211_service(&kx134_g);
#endif
}
