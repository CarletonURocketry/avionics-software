/**
 * @file debug-commands-analog.c
 * @desc Commands for debugging CLI
 * @author Samuel Dewan
 * @date 2020-08-11
 * Last Author:
 * Last Edited On:
 */

#include "debug-commands-analog.h"

#include "debug-commands.h"

#include <string.h>
#include <ctype.h>
#include "config.h"
#include "wdt.h"
#include "adc.h"
#include "dac.h"

// MARK: Temp

void debug_temp (uint8_t argc, char **argv, struct console_desc_t *console)
{
    // Read temperature log values from
    uint8_t room_temp_val_int = (uint8_t)(((*((uint32_t*)
                                              NVMCTRL_FUSES_ROOM_TEMP_VAL_INT_ADDR)) &
                                           NVMCTRL_FUSES_ROOM_TEMP_VAL_INT_Msk) >>
                                          NVMCTRL_FUSES_ROOM_TEMP_VAL_INT_Pos);
    uint8_t room_temp_val_dec = (uint8_t)(((*((uint32_t*)
                                              NVMCTRL_FUSES_ROOM_TEMP_VAL_DEC_ADDR)) &
                                           NVMCTRL_FUSES_ROOM_TEMP_VAL_DEC_Msk) >>
                                          NVMCTRL_FUSES_ROOM_TEMP_VAL_DEC_Pos);
    uint8_t hot_temp_val_int = (uint8_t)(((*((uint32_t*)
                                             NVMCTRL_FUSES_HOT_TEMP_VAL_INT_ADDR)) &
                                          NVMCTRL_FUSES_HOT_TEMP_VAL_INT_Msk) >>
                                         NVMCTRL_FUSES_HOT_TEMP_VAL_INT_Pos);
    uint8_t hot_temp_val_dec = (uint8_t)(((*((uint32_t*)
                                             NVMCTRL_FUSES_HOT_TEMP_VAL_DEC_ADDR)) &
                                          NVMCTRL_FUSES_HOT_TEMP_VAL_DEC_Msk) >>
                                         NVMCTRL_FUSES_HOT_TEMP_VAL_DEC_Pos);
    int8_t room_int1v_val = (int8_t)(((*((uint32_t*)
                                         NVMCTRL_FUSES_ROOM_INT1V_VAL_ADDR)) &
                                      NVMCTRL_FUSES_ROOM_INT1V_VAL_Msk) >>
                                     NVMCTRL_FUSES_ROOM_INT1V_VAL_Pos);
    int8_t hot_int1v_val = (int8_t)(((*((uint32_t*)
                                        NVMCTRL_FUSES_HOT_INT1V_VAL_ADDR)) &
                                     NVMCTRL_FUSES_HOT_INT1V_VAL_Msk) >>
                                    NVMCTRL_FUSES_HOT_INT1V_VAL_Pos);

    uint16_t room_adc_val = (uint16_t)(((*((uint32_t*)
                                           NVMCTRL_FUSES_ROOM_ADC_VAL_ADDR)) &
                                        NVMCTRL_FUSES_ROOM_ADC_VAL_Msk) >>
                                       NVMCTRL_FUSES_ROOM_ADC_VAL_Pos);
    uint16_t hot_adc_val = (uint16_t)(((*((uint32_t*)
                                          NVMCTRL_FUSES_HOT_ADC_VAL_ADDR)) &
                                       NVMCTRL_FUSES_HOT_ADC_VAL_Msk) >>
                                      NVMCTRL_FUSES_HOT_ADC_VAL_Pos);

    char str[8];

    console_send_str(console, "Temperature (course): ");
    int16_t t_course = adc_get_temp_course();
    debug_print_fixed_point(console, t_course, 2);
    console_send_str(console, " C\nTemperature (fine): ");
    int16_t t_fine = adc_get_temp_fine();
    debug_print_fixed_point(console, t_fine, 2);

    wdt_pat();

    console_send_str(console, " C\n\nTemperature Log Row:");

    console_send_str(console, "\n     ROOM_TEMP_VAL: ");
    utoa(room_temp_val_int, str, 10);
    console_send_str(console, str);
    console_send_str(console, ".");
    utoa(room_temp_val_dec, str, 10);
    console_send_str(console, str);

    console_send_str(console, " C\n      HOT_TEMP_VAL: ");
    utoa(hot_temp_val_int, str, 10);
    console_send_str(console, str);
    console_send_str(console, ".");
    utoa(hot_temp_val_dec, str, 10);
    console_send_str(console, str);

    wdt_pat();

    console_send_str(console, " C\n    ROOM_INT1V_VAL: ");
    console_send_str(console, (room_int1v_val >= 0) ? "1." : "0.");
    utoa((room_int1v_val >= 0) ? ((unsigned int)room_int1v_val) :
         ((unsigned int)(1000 + room_int1v_val)), str, 10);
    console_send_str(console, str);

    console_send_str(console, " V\n     HOT_INT1V_VAL: ");
    console_send_str(console, (hot_int1v_val >= 0) ? "1." : "0.");
    utoa((hot_int1v_val >= 0) ? ((unsigned int)hot_int1v_val) :
         ((unsigned int)(1000 + hot_int1v_val)), str, 10);
    console_send_str(console, str);

    wdt_pat();

    console_send_str(console, " V\n      ROOM_ADC_VAL: 0x");
    utoa(room_adc_val, str, 16);
    console_send_str(console, str);

    console_send_str(console, "\n       HOT_ADC_VAL: 0x");
    utoa(hot_adc_val, str, 16);
    console_send_str(console, str);
    console_send_str(console, "\n");
}


