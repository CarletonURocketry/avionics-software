/**
 * @file sdspi-states.c
 * @desc SD Card via SPI state machine handlers.
 * @author Samuel Dewan
 * @date 2021-05-28
 * Last Author:
 * Last Edited On:
 */

#include "sdspi-states.h"

#include <string.h>

#include "board.h"

#ifdef ENABLE_SDSPI

#ifndef SDSPI_USE_CRC
#define SDSPI_USE_CRC 0
#endif

#define SDSPI_INSERT_GLITCH_FILTER_TIME    2
#define SDSPI_BUSY_CHECK_BYTES  1
#define SDSPI_CMD_READ_LENGTH   8
#define SDSPI_NUM_INIT_RETRIES  5

#define SDSPI_CMD_TIMEOUT           MS_TO_MILLIS(50)
#define SDSPI_BLK_READ_TIMEOUT      MS_TO_MILLIS(250)
#define SDSPI_WRITE_BUSY_TIMEOUT    MS_TO_MILLIS(250)
#define SDSPI_WRITE_RSP_TIMEOUT     MS_TO_MILLIS(10)


static uint8_t sdspi_crc_7(uint8_t const *msg, size_t length)
{
    static uint8_t const sdspi_crc7_lut[] = {
        0x00, 0x09, 0x12, 0x1B, 0x24, 0x2D, 0x36, 0x3F,
        0x48, 0x41, 0x5A, 0x53, 0x6C, 0x65, 0x7E, 0x77,
        0x19, 0x10, 0x0B, 0x02, 0x3D, 0x34, 0x2F, 0x26,
        0x51, 0x58, 0x43, 0x4A, 0x75, 0x7C, 0x67, 0x6E,
        0x32, 0x3B, 0x20, 0x29, 0x16, 0x1F, 0x04, 0x0D,
        0x7A, 0x73, 0x68, 0x61, 0x5E, 0x57, 0x4C, 0x45,
        0x2B, 0x22, 0x39, 0x30, 0x0F, 0x06, 0x1D, 0x14,
        0x63, 0x6A, 0x71, 0x78, 0x47, 0x4E, 0x55, 0x5C,
        0x64, 0x6D, 0x76, 0x7F, 0x40, 0x49, 0x52, 0x5B,
        0x2C, 0x25, 0x3E, 0x37, 0x08, 0x01, 0x1A, 0x13,
        0x7D, 0x74, 0x6F, 0x66, 0x59, 0x50, 0x4B, 0x42,
        0x35, 0x3C, 0x27, 0x2E, 0x11, 0x18, 0x03, 0x0A,
        0x56, 0x5F, 0x44, 0x4D, 0x72, 0x7B, 0x60, 0x69,
        0x1E, 0x17, 0x0C, 0x05, 0x3A, 0x33, 0x28, 0x21,
        0x4F, 0x46, 0x5D, 0x54, 0x6B, 0x62, 0x79, 0x70,
        0x07, 0x0E, 0x15, 0x1C, 0x23, 0x2A, 0x31, 0x38,
        0x41, 0x48, 0x53, 0x5A, 0x65, 0x6C, 0x77, 0x7E,
        0x09, 0x00, 0x1B, 0x12, 0x2D, 0x24, 0x3F, 0x36,
        0x58, 0x51, 0x4A, 0x43, 0x7C, 0x75, 0x6E, 0x67,
        0x10, 0x19, 0x02, 0x0B, 0x34, 0x3D, 0x26, 0x2F,
        0x73, 0x7A, 0x61, 0x68, 0x57, 0x5E, 0x45, 0x4C,
        0x3B, 0x32, 0x29, 0x20, 0x1F, 0x16, 0x0D, 0x04,
        0x6A, 0x63, 0x78, 0x71, 0x4E, 0x47, 0x5C, 0x55,
        0x22, 0x2B, 0x30, 0x39, 0x06, 0x0F, 0x14, 0x1D,
        0x25, 0x2C, 0x37, 0x3E, 0x01, 0x08, 0x13, 0x1A,
        0x6D, 0x64, 0x7F, 0x76, 0x49, 0x40, 0x5B, 0x52,
        0x3C, 0x35, 0x2E, 0x27, 0x18, 0x11, 0x0A, 0x03,
        0x74, 0x7D, 0x66, 0x6F, 0x50, 0x59, 0x42, 0x4B,
        0x17, 0x1E, 0x05, 0x0C, 0x33, 0x3A, 0x21, 0x28,
        0x5F, 0x56, 0x4D, 0x44, 0x7B, 0x72, 0x69, 0x60,
        0x0E, 0x07, 0x1C, 0x15, 0x2A, 0x23, 0x38, 0x31,
        0x46, 0x4F, 0x54, 0x5D, 0x62, 0x6B, 0x70, 0x79
    };

    uint8_t crc = 0;
    for (size_t i = 0; i < length; i++) {
        uint8_t const pos = ((crc << 1) ^ msg[i]);
        crc = sdspi_crc7_lut[pos];
    }

    return crc;
}

#if SDSPI_USE_CRC == 1
static uint16_t sdspi_crc_16(uint8_t const *msg, size_t length)
{
    // CRC: width=16 poly=0x1021 init=0x0000 refin=false refout=false
    //      xorout=0x0000

    static uint16_t const sdspi_crc16_lut[] = {
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
        0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
        0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
        0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
        0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
        0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
        0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
        0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
        0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
        0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
        0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
        0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
        0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
        0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
        0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
        0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
        0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
        0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
        0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
        0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
        0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
        0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
        0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
        0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
        0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
        0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
        0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
        0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
        0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
    };

    uint16_t crc = 0;
    for (size_t i = 0; i < length; i++) {
        uint8_t const pos = (uint8_t)((crc >> 8) ^ msg[i]);
        crc = (uint16_t)((crc << 8) ^ sdspi_crc16_lut[pos]);
    }

    return crc;
}
#endif


static inline int sdspi_init_retry(struct sdspi_desc_t *inst)
{
    inst->init_retry_count++;

    if (inst->init_retry_count > SDSPI_NUM_INIT_RETRIES) {
        inst->state = SDSPI_TOO_MANY_INIT_RETRIES;
        return 0;
    }

    // Retry right away
    return 1;
}

static inline void sdspi_end_spi_session(struct sdspi_desc_t *inst)
{
    if (!inst->spi_session_open) {
        return;
    }

    sercom_spi_end_session(inst->spi_inst, inst->spi_tid);
    inst->spi_session_open = 0;

    allow_sleep();
}


enum sdspi_cmd_substate {
    SDSPI_CMD_OPEN_SESSION = 0,
    SDSPI_CMD_CHECK_BUSY,
    SDSPI_CMD_CHECK_BUSY_RSP,
    SDSPI_CMD_MARSHAL,
    SDSPI_CMD_SEND,
    SDSPI_CMD_GET_RSP,
    SDSPI_CMD_INCOMPLETE_RSP
};


/** Describes the result of the sdspi_handle_cmd_state function */
enum sdspi_cmd_state_result {
    SDSPI_CMD_RES_DONE,
    SDSPI_CMD_RES_AGAIN,
    SDSPI_CMD_RES_BUSY_WAIT,
    SDSPI_CMD_RES_QUEUE_WAIT,
    SDSPI_CMD_RES_IN_PROGRESS,
    SDSPI_CMD_RES_TIMEOUT,
    SDSPI_CMD_RES_FAILED
};

