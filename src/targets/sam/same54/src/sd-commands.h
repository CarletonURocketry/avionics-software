/**
 * @file sd-commands.h
 * @desc SD Card commands from SD Card Physical Layer Simplified Specification
 *       v3.01
 * @author Samuel Dewan
 * @date 2021-07-01
 * Last Author:
 * Last Edited On:
 */

#ifndef sd_commands_h
#define sd_commands_h

//
//
//  Commands
//
//

enum sd_command_index {
    /** GO_IDLE_STATE - reset card to idle state */
    SD_CMD0 = 0,
    /** ALL_SEND_CID */
    SD_CMD2 = 2,
    /** SEND_RELATIVE_ADDRESS - Ask card to publish new RCA */
    SD_CMD3 = 3,
    /** SET_DSR */
    SD_CMD4 = 4,
    /** SWITCH_FUNC */
    SD_CMD6 = 6,
    /** SELECT_CARD */
    SD_CMD7 = 7,
    /** SEND_IF_COND - verify SD Memory Card interface operating condition.*/
    SD_CMD8 = 8,
    /** SEND_CSD - read the Card Specific Data (CSD register) */
    SD_CMD9 = 9,
    /** SEND_CID - read the card identification information (CID register) */
    SD_CMD10 = 10,
    /** VOLTAGE_SWITCH */
    SD_CMD11 = 11,
    /** STOP_TRANSMISSION - stop transmission in multi-block read mode */
    SD_CMD12 = 12,
    /** SEND_STATUS - read the card status register */
    SD_CMD13 = 13,
    /** GO_INACTIVE_STATE */
    SD_CMD15 = 15,
    /** SET_BLOCKLEN */
    SD_CMD16 = 16,
    /** READ_SINGLE_BLOCK - read a single data block from the card */
    SD_CMD17 = 17,
    /** READ_MULTIPLE_BLOCK */
    SD_CMD18 = 18,
    /** SEND_TUNING_BLOCK */
    SD_CMD19 = 19,
    /** SPEED_CLASS_CONTROL */
    SD_CMD20 = 20,
    /** SET_BLOCK_COUNT - specify block count for CMD18 or CMD25 */
    SD_CMD23 = 23,
    /** WRITE_BLOCK - write a single data block to the card */
    SD_CMD24 = 24,
    /** WRITE_MULTIPLE_BLOCK - write blocks of data until a STOP_TRANSMISSION */
    SD_CMD25 = 25,
    /** PROGRAM_CSD */
    SD_CMD27 = 27,
    /** SET_WRITE_PROT */
    SD_CMD28 = 28,
    /** CLEAR_WRITE_PROT */
    SD_CMD29 = 29,
    /** SEND_WRITE_PROT */
    SD_CMD30 = 30,
    /** ERASE_WR_BLK_START - sets the address of the first block to be erased */
    SD_CMD32 = 32,
    /** ERASE_WR_BLK_END - sets the address of the last block of the continuous
        range to be erased*/
    SD_CMD33 = 33,
    /** ERASE - erase all previously selected blocks */
    SD_CMD38 = 38,
    /** LOCK_UNLOCK */
    SD_CMD42 = 42,
    /** APP_CMD - escape for application specific command */
    SD_CMD55 = 55,
    /** GEN_CMD */
    SD_CMD56 = 56,
    /** READ_OCR - read the OCR register of a card */
    SD_CMD58 = 58,
    /** CRC_ON_OFF - turn the requirement to send the CRC with a command
        on/off */
    SD_CMD59 = 59
};

enum sd_application_command_index {
    /** SET_BUS_WIDTH */
    SD_ACMD6 = 6,
    /** SD_STATUS */
    SD_ACMD13 = 13,
    /** SEND_NUM_WR_BLOCKS -  */
    SD_ACMD22 = 22,
    /** SET_WR_BLK_ERASE_COUNT - Set the number of write blocks to be
        pre-erased before writing */
    SD_ACMD23 = 23,
    /** SD_SEND_OP_COND - Sends host capacity support information and
        activates the card's initialization process */
    SD_ACMD41 = 41,
    /** SET_CLR_CARD_DETECT */
    SD_ACMD42 = 42,
    /** SEND_SCR */
    SD_ACMD51 = 51
};

//
//
//  Command Arguments
//
//

