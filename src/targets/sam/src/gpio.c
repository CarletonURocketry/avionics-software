/**
 * @file gpio.c
 * @desc Unified driver for internal and external GPIO
 * @author Samuel Dewan
 * @date 2019-03-15
 * Last Author: Samuel Dewan
 * Last Edited On: 2020-04-05
 */

#include "gpio.h"

#include "target.h"

#define EIC_IRQ_PRIORITY   3

#ifndef PORT_IOBUS
#define PORT_IOBUS PORT
#endif

struct gpio_interrupt_t {
    gpio_interrupt_cb callback;
    void *context;
};

/**
 *  SAMD21 external interrupt handlers
 */
static struct gpio_interrupt_t gpio_int_callbacks[EIC_EXTINT_NUM] = {0};

struct external_io_int_t {
    gpio_interrupt_cb callback;
    void *context;
    union gpio_pin_t pin;
};

/**
 *  External IO interrupt handlers
 */
struct external_io_int_t gpio_ext_io_ints[GPIO_MAX_EXTERNAL_IO_INTERRUPTS] = {0};


/**
 *  Pointer to descriptor for MCP23S17 IO expander.
 */
static struct mcp23s17_desc_t *gpio_mcp23s17_g;

/**
 *  Array of descriptors for RN2483 radios with GPIO.
 */
static struct radio_instance_desc *const *gpio_radios_g;


/**
 *  Callback function for MCP23S17 interrupt
 */
static void gpio_mcp23s17_int_cb (void *context, union gpio_pin_t pin,
                                  uint8_t value);

/**
 *  Function to be called by MCP23S17 driver when an interrupt has occurred
 */
static void gpio_mcp23s17_interrupt_occurred (struct mcp23s17_desc_t *inst,
                                             union mcp23s17_pin_t pin,
                                             uint8_t value);

/**
 *  Returns the a pointer to the radio instance for a given radio number or
 *  NULL if no such radio is available
 */
static struct rn2483_desc_t *gpio_get_rn2483_inst (uint8_t radio_num);


#if defined(SAMD2x)
#define NUM_GPIO_PIN_INTERRUPTS  64
static const int8_t gpio_pin_interrupts[] = {
    //PA0                       PA7
    0,  1,  2,  3,  4,  5,  6,  7,
    //PA8                       PA15
    -1,  9, 10, 11, 12, 13, 14, 15,
    //PA16                      PA23
    0,  1,  2,  3,  4,  5,  6,  7,
    //PA24                      PA31
    12, 13, -2, 15,  8, -2, 10, 11,
    //PB0                       PB7
    0,  1,  2,  3,  4,  5,  6,  7,
    //PB8                       PB15
    8,  9, 10, 11, 12, 13,  14, 15,
    //PB16                      PB23
    0,  1, -2, -2, -2, -2,  6,  7,
    //PB24                      PB31
    -2, -2, -2, -2, -2, -2, 14, 15
};
#elif defined(SAMx5x)
#define NUM_GPIO_PIN_INTERRUPTS  128
static const int8_t gpio_pin_interrupts[] = {
    //PA0                       PA7
    0,  1,  2,  3,  4,  5,  6,  7,
    //PA8                       PA15
    -1, 9,  10, 11, 12, 13, 14, 15,
    //PA16                      PA23
    0,  1,  2,  3,  4,  5,  6,  7,
    //PA24                      PA31
    8,  9,  -2, 11, -2, -2, 14, 15,
    //PB0                       PB7
    0,  1,  2,  3,  4,  5,  6,  7,
    //PB8                       PB15
    8,  9,  10, 11, 12, 13, 14, 15,
    //PB16                      PB23
    0,  1,  2,  3,  4,  5,  6,  7,
    //PB24                      PB31
    8,  9,  12, 13, 14, 15, 14, 15,
    //PC0                       PC7
    0,  1,  2,  3,  4,  5,  6,  9,
    //PC8                       PC15
    -2, -2, 10, 11, 12, 13, 14, 15,
    //PC16                      PC23
    0,  1,  2,  3,  4,  5,  6,  7,
    //PC24                      PC31
    8,  9,  10, 11, 12, -2, 14, 15,
    //PD0                       PD7
    0,  1,  -2, -2, -2, -2, -2, -2,
    //PD8                       PD15
    3,  4,  5,  6,  7,  -2, -2, -2,
    //PD16                      PD23
    -2, -2, -2, -2, 10, 11, -2, -2,
    //PD24                      PD31
    -2, -2, -2, -2, -2, -2, -2, -2
};