static enum sdspi_cmd_state_result sdspi_handle_cmd_state(
                                                    struct sdspi_desc_t *inst,
                                                    uint8_t cmd_index,
                                                    uint32_t arg,
                                                    uint32_t baudrate,
                                                    uint8_t rsp_len,
                                                    uint8_t busy_check,
                                                    uint8_t end_session,
                                                    uint8_t start_session)
{
    union sdspi_command *const cmd = (union sdspi_command*)inst->cmd_buffer;
    uint8_t ret;

    switch (inst->substate) {
        case SDSPI_CMD_OPEN_SESSION:
            inst->cmd_start_time = millis;

            if (!start_session) {
                inst->substate = busy_check ? SDSPI_CMD_CHECK_BUSY :
                                              SDSPI_CMD_MARSHAL;
                return SDSPI_CMD_RES_AGAIN;
            }

            ret = sercom_spi_start_session(inst->spi_inst, &inst->spi_tid,
                                           baudrate, inst->cs_pin_group,
                                           inst->cs_pin_mask);

            if (ret != 0) {
                // Session was not queued succesfully
                inst->substate = 0;
                return SDSPI_CMD_RES_FAILED;
            }

            inst->spi_session_open = 1;
            // Prevent sleeping while we are in a session
            inhibit_sleep();

            if (!busy_check) {
                inst->substate = SDSPI_CMD_MARSHAL;
                return SDSPI_CMD_RES_AGAIN;
            } else {
                inst->substate = SDSPI_CMD_CHECK_BUSY;
            }

            /* fallthrough */
        case SDSPI_CMD_CHECK_BUSY:
            ret = sercom_spi_start_session_transaction(inst->spi_inst,
                                                       inst->spi_tid,
                                                       NULL, 0,
                                                       inst->cmd_buffer,
                                                       SDSPI_BUSY_CHECK_BYTES);
            if (ret != 0) {
                // Could not queue transaction in session
                return SDSPI_CMD_RES_QUEUE_WAIT;
            }

            inst->spi_in_progress = 1;
            inst->substate = SDSPI_CMD_CHECK_BUSY_RSP;

            return SDSPI_CMD_RES_IN_PROGRESS;
        case SDSPI_CMD_CHECK_BUSY_RSP:
            // If we make it here the SPI transaction is done
            inst->spi_in_progress = 0;

            // The SD card will hold the MISO line low if it is busy. Check to
            // make sure that the last bit we read from the card is high, we
            // don't care about any of the other bits, only the most recent.
            if (!(inst->cmd_buffer[SDSPI_BUSY_CHECK_BYTES - 1] & (1 << 0))) {
                // Card is busy

                // Check if we have timed out
                if ((millis - inst->cmd_start_time) > SDSPI_CMD_TIMEOUT) {
                    sdspi_end_spi_session(inst);
                    inst->substate = 0;
                    return SDSPI_CMD_RES_TIMEOUT;
                }

                // Check again
                inst->substate = SDSPI_CMD_CHECK_BUSY;
                return SDSPI_CMD_RES_BUSY_WAIT;
            }

            // Ready to send command
            inst->substate = SDSPI_CMD_MARSHAL;
            /* fallthrough */
        case SDSPI_CMD_MARSHAL:
            // Prepare command in command buffer
            cmd->start_bit = 0;
            cmd->transmition_bit = 1;
            cmd->command_index = cmd_index & 0x3f;
            cmd->argument = __builtin_bswap32(arg);
            cmd->crc = sdspi_crc_7(cmd->raw, 5);
            cmd->end_bit = 1;

            inst->substate = SDSPI_CMD_SEND;
            /* fallthrough */
        case SDSPI_CMD_SEND:
            ret = sercom_spi_start_session_transaction(inst->spi_inst,
                                                    inst->spi_tid,
                                                    inst->cmd_buffer,
                                                    sizeof(union sdspi_command),
                                                    inst->cmd_buffer,
                                                    SDSPI_CMD_READ_LENGTH);

            if (ret != 0) {
                // Could not queue transaction in session
                return SDSPI_CMD_RES_QUEUE_WAIT;
            }

            inst->spi_in_progress = 1;
            inst->substate = SDSPI_CMD_GET_RSP;

            return SDSPI_CMD_RES_IN_PROGRESS;
        case SDSPI_CMD_GET_RSP:
            // If we make it here the SPI transaction is done
            inst->spi_in_progress = 0;

            // Check if we received any of the response to the command
            if (inst->bytes_in == 0) {
                // We have not recieved any of the response yet. Search for the
                // start of the response in the data we received.

                uint8_t rsp_start = 0;
                for (; rsp_start < SDSPI_CMD_READ_LENGTH; rsp_start++) {
                    if (!(inst->cmd_buffer[rsp_start] & (1 << 7))) {
                        // This byte is the start of a response
                        break;
                    }
                }

                uint16_t new_bytes = SDSPI_CMD_READ_LENGTH - rsp_start;

                if (new_bytes > rsp_len) {
                    new_bytes = rsp_len;
                }

                // Move the reponse bytes we received to the begining of the
                // buffer
                memmove(inst->cmd_buffer, inst->cmd_buffer + rsp_start,
                        new_bytes);
                // Update the count of received bytes
                inst->bytes_in += new_bytes;
                // Clear the part of the buffer that does not contain valid
                // response bytes
                memset(inst->cmd_buffer + inst->bytes_in, 0xFF, rsp_start);
            } else {
                // This is the second part of a response that started in the
                // previous transaction, we will have the whole reponse in the
                // right place now
                inst->bytes_in = rsp_len;
            }

            if (inst->bytes_in == rsp_len) {
                // We have received the full response and are done with this
                // command
                break;
            }

            inst->substate = SDSPI_CMD_INCOMPLETE_RSP;
            /* fallthrough */
        case SDSPI_CMD_INCOMPLETE_RSP:
            // Check if we have timed out (we don't time out if we are already
            // receiving the response, only if we are still waiting for it to
            // start)
            if ((inst->bytes_in == 0) && ((millis - inst->cmd_start_time) >
                                          SDSPI_CMD_TIMEOUT)) {
                sdspi_end_spi_session(inst);
                inst->substate = 0;
                return SDSPI_CMD_RES_TIMEOUT;
            }

            // We need to queue another SPI transaction to get the rest of the
            // response
            uint16_t const num_bytes_left = (inst->bytes_in == 0) ?
                                            SDSPI_CMD_READ_LENGTH :
                                            (rsp_len - inst->bytes_in);
            ret = sercom_spi_start_session_transaction(inst->spi_inst,
                                            inst->spi_tid, NULL, 0,
                                            inst->cmd_buffer + inst->bytes_in,
                                            num_bytes_left);

            if (ret != 0) {
                // Could not queue transaction in session
                return SDSPI_CMD_RES_QUEUE_WAIT;
            }

            inst->spi_in_progress = 1;
            inst->substate = SDSPI_CMD_GET_RSP;

            return SDSPI_CMD_RES_IN_PROGRESS;
        default:
            // Should never happen
            sdspi_end_spi_session(inst);
            inst->substate = 0;
            return SDSPI_CMD_RES_FAILED;
    }

    // We are done with this command. The response is in inst->cmd_buffer.
    // Be careful! If this was a command that reads a block (or register) there
    // could be a control token and even some data from the block in the
    // cmd_buffer register as well!

    if (end_session) {
        // End SPI session
        sdspi_end_spi_session(inst);
    }

    // Clean up for next time
    inst->substate = 0;
    inst->bytes_in = 0;

    return SDSPI_CMD_RES_DONE;
}

enum sdspi_blk_read_substate {
    SDSPI_BLK_READ_INIT = 0,
    SDSPI_BLK_READ_SEARCH_BST,
    SDSPI_BLK_READ_READ_BST,
    SDSPI_BLK_READ_READ_BLK,
    SDSPI_BLK_READ_SEND_STOP_CMD,
    SDSPI_BLK_READ_READ_BLK_DONE,
    SDSPI_BLK_READ_READ_CRC,
    SDSPI_BLK_READ_READ_CRC_DONE
};

/** Describes the result of the sdspi_handle_write_state function */
enum sdspi_blk_read_state_result {
    SDSPI_BLK_READ_RES_DONE,
    SDSPI_BLK_READ_RES_AGAIN,
    SDSPI_BLK_READ_RES_QUEUE_WAIT,
    SDSPI_BLK_READ_RES_IN_PROGRESS,
    SDSPI_BLK_READ_RES_DATA_ERROR,
    SDSPI_BLK_READ_RES_CRC_ERROR,
    SDSPI_BLK_READ_RES_TIMEOUT,
    SDSPI_BLK_READ_RES_FAILED
};