#define SD_FG1_ACCESS_MODE_SDR12    0x0
#define SD_FG1_ACCESS_MODE_SDR25    0x1
#define SD_FG1_ACCESS_MODE_SDR50    0x2
#define SD_FG1_ACCESS_MODE_SDR104   0x3
#define SD_FG1_ACCESS_MODE_DDR50    0x4
#define SD_FG_NO_CHANGE             0xf

/**
 *  See SD Physical Layer Simplified Specification v3.01 - Table 4-28
 */
union sd_cmd6_arg {
    struct {
        uint32_t function_group_1:4;
        uint32_t function_group_2:4;
        uint32_t function_group_3:4;
        uint32_t function_group_4:4;
        uint32_t function_group_5:4;    // Reserved, all 0 or all 1
        uint32_t function_group_6:4;    // Reserved, all 0 or all 1
        uint32_t RESERVED:7;            // All 0
        uint32_t mode:1;                // 0 = check, 1 = switch
    };
    uint32_t raw;
};

#define SD_VHS_27_36    0b0001
#define SD_VHS_LOW      0b0010

/**
 *  See SD Physical Layer Simplified Specification v3.01 - Table 4-19
 */
union sd_cmd8_arg {
    struct {
        uint32_t check_pattern:8;
        uint32_t voltage_supplied:4; // VHS
        uint32_t RESERVED:20;
    };
    uint32_t raw;
};

#define SD_BUS_WIDTH_1  0b00
#define SD_BUS_WIDTH_4  0b10

/**
 *  See SD Physical Layer Simplified Specification v3.01 - Table 4-27
 */
union sd_acmd6_arg {
    struct {
        uint32_t bus_width:2;
        uint32_t RESERVED:30;
    };
    uint32_t raw;
};

/**
 *  See SD Physical Layer Simplified Specification v3.01 - Table 4-27
 */
union sd_acmd23_arg {
    struct {
        uint32_t num_blocks:23;
        uint32_t RESERVED:9;
    };
    uint32_t raw;
};

/**
 *  See SD Physical Layer Simplified Specification v3.01 - Figure 4-3
 */
union sd_acmd41_arg {
    struct {
        uint32_t RESERVED:15 __attribute__((aligned(4)));
        uint32_t volt_range_2V7_2V8:1;      // 2.7 to 2.8 volts
        uint32_t volt_range_2V8_2V9:1;      // 2.8 to 2.9 volts
        uint32_t volt_range_2V9_3V0:1;      // 2.9 to 3.0 volts
        uint32_t volt_range_3V0_3V1:1;      // 3.0 to 3.1 volts
        uint32_t volt_range_3V1_3V2:1;      // 3.1 to 3.2 volts
        uint32_t volt_range_3V2_3V3:1;      // 3.2 to 3.3 volts
        uint32_t volt_range_3V3_3V4:1;      // 3.3 to 3.4 volts
        uint32_t volt_range_3V4_3V5:1;      // 3.4 to 3.5 volts
        uint32_t volt_range_3V5_3V6:1;      // 3.5 to 3.6 volts
        uint32_t s18r:1;                    // Request 1.8 volt signaling
        uint32_t RESERVED1:3;
        uint32_t xpc:1;                     // SDXC power control
        uint32_t fb:1;                      // Fast Boot (for eSD) Should be 0
        uint32_t hcs:1;                     // Host capacity support
        uint32_t busy:1;                    // Should be 0
    };
    uint32_t raw;
};

union sd_rca_arg {
    struct {
        uint16_t RESERVED;
        uint16_t rca;
    };
    uint32_t raw;
};

//
//
//  Command Responses
//
//

/**
 *  See SD Physical Layer Simplified Specification v3.01 - Figure 4-33
 */
union sd_cmd3_rsp { // R6
    struct {
        uint16_t RESERVED_MANUFACTURER_TEST:2 __attribute__((aligned(4)));
        uint16_t RESERVED_APP_SPECIFIC:1;
        uint16_t ake_seq_error:1;       // Error in sequence of auth process
        uint16_t RESERVED_SDIO:1;
        uint16_t app_cmd:1;             // Card expects ACMD or command was ACMD
        uint16_t RESERVED:2;
        uint16_t read_for_data:1;       // Buffer empty
        uint16_t current_state:4;
        uint16_t error:1;
        uint16_t illegal_comand:1;
        uint16_t com_crc_error:1;
        uint16_t rca;
    };
    uint32_t raw;
};

