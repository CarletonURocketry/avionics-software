/**
 * @file pins-xplained-pro.h
 * @desc Pinout configuration for revision Atmel SAMD21 Xplained Pro board
 * @author Samuel Dewan
 * @date 2019-08-29
 * Last Author:
 * Last Edited On:
 */

#ifndef pins_xplained_pro_h
#define pins_xplained_pro_h

#ifdef pins_h
#error Cannot include more than one pin description file!
#endif

#define pins_h


/* Debug LEDs */
#define DEBUG0_LED_PIN GPIO_PIN_FOR(PIN_PB30)
#define DEBUG1_LED_PIN MCP23S17_PIN_FOR(MCP23S17_PORT_A, 7)

/* Stat LEDs */
#define STAT_R_LED_PIN MCP23S17_PIN_FOR(MCP23S17_PORT_A, 6)
#define STAT_G_LED_PIN MCP23S17_PIN_FOR(MCP23S17_PORT_A, 5)

/* IO Expander */
// MCP23S17 CS pin mask
#define IO_EXPANDER_CS_PIN_MASK PORT_PA28
// MCP23S17 CS pin group
#define IO_EXPANDER_CS_PIN_GROUP 0
// MCP23S17 Interrupt pin
#define IO_EXPANDER_INT_PIN ((uint16_t)PIN_PA27)

/* SD Card */
#define SD_CS_PIN_MASK  PORT_PA11
#define SD_CS_PIN_GROUP 0
#define SD_DETECT_PIN MCP23S17_PIN_FOR(MCP23S17_PORT_A, 4)

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
#define GPIO_11  MCP23S17_PIN_FOR(MCP23S17_PORT_A, 4)
#define GPIO_12  MCP23S17_PIN_FOR(MCP23S17_PORT_A, 3)
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
#define ANALOG_A6   11
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

#endif /* pins_xplained_pro_h */
