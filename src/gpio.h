/**
 * @file gpio.h
 * @desc Unified driver for internal and external GPIO
 * @author Samuel Dewan
 * @date 2019-03-15
 * Last Author: Samuel Dewan
 * Last Edited On: 2020-04-05
 */

#ifndef gpio_h
#define gpio_h

#include "global.h"

#include "mcp23s17.h"
#include "rn2483.h"
#include "radio-transport.h"

#define GPIO_MAX_EXTERNAL_IO_INTERRUPTS 8

/** Type of GPIO pin */
enum gpio_pin_type {
    /** SAMD21 pin */
    GPIO_INTERNAL_PIN,
    /** Pin on MCP23S17 IO expander */
    GPIO_MCP23S17_PIN,
    /** Pin on RN2483 radio */
    GPIO_RN2483_PIN,
    /** Pin on RFM69HCW radio */
    GPIO_RFM69HCW_PIN
};

/** Represents an IO pin */
union gpio_pin_t {
    struct {
        /** GPIO pin type */
        enum gpio_pin_type type:8;
        union {
            /** SAMD21 pin */
            union {
                struct {
                    /** Pin number */
                    uint8_t pin:5;
                    /** Port, 0 for A, 1 for B */
                    uint8_t port:1;
                };
                /** Raw value (used for iterating over pins) */
                uint8_t raw:6;
            } internal;
            /** MCP23S17 Pin */
            union mcp23s17_pin_t mcp23s17;
            /** RN2483 Pin */
            struct {
                enum rn2483_pin pin:5;
                uint8_t radio:3;
            } rn2483;
            /** RFM69HCW Pin */
            uint8_t rfm69hcw;
        };
    };
    /** Raw value used for comparisons */
    uint16_t raw;
};

/** Mode for GPIO pin */
enum gpio_pin_mode {
    /** Digital circuitry for pin disabled */
    GPIO_PIN_DISABLED,
    /** Input disabled, weak output using pull resistors */
    GPIO_PIN_OUTPUT_PULL,
    /** Totem-poll (push-pull) output */
    GPIO_PIN_OUTPUT_TOTEM,
    /** Totem-poll (push-pull) output with extra drive strength enabled */
    GPIO_PIN_OUTPUT_STRONG,
    /** Input */
    GPIO_PIN_INPUT
};

/** Pull resistor configuration for GPIO pin */
enum gpio_pull_mode {
    /** No pull resistor */
    GPIO_PULL_NONE,
    /** Pull up resistor */
    GPIO_PULL_HIGH,
    /** Pull down resistor */
    GPIO_PULL_LOW
};

/** Interrupt trigger */
enum gpio_interrupt_trigger {
    /** Trigger interrupt when pin transitions from low to high */
    GPIO_INTERRUPT_RISING_EDGE,
    /** Trigger interrupt when pin transitions from high to low */
    GPIO_INTERRUPT_FALLING_EDGE,
    /** Trigger interrupt when pin changes */
    GPIO_INTERRUPT_BOTH_EDGES,
    /** Trigger interrupt when pin is high */
    GPIO_INTERRUPT_HIGH,
    /** Trigger interrupt when pin is low */
    GPIO_INTERRUPT_LOW
};

/** Type of function called when an interrupt occurs */
typedef void (*gpio_interrupt_cb)(union gpio_pin_t pin, uint8_t value);

#define GPIO_PIN_FOR(x) ((union gpio_pin_t){.type = GPIO_INTERNAL_PIN, .internal.raw = x})
#define MCP23S17_PIN_FOR(port, pin) ((union gpio_pin_t){.type = GPIO_MCP23S17_PIN, .mcp23s17.value = (pin | (port << 3))})
#define RN2483_PIN_FOR(radio, pin) ((union gpio_pin_t){.type = GPIO_RN2483_PIN, .rn2483.radio = radio, .rn2483.pin = pin})

