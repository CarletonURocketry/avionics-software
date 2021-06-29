/**
 * @file board.h
 * @desc Board configuration for revision B MCU board
 * @author Samuel Dewan
 * @date 2020-09-05
 * Last Author:
 * Last Edited On:
 */

#ifndef board_h
#define board_h

#include "gpio.h"

#define BOARD_STRING "MCU Rev. B"

//
//
//  General Flags
//
//

#define ENABLE_WATCHDOG
#define DEBUG_BLINK_PERIOD 1000

//
//
//  Pin Definitions
//
//

/* Debug LEDs */
#define DEBUG0_LED_PIN GPIO_PIN_FOR(PIN_PB15)
#define DEBUG1_LED_PIN MCP23S17_PIN_FOR(MCP23S17_PORT_A, 7)

/* Stat LEDs */
#define STAT_R_LED_PIN MCP23S17_PIN_FOR(MCP23S17_PORT_A, 6)
#define STAT_G_LED_PIN MCP23S17_PIN_FOR(MCP23S17_PORT_A, 5)
#define STAT_B_LED_PIN MCP23S17_PIN_FOR(MCP23S17_PORT_A, 3)

/* IO Expander */
// MCP23S17 CS pin mask
#define IO_EXPANDER_CS_PIN_MASK PORT_PA27
// MCP23S17 CS pin group
#define IO_EXPANDER_CS_PIN_GROUP 0
// MCP23S17 Interrupt pin
#define IO_EXPANDER_INT_PIN ((uint16_t)PIN_PA28)

/* SD Card via SPI */
#define SDSPI_CS_PIN_MASK   PORT_PA11
#define SDSPI_CS_PIN_GROUP 0
#define SDSPI_DETECT_PIN MCP23S17_PIN_FOR(MCP23S17_PORT_A, 4)

/* GPIO */
#define GPIO_0  GPIO_PIN_FOR(PIN_PB10)
#define GPIO_1  GPIO_PIN_FOR(PIN_PB11)
#define GPIO_2  GPIO_PIN_FOR(PIN_PA15)
#define GPIO_3  GPIO_PIN_FOR(PIN_PA14)
#define GPIO_4  GPIO_PIN_FOR(PIN_PA18)
#define GPIO_5  GPIO_PIN_FOR(PIN_PA19)
#define GPIO_6  GPIO_PIN_FOR(PIN_PA20)
#define GPIO_7  GPIO_PIN_FOR(PIN_PA21)
#define GPIO_8  GPIO_PIN_FOR(PIN_PB30)
#define GPIO_9  GPIO_PIN_FOR(PIN_PB23)
#define GPIO_10  GPIO_PIN_FOR(PIN_PB22)
#define GPIO_11  GPIO_PIN_FOR(PIN_PB31)
#define GPIO_12  GPIO_PIN_FOR(PIN_PB03)
#define GPIO_13  MCP23S17_PIN_FOR(MCP23S17_PORT_A, 2)
#define GPIO_14  MCP23S17_PIN_FOR(MCP23S17_PORT_A, 1)
#define GPIO_15  MCP23S17_PIN_FOR(MCP23S17_PORT_A, 0)
#define GPIO_16  MCP23S17_PIN_FOR(MCP23S17_PORT_B, 7)
#define GPIO_17  MCP23S17_PIN_FOR(MCP23S17_PORT_B, 6)
#define GPIO_18  MCP23S17_PIN_FOR(MCP23S17_PORT_B, 5)
#define GPIO_19  MCP23S17_PIN_FOR(MCP23S17_PORT_B, 4)
#define GPIO_20  MCP23S17_PIN_FOR(MCP23S17_PORT_B, 3)
#define GPIO_21  MCP23S17_PIN_FOR(MCP23S17_PORT_B, 2)
#define GPIO_22  MCP23S17_PIN_FOR(MCP23S17_PORT_B, 1)
#define GPIO_23  MCP23S17_PIN_FOR(MCP23S17_PORT_B, 0)

/* Analog */
#define DAC_OUT GPIO_PIN_FOR(PIN_PA02)

// ADC header pin channels
#define ANALOG_A0   18
#define ANALOG_A1   17
#define ANALOG_A2   7
#define ANALOG_A3   3
#define ANALOG_A4   15
#define ANALOG_A5   13
#define ANALOG_A6   1
#define ANALOG_A7   9
#define ANALOG_A8   8
#define ANALOG_A9   10
#define ANALOG_A10  12
#define ANALOG_A11  14
#define ANALOG_A12  2
#define ANALOG_A13  6
#define ANALOG_A14  16

#define NUM_ANALOG_PINS 15