#endif


void init_gpio(uint32_t eic_clock_mask, struct mcp23s17_desc_t *mcp23s17,
               uint16_t mcp23s17_int_pin,
               struct radio_instance_desc *const *radios)
{
    gpio_mcp23s17_g = mcp23s17;
    gpio_radios_g = radios;

    /* Configure External Interrupt Controller */

    // CLK_EIC_APB is enabled by default, so we will not enable it here
    /* Select a core clock for the EIC to allow edge detection and filtering */
    set_perph_generic_clock(PERPH_GCLK_EIC, eic_clock_mask);

    /* Reset EIC */
#if defined(SAMD2x)
    EIC->CTRL.bit.SWRST = 1;
    // Wait for reset to complete
    while (EIC->CTRL.bit.SWRST);
#elif defined(SAMx5x)
    EIC->CTRLA.bit.SWRST = 1;
    // Wait for reset to complete
    while (EIC->CTRLA.bit.SWRST);
#endif

    /* Ensure that NMI is disabled */
    EIC->NMICTRL.bit.NMISENSE = 0;

    /* Ensure that all events and interrupts start disabled and cleared */
    EIC->EVCTRL.reg = 0;
    EIC->INTENCLR.reg = 0xFFFF;
    EIC->INTFLAG.reg = 0xFFFF;

    /* Configure MCP23S17 interrupt pin */
    if (mcp23s17 != NULL) {
#if defined(SAMD2x)
        /* Ensure that no external interrupts will wake the CPU except for the
         MCP23S17 interrupt */
        EIC->WAKEUP.reg = (1 << gpio_pin_interrupts[mcp23s17_int_pin]);
#endif

        union gpio_pin_t mcp23s17_gpio = GPIO_PIN_FOR(mcp23s17_int_pin);

        uint8_t const mcp23s17_eic_num = gpio_pin_interrupts[mcp23s17_int_pin];
        gpio_int_callbacks[mcp23s17_eic_num].callback = &gpio_mcp23s17_int_cb;

        // Enable input
        PORT_IOBUS->Group[mcp23s17_gpio.internal.port].PINCFG[
                                    mcp23s17_gpio.internal.pin].bit.INEN = 1;
        // Set PMUX to interrupt (function A)
        if (mcp23s17_int_pin & 1) {
            PORT_IOBUS->Group[mcp23s17_gpio.internal.port].PMUX[mcp23s17_gpio.internal.pin >> 1].bit.PMUXO = 0x0;
        } else {
            PORT_IOBUS->Group[mcp23s17_gpio.internal.port].PMUX[mcp23s17_gpio.internal.pin >> 1].bit.PMUXE = 0x0;
        }
        // Enable PMUX
        PORT_IOBUS->Group[mcp23s17_gpio.internal.port].PINCFG[
                                    mcp23s17_gpio.internal.pin].bit.PMUXEN = 1;

        // Set sense for interrupt to falling edge with filter
        EIC->CONFIG[gpio_pin_interrupts[mcp23s17_int_pin] >> 3].reg |=
                        ((EIC_CONFIG_FILTEN0 <<
                          (4 * (gpio_pin_interrupts[mcp23s17_int_pin] & 0x7))) |
                         (EIC_CONFIG_SENSE0_FALL <<
                          (4 * (gpio_pin_interrupts[mcp23s17_int_pin] & 0x7))));

        // Enable interrupt for MCP23S17 interrupt pin
        EIC->INTENSET.reg = (1 << gpio_pin_interrupts[mcp23s17_int_pin]);

        // Set MCP23S17 interrupt callback
        mcp23s17_set_interrupt_callback(gpio_mcp23s17_g,
                                        gpio_mcp23s17_interrupt_occurred);
    }

