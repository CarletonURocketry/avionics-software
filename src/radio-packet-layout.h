/**
 * @file radio-packet-layout.h
 * @desc Helpers for parsing the radio packet format
 * @author Samuel Dewan
 * @date 2020-03-24
 * Last Author:
 * Last Edited On:
 */

#ifndef radio_packet_layout_h
#define radio_packet_layout_h

#include <stdint.h>
#include <string.h>
#define RADIO_SUPPORTED_FORMAT_VERSION  0


/** Minimum amount of time between transmissions */
#define RADIO_TX_BACKOFF_TIME   200


// MARK: Packet Header

//
//
//  Packet Header Constants
//
//

#define RADIO_MAX_PACKET_SIZE           128
#define RADIO_PACKET_HEADER_LENGTH      12
#define RADIO_PACKET_CALLSIGN_LENGTH    6

enum radio_packet_device_address {
    RADIO_DEVICE_ADDRESS_GROUND_STATION = 0x0,
    RADIO_DEVICE_ADDRESS_ROCKET = 0x1,
    RADIO_DEVICE_ADDRESS_MULTICAST = 0xF
};


//
//
//  Packet Header Functions
//
//

/**
 *  Get the callsign from a radio packet.
 *
 *  @note This function does not make any attempt to validate the packet that
 *        is passed to it. At a minimum the caller should check that the packet
 *        is long enough to contain a valid header before calling this function.
 *
 *  @param packet The packet from which the callsign should be parsed.
 *  @param buffer Buffer in which the callsign should be placed, must be at
 *                least 7 bytes long (6 byte callsign and null character)
 *
 *  @return The buffer
 */
static inline char *radio_packet_callsign(const uint8_t *packet, char *buffer)
{
    memcpy(buffer, packet, RADIO_PACKET_CALLSIGN_LENGTH);
    buffer[RADIO_PACKET_CALLSIGN_LENGTH] = '\0';
    return buffer;
}

/**
 *  Get the length from a radio packet.
 *
 *  @note This function does not make any attempt to validate the packet that
 *        is passed to it. At a minimum the caller should check that the packet
 *        is long enough to contain a valid header before calling this function.
 *
 *  @param packet The packet from which the length should be parsed
 *
 *  @return The length of the packet
 */
__attribute__((const))
static inline uint8_t radio_packet_length(const uint8_t *packet)
{
    uint8_t len = packet[RADIO_PACKET_CALLSIGN_LENGTH] & 0x3f;
    return (len + 1) * 4;
}

/**
 *  Get the format version from a radio packet.
 *
 *  @note This function does not make any attempt to validate the packet that
 *        is passed to it. At a minimum the caller should check that the packet
 *        is long enough to contain a valid header before calling this function.
 *
 *  @param packet The packet from which the format version should be parsed
 *
 *  @return The format version of the packet
 */
__attribute__((const))
static inline uint8_t radio_packet_format_version(const uint8_t *packet)
{
    uint8_t low = packet[RADIO_PACKET_CALLSIGN_LENGTH] >> 6;
    uint8_t high = (packet[RADIO_PACKET_CALLSIGN_LENGTH + 1] & 0x7) >> 2;
    return low | high;
}

/**
 *  Get the source address from a radio packet.
 *
 *  @note This function does not make any attempt to validate the packet that
 *        is passed to it. At a minimum the caller should check that the packet
 *        is long enough to contain a valid header before calling this function.
 *
 *  @param packet The packet from which the source address should be parsed
 *
 *  @return The source address of the packet
 */
__attribute__((const))
static inline enum radio_packet_device_address radio_packet_src_addr(
                                                        const uint8_t *packet)
{
    uint8_t addr = packet[RADIO_PACKET_CALLSIGN_LENGTH + 2] & 0xf;
    return (enum radio_packet_device_address)(addr);
}

/**
 *  Get the packet number from a radio packet.
 *
 *  @note This function does not make any attempt to validate the packet that
 *        is passed to it. At a minimum the caller should check that the packet
 *        is long enough to contain a valid header before calling this function.
 *
 *  @param packet The packet from which the packet number should be parsed
 *
 *  @return The packet number of the packet
 */
