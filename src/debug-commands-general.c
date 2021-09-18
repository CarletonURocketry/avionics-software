/**
 * @file debug-commands-general.c
 * @desc Commands for debugging CLI
 * @author Samuel Dewan
 * @date 2020-08-11
 * Last Author:
 * Last Edited On:
 */

#include "debug-commands-general.h"

#include "debug-commands.h"

#include <ctype.h>

#include "variant.h"
#include "board.h"
#include "target.h"
#include "gpio.h"
#include "wdt.h"
#include "sercom-i2c.h"
#include "sercom-spi.h"

// MARK: Version

void debug_version (uint8_t argc, char **argv, struct console_desc_t *console)
{
    console_send_str(console, VERSION_STRING);
    console_send_str(console, BUILD_STRING);
    console_send_str(console, "Target: "TARGET_STRING"\n");
    console_send_str(console, "Board: "BOARD_STRING"\n");
    console_send_str(console, "Variant: "VARIANT_STRING"\n");
}

// MARK: DID

void debug_did (uint8_t argc, char **argv, struct console_desc_t *console)
{
    char str[11];

    str[0] = '0';
    str[1] = 'x';

    console_send_str(console, "Serial Number: ");
#if defined(SAMD2x)
    utoa(*((uint32_t*)0x0080A00C), str + 2, 16);
#elif defined(SAMx5x)
    utoa(*((uint32_t*)0x008061FC), str + 2, 16);
#endif
    console_send_str(console, str);
#if defined(SAMD2x)
    utoa(*((uint32_t*)0x0080A040), str + 2, 16);
#elif defined(SAMx5x)
    utoa(*((uint32_t*)0x00806010), str + 2, 16);
#endif
    console_send_str(console, str+2);
#if defined(SAMD2x)
    utoa(*((uint32_t*)0x0080A044), str + 2, 16);
#elif defined(SAMx5x)
    utoa(*((uint32_t*)0x00806014), str + 2, 16);
#endif
    console_send_str(console, str+2);
#if defined(SAMD2x)
    utoa(*((uint32_t*)0x0080A048), str + 2, 16);
#elif defined(SAMx5x)
    utoa(*((uint32_t*)0x00806018), str + 2, 16);
#endif
    console_send_str(console, str+2);

    console_send_str(console, "\n\nDevice Identification: ");
    utoa(DSU->DID.reg, str + 2, 16);
    console_send_str(console, str);

    console_send_str(console, "\n\tPROCESSOR: ");
    utoa(DSU->DID.bit.PROCESSOR, str + 2, 16);
    console_send_str(console, str);

    console_send_str(console, "\n\tFAMILY: ");
    utoa(DSU->DID.bit.FAMILY, str + 2, 16);
    console_send_str(console, str);

    console_send_str(console, "\n\tSERIES: ");
    utoa(DSU->DID.bit.SERIES, str + 2, 16);
    console_send_str(console, str);

    console_send_str(console, "\n\tDIE: ");
    utoa(DSU->DID.bit.DIE, str + 2, 16);
    console_send_str(console, str);

    console_send_str(console, "\n\tREVISION: ");
    utoa(DSU->DID.bit.REVISION, str + 2, 16);
    console_send_str(console, str);
    str[2] = ' ';
    str[3] = '(';
    str[4] = (char)(DSU->DID.bit.REVISION + 'A');
    str[5] = ')';
    str[6] = 0;
    console_send_str(console, str + 2);

    console_send_str(console, "\n\tDEVSEL: ");
    utoa(DSU->DID.bit.DEVSEL, str + 2, 16);
    console_send_str(console, str);

    console_send_str(console, "\nCPUID: ");
    utoa(SCB->CPUID, str + 2, 16);
    console_send_str(console, str);

    console_send_str(console, "\n");
}

// MARK: Reset Cause