    /* Enabled interrupts from EIC in NVIC */
#if defined(SAMD2x)
    NVIC_SetPriority(EIC_IRQn, EIC_IRQ_PRIORITY);
    NVIC_EnableIRQ(EIC_IRQn);
#elif defined(SAMx5x)
    NVIC_SetPriority(EIC_0_IRQn, EIC_IRQ_PRIORITY);
    NVIC_EnableIRQ(EIC_0_IRQn);
    NVIC_SetPriority(EIC_1_IRQn, EIC_IRQ_PRIORITY);
    NVIC_EnableIRQ(EIC_1_IRQn);
    NVIC_SetPriority(EIC_2_IRQn, EIC_IRQ_PRIORITY);
    NVIC_EnableIRQ(EIC_2_IRQn);
    NVIC_SetPriority(EIC_3_IRQn, EIC_IRQ_PRIORITY);
    NVIC_EnableIRQ(EIC_3_IRQn);
    NVIC_SetPriority(EIC_4_IRQn, EIC_IRQ_PRIORITY);
    NVIC_EnableIRQ(EIC_4_IRQn);
    NVIC_SetPriority(EIC_5_IRQn, EIC_IRQ_PRIORITY);
    NVIC_EnableIRQ(EIC_5_IRQn);
    NVIC_SetPriority(EIC_6_IRQn, EIC_IRQ_PRIORITY);
    NVIC_EnableIRQ(EIC_6_IRQn);
    NVIC_SetPriority(EIC_7_IRQn, EIC_IRQ_PRIORITY);
    NVIC_EnableIRQ(EIC_7_IRQn);
    NVIC_SetPriority(EIC_8_IRQn, EIC_IRQ_PRIORITY);
    NVIC_EnableIRQ(EIC_8_IRQn);
    NVIC_SetPriority(EIC_9_IRQn, EIC_IRQ_PRIORITY);
    NVIC_EnableIRQ(EIC_9_IRQn);
    NVIC_SetPriority(EIC_10_IRQn, EIC_IRQ_PRIORITY);
    NVIC_EnableIRQ(EIC_10_IRQn);
    NVIC_SetPriority(EIC_11_IRQn, EIC_IRQ_PRIORITY);
    NVIC_EnableIRQ(EIC_11_IRQn);
    NVIC_SetPriority(EIC_12_IRQn, EIC_IRQ_PRIORITY);
    NVIC_EnableIRQ(EIC_12_IRQn);
    NVIC_SetPriority(EIC_13_IRQn, EIC_IRQ_PRIORITY);
    NVIC_EnableIRQ(EIC_13_IRQn);
    NVIC_SetPriority(EIC_14_IRQn, EIC_IRQ_PRIORITY);
    NVIC_EnableIRQ(EIC_14_IRQn);
    NVIC_SetPriority(EIC_15_IRQn, EIC_IRQ_PRIORITY);
    NVIC_EnableIRQ(EIC_15_IRQn);
#endif

    /* Enable EIC */
#if defined(SAMD2x)
    EIC->CTRL.bit.ENABLE = 1;
#elif defined(SAMx5x)
    EIC->CTRLA.bit.ENABLE = 1;
#endif
}