// MARK: Analog

static void debug_analog_print_channel (struct console_desc_t *console,
                                        uint8_t channel, int32_t parsed_value,
                                        uint8_t decimals, const char *name,
                                        const char *unit)
{
    char str[10];

    if (*name == '\0') {
        utoa(channel, str, 10);
        for (uint8_t i = strlen(str); i < 16; i++) {
            console_send_str(console, " ");
        }
        console_send_str(console, str);
    } else {
        for (uint8_t i = strlen(name); i < 16; i++) {
            console_send_str(console, " ");
        }
        console_send_str(console, name);
    }
    console_send_str(console, ": 0x");

    utoa(adc_get_value(channel), str, 16);
    console_send_str(console, str);

    if (*unit != '\0') {
        console_send_str(console, " (");

        debug_print_fixed_point(console, parsed_value, decimals);

        console_send_str(console, " ");
        console_send_str(console, unit);
        console_send_str(console, ")\n");
    } else {
        console_send_str(console, "\n");
    }
}

void debug_analog (uint8_t argc, char **argv, struct console_desc_t *console)
{
    char str[16];

    console_send_str(console, "Last sweep was at ");
    utoa(adc_get_last_sweep_time(), str, 10);
    console_send_str(console, str);
    console_send_str(console, " (");
    utoa(millis - adc_get_last_sweep_time(), str, 10);
    console_send_str(console, str);
    console_send_str(console, " milliseconds ago)\n");

    uint32_t channel_mask = adc_get_channel_mask();

    if (channel_mask & ~0x1F000000) {
        // Use header pin numbers
        uint8_t analog_header_chans[] = HEADER_ANALOG_PINS;
        // Iterate over all of the analog header pins
        for (uint8_t i = 0; i < NUM_ANALOG_PINS; i++) {
            uint8_t chan = analog_header_chans[i];

            if (channel_mask & (1 << chan)) {
                uint16_t volts = adc_get_value_millivolts(
                                                        analog_header_chans[i]);

                utoa(i, str , 10);
                uint8_t p = strlen(str);
                if (chan < 10) {
                    str[p] = ' ';
                    p++;
                }
                str[p] = ' ';
                str[p + 1] = '(';
                utoa(chan, str + p + 2, 10);
                p = strlen(str);
                str[p] = ')';
                str[p + 1] = '\0';

                debug_analog_print_channel(console, analog_header_chans[i],
                                           (uint32_t)volts, 3, str, "V");
            }
        }

        wdt_pat();
        console_send_str(console, "\n");
    }
    // Iterate over enabled internal channels
    channel_mask &= 0x1F000000;

    while (channel_mask != 0) {
        uint32_t t = channel_mask & -channel_mask;
        int i = __builtin_ctzl(channel_mask);

        if (i == ADC_INPUTCTRL_MUXPOS_TEMP_Val) {
            int16_t temp = adc_get_temp_fine();
            debug_analog_print_channel(console, i, (uint32_t)temp, 2,
                                       "Temperature", "C");
        } else if (i == ADC_INPUTCTRL_MUXPOS_BANDGAP_Val) {
            uint16_t volts = adc_get_value_millivolts(i);
            debug_analog_print_channel(console, i, (uint32_t)volts, 3,
                                       "Bandgap", "V");
        } else if (i == ADC_INPUTCTRL_MUXPOS_SCALEDCOREVCC_Val) {
            int16_t volts = adc_get_core_vcc();
            debug_analog_print_channel(console, i, (uint32_t)volts, 3,
                                       "Core VCC", "V");
        } else if (i == ADC_INPUTCTRL_MUXPOS_SCALEDIOVCC_Val) {
            int16_t volts = adc_get_io_vcc();
            debug_analog_print_channel(console, i, (uint32_t)volts, 3,
                                       "IO VCC", "V");
        } else if (i == ADC_INPUTCTRL_MUXPOS_DAC_Val) {
            uint16_t volts = adc_get_value_millivolts(i);
            debug_analog_print_channel(console, i, (uint32_t)volts, 3,
                                       "DAC", "V");
        } else {
            uint16_t volts = adc_get_value_millivolts(i);
            debug_analog_print_channel(console, i, (uint32_t)volts, 3, "",
                                       "V");
        }

        channel_mask ^= t;
        wdt_pat();
    }
}

