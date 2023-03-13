/**
 * @file sdspi-commands.h
 * @desc SD Card commands from SD Card Physical Layer Simplified Specification
 *       v3.01
 * @author Samuel Dewan
 * @date 2021-05-28
 * Last Author:
 * Last Edited On:
 */

#ifndef sdspi_commands_h
#define sdspi_commands_h

// We always want to use a 512 byte block size
#define SDSPI_BLOCK_SIZE 512


union sdspi_command {
    struct {
        uint8_t command_index:6 __attribute__((aligned(1)));
        uint8_t transmition_bit:1;  // Always 1
        uint8_t start_bit:1;        // Always 0
        uint32_t argument;
        uint8_t end_bit:1 __attribute__((aligned(1)));  // Always 1
        uint8_t crc:7;
    } __attribute__((packed));
    uint8_t raw[6];
};


enum sdspi_command_index {
    /** GO_IDLE_STATE - init card in spi mode if CS low */
    SDSPI_CMD0  = 0,
    /** Initialize card fallback code */
    SDSPI_CMD1  = 1,
    SDSPI_CMD6  = 6,
    /** SEND_IF_COND - verify SD Memory Card interface operating condition.*/
    SDSPI_CMD8  = 8,
    /** SEND_CSD - read the Card Specific Data (CSD register) */
    SDSPI_CMD9  = 9,
    /** SEND_CID - read the card identification information (CID register) */
    SDSPI_CMD10  = 10,
    /** STOP_TRANSMISSION - stop transmission in multi-block read mode */
    SDSPI_CMD12  = 12,
    /** SEND_STATUS - read the card status register */
    SDSPI_CMD13  = 13,
    /** Set block size*/
    SDSPI_CMD16  = 16,
    /** READ_BLOCK - read a single data block from the card */
    SDSPI_CMD17  = 17,
    SDSPI_CMD18  = 18,
    /** WRITE_BLOCK - write a single data block to the card */
    SDSPI_CMD24  = 24,
    /** WRITE_MULTIPLE_BLOCK - write blocks of data until a STOP_TRANSMISSION */
    SDSPI_CMD25  = 25,
    SDSPI_CMD27  = 27,
    SDSPI_CMD28  = 28,
    SDSPI_CMD29  = 29,
    SDSPI_CMD30  = 30,
    /** ERASE_WR_BLK_START - sets the address of the first block to be erased */
    SDSPI_CMD32  = 32,
    /** ERASE_WR_BLK_END - sets the address of the last block of the continuous
        range to be erased*/
    SDSPI_CMD33  = 33,
    /** ERASE - erase all previously selected blocks */
    SDSPI_CMD38  = 38,
    SDSPI_CMD42  = 42,
    /** APP_CMD - escape for application specific command */
    SDSPI_CMD55  = 55,
    SDSPI_CMD56  = 56,
    /** READ_OCR - read the OCR register of a card */
    SDSPI_CMD58  = 58,
    /** CRC_ON_OFF - turn the requirement to send the CRC with a command
        on/off */
    SDSPI_CMD59  = 59
};

enum sdspi_application_command_index {
    SDSPI_ACMD13 = 13,
    SDSPI_ACMD18 = 18,
    SDSPI_ACMD22 = 22,
    /** SET_WR_BLK_ERASE_COUNT - Set the number of write blocks to be
        pre-erased before writing */
    SDSPI_ACMD23 = 23,
    SDSPI_ACMD25 = 25,
    SDSPI_ACMD26 = 26,
    SDSPI_ACMD38 = 38,
    /** SD_SEND_OP_COMD - Sends host capacity support information and
        activates the card's initialization process */
    SDSPI_ACMD41 = 41,
    SDSPI_ACMD42 = 42,
    SDSPI_ACMD51 = 51
};

#define SDSPI_ACMD41_HCS    (1 << 30)
#define SDSPI_CMD1_HCS    ( 1 << 30)





#define SDSPI_DRT_VALID(x)  (((x & (1 << 0)) != 0) && ((x & (1 << 4)) == 0))
#define SDSPI_DRT_STATUS(x) ((x & 0xe) >> 1)

enum sdspi_drt_status {
    SDSPI_DRT_STATUS_ACCEPTED = 0b010,
    SDSPI_DRT_STATUS_CRC_ERROR = 0b101,
    SDSPI_DRT_STATUS_WRITE_ERROR = 0b110
};


#define SDSPI_SINGLE_BLOCK_START_TOKEN  0b11111110
#define SDSPI_MULTI_BLOCK_START_TOKEN   0b11111100
#define SDSPI_MULTI_BLOCK_STOP_TOKEN    0b11111101