uint8_t gpio_set_pin_mode(union gpio_pin_t pin, enum gpio_pin_mode mode)
{
    struct rn2483_desc_t *rn2483;
    switch (pin.type) {
        case GPIO_INTERNAL_PIN:
            // Set or clear DIR
            if ((mode == GPIO_PIN_OUTPUT_TOTEM) ||
                (mode == GPIO_PIN_OUTPUT_STRONG)) {
                // Output circuitry should be enabled, set DIR
                PORT_IOBUS->Group[pin.internal.port].DIRSET.reg = (1 << pin.internal.pin);
            } else {
                // Output circuitry should be disabled, clear DIR
                PORT_IOBUS->Group[pin.internal.port].DIRCLR.reg = (1 << pin.internal.pin);
            }

            // Write to INEN, PULLEN and DRVSTR
            PORT_IOBUS->Group[pin.internal.port].PINCFG[pin.internal.pin].bit.INEN =
                                            (mode == GPIO_PIN_INPUT);
            PORT_IOBUS->Group[pin.internal.port].PINCFG[pin.internal.pin].bit.PULLEN =
                                            (mode == GPIO_PIN_OUTPUT_PULL);
            PORT_IOBUS->Group[pin.internal.port].PINCFG[pin.internal.pin].bit.DRVSTR =
                                            (mode == GPIO_PIN_OUTPUT_STRONG);
            return 0;
        case GPIO_MCP23S17_PIN:
            if ((mode == GPIO_PIN_OUTPUT_TOTEM) ||
                (mode == GPIO_PIN_OUTPUT_STRONG)) {
                // Output
                mcp23s17_set_pin_mode(gpio_mcp23s17_g, pin.mcp23s17,
                                      MCP23S17_MODE_OUTPUT);
                return 0;
            } else if (mode == GPIO_PIN_INPUT) {
                // Input
                mcp23s17_set_pin_mode(gpio_mcp23s17_g, pin.mcp23s17,
                                      MCP23S17_MODE_INPUT);
                return 0;
            } else {
                // Not supported
                return 1;
            }
        case GPIO_RN2483_PIN:
            rn2483 = gpio_get_rn2483_inst(pin.rn2483.radio);
            if (rn2483 == NULL) {
                // Not a valid radio number
                return 1;
            } else if ((mode == GPIO_PIN_OUTPUT_TOTEM) ||
                (mode == GPIO_PIN_OUTPUT_STRONG)) {
                // Output
                return rn2483_set_pin_mode(rn2483, pin.rn2483.pin,
                                           RN2483_PIN_MODE_OUTPUT);
            } else if (mode == GPIO_PIN_INPUT) {
                // Input
                return rn2483_set_pin_mode(rn2483, pin.rn2483.pin,
                                           RN2483_PIN_MODE_INPUT);
            } else {
                // Not supported
                return 1;
            }
        case GPIO_RFM69HCW_PIN:
            // Not yet supported
            return 1;
        default:
            return 1;
    }
}

enum gpio_pin_mode gpio_get_pin_mode(union gpio_pin_t pin)
{
    struct rn2483_desc_t *rn2483;
    switch (pin.type) {
        case GPIO_INTERNAL_PIN:
            if (PORT_IOBUS->Group[pin.internal.port].PINCFG[pin.internal.pin].bit.INEN) {
                // Input
                return GPIO_PIN_INPUT;
            } else if (PORT_IOBUS->Group[pin.internal.port].PINCFG[pin.internal.pin].bit.PULLEN) {
                // Weak output
                return GPIO_PIN_OUTPUT_PULL;
            } else if (PORT_IOBUS->Group[pin.internal.port].DIR.reg & (1<<pin.internal.pin)) {
                // Output
                if (PORT_IOBUS->Group[pin.internal.port].PINCFG[pin.internal.pin].bit.DRVSTR) {
                    // Extra drive strength
                    return GPIO_PIN_OUTPUT_STRONG;
                } else {
                    // Normal drive strength
                    return GPIO_PIN_OUTPUT_TOTEM;
                }
            } else {
                // Disabled
                return GPIO_PIN_DISABLED;
            }
        case GPIO_MCP23S17_PIN:
            if (mcp23s17_get_pin_mode(gpio_mcp23s17_g, pin.mcp23s17) ==
                    MCP23S17_MODE_INPUT) {
                return GPIO_PIN_INPUT;
            } else {
                return GPIO_PIN_OUTPUT_TOTEM;
            }
        case GPIO_RN2483_PIN:
            rn2483 = gpio_get_rn2483_inst(pin.rn2483.radio);
            if (rn2483 == NULL) {
                // Not a valid radio number
                return GPIO_PIN_DISABLED;
            } else if (rn2483_get_pin_mode(rn2483, pin.rn2483.pin) ==
                            RN2483_PIN_MODE_INPUT) {
                return GPIO_PIN_INPUT;
            } else if (rn2483_get_pin_mode(rn2483, pin.rn2483.pin) ==
                            RN2483_PIN_MODE_OUTPUT) {
                return GPIO_PIN_OUTPUT_TOTEM;
            } else {
                // Pin could be configured as an analog input, not handled by
                // GPIO module
                return GPIO_PIN_DISABLED;
            }
        case GPIO_RFM69HCW_PIN:
            // Not yet supported
            return GPIO_PIN_DISABLED;
        default:
            return GPIO_PIN_DISABLED;
    }
}