static inline union sd_cmd3_rsp sd_get_cmd3_rsp(SDHC_RR_Type const volatile *RR)
{
    return (union sd_cmd3_rsp){ .raw = RR[0].reg };
}

/**
 *  See SD Physical Layer Simplified Specification v3.01 - Table 4-34
 */
union sd_cmd8_rsp { // R7
    struct {
        uint32_t check_pattern_echo:8;
        uint32_t voltage_accepted:4; // VHS
        uint32_t RESERVED:20;
    };
    uint32_t raw;
};

static inline union sd_cmd8_rsp sd_get_cmd8_rsp(SDHC_RR_Type const volatile *RR)
{
    return (union sd_cmd8_rsp){ .raw = RR[0].reg };
}

/**
 *  See SD Physical Layer Simplified Specification v3.01 - Figure 4-4
 */
union sd_acmd41_rsp {
    struct {
        uint32_t RESERVED:15 __attribute__((aligned(4)));
        uint32_t volt_range_2V7_2V8:1;      // 2.7 to 2.8 volts
        uint32_t volt_range_2V8_2V9:1;      // 2.8 to 2.9 volts
        uint32_t volt_range_2V9_3V0:1;      // 2.9 to 3.0 volts
        uint32_t volt_range_3V0_3V1:1;      // 3.0 to 3.1 volts
        uint32_t volt_range_3V1_3V2:1;      // 3.1 to 3.2 volts
        uint32_t volt_range_3V2_3V3:1;      // 3.2 to 3.3 volts
        uint32_t volt_range_3V3_3V4:1;      // 3.3 to 3.4 volts
        uint32_t volt_range_3V4_3V5:1;      // 3.4 to 3.5 volts
        uint32_t volt_range_3V5_3V6:1;      // 3.5 to 3.6 volts
        uint32_t s18a:1;                    // Switch to 1.8 v accepted
        uint32_t RESERVED1:5;
        uint32_t ccs:1;                     // Card capacity statys
        uint32_t busy:1;                    // Busy status
    };
    uint32_t raw;
};

static inline union sd_acmd41_rsp sd_get_acmd41_rsp(
                                                SDHC_RR_Type const volatile *RR)
{
    return (union sd_acmd41_rsp){ .raw = RR[0].reg };
}

//
//
//  Status responses
//
//

#define SD_CURRENT_STATE_IDLE   0
#define SD_CURRENT_STATE_READY  1
#define SD_CURRENT_STATE_IDENT  2
#define SD_CURRENT_STATE_STBY   3
#define SD_CURRENT_STATE_TRAN   4
#define SD_CURRENT_STATE_DATA   5
#define SD_CURRENT_STATE_RCV    6
#define SD_CURRENT_STATE_PRG    7
#define SD_CURRENT_STATE_DIS    6

/**
 *  See SD Physical Layer Simplified Specification v3.01 - Table 4-36
 */
union sd_card_status_rsp {
    struct {
        uint32_t RESERVED_MANUFACTURER_TEST:2 __attribute__((aligned(4)));
        uint32_t RESERVED_APP_SPECIFIC:1;
        uint32_t ake_seq_error:1;       // Error in sequence of auth process
        uint32_t RESERVED_SDIO:1;
        uint32_t app_cmd:1;             // Card expects ACMD or command was ACMD
        uint32_t RESERVED:2;
        uint32_t read_for_data:1;       // Buffer empty
        uint32_t current_state:4;
        uint32_t erase_reset:1;
        uint32_t card_ecc_disabled:1;   // Command executed with using ECC
        uint32_t wp_erase_skip:1;
        uint32_t csd_overwrite:1;       // Indicates error in writing CSD
        uint32_t RESERVED1:2;
        uint32_t error:1;
        uint32_t cc_error:1;            // Card controller error
        uint32_t card_ecc_failed:1;
        uint32_t illegal_comand:1;
        uint32_t com_crc_error:1;
        uint32_t lock_unlock_failed:1;
        uint32_t card_is_locked:1;
        uint32_t wp_violation:1;
        uint32_t erase_param:1;         // Invalid selection of erase blocks
        uint32_t erase_seq_error:1;
        uint32_t address_error:1;       // Misaligned address
        uint32_t out_of_range:1;
    };
    uint32_t raw;
};

static inline union sd_card_status_rsp sd_get_card_status_rsp(
                                                    SDHC_RR_Type const volatile *RR)
{
    return (union sd_card_status_rsp){ .raw = RR[0].reg };
}

