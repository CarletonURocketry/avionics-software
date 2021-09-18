/**
 * @file variant.c
 * @desc Variant specific functions and data for test variant
 * @author Samuel Dewan
 * @date 2020-09-05
 * Last Author:
 * Last Edited On:
 */

#include "variant.h"

#include "global.h"
#include "board.h"

#include "console.h"
#include "cli.h"
#include "debug-commands.h"

#include "gnss-xa1110.h"
#include "ms5611.h"
#include "rn2483.h"

#include "radio-transport.h"
#include "logging.h"

#include "ground.h"
#include "telemetry.h"


#ifdef ENABLE_LORA
// This structure must be 4 bytes aligned because the radio transport module
// uses the two least significant bits of a pointer to this structure to store
// other information.
struct radio_transport_desc radio_transport_g __attribute__((__aligned__(4)));

#ifdef LORA_RADIO_0_UART
static struct radio_instance_desc radio_0_g;
#ifdef LORA_RADIO_0_ANT_MASK
static struct radio_antmgr_desc radio_antmgr_0_g;
#endif
#endif
#ifdef LORA_RADIO_1_UART
static struct radio_instance_desc radio_1_g;
#ifdef LORA_RADIO_1_ANT_MASK
static struct radio_antmgr_desc radio_antmgr_1_g;
#endif
#endif
#ifdef LORA_RADIO_2_UART
static struct radio_instance_desc radio_2_g;
#ifdef LORA_RADIO_2_ANT_MASK
static struct radio_antmgr_desc radio_antmgr_2_g;
#endif
#endif
#ifdef LORA_RADIO_3_UART
static struct radio_instance_desc radio_3_g;
#ifdef LORA_RADIO_3_ANT_MASK
static struct radio_antmgr_desc radio_antmgr_3_g;
#endif
#endif

/** Array of pointers to radio instances that are in use. */
struct radio_instance_desc *const radios_g[] =
{
#ifdef LORA_RADIO_0_UART
    &radio_0_g,
#endif
#ifdef LORA_RADIO_1_UART
    &radio_1_g,
#endif
#ifdef LORA_RADIO_2_UART
    &radio_2_g,
#endif
#ifdef LORA_RADIO_3_UART
    &radio_3_g
#endif
    NULL
};

// Check that in use uart descriptors are at least 4 byte aligned
#ifdef LORA_RADIO_0_UART
_Static_assert((((uintptr_t)&LORA_RADIO_0_UART) & 0b11) == 0, "UART descriptor "
               "for radio 0 must be 4 byte aligned.");
#endif
#ifdef LORA_RADIO_1_UART
_Static_assert((((uintptr_t)&LORA_RADIO_1_UART) & 0b11) == 0, "UART descriptor "
               "for radio 1 must be 4 byte aligned.");
#endif
#ifdef LORA_RADIO_2_UART
_Static_assert((((uintptr_t)&LORA_RADIO_2_UART) & 0b11) == 0, "UART descriptor "
               "for radio 2 must be 4 byte aligned.");
#endif
#ifdef LORA_RADIO_3_UART
_Static_assert((((uintptr_t)&LORA_RADIO_3_UART) & 0b11) == 0, "UART descriptor "
               "for radio 3 must be 4 byte aligned.");
#endif

#define UART_EMBED_NUM(u, i) ((struct sercom_uart_desc_t *)(((uintptr_t)&u) | i))

/**
 *  Array of pointers to the sercom uart instance for each active radio.
 *  The least significant 2 bits of each address is used to encode the radio
 *  number.
 */
static struct sercom_uart_desc_t *const radio_uarts_g[] =
{
#ifdef LORA_RADIO_0_UART
    UART_EMBED_NUM(LORA_RADIO_0_UART, 0),
#endif
#ifdef LORA_RADIO_1_UART
    UART_EMBED_NUM(LORA_RADIO_1_UART, 1),
#endif
#ifdef LORA_RADIO_2_UART
    UART_EMBED_NUM(LORA_RADIO_2_UART, 2),
#endif
#ifdef LORA_RADIO_3_UART
    UART_EMBED_NUM(LORA_RADIO_3_UART, 3),
#endif
    NULL
};

