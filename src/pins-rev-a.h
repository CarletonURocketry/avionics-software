/**
 * @file pins-rev-a.h
 * @desc Pinout configuration for revision A MCU board
 * @author Samuel Dewan
 * @date 2019-06-21
 * Last Author:
 * Last Edited On:
 */

#ifndef pins_rev_a_h
#define pins_rev_a_h

/* Debug LEDs */
#define DEBUG1_LED_PIN MCP23S17_PIN_FOR(MCP23S17_PORT_A, 7)
#define DEBUG0_LED_PIN GPIO_PIN_FOR(PIN_PB15)

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
#define SD_DETECT_PIN MCP23S17_PIN_FOR(MCP23S17_PORT_A, 4)

/* GPIO */
#define GPIO_0_PIN  GPIO_PIN_FOR(PIN_PB10)
#define GPIO_1_PIN  GPIO_PIN_FOR(PIN_PB11)
#define GPIO_2_PIN  GPIO_PIN_FOR(PIN_PA15)
#define GPIO_3_PIN  GPIO_PIN_FOR(PIN_PA14)
#define GPIO_4_PIN  GPIO_PIN_FOR(PIN_PA18)
#define GPIO_5_PIN  GPIO_PIN_FOR(PIN_PA19)
#define GPIO_6_PIN  GPIO_PIN_FOR(PIN_PA20)
#define GPIO_7_PIN  GPIO_PIN_FOR(PIN_PA21)
#define GPIO_8_PIN  GPIO_PIN_FOR(PIN_PB30)
#define GPIO_9_PIN  GPIO_PIN_FOR(PIN_PB23)
#define GPIO_10_PIN  GPIO_PIN_FOR(PIN_PB22)
#define GPIO_11_PIN  MCP23S17_PIN_FOR(MCP23S17_PORT_A, 4)
#define GPIO_12_PIN  MCP23S17_PIN_FOR(MCP23S17_PORT_A, 3)
#define GPIO_13_PIN  MCP23S17_PIN_FOR(MCP23S17_PORT_A, 2)
#define GPIO_14_PIN  MCP23S17_PIN_FOR(MCP23S17_PORT_A, 1)
#define GPIO_15_PIN  MCP23S17_PIN_FOR(MCP23S17_PORT_A, 0)
#define GPIO_16_PIN  MCP23S17_PIN_FOR(MCP23S17_PORT_B, 7)
#define GPIO_17_PIN  MCP23S17_PIN_FOR(MCP23S17_PORT_B, 6)
#define GPIO_18_PIN  MCP23S17_PIN_FOR(MCP23S17_PORT_B, 5)
#define GPIO_19_PIN  MCP23S17_PIN_FOR(MCP23S17_PORT_B, 4)
#define GPIO_20_PIN  MCP23S17_PIN_FOR(MCP23S17_PORT_B, 3)
#define GPIO_21_PIN  MCP23S17_PIN_FOR(MCP23S17_PORT_B, 2)
#define GPIO_22_PIN  MCP23S17_PIN_FOR(MCP23S17_PORT_B, 1)
#define GPIO_23_PIN  MCP23S17_PIN_FOR(MCP23S17_PORT_B, 0)

/* Analog */
// ADC header pin channels
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

#endif /* pins_rev_a_h */