/**
 *  Initialize the GPIO system.
 *
 *  @param eic_clock_mask Mask for the clock to be used for the external
 *                        interrupt controller
 *  @param mcp23s17 An MCP23S17 instance for external GPIO, can be NULL, must be
 *                  initialized if not NULL
 *  @param mcp23s17_int_pin Pin number for the MCP23S17 interrupt pin
 *  @param radios List of RN2483 radios that can be used for GPIO
 */
extern void init_gpio(uint32_t eic_clock_mask, struct mcp23s17_desc_t *mcp23s17,
                      uint16_t mcp23s17_int_pin,
                      struct radio_instance_desc *const *radios);

/**
 *  Set the mode of a pin.
 *
 *  @note Not all pins support all modes, pins on external devices may have
 *        substantially less functionality than internal ones. This function
 *        will return a non-zero value if the pin does not support the desired
 *        mode.
 *
 *  @param pin The pin for which the mode should be set
 *  @param mode The mode to which the pin should be set
 *
 *  @return 0 if the mode was set successfully, a non-zero value otherwise
 */
extern uint8_t gpio_set_pin_mode(union gpio_pin_t pin, enum gpio_pin_mode mode);

/**
 *  Get the mode of a pin.
 *
 *  @param pin The pin for which the mode should be determined
 *
 *  @return The current mode of the pin
 */
extern enum gpio_pin_mode gpio_get_pin_mode(union gpio_pin_t pin);

/**
 *  Set the pull resistor for a pin which is configured as an input.
 *
 *  @note Not all pins have pull-up and pull-down resistors. This function will
 *        return a non-zero value if the pin does not support the desired pull
 *        or if the pin is configured as a totem-poll output.
 *
 *  @param pin The pin for which the pull resistor should be set
 *  @param pull The desired pull resistor setting
 *
 *  @return 0 if the pull resistor was set successfully, a non-0 value otherwise
 *
 */
extern uint8_t gpio_set_pull(union gpio_pin_t pin, enum gpio_pull_mode pull);

/**
 *  Get the value from a pin which is configured as an input.
 *
 *  @note If the pin is configured as an output its output value will be
 *        returned.
 *
 *  @param pin The pin for which the value should be found
 *
 *  @return The value of the pin, 1 = logic high, 0 = logic low
 */
extern uint8_t gpio_get_input(union gpio_pin_t pin);

/**
 *  Set the value of a pin which is configured as an output.
 *
 *  @param pin The pin for which the value should be set
 *  @param value The new value for the pin, 1 = logic high, 0 = logic low
 *
 *  @return 0 if successful, a non-zero value if the pin is not configured as
 *          an output and/or does not support output
 */
extern uint8_t gpio_set_output(union gpio_pin_t pin, uint8_t value);

/**
 *  Toggle the value of a pin which is configured as an output.
 *
 *  @param pin The pin for which the value should be toggled
 *
 *  @return 0 if successful, a non-zero value if the pin is not configured as
 *          an output and/or does not support output
 */
extern uint8_t gpio_toggle_output(union gpio_pin_t pin);

/**
 *  Enabled an interrupt for a pin which is configured as an input.
 *
 *  @param pin The pin for which the interrupt should be enabled
 *  @param trigger The trigger type for the interrupt
 *  @param filter Whether or not a filter should be
 *  @param callback The function to be called when the interrupt occurs
 *
 *  @return 0 if interrupt enabled successfully, 1 otherwise
 */
extern uint8_t gpio_enable_interrupt(union gpio_pin_t pin,
                                    enum gpio_interrupt_trigger trigger,
                                    uint8_t filter,
                                    gpio_interrupt_cb callback);

/**
 *  Disable the interrupt for a pin.
 *
 *  @param pin The pin for which the interrupt should be disabled
 *
 *  @return 0 if successful or 1 if the interrupt could not be disabled, if the
 *          interrupt was not enabled it is considered to have been successfully
 *          disabled
 */
extern uint8_t gpio_disable_interrupt(union gpio_pin_t pin);

#endif /* gpio_h */