void debug_rcause (uint8_t argc, char **argv, struct console_desc_t *console)
{
    console_send_str(console, "Last reset due to: ");
#if defined(SAMD2x)
    switch (PM->RCAUSE.reg) {
#elif defined(SAMx5x)
    switch (RSTC->RCAUSE.reg) {
#endif
        case (1 << 0):
            console_send_str(console, "power on reset\n");
            break;
        case (1 << 1):
            console_send_str(console, "BOD12\n");
            break;
        case (1 << 2):
            console_send_str(console, "BOD33\n");
            break;
#if defined(SAMx5x)
        case (1 << 3):
            console_send_str(console, "NVM\n");
            break;
#endif
        case (1 << 4):
            console_send_str(console, "external reset\n");
            break;
        case (1 << 5):
            console_send_str(console, "watchdog\n");
            break;
        case (1 << 6):
            console_send_str(console, "system reset request\n");
            break;
#if defined(SAMx5x)
        case (1 << 7):
            console_send_str(console, "backup\n");
            break;
#endif
        default:
            console_send_str(console, "unknown\n");
            break;
    }
}

// MARK: I2C Scan

void debug_i2c_scan (uint8_t argc, char **argv, struct console_desc_t *console)
{
    // Start a scan of the I2C bus
    uint8_t i2c_t_id;
    sercom_i2c_start_scan(&i2c0_g, &i2c_t_id);

    // Wait for scan to complete
    while (!sercom_i2c_transaction_done(&i2c0_g, i2c_t_id)) wdt_pat();

    // Check if scan completed successfully
    switch (sercom_i2c_transaction_state(&i2c0_g, i2c_t_id)) {
        case I2C_STATE_BUS_ERROR:
            console_send_str(console, "Scan failed: Bus Error\n");
            sercom_i2c_clear_transaction(&i2c0_g, i2c_t_id);
            return;
        case I2C_STATE_ARBITRATION_LOST:
            console_send_str(console, "Scan failed: Arbitration Lost\n");
            sercom_i2c_clear_transaction(&i2c0_g, i2c_t_id);
            return;
        case I2C_STATE_SLAVE_NACK:
            console_send_str(console, "Scan failed: Slave NACK\n");
            sercom_i2c_clear_transaction(&i2c0_g, i2c_t_id);
            return;
        default:
            // Success!
            break;
    }

    // Print scan results
    char str[8];

    console_send_str(console, "Available Devices:\n");
    for (int i = 0; i < 128; i++) {
        if (sercom_i2c_device_available(&i2c0_g, i2c_t_id, i)) {
            console_send_str(console, "0b");
            utoa(i, str, 2);
            for (int j = strlen(str); j < 7; j++) {
                console_send_str(console, "0");
            }
            console_send_str(console, str);

            console_send_str(console, " (0x");
            utoa(i, str, 16);
            if (strlen(str) == 1) {
                console_send_str(console, "0");
            }
            console_send_str(console, str);
            console_send_str(console, ")\n");
        }
    }

    // Clear transaction
    sercom_i2c_clear_transaction(&i2c0_g, i2c_t_id);
}

// MARK: IO Expander Regs

void debug_io_exp_regs (uint8_t argc, char **argv,
                        struct console_desc_t *console)
{
#ifndef ENABLE_IO_EXPANDER
    console_send_str(console, "IO expander not enabled.\n");
#else
    uint8_t address = 0;

    if (argc > 2) {
        console_send_str(console, DEBUG_IO_EXP_REGS_HELP);
        return;
    } else if (argc == 2) {
        char* end;

        uint32_t addr = strtoul(argv[1], &end, 0);
        if ((*end != '\0') || (addr > 7)) {
            console_send_str(console, DEBUG_IO_EXP_REGS_HELP);
            return;
        }

        address = (uint8_t)addr;
    }

    uint8_t command[] = {((MCP23S17_ADDR | address) << 1) | 1, 0};
    struct mcp23s17_register_map registers;


    uint8_t t_id;
    uint8_t s = sercom_spi_start(&spi0_g, &t_id, 8000000UL,
                                 IO_EXPANDER_CS_PIN_GROUP,
                                 IO_EXPANDER_CS_PIN_MASK, command, 2,
                                 (uint8_t*)&registers, 10);

    if (s) {
        console_send_str(console, "Failed to queue SPI transaction.\n");
        return;
    }

    while (!sercom_spi_transaction_done(&spi0_g, t_id)) wdt_pat();
    sercom_spi_clear_transaction(&spi0_g, t_id);

    command[1] = 0x0B;
    s = sercom_spi_start(&spi0_g, &t_id, 8000000UL, IO_EXPANDER_CS_PIN_GROUP,
                         IO_EXPANDER_CS_PIN_MASK, command, 2,
                         (uint8_t*)&registers.IOCON, 11);

    if (s) {
        console_send_str(console, "Failed to queue SPI transaction.\n");
        return;
    }

    debug_print_byte_with_pad(console, "   IODIRA: 0b", registers.IODIR[0].reg, "\n");
    debug_print_byte_with_pad(console, "   IODIRB: 0b", registers.IODIR[1].reg, "\n\n");
    debug_print_byte_with_pad(console, "    IPOLA: 0b", registers.IPOL[0].reg, "\n");
    debug_print_byte_with_pad(console, "    IPOLB: 0b", registers.IPOL[1].reg, "\n\n");
    wdt_pat();
    debug_print_byte_with_pad(console, " GPINTENA: 0b", registers.GPINTEN[0].reg, "\n");
    debug_print_byte_with_pad(console, " GPINTENB: 0b", registers.GPINTEN[1].reg, "\n\n");
    debug_print_byte_with_pad(console, "  DEFVALA: 0b", registers.DEFVAL[0].reg, "\n");
    debug_print_byte_with_pad(console, "  DEFVALB: 0b", registers.DEFVAL[1].reg, "\n\n");
    wdt_pat();
    debug_print_byte_with_pad(console, "  INTCONA: 0b", registers.INTCON[0].reg, "\n");
    debug_print_byte_with_pad(console, "  INTCONB: 0b", registers.INTCON[1].reg, "\n\n");

    while (!sercom_spi_transaction_done(&spi0_g, t_id)) wdt_pat();
    sercom_spi_clear_transaction(&spi0_g, t_id);

    debug_print_byte_with_pad(console, "   IOCON: 0b", registers.IOCON.reg, "\n\n");
    debug_print_byte_with_pad(console, "   GPPUA: 0b", registers.GPPU[0].reg, "\n");
    debug_print_byte_with_pad(console, "   GPPUB: 0b", registers.GPPU[1].reg, "\n\n");
    wdt_pat();
    debug_print_byte_with_pad(console, "   INTFA: 0b", registers.INTF[0].reg, "\n");
    debug_print_byte_with_pad(console, "   INTFB: 0b", registers.INTF[1].reg, "\n\n");
    debug_print_byte_with_pad(console, " INTCAPA: 0b", registers.INTCAP[0].reg, "\n");
    wdt_pat();
    debug_print_byte_with_pad(console, " INTCAPB: 0b", registers.INTCAP[1].reg, "\n\n");
    debug_print_byte_with_pad(console, "   GPIOA: 0b", registers.GPIO[0].reg, "\n");
    debug_print_byte_with_pad(console, "   GPIOB: 0b", registers.GPIO[1].reg, "\n\n");
    wdt_pat();
    debug_print_byte_with_pad(console, "   OLATA: 0b", registers.OLAT[0].reg, "\n");
    debug_print_byte_with_pad(console, "   OLATB: 0b", registers.OLAT[1].reg, "\n");
#endif
}

// MARK: GPIO

#define NUM_HEADER_PINS 24
static const union gpio_pin_t HEADER_PINS[] = {
    GPIO_0,  GPIO_1,  GPIO_2,
    GPIO_3,  GPIO_4,  GPIO_5,
    GPIO_6,  GPIO_7,  GPIO_8,
    GPIO_9,  GPIO_10, GPIO_11,
    GPIO_12, GPIO_13, GPIO_14,
    GPIO_15, GPIO_16, GPIO_17,
    GPIO_18, GPIO_19, GPIO_20,
    GPIO_21, GPIO_22, GPIO_23
};

void debug_gpio (uint8_t argc, char **argv, struct console_desc_t *console)
{
    if ((argc < 3) || (argc > 4)) {
        // Not enough arguments
        console_send_str(console, DEBUG_GPIO_HELP);
        console_send_str(console, "\n");
        return;
    }

    char str[10];

    // Create pin with invalid value
    union gpio_pin_t pin = { .raw = 0xFFFF };

    /* Parse pin */
    char* end;
    if (argv[2][0] == 'g' || isdigit((unsigned char)argv[2][0])) {
        // Try and parse pin as a header pin number
        uint32_t pin_num = strtoul(argv[2] + (argv[2][0] == 'g'), &end, 10);
        if ((*end == '\0') && (pin_num < NUM_HEADER_PINS)) {
            pin = HEADER_PINS[pin_num];
        }
    } else if ((argv[2][0] == 'p') && ((argv[2][1] == 'a') || (argv[2][1] == 'b'))) {
        // Try and parse pin as an internal pin number
        uint32_t pin_num = strtoul(argv[2] + 2, &end, 10);
        if ((*end == '\0') && (pin_num < 32)) {
            pin.type = GPIO_INTERNAL_PIN;
            pin.internal.port = (argv[2][1] == 'b');
            pin.internal.pin = pin_num;
        }
    } else if ((argv[2][0] == 'e') && ((argv[2][1] == 'a') || (argv[2][1] == 'b'))) {
#ifdef ENABLE_IO_EXPANDER
        // Try and parse pin as an IO expander pin number
        uint32_t pin_num = strtoul(argv[2] + 2, &end, 10);
        if ((*end == '\0') && (pin_num < 8)) {
            pin.type = GPIO_MCP23S17_PIN;
            pin.mcp23s17.port = (argv[2][1] == 'a') ? MCP23S17_PORT_A : MCP23S17_PORT_B;
            pin.mcp23s17.pin = pin_num;
        }
#else
        console_send_str(console, "IO expander not enabled.\n");
        return;
#endif
    } else if (argv[2][0] == 'r') {
        // Try and parse pin as a RN2483 pin number
        uint32_t radio_num = strtoul(argv[2] + 1, &end, 10);
        if ((*end == '.') && (radio_num < 8)) {
            uint32_t pin_num = strtoul(end + 1, &end, 10);
            if ((*end == '\0') && (pin_num < RN2483_NUM_PINS)) {
                pin.type = GPIO_RN2483_PIN;
                pin.rn2483.radio = radio_num;
                pin.rn2483.pin = pin_num;
            }
        }
    }
#ifdef DEBUG0_LED_PIN
    else if (!strcmp(argv[2], "DEBUG0")) {
        pin = DEBUG0_LED_PIN;
    }
#endif
#ifdef DEBUG1_LED_PIN
    else if (!strcmp(argv[2], "DEBUG1")) {
        pin = DEBUG1_LED_PIN;
    }
#endif
#ifdef STAT_R_LED_PIN
    else if (!strcmp(argv[2], "STAT_R")) {
        pin = STAT_R_LED_PIN;
    }
#endif
#ifdef STAT_G_LED_PIN
    else if (!strcmp(argv[2], "STAT_G")) {
        pin = STAT_G_LED_PIN;
    }
#endif
#ifdef STAT_B_LED_PIN
    else if (!strcmp(argv[2], "STAT_B")) {
        pin = STAT_B_LED_PIN;
    }
#endif
#ifdef SD_ACTIVE_LED_PIN
    else if (!strcmp(argv[2], "SD_ACTIVE")) {
        pin = SD_ACTIVE_LED_PIN;
    }
#endif

    if (pin.raw == 0xFFFF) {
        // Pin was not valid
        console_send_str(console, "\"");
        console_send_str(console, argv[2]);
        console_send_str(console, "\" is not a valid pin.\n");
        return;
    }

    // Run command
    if (!strcmp(argv[1], "mode")) {
        if (argc < 4) {
            // Not enough arguments
            console_send_str(console, "Mode command requires an argument.\n");
            return;
        }

        enum gpio_pin_mode mode = GPIO_PIN_DISABLED;

        if (!strcmp(argv[3], "input") || !strcmp(argv[3], "in")) {
            mode = GPIO_PIN_INPUT;
        } else if (!strcmp(argv[3], "output") || !strcmp(argv[3], "out")) {
            mode = GPIO_PIN_OUTPUT_TOTEM;
        } else if (!strcmp(argv[3], "strong")) {
            mode = GPIO_PIN_OUTPUT_STRONG;
        } else if (!strcmp(argv[3], "pull")) {
            mode = GPIO_PIN_OUTPUT_PULL;
        } else {
            console_send_str(console, "\"");
            console_send_str(console, argv[3]);
            console_send_str(console, "\" is not a valid pin mode.\n");
            return;
        }

        if (gpio_set_pin_mode(pin, mode)) {
            console_send_str(console, "Could not set pin mode.\n");
        }
    } else if (!strcmp(argv[1], "pull")) {
        if (argc < 4) {
            // Not enough arguments
            console_send_str(console, "Pull command requires an argument.\n");
            return;
        }

        enum gpio_pull_mode pull;

        if (!strcmp(argv[3], "high")) {
            pull = GPIO_PULL_HIGH;
        } else if (!strcmp(argv[3], "low")) {
            pull = GPIO_PULL_LOW;
        } else if (!strcmp(argv[3], "none")) {
            pull = GPIO_PULL_NONE;
        } else {
            console_send_str(console, "\"");
            console_send_str(console, argv[3]);
            console_send_str(console, "\" is not a valid pull type.\n");
            return;
        }

        if (gpio_set_pull(pin, pull)) {
            console_send_str(console, "Could not set pull.\n");
        }
    } else if (!strcmp(argv[1], "out")) {
        if (argc < 4) {
            // Not enough arguments
            console_send_str(console, "Out command requires an argument.\n");
            return;
        }

        uint8_t value = 0;

        if (!strcmp(argv[3], "high") || !strcmp(argv[3], "1")) {
            value = 1;
        } else if (!strcmp(argv[3], "low") || !strcmp(argv[3], "0")) {
            value = 0;
        } else {
            console_send_str(console, "\"");
            console_send_str(console, argv[3]);
            console_send_str(console, "\" is not a valid pin value.\n");
            return;
        }

        if (gpio_set_output(pin, value)) {
            console_send_str(console, "Could not set output.\n");
        }
    } else if (!strcmp(argv[1], "status")) {
        if (argc > 3) {
            // Too many arguments
            console_send_str(console, "Too many arguments for status command.\n");
            return;
        }

        uint8_t out_val = 0;

        switch (pin.type) {
            case GPIO_INTERNAL_PIN:
                console_send_str(console, "Internal pin: Port ");
                console_send_str(console, pin.internal.port ? "B" : "A");
                console_send_str(console, " Pin ");
                utoa(pin.internal.pin, str, 10);
                console_send_str(console, str);

                out_val = !!(PORT->Group[pin.internal.port].OUT.reg & (1 << pin.internal.pin));
                break;
#ifdef ENABLE_IO_EXPANDER
            case GPIO_MCP23S17_PIN:
                console_send_str(console, "IO expander pin: Port ");
                console_send_str(console, pin.mcp23s17.port ? "B" : "A");
                console_send_str(console, " Pin ");
                utoa(pin.mcp23s17.pin, str, 10);
                console_send_str(console, str);

                out_val = !!(io_expander_g.registers.OLAT[pin.mcp23s17.port].reg & (1 << pin.mcp23s17.pin));
                break;
#endif
            case GPIO_RN2483_PIN:
                console_send_str(console, "RN2483 pin: Radio ");
                utoa(pin.rn2483.radio, str, 10);
                console_send_str(console, str);
                console_send_str(console, " Pin ");
                utoa(pin.rn2483.pin, str, 10);
                console_send_str(console, str);

                // TODO:
                //out_val = rn2483_g.pins[pin.rn2483.pin].value;
                break;
            case GPIO_RFM69HCW_PIN:
                // Not yet supported
                break;
            default:
                break;
        }

        console_send_str(console, "\n  Mode: ");

        switch (gpio_get_pin_mode(pin)) {
            case GPIO_PIN_DISABLED:
                console_send_str(console, "disabled\n");
                break;
            case GPIO_PIN_OUTPUT_PULL:
                console_send_str(console, "output - pull\n");
                break;
            case GPIO_PIN_OUTPUT_TOTEM:
                console_send_str(console, "output - totem\n");
                break;
            case GPIO_PIN_OUTPUT_STRONG:
                console_send_str(console, "output - strong\n");
                break;
            case GPIO_PIN_INPUT:
                console_send_str(console, "input\n");
                break;
        }

        if (gpio_get_pin_mode(pin) != GPIO_PIN_DISABLED) {
            console_send_str(console, "  Value: ");
            if (gpio_get_pin_mode(pin) == GPIO_PIN_INPUT) {
                utoa(gpio_get_input(pin), str, 10);
            } else {
                utoa(out_val, str, 10);
            }
            console_send_str(console, str);
            console_send_str(console, "\n");
        }
    } else if (!strcmp(argv[1], "in")) {
        if (argc > 3) {
            // Too many arguments
            console_send_str(console, "Too many arguments for in command.\n");
            return;
        }

        console_send_str(console, "Value: ");
        utoa(gpio_get_input(pin), str, 10);
        console_send_str(console, str);
        console_send_str(console, "\n");
    } else if (!strcmp(argv[1], "poll")) {
        if (argc > 3) {
            // Too many arguments
            console_send_str(console, "Too many arguments for poll command.\n");
            return;
        }

        switch (pin.type) {
            case GPIO_INTERNAL_PIN:
                // Does not need to be polled
                break;
#ifdef ENABLE_IO_EXPANDER
            case GPIO_MCP23S17_PIN:
                mcp23s17_poll(&io_expander_g);
                while (mcp23s17_poll_in_progress(&io_expander_g)) {
                    mcp23s17_service(&io_expander_g);
                }
                break;
#endif
            case GPIO_RN2483_PIN:
                // TODO:
                //                rn2483_poll_gpio_pin(&rn2483_g, pin.rn2483.pin);
                //                while (rn2483_poll_gpio_pin_in_progress(&rn2483_g, pin.rn2483.pin)) {
                //                    rn2483_service(&rn2483_g);
                //                }
                break;
            case GPIO_RFM69HCW_PIN:
                // Not yet supported
                break;
            default:
                break;
        }

        console_send_str(console, "Value: ");
        utoa(gpio_get_input(pin), str, 10);
        console_send_str(console, str);
        console_send_str(console, "\n");
    } else {
        // Not a valid command
        console_send_str(console, "\"");
        console_send_str(console, argv[1]);
        console_send_str(console, "\" is not a valid command.\n");
    }
}