#define SDSPI_IS_DATA_ERROR(x)  (~x & 0xf0)

#define SDSPI_DATA_ERROR_ERROR              (1 << 0)
#define SDSPI_DATA_ERROR_CC_ERROR           (1 << 1)
#define SDSPI_DATA_ERROR_CARD_ECC_FAILED    (1 << 2)
#define SDSPI_DATA_ERROR_OUT_OF_RANGE       (1 << 3)

union sdspi_data_error_token {
    struct {
        uint8_t error:1 __attribute__((aligned(1)));
        uint8_t cc_error:1;
        uint8_t card_ecc_failed:1;
        uint8_t out_of_range:1;
        uint8_t RESERVED:4;         // Will be all 0
    } __attribute__((packed));
    uint8_t raw;
};



/**
 *  See SD Physical Layer Simplified Specification v3.01 - Section 5.1
 */
union sdspi_ocr_reg {
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
    uint8_t raw[4];
};

/**
 *  Takes an array of 4 bytes and converts it into an OCR register structure.
 */
static inline union sdspi_ocr_reg sdspi_swap_ocr(uint8_t const *rsp)
{
    return (union sdspi_ocr_reg){ .raw = { rsp[3], rsp[2], rsp[1], rsp[0] } };
}

/**
 *  See SD Physical Layer Simplified Specification v3.01 - Section 5.2
 */
union sdspi_cid_reg {
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
    uint8_t raw[16];
};

/**
 *  Takes an array of 16 bytes and converts it into a CID register structure.
 */
static inline union sdspi_cid_reg sdspi_swap_cid(uint8_t const *rsp)
{
    return (union sdspi_cid_reg){ .raw = { rsp[15], rsp[14], rsp[13], rsp[12],
                                           rsp[11], rsp[10], rsp[9], rsp[8],
                                           rsp[7], rsp[6], rsp[5], rsp[4],
                                           rsp[3], rsp[2], rsp[1], rsp[0] }
                                };
}

/**
 *  See SD Physical Layer Simplified Specification v3.01 - Section 5.3.2
 */
union sdspi_csd_1_reg {
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
    uint8_t raw[16];
};

#define SDSPI_CSD_1_SIZE(x) (((uint64_t)(x.c_size + 1) * (1 << \
                            (x.c_size_mult + 2))) * (1 << x.read_bl_len))
#define SDSPI_CSD_1_BLOCKS(x) (size_t)(SDSPI_CSD_1_SIZE(x) / SDSPI_BLOCK_SIZE)

/**
 *  Takes an array of 16 bytes and converts it into a CSD v1.0 register
 *  structure.
 */
static inline union sdspi_csd_1_reg sdspi_swap_csd_1(uint8_t const *rsp)
{
    return (union sdspi_csd_1_reg){ .raw = { rsp[15], rsp[14],
                                             rsp[13], rsp[12],
                                             rsp[11], rsp[10],
                                             rsp[9], rsp[8],
                                             rsp[7], rsp[6],
                                             rsp[5], rsp[4],
                                             rsp[3], rsp[2],
                                             rsp[1], rsp[0] }
                                   };
}

/**
 *  See SD Physical Layer Simplified Specification v3.01 - Section 5.3.3
 */
union sdspi_csd_2_reg {
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
    uint8_t raw[16];
};

#define SDSPI_CSD_2_SIZE(x) (((uint64_t)(x.c_size + 1)) << 19)
#define SDSPI_CSD_2_BLOCKS(x) ((size_t)((x.c_size + 1) << 10))

/**
 *  Takes an array of 16 bytes and converts it into a CSD v2.0 register
 *  structure.
 */
static inline union sdspi_csd_2_reg sdspi_swap_csd_2(uint8_t const *rsp)
{
    return (union sdspi_csd_2_reg){ .raw = { rsp[15], rsp[14],
                                             rsp[13], rsp[12],
                                             rsp[11], rsp[10],
                                             rsp[9], rsp[8],
                                             rsp[7], rsp[6],
                                             rsp[5], rsp[4],
                                             rsp[3], rsp[2],
                                             rsp[1], rsp[0] }
                                  };
}



/**
 *  See SD Physical Layer Simplified Specification v3.01 - Section 5.6
 */
