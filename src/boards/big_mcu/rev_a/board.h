/**
 * @file board.h
 * @desc Board configuration for SAME54 MCU board
 * @author Samuel Dewan
 * @date 2021-06-05
 * Last Author:
 * Last Edited On:
 */

#ifndef board_h
#define board_h

#include "gpio.h"

#define BOARD_STRING "Big MCU Rev. A"

//
//
//  General Flags
//
//

#define ENABLE_WATCHDOG
#define DEBUG_BLINK_PERIOD MS_TO_MILLIS(1000)

//
//
//  Pin Definitions
//
//

/* Debug LEDs */
#define DEBUG0_LED_PIN GPIO_PIN_FOR(PIN_PA18)
#define DEBUG1_LED_PIN GPIO_PIN_FOR(PIN_PA19)

/* Stat LEDs */
#define STAT_R_LED_PIN GPIO_PIN_FOR(PIN_PB21)
#define STAT_G_LED_PIN GPIO_PIN_FOR(PIN_PB20)
#define STAT_B_LED_PIN GPIO_PIN_FOR(PIN_PB19)

#define SD_ACTIVE_LED_PIN GPIO_PIN_FOR(PIN_PB31)

/* GPIO */
#define GPIO_0  GPIO_PIN_FOR(PIN_PB13)
#define GPIO_1  GPIO_PIN_FOR(PIN_PD10)
#define GPIO_2  GPIO_PIN_FOR(PIN_PD11)
#define GPIO_3  GPIO_PIN_FOR(PIN_PD12)
#define GPIO_4  GPIO_PIN_FOR(PIN_PC10)
#define GPIO_5  GPIO_PIN_FOR(PIN_PC11)
#define GPIO_6  GPIO_PIN_FOR(PIN_PC12)
#define GPIO_7  GPIO_PIN_FOR(PIN_PC13)
#define GPIO_8  GPIO_PIN_FOR(PIN_PA07)
#define GPIO_9  GPIO_PIN_FOR(PIN_PA06)
#define GPIO_10 GPIO_PIN_FOR(PIN_PA04)
#define GPIO_11 GPIO_PIN_FOR(PIN_PB09)
#define GPIO_12 GPIO_PIN_FOR(PIN_PB08)
#define GPIO_13 GPIO_PIN_FOR(PIN_PB07)
#define GPIO_14 GPIO_PIN_FOR(PIN_PB06)
#define GPIO_15 GPIO_PIN_FOR(PIN_PD01)
#define GPIO_16 PIN_NONE
#define GPIO_17 PIN_NONE
#define GPIO_18 PIN_NONE
#define GPIO_19 PIN_NONE
#define GPIO_20 PIN_NONE
#define GPIO_21 PIN_NONE
#define GPIO_22 GPIO_PIN_FOR(PIN_PB22)
#define GPIO_23 GPIO_PIN_FOR(PIN_PB23)

/* Analog */
#define DAC_OUT0 GPIO_PIN_FOR(PIN_PA05)
#define DAC_OUT0_CHANNEL    1
#define DAC_OUT0_CHAN_MASK  (1 << DAC_OUT0_CHANNEL)

#define DAC_OUT1 GPIO_PIN_FOR(PIN_PA02)
#define DAC_OUT1_CHANNEL    0
#define DAC_OUT1_CHAN_MASK  (1 << DAC_OUT1_CHANNEL)

#if 0
// ADC analog header pin channels
#define ANALOG_A0   19 //adc0 ain[4]
#define ANALOG_A1   27 //adc1 ain[11]
#define ANALOG_A2   14 //adc0 ain[14]
#define ANALOG_A3   13 //adc0 ain[13]
#define ANALOG_A4   12 //adc1 ain[12]
#define ANALOG_A5   29 //adc1 ain[13]
#define ANALOG_A6   28 //adc1 ain[12]
#define ANALOG_A7   15 //adc0 ain[15]

// ADC internal channels 
#define INTERNAL_SCALED_CORE_VCC	32
#define INTERNAL_SCALED_VBAT 		33	
#define INTERNAL_SCALED_IO_VCC		34
#define INTERNAL_BANDGAP_VCC		35
#define INTERNAL_TEMP_SENSOR_PTAT	36
#define INTERNAL_TEMP_SENSOR_CTAT	37
#define INTERNAL_DAC			    38


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
#endif



//
//
//  SPI 0
//
//

/* SERCOM instance to be used for SPI0, SPI0 is disabled if not defined */
#define SPI0_SERCOM_INST SERCOM6
/* DMA Channel used for SPI0 receive, DMA not used if not defined or defined as
 -1 */
#define SPI0_RX_DMA_CHAN 8
/* DMA Channel used for SPI0 transmit, DMA not used if not defined or defined as
 -1 */
#define SPI0_TX_DMA_CHAN 9
/* SPI Instance */
extern struct sercom_spi_desc_t spi0_g;

//
//
//  SPI 1
//
//

/* SERCOM instance to be used for SPI1, SPI1 is disabled if not defined */
#define SPI1_SERCOM_INST SERCOM5
/* DMA Channel used for SPI1 receive, DMA not used if not defined or defined as
 -1 */
