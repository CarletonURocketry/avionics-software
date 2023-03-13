/**
 * @file logging.c
 * @desc Service which handles logging of data to an SD card
 * @author Samuel Dewan
 * @date 2021-07-11
 * Last Author:
 * Last Edited On:
 */

#include "logging.h"

#include "src/sd/mbr.h"


#define LOGGING_MAX_SD_RETRIES  3

#define LOGGING_BUFFER_WRITE_INTERVAL   10000
#define LOGGING_SB_WRITE_INTERVAL       20000
#define LOGGING_WATERMARK               32



void init_logging(struct logging_desc_t *inst, sd_desc_ptr_t sd_desc,
                  struct sd_funcs sd_funcs, uint8_t continue_flight)
{
    inst->buffer[0].count = 0;
    inst->buffer[0].checkout_count = 0;
    inst->buffer[1].count = 0;
    inst->buffer[1].checkout_count = 0;

    inst->sd_desc = sd_desc;
    inst->sd_funcs = sd_funcs;

    inst->init_retry_count = 0;
    inst->out_of_space_count = 0;

    inst->insert_point = NULL;
    inst->continue_flight = !!continue_flight;
    inst->state = LOGGING_GET_MBR;
    inst->should_pause = 0;

    logging_service(inst);
}

/**
 *  Callback function for when SD operation is complete.
 *
 *  @param context Will be a pointer to logging service instance descriptor
 *  @param result SD operation result
 *  @param num_blocks The number of blocks that where read or written
 */
static void logging_sd_callback(void *context, enum sd_op_result result,
                                uint32_t num_blocks)
{
    struct logging_desc_t *const inst = (struct logging_desc_t *)context;

    if (inst->state == LOGGING_MBR_WAIT) {
        if (result != SD_OP_SUCCESS) {
            // Retry
            inst->init_retry_count++;
            if (inst->init_retry_count > LOGGING_MAX_SD_RETRIES) {
                inst->state = LOGGING_TOO_MANY_SD_RETRIES;
            } else {
                inst->state = LOGGING_GET_MBR;
            }
        } else {
            inst->init_retry_count = 0;
            inst->state = LOGGING_MBR_PARSE;
        }
        inst->sd_write_in_progress = 0;
        return;
    } else if (inst->state == LOGGING_SUPERBLOCK_WAIT) {
        if (result != SD_OP_SUCCESS) {
            inst->state = LOGGING_TOO_MANY_SD_RETRIES;
        } else {
            inst->state = LOGGING_SUPERBLOCK_PARSE;
        }
        inst->sd_write_in_progress = 0;
        return;
    }

    // Check if a buffer is being written
    if (inst->blocks_in_progress == 0) {
        // Superblock write completed
        inst->sd_write_in_progress = 0;
        return;
    }

    if (num_blocks != inst->blocks_in_progress) {
        // Failed to write all of the blocks to the card, will need to try again
        inst->blocks_in_progress = 0;
        inst->sd_write_in_progress = 0;
        return;
    }

    // Write complete, increment num blocks
    inst->sb.flights[inst->flight].num_blocks += inst->blocks_in_progress;

    // Clear buffer
    inst->buffer[inst->buffer_write_num].count = 0;
    inst->buffer[inst->buffer_write_num].pending_write = 0;

    inst->blocks_in_progress = 0;
    inst->sd_write_in_progress = 0;
    return;
}

/**
 *  Write the superblock to the SD card if necessary.
 *
 *  @param inst Logging service instance descriptor
 *  @param now Whether the superblock needs to be written right way even if the
 *             normal interval has not expired
 *
 *  @return 0 if successful
 */