__attribute__((const))
static inline uint16_t radio_packet_number(const uint8_t *packet)
{
    uint16_t low = (packet[RADIO_PACKET_CALLSIGN_LENGTH + 2] & 0xf0) >> 4;
    uint16_t high = ((uint16_t)packet[RADIO_PACKET_CALLSIGN_LENGTH + 3]) << 4;
    return low | high;
}

/**
 *  Get the deduplication code from a radio packet. The deduplication code is
 *  a combination of the source address and packet number.
 *
 *  @note This function does not make any attempt to validate the packet that
 *        is passed to it. At a minimum the caller should check that the packet
 *        is long enough to contain a valid header before calling this function.
 *
 *  @param packet The packet from which the deduplication code should be parsed
 *
 *  @return The deduplication code of the packet
 */
__attribute__((const))
static inline uint16_t radio_packet_deduplication_code(const uint8_t *packet)
{
    uint16_t low = ((uint16_t)packet[RADIO_PACKET_CALLSIGN_LENGTH + 2]);
    uint16_t high = ((uint16_t)packet[RADIO_PACKET_CALLSIGN_LENGTH + 3]) << 8;
    return low | high;
}

/**
 *  Marshal a packet header.
 *
 *  @param packet Buffer in which header should be created
 *  @param callsign The callsign to be placed in the header
 *  @param format_version Format version to be placed in the header
 *  @param source_address Source address to be placed in the header
 *  @param packet_number Packet number to be placed in the header
 *  @param packet_length Length of packet to be placed in the header (including
 *                       length of header)
 *
 *  @return The packet buffer
 */
static inline uint8_t *radio_packet_marshal_header(uint8_t *packet,
                                const char *callsign, uint8_t format_version,
                                enum radio_packet_device_address source_address,
                                uint16_t packet_number, uint8_t packet_length)
{
    memcpy(packet, callsign, RADIO_PACKET_CALLSIGN_LENGTH);
    uint8_t version_low = (format_version & 0x3) << 6;
    uint8_t version_high = (format_version & 0x1c) >> 2;
    packet_length = (((packet_length / 4) - 1) & 0x3f);
    packet[RADIO_PACKET_CALLSIGN_LENGTH] = packet_length | version_low;
    packet[RADIO_PACKET_CALLSIGN_LENGTH + 1] = version_high;
    packet[RADIO_PACKET_CALLSIGN_LENGTH + 2] = (source_address & 0xf) << 2;
    return packet;
}

/**
 *  Update the length field in a radio packet header.
 *
 *  @param packet Pointer to beginning of header
 *  @param length New length for packet
 */
static inline void radio_packet_set_length(uint8_t *packet, uint8_t length)
{
    packet[RADIO_PACKET_CALLSIGN_LENGTH] &= ~(0x3f);
    packet[RADIO_PACKET_CALLSIGN_LENGTH] |= (((length / 4) - 1) & 0x3f);
}

/**
 *  Update the packet number in a radio packet header.
 *
 *  @param packet Pointer to beginning of header
 *  @param number New packet number for packet
 */
static inline void radio_packet_set_number(uint8_t *packet, uint16_t number)
{
    packet[RADIO_PACKET_CALLSIGN_LENGTH + 2] &= ~(0xf0);
    packet[RADIO_PACKET_CALLSIGN_LENGTH + 2] |= (number & 0xf) << 4;
    packet[RADIO_PACKET_CALLSIGN_LENGTH + 3] = (uint8_t)(number >> 4);
}

/**
 *  Perform sanity checks on received packet.
 *
 *  @param packet The packet on which sanity checks should be performed
 *  @param bytes_received The number of bytes that where received for the
 *                        packet
 *
 *  @return Zero if checks fail, a non-zero value otherwise
 */
__attribute__((const))
static inline int radio_packet_sanity_check(uint8_t *packet,
                                            uint8_t bytes_received)
{
    // Check that enough bytes where received to contain a header
    if (bytes_received < RADIO_PACKET_HEADER_LENGTH) {
        return 0;
    }
    // Check that packet's length matches the number of bytes received
    if (bytes_received != radio_packet_length(packet)) {
        return 0;
    }
    // Check that the packet version is something that we can parse
    if (RADIO_SUPPORTED_FORMAT_VERSION != radio_packet_format_version(packet)) {
        return 0;
    }

    return 1;
}


// MARK: Block Header

//
//
//  Block Header Constants
//
//

#define RADIO_MAX_BLOCK_SIZE            128
#define RADIO_BLOCK_HEADER_LENGTH       4