union sdspi_scr_reg {
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

#define SDSPI_CSD_2_SIZE(x) (((uint64_t)(x.c_size + 1)) << 19)
#define SDSPI_CSD_2_BLOCKS(x) ((size_t)((x.c_size + 1) << 10))

/**
 *  Takes an array of 8 bytes and converts it into a SCR register structure.
 */
static inline union sdspi_scr_reg sdspi_swap_scr(uint8_t const *rsp)
{
    return (union sdspi_scr_reg){ .raw = { rsp[7], rsp[6], rsp[5], rsp[4],
                                           rsp[3], rsp[2], rsp[1], rsp[0] }
                                };
}


#define SDSPI_R1_IN_IDLE_STATE          (1 << 0)
#define SDSPI_R1_ERASE_RESET            (1 << 1)
#define SDSPI_R1_ILLEGAL_COMMAND        (1 << 2)
#define SDSPI_R1_COM_CRC_ERROR          (1 << 3)
#define SDSPI_R1_ERASE_SEQUENCE_ERROR   (1 << 4)
#define SDSPI_R1_ADDRESS_ERROR          (1 << 5)
#define SDSPI_R1_PARAMETER_ERROR        (1 << 6)

#define SDSPI_R2_CARD_LOCKED            (1 << 0)
#define SDSPI_R2_WP_ERASE_SKIP          (1 << 1)
#define SDSPI_R2_LOCK_CMD_FAILED        (1 << 1)
#define SDSPI_R2_ERROR                  (1 << 2)
#define SDSPI_R2_CC_ERROR               (1 << 3)
#define SDSPI_R2_CARD_ECC_FAILED        (1 << 4)
#define SDSPI_R2_WP_VIOLATION           (1 << 5)
#define SDSPI_R2_ERASE_PROGRAM          (1 << 6)
#define SDSPI_R2_OUT_OF_RANGE           (1 << 7)


/**
 *  See SD Physical Layer Simplified Specification v3.01 - Section 7.3.2.1
 */
union sdspi_response_r1 {
    struct {
        uint8_t in_idle_state:1 __attribute__((aligned(1)));
        uint8_t erase_reset:1;
        uint8_t illegal_command:1;
        uint8_t com_crc_error:1;
        uint8_t erase_sequence_error:1;
        uint8_t address_error:1;
        uint8_t parameter_error:1;
        uint8_t RESERVED:1;             // Always 0
    } __attribute__ ((packed));
    uint8_t raw;
};

/**
 *  See SD Physical Layer Simplified Specification v3.01 - Section 7.3.2.3
 */
union sdspi_response_r2 {
    struct {
        uint8_t card_is_locked:1 __attribute__((aligned(1)));
        uint8_t wp_erase_skip:1;
        uint8_t error:1;
        uint8_t cc_error:1;
        uint8_t card_ecc_failed:1;
        uint8_t wp_violation:1;
        uint8_t erase_program:1;
        uint8_t out_of_range:1;

        union sdspi_response_r1 r1;
    } __attribute__((packed));
    uint8_t raw[2];
};

/**
 *  Takes an array of 2 bytes and converts it into an R2 response structure.
 */
static inline union sdspi_response_r2 sdspi_swap_r2(uint8_t const *rsp)
{
    return (union sdspi_response_r2){ .raw = { rsp[1], rsp[0] } };
}


/**
 *  See SD Physical Layer Simplified Specification v3.01 - Section 7.3.2.4
 */
union sdspi_response_r3 {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpacked"
#pragma GCC diagnostic ignored "-Wattributes"
    struct {
        union sdspi_ocr_reg ocr;
        union sdspi_response_r1 r1;
    } __attribute__((packed));
#pragma GCC diagnostic pop
    uint8_t raw[5];
};

/**
 *  Takes an array of 5 bytes and converts it into an R3 response structure.
 */
static inline union sdspi_response_r3 sdspi_swap_r3(uint8_t const *rsp)
{
    return (union sdspi_response_r3){ .raw = { rsp[4], rsp[3], rsp[2],
                                               rsp[1], rsp[0] } };
}


/**
 *  See SD Physical Layer Simplified Specification v3.01 - Section 7.3.2.6
 */
union sdspi_response_r7 {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
    struct {
        uint32_t check_pattern:8;
        uint32_t voltage_accepted:4;
        uint32_t RESERVED:16;
        uint32_t cmd_version:4;

        union sdspi_response_r1 r1;
    } __attribute__((packed));
#pragma GCC diagnostic pop
    uint8_t raw[5];
};

/**
 *  Takes an array of 5 bytes and converts it into an R7 response structure.
 */
static inline union sdspi_response_r7 sdspi_swap_r7(uint8_t const *rsp)
{
    return (union sdspi_response_r7){ .raw = { rsp[4], rsp[3], rsp[2],
                                               rsp[1], rsp[0] } };
}




union sdspi_cmd8_arg {
    struct {
        uint32_t check_pattern:8;
        uint32_t supply_voltage:4;
        uint32_t RESERVED:20;
    };
    uint32_t raw;
};


#endif /* sdspi_commands_h */
