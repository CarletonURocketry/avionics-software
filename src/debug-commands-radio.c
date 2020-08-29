/**
 * @file debug-commands-radio.c
 * @desc Commands for debugging CLI
 * @author Samuel Dewan
 * @date 2020-08-11
 * Last Author:
 * Last Edited On:
 */

#include "debug-commands-radio.h"

#include "debug-commands.h"

#include "config.h"
#include "wdt.h"
#include "sercom-uart.h"
#include "radio-types.h"
#include "radio-antmgr.h"
#include "radio-packet-layout.h"
#include "radio-control-block-layout.h"

// MARK: LoRa Version

void debug_lora_version (uint8_t argc, char **argv,
                         struct console_desc_t *console)
{
    sercom_uart_put_string_blocking(&uart1_g, "sys get ver\r\n");
    while (!sercom_uart_has_line(&uart1_g)) {
        wdt_pat();
    }
    char str[128];
    sercom_uart_get_line(&uart1_g, str, 128);
    console_send_str(console, str);
    console_send_str(console, "\n");
}

// MARK: Radio Info

static void print_radio_firmware_version(struct console_desc_t *console,
                                         uint16_t version)
{
    char str[4];
    utoa((version & RN2483_VER_NUM_MAJOR_MASK) >> RN2483_VER_NUM_MAJOR_POS, str,
         10);
    console_send_str(console, str);
    console_send_str(console, ".");
    utoa((version & RN2483_VER_NUM_MINOR_MASK) >> RN2483_VER_NUM_MINOR_POS, str,
         10);
    console_send_str(console, str);
    console_send_str(console, ".");
    utoa((version & RN2483_VER_NUM_REV_MASK) >> RN2483_VER_NUM_REV_POS, str,
         10);
    console_send_str(console, str);
}

static void print_enabled_antennas(struct console_desc_t *console,
                                   const struct radio_antmgr_desc *antmgr)
{
    int first = 1;
    if (antmgr->antenna_mask & ANTMGR_ANT_1_MASK) {
        first = 0;
        console_send_str(console, "1");
    }
    if (antmgr->antenna_mask & ANTMGR_ANT_2_MASK) {
        if (!first) {
            console_send_str(console, ", ");
        }
        first = 0;
        console_send_str(console, "2");
    }
    if (antmgr->antenna_mask & ANTMGR_ANT_3_MASK) {
        if (!first) {
            console_send_str(console, ", ");
        }
        first = 0;
        console_send_str(console, "3");
    }
    if (antmgr->antenna_mask & ANTMGR_ANT_4_MASK) {
        if (!first) {
            console_send_str(console, ", ");
        }
        first = 0;
        console_send_str(console, "4");
    }
}

void debug_radio_info (uint8_t argc, char **argv,
                       struct console_desc_t *console)
{
    char str[32];

    // Transport info
    console_send_str(console, "Last TX time: ");
    utoa(radio_transport_g.last_tx_time, str, 10);
    console_send_str(console, str);
    console_send_str(console, " (");
    utoa(millis - radio_transport_g.last_tx_time, str, 10);
    console_send_str(console, str);
    console_send_str(console, " milliseconds ago)\nLast RX time: ");
    utoa(radio_transport_g.last_rx_time, str, 10);
    console_send_str(console, str);
    console_send_str(console, " (");
    utoa(millis - radio_transport_g.last_rx_time, str, 10);
    console_send_str(console, str);
    console_send_str(console, " milliseconds ago)\n");

