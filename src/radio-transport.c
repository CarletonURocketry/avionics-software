/**
 * @file radio-transport.c
 * @desc Transport layer for radio stack
 * @author Samuel Dewan
 * @date 2020-03-24
 * Last Author:
 * Last Edited On:
 */

#include "radio-transport.h"
#include "radio-packet-layout.h"
#include "radio-control-block-layout.h"
#include "lora-config.h"

#include "radio-chanmgr.h"
#include "radio-antmgr.h"

#define RADIO_PACKET_WATERLINE  100

#define RADIO_CREATE_RX_CB_CONTEXT(p, n) ((void*)((uintptr_t)p | (n & 0b11)))
#define RADIO_RX_CB_CONTEXT_POINTER(c) ((struct radio_transport_desc*)((uintptr_t)c & (~0b11)))
#define RADIO_RX_CB_CONTEXT_NUM(c) ((uint8_t)((uintptr_t)c & 0b11))


/**
 *  Function called when a packet is received.
 *
 *  @param rn2483 Radio driver instance for radio on which packet was received
 *  @param context Radio transport instance
 *  @param data Received data
 *  @param length Number of bytes received
 *  @param snr Signal to noise ratio
 *  @param rssi Received signal strength indicator
 *
 *  @return 1 if we should continue receiving, 0 otherwise
 */
static int radio_rx_callback(struct rn2483_desc_t *inst, void *context,
                             uint8_t *data, uint8_t length, int8_t snr,
                             int8_t rssi);


// MARK: Init

void init_radio_transport(struct radio_transport_desc *inst,
                          struct radio_instance_desc *const *radios,
                          struct sercom_uart_desc_t *const *radio_uarts,
                          const struct radio_antenna_info *radio_antennas,
                          enum radio_search_role search_role,
                          enum radio_packet_device_address address)
{
    inst->radios = radios;
    inst->search_role = search_role;
    inst->address = address;

    inst->last_tx_time = 0;
    inst->last_rx_time = 0;

    // Initialize channel manager
    init_radio_chanmgr(inst);

    // Initialize radios and antennas
    for (unsigned int i = 0; i < RADIO_MAX_NUM_RADIOS; i++) {
        if (radios[i] == NULL) {
            break;
        }

        // Get uart pointer
        struct sercom_uart_desc_t *u = ((struct sercom_uart_desc_t *)
                                        ((uintptr_t)radio_uarts[i] & (~0b11)));

        // Initialize RN2483 driver
        init_rn2483(&radios[i]->rn2483, u, &inst->radio_settings);

        // Initialize antennas
        if (radio_antennas[i].antmgr != NULL) {
            // This radio has an antenna switch and requires dynamic antenna
            // selection
            init_radio_antmgr(radios[i], &radio_antennas[i]);
        } else if (radio_antennas[i].fixed_antenna_num != 0) {
            // This radio has an antenna switch but a fixed antenna should be
            // used
            radio_antmgr_set_fixed(radios[i], &radio_antennas[i]);
        } else {
            // This radio does not have an antenna switch
            radios[i]->antmgr = NULL;
        }

        // Get radio instance number
        radios[i]->radio_num = (uint8_t)(((uintptr_t)radio_uarts[i]) & 0b11);

        // Have the radio start receiving as soon as possible
        rn2483_receive(&radios[i]->rn2483, radio_rx_callback,
                       RADIO_CREATE_RX_CB_CONTEXT(inst, i));
    }

    // Set TX state
    inst->tx_state = RADIO_TRANS_TX_IDLE;

    // Initialize buffer block descriptors
    for (unsigned int i = 0; i < RADIO_BLOCKS_PER_PACKET; i++) {
        inst->buffer_info[i].length = 0;
    }

    // Initialize packet header in buffer
    radio_packet_marshal_header(inst->packet_buffer, LORA_CALLSIGN,
                                RADIO_SUPPORTED_FORMAT_VERSION, address, 0,
                                RADIO_PACKET_HEADER_LENGTH);

    // Initialize packet header in priority buffer
    radio_packet_marshal_header(inst->priority_packet_buffer, LORA_CALLSIGN,
                                RADIO_SUPPORTED_FORMAT_VERSION, address, 0,
                                RADIO_PACKET_HEADER_LENGTH);

    inst->packet_number = 0;

    // Initialize flags
    inst->priority_tx_in_progress = 0;
    inst->buffer_slack_time_valid = 0;
    inst->prebuffer_slack_time_valid = 0;
}