/**
 *  See SD Physical Layer Simplified Specification v3.01 - Table 4-38
 */
union sd_status_rsp {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
    struct {
        uint8_t RESERVED[49];
        uint8_t uhs_au_size:4;
        uint8_t uhs_speed_grade:4;
        uint8_t erase_offset:2;
        uint8_t erase_timeout:6;
        uint16_t erase_size;
        uint8_t RESERVED1:4;
        uint8_t au_size:4;
        uint8_t performance_move;
        uint8_t speed_class;
        uint32_t size_of_protected_area;
        uint16_t sd_card_type;
        uint16_t RESERVED2:13;
        uint16_t secured_mode:1;
        uint16_t dat_bus_width:2;
    }  __attribute__((packed));
    uint8_t raw[64];
#pragma GCC diagnostic pop
};

/**
 *  Takes an array of 65 bytes and converts it into a SD Status response
 *  structure.
 */
static inline union sd_status_rsp sd_swap_sd_status(uint8_t const *rsp)
{
    union sd_status_rsp status;
    for (int i = 0; i < 64; i++) {
        status.raw[i] = rsp[63 - i];
    }
    return status;
}

/**
 *  See SD Physical Layer Simplified Specification v3.01 - Table 4-11
 */
union sd_switch_function_status_rsp {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
    struct {
        uint8_t RESERVED[34];
        uint16_t function_group_1_busy;
        uint16_t function_group_2_busy;
        uint16_t function_group_3_busy;
        uint16_t function_group_4_busy;
        uint16_t function_group_5_busy;
        uint16_t function_group_6_busy;
        uint8_t version;
        uint8_t function_group_1:4;
        uint8_t function_group_2:4;
        uint8_t function_group_3:4;
        uint8_t function_group_4:4;
        uint8_t function_group_5:4;
        uint8_t function_group_6:4;
        uint16_t function_group_1_info;
        uint16_t function_group_2_info;
        uint16_t function_group_3_info;
        uint16_t function_group_4_info;
        uint16_t function_group_5_info;
        uint16_t function_group_6_info;
        uint16_t max_current;
    }  __attribute__((packed));
    uint8_t raw[64];
#pragma GCC diagnostic pop
};

/**
 *  Takes an array of 64 bytes and converts it into a SD Status response
 *  structure.
 */
static inline union sd_switch_function_status_rsp sd_swap_switch_func_status(
                                                            uint8_t const *rsp)
{
    union sd_switch_function_status_rsp status;
    for (int i = 0; i < 64; i++) {
        status.raw[i] = rsp[63 - i];
    }
    return status;
}

//
//
//  Registers
//
//


/**
 *  See SD Physical Layer Simplified Specification v3.01 - Section 5.1
 */
union sd_ocr_reg {
    struct {
        uint32_t RESEVED:7 __attribute__((aligned(4)));
        uint32_t low_voltage_range:1;       // Set for dual voltage card (after
                                            // CMD8)
        uint32_t RESERVED1:7;
        uint32_t volt_range_2V7_2V8:1;      // 2.7 to 2.8 volts
        uint32_t volt_range_2V8_2V9:1;      // 2.8 to 2.9 volts
        uint32_t volt_range_2V9_3V0:1;      // 2.9 to 3.0 volts
        uint32_t volt_range_3V0_3V1:1;      // 3.0 to 3.1 volts
        uint32_t volt_range_3V1_3V2:1;      // 3.1 to 3.2 volts
        uint32_t volt_range_3V2_3V3:1;      // 3.2 to 3.3 volts
        uint32_t volt_range_3V3_3V4:1;      // 3.3 to 3.4 volts
        uint32_t volt_range_3V4_3V5:1;      // 3.4 to 3.5 volts
        uint32_t volt_range_3V5_3V6:1;      // 3.5 to 3.6 volts
        uint32_t accepts_1V8:1;
        uint32_t RESERVED2:5;
        uint32_t card_capacity_status:1;    // Only valid if power up status set
        uint32_t card_power_up_status:1;    // Set when card is done power up
    } __attribute__((packed));
    uint32_t RR[1];
    uint8_t raw[4];
};

/**
 *  Gets the OCR register value from the SDHC response registers
 */
static inline union sd_ocr_reg sd_get_ocr_rsp(SDHC_RR_Type const volatile *RR)
{
    return (union sd_ocr_reg){ .RR = { RR[0].reg } };
}

