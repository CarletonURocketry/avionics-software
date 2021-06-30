/**
 * @file variant.h
 * @desc Test variant
 * @author Samuel Dewan
 * @date 2020-09-05
 * Last Author:
 * Last Edited On:
 */

#ifndef variant_h
#define variant_h

#include "ms5611.h"
#include "rn2483.h"
#include "radio-transport.h"
#include "radio-antmgr.h"

/* String to identify this configuration */
#define VARIANT_STRING "Test"

/* USB Strings */
#define USB_MANUFACTURER_STRING u"CU InSpace"
#define USB_PRODUCT_STRING u"CU InSpace MCU Board"

//
//
//  I2C
//
//

/* Uncomment to initialize I2C interface */
#define ENABLE_I2C0
/* I2C Speed, defaults to I2C_MODE_STANDARD (100 KHz) if not defined */
#define I2C0_SPEED I2C_MODE_FAST

/* Uncomment to initialize I2C interface */
#define ENABLE_I2C1
/* I2C Speed, defaults to I2C_MODE_STANDARD (100 KHz) if not defined */
#define I2C1_SPEED I2C_MODE_FAST

//
//
//  SPI
//
//

/* Uncomment to initialize SPI interface */
#define ENABLE_SPI0

/* Uncomment to initialize SPI interface */
#define ENABLE_SPI1

//
//
//  UARTs
//
//

// UART0
/* Uncomment to initialize UART */
#define ENABLE_UART0
/* Baud rate for UART */
#define UART0_BAUD 57600UL
/* Define as one if UART should echo received bytes and provide line editing,
    0 otherwise */
#define UART0_ECHO 0

// UART1
/* Uncomment to initialize UART */
#define ENABLE_UART1
/* Baud rate for UART */
#define UART1_BAUD 57600UL
/* Define as one if UART should echo received bytes and provide line editing,
    0 otherwise */
#define UART1_ECHO 0

// UART2
/* Uncomment to initialize UART */
#define ENABLE_UART2
/* Baud rate for UART */
#define UART2_BAUD 115200UL
/* Define as one if UART should echo received bytes and provide line editing,
    0 otherwise */
#define UART2_ECHO 0

// UART3
/* Uncomment to initialize UART */
#define ENABLE_UART3
/* Baud rate for UART */
#define UART3_BAUD 9600UL
/* Define as one if UART should echo received bytes and provide line editing,
    0 otherwise */
#define UART3_ECHO 0

//
//
//  USB
//
//

/* USB enabled if defined */
#define ENABLE_USB

/* Define to enable USB CDC port 0 */
#define ENABLE_USB_CDC_PORT_0
/* Define to enable echo on USB CDC port 0 */
#define USB_CDC_PORT_0_ECHO
/* Define to enable USB CDC port 1 */
//#define ENABLE_USB_CDC_PORT_1
/* Define to enable echo on USB CDC port 1 */
//#define USB_CDC_PORT_1_ECHO
/* Define to enable USB CDC port 2 */
//#define ENABLE_USB_CDC_PORT_2
/* Define to enable echo on USB CDC port 2 */
//#define USB_CDC_PORT_2_ECHO
// Note: USB CDC ports should be enabled in order to minimize memory usage

//
//
// Console
//
//

/* Debugging console enabled if defined */
#define ENABLE_CONSOLE
/* UART instance to be used for console, USB is used if not defined (and USB is
 enabled) */
//#define CONSOLE_UART uart3_g
/* USB CDC port to be used for debugging console, this is ignored if
   CONSOLE_UART is defined */
#define CONSOLE_CDC_PORT 0

//
//
//  Debug CLI
//
//

/* Debug CLI enabled if defined */
#define ENABLE_DEBUG_CLI
/* Prompt for DEBUG CLI */


//
//
//  Radio
//
//

/** Defining this macro enables the use of LoRa radios */
#define ENABLE_LORA


#ifdef ENABLE_LORA

extern struct radio_transport_desc radio_transport_g;
extern struct radio_instance_desc *const radios_g[];

/*
 * Up to four radios (radios 0 through 3) each can be enabled by defining the
 * macro `LORA_RADIO_n_UART`. If the UART marco is not defined for a radio that
 * radio instance will not be enabled.
 *
 * Each radio may have an associated antenna switch. The antenna presence of an
 * switch for a radio can be indicated by defining one of two macros:
 *  LORA_RADIO_n_ANT_MASK
 *      If this macro is defined an antenna manager module will be initialized
 *      for the radio using the given mask to indicate which of it's antenna
 *      ports can be used. This mask should be a binary or of the macros of the
 *      form ANTMGR_ANT_n_MASK where n is between 1 and 4 inclusive.
 *  LORA_RADIO_0_ANT_FIXED
 *      If this macro is defined the antenna switch will be configured to use a
 *      specific antenna port and no antmgr module will be initialized.
 * It is not valid to define both LORA_RADIO_n_ANT_MASK and
 * LORA_RADIO_n_ANT_FIXED.
 */

// Radio 0
#define LORA_RADIO_0_UART   uart1_g
//#define LORA_RADIO_0_ANT_MASK (ANTMGR_ANT_1_MASK | ANTMGR_ANT_2_MASK |
//                                ANTMGR_ANT_3_MASK | ANTMGR_ANT_4_MASK)
#define LORA_RADIO_0_ANT_FIXED  4
// Radio 1
//#define LORA_RADIO_1_UART   uart0_g
// Radio 2
//#define LORA_RADIO_2_UART   uart2_g
// Radio 3
//#define LORA_RADIO_3_UART   uart3_g

/*
 * Select the role that this endpoint should take on when it falls back into
 * search mode.
 */
#define LORA_RADIO_SEARCH_ROLE  RADIO_SEARCH_ROLE_ADVERTISE

/*
 * Select the device address that this endpoint should use
 */
#define LORA_DEVICE_ADDRESS RADIO_DEVICE_ADDRESS_GROUND_STATION

#endif /* ENABLE_LORA */

//
//
//  Altimeter
//
//

/* Altimeter enabled if defined */
#define ENABLE_ALTIMETER
/* Altimeter CSB setting */
#define ALTIMETER_CSB 0
/* Altimeter sample period in milliseconds */
#define ALTIMETER_PERIOD MS_TO_MILLIS(1000)
extern struct ms5611_desc_t altimeter_g;

//
//
// GNSS
//
//

/* GNSS enabled if defined */
#define ENABLE_GNSS
/* UART used to communicate with GNSS */
#define GNSS_UART uart2_g

#endif /* variant_h */