// MARK: Service

/**
 *  Reset the packet buffer after a packet has been sent. Any prebuffered blocks
 *  will be moved into position to be sent in the next packet.
 *
 *  @param inst The instance for which the packet buffer should be reset
 */
static void reset_packet_buffer(struct radio_transport_desc *inst)
{
    // Clear out blocks that have been sent and shift over any prebuffered
    // blocks
    uint8_t block_shift_index = 0;
    uint8_t new_length = RADIO_PACKET_HEADER_LENGTH;

    for (unsigned int i = 0; i < RADIO_BLOCKS_PER_PACKET; i++) {
        struct radio_trans_buff_blk_info *block = &inst->buffer_info[i];
        if (block->prebuffered && block->length) {
            // This is a prebuffered block, shift it down if necessary
            block->prebuffered = 0;
            // Shift data
            memmove(inst->packet_buffer + new_length,
                    inst->packet_buffer + block->offset, block->length);
            new_length += block->length;
            // Shift descriptor
            memcpy(&inst->buffer_info[block_shift_index], block,
                   sizeof(struct radio_trans_buff_blk_info));
            block_shift_index++;
        }
        // Clear block
        block->length = 0;
    }

    // Update slack time
    if (inst->prebuffer_slack_time_valid) {
        inst->buffer_slack_time_base = inst->prebuffer_slack_time_base;
        inst->buffer_slack_time = inst->prebuffer_slack_time;
    }
    inst->buffer_slack_time_valid = inst->prebuffer_slack_time_valid;
    inst->prebuffer_slack_time_valid = 0;

    // Update packet length
    radio_packet_set_length(inst->packet_buffer, new_length);
}

/**
 *  Update the tx radio field for any signal report request blocks in a packet.
 *
 *  @param packet The packet to be updated
 *  @param radio_num The new tx radio number
 */
static void update_sig_report_radio_nums(uint8_t *packet, uint8_t radio_num)
{
    uint8_t packet_length = radio_packet_length(packet);

    for (uint8_t offset = RADIO_PACKET_HEADER_LENGTH; offset < packet_length;
            offset += radio_block_length(packet + offset)) {
        uint8_t *block = packet + offset;
        if ((radio_block_type(block) == RADIO_BLOCK_TYPE_CONTROL) &&
            (radio_block_subtype(block) == RADIO_CONTROL_BLOCK_SIGNAL_REPORT) &&
            (radio_block_sig_report_req(block))) {
            radio_block_sig_report_set_tx_radio(block, radio_num);
        }
    }
}

/**
 *  Start a transmission.
 *
 *  @param inst The instance for which the transmission should be started
 *  @param buffer The buffer to be transmitted
 *  @param length The number of bytes to be transmitted
 */
static void start_tx(struct radio_transport_desc *inst, uint8_t *buffer,
                     uint8_t length)
{
    radio_packet_set_number(buffer, inst->packet_number);
    struct radio_instance_desc *tx_radio = radio_chanmgr_get_tx_radio(inst);
    update_sig_report_radio_nums(buffer, tx_radio->radio_num);
    inst->tx_radio = &tx_radio->rn2483;
    const enum rn2483_operation_result result = rn2483_send(inst->tx_radio,
                                    buffer, length, &inst->tx_transaction_id);
    if (result == RN2483_OP_SUCCESS) {
        inst->last_tx_time = millis;
        inst->packet_number++;
        inst->tx_state = RADIO_TRANS_TX_IN_PROGRESS;
    }
}

/**
 *  Remove a contiguous sequence of blocks from the packet buffer.
 *
 *  @param inst The instance for which block should be removed
 *  @param first The index of the first block to be removed
 *  @param end The index of the next block after first that should not be
 *             removed
 */