static enum sdspi_blk_read_state_result sdspi_handle_read_block(
                                                    struct sdspi_desc_t *inst,
                                                    uint8_t *dest,
                                                    uint16_t block_length,
                                                    uint8_t end_session,
                                                    uint8_t search_cmd_buffer,
                                                    uint8_t send_stop_cmd)
{
    uint8_t st_tok_off;
    uint8_t ret;

    switch (inst->substate) {
        case SDSPI_BLK_READ_INIT:
            inst->cmd_start_time = millis;

            if (!search_cmd_buffer) {
                inst->substate = SDSPI_BLK_READ_READ_BST;
                return SDSPI_BLK_READ_RES_AGAIN;
            }

            // Find out how many bytes we expect to have received as part of the
            // command response. This is the size of the command buffer minus
            // one byte for the R1 response token.
            uint16_t const cmd_buf_bytes = sizeof(inst->cmd_buffer) - 1;
            // Find the offset of where the bytes from the command buffer will
            // need to be placed
            uint16_t const cpy_offset = (sizeof(inst->rsp_buffer) -
                                         cmd_buf_bytes);
            // Clear the part of the response buffer that won't be copied from
            // the command buffer
            memset(inst->rsp_buffer, 0xFF, cpy_offset);
            // Copy over the bytes from the cmd buffer to the end of the rsp
            // buffer
            memcpy(inst->rsp_buffer + cpy_offset, inst->cmd_buffer + 1,
                   cmd_buf_bytes);

            inst->substate = SDSPI_BLK_READ_SEARCH_BST;
            /* fallthrough */
        case SDSPI_BLK_READ_SEARCH_BST:
            // If we made it here the SPI transaction is complete (or there
            // never was one)
            inst->spi_in_progress = 0;
            // Search for start block token or data error token
            st_tok_off = 0;
            for (; st_tok_off < sizeof(inst->rsp_buffer); st_tok_off++) {
                if (inst->rsp_buffer[st_tok_off] ==
                        SDSPI_SINGLE_BLOCK_START_TOKEN) {
                    // This is the start block token
                    break;
                } else if (SDSPI_IS_DATA_ERROR(inst->rsp_buffer[st_tok_off])) {
                    // This is a data error token
                    // Put error token in start of rsp_buffer so that the state
                    // code can parse it to find out the reason for the error.
                    inst->rsp_buffer[0] = inst->rsp_buffer[st_tok_off];
                    inst->substate = 0;

                    sdspi_end_spi_session(inst);
                    inst->substate = 0;
                    return SDSPI_BLK_READ_RES_DATA_ERROR;
                }
            }

            if (st_tok_off < sizeof(inst->rsp_buffer)) {
                // We found the start of the data!
                // Copy any data that ended up in the CMD buffer into our
                // destination buffer
                uint8_t const data_off = st_tok_off + 1;
                inst->bytes_in = sizeof(inst->rsp_buffer) - data_off;
                if (inst->bytes_in > block_length) {
                    inst->bytes_in = block_length;
                }

                memmove(dest, inst->rsp_buffer + data_off, inst->bytes_in);

                if (inst->bytes_in == block_length) {
                    // We have received all of the bytes we want

                    // Check if we got any CRC bytes too
                    inst->bytes_in = (sizeof(inst->rsp_buffer) - data_off -
                                      block_length);

                    // Copy CRC bytes into cmd_buffer
                    memcpy(inst->cmd_buffer, inst->rsp_buffer + data_off +
                           block_length, inst->bytes_in);

                    if (inst->bytes_in < 2) {
                        // We still need to get more of the CRC
                        inst->substate = SDSPI_BLK_READ_READ_CRC;
                    } else {
                        // We have the whole CRC
                        inst->substate = SDSPI_BLK_READ_READ_CRC_DONE;
                    }
                    return SDSPI_BLK_READ_RES_AGAIN;
                }

                // Go to read block substate to read the rest of the block
                inst->substate = SDSPI_BLK_READ_READ_BLK;
                return SDSPI_BLK_READ_RES_AGAIN;
            }

            // We didn't find the start token yet. We need to read in another
            // cmd_buffer worth of bytes to check for it.
            inst->substate = SDSPI_BLK_READ_READ_BST;
            /* fallthrough */
        case SDSPI_BLK_READ_READ_BST:
            // Check if we have timed out
            if ((millis - inst->cmd_start_time) > SDSPI_BLK_READ_TIMEOUT) {
                sdspi_end_spi_session(inst);
                inst->substate = 0;
                return SDSPI_BLK_READ_RES_TIMEOUT;
            }

            // Read in another 16 bytes to search for the BST in
            ret = sercom_spi_start_session_transaction(inst->spi_inst,
                                                    inst->spi_tid, NULL, 0,
                                                    inst->rsp_buffer,
                                                    sizeof(inst->rsp_buffer));

            if (ret != 0) {
                // Could not queue transaction in session
                return SDSPI_BLK_READ_RES_QUEUE_WAIT;
            }

            inst->spi_in_progress = 1;
            inst->substate = SDSPI_BLK_READ_SEARCH_BST;

            return SDSPI_BLK_READ_RES_IN_PROGRESS;
        case SDSPI_BLK_READ_READ_BLK:
            ;
            // Calculate how many bytes we need to receive. It should be a few
            // less if we need to send the stop command.
            uint16_t in_len = block_length - inst->bytes_in;
            if (send_stop_cmd) {
                in_len -= sizeof(union sdspi_command) - 2;
            }
            ret = sercom_spi_start_session_transaction(inst->spi_inst,
                                                inst->spi_tid, NULL, 0,
                                                dest + inst->bytes_in, in_len);

            if (ret != 0) {
                // Could not queue transaction in session
                return SDSPI_BLK_READ_RES_QUEUE_WAIT;
            }

            if (send_stop_cmd) {
                // Marshal the stop cmd while the SPI transaction is running
                union sdspi_command *const cmd =
                                        (union sdspi_command*)inst->cmd_buffer;
                cmd->start_bit = 0;
                cmd->transmition_bit = 1;
                cmd->command_index = SDSPI_CMD12;
                cmd->argument = 0;
                cmd->crc = sdspi_crc_7(cmd->raw, 5);
                cmd->end_bit = 1;
            }

            inst->spi_in_progress = 1;
            inst->substate = SDSPI_BLK_READ_READ_BLK_DONE;

            return SDSPI_BLK_READ_RES_IN_PROGRESS;
        case SDSPI_BLK_READ_SEND_STOP_CMD:
            ;
            // Start sending the stop command while we receive the last few
            // bytes of the data block
            static uint16_t const len = sizeof(union sdspi_command) - 2;
            uint8_t *const new_dest = dest + (block_length - len);
            ret = sercom_spi_start_simultaneous_session_transaction(
                                                            inst->spi_inst,
                                                            inst->spi_tid,
                                                            inst->cmd_buffer,
                                                            new_dest, len);

            if (ret != 0) {
                // Could not queue transaction in session
                return SDSPI_BLK_READ_RES_QUEUE_WAIT;
            }

            inst->spi_in_progress = 1;
            inst->substate = SDSPI_BLK_READ_READ_CRC;

            return SDSPI_BLK_READ_RES_IN_PROGRESS;
        case SDSPI_BLK_READ_READ_BLK_DONE:
            // If we made it here the SPI transaction is complete
            inst->spi_in_progress = 0;
            inst->bytes_in = 0;

            if (send_stop_cmd) {
                inst->substate = SDSPI_BLK_READ_SEND_STOP_CMD;
                return SDSPI_BLK_READ_RES_AGAIN;
            }

            // Start reading the CRC
            inst->substate = SDSPI_BLK_READ_READ_CRC;
            /* fallthrough */
        case SDSPI_BLK_READ_READ_CRC:
            if (send_stop_cmd) {
                // If we made it here the previous stop command/data transaction
                // is complete
                inst->spi_in_progress = 0;
                inst->bytes_in = 0;

                // Send last two bytes of stop command while receiving CRC
                uint8_t *const buff = (inst->cmd_buffer +
                                       (sizeof(union sdspi_command) - 2));
                ret = sercom_spi_start_simultaneous_session_transaction(
                                                        inst->spi_inst,
                                                        inst->spi_tid, buff,
                                                        inst->cmd_buffer, 2);
            } else {
                ret = sercom_spi_start_session_transaction(inst->spi_inst,
                                                        inst->spi_tid, NULL, 0,
                                                        inst->cmd_buffer,
                                                        2 - inst->bytes_in);
            }

            if (ret != 0) {
                // Could not queue transaction in session
                return SDSPI_BLK_READ_RES_QUEUE_WAIT;
            }

            inst->spi_in_progress = 1;
            inst->substate = SDSPI_BLK_READ_READ_CRC_DONE;

            return SDSPI_BLK_READ_RES_IN_PROGRESS;
        case SDSPI_BLK_READ_READ_CRC_DONE:
            // If we made it here the SPI transaction is complete (or there
            // never was one)
            inst->spi_in_progress = 0;

            // End the session before we start verifying the CRC
            if (end_session) {
                // End SPI session
                sdspi_end_spi_session(inst);
            }

#if SDSPI_USE_CRC == 1
            // Verify crc
            uint16_t const crc = ((((uint16_t)inst->cmd_buffer[0]) << 8) |
                                  inst->cmd_buffer[1]);
            if (crc != sdspi_crc_16(dest, block_length)) {
                // CRC failed
                inst->substate = 0;
                return SDSPI_BLK_READ_RES_CRC_ERROR;
            }
#endif

            // All done!
            break;
        default:
            // Should never happen
            sdspi_end_spi_session(inst);
            return SDSPI_BLK_READ_RES_FAILED;
    };

    // Clean up for next time
    inst->substate = 0;
    inst->bytes_in = 0;

    return SDSPI_BLK_READ_RES_DONE;
}


enum sdspi_blk_write_substate {
    SDSPI_BLK_WRITE_INIT = 0,
    SDSPI_BLK_WRITE_BUSY_CHECK,
    SDSPI_BLK_WRITE_BUSY_CHECK_RSP,
    SDSPI_BLK_WRITE_SEND_START_TOKEN,
    SDSPI_BLK_WRITE_SEND_BLOCK,
    SDSPI_BLK_WRITE_SEND_CRC,
    SDSPI_BLK_WRITE_CHECK_DATA_RSP,
    SDSPI_BLK_WRITE_GET_DATA_RSP
};