/**
 *  Possible types for radio packet blocks.
 */
enum radio_block_type {
    RADIO_BLOCK_TYPE_CONTROL = 0x0,
    RADIO_BLOCK_TYPE_COMMAND = 0x1,
    RADIO_BLOCK_TYPE_DATA = 0x2
};

/**
 *  Possible subtypes for radio packet control blocks.
 */
enum radio_block_control_subtype {
    RADIO_CONTROL_BLOCK_SIGNAL_REPORT = 0x00,
    RADIO_CONTROL_BLOCK_CMD_ACK = 0x01,
    RADIO_CONTROL_BLOCK_CMD_NONCE_REQ = 0x02,
    RADIO_CONTROL_BLOCK_CMD_NONCE = 0x03,
    RADIO_CONTROL_BLOCK_BEACON = 0x04,
    RADIO_CONTROL_BLOCK_BEACON_RSP = 0x05
};

#define RADIO_CONTROL_BLOCK_NUM_SUBTYPES    6

/**
 *  Possible subtypes for radio packet command blocks.
 */
enum radio_block_command_subtype {
    RADIO_COMMAND_BLOCK_RESET = 0x00,
    RADIO_COMMAND_BLOCK_REQUEST = 0x01,
    RADIO_COMMAND_BLOCK_DEPLOY = 0x02,
    RADIO_COMMAND_BLOCK_TARE = 0x03
};

#define RADIO_COMMAND_BLOCK_NUM_SUBTYPES    4

/**
 *  Possible subtypes for radio packet data blocks.
 */
enum radio_block_data_subtype {
    RADIO_DATA_BLOCK_DEBUG = 0x00,
    RADIO_DATA_BLOCK_STATUS = 0x01,
    RADIO_DATA_BLOCK_STARTUP = 0x02,
    RADIO_DATA_BLOCK_ALTITUDE = 0x03,
    RADIO_DATA_BLOCK_ACCELERATION = 0x04,
    RADIO_DATA_BLOCK_ANGULAR_VELOCITY = 0x05,
    RADIO_DATA_BLOCK_GNSS = 0x06,
    RADIO_DATA_BLOCK_GNSS_META = 0x07,
    RADIO_DATA_BLOCK_POWER = 0x08,
    RADIO_DATA_BLOCK_TEMPERATURE = 0x09,
    RADIO_DATA_BLOCK_MPU9250_IMU = 0x0a,
    RADIO_DATA_BLOCK_KX134_1211_ACCEL = 0x0b
};

#define RADIO_DATA_BLOCK_NUM_SUBTYPES    9

//
//
//  Block Header Functions
//
//

/**
 *  Get the length of a radio block.
 *
 *  @param block Pointer to header of the block
 *
 *  @return The length of the block
 */
__attribute__((const))
static inline uint8_t radio_block_length(const uint8_t *block)
{
    return ((block[0] & 0x1f) + 1) * 4;
}

/**
 *  Check whether a block is signed.
 *
 *  @param block Pointer to header of the block
 *
 *  @return zero if the block is not signed, a non-zero value if it is
 */
__attribute__((const))
static inline int radio_block_has_signature(const uint8_t *block)
{
    return block[0] & (1 << 5);
}

/**
 *  Get a block's type.
 *
 *  @param block Pointer to header of the block
 *
 *  @return The type of the block
 */
__attribute__((const))
static inline enum radio_block_type radio_block_type(const uint8_t *block)
{
    uint8_t low = (block[0] & 0xc0) >> 6;
    uint8_t high = (block[1] & 0x3) << 2;
    return (enum radio_block_type)(low | high);
}

/**
 *  Get a block's subtype.
 *
 *  @param block Pointer to header of the block
 *
 *  @return The subtype of the block
 */
__attribute__((const))
static inline uint8_t radio_block_subtype(const uint8_t *block)
{
    return (block[1] & 0xfc) >> 2;
}

/**
 *  Get a block's destination address.
 *
 *  @param block Pointer to header of the block
 *
 *  @return The block's destination address
 */
__attribute__((const))
static inline enum radio_packet_device_address radio_block_dest_addr(
                                                        const uint8_t *block)
{
    return (enum radio_packet_device_address)(block[2] & 0xf);
}