uint8_t gpio_set_pull(union gpio_pin_t pin, enum gpio_pull_mode pull)
{
    switch (pin.type) {
        case GPIO_INTERNAL_PIN:
            if (pull == GPIO_PULL_HIGH) {
                // Set pull direction to up
                PORT_IOBUS->Group[pin.internal.port].OUTSET.reg = (1 << pin.internal.pin);
            } else if (pull == GPIO_PULL_LOW) {
                // Set pull direction to down
                PORT_IOBUS->Group[pin.internal.port].OUTCLR.reg = (1 << pin.internal.pin);
            }

            // Enable or disable pull resistors
            PORT_IOBUS->Group[pin.internal.port].PINCFG[pin.internal.pin].bit.PULLEN =
                                                    (pull != GPIO_PULL_NONE);

            return 0;
        case GPIO_MCP23S17_PIN:
            if (pull == GPIO_PULL_NONE) {
                mcp23s17_set_pull_up(gpio_mcp23s17_g, pin.mcp23s17,
                                     MCP23S17_PULL_UP_DISABLED);
            } else if (pull == GPIO_PULL_HIGH) {
                mcp23s17_set_pull_up(gpio_mcp23s17_g, pin.mcp23s17,
                                     MCP23S17_PULL_UP_ENABLED);
            } else {
                // Not supported
                return 1;
            }
            return 0;
        case GPIO_RN2483_PIN:
            // Module does not support pull resistors
            return 1;
        case GPIO_RFM69HCW_PIN:
            // Not yet supported
            return 1;
        default:
            return 1;
    }
}

uint8_t gpio_get_input(union gpio_pin_t pin)
{
    struct rn2483_desc_t *rn2483;
    switch (pin.type) {
        case GPIO_INTERNAL_PIN:
            // Use PORT instead of PORT_IOBUS because reads from PORT_IOBUS
            // don't seem to trigger on demand sampling
            return !!(PORT->Group[pin.internal.port].IN.reg & (1 << pin.internal.pin));
        case GPIO_MCP23S17_PIN:
            return mcp23s17_get_input(gpio_mcp23s17_g, pin.mcp23s17);
        case GPIO_RN2483_PIN:
            rn2483 = gpio_get_rn2483_inst(pin.rn2483.radio);
            if (rn2483 == NULL) {
                // Not a valid radio number
                return 0;
            }
            return rn2483_get_input(rn2483, pin.rn2483.pin);
        case GPIO_RFM69HCW_PIN:
            // Not yet supported
            return 0;
        default:
            return 0;
    }
}

uint8_t gpio_set_output(union gpio_pin_t pin, uint8_t value)
{
    struct rn2483_desc_t *rn2483;
    switch (pin.type) {
        case GPIO_INTERNAL_PIN:
            if (!PORT_IOBUS->Group[pin.internal.port].PINCFG[pin.internal.pin].bit.INEN) {
                if (value) {
                    PORT_IOBUS->Group[pin.internal.port].OUTSET.reg = (1<<pin.internal.pin);
                } else {
                    PORT_IOBUS->Group[pin.internal.port].OUTCLR.reg = (1<<pin.internal.pin);
                }
            } else {
                // pin is input
                return 1;
            }
            return 0;
        case GPIO_MCP23S17_PIN:
            mcp23s17_set_output(gpio_mcp23s17_g, pin.mcp23s17, value);
            return 0;
        case GPIO_RN2483_PIN:
            rn2483 = gpio_get_rn2483_inst(pin.rn2483.radio);
            if (rn2483 == NULL) {
                // Not a valid radio number
                return 1;
            }
            rn2483_set_output(rn2483, pin.rn2483.pin, value);
            return 0;
        case GPIO_RFM69HCW_PIN:
            // Not yet supported
            return 1;
        default:
            return 1;
    }
}

uint8_t gpio_toggle_output(union gpio_pin_t pin)
{
    struct rn2483_desc_t *rn2483;
    switch (pin.type) {
        case GPIO_INTERNAL_PIN:
            if (!PORT_IOBUS->Group[pin.internal.port].PINCFG[pin.internal.pin].bit.INEN) {
                PORT_IOBUS->Group[pin.internal.port].OUTTGL.reg = (1 << pin.internal.pin);
            } else {
                // pin is input
                return 1;
            }
            return 0;
        case GPIO_MCP23S17_PIN:
            mcp23s17_toggle_output(gpio_mcp23s17_g, pin.mcp23s17);
            return 0;
        case GPIO_RN2483_PIN:
            rn2483 = gpio_get_rn2483_inst(pin.rn2483.radio);
            if (rn2483 == NULL) {
                // Not a valid radio number
                return 1;
            }
            rn2483_toggle_output(rn2483, pin.rn2483.pin);
            return 0;
        case GPIO_RFM69HCW_PIN:
            // Not yet supported
            return 1;
        default:
            return 1;
    }
}

