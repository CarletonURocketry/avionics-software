/**
 * @file radio-types.h
 * @desc Types used by radio stack.
 * @author Samuel Dewan
 * @date 2020-04-07
 * Last Author:
 * Last Edited On:
 */

#ifndef radio_transport_types_h
#define radio_transport_types_h

#include "rn2483.h"
#include "sky13414.h"
#include "radio-packet-layout.h"

/** Maximum number of radio instances */
#define RADIO_MAX_NUM_RADIOS        4
/** Maximum number of antennas per antenna switch */
#define RADIO_MAX_NUM_ANTENNAS      4

/** Maximum number of blocks to be queued in a single packet */
#define RADIO_BLOCKS_PER_PACKET     8
/** Length of priority buffer */
#define RADIO_PRIORITY_BUF_LENGTH   16

/** Number of deduplication code to be stored */
#define RADIO_DEDUPLICATION_LIST_LENGTH 4

//  MARK: Helpers
/**
 *  Update a moving average value.
 *  The pseudo average is weighted to give more recent values more precedence.
 *  The factor value indicates how quickly the average follows new data. A
 *  factor of 1 would result in the "average" simply being equal to the most
 *  recent value, larger factors mean that it takes longer for the average to
 *  catch up to changes in the data.
 *
 *  @param average Pointer to the average to be updated
 *  @param new_value The new value to be added to the average
 *  @param count The number of values in the data set so far, this only matters
 *               if the count is less than the factor
 *  @param factor Indicates how quickly the average should follow new data
 *
 *  @return An updated count value, this count value will not exceed the factor
 */
static inline uint8_t update_moving_average(int8_t *average, int8_t new_value,
                                            uint8_t count, uint8_t factor)
{
    const uint8_t new_count = count + 1;
    const uint8_t num = (new_count >= factor) ? factor : new_count;
    *average += ((new_value - *average) + (num / 2)) / num;
    return (count < factor) ? new_count : count;
}

//  MARK: Forward Decelerations
struct radio_instance_desc;
struct radio_transport_desc;

//  MARK: Callback Types

/** Callback function for a received packet. */
typedef void (*radio_rx_packet_cb)(const uint8_t *packet, uint8_t length,
                                   uint8_t radio_num, uint8_t antenna_num,
                                   int8_t snr, int8_t rssi, int valid);
/** Callback function for a received block. */
typedef void (*radio_rx_block_cb)(const uint8_t *block);

//  MARK: Antenna Manager

/**
 *  Descriptor for instance of radio antenna manager.
 */
struct radio_antmgr_desc {
    /** Antenna switch driver instance */
    struct sky13414_desc_t antenna_switch;
    /** The a weighted average of the RX power loss for each antenna */
    int8_t avg_rx_power_loss[RADIO_MAX_NUM_ANTENNAS];
    /** The a weighted average of the TX power loss for each antenna */
    int8_t avg_tx_power_loss[RADIO_MAX_NUM_ANTENNAS];
    /** The counts for the number of samples in the weighted average for each
        antenna's RX power loss */
    uint8_t rx_power_loss_counts;
    /** The counts for the number of samples in the weighted average for each
        antenna's TX power loss */
    uint8_t tx_power_loss_counts;
    /** A mask that indicates which antennas are in use */
    uint8_t antenna_mask:RADIO_MAX_NUM_ANTENNAS;
};

/**
 *  Description of the antenna configuration for a radio.
 */
struct radio_antenna_info {
    /** Pointer to the antmgr structure for a radio */
    struct radio_antmgr_desc *antmgr;
    /** Mask that indicates which antennas are in use */
    uint8_t antenna_mask:RADIO_MAX_NUM_ANTENNAS;
    /** Value that indicates which fixed antenna is in use, 0 if the antenna
        should be dynamically selected */
    uint8_t fixed_antenna_num:3;
};

//  MARK: Per Radio Data

/** Describes the role that a device will take when searching for another
    device. */
enum radio_search_role {
    /** Device will transmit advertising messages and slowly hop channels */
    RADIO_SEARCH_ROLE_ADVERTISE,
    /** Device will listen for advertising messages and quickly hop channels */
    RADIO_SEARCH_ROLE_LISTEN
};

/**
 *  Descriptor that contains all of the information related to a particular
 *  radio.
 */
struct radio_instance_desc {
    /** Information for RN2483 radio driver */
    struct rn2483_desc_t rn2483;
    /** Pointer to an antenna manager for this radio, NULL if this radio does
        not require dynamic antenna selection */
    struct radio_antmgr_desc *antmgr;