#define HEADER_ANALOG_PINS {ANALOG_A0, ANALOG_A1, ANALOG_A2, ANALOG_A3, \
                            ANALOG_A4, ANALOG_A5, ANALOG_A6, ANALOG_A7, \
                            ANALOG_A8, ANALOG_A9, ANALOG_A10, ANALOG_A11, \
                            ANALOG_A12, ANALOG_A13, ANALOG_A14}

#define EXTERNAL_ANALOG_MASK ((1 << ANALOG_A0) | (1 << ANALOG_A1) | \
                              (1 << ANALOG_A2) | (1 << ANALOG_A3) | \
                              (1 << ANALOG_A4) | (1 << ANALOG_A5) | \
                              (1 << ANALOG_A6) | (1 << ANALOG_A7) | \
                              (1 << ANALOG_A8) | (1 << ANALOG_A9) | \
                              (1 << ANALOG_A10) | (1 << ANALOG_A11) | \
                              (1 << ANALOG_A12) | (1 << ANALOG_A13) | \
                              (1 << ANALOG_A14))

//
//
//  SPI
//
//

/* SERCOM instance to be used for SPI, SPI is disabled if not defined */
#define SPI0_SERCOM_INST SERCOM4
/* DMA Channel used for SPI receive, DMA not used if not defined or defined as
 -1 */
#define SPI0_RX_DMA_CHAN 4
/* DMA Channel used for SPI transmit, DMA not used if not defined or defined as
 -1 */
#define SPI0_TX_DMA_CHAN 5
/* SPI Instance */
extern struct sercom_spi_desc_t spi0_g;


//
//
//  I2C
//
//

/* SERCOM instance to be used for I2C, I2C is disabled if not defined */
#define I2C0_SERCOM_INST SERCOM5
/* DMA Channel used for I2C, DMA not used if not defined or defined as -1 */
//#define I2C0_DMA_CHAN 6
/* I2C Instance */
extern struct sercom_i2c_desc_t i2c0_g;


//
//
//  UARTs
//
//

// UART0
/* SERCOM instance to be used for UART */
#define UART0_SERCOM_INST SERCOM0
/* DMA Channel used for UART TX, DMA not used if not defined or defined as -1 */
#define UART0_DMA_CHAN 7
/* Group number for UART TX pin */
#define UART0_TX_PIN_GROUP 0
/* Pin number for UART TX pin */
#define UART0_TX_PIN_NUM 4
/* UART Instance */
extern struct sercom_uart_desc_t uart0_g;

// UART1
/* SERCOM instance to be used for UART */
#define UART1_SERCOM_INST SERCOM1
/* DMA Channel used for UART TX, DMA not used if not defined or defined as -1 */
#define UART1_DMA_CHAN 8
/* Group number for UART TX pin */
#define UART1_TX_PIN_GROUP 0
/* Pin number for UART TX pin */
#define UART1_TX_PIN_NUM 16
/* UART Instance */
extern struct sercom_uart_desc_t uart1_g;

// UART2
/* SERCOM instance to be used for UART */
#define UART2_SERCOM_INST SERCOM2
/* DMA Channel used for UART TX, DMA not used if not defined or defined as -1 */
#define UART2_DMA_CHAN 9
/* Group number for UART TX pin */
#define UART2_TX_PIN_GROUP 0
/* Pin number for UART TX pin */
#define UART2_TX_PIN_NUM 12
/* UART Instance */
extern struct sercom_uart_desc_t uart2_g;

// UART3
/* SERCOM instance to be used for UART */
#define UART3_SERCOM_INST SERCOM3
/* DMA Channel used for UART TX, DMA not used if not defined or defined as -1 */
#define UART3_DMA_CHAN 10
/* Group number for UART TX pin */
#define UART3_TX_PIN_GROUP 0
/* Pin number for UART TX pin */
#define UART3_TX_PIN_NUM 22
/* UART Instance */
extern struct sercom_uart_desc_t uart3_g;


//
//
//  MCP23S17 GPIO Expander
//
//

/* IO Expander enabled if defined */
#define ENABLE_IO_EXPANDER
/* IO Expander Instance */
extern struct mcp23s17_desc_t io_expander_g;


//
//
//  Analog
//
//

/* ADC enabled if defined */
#define ENABLE_ADC
/* Period between ADC sweeps in milliseconds */
#define ADC_PERIOD 2000
/* DMA Channel used for ADC results, DMA not used if not defined or defined
 as -1 */
#define ADC_DMA_CHAN 11
/* Maximum impedance of source in ohms, see figure 37-5 in SAMD21 datasheet */
#define ADC_SOURCE_IMPEDANCE 100000

//
//
// SD SPI
//
//

#define ENABLE_SDSPI
#define SDSPI_BAUDRATE_INIT     250000UL
#define SDSPI_BAUDRATE          12000000UL
#define SDSPI_USE_CRC   1

#ifdef ENABLE_SDSPI
extern struct sdspi_desc_t sdspi_g;
#endif

#endif /* board_h */
