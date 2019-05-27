/**
 * @file config.h
 * @desc Hardware Configuration
 * @author Samuel Dewan
 * @date 2019-02-15
 * Last Author:
 * Last Edited On:
 */

#ifndef config_h
#define config_h

#include "sercom-spi.h"
#include "sercom-i2c.h"
#include "sercom-uart.h"

#include "gpio.h"

//
//
//  General
//
//

/* String to identify this configuration */
#define CONFIG_STRING "Test Config\n"
/* Globaly enable DMA if defined */
#define ENABLE_DMA
/* Enable Micro Trace Buffer if defined */
#define ENABLE_MTB

/* Debug LEDs */
#define DEBUG0_LED_PIN GPIO_PIN_FOR(PIN_PB15)
#define DEBUG1_LED_PIN MCP23S17_PIN_FOR(MCP23S17_PORT_A, 7)
/* Stat LEDs */
#define STAT_R_LED_PIN MCP23S17_PIN_FOR(MCP23S17_PORT_A, 6)
#define STAT_G_LED_PIN MCP23S17_PIN_FOR(MCP23S17_PORT_A, 5)

//
//
//  SPI
//
//

/* SERCOM instance to be used for SPI, SPI is disabled if not defined */
#define SPI_SERCOM_INST SERCOM4
/* DMA Channel used for SPI receive, DMA not used if not defined or defined as -1 */
#define SPI_RX_DMA_CHAN 4
/* DMA Channel used for SPI transmit, DMA not used if not defined or defined as -1 */
#define SPI_TX_DMA_CHAN 5
/* SPI Instance */
extern struct sercom_spi_desc_t spi_g;


//
//
//  I2C
//
//

/* SERCOM instance to be used for I2C, I2C is disabled if not defined */
#define I2C_SERCOM_INST SERCOM5
/* DMA Channel used for I2C, DMA not used if not defined or defined as -1 */
#define I2C_DMA_CHAN 6
/* I2C Speed, defaults to I2C_MODE_STANDARD (100 KHz) if not defined */
#define I2C_SPEED I2C_MODE_FAST
/* I2C Instance */
extern struct sercom_i2c_desc_t i2c_g;


//
//
//  UARTs
//
//

// UART0
/* SERCOM instance to be used for UART, UART is disabled if not defined */
#define UART0_SERCOM_INST SERCOM0
/* DMA Channel used for UART TX, DMA not used if not defined or defined as -1 */
#define UART0_DMA_CHAN 7
/* Baud rate for UART */
#define UART0_BAUD 9600UL
/* Define as one if UART should echo received bytes and provide line editing,
   0 otherwise */
#define UART0_ECHO 0
/* UART Instance */
extern struct sercom_uart_desc_t uart0_g;

// UART1
/* SERCOM instance to be used for UART, UART is disabled if not defined */
#define UART1_SERCOM_INST SERCOM1
/* DMA Channel used for UART TX, DMA not used if not defined or defined as -1 */
#define UART1_DMA_CHAN 8
/* Baud rate for UART */
#define UART1_BAUD 9600UL
/* Define as one if UART should echo received bytes and provide line editing,
   0 otherwise */
#define UART1_ECHO 0
/* UART Instance */
extern struct sercom_uart_desc_t uart1_g;

// UART2
/* SERCOM instance to be used for UART, UART is disabled if not defined */
#define UART2_SERCOM_INST SERCOM2
/* DMA Channel used for UART TX, DMA not used if not defined or defined as -1 */
#define UART2_DMA_CHAN 9
/* Baud rate for UART */
#define UART2_BAUD 9600UL
/* Define as one if UART should echo received bytes and provide line editing,
   0 otherwise */
#define UART2_ECHO 0
/* UART Instance */
extern struct sercom_uart_desc_t uart2_g;

// UART3
/* SERCOM instance to be used for UART, UART is disabled if not defined */
#define UART3_SERCOM_INST SERCOM3
/* DMA Channel used for UART TX, DMA not used if not defined or defined as -1 */
#define UART4_DMA_CHAN 10
/* Baud rate for UART */
#define UART3_BAUD 115200UL
/* Define as one if UART should echo received bytes and provide line editing,
   0 otherwise */
#define UART3_ECHO 1
/* UART Instance */
extern struct sercom_uart_desc_t uart3_g;

//
//
//  USB
//
//

/* USB enabled if defined */
#define ENABLE_USB

//
//
// Console
//
//

/* Console enabled if defined */
#define ENABLE_CONSOLE
/* UART instance to be used for console, USB is used if not defined (and USB is
   enabled) */
//#define CONSOLE_UART uart3_g

//
//
//  MCP23S17 GPIO Expander
//
//

/* IO Expander enabled if defined */
#define ENABLE_IO_EXPANDER
/* IO Expander Instance */
extern struct mcp23s17_desc_t io_expander_g;
/* MCP23S17 CS pin mask */
#define IO_EXPANDER_CS_PIN_MASK PORT_PA28
/* MCP23S17 CS pin group */
#define IO_EXPANDER_CS_PIN_GROUP 0
/* NCP23S17 Interrupt pin */
#define IO_EXPANDER_INT_PIN ((uint16_t)PIN_PA28)

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
/* Timer Counter used for ADC timing, TC not used if not defined or if NULL */
#define ADC_TC NULL
/* Event Channel used for ADC timing, EVSYS not used if not defined or defined
   as -1, must be defined as a valid channel if TC is defined as a value other
   than NULL */
#define ADC_EVENT_CHAN -1
/* ADC header pin channels */
#define HEADER_A0   18
#define HEADER_A1   17
#define HEADER_A2   7
#define HEADER_A3   3
#define HEADER_A4   15
#define HEADER_A5   13
#define HEADER_A6   11
#define HEADER_A7   9
#define HEADER_A8   8
#define HEADER_A9   10
#define HEADER_A10  12
#define HEADER_A11  14
#define HEADER_A12  2
#define HEADER_A13  6
#define HEADER_A14  16

#define NUM_ANALOG_HEADER_PINS 15

#define HEADER_ANALOG_PINS {HEADER_A0, HEADER_A1, HEADER_A2, HEADER_A3, \
                            HEADER_A4, HEADER_A5, HEADER_A6, HEADER_A7, \
                            HEADER_A8, HEADER_A9, HEADER_A10, HEADER_A11, \
                            HEADER_A12, HEADER_A13, HEADER_A14}

#define EXTERNAL_ANALOG_MASK ((1 << HEADER_A0) | (1 << HEADER_A1) | \
                              (1 << HEADER_A2) | (1 << HEADER_A3) | \
                              (1 << HEADER_A4) | (1 << HEADER_A5) | \
                              (1 << HEADER_A6) | (1 << HEADER_A7) | \
                              (1 << HEADER_A8) | (1 << HEADER_A9) | \
                              (1 << HEADER_A10) | (1 << HEADER_A11) | \
                              (1 << HEADER_A12) | (1 << HEADER_A13) | \
                              (1 << HEADER_A14))

//
//
//  Debug CLI
//
//

/* Debug CLI enabled if defined */
#define ENABLE_DEBUG_CLI
/* Prompt for DEBUG CLI */




#endif /* config_h */
