/**
 * @file debug-commands-sd.c
 * @desc Commands for debugging SD card
 * @author Samuel Dewan
 * @date 2021-06-01
 * Last Author:
 * Last Edited On:
 */

#include "debug-commands-sd.h"

#include "debug-commands.h"

#include "variant.h"
#include "board.h"
#include "sd.h"
#include "sdspi.h"
#include "wdt.h"
#include "mbr.h"

#include "logging.h"
#include "logging-format.h"


#if defined(ENABLE_SDSPI) || defined(ENABLE_SDHC0)

struct debug_sd_cb_context {
    uint32_t num_blocks;
    enum sd_op_result result;
    uint8_t debug_sdspi_cb_called;
};

static void debug_sd_cb(void *context, enum sd_op_result result,
                           uint32_t num_blocks)
{
    struct debug_sd_cb_context *c = (struct debug_sd_cb_context*)context;
    c->num_blocks = num_blocks;
    c->result = result;
    c->debug_sdspi_cb_called = 1;
}


static void debug_sd_read(uint8_t argc, char **argv,
                          struct console_desc_t *console,
                          struct sd_funcs sd_funcs, sd_desc_ptr_t inst,
                          void (*run_service_func)(void))
{
    uint32_t addr = 0;
    uint32_t num_blocks = 0;

    if (argc < 3) {
        // No additional arguments afer action
        console_send_str(console, "Must specify address and (optionally) "
                         "length\n");
        return;
    } else if (argc > 4) {
        console_send_str(console, "Too many arguments\n");
        return;
    }

    // Parse address
    char *end;
    addr = (uint32_t)strtoul(argv[2], &end, 0);
    if (*end != '\0') {
        console_send_str(console, "Invalid address\n");
        return;
    }

    if (argc < 4) {
        // No number of blocks
        num_blocks = 1;
    } else {
        num_blocks = (uint32_t)strtoul(argv[3], &end, 0);
        if (*end != '\0') {
            console_send_str(console, "Invalid number of blocks\n");
            return;
        }
    }

    struct debug_sd_cb_context context = { 0 };
    uint8_t *buffer = alloca(num_blocks * 512);

    int const ret = sd_funcs.read(inst, addr, num_blocks, buffer, debug_sd_cb,
                                  &context);

    if (ret != 0) {
        console_send_str(console, "Failed to start read operation.\n");
        return;
    }

    while (!context.debug_sdspi_cb_called) {
        run_service_func();
        wdt_pat();
    }

    char buf[16];

    // Print received blocks
    for (uint32_t i = 0; i < context.num_blocks; i++) {
        for (int j = 0; j < 512; j += 32) {
            for (int k = 0; k < 32; k++) {
                uint32_t const index = (i * 512) + j + k;
                utoa(buffer[index], buf, 16);
                if (buffer[index] < 0x10) {
                    console_send_str(console, "0");
                }
                console_send_str(console, buf);
            }
            wdt_pat();
            console_send_str(console, "\n");
        }
        console_send_str(console, "\n");
    }

    // Print summary
    switch (context.result) {
        case SD_OP_SUCCESS:
            console_send_str(console, "\nSuccess - ");
            break;
        case SD_OP_FAILED:
            console_send_str(console, "\nFailure - ");
            break;
        default:
            console_send_str(console, "\nUnkown result - ");
            break;
    }

    utoa(context.num_blocks, buf, 10);
    console_send_str(console, buf);
    console_send_str(console, " blocks read.\n");
}