    /** A weighted average of the power loss experienced by received packets */
    int8_t avg_rx_power_loss;
    /** A weighted average of the power loss experienced by transmitted
        packets */
    int8_t avg_tx_power_loss;
    /** The SNR value for the last received packet */
    int8_t last_rx_snr;
    /** The RSSI value for the last received packet */
    int8_t last_rx_rssi;

    /** The number of samples used in the average RX power loss value */
    uint8_t rx_power_loss_count:3;
    /** The number of samples used in the average TX power loss value */
    uint8_t tx_power_loss_count:3;
    /** User facing number for this radio (this is radio number from the
        configuration file) */
    uint8_t radio_num:2;
    
};


//  MARK: Per Transport Instance Data

/** Information about a block in the radio transport buffer. */
struct radio_trans_buff_blk_info {
    /** The time at which the block was added to the queue */
    uint32_t enqueue_time;
    /** The number of milliseconds the block can sit in the buffer before it is
        stale and should no longer be sent */
    uint16_t time_to_live;
    /** The offset of the block data in the buffer */
    uint8_t offset;
    /** The size of the block in the buffer */
    uint8_t length:7;
    /** Indicates whether this block has been stored in the buffer while the
        previous packet was still in the process of being sent */
    uint8_t prebuffered:1;
};

/** State of the radio transports transmit capability. */
enum radio_trans_tx_state {
    /** No transmission is in progress */
    RADIO_TRANS_TX_IDLE,
    /** A transmission is in progress */
    RADIO_TRANS_TX_IN_PROGRESS,
    /** A transmission has been completed by the driver, but the radio transport
        code has not yet clean up all of the state from the transmission */
    RADIO_TRANS_TX_CLEANUP
};

/**
 *  Descriptor for instance of radio transport layer driver.
 */
struct radio_transport_desc {
    /** Array of radio descriptors */
    struct radio_instance_desc *const *radios;

    /** Callback function that can be used to log packets as they are
        received */
    radio_rx_packet_cb logging_callback;
    /** Callback function used to provide ground station service with all
        received packets */
    radio_rx_packet_cb ground_callback;

    /** Callback for handler of data blocks */
    radio_rx_block_cb data_callback;
    /** Callback for handler of command blocks */
    radio_rx_block_cb command_callback;

    /** The last time at which a packet was sent */
    uint32_t last_tx_time;
    /** The last time at which a packet was received */
    uint32_t last_rx_time;
    /** The last time at which a signal report was sent */
    uint32_t last_sig_report_time;

    /** The radio to be used for the current transmission */
    struct rn2483_desc_t *tx_radio;

    /** Buffer that can be used to send a single high priority block without
        needing to queue it normally */
    uint8_t priority_packet_buffer[RADIO_PRIORITY_BUF_LENGTH];
    /** Descriptions of blocks currently in buffered packet */
    struct radio_trans_buff_blk_info buffer_info[RADIO_BLOCKS_PER_PACKET];
    /** The base time that the buffer slack time is an offset from */
    uint32_t buffer_slack_time_base;
    /** The base time that the prebuffer slack time is an offset from */
    uint32_t prebuffer_slack_time_base;
    /** The duration from the buffer slack time base at which point the buffer
        should be sent */
    uint16_t buffer_slack_time;
    /** Slack time for prebuffered blocks */
    uint16_t prebuffer_slack_time;
    /** Buffer in which packets are formed */
    uint8_t packet_buffer[RADIO_MAX_PACKET_SIZE];

    /** Settings for radios */
    struct rn2483_lora_settings_t radio_settings;

    /** Received deduplication codes */
    uint16_t rx_deduplication_codes[RADIO_DEDUPLICATION_LIST_LENGTH];

    /** The radio driver transaction id for the current transmission */
    uint8_t tx_transaction_id;

    /** The current packet deduplication number */
    uint16_t packet_number:12;
    /** The device address that this device should use */
    enum radio_packet_device_address address:4;
    /** Current transmission state */
    enum radio_trans_tx_state tx_state: 2;
    /** The role that this device should take in search mode */
    enum radio_search_role search_role:1;
    /** The index where the next deduplication code should be inserted in the
        deduplication code buffer */
    uint8_t dedup_code_position:2;
    /** Indicates whether the deduplication code buffer has empty slots */
    uint8_t dedup_codes_full:1;
    /** Indicates whether there is a transmission in progress from the priority
        buffer */
    uint8_t priority_tx_in_progress:1;
    /** Indicates whether the a slack time has been set for the TX buffer */
    uint8_t buffer_slack_time_valid:1;
    /** Indicates whether the a slack time has been set for the prebuffered
        packets in the TX buffer */
    uint8_t prebuffer_slack_time_valid:1;
};


#endif /* radio_transport_types_h */
