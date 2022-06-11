/**
 * @file logging.h
 * @desc Service which handles logging of data to an SD card
 * @author Samuel Dewan
 * @date 2021-07-11
 * Last Author:
 * Last Edited On:
 */

#ifndef logging_h
#define logging_h

#include "global.h"
#include "sd.h"

#include "target.h" // For LOGGING_BUFFER_SIZE

#include "logging-format.h"



#define LOGGING_NUM_BUFFERS 2


/**
 *  Single element in gather list used for passing data to the logging service
 *  that is spread out across multiple buffers.
 */
struct logging_gather_element {
    uint8_t const *data;
    uint16_t length;
};


enum logging_state {
    LOGGING_GET_MBR,
    LOGGING_MBR_WAIT,
    LOGGING_MBR_PARSE,
    LOGGING_GET_SUPERBLOCK,
    LOGGING_SUPERBLOCK_WAIT,
    LOGGING_SUPERBLOCK_PARSE,
    LOGGING_ACTIVE,
    LOGGING_PAUSED,
    LOGGING_TOO_MANY_SD_RETRIES,
    LOGGING_NO_VALID_MBR,
    LOGGING_NO_VALID_PARTITION,
    LOGGING_OUT_OF_SPACE,
    LOGGING_FAILED
};

struct logging_desc_t {
    /* Buffers and associated state */
    struct {
        /** Buffer */
        uint8_t data[LOGGING_BUFFER_SIZE];
        /** Number of valid bytes currently in buffer */
        uint16_t count;
        /** Number of active checkouts for buffer */
        uint8_t checkout_count;
        /** Whether the buffer is ready to be written */
        uint8_t pending_write:1;
    } buffer[LOGGING_NUM_BUFFERS];

    union {
        /** Buffer into which MBR can be read */
        uint8_t mbr_buffer[512];
        /** In memory copy of superblock */
        union logging_superblock sb;
    };

    /** Descriptor for SD card driver */
    sd_desc_ptr_t sd_desc;
    /** Access function for SD card driver */
    struct sd_funcs sd_funcs;

    /** Pointer to where in the buffers data should be placed next. The lower
        two bits of this value are used to store the current buffer number. */
    uint8_t *insert_point;
    /** Address of first block in partition */
    uint32_t part_start;
    /** Number of blocks in partition */
    uint32_t part_blocks;

    /** Last time that a buffer of data was written to the SD card */
    uint32_t last_data_write;
    /** Last time that the SD card's superblock was updated */
    uint32_t last_sb_write;

    /** Count for the number of times we have tried to add data to the buffer
        that there wasn't enough space for */
    uint32_t out_of_space_count;

    /** Number of blocks that are being written in the current SD write
        operation */
    uint16_t blocks_in_progress;

    union {
        /** The current flight number */
        uint8_t flight;
        /** Number of SD card operation retries during initialization */
        uint8_t init_retry_count;
    };

    /** Current service state */
    enum logging_state state:4;
    /** Whether we should continue the last flight or start a new one */
    uint8_t continue_flight:1;
    /** Whether an SD card write operation is ongoing */
    uint8_t sd_write_in_progress:1;
    /** Which buffer is currently being written to the SD card */
    uint8_t buffer_write_num:2;
    /** Whether the logging service should be paused as soon as it reaches the
        active state */
    uint8_t should_pause:1;
};

/**
 *  Initialize the logging service.
 *
 *  @param inst Logging service instance descriptor
 *  @param sd_desc SD card driver instance
 *  @param sd_funcs SD card driver functions
 *  @param continue_flight Whether we should continue the last flight or start a
 *                         new one
 */
extern void init_logging(struct logging_desc_t *inst,
                         sd_desc_ptr_t sd_desc, struct sd_funcs sd_funcs,
                         uint8_t continue_flight);

/**
 *  Service function to be run in each iteration of the main loop.
 *
 *  @param inst Logging service instance descriptor
 */
extern void logging_service(struct logging_desc_t *inst);

/**
 *  Temporarily stop writing data to the SD card.
 *
 *  @note This function will trigger a superblock write right away.
 *
 *  @param inst Logging service instance descriptor
 */
extern void logging_pause(struct logging_desc_t *inst);

/**
 *  Start writing data to the SD card again.
 *
 *  @param inst Logging service instance descriptor
 */
extern void logging_resume(struct logging_desc_t *inst);

/**
 *  Set the timestamp for the current flight if it has not already been set.
 *
 *  @param inst Logging service instance descriptor
 *  @param timestamp Current UTC time in seconds
 */
extern void logging_set_timestamp(struct logging_desc_t *inst,
                                  uint32_t timestamp);


/**
 *  Log a buffer of data to the SD card.
 *
 *  @param inst Logging service instance descriptor
 *  @param data The data to be logged
 *  @param length The number of bytes to be logged
 *
 *  @return 0 if successful
 */
extern int log_data(struct logging_desc_t *inst, uint8_t const *data,
                    uint16_t length);

/**
 *  Log data from one or more scattered buffers to the SD card.
 *
 *  @param inst Logging service instance descriptor
 *  @param gather_list List of buffers to be logged
 *  @param num_segments The number of buffers in the list
 *
 *  @return 0 if successful
 */
extern int log_gather(struct logging_desc_t *inst,
                      struct logging_gather_element *gather_list,
                      uint8_t num_segments);

/**
 *  Checkout a buffer to copy data to be logged into. The checked out buffer
 *  will be reserved. The buffer will not be written to the SD card until it is
 *  checked back in.
 *
 *  @param inst Logging service instance descriptor
 *  @param data Pointer to where pointer to buffer will be stored
 *  @param length Size of buffer to check out
 *
 *  @return 0 if successful
 */
extern int log_checkout(struct logging_desc_t *inst, uint8_t **data,
                        uint16_t length);

/**
 *  Checking a buffer that had been previously checked out.
 *
 *  @param inst Logging service instance descriptor
 *  @param data The buffer to be checked in
 *
 *  @return 0 if successful
 */
extern int log_checkin(struct logging_desc_t *inst, uint8_t *data);

/**
 *  Get the number of blocks written as part of the current flight.
 */
static inline uint32_t log_get_curr_flight_blocks(
                                    const struct logging_desc_t *const inst)
{
    return inst->sb.flights[inst->flight].num_blocks;
}

/**
 *  Get the number of checkouts that have been dropped because of insufficent
 *  buffer space.
 */
static inline uint32_t log_get_num_missed_checkouts(
                                    const struct logging_desc_t *const inst)
{
    return inst->out_of_space_count;
}

/**
 *  Get the SD card driver instance used by this logging instance.
 */
static inline sd_desc_ptr_t log_get_sd_desc(
                                    const struct logging_desc_t *const inst)
{
    return inst->sd_desc;
}

/**
 *  Get the SD card access functions used by this logging instance.
 */
static inline const struct sd_funcs *log_get_sd_funcs(
                                    const struct logging_desc_t *const inst)
{
    return &inst->sd_funcs;
}

#endif /* logging_h */