/** Array of antenna switch information for each active radio. */
static const struct radio_antenna_info radio_antennas_g[] =
{
#ifdef LORA_RADIO_0_UART
    (struct radio_antenna_info) {
#if defined(LORA_RADIO_0_ANT_MASK)
        .antmgr = &radio_antmgr_0_g,
        .antenna_mask = LORA_RADIO_0_ANT_MASK,
        .fixed_antenna_num = 0
#elif defined(LORA_RADIO_0_ANT_FIXED)
        .fixed_antenna_num = LORA_RADIO_0_ANT_FIXED
#else
        .antmgr = NULL
#endif
    },
#endif
#ifdef LORA_RADIO_1_UART
    (struct radio_antenna_info) {
#if defined(LORA_RADIO_1_ANT_MASK)
        .antmgr = &radio_antmgr_1_g,
        .antenna_mask = LORA_RADIO_1_ANT_MASK,
        .fixed_antenna_num = 0
#elif defined(LORA_RADIO_1_ANT_FIXED)
        .fixed_antenna_num = LORA_RADIO_1_ANT_FIXED
#else
        .antmgr = NULL
#endif
    },
#endif
#ifdef LORA_RADIO_2_UART
    (struct radio_antenna_info) {
#if defined(LORA_RADIO_2_ANT_MASK)
        .antmgr = &radio_antmgr_2_g,
        .antenna_mask = LORA_RADIO_2_ANT_MASK,
        .fixed_antenna_num = 0
#elif defined(LORA_RADIO_2_ANT_FIXED)
        .fixed_antenna_num = LORA_RADIO_2_ANT_FIXED
#else
        .antmgr = NULL
#endif
    },
#endif
#ifdef LORA_RADIO_3_UART
    (struct radio_antenna_info) {
#if defined(LORA_RADIO_3_ANT_MASK)
        .antmgr = &radio_antmgr_3_g,
        .antenna_mask = LORA_RADIO_3_ANT_MASK,
        .fixed_antenna_num = 0
#elif defined(LORA_RADIO_3_ANT_FIXED)
        .fixed_antenna_num = LORA_RADIO_3_ANT_FIXED
#else
        .antmgr = NULL
#endif
    },
#endif
};

#endif /* ENABLE_LORA */

#ifdef ENABLE_ALTIMETER
struct ms5611_desc_t altimeter_g;
#endif

#ifdef ENABLE_GNSS
struct console_desc_t gnss_console_g;
#endif

#ifdef ENABLE_GROUND_SERVICE
struct console_desc_t ground_station_console_g;
#endif

#ifdef ENABLE_CONSOLE
struct console_desc_t console_g;
#endif

#ifdef ENABLE_DEBUG_CLI
struct cli_desc_t cli_g;
#endif

#ifdef ENABLE_LOGGING
struct logging_desc_t logging_g;
#endif

void init_variant(void)
{
    // SD card logging service
#ifdef ENABLE_LOGGING
#if defined(ENABLE_SDHC0)
    init_logging(&logging_g, &sdhc0_g, sdhc_sd_funcs, 0);
#elif defined(ENABLE_SDSPI)
    init_logging(&logging_g, &sdspi_g, sdspi_sd_funcs, 0);
#endif
#ifdef LOGGING_START_PAUSED
    logging_pause(&logging_g);
#endif
#endif

    // Init Altimeter
#ifdef ENABLE_ALTIMETER
    init_ms5611(&altimeter_g, &i2c0_g, ALTIMETER_CSB, ALTIMETER_PERIOD, 1);
#endif

    // Init GNSS
#ifdef ENABLE_GNSS
    init_uart_console(&gnss_console_g, &GNSS_UART, '\0');
    init_gnss_xa1110(&gnss_console_g);
#endif

    // Console
#ifdef ENABLE_CONSOLE
#ifdef CONSOLE_UART
    init_uart_console(&console_g, &CONSOLE_UART, '\r');
#elif defined ENABLE_USB
#ifdef CONSOLE_CDC_PORT
    init_usb_cdc_console(&console_g, CONSOLE_CDC_PORT, '\r');
#else
#error Debugging console is configured to use USB, but CONSOLE_CDC_PORT is not defined.
#endif
#else
#error Debugging console is configured to use USB, but USB is not enabled.
#endif
#endif

    // Debug CLI
#ifdef ENABLE_DEBUG_CLI
    init_cli(&cli_g, &console_g, "> ", debug_commands_funcs);
#endif

    // LoRa Radios
#ifdef ENABLE_LORA
    init_radio_transport(&radio_transport_g, radios_g, radio_uarts_g,
                         radio_antennas_g, LORA_RADIO_SEARCH_ROLE,
                         LORA_DEVICE_ADDRESS);
#endif

    // Ground station console
#ifdef ENABLE_GROUND_SERVICE
#ifdef GROUND_UART
    init_uart_console(&ground_station_console_g, &GROUND_UART, '\r');
#elif defined ENABLE_USB
#ifdef GROUND_CDC_PORT
    init_usb_cdc_console(&ground_station_console_g, GROUND_CDC_PORT, '\r');
#else
#error Ground console is configured to use USB, but GROUND_CDC_PORT is not defined.
#endif
#else
#error Ground console is configured to use USB, but USB is not enabled.
#endif
    init_ground_service(&ground_station_console_g, &rn2483_g);
#endif

    // Telemetry service
#ifdef ENABLE_TELEMETRY_SERVICE
    init_telemetry_service(&rn2483_g, &altimeter_g, TELEMETRY_RATE);
#endif
}

void variant_service(void)
{
#ifdef ENABLE_CONSOLE
    console_service(&console_g);
#endif

#ifdef ENABLE_ALTIMETER
    ms5611_service(&altimeter_g);
#endif

#ifdef ENABLE_GNSS
    console_service(&gnss_console_g);
#endif

#ifdef ENABLE_LORA
    radio_transport_service(&radio_transport_g);
#endif

#ifdef ENABLE_LOGGING
    logging_service(&logging_g);
#endif

#ifdef ENABLE_GROUND_SERVICE
    ground_service();
#endif

#ifdef ENABLE_TELEMETRY_SERVICE
    telemetry_service();
#endif
}
