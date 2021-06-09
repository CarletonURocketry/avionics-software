/**
 * @file sdspi.h
 * @desc Driver for interacting with SD card via SPI interface.
 * Last Author:
 * Last Edited On:
 */

#ifndef sdspi_h
#define sdspi_h

#include "global.h"
#include "sd.h"
#include "sercom-spi.h"
#include "gpio.h"

enum sdspi_state {
    /** Waiting for SD card to be inserted (also includes a wait of at least a
        millisecond for glitch filer/startup time) */
    SDSPI_NOT_PRESENT,
    /** Send least 74 clock cycles at 400 KHz with CS high */
    SDSPI_INIT_CYCLES,
    /** Send CMD0 (soft reset) - repeat until response is 0x01 */
    SDSPI_SOFT_RESET,
    /** Send CMD8 (supply voltage info) */
    SDSPI_SEND_HOST_VOLT_INFO,
    /** Send CMD59 (set CRC) */
    SDSPI_SET_CRC,
    /** Send CMD55 (indicates next command is application specific command) */
    SDSPI_NEXT_CMD_APP_SPECIFIC,
    /** Send ACMD41 (send host capacity support info and init card) -
        repeat CMD55 and ACMD41 until response is valid*/
    SDSPI_INIT_CARD,
    /** Send CMD1 (send host capacity support info and init card) -
        repeat until response is valid */
    SDSPI_INIT_V1_CARD,
    /** Send CMD58 (read operating conditions register) */
    SDSPI_READ_OCR,
    /** Send CMD9 (read card specific data register) */
    SDSPI_READ_CSD,
    /** Get the block of data sent in response to CMD9 */
    SDSPI_READ_CSD_READ_BLOCK,
    /** Send CMD16 (set block length) */
    SDSPI_SET_BLOCK_LENGTH,

    /** Nothing to do */
    SDSPI_IDLE,

    /** Send CMD17 or CMD18 to start reading a block or multiple blocks */
    SDSPI_START_READ,
    /** Read start tokens, blocks and CRCs from card and send CMD12 to stop
        reading if needed */
    SDSPI_READ_BLOCKS,
    /** Get the response from CMD12 */
    SDSPI_READ_GET_STOP_RSP,

    /** Send CMD24 or CMD25 to start writing a block or mutliple blocks */
    SDSPI_START_WRITE,
    /** Send blocks to card with block start tokens and CRCs */
    SDSPI_WRITE_BLOCKS,
    /** Send stop token to indicate that no more blocks will be sent */
    SDSPI_WRITE_SEND_STOP_TOKEN,
    /** Send CMD13 to get status from write operation */
    SDSPI_WRITE_GET_STATUS,

    /** Failure state for when card is not supported */
    SDSPI_UNUSABLE_CARD,
    /** Failure state for when we exceed the retry count while initializing
        card */
    SDSPI_TOO_MANY_INIT_RETRIES,
    /** Failure state for all other failures */
    SDSPI_FAILED
};


struct sdspi_desc_t {
    /** SPI instance used to communicate with card */
    struct sercom_spi_desc_t *spi_inst;
    /** Mask for card's chip select pin */
    uint32_t cs_pin_mask;

    /** The time at which the current command or block read/write was started,
        used for timeout */
    uint32_t cmd_start_time;

    union {
        // Only needed during SDSPI_NOT_PRESENT state
        /** Time at which card was first detected, used for card insert glitch
            filter */
        uint32_t card_detect_time;
        // Only needed after initialization is done
        /** Capacity of card in blocks */
        uint32_t card_capacity;
    };

    /** Address for read or write operation */
    uint32_t op_addr;
    /** Total number of blocks for read or write operation */
    uint32_t block_count;
    /** Number of completed blocks for read or write operation */
    uint32_t blocks_done;
    /** Callback function to be called when operation is complete */
    sd_op_cb_t callback;
    /** Context argument for callback function */
    void *cb_context;

    union {
        /** Buffer where data from read operation should be placed */
        uint8_t *read_buffer;
        /** Buffer from which data should be written in write operation */
        uint8_t const *write_data;
    };

    /** Number of bytes transfered from current block */
    uint16_t bytes_in;

    /** Pin connected to card detect switch on SD card socket */
    union gpio_pin_t card_detect_pin;

    /** Buffer used for forming commands */
    uint8_t cmd_buffer[8];
    /** Buffer used for some responses */
    uint8_t rsp_buffer[16];

    /** Counter for command retries during initialization process */
    uint8_t init_retry_count;

    /** ID to keep track of SPI transactions */
    uint8_t spi_tid;

    /** Group in which card's chip select pin is located */
    uint8_t cs_pin_group;

    /** Current driver state */
    enum sdspi_state state:5;
    /** The driver state to go into after SDSPI_NEXT_CMD_APP_SPECIFIC */
    enum sdspi_state acmd_state:5;
    /** The current driver substate */
    uint8_t substate:3;
    /** Flag to indicate that an SPI transaction is in progress */
    uint8_t spi_in_progress:1;
    /** Flag to indicate that an SPI session is open */
    uint8_t spi_session_open:1;
    /** Flag to indicate that an SPI card is connected */
    uint8_t card_present:1;
    /** Flag to indicate that the connected card is old */
    uint8_t v1_card:1;
    /** Flag to indicate that the connected card is block rather than byte
        addressed */
    uint8_t block_addressed:1;
};



enum sdspi_status {
    SDSPI_STATUS_NO_CARD,
    SDSPI_STATUS_UNUSABLE_CARD,
    SDSPI_STATUS_TOO_MANY_INIT_RETRIES,
    SDSPI_STATUS_TOO_MANY_TIMEOUTS,
    SDSPI_STATUS_FAILED,
    SDSPI_STATUS_INITIALIZING,
    SDSPI_STATUS_READY
};

/** Standard set of functions for accessing SD card through this driver. */
extern struct sd_funcs const sdspi_sd_funcs;

/**
 *  Initialize sdspi driver.
 *
 *  @param inst Pointer to instance structure to initialize
 *  @param spi_inst sercom spi driver instance to be used
 *  @param cs_pin_mask Bitmask for chip select pin
 *  @param cs_pin_group Group number for chip select pin
 *  @param card_detect_pin Pin connected to SD card socket's card detect switch
 */
extern void init_sdpsi(struct sdspi_desc_t *inst,
                       struct sercom_spi_desc_t *spi_inst, uint32_t cs_pin_mask,
                       uint8_t cs_pin_group, union gpio_pin_t card_detect_pin);

/**
 *  Service to be run in each iteration of the main loop.
 *
 *  @param inst The sdspi driver instance for which the service should be run
 */
extern void sdspi_service(struct sdspi_desc_t *inst);

/**
 *  Get the current status of the sdspi driver.
 *
 *  @param inst The sdspi driver instance for which the status should be found
 *
 *  @return The current driver status
 */
extern enum sdspi_status sdspi_get_status(struct sdspi_desc_t *inst);


#endif /* sdspi_h */