static void debug_sd_write(uint8_t argc, char **argv,
                           struct console_desc_t *console,
                           struct sd_funcs sd_funcs, sd_desc_ptr_t inst,
                           void (*run_service_func)(void))
{
    uint32_t addr = 0;
    uint32_t num_blocks = 0;
    uint8_t fancy_pattern = 0;
    uint8_t pattern = 0;

    if (argc < 4) {
        // No additional arguments afer action
        console_send_str(console, "Must specify pattern, address and "
                         "(optionally) length\n");
        return;
    } else if (argc > 5) {
        console_send_str(console, "Too many arguments\n");
        return;
    }

    // Parse pattern
    char *end;
    if (!strcmp(argv[2], "p")) {
        // Use a fancy pattern
        fancy_pattern = 1;
    } else {
        pattern = (uint8_t)strtoul(argv[2], &end, 0);
        if (*end != '\0') {
            console_send_str(console, "Invalid pattern\n");
            return;
        }
    }

    // Parse address
    addr = (uint32_t)strtoul(argv[3], &end, 0);
    if (*end != '\0') {
        console_send_str(console, "Invalid address\n");
        return;
    }

    if (argc < 5) {
        // No number of blocks
        num_blocks = 1;
    } else {
        num_blocks = (uint32_t)strtoul(argv[4], &end, 0);
        if (*end != '\0') {
            console_send_str(console, "Invalid number of blocks\n");
            return;
        }
    }

    struct debug_sd_cb_context context = { 0 };
    uint8_t *buffer = alloca(num_blocks * 512);
    if (!fancy_pattern) {
        memset(buffer, pattern, num_blocks * 512);
    } else {
        pattern = 0;
        for (int i = 0; i < 512; i++) {
            buffer[i] = pattern++;
        }
    }

    int const ret = sd_funcs.write(inst, addr, num_blocks, buffer, debug_sd_cb,
                                   &context);

    if (ret != 0) {
        console_send_str(console, "Failed to start write operation.\n");
        return;
    }

    while (!context.debug_sdspi_cb_called) {
        run_service_func();
        wdt_pat();
    }

    // Print summary
    switch (context.result) {
        case SD_OP_SUCCESS:
            console_send_str(console, "\nSuccess - ");
            break;
        case SD_OP_FAILED:
            console_send_str(console, "\nFailure - ");
            break;
        default:
            console_send_str(console, "\nUnkown result - ");
            break;
    }

    char buf[8];
    utoa(context.num_blocks, buf, 10);
    console_send_str(console, buf);
    console_send_str(console, " blocks written.\n");
}


// MARK: SDSPI

#endif // ENABLE_SDSPI || ENABLE_SDHC0

#ifdef ENABLE_SDSPI
static void sdspi_run_service(void)
{
    sdspi_service(&sdspi_g);
}
#endif

void debug_sdspi (uint8_t argc, char **argv, struct console_desc_t *console)
{
#ifdef ENABLE_SDSPI
    char str[16];

    if (argc == 1) {
        // Print status info
        console_send_str(console, "Status: ");
        enum sdspi_status const status = sdspi_get_status(&sdspi_g);

        switch (status) {
            case SDSPI_STATUS_NO_CARD:
                console_send_str(console, "no card\n");
                break;
            case SDSPI_STATUS_UNUSABLE_CARD:
                console_send_str(console, "unusable card\n");
                break;
            case SDSPI_STATUS_TOO_MANY_INIT_RETRIES:
                console_send_str(console, "too many init retries\n");
                break;
            case SDSPI_STATUS_TOO_MANY_TIMEOUTS:
                console_send_str(console, "too many timeouts\n");
                break;
            case SDSPI_STATUS_FAILED:
                console_send_str(console, "failed\n");
                break;
            case SDSPI_STATUS_INITIALIZING:
                console_send_str(console, "initalizing\n");
                break;
            case SDSPI_STATUS_READY:
                console_send_str(console, "ready\n");
                break;
            default:
                console_send_str(console, "unknown\n");
                break;
        }

        if (status != SDSPI_STATUS_READY) {
            return;
        }

        console_send_str(console, "Capacity: ");
        utoa(sdspi_g.card_capacity, str, 10);
        console_send_str(console, str);
        console_send_str(console, " blocks\n");

        console_send_str(console, "V1 card: ");
        console_send_str(console, sdspi_g.v1_card ? "yes\n" : "no\n");

        console_send_str(console, "High capacity: ");
        console_send_str(console, sdspi_g.block_addressed ? "yes\n" : "no\n");

        return;
    }

    // We have at least two args. The second should be an action.

    if (!strcmp(argv[1], "read")) {
        // Read from card
        debug_sd_read(argc, argv, console, sdspi_sd_funcs, &sdspi_g,
                      sdspi_run_service);
    } else if (!strcmp(argv[1], "write")) {
        debug_sd_write(argc, argv, console, sdspi_sd_funcs, &sdspi_g,
                       sdspi_run_service);
    } else {
        console_send_str(console, "Unkown action.\n");
    }

#else
    console_send_str(console, "SDSPI is not enabled.\n");
#endif
}



// MARK: SDHC

#ifdef ENABLE_SDHC0
static void sdhc_run_service(void)
{
    sdhc_service(&sdhc0_g);
}
#endif

