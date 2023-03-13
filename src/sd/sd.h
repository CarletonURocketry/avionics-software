/**
 * @file sd.h
 * @desc Interface to use SDSPI or SDMMC drivers interchangably.
 * @author Samuel Dewan
 * @date 2021-06-06
 * Last Author:
 * Last Edited On:
 */

#ifndef sd_h
#define sd_h

#include "global.h"

#define SD_BLOCK_LENGTH 512

/**
 *  Represents the possible statuses for an SD card operation.
 */
enum sd_op_result {
    SD_OP_SUCCESS,
    SD_OP_FAILED
};

/** Callback function for SD card read or write operation */
typedef void (*sd_op_cb_t)(void *context, enum sd_op_result result,
                           uint32_t num_blocks);



// The sdspi.h and sdhc.h need to be included here because the above type are
// required by those headers and those headers are required for the below types.
#include "sdspi.h"
#if defined(SAMx5x)
#include "sdhc.h"
#endif

/**
 *  Transparent union to allow SD card functions to accept either an sdspi
 *  driver instance or an sdmmc driver instance.
 */
typedef union __attribute__ ((__transparent_union__)) {
    struct sdspi_desc_t *sdspi;
#if defined(SAMx5x)
    struct sdhc_desc_t *sdhc;
#endif
} sd_desc_ptr_t;

/**
 *  Represents possible SD card driver statuses.
 */
enum sd_status {
    SD_STATUS_NOT_PRESENT,
    SD_STATUS_INITIALIZING,
    SD_STATUS_READY,
    SD_STATUS_FAILED
};

/**
 *  Set of function pointers provided by an SD driver to access an SD card.
 */
struct sd_funcs {
    /**
     *  Function to read from SD card.
     *
     *  @note   It is important to be aware that the callback function may be
     *          called from an interrupt context.
     *
     *  @param inst The pointer to driver instance for type of SD card driver
     *              to which this function belongs
     *  @param addr Address to be read from in blocks, the address passed to
     *              this function is always in blocks, even for older cards that
     *              are byte addressed
     *  @param num_blocks The number of 512 byte blocks to be read
     *  @param buffer The buffer into which the read data should be placed, must
     *                be at least 512 * num_blocks bytes long, must remain valid
     *                until callback function is called
     *  @param cb Callback function for when read operation is complete
     *  @param context Pointer that will be passed to callback function
     *
     *  @return Zero if successfully started or queued to be started, a non-zero
     *          value otherwise
     */
    int (*read)(sd_desc_ptr_t inst, uint32_t addr, uint32_t num_blocks,
                uint8_t *buffer, sd_op_cb_t cb, void *context);
    /**
     *  Function to write to SD card.
     *
     *  @note   It is important to be aware that the callback function may be
     *          called from an interrupt context.
     *
     *  @param inst The pointer to driver instance for type of SD card driver
     *              to which this function belongs
     *  @param addr Address to be written to in blocks, the address passed to
     *              this function is always in blocks, even for older cards that
     *              are byte addressed
     *  @param num_blocks The number of 512 byte blocks to be written
     *  @param data The data to be written to the card, must be 512 * num_blocks
     *              bytes long, must remain valid until callback function is
     *              called
     *  @param cb Callback function for when write operation is complete
     *  @param context Pointer that will be passed to callback function
     *
     *  @return Zero if successfully started or queued to be started, a non-zero
     *          value otherwise
     */
    int (*write)(sd_desc_ptr_t inst, uint32_t addr, uint32_t num_blocks,
                 uint8_t const *data, sd_op_cb_t cb, void *context);
    /**
     *  Function to get SD card driver state.
     *
     *  @param inst The pointer to driver instance for type of SD card driver
     *              to which this function belongs
     *
     *  @return The current state of the SD driver
     */
    enum sd_status (*get_status)(sd_desc_ptr_t inst);
    /**
     *  Function to get the number of blocks that the SD card has.
     *
     *  @param inst The pointer to driver instance for type of SD card driver
     *              to which this function belongs
     *
     *  @return The number of blcoks that the SD card has or 0 if the SD card is
     *          not initialized
     */
    uint32_t (*get_num_blocks)(sd_desc_ptr_t inst);
};


#endif /* sd_h */