/** Describes the result of the sdspi_handle_write_state function */
enum sdspi_blk_write_state_result {
    SDSPI_BLK_WRITE_RES_DONE,
    SDSPI_BLK_WRITE_RES_AGAIN,
    SDSPI_BLK_WRITE_RES_QUEUE_WAIT,
    SDSPI_BLK_WRITE_RES_BUSY_WAIT,
    SDSPI_BLK_WRITE_RES_IN_PROGRESS,
    SDSPI_BLK_WRITE_RES_WRITE_ERROR,
    SDSPI_BLK_WRITE_RES_CRC_ERROR,
    SDSPI_BLK_WRITE_RES_BUSY_TIMEOUT,
    SDSPI_BLK_WRITE_RES_RSP_TIMEOUT,
    SDSPI_BLK_WRITE_RES_FAILED
};

static enum sdspi_blk_write_state_result sdspi_handle_write_block(
                                                    struct sdspi_desc_t *inst,
                                                    uint8_t const *data,
                                                    uint16_t block_length,
                                                    uint8_t end_session,
                                                    uint8_t busy_check,
                                                    uint8_t single)
{
    uint8_t ret;

    switch (inst->substate) {
        case SDSPI_BLK_WRITE_INIT:
            if (!busy_check) {
                inst->substate = SDSPI_BLK_WRITE_SEND_START_TOKEN;
                return SDSPI_BLK_WRITE_RES_AGAIN;
            }
            inst->cmd_start_time = millis;
            /* fallthrough */
        case SDSPI_BLK_WRITE_BUSY_CHECK:
            // Read a byte to check if the card is busy
            ret = sercom_spi_start_session_transaction(inst->spi_inst,
                                                       inst->spi_tid, NULL, 0,
                                                       inst->rsp_buffer,
                                                       SDSPI_BUSY_CHECK_BYTES);

            if (ret != 0) {
                // Could not queue transaction in session
                return SDSPI_BLK_WRITE_RES_QUEUE_WAIT;
            }

            inst->spi_in_progress = 1;
            inst->substate = SDSPI_BLK_WRITE_BUSY_CHECK_RSP;

            return SDSPI_BLK_WRITE_RES_IN_PROGRESS;
        case SDSPI_BLK_WRITE_BUSY_CHECK_RSP:
            // If we make it here the SPI transaction is done
            inst->spi_in_progress = 0;

            // The SD card will hold the MISO line low if it is busy. Check to
            // make sure that the last bit we read from the card is high, we
            // don't care about any of the other bits, only the most recent.
            if (!(inst->rsp_buffer[SDSPI_BUSY_CHECK_BYTES - 1] & (1 << 0))) {
                // Card is busy

                // Check if we have timed out
                if ((millis - inst->cmd_start_time) >
                            SDSPI_WRITE_BUSY_TIMEOUT) {
                    sdspi_end_spi_session(inst);
                    inst->substate = 0;
                    return SDSPI_BLK_WRITE_RES_BUSY_TIMEOUT;
                }

                // Check again
                inst->substate = SDSPI_BLK_WRITE_BUSY_CHECK;
                return SDSPI_BLK_WRITE_RES_BUSY_WAIT;
            }

            // Ready to send command
            inst->substate = SDSPI_BLK_WRITE_SEND_START_TOKEN;
            /* fallthrough */
        case SDSPI_BLK_WRITE_SEND_START_TOKEN:
            inst->cmd_buffer[0] = (single ? SDSPI_SINGLE_BLOCK_START_TOKEN :
                                   SDSPI_MULTI_BLOCK_START_TOKEN);
            // Send start token
            ret = sercom_spi_start_session_transaction(inst->spi_inst,
                                                       inst->spi_tid,
                                                       inst->cmd_buffer, 1,
                                                       NULL, 0);

            if (ret != 0) {
                // Could not queue transaction in session
                return SDSPI_BLK_WRITE_RES_QUEUE_WAIT;
            }

            inst->spi_in_progress = 1;
            inst->substate = SDSPI_BLK_WRITE_SEND_BLOCK;

            return SDSPI_BLK_WRITE_RES_IN_PROGRESS;
        case SDSPI_BLK_WRITE_SEND_BLOCK:
            // If we make it here the SPI transaction is done
            inst->spi_in_progress = 0;

            // Send block
            ret = sercom_spi_start_session_transaction(inst->spi_inst,
                                                       inst->spi_tid,
                                                       data, block_length,
                                                       NULL, 0);

            if (ret != 0) {
                // Could not queue transaction in session
                return SDSPI_BLK_WRITE_RES_QUEUE_WAIT;
            }

            inst->spi_in_progress = 1;
            inst->substate = SDSPI_BLK_WRITE_SEND_CRC;

            // Calculate CRC
#if SDSPI_USE_CRC == 1
            uint16_t const crc = sdspi_crc_16(data, block_length);
            inst->cmd_buffer[0] = (uint8_t)(crc >> 8);
            inst->cmd_buffer[1] = (uint8_t)(crc & 0xFF);
#else
            inst->cmd_buffer[0] = 0;
            inst->cmd_buffer[1] = 0;
#endif

            return SDSPI_BLK_WRITE_RES_IN_PROGRESS;
        case SDSPI_BLK_WRITE_SEND_CRC:
            // If we make it here the SPI transaction is done
            inst->spi_in_progress = 0;

            // Clear out the response buffer because while we just want to grab
            // one byte of response for now the check data rsp state will look
            // for the data response token in the whole response buffer.
            memset(inst->rsp_buffer, 0xFF, sizeof(inst->rsp_buffer));

            // Send CRC and start trying to receive data response token
            ret = sercom_spi_start_session_transaction(inst->spi_inst,
                                                       inst->spi_tid,
                                                       inst->cmd_buffer, 2,
                                                       inst->rsp_buffer, 1);

            if (ret != 0) {
                // Could not queue transaction in session
                return SDSPI_BLK_WRITE_RES_QUEUE_WAIT;
            }

            inst->cmd_start_time = millis;

            inst->spi_in_progress = 1;
            inst->substate = SDSPI_BLK_WRITE_CHECK_DATA_RSP;
            return SDSPI_BLK_WRITE_RES_IN_PROGRESS;
        case SDSPI_BLK_WRITE_CHECK_DATA_RSP:
            // If we make it here the SPI transaction is done
            inst->spi_in_progress = 0;

            size_t drt_index = 0;
            for (; drt_index < sizeof(inst->rsp_buffer); drt_index++) {
                if (SDSPI_DRT_VALID(inst->rsp_buffer[drt_index])) {
                    break;
                }
            }

            if (drt_index < sizeof(inst->rsp_buffer)) {
                // We found the data response token
                enum sdspi_drt_status const status =
                                SDSPI_DRT_STATUS(inst->rsp_buffer[drt_index]);

                if (status == SDSPI_DRT_STATUS_ACCEPTED) {
                    // Success! All done.
                    break;
                } else {
                    // Block write failed
                    sdspi_end_spi_session(inst);
                    inst->substate = 0;

                    if (status == SDSPI_DRT_STATUS_CRC_ERROR) {
                        return SDSPI_BLK_WRITE_RES_CRC_ERROR;
                    } else if (status == SDSPI_DRT_STATUS_WRITE_ERROR) {
                        return SDSPI_BLK_WRITE_RES_WRITE_ERROR;
                    } else {
                        return SDSPI_BLK_WRITE_RES_FAILED;
                    }
                }
            }

            // Check for timout
            if ((millis - inst->cmd_start_time) > SDSPI_WRITE_RSP_TIMEOUT) {
                sdspi_end_spi_session(inst);
                inst->substate = 0;
                return SDSPI_BLK_WRITE_RES_RSP_TIMEOUT;
            }

            // Did not find the data response token
            inst->state = SDSPI_BLK_WRITE_GET_DATA_RSP;
            /* fallthrough */
        case SDSPI_BLK_WRITE_GET_DATA_RSP:
            // Try to get response token
            ret = sercom_spi_start_session_transaction(inst->spi_inst,
                                                    inst->spi_tid, NULL, 0,
                                                    inst->rsp_buffer,
                                                    sizeof(inst->rsp_buffer));

            if (ret != 0) {
                // Could not queue transaction in session
                return SDSPI_BLK_WRITE_RES_QUEUE_WAIT;
            }

            inst->spi_in_progress = 1;
            inst->substate = SDSPI_BLK_WRITE_CHECK_DATA_RSP;
            return SDSPI_BLK_WRITE_RES_IN_PROGRESS;
        default:
            // Should never happen
            sdspi_end_spi_session(inst);
            return SDSPI_BLK_WRITE_RES_FAILED;
    }

    if (end_session) {
        // End SPI session
        sdspi_end_spi_session(inst);
    }

    // Clean up for next time
    inst->substate = 0;

    return SDSPI_BLK_WRITE_RES_DONE;
}


/**
 *  Wait for card to be inserted.
 */
static int sdspi_case_handler_not_present(struct sdspi_desc_t *inst)
{
    // Check if SD card is present now, if we don't have an SD detect pin we
    // just assume that the card is present
    if (gpio_pin_is_invalid(inst->card_detect_pin) ||
        (gpio_get_input(inst->card_detect_pin) == 0)) {
        // Card is present

        if (!inst->card_present) {
            // Card wasn't present before
            inst->card_present = 1;
            inst->card_detect_time = millis;
        } else if ((millis - inst->card_detect_time) >
                   SDSPI_INSERT_GLITCH_FILTER_TIME) {
            // Glich filter time is over
            inst->state = SDSPI_INIT_CYCLES;
            // Go right into next state handler
            return 1;
        }
    }

    return 0;
}