static void remove_blocks(struct radio_transport_desc *inst, unsigned int first,
                          unsigned int end)
{
    if ((end > RADIO_BLOCKS_PER_PACKET) ||
            (inst->buffer_info[end].length == 0)) {
        // We are going off the end of the array (or at least off the end of
        // the in use portion of the array). No need to shift anything as there
        // are no blocks after the one(s) to be removed, we can just mark all
        // the blocks to be removed as invalid.
        for (unsigned int i = first; i < RADIO_BLOCKS_PER_PACKET; i++) {
            inst->buffer_info[i].length = 0;
        }
        return;
    }

    // All of the blocks from end on need to be shifted down
    // Find how many bytes need to be shifted down and shift block descriptors
    uint8_t keep_lenth = 0;
    unsigned int new_first = first;
    for (unsigned int i = end; i < RADIO_BLOCKS_PER_PACKET; i++) {
        if (inst->buffer_info[i].length == 0) {
            break;
        }
        keep_lenth += inst->buffer_info[i].length;

        inst->buffer_info[new_first] = inst->buffer_info[i];
        new_first++;

        inst->buffer_info[i].length = 0;
    }

    // Copy over the blocks in the buffer
    memmove(inst->packet_buffer + inst->buffer_info[first].offset,
            inst->packet_buffer + inst->buffer_info[end].offset, keep_lenth);
}

/**
 *  Remove any expired blocks from the packet buffer.
 *
 *  @param inst The instance for which blocks should be culled
 */
static void cull_blocks(struct radio_transport_desc *inst)
{
    for (unsigned int i = 0; i < RADIO_BLOCKS_PER_PACKET; i++) {
        const struct radio_trans_buff_blk_info *blk_info =
                                                        &inst->buffer_info[i];
        if (!blk_info->length) {
            break;
        }
        if ((millis - blk_info->enqueue_time) > blk_info->time_to_live) {
            // Block has expired, should be culled
            // Find the index of the next block that shouldn't be culled, this
            // way we can remove a section of consecutive blocks all at once if
            // there is one
            unsigned int end = i;
            for (; end < RADIO_BLOCKS_PER_PACKET; end++) {
                const struct radio_trans_buff_blk_info *bi =
                                                    &inst->buffer_info[end];
                if (!bi->length || ((millis - bi->enqueue_time) <=
                                    bi->time_to_live)) {
                    // This block is either invalid or hasn't expired, either
                    // way it shouldn't be shifted
                    break;
                }
            }
            // Remove the section of blocks
            remove_blocks(inst, i, end);

            // Decrement i because the next iteration of the loop should use the
            // same index as this iteration
            i--;
        }
    }
}

void radio_transport_service(struct radio_transport_desc *inst)
{
    // Run all of the radio and antenna manager services
    for (unsigned int i = 0; i < RADIO_MAX_NUM_RADIOS; i++) {
        if (inst->radios[i] == NULL) {
            break;
        }
        rn2483_service(&inst->radios[i]->rn2483);
        if (inst->radios[i]->antmgr != NULL) {
            radio_antmgr_service(inst->radios[i]);
        }
    }

    // Run channel manager service
    radio_chanmgr_service(inst);

    // If we have a transmission in progress, check its state
    if ((inst->tx_state == RADIO_TRANS_TX_IN_PROGRESS) ||
            (inst->tx_state == RADIO_TRANS_TX_CLEANUP)) {
        const enum rn2483_send_trans_state state = rn2483_get_send_state(
                                    inst->tx_radio, inst->tx_transaction_id);
        const int complete = ((state == RN2483_SEND_TRANS_DONE) ||
                              (state == RN2483_SEND_TRANS_FAILED));
        const int written = complete || (state == RN2483_SEND_TRANS_WRITTEN);

        if ((inst->tx_state == RADIO_TRANS_TX_IN_PROGRESS) && written) {
            // The buffer is no longer in use, we are free to start creating
            // the next packet
            if (inst->priority_tx_in_progress) {
                radio_packet_set_length(inst->priority_packet_buffer,
                                        RADIO_PACKET_HEADER_LENGTH);
                inst->priority_tx_in_progress = 0;
            } else {
                reset_packet_buffer(inst);
            }
            inst->tx_state = RADIO_TRANS_TX_CLEANUP;
        }

        if ((inst->tx_state == RADIO_TRANS_TX_CLEANUP) && complete) {
            // This transmission is totally complete now
            rn2483_clear_send_transaction(inst->tx_radio,
                                          inst->tx_transaction_id);
            inst->tx_state = RADIO_TRANS_TX_IDLE;
        }
    }

    // If we don't have a transmission in progress and the tx backoff time has
    // expired check if we need to start a new transmission or do packet buffer
    // housekeeping
    if (inst->tx_state == RADIO_TRANS_TX_IDLE) {
        // No transmission in progress
        const int transmit_backoff_expired = (millis - inst->last_tx_time) >
                                                        RADIO_TX_BACKOFF_TIME;
        int regular_packet_tx_started = 0;
        // Check if we have a priority packet to send now
        const uint8_t priority_len = radio_packet_length(
                                                inst->priority_packet_buffer);
        if (transmit_backoff_expired &&
                (priority_len > RADIO_PACKET_HEADER_LENGTH)) {
            // There is data to be sent in the priority buffer
            start_tx(inst, inst->priority_packet_buffer, priority_len);
            inst->priority_tx_in_progress = 1;
        } else if (transmit_backoff_expired) {
            const uint8_t packet_len = radio_packet_length(inst->packet_buffer);
            const int packet_has_data = packet_len > RADIO_PACKET_HEADER_LENGTH;
            // Check if we need to send the buffered packet now
            if (packet_has_data && (((millis - inst->buffer_slack_time_base) >
                                     inst->buffer_slack_time) ||
                                    (packet_len > RADIO_PACKET_WATERLINE))) {
                // There is data to be sent and slack time has expired or
                // waterline has been passed.
                // start sending packet
                start_tx(inst, inst->packet_buffer, packet_len);
                regular_packet_tx_started = 1;
            }
        }
        if (!regular_packet_tx_started) {
            // Cull any blocks that have exceeded their time to live
            cull_blocks(inst);
        }
    }
}