/**
 *  Determine which pin was the source of an internal interrupt.
 *
 *  @param interrupt The interrupt number for which the pin should be found
 *  @param pin Address of pin structure which will be populated
 *
 *  @return 0 if successful, 1 if the pin could not be found
 */
static uint8_t get_pin_for_interrupt (int8_t interrupt, union gpio_pin_t *pin)
{
    for (uint8_t i = 0; i < NUM_GPIO_PIN_INTERRUPTS; i++) {
        if (gpio_pin_interrupts[i] == interrupt) {
            // Since interrupts are available on multiple pins, we have to check
            // if the interrupt is actually enabled on this pin
            pin->internal.raw = i;

            if (!PORT_IOBUS->Group[pin->internal.port].PINCFG[pin->internal.pin].bit.PMUXEN) {
                // pinmux is not enabled for this pin, it could not have been
                // the source of the interrupt
                continue;
            }

            if (pin->internal.pin & 1) {
                // Odd numbered pin
                if (!PORT_IOBUS->Group[pin->internal.port].PMUX[pin->internal.pin >> 1].bit.PMUXO) {
                    pin->type = GPIO_INTERNAL_PIN;
                    return 0;
                }
            } else {
                // Even numbered pin
                if (!PORT_IOBUS->Group[pin->internal.port].PMUX[pin->internal.pin >> 1].bit.PMUXE) {
                    pin->type = GPIO_INTERNAL_PIN;
                    return 0;
                }
            }
        }
    }
    return 1;
}