#define SPI1_RX_DMA_CHAN 10
/* DMA Channel used for SPI1 transmit, DMA not used if not defined or defined as
 -1 */
#define SPI1_TX_DMA_CHAN 11
/* SPI Instance */
extern struct sercom_spi_desc_t spi1_g;


//
//
//  I2C 0
//
//

/* SERCOM instance to be used for I2C0, I2C0 is disabled if not defined */
#define I2C0_SERCOM_INST SERCOM7
/* DMA Channel used for I2C0, DMA not used if not defined or defined as -1 */
#define I2C0_DMA_CHAN 12
/* I2C Instance */
extern struct sercom_i2c_desc_t i2c0_g;

//
//
//  I2C 1
//
//

/* SERCOM instance to be used for I2C1, I2C1 is disabled if not defined */
#define I2C1_SERCOM_INST SERCOM2
/* DMA Channel used for I2C1, DMA not used if not defined or defined as -1 */
#define I2C1_DMA_CHAN 13
/* I2C Instance */
extern struct sercom_i2c_desc_t i2c1_g;


//
//
//  UARTs
//
//

// UART0
/* SERCOM instance to be used for UART */
#define UART0_SERCOM_INST SERCOM1
/* DMA Channel used for UART TX, DMA not used if not defined or defined as -1 */
#define UART0_DMA_CHAN 14
/* Group number for UART TX pin */
#define UART0_TX_PIN_GROUP 0
/* Pin number for UART TX pin */
#define UART0_TX_PIN_NUM 16
/* UART Instance */
extern struct sercom_uart_desc_t uart0_g;

// UART1
/* SERCOM instance to be used for UART */
#define UART1_SERCOM_INST SERCOM0
/* DMA Channel used for UART TX, DMA not used if not defined or defined as -1 */
#define UART1_DMA_CHAN 15
/* Group number for UART TX pin */
#define UART1_TX_PIN_GROUP 2
/* Pin number for UART TX pin */
#define UART1_TX_PIN_NUM 17
/* UART Instance */
extern struct sercom_uart_desc_t uart1_g;

// UART2
/* SERCOM instance to be used for UART */
#define UART2_SERCOM_INST SERCOM3
/* DMA Channel used for UART TX, DMA not used if not defined or defined as -1 */
#define UART2_DMA_CHAN 16
/* Group number for UART TX pin */
#define UART2_TX_PIN_GROUP 2
/* Pin number for UART TX pin */
#define UART2_TX_PIN_NUM 23
/* UART Instance */
extern struct sercom_uart_desc_t uart2_g;

// UART3
/* SERCOM instance to be used for UART */
#define UART3_SERCOM_INST SERCOM4
/* DMA Channel used for UART TX, DMA not used if not defined or defined as -1 */
#define UART3_DMA_CHAN 17
/* Group number for UART TX pin */
#define UART3_TX_PIN_GROUP 1
/* Pin number for UART TX pin */
#define UART3_TX_PIN_NUM 27
/* UART Instance */
extern struct sercom_uart_desc_t uart3_g;



//
//
//  Analog
//
//

// TODO: SAME54 Analog
///* ADC enabled if defined */
//#define ENABLE_ADC
///* Period between ADC sweeps in milliseconds */
//#define ADC_PERIOD 2000
///* DMA Channel used for ADC results, DMA not used if not defined or defined
// as -1 */
//#define ADC_DMA_CHAN 11
///* Maximum impedance of source in ohms, see figure 37-5 in SAMD21 datasheet */
//#define ADC_SOURCE_IMPEDANCE 100000


//
//
// SD Host Controller 0
//
//

#define ENABLE_SDHC0

#ifdef ENABLE_SDHC0
extern struct sdhc_desc_t sdhc0_g;
#endif


//
//
// CAN 0
//
//

//#define ENABLE_CAN0

#define CAN0_STANDBY_PIN    PIN_PB29


//
//
// CAN 1
//
//

//#define ENABLE_CAN0

#define CAN1_STANDBY_PIN    PIN_PB28


//
//
// KX134-1211 Accelerometer
//
//

#define ENABLE_KX134_1211

// Chip select pin
#define KX134_1211_CS_PIN_GROUP 3
#define KX134_1211_CS_PIN_MASK  PORT_PD21
// Interrupt pins
#define KX134_1211_INT1_PIN GPIO_PIN_FOR(PIN_PC19)
#define KX134_1211_INT2_PIN GPIO_PIN_FOR(PIN_PC18)
// Trigger pin
#define KX134_1211_TRIG_PIN GPIO_PIN_FOR(PIN_PD20)

// Settings
#define KX134_1211_RANGE            KX134_1211_RANGE_32G
#define KX134_1211_LOW_PASS_ROLLOFF KX134_1211_LOW_PASS_ROLLOFF_2
#define KX134_1211_ODR              KX134_1211_ODR_6400000
#define KX134_1211_RES              KX134_1211_RES_16_BIT

#ifdef ENABLE_KX134_1211
extern struct kx134_1211_desc_t kx134_g;
#endif


//
//
// Temperature Sensor
//
//


//#define ENABLE_EXT_TEMP_SENSE

#define EXT_TEMP_SENSE_VDD_PIN  PIN_PC07
// TODO: Analog chan for temperature sensor

#endif /* board_h */