void debug_sdhc (uint8_t argc, char **argv, struct console_desc_t *console)
{
#ifdef ENABLE_SDHC0
    char str[16];

    if (argc == 1) {
        // Print status info
        console_send_str(console, "Status: ");
        enum sdhc_status const status = sdhc_get_status(&sdhc0_g);

        switch (status) {
            case SDHC_STATUS_NO_CARD:
                console_send_str(console, "no card\n");
                break;
            case SDHC_STATUS_UNUSABLE_CARD:
                console_send_str(console, "unusable card\n");
                break;
            case SDHC_STATUS_TOO_MANY_INIT_RETRIES:
                console_send_str(console, "too many init retries\n");
                break;
            case SDHC_STATUS_INIT_TIMEOUT:
                console_send_str(console, "init timeout\n");
                break;
            case SDHC_STATUS_FAILED:
                console_send_str(console, "failed\n");
                break;
            case SDHC_STATUS_INITIALIZING:
                console_send_str(console, "initalizing\n");
                break;
            case SDHC_STATUS_READY:
                console_send_str(console, "ready\n");
                break;
            default:
                console_send_str(console, "unknown\n");
                break;
        }

        if (status != SDHC_STATUS_READY) {
            return;
        }

        console_send_str(console, "Capacity: ");
        utoa(sdhc0_g.card_capacity, str, 10);
        console_send_str(console, str);
        console_send_str(console, " blocks\n");

        console_send_str(console, "V1 card: ");
        console_send_str(console, sdhc0_g.v1_card ? "yes\n" : "no\n");

        console_send_str(console, "High capacity: ");
        console_send_str(console, sdhc0_g.block_addressed ? "yes\n" : "no\n");

        return;
    }

    // We have at least two args. The second should be an action.

    if (!strcmp(argv[1], "read")) {
        // Read from card
        debug_sd_read(argc, argv, console, sdhc_sd_funcs, &sdhc0_g,
                      sdhc_run_service);
    } else if (!strcmp(argv[1], "write")) {
        // Write to card
        debug_sd_write(argc, argv, console, sdhc_sd_funcs, &sdhc0_g,
                       sdhc_run_service);
    } else {
        console_send_str(console, "Unkown action.\n");
    }
#else
    console_send_str(console, "SDHC is not enabled.\n");
#endif
}

// MARK: MBR

#if defined(ENABLE_SDHC0) || defined(ENABLE_SDSPI)

static void debug_print_chs_addr(struct console_desc_t *console,
                                 struct mbr_chs_address addr)
{
    char buf[16];

    console_send_str(console, "(");
    utoa(addr.cylinder, buf, 10);
    console_send_str(console, buf);
    console_send_str(console, ", ");
    utoa(addr.head, buf, 10);
    console_send_str(console, buf);
    console_send_str(console, ", ");
    utoa(addr.sector, buf, 10);
    console_send_str(console, buf);
    console_send_str(console, ")");
}

static void debug_read_mbr(struct console_desc_t *console,
                           struct sd_funcs sd_funcs, sd_desc_ptr_t inst,
                           void (*run_service_func)(void))
{
    char buf[16];

    // Read MBR
    uint8_t mbr[512];

    struct debug_sd_cb_context context = { 0 };
    int const ret = sd_funcs.read(inst, 0, 1, mbr, debug_sd_cb, &context);

    if (ret != 0) {
        console_send_str(console, "Failed to start read operation.\n");
        return;
    }

    while (!context.debug_sdspi_cb_called) {
        run_service_func();
        wdt_pat();
    }

    if (context.result != SD_OP_SUCCESS) {
        console_send_str(console, "Failed to read MBR.\n");
        return;
    }

    // Parse MBR and print partition information

    if (!mbr_is_valid(mbr)) {
        console_send_str(console, "MBR is not valid.\n");
    }

    for (int i = 0; i < MBR_MAX_NUM_PARTITIONS; i++) {
        console_send_str(console, "Partition ");
        itoa(i, buf, 10);
        console_send_str(console, buf);
        console_send_str(console, ":\n");

        uint8_t const *const entry = mbr_get_partition_entry(mbr, i);

        if (mbr_part_is_valid(entry)) {
            enum mbr_partition_type const type = mbr_part_type(entry);
            console_send_str(console, "\ttype: 0x");
            if (type < 0x10) {
                console_send_str(console, "0");
            }
            utoa(type, buf, 16);
            console_send_str(console, buf);
            if (type == MBR_PART_TYPE_CUINSPACE) {
                console_send_str(console, " (CU InSpace)");
            }
            console_send_str(console, "\n\tbootable: ");
            utoa(!!mbr_part_is_bootable(entry), buf, 10);
            console_send_str(console, buf);
            console_send_str(console, "\n\tfirst sector chs: ");
            debug_print_chs_addr(console,
                                 mbr_part_first_sector_chs_addr(entry));
            console_send_str(console, "\n\tlast sector chs: ");
            debug_print_chs_addr(console, mbr_part_last_sector_chs_addr(entry));
            console_send_str(console, "\n\tfirst sector lba: ");
            utoa(mbr_part_first_sector_lba(entry), buf, 10);
            console_send_str(console, buf);
            console_send_str(console, "\n\tnum sectors: ");
            utoa(mbr_part_num_sectors(entry), buf, 10);
            console_send_str(console, buf);
            console_send_str(console, "\n");
        } else {
            console_send_str(console, "\tempty\n");
        }
    }
}