// MARK: ADC INIT

#ifndef ENABLE_ADC

struct pin_t {
    uint8_t num:5;
    uint8_t port:1;
};

static const struct pin_t adc_pins[] = {
    {.port = 0, .num = 2},  // AIN[0]
    {.port = 0, .num = 3},  // AIN[1]
    {.port = 1, .num = 8},  // AIN[2]
    {.port = 1, .num = 9},  // AIN[3]
    {.port = 0, .num = 4},  // AIN[4]
    {.port = 0, .num = 5},  // AIN[5]
    {.port = 0, .num = 6},  // AIN[6]
    {.port = 0, .num = 7},  // AIN[7]
    {.port = 1, .num = 0},  // AIN[8]
    {.port = 1, .num = 1},  // AIN[9]
    {.port = 1, .num = 2},  // AIN[10]
    {.port = 1, .num = 3},  // AIN[11]
    {.port = 1, .num = 4},  // AIN[12]
    {.port = 1, .num = 5},  // AIN[13]
    {.port = 1, .num = 6},  // AIN[14]
    {.port = 1, .num = 7},  // AIN[15]
    {.port = 0, .num = 8},  // AIN[16]
    {.port = 0, .num = 9},  // AIN[17]
    {.port = 0, .num = 10}, // AIN[18]
    {.port = 0, .num = 11}  // AIN[19]
};

static void adc_set_pmux (uint8_t channel)
{
    struct pin_t pin = adc_pins[channel];

    if ((pin.num % 2) == 0) {
        PORT->Group[pin.port].PMUX[pin.num / 2].bit.PMUXE = 0x1;
    } else {
        PORT->Group[pin.port].PMUX[pin.num / 2].bit.PMUXO = 0x1;
    }
    PORT->Group[pin.port].PINCFG[pin.num].bit.PMUXEN = 0b1;
}
#endif