uint8_t gpio_enable_interrupt(union gpio_pin_t pin,
                              enum gpio_interrupt_trigger trigger,
                              uint8_t filter, gpio_interrupt_cb callback,
                              void *context)
{
    switch (pin.type) {
        case GPIO_INTERNAL_PIN:
            ;
            int8_t int_num = gpio_pin_interrupts[pin.internal.raw];

            if (int_num < 0) {
                // This pin cannot be used with the EIC (or doesn't exist)
                return 1;
            }

            if (gpio_get_pin_mode(pin) != GPIO_PIN_INPUT) {
                // This pin is not configured as an input
                return 1;
            }

            union gpio_pin_t pin_temp;
            if (!get_pin_for_interrupt(int_num, &pin_temp)) {
                // The EIC line for this pin is already in use (by another pin
                // or this one)
                return 1;
            }

            // Set callback function
            gpio_int_callbacks[int_num].callback = callback;
            gpio_int_callbacks[int_num].context = context;

            // Set PMUX to interrupt (function A)
            if (pin.internal.pin & 1) {
                PORT_IOBUS->Group[pin.internal.port].PMUX[pin.internal.pin >> 1].bit.PMUXO = 0;
            } else {
                PORT_IOBUS->Group[pin.internal.port].PMUX[pin.internal.pin >> 1].bit.PMUXE = 0;
            }
            // Enable PMUX
            PORT_IOBUS->Group[pin.internal.port].PINCFG[pin.internal.pin].bit.PMUXEN = 1;

#if defined(SAMx5x)
            // Disable EIC since CONFIGn registers are enable protected
            EIC->CTRLA.bit.ENABLE = 0;
            while(EIC->SYNCBUSY.bit.ENABLE);
#endif

            // Set sense for interrupt
            switch (trigger) {
                case GPIO_INTERRUPT_RISING_EDGE:
                    EIC->CONFIG[int_num >> 3].reg |= (EIC_CONFIG_SENSE0_RISE <<
                                                      (4 * (int_num & 0x7)));
                    break;
                case GPIO_INTERRUPT_FALLING_EDGE:
                    EIC->CONFIG[int_num >> 3].reg |= (EIC_CONFIG_SENSE0_FALL <<
                                                      (4 * (int_num & 0x7)));
                    break;
                case GPIO_INTERRUPT_BOTH_EDGES:
                    EIC->CONFIG[int_num >> 3].reg |= (EIC_CONFIG_SENSE0_BOTH <<
                                                      (4 * (int_num & 0x7)));
                    break;
                case GPIO_INTERRUPT_HIGH:
                    EIC->CONFIG[int_num >> 3].reg |= (EIC_CONFIG_SENSE0_HIGH <<
                                                      (4 * (int_num & 0x7)));
                    break;
                case GPIO_INTERRUPT_LOW:
                    EIC->CONFIG[int_num >> 3].reg |= (EIC_CONFIG_SENSE0_LOW <<
                                                      (4 * (int_num & 0x7)));
                    break;
            }

            // Enable filter if requested
            if (filter) {
                EIC->CONFIG[int_num >> 3].reg |= (EIC_CONFIG_FILTEN0 <<
                                                  (4 * (int_num & 0x7)));
            }

            // Enable waking from interrupt
#if defined(SAMD2x)
            EIC->WAKEUP.reg |= (1 << int_num);
#endif
            // Enable interrupt
            EIC->INTENSET.reg = (1 << int_num);

#if defined(SAMx5x)
            // Re-enable EIC
            EIC->CTRLA.bit.ENABLE = 1;
#endif

            return 0;
        case GPIO_MCP23S17_PIN:
            for (uint8_t i = 0; i < GPIO_MAX_EXTERNAL_IO_INTERRUPTS; i++) {
                if (gpio_ext_io_ints[i].callback != 0) {
                    continue;
                } else {
                    gpio_ext_io_ints[i].callback = callback;
                    gpio_ext_io_ints[i].context = context;
                    gpio_ext_io_ints[i].pin = pin;

                    if (trigger == GPIO_INTERRUPT_FALLING_EDGE) {
                        mcp23s17_enable_interrupt(gpio_mcp23s17_g, pin.mcp23s17,
                                                  MCP23S17_INT_LOW);
                    } else if (trigger == GPIO_INTERRUPT_RISING_EDGE) {
                        mcp23s17_enable_interrupt(gpio_mcp23s17_g, pin.mcp23s17,
                                                  MCP23S17_INT_HIGH);
                    } else if (trigger == GPIO_INTERRUPT_BOTH_EDGES) {
                        mcp23s17_enable_interrupt(gpio_mcp23s17_g, pin.mcp23s17,
                                                  MCP23S17_INT_EDGE);
                    } else {
                        break;
                    }

                    return 0;
                }
            }
            return 1;
        case GPIO_RN2483_PIN:
            // Module does not support interrupts
            return 1;
        case GPIO_RFM69HCW_PIN:
            // Not yet supported
            return 1;
        default:
            return 1;
    }
}

uint8_t gpio_disable_interrupt(union gpio_pin_t pin)
{
    switch (pin.type) {
        case GPIO_INTERNAL_PIN:
            // Ensure that this pin is not configured to control the EIC line
            if ((pin.internal.pin & 1) &&
                (!PORT_IOBUS->Group[pin.internal.port].PMUX[pin.internal.pin >> 1].bit.PMUXO)) {
                PORT_IOBUS->Group[pin.internal.port].PINCFG[pin.internal.pin].bit.PMUXEN = 0;
            } else if (!(pin.internal.pin & 1) &&
                       (!PORT_IOBUS->Group[pin.internal.port].PMUX[pin.internal.pin >> 1].bit.PMUXE)) {
                PORT_IOBUS->Group[pin.internal.port].PINCFG[pin.internal.pin].bit.PMUXEN = 0;
            }

            int8_t int_num = gpio_pin_interrupts[pin.internal.raw];

            union gpio_pin_t enabled_pin;
            if (get_pin_for_interrupt(int_num, &enabled_pin) ||
                (enabled_pin.raw != pin.raw)) {
                // The EIC line for this pin is not being used or this pin is
                // not the pin which is currently enabled for this interrupt
                return 0;
            }

            // Disable the interrupt in the EIC
            EIC->INTENCLR.reg = (1 << int_num);
            // Ensure that the interrupt will not wake the CPU
#if defined(SAMD2x)
            EIC->WAKEUP.reg &= ~(1 << int_num);
#endif

            // Remove handler function
            gpio_int_callbacks[int_num].callback = NULL;

            return 0;
        case GPIO_MCP23S17_PIN:
            for (uint8_t i = 0; i < GPIO_MAX_EXTERNAL_IO_INTERRUPTS; i++) {
                if (gpio_ext_io_ints[i].pin.raw == pin.raw) {
                    gpio_ext_io_ints[i].callback = NULL;
                    mcp23s17_disable_interrupt(gpio_mcp23s17_g, pin.mcp23s17);
                    return 0;
                }
            }
            // The interrupt wasn't enabled
            return 0;
        case GPIO_RN2483_PIN:
            // Module does not support interrupts
            return 1;
        case GPIO_RFM69HCW_PIN:
            // Not yet supported
            return 1;
        default:
            return 1;
    }
}



