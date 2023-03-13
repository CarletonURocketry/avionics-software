/**
 * @file sdhc.h
 * @desc Driver for SD host controller
 * @author Samuel Dewan
 * @date 2021-07-01
 * Last Author:
 * Last Edited On:
 */

#ifndef sdhc_h
#define sdhc_h

#include "global.h"
#include "sd/sd.h"


#define SDHC_ADMA2_DESC_ACT_NOP_Val     0b00
#define SDHC_ADMA2_DESC_ACT_TRAN_Val    0b10
#define SDHC_ADMA2_DESC_ACT_LINK_Val    0b11

#define SDHC_ADMA2_DESC_VALID       (1 << 0)
#define SDHC_ADMA2_DESC_END         (1 << 1)
#define SDHC_ADMA2_DESC_INTERRUPT   (1 << 2)
#define SDHC_ADMA2_DESC_ACT(x)      ((x & 0x3) << 4)
#define SDHC_ADMA2_DESC_ACT_NOP     SDHC_ADMA2_DESC_ACT(SDHC_ADMA2_DESC_ACT_NOP_Val)
#define SDHC_ADMA2_DESC_ACT_TRAN    SDHC_ADMA2_DESC_ACT(SDHC_ADMA2_DESC_ACT_TRAN_Val)
#define SDHC_ADMA2_DESC_ACT_LINK    SDHC_ADMA2_DESC_ACT(SDHC_ADMA2_DESC_ACT_LINK_Val)

struct sdhc_adma2_descriptor32 {
    union {
        struct {
            uint16_t valid:1;
            uint16_t end:1;
            uint16_t interrupt:1;
            uint16_t RESERVED:1;
            uint16_t act:2;
            uint16_t RESERVED1:10;
        } bits;
        uint16_t raw;
    } attributes;
    uint16_t    length;
    uint32_t    address;
};


enum sdhc_state {
    //
    // Misc. States
    //
    /** No card present */
    SDHC_NOT_PRESENT,
    /** Nothing to do */
    SDHC_IDLE,
    /** Send CMD55 to indicate that next command will be an app comand */
    SDHC_APP_CMD,
    /** Send CMD12 to abort failed command */
    SDHC_ABORT,
    //
    //  Initialization States
    //
    /** Send CMD0 to reset card */
    SDHC_RESET,
    /** Send CMD8 to check interface conditions */
    SDHC_CHECK_VOLTAGE,
    /** Send ACMD41 to read OCR and check supported voltage ranges */
    SDHC_CHECK_OCR,
    /** Send ACMD41 to initialize card */
    SDHC_INITIALIZE,
    /** Send CMD2 to get card ID and go into IDENT state */
    SDHC_GET_CID,
    /** Send CMD3 to get Relative Address (RCA) for card */
    SDHC_GET_RCA,
    /** Send CMD9 to read Card Specific Data register  */
    SDHC_READ_CSD,
    /** Send CMD7 to select card and go into TRAN state */
    SDHC_SELECT,
    // At this point we can switch to a 25 MHz clock
    /** Send CMD6 to switch to high speed mode */
    SDHC_SET_HIGH_SPEED,
    // At this point we must switch to HS mode and can switch to a 50 MHz clock
    /** Send ACMD6 to switch to 4 bit bus mode */
    SDHC_SET_4_BIT,
    // At this point we must switch to a 4 bit wide bus
    /** Send ACMD51 to read SD Card Configuration register */
    SDHC_READ_SCR,
    /** Send CMD16 to set block length to 512 bytes */
    SDHC_SET_BLOCK_LEN,
    // At this point we can lengthen the command timeout to 500 ms and allow
    // SDCLK to be disabled immediatly after each transaction
    /** Cleanup initialization and go to idle */
    SDHC_INIT_DONE,
    //
    //  Read States
    //
    /** Read a single block with CMD17 or multiple blocks with CMD18 */
    SDHC_READ,
    //
    //  Write States
    //
    /** Write a single block with CMD24 or multiple blocks with CMD25 */
    SDHC_WRITE,
    /** Get the number of blocks that were written with ACMD22 */
    SDHC_GET_NUM_BLOCKS_WRITTEN,
    //
    //  Failure States
    //
    /** Failure state for when card is not supported */
    SDHC_UNUSABLE_CARD,
    /** Failure state for when we exceed the retry count while initializing
        card */
    SDHC_TOO_MANY_INIT_RETRIES,
    /** Initialization command (ACMD41) failed */
    SDHC_INIT_TIMEOUT,
    /** Failure state for all other failures */
    SDHC_FAILED
};


struct sdhc_desc_t {
    uint8_t buffer[64];

    struct sdhc_adma2_descriptor32 adma2_desc;

    /** SD Host Controller Instance */
    Sdhc *sdhc;

    /** The time at which the current command or block read/write was started,
        used for timeout */
    uint32_t cmd_start_time;

    uint32_t clock_freq;

    /** Capacity of card in blocks */
    uint32_t card_capacity;

    /** Address for read or write operation */
    uint32_t op_addr;
    /** Total number of blocks for read or write operation */
    uint16_t block_count;
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

    uint16_t rca;

    /** Counter for command retries during initialization process */
    union {
        uint8_t init_retry_count;
        uint8_t op_retry_count;
    };

    /** Current driver state */
    enum sdhc_state state:5;
    /** The driver state to go into after SDHC_APP_CMD */
    enum sdhc_state acmd_state:5;
    /** The driver state to go into after SDHC_ABORT */
    enum sdhc_state abort_recovery_state:5;
    uint8_t substate:4;
    uint8_t waiting_for_interrupt:1;
    uint8_t init_cmd_started:1;
    /** Flag to indicate that the connected card is old */
    uint8_t v1_card:1;
    /** Flag to indicate that the connected card is block rather than byte
        addressed */
    uint8_t block_addressed:1;

    uint8_t cmd23_supported:1;

    uint8_t enable_high_speed:1;
    uint8_t enable_4_bit:1;
};



enum sdhc_status {
    SDHC_STATUS_NO_CARD,
    SDHC_STATUS_UNUSABLE_CARD,
    SDHC_STATUS_TOO_MANY_INIT_RETRIES,
    SDHC_STATUS_INIT_TIMEOUT,
    SDHC_STATUS_FAILED,
    SDHC_STATUS_INITIALIZING,
    SDHC_STATUS_READY
};

/** Standard set of functions for accessing SD card through this driver. */
extern struct sd_funcs const sdhc_sd_funcs;

/**
 *  Initialize SDHC driver.
 *
 *  @param inst Pointer to driver instance structure to initialize
 *  @param sdhc SD Host Controller hardware instance
 *  @param clock_freq Frequency of generic clock
 *  @param clock_mask Mask to select generic clock
 *  @param enable_high_speed Whether high speed mode should be used
 *  @param enable_4_bit Whether 4 bit mode should be used
 */
extern void init_sdhc(struct sdhc_desc_t *inst, Sdhc *sdhc, uint32_t clock_freq,
                      uint32_t clock_mask, int enable_high_speed,
                      int enable_4_bit);

/**
 *  Service to be run in each iteration of the main loop.
 *
 *  @param inst The SDHC driver instance for which the service should be run
 */
extern void sdhc_service(struct sdhc_desc_t *inst);

/**
 *  Get the current status of the sdspi driver.
 *
 *  @param inst The SDHC driver instance for which the status should be found
 *
 *  @return The current driver status
 */
extern enum sdhc_status sdhc_get_status(struct sdhc_desc_t *inst);

#endif /* sdhc_h */