/**
 *  Send at least 74 cycles at a low clock speed with CS high.
 *
 *  We send 80 cycles at 400 KHz and use a CS pin mask of 0 so that the CS pin
 *  isn't asserted.
 */
static int sdspi_case_handler_init_cycles(struct sdspi_desc_t *inst)
{
    if (inst->spi_in_progress) {
        // Transaction is complete
        sercom_spi_clear_transaction(inst->spi_inst, inst->spi_tid);
        inst->spi_in_progress = 0;
        inst->state = SDSPI_SOFT_RESET;
        // Go right into next state handler
        return 1;
    }

    memset(inst->rsp_buffer, 0xFF, 10);
    uint8_t const ret = sercom_spi_start(inst->spi_inst,
                                         &inst->spi_tid,
                                         SDSPI_BAUDRATE_INIT, 0, 0,
                                         inst->rsp_buffer, 10, NULL, 0);
    inst->spi_in_progress = !ret;

    return 0;
}

/**
 *  Send CMD0 to reset card.
 */
static int sdspi_case_handler_soft_reset(struct sdspi_desc_t *inst)
{
    enum sdspi_cmd_state_result res;
    res = sdspi_handle_cmd_state(inst, SDSPI_CMD0, 0, SDSPI_BAUDRATE_INIT, 1, 0,
                                 1, 1);

    switch (res) {
        case SDSPI_CMD_RES_DONE:
            // Check response
            if (inst->cmd_buffer[0] != 0x01) {
                // Failed. Try again.
                return sdspi_init_retry(inst);
            }

            // CMD0 succeeded!
            inst->init_retry_count = 0;
            // Go right into next state
            inst->state = SDSPI_SEND_HOST_VOLT_INFO;
            return 1;
        case SDSPI_CMD_RES_AGAIN:
            return 1;
        case SDSPI_CMD_RES_BUSY_WAIT:
        case SDSPI_CMD_RES_QUEUE_WAIT:
        case SDSPI_CMD_RES_IN_PROGRESS:
            // Come back later
            break;
        case SDSPI_CMD_RES_FAILED:
        case SDSPI_CMD_RES_TIMEOUT:
            return sdspi_init_retry(inst);
        default:
            // Should not happen
            inst->state = SDSPI_FAILED;
            break;
    }

    return 0;
}

/**
 *  Send CMD8 to send interface conditions.
 */
static int sdspi_case_handler_send_host_volt_info(struct sdspi_desc_t *inst)
{
    static union sdspi_cmd8_arg const arg = { .check_pattern = 0xAA,
                                              // 2.7 to 3.6 volts (Table 4-16)
                                              .supply_voltage = 0b0001,
                                              .RESERVED = 0 };

    enum sdspi_cmd_state_result res;
    res = sdspi_handle_cmd_state(inst, SDSPI_CMD8, arg.raw, SDSPI_BAUDRATE_INIT,
                                 5, 1, 1, 1);
    union sdspi_response_r7 rsp;

    switch (res) {
        case SDSPI_CMD_RES_DONE:
            // Swap response bytes into the right places for r7 response format
            rsp = sdspi_swap_r7(inst->cmd_buffer);

            if (rsp.r1.raw != 0x01) {
                // Command failed
                return sdspi_init_retry(inst);
            }

            if (rsp.check_pattern != 0xAA) {
                // Check pattern is incorrect, command failed
                return sdspi_init_retry(inst);
            }

            // CMD8 succeeded!
            inst->init_retry_count = 0;
            // Go right into next state
            inst->state = SDSPI_SET_CRC;
            return 1;
        case SDSPI_CMD_RES_AGAIN:
            return 1;
        case SDSPI_CMD_RES_BUSY_WAIT:
        case SDSPI_CMD_RES_QUEUE_WAIT:
        case SDSPI_CMD_RES_IN_PROGRESS:
            // Come back later
            break;
        case SDSPI_CMD_RES_FAILED:
        case SDSPI_CMD_RES_TIMEOUT:
            return sdspi_init_retry(inst);
        default:
            // Should not happen
            inst->state = SDSPI_FAILED;
            break;
    }

    return 0;
}

/**
 *  Send CMD59 to enable/disable CRC checking
 */
static int sdspi_case_handler_set_crc(struct sdspi_desc_t *inst)
{
    enum sdspi_cmd_state_result res;
    res = sdspi_handle_cmd_state(inst, SDSPI_CMD59, SDSPI_USE_CRC,
                                 SDSPI_BAUDRATE_INIT, 1, 1, 1, 1);

    switch (res) {
        case SDSPI_CMD_RES_DONE:
            // Check response
            if (inst->cmd_buffer[0] != 0x01) {
                // Failed. Try again.
                return sdspi_init_retry(inst);
            }

            // CMD59 succeeded!
            inst->init_retry_count = 0;
            // Go right into next state
            inst->state = SDSPI_NEXT_CMD_APP_SPECIFIC;
            inst->acmd_state = SDSPI_INIT_CARD;
            return 1;
        case SDSPI_CMD_RES_AGAIN:
            return 1;
        case SDSPI_CMD_RES_BUSY_WAIT:
        case SDSPI_CMD_RES_QUEUE_WAIT:
        case SDSPI_CMD_RES_IN_PROGRESS:
            // Come back later
            break;
        case SDSPI_CMD_RES_FAILED:
        case SDSPI_CMD_RES_TIMEOUT:
            return sdspi_init_retry(inst);
        default:
            // Should not happen
            inst->state = SDSPI_FAILED;
            break;
    }

    return 0;
}

/**
 *  Send CMD55 to inidicate that the next command is an application specific
 *  command.
 *
 *  After this state completes it will go into the state specified by
 *  inst->acmd_state. If CMD55 fails with the illegal command bit set and
 *  inst->acmd_state is SDSPI_INIT_CARD it will go to SDSPI_INIT_V1_CARD and set
 *  inst->v1_card. If CMD55 fails for any other reason or inst->acmd_state is
 *  not SDSPI_INIT_CARD it will go to SDSPI_FAILED.
 */
static int sdspi_case_handler_next_cmd_app_specific(struct sdspi_desc_t *inst)
{
    uint32_t const br = (inst->acmd_state == SDSPI_INIT_CARD) ?
                            SDSPI_BAUDRATE_INIT : SDSPI_BAUDRATE;

    enum sdspi_cmd_state_result res;
    res = sdspi_handle_cmd_state(inst, SDSPI_CMD55, 0, br, 1, 1, 1, 1);
    union sdspi_response_r1 rsp;

    switch (res) {
        case SDSPI_CMD_RES_DONE:
            rsp.raw = inst->cmd_buffer[0];

            // Check response
            if (rsp.illegal_command && (inst->acmd_state == SDSPI_INIT_CARD)) {
                // This is a ver 1 card
                inst->v1_card = 1;
                inst->init_retry_count = 0;
                inst->state = SDSPI_INIT_V1_CARD;
                return 1;
            }

            if ((rsp.raw != 0x00) && (rsp.raw != 0x01)) {
                // Command failed
                inst->state = SDSPI_FAILED;
                return 0;
            }

            // CMD55 succeeded!
            inst->init_retry_count = 0;
            // Go right into next state
            inst->state = inst->acmd_state;
            return 1;
        case SDSPI_CMD_RES_AGAIN:
            return 1;
        case SDSPI_CMD_RES_BUSY_WAIT:
        case SDSPI_CMD_RES_QUEUE_WAIT:
        case SDSPI_CMD_RES_IN_PROGRESS:
            // Come back later
            break;
        case SDSPI_CMD_RES_FAILED:
        case SDSPI_CMD_RES_TIMEOUT:
            return sdspi_init_retry(inst);
        default:
            // Should not happen
            inst->state = SDSPI_FAILED;
            break;
    }

    return 0;
}

/**
 *  Send ACMD41 to send host capacity support information and start card
 *  initialization.
 */