static int write_superblock(struct logging_desc_t *inst, uint8_t now)
{
    if (inst->sd_write_in_progress || (!now && ((millis - inst->last_sb_write) <
                                                LOGGING_SB_WRITE_INTERVAL))) {
        return 1;
    }

    // Start superblock write
    int const ret = inst->sd_funcs.write(inst->sd_desc, inst->part_start,
                                         1, inst->sb.raw, logging_sd_callback,
                                         inst);

    if (ret == 0) {
        inst->sd_write_in_progress = 1;
        inst->last_sb_write = millis;
    }

    return 0;
}

/**
 *  Write a data buffer to the SD card if needed.
 *
 *  @param inst Logging service instance descriptor
 */
static void write_buffer(struct logging_desc_t *inst)
{
    // If there is an SD card operation ongoing there is nothing else we can do
    // right now
    if (inst->sd_write_in_progress) {
        return;
    }

    // Disable interrupts
    uint32_t const old_primask = __get_PRIMASK();
    __disable_irq();

    // Find any full buffers and mark them as pending write
    uint8_t const current_buf = (uintptr_t)inst->insert_point & 0x3;


    for (int i = 0; i < LOGGING_NUM_BUFFERS; i++) {
        if ((inst->buffer[i].count + LOGGING_WATERMARK) > LOGGING_BUFFER_SIZE) {
            inst->buffer[i].pending_write = 1;

            if ((inst->insert_point != NULL) && (i == current_buf)) {
                // This shouldn't be the active buffer anymore
                inst->insert_point = NULL;
            }
        }
    }

    // Re-enable interrupts
    __set_PRIMASK(old_primask);

    // Check for timeout
    if ((millis - inst->last_data_write) >= LOGGING_BUFFER_WRITE_INTERVAL) {
        // Make the current buffer ready to write as long as there is data in it
        if (inst->buffer[current_buf].count != 0) {
            inst->buffer[current_buf].pending_write = 1;
        }
    }

    // Check for buffers that are ready to write
    uint8_t buf;
    for (buf = 0; buf < LOGGING_NUM_BUFFERS; buf++) {
        if (inst->buffer[buf].pending_write &&
            (inst->buffer[buf].checkout_count == 0)) {
            break;
        }
    }

    if (buf == LOGGING_NUM_BUFFERS) {
        // No buffer ready to be written
        return;
    }

    // Get buffer ready to write
    uint16_t blocks_to_write = ((inst->buffer[buf].count +
                                 (SD_BLOCK_LENGTH - 1)) / SD_BLOCK_LENGTH);
    uint16_t extra_bytes;
    uint32_t const free_blocks = (inst->part_blocks -
                                  (inst->sb.flights[inst->flight].first_block +
                                   inst->sb.flights[inst->flight].num_blocks));
    if (blocks_to_write > free_blocks) {
        // Not enough free blocks, cap blocks_to_write
        blocks_to_write = free_blocks;
        extra_bytes = 0;
    } else {
        // Calculate how many unused bytes we have in the last block
        extra_bytes = ((blocks_to_write * SD_BLOCK_LENGTH) -
                       inst->buffer[buf].count);

        if (extra_bytes != 0) {
            // Need to add a spacer to take up the rest of the last SD card
            // block
            logging_block_marshal_header(inst->buffer[buf].data +
                                         inst->buffer[buf].count,
                                         LOGGING_BLOCK_CLASS_METADATA,
                                         LOGGING_METADATA_TYPE_SPACER,
                                         extra_bytes);
            // Zero out everything after the spacer header
            memset(inst->buffer[buf].data + inst->buffer[buf].count + 4, 0,
                   extra_bytes - 4);
            inst->buffer[buf].count += extra_bytes;
        }
    }

    if (blocks_to_write == 0) {
        // We have run out of space to write blocks
        inst->state = LOGGING_OUT_OF_SPACE;
        write_superblock(inst, 1);
        return;
    }

    // Start writing block
    inst->sd_write_in_progress = 1;
    inst->blocks_in_progress = blocks_to_write;
    inst->buffer_write_num = buf;

    uint32_t const addr = (inst->part_start +
                           inst->sb.flights[inst->flight].first_block +
                           inst->sb.flights[inst->flight].num_blocks);
    int const ret = inst->sd_funcs.write(inst->sd_desc, addr,
                                blocks_to_write, inst->buffer[buf].data,
                                logging_sd_callback, inst);

    if (ret != 0) {
        // Could not start write
        inst->blocks_in_progress = 0;
        inst->sd_write_in_progress = 0;
    } else {
        inst->last_data_write = millis;
    }
}

