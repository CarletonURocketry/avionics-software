/**
 * @file radio-chanmgr.c
 * @desc Radio channel manger, monitors link quality and tracks when radio
 *       settings should be changed.
 * @author Samuel Dewan
 * @date 2020-03-24
 * Last Author:
 * Last Edited On:
 */

#include "radio-chanmgr.h"
#include "radio-transport.h"
#include "radio-packet-layout.h"
#include "radio-control-block-layout.h"

#include "lora-config.h"

#define AVG_RX_LOSS_FALL_FACTOR  6
#define AVG_RX_LOSS_RISE_FACTOR  2
#define AVG_TX_LOSS_FALL_FACTOR  4
#define AVG_TX_LOSS_RISE_FACTOR  2


void init_radio_chanmgr(struct radio_transport_desc *inst)
{
    // Load initial lora settings
    rn2483_settings_set_freq(&inst->radio_settings, LORA_FREQ);
    rn2483_settings_set_rf(&inst->radio_settings, LORA_POWER,
                           LORA_SPREADING_FACTOR, LORA_CODING_RATE,
                           LORA_BANDWIDTH);
    rn2483_settings_set_sync(&inst->radio_settings, LORA_CRC, LORA_INVERT_IQ,
                             LORA_SYNC_WORD, LORA_PRLEN);
}

void radio_chanmgr_service(struct radio_transport_desc *inst)
{
    // Periodically send a signal report request
    if ((millis - inst->last_sig_report_time) > RADIO_SIG_REPORT_PERIOD) {
        inst->last_sig_report_time = millis;

        // Find the best most recent snr and rssi from all of the attached
        // radios
        int8_t snr = INT8_MIN;
        int8_t rssi = INT8_MIN;
        for (struct radio_instance_desc *const *radio_p = inst->radios;
             *radio_p != NULL; radio_p++) {
            if ((*radio_p)->last_rx_snr >= snr) {
                snr = (*radio_p)->last_rx_snr;
                rssi = (*radio_p)->last_rx_rssi;
            }
        }

        uint8_t report[RADIO_BLOCK_SIG_REPORT_LENGHT];
        radio_block_marshal_header(report, RADIO_BLOCK_SIG_REPORT_LENGHT, 0,
                                   RADIO_DEVICE_ADDRESS_MULTICAST,
                                   RADIO_BLOCK_TYPE_CONTROL,
                                   RADIO_CONTROL_BLOCK_SIGNAL_REPORT);
        radio_block_marshal_sig_report(report, snr, rssi, 0,
                                       inst->radio_settings.power, 0);
        radio_send_block(inst, report, RADIO_BLOCK_SIG_REPORT_LENGHT, 1000,
                         RADIO_SIG_REPORT_PERIOD);
    }
}

struct radio_instance_desc *radio_chanmgr_get_tx_radio(
                                            struct radio_transport_desc *inst)
{
    struct radio_instance_desc *radio = NULL;
    int8_t tx_loss = INT8_MAX;
    for (struct radio_instance_desc *const *radio_p = inst->radios;
         *radio_p != NULL; radio_p++) {
        if ((*radio_p)->avg_tx_power_loss <= tx_loss) {
            radio = (*radio_p);
        }
    }
    return radio;
}


void radio_chanmgr_metadata_cb(struct radio_transport_desc *transport,
                               struct radio_instance_desc *radio,
                               uint8_t antenna_num, int8_t snr, int8_t rssi)
{
    // The metadata from a received callback gives us a limited amount of
    // information about our rx link quality.
    radio->last_rx_snr = snr;
    radio->last_rx_rssi = rssi;
}

void radio_chanmgr_rx_loss_cb(struct radio_transport_desc *transport,
                              struct radio_instance_desc *radio,
                              uint8_t antenna_num, int8_t rx_loss)
{
    // Update average RX loss value
    const uint8_t factor = ((rx_loss > radio->avg_rx_power_loss) ?
                            AVG_RX_LOSS_RISE_FACTOR : AVG_RX_LOSS_FALL_FACTOR);
    const uint8_t count = update_moving_average(&radio->avg_rx_power_loss,
                                                rx_loss,
                                                radio->rx_power_loss_count,
                                                factor);
    radio->rx_power_loss_count = count;
}

void radio_chanmgr_tx_loss_cb(struct radio_transport_desc *transport,
                              struct radio_instance_desc *tx_radio,
                              uint8_t antenna_num, int8_t tx_loss,
                              int8_t remote_snr)
{
    // Update average TX loss value
    const uint8_t factor = ((tx_loss > tx_radio->avg_tx_power_loss) ?
                            AVG_TX_LOSS_RISE_FACTOR : AVG_TX_LOSS_FALL_FACTOR);
    const uint8_t count = update_moving_average(&tx_radio->avg_tx_power_loss,
                                                tx_loss,
                                                tx_radio->tx_power_loss_count,
                                                factor);
    tx_radio->tx_power_loss_count = count;
}