static int sdspi_case_handler_init_card(struct sdspi_desc_t *inst)
{
    enum sdspi_cmd_state_result res;
    res = sdspi_handle_cmd_state(inst, SDSPI_ACMD41, SDSPI_ACMD41_HCS,
                                 SDSPI_BAUDRATE_INIT, 1, 1, 1, 1);
    union sdspi_response_r1 rsp;

    switch (res) {
        case SDSPI_CMD_RES_DONE:
            rsp.raw = inst->cmd_buffer[0];

            // Check response
            if (rsp.illegal_command) {
                // This is a ver 1 card
                inst->v1_card = 1;
                inst->init_retry_count = 0;
                inst->state = SDSPI_INIT_V1_CARD;
                return 1;
            }

            if (rsp.in_idle_state) {
                // Card is not initialized yet. Repeat CMD55 and ACMD41.
                inst->state = SDSPI_NEXT_CMD_APP_SPECIFIC;
                inst->acmd_state = SDSPI_INIT_CARD;
                return sdspi_init_retry(inst);
            }

            if (rsp.raw != 0) {
                // Command failed
                inst->state = SDSPI_FAILED;
                return 0;
            }

            // CMD41 succeeded!
            inst->init_retry_count = 0;
            // Go right into next state
            inst->state = SDSPI_READ_OCR;
            return 1;
        case SDSPI_CMD_RES_AGAIN:
            return 1;
        case SDSPI_CMD_RES_BUSY_WAIT:
        case SDSPI_CMD_RES_QUEUE_WAIT:
        case SDSPI_CMD_RES_IN_PROGRESS:
            // Come back later
            break;
        case SDSPI_CMD_RES_FAILED:
        case SDSPI_CMD_RES_TIMEOUT:
            return sdspi_init_retry(inst);
        default:
            // Should not happen
            inst->state = SDSPI_FAILED;
            break;
    }

    return 0;
}

/**
 *  Send CMD1 to send host capacity support information and start card
 *  initialization.
 */
static int sdspi_case_handler_init_v1_card(struct sdspi_desc_t *inst)
{
    enum sdspi_cmd_state_result res;
    res = sdspi_handle_cmd_state(inst, SDSPI_CMD1, SDSPI_CMD1_HCS,
                                 SDSPI_BAUDRATE_INIT, 1, 1, 1, 1);
    union sdspi_response_r1 rsp;

    switch (res) {
        case SDSPI_CMD_RES_DONE:
            rsp.raw = inst->cmd_buffer[0];

            // Check response
            if (rsp.in_idle_state) {
                // Card is not initialized yet. Repeat CMD55 and ACMD41.
                return sdspi_init_retry(inst);
            }

            if (rsp.raw != 0) {
                // Command failed
                inst->state = SDSPI_FAILED;
                return 0;
            }

            // CMD1 succeeded!
            inst->init_retry_count = 0;
            // Go right into next state
            inst->state = SDSPI_READ_OCR;
            return 1;
        case SDSPI_CMD_RES_AGAIN:
            return 1;
        case SDSPI_CMD_RES_BUSY_WAIT:
        case SDSPI_CMD_RES_QUEUE_WAIT:
        case SDSPI_CMD_RES_IN_PROGRESS:
            // Come back later
            break;
        case SDSPI_CMD_RES_FAILED:
        case SDSPI_CMD_RES_TIMEOUT:
            return sdspi_init_retry(inst);
        default:
            // Should not happen
            inst->state = SDSPI_FAILED;
            break;
    }

    return 0;
}

/**
 *  Send CMD58 to read OCR (operating conditions register).
 */
static int sdspi_case_handler_read_ocr(struct sdspi_desc_t *inst)
{
    enum sdspi_cmd_state_result res;
    res = sdspi_handle_cmd_state(inst, SDSPI_CMD58, 0, SDSPI_BAUDRATE, 5, 1, 1,
                                 1);
    union sdspi_response_r3 rsp;

    switch (res) {
        case SDSPI_CMD_RES_DONE:
            rsp = sdspi_swap_r3(inst->cmd_buffer);

            if (rsp.r1.raw != 0) {
                // Command failed
                inst->state = SDSPI_FAILED;
                return 0;
            }

            // Check that the card has at least one supported voltage range that
            // is close to 3.3 volts
            if (!rsp.ocr.volt_range_3V1_3V2 &&
                !rsp.ocr.volt_range_3V2_3V3 &&
                !rsp.ocr.volt_range_3V3_3V4 &&
                !rsp.ocr.volt_range_3V4_3V5) {
                inst->state = SDSPI_UNUSABLE_CARD;
                return 0;
            }

            inst->block_addressed = rsp.ocr.card_capacity_status;

            // CMD58 succeeded!
            // Go right into next state
            inst->init_retry_count = 0;
            inst->state = SDSPI_READ_CSD;
            return 1;
        case SDSPI_CMD_RES_AGAIN:
            return 1;
        case SDSPI_CMD_RES_BUSY_WAIT:
        case SDSPI_CMD_RES_QUEUE_WAIT:
        case SDSPI_CMD_RES_IN_PROGRESS:
            // Come back later
            break;
        case SDSPI_CMD_RES_FAILED:
        case SDSPI_CMD_RES_TIMEOUT:
            return sdspi_init_retry(inst);
        default:
            // Should not happen
            inst->state = SDSPI_FAILED;
            break;
    }

    return 0;
}

/**
 *  Send CMD9 to read CSD (card specific data) register.
 */
static int sdspi_case_handler_read_csd(struct sdspi_desc_t *inst)
{
    enum sdspi_cmd_state_result res;
    res = sdspi_handle_cmd_state(inst, SDSPI_CMD9, 0, SDSPI_BAUDRATE, 1, 1, 0,
                                 1);
    union sdspi_response_r1 rsp;

    switch (res) {
        case SDSPI_CMD_RES_DONE:
            rsp.raw = inst->cmd_buffer[0];

            if (rsp.raw != 0) {
                // Command failed
                inst->state = SDSPI_FAILED;
                return 0;
            }
            
            // CMD9 succeeded!
            inst->init_retry_count = 0;
            // Go right into next state
            inst->state = SDSPI_READ_CSD_READ_BLOCK;
            return 1;
        case SDSPI_CMD_RES_AGAIN:
            return 1;
        case SDSPI_CMD_RES_BUSY_WAIT:
        case SDSPI_CMD_RES_QUEUE_WAIT:
        case SDSPI_CMD_RES_IN_PROGRESS:
            // Come back later
            break;
        case SDSPI_CMD_RES_FAILED:
        case SDSPI_CMD_RES_TIMEOUT:
            return sdspi_init_retry(inst);
        default:
            // Should not happen
            inst->state = SDSPI_FAILED;
            break;
    }

    return 0;
}


static int sdspi_case_handler_read_csd_read_block(struct sdspi_desc_t *inst)
{
    enum sdspi_blk_read_state_result res;
    res = sdspi_handle_read_block(inst, inst->rsp_buffer,
                                  sizeof(union sdspi_csd_1_reg), 1, 1, 0);
    union sdspi_csd_1_reg csd_1;
    union sdspi_csd_2_reg csd_2;

    switch (res) {
        case SDSPI_BLK_READ_RES_DONE:
            csd_2 = sdspi_swap_csd_2(inst->rsp_buffer);

            if (csd_2.csd_structure == 0b01) {
                // This is v2 of the CSD register
                inst->card_capacity = SDSPI_CSD_2_BLOCKS(csd_2);
            } else if (csd_2.csd_structure == 0b00) {
                // This is v1 of the CSD register
                csd_1 = sdspi_swap_csd_1(inst->rsp_buffer);
                inst->card_capacity = SDSPI_CSD_1_BLOCKS(csd_1);
            } else {
                // Unkown CSD register layout
                inst->state = SDSPI_UNUSABLE_CARD;
                return 0;
            }

            inst->init_retry_count = 0;
            inst->state = SDSPI_SET_BLOCK_LENGTH;
            return 1;
        case SDSPI_BLK_READ_RES_AGAIN:
            return 1;
        case SDSPI_BLK_READ_RES_QUEUE_WAIT:
        case SDSPI_BLK_READ_RES_IN_PROGRESS:
            // Come back later
            break;
        case SDSPI_BLK_READ_RES_DATA_ERROR:
        case SDSPI_BLK_READ_RES_CRC_ERROR:
        case SDSPI_BLK_READ_RES_TIMEOUT:
            // Try again
            inst->state = SDSPI_READ_CSD;
            return sdspi_init_retry(inst);
        case SDSPI_BLK_READ_RES_FAILED:
        default:
            inst->state = SDSPI_FAILED;
            break;
    }

    return 0;
}

/**
 *  Send CMD16 to set block length.
 */
static int sdspi_case_handler_set_block_length(struct sdspi_desc_t *inst)
{
    enum sdspi_cmd_state_result res;
    res = sdspi_handle_cmd_state(inst, SDSPI_CMD16, SDSPI_BLOCK_SIZE,
                                 SDSPI_BAUDRATE, 1, 1, 1, 1);
    union sdspi_response_r1 rsp;

    switch (res) {
        case SDSPI_CMD_RES_DONE:
            rsp.raw = inst->cmd_buffer[0];

            if (rsp.raw != 0) {
                // Command failed
                inst->state = SDSPI_FAILED;
                return 0;
            }

            // CMD16 succeeded!
            inst->init_retry_count = 0;
            // Go right into next state
            inst->state = SDSPI_IDLE;
            return 1;
        case SDSPI_CMD_RES_AGAIN:
            return 1;
        case SDSPI_CMD_RES_BUSY_WAIT:
        case SDSPI_CMD_RES_QUEUE_WAIT:
        case SDSPI_CMD_RES_IN_PROGRESS:
            // Come back later
            break;
        case SDSPI_CMD_RES_FAILED:
        case SDSPI_CMD_RES_TIMEOUT:
            return sdspi_init_retry(inst);
        default:
            // Should not happen
            inst->state = SDSPI_FAILED;
            break;
    }

    return 0;
}