    // Radio instances
    for (struct radio_instance_desc *const *radio_p = radios_g;
                *radio_p != NULL; radio_p++) {
        console_send_str(console, "\nRadio ");
        utoa((*radio_p)->radio_num, str, 10);
        console_send_str(console, str);
        console_send_str(console, ":\n\tFirmware version: ");

        // Firmware version
        print_radio_firmware_version(console, (*radio_p)->rn2483.version);

        // Driver State
        console_send_str(console, "\n\tDriver state: 0x");
        utoa((*radio_p)->rn2483.state, str, 16);
        console_send_str(console, str);

        // Link performance
        console_send_str(console, "\n\tLink performance:\n\t\tAvg. RX loss: ");
        itoa((*radio_p)->avg_rx_power_loss, str, 10);
        console_send_str(console, str);
        console_send_str(console, " dBm\n\t\tAvg. TX loss: ");
        itoa((*radio_p)->avg_tx_power_loss, str, 10);
        console_send_str(console, str);
        console_send_str(console, " dBm\n\t\tLast SNR: ");
        itoa((*radio_p)->last_rx_snr, str, 10);
        console_send_str(console, str);
        console_send_str(console, " dBm\n\t\tLast RSSI: ");
        itoa((*radio_p)->last_rx_rssi, str, 10);
        console_send_str(console, str);

        // Antenna Information
        console_send_str(console, " dBm\n\tAntenna: ");
        if ((*radio_p)->antmgr == NULL) {
            console_send_str(console, "fixed");
        } else {
            console_send_str(console, "\n\t\tEnabled antennas: ");
            print_enabled_antennas(console, (*radio_p)->antmgr);
            console_send_str(console, "\n\t\tCurrent antenna: ");
            utoa(radio_antmgr_get_current_antenna(*radio_p), str, 10);
            console_send_str(console, str);
        }

        wdt_pat();
    }

    // Radio settings
    console_send_str(console, "\n\nRadio Settings:\n\tFrequency: ");
    utoa(radio_transport_g.radio_settings.freq, str, 10);
    console_send_str(console, str);
    console_send_str(console, "\n\tTX Power: ");
    utoa(radio_transport_g.radio_settings.power, str, 10);
    console_send_str(console, str);
    console_send_str(console, " dBm\n\tSpreading Factor: ");
    switch (radio_transport_g.radio_settings.spreading_factor) {
        case RN2483_SF_SF7:
            utoa(7, str, 10);
            break;
        case RN2483_SF_SF8:
            utoa(8, str, 10);
            break;
        case RN2483_SF_SF9:
            utoa(9, str, 10);
            break;
        case RN2483_SF_SF10:
            utoa(10, str, 10);
            break;
        case RN2483_SF_SF11:
            utoa(11, str, 10);
            break;
        case RN2483_SF_SF12:
            utoa(12, str, 10);
            break;
    }
    console_send_str(console, str);
    console_send_str(console, "\n\tCoding Rate: 4/");
    switch (radio_transport_g.radio_settings.coding_rate) {
        case RN2483_CR_4_5:
            utoa(5, str, 10);
            break;
        case RN2483_CR_4_6:
            utoa(6, str, 10);
            break;
        case RN2483_CR_4_7:
            utoa(7, str, 10);
            break;
        case RN2483_CR_4_8:
            utoa(8, str, 10);
            break;
    }
    console_send_str(console, str);
    console_send_str(console, "\n\tBandwidth: ");
    switch (radio_transport_g.radio_settings.bandwidth) {
        case RN2483_BW_125:
            utoa(125, str, 10);
            break;
        case RN2483_BW_250:
            utoa(250, str, 10);
            break;
        case RN2483_BW_500:
            utoa(500, str, 10);
            break;
    }
    console_send_str(console, str);
    console_send_str(console, " kHz\n\tPreamble Length: ");
    utoa(radio_transport_g.radio_settings.preamble_length, str, 10);
    console_send_str(console, str);
    console_send_str(console, "\n\tCRC: ");
    console_send_str(console, radio_transport_g.radio_settings.crc ?
                     "yes" : "no");
    console_send_str(console, "\n\tInvert IQ: ");
    console_send_str(console, radio_transport_g.radio_settings.invert_qi ?
                     "yes" : "no");
    console_send_str(console, "\n\tSync Word: 0x");
    utoa(radio_transport_g.radio_settings.sync_byte, str, 16);
    console_send_str(console, str);

    console_send_str(console, "\n");
    return;
}

// MARK: Radio RX

static struct console_desc_t *radio_rx_console = NULL;