void debug_adc_init (uint8_t argc, char **argv, struct console_desc_t *console)
{
#ifdef ENABLE_ADC
    console_send_str(console, "ADC driver is enabled. Use \"analog\" command "
                     "instead\n");
#else
    adc_set_pmux(18);
    adc_set_pmux(17);
    adc_set_pmux(16);
    adc_set_pmux(15);

    /* Enable the APBC clock for the ADC */
    PM->APBCMASK.reg |= PM_APBCMASK_ADC;

    /* Select the core clock for the ADC */
    GCLK->CLKCTRL.reg = (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK3 | GCLK_CLKCTRL_ID_ADC);
    // Wait for synchronization
    while (GCLK->STATUS.bit.SYNCBUSY);

    /* Reset ADC */
    ADC->CTRLA.bit.SWRST = 1;
    // Wait for reset to complete
    while (ADC->CTRLA.bit.SWRST | ADC->STATUS.bit.SYNCBUSY);

    ADC->SAMPCTRL.bit.SAMPLEN = 17;

    /* Use internal 1.0 V reference with reference buffer offset compensation */
    ADC->REFCTRL.reg = /*ADC_REFCTRL_REFCOMP |*/ ADC_REFCTRL_REFSEL_INT1V;

    /* 256x oversampling and decimation for 16 bit effective resolution */
    ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_256 | ADC_AVGCTRL_ADJRES(0);

    /* Configure Control B register */
    // Calculate prescaler value
    uint32_t prescaler = 30 - __builtin_clz((8000000UL - 1) / 2100000UL);
    // Make sure prescaler is at most 512
    prescaler &= 0x1FF;

    // Set prescaler, 16 bit output, free-running mode
    ADC->CTRLB.reg = (ADC_CTRLB_PRESCALER(prescaler) |
                      ADC_CTRLB_RESSEL_16BIT);
    // Wait for synchronization
    while (ADC->STATUS.bit.SYNCBUSY);

    /* Enable temperature reference */
    SYSCTRL->VREF.reg |= SYSCTRL_VREF_TSEN;

    ADC->INTENSET.bit.RESRDY = 1;
    // Enable ADC interrupt in NVIC
    NVIC_SetPriority(ADC_IRQn, 3);
    NVIC_EnableIRQ(ADC_IRQn);

    /* Enable ADC */
    ADC->CTRLA.bit.ENABLE = 1;
    // Wait for synchronization
    while (ADC->STATUS.bit.SYNCBUSY);
#endif
}

// MARK: ADC Read

#ifndef ENABLE_ADC
static volatile uint16_t adc_results[0x1C];
static volatile uint32_t adc_result_ready;
static volatile uint8_t adc_current_chan;
static volatile uint8_t adc_last_chan;

static void print_adc_result (struct console_desc_t *console, uint8_t channel)
{
    char str[8];

    while (!(adc_result_ready & (1 << channel))) {
        wdt_pat();
    }
    adc_result_ready &= ~(1 << channel);

    console_send_str(console, "Result (0x");
    utoa(channel, str, 16);
    console_send_str(console, str);
    console_send_str(console, "): 0x");
    utoa(adc_results[channel], str, 16);
    console_send_str(console, str);
    console_send_str(console, "\n");
}
#endif

void debug_adc_read (uint8_t argc, char **argv, struct console_desc_t *console)
{
#ifdef ENABLE_ADC
    console_send_str(console, "ADC driver is enabled. Use \"analog\" command "
                     "instead\n");
#else

    if ((argc < 2) || (argc > 3)) {
        console_send_str(console, DEBUG_ADC_READ_HELP);
        console_send_str(console, "\n");
        return;
    }

    char* end;

    uint32_t scan_start = strtoul(argv[1], &end, 0);
    if ((*end != '\0') || (scan_start > 0x1C)) {
        console_send_str(console, DEBUG_ADC_READ_HELP);
        console_send_str(console, "\n");
        return;
    }

    uint32_t scan_end;
    if (argc == 3) {
        scan_end = strtoul(argv[2], &end, 0);
        if ((*end != '\0') || (scan_end > 0x1C) || (scan_end < scan_start)) {
            console_send_str(console, DEBUG_ADC_READ_HELP);
            console_send_str(console, "\n");
            return;
        }
    } else {
        scan_end = scan_start;
    }
    adc_last_chan = scan_end;

    ADC->INPUTCTRL.reg = (ADC_INPUTCTRL_GAIN_1X |
                          ADC_INPUTCTRL_INPUTOFFSET(0) |
                          ADC_INPUTCTRL_INPUTSCAN(scan_end - scan_start) |
                          ADC_INPUTCTRL_MUXNEG_GND |
                          ADC_INPUTCTRL_MUXPOS(scan_start));

    adc_current_chan = scan_start;

    ADC->CTRLB.bit.FREERUN = 0b1;
    // Wait for synchronization
    while (ADC->STATUS.bit.SYNCBUSY);

    for (uint8_t i = scan_start; i <= scan_end; i++) {
        print_adc_result(console, i);
    }
#endif
}

#ifndef ENABLE_ADC
void ADC_Handler (void)
{
    if (ADC->INTFLAG.bit.RESRDY) {
        adc_results[adc_current_chan] = ADC->RESULT.reg;
        adc_result_ready |= (1 << adc_current_chan);

        adc_current_chan++;

        if (adc_current_chan > adc_last_chan) {
            ADC->CTRLB.bit.FREERUN = 0b0;
            ADC->SWTRIG.bit.FLUSH = 0b1;
        }
    }
}
#endif