static void debug_create_partition(uint8_t argc, char **argv,
                                   struct console_desc_t *console,
                                   struct sd_funcs sd_funcs, sd_desc_ptr_t inst,
                                   void (*run_service_func)(void))
{
    uint32_t first_block = 2048;
    uint32_t length;

    if (argc > 4) {
        console_send_str(console, "Too many arguments.\n");
        return;
    }

    if (argc > 2) {
        // First block is specified
        char *end;
        first_block = (uint32_t)strtoul(argv[2], &end, 0);
        if (*end != '\0') {
            console_send_str(console, "Invalid first block.\n");
            return;
        }
    }

    if (argc == 4) {
        // Length is specified
        char *end;
        length = (uint32_t)strtoul(argv[3], &end, 0);
        if (*end != '\0') {
            console_send_str(console, "Invalid length.\n");
            return;
        }
    } else {
        length = sd_funcs.get_num_blocks(inst) - first_block;
    }

    uint8_t mbr[512];

    mbr_init(mbr);
    mbr_init_partition(mbr, 0, MBR_PART_TYPE_CUINSPACE, first_block, length);

    struct debug_sd_cb_context context = { 0 };

    int const ret = sd_funcs.write(inst, 0, 1, mbr, debug_sd_cb, &context);

    if (ret != 0) {
        console_send_str(console, "Failed to start write operation.\n");
        return;
    }

    while (!context.debug_sdspi_cb_called) {
        run_service_func();
        wdt_pat();
    }

    if (context.result != SD_OP_SUCCESS) {
        console_send_str(console, "Failed to write MBR.\n");
        return;
    }
}

#endif

void debug_mbr(uint8_t argc, char **argv, struct console_desc_t *console)
{
#if defined(ENABLE_SDHC0)
    struct sd_funcs sd_funcs = sdhc_sd_funcs;
    struct sdhc_desc_t *sd_inst = &sdhc0_g;
    void (*run_service_func)(void) = sdhc_run_service;
#elif defined(ENABLE_SDSPI)
    struct sd_funcs sd_funcs = sdspi_sd_funcs;
    struct sdspi_desc_t *sd_inst = &sdspi_g;
    void (*run_service_func)(void) = sdspi_run_service;
#else
    console_send_str(console, "No SD card interface enabled.\n");
    return;
#endif

#if defined(ENABLE_SDHC0) || defined(ENABLE_SDSPI)
    if (argc == 1) {
        // Read MBR
        debug_read_mbr(console, sd_funcs, sd_inst, run_service_func);
        return;
    }

    if (!strcmp(argv[1], "create")) {
        debug_create_partition(argc, argv, console, sd_funcs, sd_inst,
                               run_service_func);
        return;
    } else {
        console_send_str(console, "Unkown option.\n");
    }
#endif
}