// MARK: TX

/**
 *  Update the slack time for a packet if the new slack time is shorter.
 *
 *  @param base Pointer to the base of the slack time to be updated
 *  @param slack_time Pointer the the slack time to be updated
 *  @param new_base The base of the new slack time
 *  @param new_slack_time The new slack time
 *  @param slack_time_valid Whether the slack time to be updated is valid
 */
static void update_slack_time(uint32_t *base, uint16_t *slack_time,
                              uint32_t new_base, uint16_t new_slack_time,
                              uint8_t slack_time_valid)
{
    if (!slack_time_valid) {
        *base = new_base;
        *slack_time = new_slack_time;
    }

    uint32_t low_base = *base;
    uint16_t low_slack_time = *slack_time;
    uint32_t high_base = new_base;
    uint16_t high_slack_time = new_slack_time;

    if (*base > new_base) {
        low_base = new_base;
        low_slack_time = new_slack_time;
        high_base = *base;
        high_slack_time = *slack_time;
    }

    uint32_t base_diff = high_base - low_base;

    if (((uint32_t)high_slack_time + base_diff) < low_slack_time) {
        *base = high_base;
        *slack_time = high_slack_time;
    } else {
        *base = low_base;
        *slack_time = low_slack_time;
    }
}

int radio_send_block(struct radio_transport_desc *inst, const uint8_t *block,
                     uint8_t block_length, uint16_t slack_time,
                     uint16_t time_to_live)
{
    // Find the index of the next free block descriptor
    struct radio_trans_buff_blk_info *block_desc = NULL;
    struct radio_trans_buff_blk_info *prev_block_desc = NULL;
    for (unsigned int i = 0; i < RADIO_BLOCKS_PER_PACKET; i++) {
        if (inst->buffer_info[i].length == 0) {
            block_desc = &inst->buffer_info[i];
            break;
        }
        prev_block_desc = &inst->buffer_info[i];
    }

    if (block_desc == NULL) {
        // There are no free block descriptors in the packet buffer
        return 1;
    }

    // Find the offset where the block should be placed in the buffer
    if (prev_block_desc == NULL) {
        // This is the first block in the buffer, it goes right after the header
        block_desc->offset = radio_packet_length(inst->packet_buffer);
    } else {
        // This block goes after the previous one
        block_desc->offset = prev_block_desc->offset + prev_block_desc->length;
    }

    // Check that there is enough space in the  buffer for the packet
    const uint8_t new_buffer_len = block_desc->offset + block_length;
    if (new_buffer_len > RADIO_MAX_PACKET_SIZE) {
        // There is not enough space in the buffer to queue the block
        return 1;
    }

    // Setup our block descriptor
    block_desc->enqueue_time = millis;
    block_desc->time_to_live = time_to_live;
    block_desc->length = block_length;

    // Check if we need to prebuffer this packet because a tx is ongoing
    block_desc->prebuffered = (!inst->priority_tx_in_progress &&
                               (inst->tx_state != RADIO_TRANS_TX_IDLE) &&
                               (inst->tx_state != RADIO_TRANS_TX_CLEANUP));

    // Update the buffered packet's slack time and update packet length if
    // needed
    if (block_desc->prebuffered) {
        update_slack_time(&inst->prebuffer_slack_time_base,
                          &inst->prebuffer_slack_time, block_desc->enqueue_time,
                          slack_time, inst->prebuffer_slack_time_valid);
        inst->prebuffer_slack_time_valid = 1;
    } else {
        update_slack_time(&inst->buffer_slack_time_base,
                          &inst->buffer_slack_time, block_desc->enqueue_time,
                          slack_time, inst->buffer_slack_time_valid);
        inst->buffer_slack_time_valid = 1;
        radio_packet_set_length(inst->packet_buffer, new_buffer_len);
    }

    // Copy block data into buffer
    memcpy(inst->packet_buffer + block_desc->offset, block, block_length);

    // Run the service function to start sending right away if possible
    radio_transport_service(inst);

    return 0;
}

