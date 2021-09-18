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

