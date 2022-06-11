/**
 * @file variant.h
 * @desc Rocket variant
 * @author Samuel Dewan
 * @date 2020-09-05
 * Last Author:
 * Last Edited On:
 */

#ifndef variant_h
#define variant_h

#if !defined(telemetry_h) // don't want to include telemetry.h yet
#define telemtry_h_skipped
#define telemetry_h
#endif
#include "ms5611.h"
#include "mpu9250.h"
#ifdef telemtry_h_skipped
#undef telemetry_h
#undef telemtry_h_skipped
#endif
#include "rn2483.h"
#include "radio-transport.h"
#include "radio-antmgr.h"

#include "deployment.h"

/* String to identify this configuration */
#define VARIANT_STRING "Rocket"

/* USB Strings */
#define USB_MANUFACTURER_STRING u"CU InSpace"
#define USB_PRODUCT_STRING u"CU InSpace MCU Board - Rocket"


//
//
//  Header Pins
//
//

#define ARMED_SENSE_PIN     GPIO_15
#define EMATCH_1_PIN        GPIO_22
#define EMATCH_2_PIN        GPIO_23


//
//
//  Deployment configuration
//
//

/* Acceleration threashold to trigger transition into powered ascent state in
   g */
#define DEPLOYMENT_POWERED_ASCENT_ACCEL_THREASHOLD  4
/* Backup altitude threashold to trigger transition into powered ascent state in
   meters */
#define DEPLOYMENT_POWERED_ASCENT_ALT_THREASHOLD    100
/* Acceleration threashold to trigger transition into coasting ascent state in
   g */
#define DEPLOYMENT_COASTING_ASCENT_ACCEL_THREASHOLD 1
/* Backup altitude threashold to trigger transition into coasting ascent state
   in meters */
#define DEPLOYMENT_COASTING_ASCENT_ALT_THREASHOLD   2000
/* Mininum altitude threashold for transition into coasting ascent state in
   meters */
#define DEPLOYMENT_COASTING_ASCENT_ALT_MINIMUM      500
/* Number of consecutive samples below the maximum altitude we have seen
   required to deploy drogue chute */
#define DEPLOYMENT_DESCENDING_SAMPLE_THREASHOLD     5
/* Amount of change in altitude required to indicate that we are still moving in
   meters */
#define DEPLOYMENT_LANDED_ALT_CHANGE                0.5f
/* Number of consecutive samples of staying still required before we can be
   sure that we have landed */
#define DEPLOYMENT_LANDED_SAMPLE_THREASHOLD         100

/* Length of time that current is applied to ematches in milliseconds */
#define DEPLOYMENT_EMATCH_FIRE_DURATION             500


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
#define LORA_RADIO_SEARCH_ROLE  RADIO_SEARCH_ROLE_LISTEN

/*
 * Select the device address that this endpoint should use
 */
#define LORA_DEVICE_ADDRESS RADIO_DEVICE_ADDRESS_ROCKET

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
#define ALTIMETER_PERIOD MS_TO_MILLIS(100)
extern struct ms5611_desc_t altimeter_g;

//
//
//  IMU
//
//

/* IMU enabled if defined */
#define ENABLE_IMU
/* I2C sddress for IMU */
#define IMU_ADDR 0b1101000U
/* IMU interrupt pin */
#define IMU_INT_PIN GPIO_4

#define IMU_GYRO_FSR            MPU9250_GYRO_FSR_2000DPS
#define IMU_GYRO_BW             MPU9250_GYRO_BW_41HZ
#define IMU_ACCEL_FSR           MPU9250_ACCEL_FSR_16G
#define IMU_ACCEL_BW            MPU9250_ACCEL_BW_45HZ
#define IMU_AG_SAMPLE_RATE      100
#define IMU_MAG_SAMPLE_RATE     AK8963_ODR_100HZ
#define IMU_USE_FIFO            1

#ifdef ENABLE_IMU
extern struct mpu9250_desc_t imu_g;
#endif

//
//
//  GNSS
//
//

/* GNSS enabled if defined */
#define ENABLE_GNSS
/* UART used to communicate with GNSS */
#define GNSS_UART uart2_g

//
//
//  Logging
//
//

#define ENABLE_LOGGING
//#define LOGGING_START_PAUSED

#ifdef ENABLE_LOGGING
extern struct logging_desc_t logging_g;
#endif

//
//
//  Telemetry
//
//

#define ENABLE_TELEMETRY_SERVICE

#ifdef ENABLE_TELEMETRY_SERVICE
extern struct telemetry_service_desc_t telemetry_g;
#endif

//
//
//  Deployment
//
//

#define ENABLE_DEPLOYMENT_SERVICE

#ifdef ENABLE_DEPLOYMENT_SERVICE
extern struct deployment_service_desc_t deployment_g;
#endif

#endif /* variant_h */
