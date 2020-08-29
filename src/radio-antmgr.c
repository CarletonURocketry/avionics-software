/**
 * @file radio-antmgr.c
 * @desc Radio antenna manger, monitors link quality and selects best antenna.
 * @author Samuel Dewan
 * @date 2020-03-24
 * Last Author:
 * Last Edited On:
 */

#include "radio-antmgr.h"

#include "lora-config.h"

#define AVG_RX_LOSS_FALL_FACTOR  3
#define AVG_RX_LOSS_RISE_FACTOR  2
#define AVG_TX_LOSS_FALL_FACTOR  2
#define AVG_TX_LOSS_RISE_FACTOR  2

static enum sky13414_state ant_num_to_sky13414_state (unsigned int antenna);


void init_radio_antmgr(struct radio_instance_desc *inst,
                       const struct radio_antenna_info *info)
{
    // Store antenna manager data
    inst->antmgr = info->antmgr;
    inst->antmgr->antenna_mask = info->antenna_mask;

    // Initialize antenna switch driver
    init_sky13414(&inst->antmgr->antenna_switch, &inst->rn2483,
                  ANTENNA_SWITCH_V1, ANTENNA_SWITCH_V2, ANTENNA_SWITCH_V3);

    // Select first enabled antenna
    int ant = __builtin_ffs(inst->antmgr->antenna_mask);
    sky13414_set(&inst->antmgr->antenna_switch, ant_num_to_sky13414_state(ant));
}

void radio_antmgr_service(struct radio_instance_desc *inst)
{

}

void radio_antmgr_set_fixed(struct radio_instance_desc *inst,
                            const struct radio_antenna_info *info)
{
    // Create temporary SKY13414 driver instance
    struct sky13414_desc_t ant_switch;
    init_sky13414(&ant_switch, &inst->rn2483, ANTENNA_SWITCH_V1,
                  ANTENNA_SWITCH_V2, ANTENNA_SWITCH_V3);

    // Select desired antenna
    sky13414_set(&ant_switch,
                 ant_num_to_sky13414_state(info->fixed_antenna_num));
}

uint8_t radio_antmgr_get_current_antenna(const struct radio_instance_desc *inst)
{
    if (inst->antmgr == NULL) {
        return 0;
    }
    switch (inst->antmgr->antenna_switch.state) {
        case SKY13414_RF1:
            return 1;
        case SKY13414_RF2:
            return 2;
        case SKY13414_RF3:
            return 3;
        case SKY13414_RF4:
            return 4;
        default:
            return 0;
    }
}

void radio_antmgr_metadata_cb(struct radio_transport_desc *transport,
                              struct radio_instance_desc *radio,
                              uint8_t antenna_num, int8_t snr, int8_t rssi)
{

}

void radio_antmgr_rx_loss_cb(struct radio_transport_desc *transport,
                             struct radio_instance_desc *radio,
                             uint8_t antenna_num, int8_t rx_loss)
{
    // Get point to value to be updated
    int8_t *const avg = &radio->antmgr->avg_rx_power_loss[antenna_num];
    // Get current count of rx power loss data points
    uint8_t count = ((radio->antmgr->rx_power_loss_counts >>
                      (2 * antenna_num)) & 0x3);
    // Determine window size
    const uint8_t factor = ((rx_loss > *avg) ?
                            AVG_RX_LOSS_RISE_FACTOR : AVG_RX_LOSS_FALL_FACTOR);
    // Update the average
    count = update_moving_average(avg, rx_loss, count, factor);
    // Update the count of rx power loss data points
    radio->antmgr->rx_power_loss_counts &= ~(0x3 << (2 * antenna_num));
    radio->antmgr->rx_power_loss_counts |= (count << (2 * antenna_num));
}

void radio_antmgr_tx_loss_cb(struct radio_transport_desc *transport,
                             struct radio_instance_desc *tx_radio,
                             uint8_t antenna_num, int8_t tx_loss,
                             int8_t remote_snr)
{
    // Get point to value to be updated
    int8_t *const avg = &tx_radio->antmgr->avg_tx_power_loss[antenna_num];
    // Get current count of tx power loss data points
    uint8_t count = ((tx_radio->antmgr->tx_power_loss_counts >>
                      (2 * antenna_num)) & 0x3);
    // Determine window size
    const uint8_t factor = ((tx_loss > *avg) ?
                            AVG_RX_LOSS_RISE_FACTOR : AVG_RX_LOSS_FALL_FACTOR);
    // Update the average
    count = update_moving_average(avg, tx_loss, count, factor);
    // Update the count of rx power loss data points
    tx_radio->antmgr->tx_power_loss_counts &= ~(0x3 << (2 * antenna_num));
    tx_radio->antmgr->tx_power_loss_counts |= (count << (2 * antenna_num));
}

static enum sky13414_state ant_num_to_sky13414_state (unsigned int antenna)
{
    switch (antenna) {
        case 1:
            return SKY13414_RF1;
        case 2:
            return SKY13414_RF2;
        case 3:
            return SKY13414_RF3;
        case 4:
            return SKY13414_RF4;
        default:
            return SKY13414_50_OHM;
    }
}