/**
 *  See SD Physical Layer Simplified Specification v3.01 - Section 5.2
 */
union sd_cid_reg {
    struct {
        uint8_t end_bit:1 __attribute__((aligned(1)));  // Always 1
        uint8_t crc:7;
        uint16_t manufacture_month:4;
        uint16_t manufacture_year:8;    // Add 2000
        uint16_t RESERVED:4;
        uint32_t serial_num;
        uint8_t product_rev_minor:4 __attribute__((aligned(1))); // Binary coded
                                                                 // decimal: minor version
        uint8_t product_rev_major:4;    // Binary coded decimal: major version
        char product_name[5];
        char application_id[2];
        uint8_t manufacurer_id;
    } __attribute__((packed));
    uint32_t RR[4];
    uint8_t raw[16];
};

/**
 *  Gets the CID register value from the SDHC response registers
 */
static inline union sd_cid_reg sd_get_cid_rsp(SDHC_RR_Type const volatile *RR)
{
    volatile uint8_t const *const RR8[4] = {
        (volatile uint8_t const *)&RR[0], (volatile uint8_t const *)&RR[1],
        (volatile uint8_t const *)&RR[2], (volatile uint8_t const *)&RR[3]
    };

    return (union sd_cid_reg){ .raw = {
        0, RR8[0][0], RR8[0][1], RR8[0][2], RR8[0][3], RR8[1][0], RR8[1][1],
        RR8[1][2], RR8[1][3], RR8[2][0], RR8[2][1], RR8[2][2], RR8[2][3],
        RR8[3][0], RR8[3][1], RR8[3][2]
    } };
}

/**
 *  See SD Physical Layer Simplified Specification v3.01 - Section 5.3.2
 */
union sd_csd_1_reg {
    struct {
        uint8_t end_bit:1 __attribute__((aligned(1)));  // always 1
        uint8_t crc:7;
        uint8_t RESERVED:2 __attribute__((aligned(1))); // always 0
        uint8_t file_format:2;
        uint8_t tmp_write_protect:1;
        uint8_t perm_write_protect:1;
        uint8_t copy:1;
        uint8_t file_format_group:1;

        uint64_t RESERVED1:5;           // always 0
        uint64_t write_bl_partial:1;
        uint64_t write_bl_len:4;
        uint64_t r2w_factor:3;
        uint64_t RESERVED2:2;           // always 0
        uint64_t wp_grp_enable:1;
        uint64_t wp_grp_size:7;
        uint64_t sector_size:7;
        uint64_t erase_bk_enable:1;
        uint64_t c_size_mult:3;
        uint64_t vdd_w_curr_max:3;
        uint64_t vdd_w_curr_min:3;
        uint64_t vdd_r_curr_max:3;
        uint64_t vdd_r_curr_min:3;
        uint64_t c_size:12;
        uint64_t RESERVED3:2;           // always 0
        uint64_t dsr_imp:1;
        uint64_t read_blk_misalign:1;
        uint64_t write_blk_misalign:1;
        uint64_t read_bl_partial:1;     // 1

        uint16_t read_bl_len:4 __attribute__((aligned(2)));
        uint16_t ccc:12;                // 0b01x110110101
        uint8_t tran_speed;             // 0x32 or 0x5a
        uint8_t nsac;                   // Units are 100k clock cycles
        uint8_t taac;                   // data read access time
        uint8_t RESERVED4:6 __attribute__((aligned(1)));    // always 0
        uint8_t csd_structure:2;        // 0
    } __attribute__((packed));
    uint32_t RR[4];
    uint8_t raw[16];
};

#define SD_CSD_1_SIZE(x) (((uint64_t)(x.c_size + 1) * (1 << \
                                (x.c_size_mult + 2))) * (1 << x.read_bl_len))
#define SD_CSD_1_BLOCKS(x) (size_t)(SD_CSD_1_SIZE(x) / 512)

/**
 *  Gets the CSD register v1 value from the SDHC response registers
 */
static inline union sd_csd_1_reg sd_get_csd_1_rsp(
                                                SDHC_RR_Type const volatile *RR)
{
    volatile uint8_t const *const RR8[4] = {
        (volatile uint8_t const *)&RR[0], (volatile uint8_t const *)&RR[1],
        (volatile uint8_t const *)&RR[2], (volatile uint8_t const *)&RR[3]
    };

    return (union sd_csd_1_reg){ .raw = {
        0, RR8[0][0], RR8[0][1], RR8[0][2], RR8[0][3], RR8[1][0], RR8[1][1],
        RR8[1][2], RR8[1][3], RR8[2][0], RR8[2][1], RR8[2][2], RR8[2][3],
        RR8[3][0], RR8[3][1], RR8[3][2]
    } };
}