// MARK: DAC

enum debug_dac_mode {
    DEBUG_DAC_MODE_VOLTS,
    DEBUG_DAC_MODE_RAW
};

static uint16_t parse_value(char *str, char **end, uint8_t *has_decimal)
{
    uint16_t value = (uint16_t)strtoul(str, end, 0);

    if (**end == '.') {
        // Number has decimal
        *has_decimal = 1;
        value *= 1000;

        char *s = *end + 1;

        if (*s == '\0') {
            return 0;
        }

        for (uint16_t weight = 100; weight != 0; weight /= 10) {
            if (isdigit((unsigned char)*s)) {
                value += weight * (*s - '0');
            } else {
                break;
            }
            s++;
        }
        *end = s;
    }

    return value;
}

void debug_dac (uint8_t argc, char **argv, struct console_desc_t *console)
{
    static uint8_t dac_initialized;

    uint8_t has_decimal = 0;
    uint16_t value = 0;
    enum debug_dac_mode mode = DEBUG_DAC_MODE_RAW;

    if (!dac_initialized && (argc != 3)) {
        console_send_str(console, "DAC not initialized, run 'dac init 1V' or "\
                         "'dac init 3.3V'.\n");
        return;
    }

    if (argc == 2) {
        // One argument provided, it should be a value

        char* end;
        value = parse_value(argv[1], &end, &has_decimal);

        if (!strcasecmp(end, "v")) {
            mode = DEBUG_DAC_MODE_VOLTS;
        } else if (*end != '\0') {
            console_send_str(console, DEBUG_DAC_HELP);
            console_send_str(console, "\n");
            return;
        }
    } else if (argc == 3) {
        if (!strncasecmp(argv[1], "init", 4)) {
            if (!strcasecmp(argv[2], "1v")) {
                init_dac(GCLK_CLKCTRL_GEN_GCLK0, DAC_REF_1V, 1, 1);
            } else if (!strcasecmp(argv[2], "3.3v")) {
                init_dac(GCLK_CLKCTRL_GEN_GCLK0, DAC_REF_AVCC, 1, 1);
            } else {
                console_send_str(console, "DAC not initialized, run 'dac init "\
                                 "1V' or 'dac init 3.3V'.\n");
                return;
            }
            dac_initialized = 1;
            return;
        } else if (dac_initialized) {
            // Two arguments provided, the first should be a value and the
            // second should be a unit
            char* end;
            value = parse_value(argv[1], &end, &has_decimal);

            if (*end != '\0') {
                console_send_str(console, DEBUG_DAC_HELP);
                console_send_str(console, "\n");
                return;
            }

            if (!strcasecmp(argv[2], "v") || !strcasecmp(argv[2], "volts")) {
                mode = DEBUG_DAC_MODE_VOLTS;
            } else if (!strcasecmp(argv[2], "raw")) {
                mode = DEBUG_DAC_MODE_RAW;
            } else {
                console_send_str(console, DEBUG_DAC_HELP);
                console_send_str(console, "\n");
                return;
            }
        } else {
            console_send_str(console, "DAC not initialized, run 'dac init 1V' "\
                             "or 'dac init 3.3V'.\n");
            return;
        }
    } else if (argc > 3) {
        console_send_str(console, DEBUG_DAC_HELP);
        return;
    }

    // Check that if a value with a decimal was provided, it is a voltage
    if (has_decimal && (mode == DEBUG_DAC_MODE_RAW)) {
        console_send_str(console, "Raw value must be an integer.\n");
        return;
    }

    // Update DAC Value
    if (argc != 1) {
        if (mode == DEBUG_DAC_MODE_VOLTS) {
            if (!has_decimal) {
                value *= 1000;
            }
            dac_set_millivolts(value);
        } else if (mode == DEBUG_DAC_MODE_RAW) {
            dac_set(value);
        }
    }

    char str[8];

    // Print current DAC value
    console_send_str(console, "DAC value: ");
    utoa(dac_get_value(), str, 10);
    console_send_str(console, str);
    console_send_str(console, " (");
    debug_print_fixed_point(console, dac_get_value_millivolts(), 3);
    console_send_str(console, ")\n");
}