static void gpio_mcp23s17_int_cb (void *context, union gpio_pin_t pin,
                                  uint8_t value)
{
    mcp23s17_handle_interrupt(gpio_mcp23s17_g);
}

static void gpio_mcp23s17_interrupt_occurred (struct mcp23s17_desc_t *inst,
                                              union mcp23s17_pin_t pin,
                                              uint8_t value)
{
    for (uint8_t i = 0; i < GPIO_MAX_EXTERNAL_IO_INTERRUPTS; i++) {
        if ((gpio_ext_io_ints[i].pin.type == GPIO_MCP23S17_PIN) &&
                (gpio_ext_io_ints[i].pin.mcp23s17.value == pin.value)) {
            if (gpio_ext_io_ints[i].callback != NULL) {
                gpio_ext_io_ints[i].callback(gpio_ext_io_ints[i].context,
                                             (union gpio_pin_t)
                                {.type = GPIO_MCP23S17_PIN, .mcp23s17 = pin},
                                             value);
            }
        }
    }
}

static struct rn2483_desc_t *gpio_get_rn2483_inst (uint8_t radio_num)
{
    for (struct radio_instance_desc *const *radio = gpio_radios_g;
            *radio != NULL;  radio++) {
        if ((*radio)->radio_num == radio_num) {
            return &(*radio)->rn2483;
        }
    }
    return NULL;
}

#if defined(SAMD2x)
void EIC_Handler (void)
{
    for (uint8_t i = 0; i < EIC_EXTINT_NUM; i++) {
        if (EIC->INTFLAG.reg & (1 << i)) {
            // Interrupt i has occurred
            if (gpio_int_callbacks[i].callback != NULL) {
                union gpio_pin_t pin;
                if (!get_pin_for_interrupt(i, &pin)) {
                    gpio_int_callbacks[i].callback(gpio_int_callbacks[i].context,
                                                   pin, gpio_get_input(pin));
                }
            }
            // Clear interrupt flag
            EIC->INTFLAG.reg = (1 << i);
        }
    }
}
#elif defined(SAMx5x)
#define EIC_HANDLER(name, num) void name (void) {\
    if (gpio_int_callbacks[num].callback != NULL) { \
        union gpio_pin_t pin;\
        if (!get_pin_for_interrupt(num, &pin)) {\
            gpio_int_callbacks[num].callback(gpio_int_callbacks[num].context,\
                                             pin, gpio_get_input(pin));\
        }\
    } \
    EIC->INTFLAG.reg = (1 << num);\
}

EIC_HANDLER(EIC_0_Handler, 0)

EIC_HANDLER(EIC_1_Handler, 1)

EIC_HANDLER(EIC_2_Handler, 2)

EIC_HANDLER(EIC_3_Handler, 3)

EIC_HANDLER(EIC_4_Handler, 4)

EIC_HANDLER(EIC_5_Handler, 5)

EIC_HANDLER(EIC_6_Handler, 6)

EIC_HANDLER(EIC_7_Handler, 7)

EIC_HANDLER(EIC_8_Handler, 8)

EIC_HANDLER(EIC_9_Handler, 9)

EIC_HANDLER(EIC_10_Handler, 10)

EIC_HANDLER(EIC_11_Handler, 11)

EIC_HANDLER(EIC_12_Handler, 12)

EIC_HANDLER(EIC_13_Handler, 13)

EIC_HANDLER(EIC_14_Handler, 14)

EIC_HANDLER(EIC_15_Handler, 15)

#endif