void logging_service(struct logging_desc_t *inst)
{
    int ret;

    switch (inst->state) {
        case LOGGING_GET_MBR:
            ret = inst->sd_funcs.read(inst->sd_desc, 0, 1, inst->mbr_buffer,
                                      logging_sd_callback, inst);

            if (ret == 0) {
                inst->state = LOGGING_MBR_WAIT;
            }
            break;
        case LOGGING_MBR_WAIT:
            break;
        case LOGGING_MBR_PARSE:
            // MBR has been retrieved and stored in inst->mbr_buffer

            // Check if MBR is valid
            if (!mbr_is_valid(inst->mbr_buffer)) {
                inst->state = LOGGING_NO_VALID_MBR;
                return;
            }

            // Search for valid partition
            uint8_t p;
            uint8_t const *partition_entry;
            for (p = 0; p < MBR_MAX_NUM_PARTITIONS; p++) {
                partition_entry = mbr_get_partition_entry(inst->mbr_buffer, p);
                if (mbr_part_is_valid(partition_entry) &&
                    (mbr_part_type(partition_entry) ==
                     MBR_PART_TYPE_CUINSPACE)) {
                    break;
                }
            }

            if (p == MBR_MAX_NUM_PARTITIONS) {
                inst->state = LOGGING_NO_VALID_PARTITION;
                return;
            }

            // Parse partition
            inst->part_start = mbr_part_first_sector_lba(partition_entry);
            inst->part_blocks = mbr_part_num_sectors(partition_entry);
            inst->state = LOGGING_GET_SUPERBLOCK;
            /* fall-through */
        case LOGGING_GET_SUPERBLOCK:
            ret = inst->sd_funcs.read(inst->sd_desc, inst->part_start, 1,
                                      inst->sb.raw, logging_sd_callback,
                                      inst);

            if (ret == 0) {
                inst->state = LOGGING_SUPERBLOCK_WAIT;
            }
            break;
        case LOGGING_SUPERBLOCK_WAIT:
            break;
        case LOGGING_SUPERBLOCK_PARSE:
            // Check that magic numbers are correct
            if (strncmp(LOGGING_SB_MAGIC, inst->sb.magic, 8) != 0) {
                inst->state = LOGGING_NO_VALID_PARTITION;
                return;
            }
            if (strncmp(LOGGING_SB_MAGIC, inst->sb.magic2, 8) != 0) {
                inst->state = LOGGING_NO_VALID_PARTITION;
                return;
            }

            // Check version
            if (inst->sb.version != LOGGING_FORMAT_VERSION) {
                inst->state = LOGGING_NO_VALID_PARTITION;
                return;
            }

            // Find the next unused flight (or the most recently used flight)
            for (inst->flight = 0; inst->flight < LOGGING_SB_NUM_FLIGHTS;
                 inst->flight++) {
                if (inst->sb.flights[inst->flight].first_block == 0) {
                    break;
                }
            }

            if (inst->continue_flight && (inst->flight > 0)) {
                inst->flight -= 1;
            } else if (inst->flight == LOGGING_SB_NUM_FLIGHTS) {
                // No free flights
                inst->state = LOGGING_NO_VALID_PARTITION;
                return;
            }

            // Initialize flight slot
            if (!inst->continue_flight) {
                if (inst->flight != 0) {
                    // Start the data for this flight right after the data for
                    // the last flight
                    inst->sb.flights[inst->flight].first_block =
                            (inst->sb.flights[inst->flight - 1].first_block +
                             inst->sb.flights[inst->flight - 1].num_blocks);
                }

                if (inst->sb.flights[inst->flight].first_block == 0) {
                    // Don't let the data for a flight overwrite the superblock
                    inst->sb.flights[inst->flight].first_block = 1;
                }

                inst->sb.flights[inst->flight].num_blocks = 0;
                inst->sb.flights[inst->flight].timestamp = 0;
            }

            if (inst->should_pause) {
                inst->state = LOGGING_PAUSED;
                inst->should_pause = 0;
                return;
            }

            inst->state = LOGGING_ACTIVE;
            /* fall-through */
        case LOGGING_ACTIVE:
            // Check if we need to write some blocks
            write_superblock(inst, 0);
            write_buffer(inst);
            break;
        case LOGGING_PAUSED:
            break;
        case LOGGING_TOO_MANY_SD_RETRIES:
        case LOGGING_NO_VALID_MBR:
        case LOGGING_NO_VALID_PARTITION:
        case LOGGING_OUT_OF_SPACE:
        case LOGGING_FAILED:
        default:
            break;
    }
}