static int sdspi_case_hander_idle(struct sdspi_desc_t *inst)
{
    return 0;
}



/**
 *  Send CMD17 or CMD18 to start reading a block or multiple blocks.
 */
static int sdspi_case_handler_start_read(struct sdspi_desc_t *inst)
{
    enum sdspi_cmd_state_result res;
    res = sdspi_handle_cmd_state(inst, ((inst->block_count == 1) ? SDSPI_CMD17 :
                                        SDSPI_CMD18), inst->op_addr,
                                 SDSPI_BAUDRATE, 1, 1, 0, 1);
    union sdspi_response_r1 rsp;

    switch (res) {
        case SDSPI_CMD_RES_DONE:
            rsp.raw = inst->cmd_buffer[0];

            if (rsp.raw != 0) {
                // Command failed
                if (inst->callback != NULL) {
                    inst->callback(inst->cb_context, SD_OP_FAILED, 0);
                }

                inst->state = SDSPI_IDLE;
                return 0;
            }

            // CMD17/CMD18 succeeded!
            // Go right into next state
            inst->state = SDSPI_READ_BLOCKS;
            return 1;
        case SDSPI_CMD_RES_AGAIN:
            return 1;
        case SDSPI_CMD_RES_BUSY_WAIT:
        case SDSPI_CMD_RES_QUEUE_WAIT:
        case SDSPI_CMD_RES_IN_PROGRESS:
            // Come back later
            break;
        case SDSPI_CMD_RES_FAILED:
        case SDSPI_CMD_RES_TIMEOUT:
            // Command failed
            if (inst->callback != NULL) {
                inst->callback(inst->cb_context, SD_OP_FAILED, 0);
            }
            inst->state = SDSPI_IDLE;
            break;
        default:
            // Should not happen
            inst->state = SDSPI_FAILED;
            break;
    }

    return 0;
}

static int sdspi_case_handler_read_blocks(struct sdspi_desc_t *inst)
{
    enum sdspi_blk_read_state_result res;

    uint8_t *const buffer = inst->read_buffer + (SDSPI_BLOCK_SIZE *
                                                 inst->blocks_done);

    // Only end session if we are reading a single block
    uint8_t const end_session = inst->block_count == 1;
    // Only check for a block start token in the command buffer if this is the
    // first block being read
    uint8_t const search_cmd_buf = inst->blocks_done == 0;
    // Only send the stop command if this is the last block of a multi-block
    // read
    uint8_t const send_stop_cmd = ((inst->block_count != 1) &&
                                   (inst->blocks_done ==
                                    (inst->block_count - 1)));

    res = sdspi_handle_read_block(inst, buffer, SDSPI_BLOCK_SIZE, end_session,
                                  search_cmd_buf, send_stop_cmd);

    switch (res) {
        case SDSPI_BLK_READ_RES_DONE:
            inst->blocks_done++;

            if (inst->blocks_done < inst->block_count) {
                // Not done yet
                return 1;
            }

            if (inst->block_count == 1) {
                // All done
                if (inst->callback != NULL) {
                    inst->callback(inst->cb_context, SD_OP_SUCCESS,
                                   inst->blocks_done);
                }

                inst->state = SDSPI_IDLE;
                return 0;
            } else {
                // Need to get response to stop command
                inst->state = SDSPI_READ_GET_STOP_RSP;
                // Jump right to reading the command response
                inst->substate = SDSPI_CMD_INCOMPLETE_RSP;
                inst->bytes_in = 0;
                return 1;
            }
        case SDSPI_BLK_READ_RES_AGAIN:
            return 1;
        case SDSPI_BLK_READ_RES_QUEUE_WAIT:
        case SDSPI_BLK_READ_RES_IN_PROGRESS:
            // Come back later
            break;
        case SDSPI_BLK_READ_RES_DATA_ERROR:
        case SDSPI_BLK_READ_RES_CRC_ERROR:
        case SDSPI_BLK_READ_RES_TIMEOUT:
        case SDSPI_BLK_READ_RES_FAILED:
            // Command failed
            if (inst->callback != NULL) {
                inst->callback(inst->cb_context, SD_OP_FAILED,
                               inst->blocks_done);
            }
            inst->state = SDSPI_IDLE;
            break;
        default:
            inst->state = SDSPI_FAILED;
            break;
    }

    return 0;
}

/**
 *  Get the response to CMD12.
 */
static int sdspi_case_handler_read_get_stop_rsp(struct sdspi_desc_t *inst)
{
    enum sdspi_cmd_state_result res;
    res = sdspi_handle_cmd_state(inst, 0, 0, SDSPI_BAUDRATE, 1, 0, 1, 1);
    union sdspi_response_r1 rsp;

    switch (res) {
        case SDSPI_CMD_RES_DONE:
            rsp.raw = inst->cmd_buffer[0];

            if (rsp.raw != 0) {
                // Command failed
                if (inst->callback != NULL) {
                    inst->callback(inst->cb_context, SD_OP_FAILED,
                                   inst->blocks_done);
                }

                inst->state = SDSPI_IDLE;
                return 0;
            }

            // CMD12 succeeded!
            if (inst->callback != NULL) {
                inst->callback(inst->cb_context, SD_OP_SUCCESS,
                               inst->blocks_done);
            }
            // Go right into next state
            inst->state = SDSPI_IDLE;
            return 1;
        case SDSPI_CMD_RES_AGAIN:
            return 1;
        case SDSPI_CMD_RES_BUSY_WAIT:
        case SDSPI_CMD_RES_QUEUE_WAIT:
        case SDSPI_CMD_RES_IN_PROGRESS:
            // Come back later
            break;
        case SDSPI_CMD_RES_FAILED:
        case SDSPI_CMD_RES_TIMEOUT:
            // Command failed
            if (inst->callback != NULL) {
                inst->callback(inst->cb_context, SD_OP_FAILED,
                               inst->blocks_done);
            }
            inst->state = SDSPI_IDLE;
            break;
        default:
            // Should not happen
            inst->state = SDSPI_FAILED;
            break;
    }

    return 0;
}



/**
 *  Send CMD24 or CMD25 to start writing a block or mutliple blocks.
 */
static int sdspi_case_handler_start_write(struct sdspi_desc_t *inst)
{
    enum sdspi_cmd_state_result res;
    res = sdspi_handle_cmd_state(inst, ((inst->block_count == 1) ? SDSPI_CMD24 :
                                        SDSPI_CMD25), inst->op_addr,
                                 SDSPI_BAUDRATE, 1, 1, 0, 1);
    union sdspi_response_r1 rsp;

    switch (res) {
        case SDSPI_CMD_RES_DONE:
            rsp.raw = inst->cmd_buffer[0];

            if (rsp.raw != 0) {
                // Command failed
                if (inst->callback != NULL) {
                    inst->callback(inst->cb_context, SD_OP_FAILED, 0);
                }

                inst->state = SDSPI_IDLE;
                return 0;
            }

            // CMD24/CMD25 succeeded!
            // Go right into next state
            inst->state = SDSPI_WRITE_BLOCKS;
            return 1;
        case SDSPI_CMD_RES_AGAIN:
            return 1;
        case SDSPI_CMD_RES_BUSY_WAIT:
        case SDSPI_CMD_RES_QUEUE_WAIT:
        case SDSPI_CMD_RES_IN_PROGRESS:
            // Come back later
            break;
        case SDSPI_CMD_RES_FAILED:
        case SDSPI_CMD_RES_TIMEOUT:
            // Command failed
            if (inst->callback != NULL) {
                inst->callback(inst->cb_context, SD_OP_FAILED,
                               inst->block_count);
            }
            inst->state = SDSPI_IDLE;
            break;
        default:
            // Should not happen
            inst->state = SDSPI_FAILED;
            break;
    }

    return 0;
}