static void debug_radio_rx_cb (const uint8_t *packet, uint8_t length,
                               uint8_t radio_num, uint8_t antenna_num,
                               int8_t snr, int8_t rssi, int valid)
{
    if (radio_rx_console == NULL) {
        return;
    }

    char str[8];

    console_send_str(radio_rx_console, "RX on radio ");
    utoa(radio_num, str, 10);
    console_send_str(radio_rx_console, str);
    console_send_str(radio_rx_console, ", antenna ");
    utoa(antenna_num, str, 10);
    console_send_str(radio_rx_console, str);
    console_send_str(radio_rx_console, " at ");
    utoa(millis, str, 10);
    console_send_str(radio_rx_console, str);
    console_send_str(radio_rx_console, ": length = ");
    utoa(length, str, 10);
    console_send_str(radio_rx_console, str);
    console_send_str(radio_rx_console, ", snr = ");
    itoa(snr, str, 10);
    console_send_str(radio_rx_console, str);
    console_send_str(radio_rx_console, ", rssi = ");
    itoa(rssi, str, 10);
    console_send_str(radio_rx_console, str);
    console_send_str(radio_rx_console, valid ? " (valid)" : " (not valid)");

    if (!valid) {
        // Packet is not valid, there may not be a header to parse.
        return;
    }

    char callsign[RADIO_PACKET_CALLSIGN_LENGTH + 1];
    radio_packet_callsign(packet, callsign);

    console_send_str(radio_rx_console, "\n\tHeader: callsign = \"");
    console_send_str(radio_rx_console, callsign);
    console_send_str(radio_rx_console, "\", length = ");
    utoa(radio_packet_length(packet), str, 10);
    console_send_str(radio_rx_console, str);
    console_send_str(radio_rx_console, ", format ver = ");
    utoa(radio_packet_format_version(packet), str, 10);
    console_send_str(radio_rx_console, str);
    console_send_str(radio_rx_console, ", src addr = 0x");
    utoa(radio_packet_src_addr(packet), str, 16);
    console_send_str(radio_rx_console, str);
    console_send_str(radio_rx_console, ", pkt # = ");
    utoa(radio_packet_number(packet), str, 10);
    console_send_str(radio_rx_console, str);

    console_send_str(radio_rx_console, "\n\tPayload: ");
    for (uint8_t i = RADIO_PACKET_HEADER_LENGTH; i < length; i++) {
        if (packet[i] < 16) {
            console_send_str(radio_rx_console, "0");
        }
        utoa(packet[i], str, 16);
        console_send_str(radio_rx_console, str);
        console_send_str(radio_rx_console, " ");
    }
    console_send_str(radio_rx_console, "\n");

    // Print blocks
    for (const uint8_t *block = radio_packet_fist_block(packet); block != NULL;
                        block = radio_packet_next_block(packet, block)) {
        const enum radio_block_type type = radio_block_type(block);
        const uint8_t subtype = radio_block_subtype(block);
        const uint8_t block_length = radio_block_length(block);

        console_send_str(radio_rx_console, "\n\tBlock: type = 0x");
        utoa(type, str, 16);
        console_send_str(radio_rx_console, str);
        console_send_str(radio_rx_console, ", subtype = 0x");
        utoa(subtype, str, 16);
        console_send_str(radio_rx_console, str);
        console_send_str(radio_rx_console, ", length = ");
        utoa(block_length, str, 10);
        console_send_str(radio_rx_console, str);
        console_send_str(radio_rx_console, ", dest addr = 0x");
        utoa(radio_block_dest_addr(block), str, 16);
        console_send_str(radio_rx_console, str);
        console_send_str(radio_rx_console, ", signature: ");
        console_send_str(radio_rx_console, radio_block_has_signature(block) ?
                         "yes" : "no");

        if (!radio_block_sanity_check(packet, block)) {
            // Block is not valid
            console_send_str(radio_rx_console, "\n\t\tBlock not valid.");
        } else if ((type == RADIO_BLOCK_TYPE_DATA) &&
                   (subtype == RADIO_DATA_BLOCK_DEBUG)) {
            console_send_str(radio_rx_console, "\n\t\tDebug message: \"");
            char message[128];
            strncpy(message, (const char*)radio_block_payload(block),
                    (block_length - RADIO_BLOCK_HEADER_LENGTH) + 1);
            message[(block_length - RADIO_BLOCK_HEADER_LENGTH)] = '\0';
            console_send_str(radio_rx_console, message);
            console_send_str(radio_rx_console, "\"");
        } else if ((type == RADIO_BLOCK_TYPE_CONTROL) &&
                   (subtype == RADIO_CONTROL_BLOCK_SIGNAL_REPORT)) {
            console_send_str(radio_rx_console, "\n\t\tSignal Report: snr = ");
            itoa(radio_block_sig_report_snr(block), str, 10);
            console_send_str(radio_rx_console, str);
            console_send_str(radio_rx_console, ", rssi = ");
            itoa(radio_block_sig_report_rssi(block), str, 10);
            console_send_str(radio_rx_console, str);
            console_send_str(radio_rx_console, ", radio = ");
            utoa(radio_block_sig_report_radio(block), str, 10);
            console_send_str(radio_rx_console, str);
            console_send_str(radio_rx_console, ", tx power = ");
            itoa(radio_block_sig_report_tx_power(block), str, 10);
            console_send_str(radio_rx_console, str);
            console_send_str(radio_rx_console, ", request: ");
            console_send_str(radio_rx_console,
                             radio_block_sig_report_req(block) ? "yes" : "no");
        }
    }

    console_send_str(radio_rx_console, "\n\n\n");
}