// MARK: Format
void debug_format(uint8_t argc, char **argv, struct console_desc_t *console)
{
#if defined(ENABLE_SDHC0)
    struct sd_funcs sd_funcs = sdhc_sd_funcs;
    struct sdhc_desc_t *sd_inst = &sdhc0_g;
    void (*run_service_func)(void) = sdhc_run_service;
#elif defined(ENABLE_SDSPI)
    struct sd_funcs sd_funcs = sdspi_sd_funcs;
    struct sdspi_desc_t *sd_inst = &sdspi_g;
    void (*run_service_func)(void) = sdspi_run_service;
#else
    console_send_str(console, "No SD card interface enabled.\n");
    return;
#endif

#if defined(ENABLE_SDHC0) || defined(ENABLE_SDSPI)
    if (argc != 2) {
        console_send_str(console, DEBUG_FORMAT_HELP);
        console_send_str(console, "\n");
        return;
    }

    char *end;
    unsigned long part_num = strtoul(argv[1], &end, 0);
    if ((*end != '\0') || (part_num >= MBR_MAX_NUM_PARTITIONS)) {
        console_send_str(console, "Invalid partition number.\n");
        return;
    }

    // Get the MBR
    union logging_superblock sb;

    struct debug_sd_cb_context context = { 0 };
    int ret = sd_funcs.read(sd_inst, 0, 1, sb.raw, debug_sd_cb, &context);

    if (ret != 0) {
        console_send_str(console, "Failed to start read operation.\n");
        return;
    }

    while (!context.debug_sdspi_cb_called) {
        run_service_func();
        wdt_pat();
    }

    if (context.result != SD_OP_SUCCESS) {
        console_send_str(console, "Failed to read MBR.\n");
        return;
    }

    // Parse MBR and print partition information

    if (!mbr_is_valid(sb.raw)) {
        console_send_str(console, "MBR is not valid.\n");
        return;
    }

    const uint8_t *const part = mbr_get_partition_entry(sb.raw, part_num);
    if (mbr_part_type(part) != MBR_PART_TYPE_CUINSPACE) {
        console_send_str(console, "Partition type not correct (use mbr create "
                         "first).\n");
        return;
    }

    uint32_t const part_start = mbr_part_first_sector_lba(part);
    uint32_t const part_len = mbr_part_num_sectors(part);

    // Create superblock
    memset(sb.raw, 0, sizeof(sb));
    memcpy(sb.magic, LOGGING_SB_MAGIC, 8);
    memcpy(sb.magic2, LOGGING_SB_MAGIC, 8);

    sb.version = LOGGING_FORMAT_VERSION;
    sb.continued = 0;
    sb.partition_length = part_len;


    // If logging is enabled, pause it
#ifdef ENABLE_LOGGING
    logging_pause(&logging_g);
#endif


    // Write superblock to SD card
    context.debug_sdspi_cb_called = 0;

    ret = sd_funcs.write(sd_inst, part_start, 1, sb.raw, debug_sd_cb, &context);

    if (ret != 0) {
        console_send_str(console, "Failed to start write operation.\n");
        return;
    }

    while (!context.debug_sdspi_cb_called) {
        run_service_func();
        wdt_pat();
    }

    if (context.result != SD_OP_SUCCESS) {
        console_send_str(console, "Failed to write superblock.\n");
        return;
    }
#endif
}


// MARK: Logging