/**
 *  Get the payload from a block.
 *
 *  @param block Pointer to header of the block
 *
 *  @return Pointer to the payload of the block
 */
__attribute__((const))
static inline const uint8_t *radio_block_payload(const uint8_t *block)
{
    return block + RADIO_BLOCK_HEADER_LENGTH;
}

/**
 *  Marshal a black header.
 *
 *  @param block Pointer to buffer in which block header should be created
 *  @param length The length of the block (including size of header)
 *  @param has_sig Whether the block will be signed
 *  @param type The type of the block
 *  @param subtype The subtype of the block
 */
static inline uint8_t *radio_block_marshal_header(uint8_t *block,
                                        uint8_t length, int has_sig,
                                        enum radio_packet_device_address dest,
                                        enum radio_block_type type,
                                        uint8_t subtype)
{
    uint8_t type_low = (type & 0x3) << 6;
    uint8_t type_high = (type & 0xc) >> 2;
    block[0] = (((length / 4) - 1) & 0x1f) | ((!!has_sig) << 5) | type_low;
    block[1] = type_high | ((subtype & 0x3f) << 2);
    block[2] = dest & 0xf;
    return block;
}

//
//
//  Block Iteration Functions
//
//

/**
 *  Get the first block from a packet.
 *
 *  @param packet The packet from which the first block should be returned
 *
 *  @return A pointer to the packet's first block or NULL if the packet does not
 *          have any blocks
 */
__attribute__((const))
static inline const uint8_t *radio_packet_fist_block(const uint8_t *packet)
{
    // Check that there is actually enough data do have a first block
    if (radio_packet_length(packet) < (RADIO_PACKET_HEADER_LENGTH +
                                       RADIO_BLOCK_HEADER_LENGTH)) {
        return NULL;
    }
    // If there is enough data, the first block header starts right after the
    // packet header
    return packet + RADIO_PACKET_HEADER_LENGTH;
}

/**
 *  Get the next block from a packet.
 *
 *  @param packet The packet from which the next block should be returned
 *  @param current The current block in the packet (the block after this block
 *                 will be returned)
 *
 *  @return A pointer to the packet's next block or NULL if current is the last
 *          block in the packet or if the current block is invalid in such a way
 *          that the next packet cannot be found
 */
__attribute__((const))
static inline const uint8_t *radio_packet_next_block(const uint8_t *packet,
                                                     const uint8_t *current)
{
    // Get length of current block
    uint8_t curr_len = radio_block_length(current);
    // Find offset of next block
    const uint8_t *next = current + curr_len;
    // Check that the packet is big enough to have a block header at the address
    // of the next block
    if (radio_packet_length(packet) < (((uintptr_t)next - (uintptr_t)packet) +
                                       RADIO_BLOCK_HEADER_LENGTH)) {
        // The current block must be the last block
        return NULL;
    }

    return next;
}

/**
 *  Perform sanity checks on received block.
 *
 *  @param packet The packet that contains the block
 *  @param block The block on which sanity checks should be performed
 *
 *  @return Zero if checks fail, a non-zero value otherwise
 */
__attribute__((const))
static inline int radio_block_sanity_check(const uint8_t *packet,
                                           const uint8_t *block)
{
    // Check that the packet is big enough to contain the whole bloc
    if (radio_packet_length(packet) < (((uintptr_t)block - (uintptr_t)packet) +
                                       radio_block_length(block))) {
        return 0;
    }
    // Check that type and subtype are valid
    switch (radio_block_type(block)) {
        case RADIO_BLOCK_TYPE_CONTROL:
            if (radio_block_subtype(block) >=
                    RADIO_CONTROL_BLOCK_NUM_SUBTYPES) {
                // Unknown subtype of control block
                return 0;
            }
            break;
        case RADIO_BLOCK_TYPE_COMMAND:
            if (radio_block_subtype(block) >=
                    RADIO_COMMAND_BLOCK_NUM_SUBTYPES) {
                // Unknown subtype of command block
                return 0;
            }
            break;
        case RADIO_BLOCK_TYPE_DATA:
            if (radio_block_subtype(block) >=
                    RADIO_DATA_BLOCK_NUM_SUBTYPES) {
                // Unknown subtype of data block
                return 0;
            }
            break;
        default:
            // Unknown type
            return 0;
    }
    return 1;
}



#endif /* radio_packet_layout_h */
