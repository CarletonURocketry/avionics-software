/**
 * @file debug-commands.c
 * @desc Functions to be run from CLI for debugging purposes.
 * @author Samuel Dewan
 * @date 2019-09-22
 * Last Author: Samuel Dewan
 * Last Edited On: 2019-07-25
 */

#include "debug-commands.h"

#include <string.h>
#include <math.h>

#include "debug-commands-general.h"
#include "debug-commands-analog.h"
#include "debug-commands-sensors.h"
#include "debug-commands-radio.h"
#include "debug-commands-sd.h"

// MARK: Helpers
void debug_print_fixed_point (struct console_desc_t *console, int32_t value,
                              uint8_t decimal_places)
{
    int32_t scale = pow(10, decimal_places);
    
    char str[10];
    int32_t whole = value / scale;
    
    if ((whole == 0) && (value < 0)) {
        console_send_str(console, "-0.");
    } else {
        itoa(whole, str, 10);
        console_send_str(console, str);
        console_send_str(console, ".");
    }
    
    int32_t frac = abs(value - (whole * scale));
    itoa(frac, str, 10);
    for (int i = strlen(str); i < decimal_places; i++) {
        console_send_str(console, "0");
    }
    console_send_str(console, str);
}

void debug_print_byte_with_pad (struct console_desc_t *console,
                                const char *line_start, uint8_t byte,
                                const char *line_end)
{
    char str[9];

    console_send_str(console, line_start);

    utoa(byte, str, 2);
    for (uint8_t i = strlen(str); i < 8; i++) {
        console_send_str(console, "0");
    }
    console_send_str(console, str);

    console_send_str(console, line_end);
}

// MARK: Commands table

const struct cli_func_desc_t debug_commands_funcs[] = {
    // General
    {.func = debug_version, .name = DEBUG_VERSION_NAME,
        .help_string = DEBUG_VERSION_HELP},
    {.func = debug_did, .name = DEBUG_DID_NAME, .help_string = DEBUG_DID_HELP},
    {.func = debug_i2c_scan, .name = DEBUG_I2C_SCAN_NAME,
        .help_string = DEBUG_I2C_SCAN_HELP},
    {.func = debug_io_exp_regs, .name = DEBUG_IO_EXP_REGS_NAME,
        .help_string = DEBUG_IO_EXP_REGS_HELP},
    {.func = debug_gpio, .name = DEBUG_GPIO_NAME,
        .help_string = DEBUG_GPIO_HELP},
    // Analog
    {.func = debug_temp, .name = DEBUG_TEMP_NAME,
        .help_string = DEBUG_TEMP_HELP},
    {.func = debug_analog, .name = DEBUG_ANALOG_NAME,
        .help_string = DEBUG_ANALOG_HELP},
    {.func = debug_adc_init, .name = DEBUG_ADC_INIT_NAME,
        .help_string = DEBUG_ADC_INIT_HELP},
    {.func = debug_adc_read, .name = DEBUG_ADC_READ_NAME,
        .help_string = DEBUG_ADC_READ_HELP},
    {.func = debug_dac, .name = DEBUG_DAC_NAME, .help_string = DEBUG_DAC_HELP},
    // Sensors
    {.func = debug_alt_prom, .name = DEBUG_ALT_PROM_NAME,
        .help_string = DEBUG_ALT_PROM_HELP},
    {.func = debug_imu_wai, .name = DEBUG_IMU_WAI_NAME,
        .help_string = DEBUG_IMU_WAI_HELP},
    {.func = debug_alt, .name = DEBUG_ALT_NAME, .help_string = DEBUG_ALT_HELP},
    {.func = debug_alt_tare_now, .name = DEBUG_ALT_TARE_NOW_NAME,
        .help_string = DEBUG_ALT_TARE_NOW_HELP},
    {.func = debug_alt_tare_next, .name = DEBUG_ALT_TARE_NEXT_NAME,
        .help_string = DEBUG_ALT_TARE_NEXT_HELP},
    {.func = debug_gnss, .name = DEBUG_GNSS_NAME,
        .help_string = DEBUG_GNSS_HELP},
    // Radio
    {.func = debug_lora_version, .name = DEBUG_LORA_VERSION_NAME,
        .help_string = DEBUG_LORA_VERSION_HELP},
    {.func = debug_radio_info, .name = DEBUG_RADIO_INFO_NAME,
        .help_string = DEBUG_RADIO_INFO_HELP},
    {.func = debug_radio_rx, .name = DEBUG_RADIO_RX_NAME,
        .help_string = DEBUG_RADIO_RX_HELP},
    {.func = debug_radio_tx, .name = DEBUG_RADIO_TX_NAME,
        .help_string = DEBUG_RADIO_TX_HELP},
    {.func = debug_sdspi, .name = DEBUG_SDSPI_NAME,
        .help_string = DEBUG_SDSPI_HELP},
    {.func = NULL, .name = NULL, .help_string = NULL}
};