void logging_pause(struct logging_desc_t *inst)
{
    if (inst->state == LOGGING_ACTIVE) {
        inst->state = LOGGING_PAUSED;
        write_superblock(inst, 1);
    } else {
        // Indicate that we should go to the paused state next instead of active
        inst->should_pause = 1;
    }
}

void logging_resume(struct logging_desc_t *inst)
{
    inst->state = LOGGING_ACTIVE;
}

void logging_set_timestamp(struct logging_desc_t *inst, uint32_t timestamp)
{
    if (inst->sb.flights[inst->flight].timestamp == 0) {
        inst->sb.flights[inst->flight].timestamp = timestamp;
    }
}



/**
 *  Switch to a new buffer.
 *
 *  @param inst Logging service instance descriptor
 *  @param required_length The number of bytes required right away in the new
 *                         buffer
 *  @param new_insert_point Pointer to where new insert point will be stored
 *
 *  @return 0 if successful
 */
static int select_buffer(struct logging_desc_t *inst, uint16_t required_length,
                         uintptr_t *new_insert_point)
{
    // Grab the current insert point
    uintptr_t ip = (uintptr_t)__atomic_load_n(&inst->insert_point, __ATOMIC_SEQ_CST);

    for (;;) {
        // Get current buffer index from lower bits of insert point
        uint8_t const cur_buf_idx = ip & (uintptr_t)0x3;
        // Clear lower bits of insert point to get proper pointer
        ip &= ~((uintptr_t)0x3);

        // If the current insert point is valid we might be able to use it
        if (ip != (uintptr_t)NULL) {
            // Check if we have enough space in the current buffer
            uint32_t const curr_count = ip - (uintptr_t)inst->buffer[cur_buf_idx].data;
            if ((curr_count + required_length) <= LOGGING_BUFFER_SIZE) {
                // No need to select a different buffer
                break;
            }
        }

        // Search for an empty buffer we can use
        uint8_t buf_idx = (cur_buf_idx + 1) % LOGGING_NUM_BUFFERS;
        while (buf_idx != cur_buf_idx) {
            if (inst->buffer[buf_idx].count == 0) {
                break;
            }
            buf_idx = (buf_idx + 1) % LOGGING_NUM_BUFFERS;
        }

        if (buf_idx == cur_buf_idx) {
            // No empty buffers where available
            return 1;
        }

        // Switch to the chosen buffer
        uintptr_t const new_ip = (uintptr_t)inst->buffer[buf_idx].data | buf_idx;

        // Try to update the insert point to make space in the buffer
        ip |= cur_buf_idx;
        if (__atomic_compare_exchange_n(&inst->insert_point, &ip, new_ip, 0,
                                        __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
            if (inst->buffer[cur_buf_idx].count != 0) {
                inst->buffer[cur_buf_idx].pending_write = 1;
            }
            break;
        }
        // __atomic_compare_exchange_n will have updated ip
    }

    *new_insert_point = (uintptr_t)inst->insert_point;
    return 0;
}




int log_data(struct logging_desc_t *inst, uint8_t const *data, uint16_t length)
{
    struct logging_gather_element gather = {
        .data = data,
        .length = length
    };

    return log_gather(inst, &gather, 1);
}

int log_gather(struct logging_desc_t *inst,
               struct logging_gather_element *gather_list, uint8_t num_segments)
{
    // Calculate full length of data to be logged
    uint16_t length = 0;
    for (uint8_t i = 0; i < num_segments; i++) {
        if (__builtin_add_overflow(length, gather_list[i].length, &length)) {
            return 1;
        }
    }

    // Allocate buffer space
    uint8_t *buffer;
    int ret = log_checkout(inst, &buffer, length);

    if (ret != 0) {
        return 1;
    }

    // Copy data into buffer
    uint16_t offset = 0;
    for (uint8_t i = 0; i < num_segments; i++) {
        memcpy(buffer + offset, gather_list[i].data, gather_list[i].length);
        offset += gather_list[i].length;
    }

    // Release checkout on buffer
    log_checkin(inst, buffer);

    return 0;
}

int log_checkout(struct logging_desc_t *inst, uint8_t **data, uint16_t length)
{
    // Grab the current insert point
    uintptr_t ip = (uintptr_t)__atomic_load_n(&inst->insert_point,
                                              __ATOMIC_SEQ_CST);

    for (;;) {
        // Get current buffer index from lower bits of insert point
        uint8_t const buf_idx = ip & (uintptr_t)0x3;
        // Clear lower bits of insert point to get proper pointer
        ip &= ~((uintptr_t)0x3);

        // Check that the insert point is valid and there is enough space in the
        // current buffer
        uint32_t const current_count = ip - (uintptr_t)inst->buffer[buf_idx].data;
        if ((ip == (uintptr_t)NULL) || ((current_count + length) >
                                        LOGGING_BUFFER_SIZE)) {
            int const ret = select_buffer(inst, length, &ip);

            if (ret != 0) {
                // No buffers available
                inst->out_of_space_count++;
                return 1;
            }
            continue;
        }

        // Figure out what we need to update the insert point to
        uintptr_t const new_ip = (ip + length) | buf_idx;

        // Checkout buffer
        __atomic_add_fetch(&inst->buffer[buf_idx].checkout_count, 1,
                           __ATOMIC_SEQ_CST);

        // Try and update the insert point to make space in the buffer
        ip |= buf_idx;
        if (__atomic_compare_exchange_n(&inst->insert_point, &ip, new_ip, 0,
                                        __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
            // Update count
            __atomic_add_fetch(&inst->buffer[buf_idx].count, length,
                               __ATOMIC_SEQ_CST);
            // Successfully reserved space in buffer
            break;
        }

        // Release checkout on buffer since we might end up using a different
        // buffer next time through the loop
        __atomic_sub_fetch(&inst->buffer[buf_idx].checkout_count, 1,
                           __ATOMIC_SEQ_CST);
        // __atomic_compare_exchange_n will have updated ip
    }

    *data = (uint8_t*)(ip & ~0x3);
    return 0;
}

int log_checkin(struct logging_desc_t *inst, uint8_t *data)
{
    // Figure out which buffer data is from
    for (int i = 0; i < LOGGING_NUM_BUFFERS; i++) {
        uint8_t *const buffer_end = inst->buffer[i].data + LOGGING_BUFFER_SIZE;
        if ((data >= inst->buffer[i].data) && (data < buffer_end)) {
            // Release checkout on buffer
            __atomic_sub_fetch(&inst->buffer[i].checkout_count, 1,
                               __ATOMIC_SEQ_CST);
            return 0;
        }
    }
    return 1;
}