int radio_send_block_priority(struct radio_transport_desc *inst,
                              const uint8_t *block, uint8_t block_length)
{
    // Make sure that the priority buffer isn't already in use
    if (inst->priority_tx_in_progress) {
        return 1;
    }

    // Make sure we have enough room in the priority buffer for the block
    const uint8_t len = radio_packet_length(inst->priority_packet_buffer);
    const uint8_t new_len = len + block_length;
    if (new_len > RADIO_PRIORITY_BUF_LENGTH) {
        // There is not enough room in the priority buffer for this block
        return 1;
    }

    // Update packet length
    radio_packet_set_length(inst->priority_packet_buffer, new_len);

    // Copy block data into the priority buffer
    memcpy(inst->priority_packet_buffer + len, block, block_length);

    // Run the service function to start sending right away if possible
    radio_transport_service(inst);

    return 0;
}


// MARK: RX

/**
 *  Get the antenna that is currently being used for a given radio.
 *
 *  @param radio The radio for which the antenna in use should be determined
 *
 *  @return The index of the antenna in use for the radio or zero if the radio
 *          does not have an antenna manager
 */
static inline uint8_t get_current_antenna(
                                        const struct radio_instance_desc *radio)
{
    if (radio->antmgr == NULL) {
        // No antenna manager for this radio
        return 0;
    } else {
        return radio_antmgr_get_current_antenna(radio);
    }
}

/**
 *  Get the radio descriptor for a given radio number.
 *
 *  @param inst The radio transport instance which control the radio
 *  @param radio_num The number of the radio for which the descriptor should be
 *                   found
 *
 *  @return A pointer to the radio instance descriptor for the radio with the
 *          given number or NULL if no such radio was found
 */
static inline struct radio_instance_desc *get_radio(
                                            struct radio_transport_desc *inst,
                                            uint8_t radio_num)
{
    struct radio_instance_desc *radio = NULL;
    for (struct radio_instance_desc *const *radio_p = inst->radios;
         *radio_p != NULL; radio_p++) {
        if ((*radio_p)->radio_num == radio_num) {
            radio = *radio_p;
            break;
        }
    }
    return radio;
}

/**
 *  Calculate RX loss given information from a signal report block.
 *
 *  @param remote_tx_power The TX power reported in the signal report
 *  @param rx_rssi The received signal strength for the packet containing the
 *                 signal report block
 *
 *  @return The RX loss for the packet containing the signal report block
 */
__attribute__((pure))
static inline int8_t calc_rx_loss(int8_t remote_tx_power, int8_t rx_rssi)
{
    // A signal report request block gives us enough information to estimate the
    // loss between the packet being sent and us receiving it. We call this
    // value our RX loss.
    // The link budget will look something like this:
    //      RX Power = TX Power + TX Ant Gain - Path Loss + RX Ant Gain
    // Since we do not know the antenna gains, our RX loss includes them, hence
    // the RX loss is as follows:
    //      RX Loss = Path Loss - (TX Ant Gain + RX Ant Gain)
    // When we receive a signal report request we know the power with which it
    // was sent and the power that we received it at (the RSSI of the received
    // packet). The RX Loss can be found as follows:
    //      RX Loss = remote_tx_power - rx_rssi
    return remote_tx_power - rx_rssi;
}

/**
 *  Calculate RX loss given information from a signal report response block.
 *
 *  @param tx_power The currently configured TX power
 *  @param remote_rssi The received signal strength reported in the signal
 *                     report response block
 *
 *  @return The TX loss for the signal report request that the signal report
 *          response block describes
 */