static int sdspi_case_handler_write_blocks(struct sdspi_desc_t *inst)
{
    enum sdspi_blk_write_state_result res;

    uint8_t const *const data = inst->write_data + (SDSPI_BLOCK_SIZE *
                                                    inst->blocks_done);

    res = sdspi_handle_write_block(inst, data, SDSPI_BLOCK_SIZE, 0,
                                   (inst->blocks_done != 0),
                                   (inst->block_count == 1));

    switch (res) {
        case SDSPI_BLK_WRITE_RES_DONE:
            inst->blocks_done++;

            if (inst->blocks_done < inst->block_count) {
                // Not done yet
                return 1;
            }

            if (inst->block_count == 1) {
                // Need to send CMD13 to get status
                inst->state = SDSPI_WRITE_GET_STATUS;
            } else {
                // Need to send stop token
                inst->state = SDSPI_WRITE_SEND_STOP_TOKEN;
            }
            return 1;
        case SDSPI_BLK_WRITE_RES_AGAIN:
            return 1;
        case SDSPI_BLK_WRITE_RES_QUEUE_WAIT:
        case SDSPI_BLK_WRITE_RES_BUSY_WAIT:
        case SDSPI_BLK_WRITE_RES_IN_PROGRESS:
            // Come back later
            break;
        case SDSPI_BLK_WRITE_RES_WRITE_ERROR:
        case SDSPI_BLK_WRITE_RES_CRC_ERROR:
        case SDSPI_BLK_WRITE_RES_BUSY_TIMEOUT:
        case SDSPI_BLK_WRITE_RES_RSP_TIMEOUT:
        case SDSPI_BLK_WRITE_RES_FAILED:
            // Command failed
            if (inst->callback != NULL) {
                inst->callback(inst->cb_context, SD_OP_FAILED,
                               inst->block_count);
            }
            inst->state = SDSPI_IDLE;
            break;
        default:
            // Should not happen
            inst->state = SDSPI_FAILED;
            break;
    }

    return 0;
}

enum sdspi_send_stop_token_subtype {
    SDSPI_SEND_STOP_TOKEN_SUBTYPE_INIT = 0,
    SDSPI_SEND_STOP_TOKEN_SUBTYPE_BUSY_CHECK,
    SDSPI_SEND_STOP_TOKEN_SUBTYPE_BUSY_CHECK_RSP,
    SDSPI_SEND_STOP_TOKEN_SUBTYPE_SEND,
    SDSPI_SEND_STOP_TOKEN_SUBTYPE_WAIT
};

static int sdspi_case_handler_write_send_stop_token(struct sdspi_desc_t *inst)
{
    uint8_t ret;

    switch (inst->substate) {
        case SDSPI_SEND_STOP_TOKEN_SUBTYPE_INIT:
            inst->cmd_start_time = millis;
            inst->cmd_buffer[0] = SDSPI_MULTI_BLOCK_STOP_TOKEN;
            inst->substate = SDSPI_SEND_STOP_TOKEN_SUBTYPE_BUSY_CHECK;
            /* fallthrough */
        case SDSPI_SEND_STOP_TOKEN_SUBTYPE_BUSY_CHECK:
            // Read a byte to check if the card is busy
            ret = sercom_spi_start_session_transaction(inst->spi_inst,
                                                       inst->spi_tid, NULL, 0,
                                                       inst->rsp_buffer,
                                                       SDSPI_BUSY_CHECK_BYTES);

            if (ret != 0) {
                // Could not queue transaction in session
                return 0;
            }

            inst->spi_in_progress = 1;
            inst->substate = SDSPI_SEND_STOP_TOKEN_SUBTYPE_BUSY_CHECK_RSP;

            return 0;
        case SDSPI_SEND_STOP_TOKEN_SUBTYPE_BUSY_CHECK_RSP:
            // If we make it here the SPI transaction is done
            inst->spi_in_progress = 0;

            // The SD card will hold the MISO line low if it is busy. Check to
            // make sure that the last bit we read from the card is high, we
            // don't care about any of the other bits, only the most recent.
            if (!(inst->rsp_buffer[SDSPI_BUSY_CHECK_BYTES - 1] & (1 << 0))) {
                // Card is busy

                // Check if we have timed out
                if ((millis - inst->cmd_start_time) >
                        SDSPI_WRITE_BUSY_TIMEOUT) {
                    sdspi_end_spi_session(inst);
                    inst->substate = 0;
                    inst->state = SDSPI_FAILED;
                    return 0;
                }

                // Check again
                inst->substate = SDSPI_SEND_STOP_TOKEN_SUBTYPE_BUSY_CHECK;
                return 1;
            }

            // Ready to send stop token
            inst->substate = SDSPI_SEND_STOP_TOKEN_SUBTYPE_SEND;
            /* fallthrough */
        case SDSPI_SEND_STOP_TOKEN_SUBTYPE_SEND:
            ret = sercom_spi_start_session_transaction(inst->spi_inst,
                                                       inst->spi_tid,
                                                       inst->cmd_buffer, 1,
                                                       NULL, 0);

            if (ret != 0) {
                // Could not queue transaction in session
                return 0;
            }

            inst->spi_in_progress = 1;
            inst->substate = SDSPI_SEND_STOP_TOKEN_SUBTYPE_WAIT;

            return 0;
        case SDSPI_SEND_STOP_TOKEN_SUBTYPE_WAIT:
            // If we made it here the SPI transaction is over
            break;
        default:
            // Should not happen
            sdspi_end_spi_session(inst);
            inst->state = SDSPI_FAILED;
            return 0;
    }

    // Send CMD13 to get card status
    inst->substate = 0;
    inst->state = SDSPI_WRITE_GET_STATUS;
    return 1;
}

/**
 *  Send CMD13 to get card status to see if write operation suceeded.
 */
static int sdspi_case_handler_write_get_status(struct sdspi_desc_t *inst)
{
    enum sdspi_cmd_state_result res;
    res = sdspi_handle_cmd_state(inst, SDSPI_CMD13, 0, SDSPI_BAUDRATE, 2, 1, 1,
                                 0);
    union sdspi_response_r2 rsp;

    switch (res) {
        case SDSPI_CMD_RES_DONE:
            rsp = sdspi_swap_r2(inst->cmd_buffer);

            if ((rsp.raw[0] != 0) || (rsp.raw[1] != 0)) {
                // Command failed
                if (inst->callback != NULL) {
                    inst->callback(inst->cb_context, SD_OP_FAILED, 0);
                }
            } else {
                // Write succeded
                if (inst->callback != NULL) {
                    inst->callback(inst->cb_context, SD_OP_SUCCESS,
                                   inst->blocks_done);
                }
            }

            // CMD13 succeeded!
            // We are all done the write operation
            inst->state = SDSPI_IDLE;
            return 0;
        case SDSPI_CMD_RES_AGAIN:
            return 1;
        case SDSPI_CMD_RES_BUSY_WAIT:
        case SDSPI_CMD_RES_QUEUE_WAIT:
        case SDSPI_CMD_RES_IN_PROGRESS:
            // Come back later
            break;
        case SDSPI_CMD_RES_FAILED:
        case SDSPI_CMD_RES_TIMEOUT:
            // Command failed
            if (inst->callback != NULL) {
                inst->callback(inst->cb_context, SD_OP_FAILED,
                               inst->block_count);
            }
            inst->state = SDSPI_IDLE;
            break;
        default:
            // Should not happen
            inst->state = SDSPI_FAILED;
            break;
    }

    return 0;
}



static int sdspi_case_handler_failed(struct sdspi_desc_t *inst)
{
    // Make sure that we are not leaving a session hanging open
    sdspi_end_spi_session(inst);
    return 0;
}



const sdspi_state_handler_t sdspi_state_handlers[] = {
    sdspi_case_handler_not_present,             // SDSPI_NOT_PRESENT
    sdspi_case_handler_init_cycles,             // SDSPI_INIT_CYCLES
    sdspi_case_handler_soft_reset,              // SDSPI_SOFT_RESET
    sdspi_case_handler_send_host_volt_info,     // SDSPI_SEND_HOST_VOLT_INFO
    sdspi_case_handler_set_crc,                 // SDSPI_SET_CRC
    sdspi_case_handler_next_cmd_app_specific,   // SDSPI_NEXT_CMD_APP_SPECIFIC
    sdspi_case_handler_init_card,               // SDSPI_INIT_CARD
    sdspi_case_handler_init_v1_card,            // SDSPI_INIT_V1_CARD
    sdspi_case_handler_read_ocr,                // SDSPI_READ_OCR
    sdspi_case_handler_read_csd,                // SDSPI_READ_CSD
    sdspi_case_handler_read_csd_read_block,     // SDSPI_READ_CSD_READ_BLOCK
    sdspi_case_handler_set_block_length,        // SDSPI_SET_BLOCK_LENGTH

    sdspi_case_hander_idle,                     // SDSPI_IDLE

    sdspi_case_handler_start_read,              // SDSPI_START_READ
    sdspi_case_handler_read_blocks,             // SDSPI_READ_BLOCKS
    sdspi_case_handler_read_get_stop_rsp,       // SDSPI_READ_GET_STOP_RSP

    sdspi_case_handler_start_write,             // SDSPI_START_WRITE
    sdspi_case_handler_write_blocks,            // SDSPI_WRITE_BLOCKS
    sdspi_case_handler_write_send_stop_token,   // SDSPI_WRITE_SEND_STOP_TOKEN
    sdspi_case_handler_write_get_status,        // SDSPI_WRITE_GET_STATUS

    sdspi_case_handler_failed,                  // SDSPI_UNUSABLE_CARD
    sdspi_case_handler_failed,                  // SDSPI_TOO_MANY_INIT_RETRIES
    sdspi_case_handler_failed                   // SDSPI_FAILED
};

#endif // ENABLE_SDSPI
