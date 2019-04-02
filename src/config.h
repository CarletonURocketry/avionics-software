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
#define CONSOLE_UART uart3_g //remove this after

//
//
//  Debug CLI
//
//

/* Debug CLI enabled if defined */
#define ENABLE_DEBUG_CLI
/* Prompt for DEBUG CLI */




#endif /* config_h */