__attribute__((pure))
static inline int8_t calc_tx_loss(int8_t tx_power, int8_t remote_rssi)
{
    // A signal report response gives us enough information to estimate the
    // loss between us sending a packet and the receiver receiving it. We call
    // this value our TX loss.
    // The link budget will look something like this:
    //      RX Power = TX Power + TX Ant Gain - Path Loss + RX Ant Gain
    // Since we do not know the antenna gains, our TX loss includes them, hence
    // the TX loss is as follows:
    //      TX Loss = Path Loss - (TX Ant Gain + RX Ant Gain)
    // When we receive a signal report response we know how much transmit power
    // we are currently using and we know the received power at the other end.
    // The TX Loss can be found as follows:
    //      TX Loss = tx_power - remote_rssi
    return tx_power - remote_rssi;
}

/**
 *  Handle a received control block.
 *
 *  @param inst The radio transport instance
 *  @param radio The radio instance on which the block was received
 *  @param antenna_num The number of the antenna that the block was received on
 *  @param block The control block to be handled
 *  @param source The source address of the packet
 *  @param is_duplicate Whether this block is from a duplicate packet
 *  @param snr Signal to noise ratio for packet
 *  @param rssi Received signal strength indicator for packer
 */
static inline void handle_control(struct radio_transport_desc *inst,
                                  struct radio_instance_desc *radio,
                                  uint8_t antenna_num, const uint8_t *block,
                                  enum radio_packet_device_address source,
                                  int is_duplicate, int8_t snr, int8_t rssi)
{
    uint8_t subtype = radio_block_subtype(block);

    if (is_duplicate && (subtype != RADIO_CONTROL_BLOCK_SIGNAL_REPORT)) {
        // Ignore any duplicate blocks except for signal reports
        return;
    }

    switch (radio_block_subtype(block)) {
        case RADIO_CONTROL_BLOCK_SIGNAL_REPORT:
            ;
            // Parse signal report block
            int8_t remote_snr = radio_block_sig_report_snr(block);
            int8_t remote_rssi = radio_block_sig_report_rssi(block);
            uint8_t tx_radio_num = radio_block_sig_report_radio(block);
            int8_t remote_tx_power = radio_block_sig_report_tx_power(block);
            // Calculate RX loss on signal report block
            int8_t rx_loss = calc_rx_loss(remote_tx_power, rssi);

            // Call callback functions for RX power loss estimate
            radio_chanmgr_rx_loss_cb(inst, radio, antenna_num, rx_loss);
            if (radio->antmgr != NULL) {
                radio_antmgr_rx_loss_cb(inst, radio, antenna_num, rx_loss);
            }

            if (radio_block_sig_report_req(block)) {
                /* This is a request */
                // Send a reply
                uint8_t report[RADIO_BLOCK_SIG_REPORT_LENGHT];
                radio_block_marshal_header(report,
                                           RADIO_BLOCK_SIG_REPORT_LENGHT, 0,
                                           source, RADIO_BLOCK_TYPE_CONTROL,
                                           RADIO_CONTROL_BLOCK_SIGNAL_REPORT);
                radio_block_marshal_sig_report(report, snr, rssi, tx_radio_num,
                                               inst->radio_settings.power, 0);
                radio_send_block(inst, report, RADIO_BLOCK_SIG_REPORT_LENGHT,
                                 250, 0);
            } else {
                /* This is a reply */
                // Identify tx radio
                struct radio_instance_desc *tx_radio = get_radio(inst,
                                                                 tx_radio_num);
                if (tx_radio == NULL) {
                    // Could not find radio
                    return;
                }
                // Identify TX antenna
                uint8_t tx_antenna = radio_antmgr_get_current_antenna(tx_radio);
                // Calculate TX power loss
                int8_t tx_loss = calc_tx_loss(inst->radio_settings.power,
                                              remote_rssi);
                // Call callback functions for TX power loss estimate
                radio_chanmgr_tx_loss_cb(inst, tx_radio, tx_antenna, tx_loss,
                                         remote_snr);
                if (tx_radio->antmgr != NULL) {
                    radio_antmgr_tx_loss_cb(inst, tx_radio, tx_antenna, tx_loss,
                                            remote_snr);
                }
            }
            break;
        case RADIO_CONTROL_BLOCK_CMD_ACK:
            break;
        case RADIO_CONTROL_BLOCK_CMD_NONCE_REQ:
            break;
        case RADIO_CONTROL_BLOCK_CMD_NONCE:
            break;
        default:
            break;
    }
}