void debug_radio_rx (uint8_t argc, char **argv, struct console_desc_t *console)
{
    // Store console pointer to global
    radio_rx_console = console;
    // Store current logging callback
    radio_rx_packet_cb log_cb = radio_transport_g.logging_callback;
    // Set logging callback to debug callback
    radio_transport_g.logging_callback = debug_radio_rx_cb;
    // Wait for newline
    while (!console_has_line(console)) {
        radio_transport_service(&radio_transport_g);
        wdt_pat();
    }
    // Eat the line
    while (console_has_line(console)) {
        char line[64];
        console_get_line(console, line, 64);
    }
    // Restore old logging callback
    radio_transport_g.logging_callback = log_cb;
    // Clear global console pointer
    radio_rx_console = NULL;
}

// MARK: Radio TX

#define DEBUG_RADIO_TX_MAX_LEN  100

void debug_radio_tx (uint8_t argc, char **argv, struct console_desc_t *console)
{
    // Check that we have at least one argument
    if (argc < 2) {
        console_send_str(console, "No message specified.\n");
        return;
    }

    // Start creating block
    uint8_t block[RADIO_BLOCK_HEADER_LENGTH + DEBUG_RADIO_TX_MAX_LEN];

    // Create message from arguments
    char *message = (char*)(block + RADIO_BLOCK_HEADER_LENGTH);
    message[0] = '\0';

    for (uint8_t i = 1; i < argc; i++) {
        size_t message_length = strlen(message);

        if (message_length >= DEBUG_RADIO_TX_MAX_LEN) {
            break;
        } else if (i != 1) {
            message[message_length] = ' ';
            message_length++;
        }

        strncpy(message + message_length, argv[i],
                DEBUG_RADIO_TX_MAX_LEN - message_length);
    }

    // Pad out the end of the message to make it a multiple of 4 bytes long
    uint8_t block_length = strlen(message) + RADIO_BLOCK_HEADER_LENGTH;
    for (; (block_length & 0x3) != 0; block_length++) {
        message[block_length] = '\0';
    }

    // Create block header
    radio_block_marshal_header(block, block_length, 0,
                               RADIO_DEVICE_ADDRESS_MULTICAST,
                               RADIO_BLOCK_TYPE_DATA,
                               RADIO_DATA_BLOCK_DEBUG);
    radio_send_block(&radio_transport_g, block, block_length, 0, 0);
}