void debug_logging(uint8_t argc, char **argv, struct console_desc_t *console)
{
#ifdef ENABLE_LOGGING
    char str[16];

    if ((argc == 1) || (!strcmp(argv[1], "info"))) {
        // Print information about logging service

        // State
        console_send_str(console, "Logging service state: ");
        switch (logging_g.state) {
            case LOGGING_GET_MBR:
            case LOGGING_MBR_WAIT:
            case LOGGING_MBR_PARSE:
                console_send_str(console, "getting MBR\n");
                return;
            case LOGGING_GET_SUPERBLOCK:
            case LOGGING_SUPERBLOCK_WAIT:
            case LOGGING_SUPERBLOCK_PARSE:
                console_send_str(console, "getting superblock\n");
                return;
            case LOGGING_ACTIVE:
                console_send_str(console, "active\n");
                break;
            case LOGGING_PAUSED:
                console_send_str(console, "paused\n");
                break;
            case LOGGING_TOO_MANY_SD_RETRIES:
                console_send_str(console, "failed - too many SD retires\n");
                return;
            case LOGGING_NO_VALID_MBR:
                console_send_str(console, "failed - no valid MBR\n");
                return;
            case LOGGING_NO_VALID_PARTITION:
                console_send_str(console, "failed - too many SD retires\n");
                return;
            case LOGGING_OUT_OF_SPACE:
                console_send_str(console, "failed - out of space\n");
                break;
            case LOGGING_FAILED:
                console_send_str(console, "failed\n");
                break;
        }

        // Times
        console_send_str(console, "\nLast data write: ");
        utoa(logging_g.last_data_write, str, 10);
        console_send_str(console, str);
        console_send_str(console, " (");
        utoa(MILLIS_TO_MS(millis - logging_g.last_data_write), str, 10);
        console_send_str(console, str);
        console_send_str(console, "  milliseconds ago)");

        console_send_str(console, "\nLast superblock write: ");
        utoa(logging_g.last_sb_write, str, 10);
        console_send_str(console, str);
        console_send_str(console, " (");
        utoa(MILLIS_TO_MS(millis - logging_g.last_sb_write), str, 10);
        console_send_str(console, str);
        console_send_str(console, "  milliseconds ago)");

        // Write in progress info
        console_send_str(console, "\nSD write in progress: ");
        console_send_str(console,
                         logging_g.sd_write_in_progress ? "yes" : "no");

        // Partition info
        console_send_str(console, "\n\nPartition start: ");
        utoa(logging_g.part_start, str, 10);
        console_send_str(console, str);
        console_send_str(console, "\nPartition length: ");
        utoa(logging_g.part_blocks, str, 10);
        console_send_str(console, str);

        //  Flight info
        console_send_str(console, "\nFlight: ");
        utoa(logging_g.flight, str, 10);
        console_send_str(console, str);
        console_send_str(console, "\nFirst block in flight: ");
        utoa(logging_g.sb.flights[logging_g.flight].first_block, str, 10);
        console_send_str(console, str);
        console_send_str(console, "\nNumber of blocks in flight: ");
        utoa(logging_g.sb.flights[logging_g.flight].num_blocks, str, 10);
        console_send_str(console, str);

        // Current buffer
        console_send_str(console, "\n\nCurrent buffer: ");
        if (logging_g.insert_point == NULL) {
            console_send_str(console, "none");
        } else {
            uintptr_t buf = ((uintptr_t)logging_g.insert_point) & 0x3;
            utoa(buf, str, 10);
            console_send_str(console, str);
        }

        // Buffer info
        for (int i = 0; i < LOGGING_NUM_BUFFERS; i++) {
            console_send_str(console, "\nBuffer ");
            itoa(i, str, 10);
            console_send_str(console, str);
            console_send_str(console, " -> count: ");
            utoa(logging_g.buffer[i].count, str, 10);
            console_send_str(console, str);
            console_send_str(console, "/");
            utoa(LOGGING_BUFFER_SIZE, str, 10);
            console_send_str(console, str);
            console_send_str(console, ", checkouts: ");
            utoa(logging_g.buffer[i].checkout_count, str, 10);
            console_send_str(console, str);
            console_send_str(console, ", pending: ");
            console_send_str(console,
                             logging_g.buffer[i].pending_write ? "yes" : "no");
        }

        // Insert point
        console_send_str(console, "\nInsert point: ");
        if (logging_g.insert_point == NULL) {
            console_send_str(console, "NULL");
        } else {
            int i = 0;
            for (; i < LOGGING_NUM_BUFFERS; i++) {
                uintptr_t const start = (uintptr_t)logging_g.buffer[i].data;
                uintptr_t const end = start + LOGGING_BUFFER_SIZE;
                if (((uintptr_t)logging_g.insert_point >= start) &&
                    ((uintptr_t)logging_g.insert_point <= end)) {
                    console_send_str(console, "buffer ");
                    itoa(i, str, 10);
                    console_send_str(console, str);
                    console_send_str(console, " + ");
                    utoa((uintptr_t)logging_g.insert_point - start, str, 10);
                    console_send_str(console, str);
                    break;
                }
            }

            if (i == LOGGING_NUM_BUFFERS) {
                utoa((uintptr_t)logging_g.insert_point, str, 16);
                console_send_str(console, str);
            }
        }

        console_send_str(console, "\nNumber of missed checkouts: ");
        utoa(logging_g.out_of_space_count, str, 10);
        console_send_str(console, str);

        console_send_str(console, "\n");

        return;
    }

    if (!strcmp(argv[1], "pause")) {
        logging_pause(&logging_g);
        return;
    } else if (!strcmp(argv[1], "resume")) {
        logging_resume(&logging_g);
        return;
    } else {
        console_send_str(console, "Unkown option.\n");
    }
#else
    console_send_str(console, "Logging service not enabled.\n");
#endif
}