/**
 *  Handle a received command or data block.
 *
 *  @param inst The radio transport instance
 *  @param block The block to be handled
 *  @param is_duplicate Whether this block is from a duplicate packet
 *  @param snr Signal to noise ratio for packet
 *  @param rssi Received signal strength indicator for packer
 */
static inline void handle_block(struct radio_transport_desc *inst,
                                const uint8_t *block)
{
    switch (radio_block_type(block)) {
        case RADIO_BLOCK_TYPE_COMMAND:
            if (inst->command_callback != NULL) {
                inst->command_callback(block);
            }
            break;
        case RADIO_BLOCK_TYPE_DATA:
            if (inst->data_callback != NULL) {
                inst->data_callback(block);
            }
            break;
        default:
            // This should not be possible, block should already be validated
            break;
    }
}


static int radio_rx_callback(struct rn2483_desc_t *rn2483, void *context,
                             uint8_t *data, uint8_t length, int8_t snr,
                             int8_t rssi)
{
    // Unpack context pointer
    struct radio_transport_desc *inst = RADIO_RX_CB_CONTEXT_POINTER(context);
    uint8_t radio_num = RADIO_RX_CB_CONTEXT_NUM(context);

    // Find radio instance
    struct radio_instance_desc *radio = get_radio(inst, radio_num);
    if (__builtin_expect(radio == NULL, 0)) {
        // Could not find radio. This should never happen.
        return 0;
    }

    // Identify current antenna
    uint8_t antenna_num = get_current_antenna(radio);

    // Call metadata callbacks for channel and antenna managers
    radio_chanmgr_metadata_cb(inst, radio, antenna_num, snr, rssi);
    if (radio->antmgr != NULL) {
        radio_antmgr_metadata_cb(inst, radio, antenna_num, snr, rssi);
    }

    // Perform sanity check on packet
    int valid = radio_packet_sanity_check(data, length);

    // Call logging callback if one exists
    if (inst->logging_callback != NULL) {
        inst->logging_callback(data, length, radio_num, antenna_num, snr, rssi,
                               valid);
    }

    // Do not continue if packet did not pass sanity check
    if (!valid) {
        return 1;
    }

    // Record time and signal quality
    inst->last_rx_time = millis;

    // Call ground station callback if one is provided
    if (inst->ground_callback != NULL) {
        inst->ground_callback(data, length, radio_num, antenna_num, snr, rssi,
                               valid);
    }

    // Start parsing the packet
    uint8_t packet_length = radio_packet_length(data);

    // Do not continue if the packet does not have at least one block
    if (packet_length < (RADIO_PACKET_HEADER_LENGTH +
                         RADIO_BLOCK_HEADER_LENGTH)) {
        return 1;
    }

    // Check deduplication code
    int is_duplicate = 0;
    uint16_t dedup_code = radio_packet_deduplication_code(data);
    for (int i = 0; ((inst->dedup_codes_full &&
                      (i < RADIO_DEDUPLICATION_LIST_LENGTH)) ||
                     (i < inst->dedup_code_position)); i++) {
        if (dedup_code == inst->rx_deduplication_codes[i]) {
            // This packet is a duplicate
            is_duplicate = 1;
            break;
        }
    }

    // Record deduplication code
    if (!is_duplicate) {
        inst->rx_deduplication_codes[inst->dedup_code_position] = dedup_code;
        inst->dedup_code_position++;
        inst->dedup_codes_full |= (inst->dedup_code_position == 0);
    }

    // Handle blocks
    const uint8_t *block = radio_packet_fist_block(data);
    for (; block != NULL; block = radio_packet_next_block(data, block)) {
        if (!radio_block_sanity_check(data, block)) {
            // Block is not valid
            continue;
        }
        enum radio_packet_device_address address = radio_block_dest_addr(block);
        if ((address != inst->address) &&
            (address != RADIO_DEVICE_ADDRESS_MULTICAST)) {
            // Block is not for us
            continue;
        }

        if (radio_block_type(block) == RADIO_BLOCK_TYPE_CONTROL) {
            handle_control(inst, radio, antenna_num, block,
                           radio_packet_src_addr(data), is_duplicate, snr,
                           rssi);
        } else if (!is_duplicate) {
            handle_block(inst, block);
        }
    }

    // Continue receiving
    return 1;
}