/**
 *  See SD Physical Layer Simplified Specification v3.01 - Section 5.3.3
 */
union sd_csd_2_reg {
    struct {
        uint8_t end_bit:1 __attribute__((aligned(1)));  // always 1
        uint8_t crc:7;
        uint8_t RESERVED:2 __attribute__((aligned(1))); // always 0
        uint8_t file_format:2;          // 0
        uint8_t tmp_write_protect:1;
        uint8_t perm_write_protect:1;
        uint8_t copy:1;
        uint8_t file_format_group:1;    // 0

        uint64_t RESERVED1:5;           // always 0
        uint64_t write_bl_partial:1;    // 0
        uint64_t write_bl_len:4;        // 9
        uint64_t r2w_factor:3;          // 0b010
        uint64_t RESERVED2:2;           // always 0
        uint64_t wp_grp_enable:1;       // 0
        uint64_t wp_grp_size:7;         // 0
        uint64_t sector_size:7;         // 0x7f
        uint64_t erase_bk_enable:1;     // 1
        uint64_t RESERVED3:1;           // always 0
        uint64_t c_size:22;
        uint64_t RESERVED4:6;           // always 0
        uint64_t dsr_imp:1;
        uint64_t read_blk_misalign:1;   // 0
        uint64_t write_blk_misalign:1;  // 0
        uint64_t read_bl_partial:1;     // 0

        uint16_t read_bl_len:4 __attribute__((aligned(2))); // 9
        uint16_t ccc:12;                // 0b01x110110101
        uint8_t tran_speed;             // 0x32, 0x5a, 0x0b or 0x3b
        uint8_t nsac;                   // data read access time (clk) = 0x00
        uint8_t taac;                   // data read access time =0x0e
        uint8_t RESERVED5:6 __attribute__((aligned(1)));    // always 0
        uint8_t csd_structure:2;        // 1
    } __attribute__((packed));
    uint32_t RR[4];
    uint8_t raw[16];
};

#define SD_CSD_2_SIZE(x) (((uint64_t)(x.c_size + 1)) << 19)
#define SD_CSD_2_BLOCKS(x) ((size_t)((x.c_size + 1) << 10))

/**
 *  Gets the CSD register v2 value from the SDHC response registers
 */
static inline union sd_csd_2_reg sd_get_csd_2_rsp(
                                                volatile SDHC_RR_Type const *RR)
{
    volatile uint8_t const *const RR8[4] = {
        (volatile uint8_t const *)&RR[0], (volatile uint8_t const *)&RR[1],
        (volatile uint8_t const *)&RR[2], (volatile uint8_t const *)&RR[3]
    };

    return (union sd_csd_2_reg){ .raw = {
        0, RR8[0][0], RR8[0][1], RR8[0][2], RR8[0][3], RR8[1][0], RR8[1][1],
        RR8[1][2], RR8[1][3], RR8[2][0], RR8[2][1], RR8[2][2], RR8[2][3],
        RR8[3][0], RR8[3][1], RR8[3][2]
    } };
}

/**
 *  See SD Physical Layer Simplified Specification v3.01 - Section 5.6
 */
union sd_scr_reg {
    struct {
        uint32_t manufacturer_data __attribute__((aligned(4)));
        uint32_t command_support:2 __attribute__((aligned(4)));
        uint32_t RESERVED:9;
        uint32_t ex_security:4;
        uint32_t sd_spec3:1;
        uint32_t sd_bus_widths:4;
        uint32_t sd_security:3;
        uint32_t data_stat_after_erase:1;
        uint32_t sd_spec:4;
        uint32_t scr_structure:4;
    } __attribute__((packed));
    uint8_t raw[8];
};

/**
 *  Takes an array of 8 bytes and converts it into a SCR register structure.
 */
static inline union sd_scr_reg sd_swap_scr(uint8_t const *rsp)
{
    return (union sd_scr_reg){ .raw = { rsp[7], rsp[6], rsp[5], rsp[4],
                                        rsp[3], rsp[2], rsp[1], rsp[0] }
    };
}

#endif /* sd_commands_h */
