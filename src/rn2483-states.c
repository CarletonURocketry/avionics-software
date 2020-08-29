/**
 * @file rn2483-states.c
 * @desc State handler functions for RN2483 driver
 * @author Samuel Dewan
 * @date 2020-02-08
 * Last Author:
 * Last Edited On:
 */

#include "rn2483.h"
#include "rn2483-states.h"

#include <string.h>
#include <ctype.h>

/** Data stored at the beginning of the instance buffer during the receive wait,
    snr and rssi states */
struct rn2483_rx_info {
    union {
        /** Buffer for responses to snr and rssi commands. Long enough for a
            four character response (negative 3 digit number) and a newline and
            carriage return */
        uint8_t rx_info_buffer[6];
        struct {
            /** Boolean value to indicate whether there is a character left over
                from the last time the rx data wait state was run */
            uint8_t have_leftover;
            /** Character left over from last time the rx data wait state was
                run */
            char leftover;
        };
    };
    /** Place to store signal to noise ratio while getting the rssi */
    int8_t snr;
    /** Place to store length of received packet while getting the snr and
        rssi */
    uint8_t length;
};

/** Describes the result of the handle_state function */
enum rn2483_state_result {
    RN2483_RES_CONTINUE,
    RN2483_RES_WAIT,
    RN2483_RES_FAILED
};

// MARK: Constants

/** Default receive window size */
#define RN2483_RX_WINDOW_SIZE   50000
/** Number of milliseconds to wait for a radio_err after getting ok from
    rxstop */
#define RN2483_RXSTOP_WAIT_TIME 5
/** Maximum number of times to try and reset radio */
#define RN2483_MAX_NUM_RESETS   5

/** Duration in milliseconds of break condition send in order to trigger the
    auto-baud detection sequence */
#define RN2483_AUTO_BAUD_BREAK_LENGTH   2

static const char* const RN2483_RSP_OK = "ok";
#define RN2483_RSP_OK_LEN 2
static const char* const RN2483_RSP_RESET_OK = "RN2483";
#define RN2483_RSP_RESET_OK_LEN 6
static const char* const RN2483_RSP_TX_OK = "radio_tx_ok";
#define RN2483_RSP_TX_OK_LEN 11
static const char* const RN2483_RSP_RX_OK = "radio_rx";
#define RN2483_RSP_RX_OK_LEN 8
static const char* const RN2483_RSP_RX_ERR = "radio_err";
#define RN2483_RSP_RX_ERR_LEN 9
static const char* const RN2483_RSP_PAUSE_MAC = "4294967245";
#define RN2483_RSP_PAUSE_MAC_LEN 10

#define RN2483_RX_DATA_OFFSET (sizeof(struct rn2483_rx_info))

static const char* const RN2483_CMD_RESET = "sys reset\r\n";
static const char* const RN2483_CMD_WDT = "radio set wdt 0\r\n";
static const char* const RN2483_CMD_PAUSE_MAC = "mac pause\r\n";

static const char* const RN2483_CMD_MODE = "radio set mod lora\r\n";
static const char* const RN2483_CMD_FREQ = "radio set freq ";
static const char* const RN2483_CMD_PWR = "radio set pwr ";
static const char* const RN2483_CMD_SF = "radio set sf ";
static const char* const RN2483_CMD_CRC = "radio set crc ";
static const char* const RN2483_CMD_IQI = "radio set iqi ";
static const char* const RN2483_CMD_CR = "radio set cr ";
static const char* const RN2483_CMD_SYNC = "radio set sync ";
static const char* const RN2483_CMD_BW = "radio set bw ";
static const char* const RN2483_CMD_PRLEN = "radio set prlen ";

static const char* const RN2483_CMD_TX = "radio tx ";
static const char* const RN2483_CMD_RX = "radio rx ";
static const char* const RN2483_CMD_SNR = "radio get snr\r\n";
static const char* const RN2483_CMD_RSSI = "radio get rssi\r\n";
static const char* const RN2483_CMD_RXSTOP = "radio rxstop\r\n";

static const char* const RN2483_CMD_SET_PINMODE = "sys set pinmode ";
static const char* const RN2483_CMD_SET_PINDIG = "sys set pindig ";
static const char* const RN2483_CMD_GET_PINDIG = "sys get pindig ";
static const char* const RN2483_CMD_GET_PINANA = "sys get pinana ";

static const char* const RN2483_STR_ON = "on\r\n";
static const char* const RN2483_STR_OFF = "off\r\n";

static const char* const RN2483_STR_SF_7 = "sf7\r\n";
static const char* const RN2483_STR_SF_8 = "sf8\r\n";
static const char* const RN2483_STR_SF_9 = "sf9\r\n";
static const char* const RN2483_STR_SF_10 = "sf10\r\n";
static const char* const RN2483_STR_SF_11 = "sf11\r\n";
static const char* const RN2483_STR_SF_12 = "sf12\r\n";

static const char* const RN2483_STR_CR_4_5 = "4/5\r\n";
static const char* const RN2483_STR_CR_4_6 = "4/6\r\n";
static const char* const RN2483_STR_CR_4_7 = "4/7\r\n";
static const char* const RN2483_STR_CR_4_8 = "4/8\r\n";

static const char* const RN2483_STR_BW125 = "125\r\n";
static const char* const RN2483_STR_BW250 = "250\r\n";
static const char* const RN2483_STR_BW500 = "500\r\n";

static const char* const RN2483_STR_PINSTATE_HIGH = " 1\r\n";
static const char* const RN2483_STR_PINSTATE_LOW = " 0\r\n";

static const char* const RN2483_STR_PIN_MODE_DIGOUT = " digout\r\n";
static const char* const RN2483_STR_PIN_MODE_DIGIN = " digin\r\n";
static const char* const RN2483_STR_PIN_MODE_ANA = " ana\r\n";

static const char* const RN2483_PIN_NAMES[] = { "GPIO0",  "GPIO1",  "GPIO2",
                                                "GPIO3",  "GPIO4",  "GPIO5",
                                                "GPIO6",  "GPIO7",  "GPIO8",
                                                "GPIO9",  "GPIO10",  "GPIO11",
                                                "GPIO12",  "GPIO13",
                                                "UART_CTS", "UART_RTS", "TEST0",
                                                "TEST1" };



// MARK: Helpers

/**
 *  Validate an RN2483 version string and store the version information.
 *
 *  @param inst The instance descriptor in which the version information should
 *              be stored
 *  @param version_string The version string to be parsed
 *
 *  @return 0 if successful
 */
static uint8_t __attribute__((pure)) parse_version (struct rn2483_desc_t *inst,
                                                    const char *version_string)
{
    size_t length = strlen(version_string);
    
    // Sanity checks
    if (length < (RN2483_RSP_RESET_OK_LEN + 6)) {
        // If we don't have at least enough length for the module number, a
        // space, 3 digits and two decimal places then the string isn't valid
        return 1;
    } else if (strncmp(RN2483_RSP_RESET_OK, version_string,
                       RN2483_RSP_RESET_OK_LEN)) {
        // Model is not correct, this isn't a valid version string from an
        // RN2483
        return 1;
    }
    
    char *end;
    
    // Parse first section (major version)
    unsigned long major = strtoul(version_string + RN2483_RSP_RESET_OK_LEN + 1,
                                  &end, 10);
    if (*end != '.') {
        // Version number format is not valid
        return 1;
    } else if (major > ((((unsigned long)1) << RN2483_VER_NUM_MAJOR_BITS) - 1)) {
        // Major version is too high for us to store
        return 1;
    }
    
    // Parse second section (minor version)
    unsigned long minor = strtoul(end + 1, &end, 10);
    if (*end != '.') {
        // Version number format is not valid
        return 1;
    } else if (minor > ((((unsigned long)1) << RN2483_VER_NUM_MINOR_BITS) - 1)) {
        // Minor version is too high for us to store
        return 1;
    }
    
    // Parse third section (revision)
    unsigned long rev = strtoul(end + 1, &end, 10);
    if (*end != ' ') {
        // Version number format is not valid
        return 1;
    } else if (rev > ((((unsigned long)1) << RN2483_VER_NUM_REV_BITS) - 1)) {
        // revision is too high for us to store
        return 1;
    }
    
    inst->version = RN2483_VERSION(major, minor, rev);
    
    return 0;
}

/**
 *  Parse a nibble from a single hexadecimal digit.
 *
 *  @param c Character containing the digit to be parsed
 *  @param dest Pointer to where data should be stored
 *  @param offset Number of bits which data should be offset within byte
 *
 *  @return 0 If successful
 */
static uint8_t parse_nibble (char c, uint8_t *dest, int offset)
{
    if (c < '0') {
        return 1;
    } else if (c >= 'a') {
        c -= 32;
    }
    c -= '0';
    if (c <= 9) {
        goto store_nibble;
    }
    c -= 7;
    if (c <= 9) {
        return 1;
    } else if (c <= 15) {
        goto store_nibble;
    }
    return 1;
store_nibble:
    *dest |= (c << offset);
    return 0;
}

/**
 *  Handle a state where a command is sent and a response is read back.
 *
 *  @param inst The RN2483 driver instance
 *  @param out_buffer The buffer from which the command will be sent, this
 *                    buffer must not change until the state is complete
 *  @param expected_response The response expected from the radio module
 *  @param compare_length The maximum number of characters to compare between
 *                        the response and expected response
 *  @param next_state The state which should be entered next
 *
 *  @return 0 if the FSM should move to the next state, something else otherwise
 */
static enum rn2483_state_result handle_state (struct rn2483_desc_t *inst,
                                              const char *out_buffer,
                                              const char *expected_response,
                                              size_t compare_length,
                                              enum rn2483_state next_state)
{
    if (inst->waiting_for_line) {
        /* response received */
        inst->waiting_for_line = 0;
        // Clear output position for next transaction
        inst->position = 0;
        // Clear command ready flag for next state
        inst->cmd_ready = 0;
        
        // Get the received line
        sercom_uart_get_line(inst->uart, inst->buffer, RN2483_BUFFER_LEN);
        
        if (!strncmp(inst->buffer, expected_response, compare_length)) {
            // Success! Go to next state
            inst->state = next_state;
            return RN2483_RES_CONTINUE;
        } else {
            // Something went wrong, go to failed state
            inst->state = RN2483_FAILED;
            return RN2483_RES_FAILED;
        }
    } else {
        /* send command */
        // Send as much of the command as we can fit in the SERCOM
        // driver's output buffer
        inst->position += sercom_uart_put_string(inst->uart,
                                                (out_buffer + inst->position));
        // If we have sent the whole command we need to wait for the
        // response from the radio
        inst->waiting_for_line = inst->position == strlen(out_buffer);
        return RN2483_RES_WAIT;
    }
}

/**
 *  Get the first pointer into an RN2483 instance buffer where a word can be
 *  stored.
 *
 *  @param inst The RN2483 driver instance
 */
static uint32_t *get_word_ptr (struct rn2483_desc_t *inst)
{
    char *buffer = inst->buffer;
    // Get the first 4 byte aligned pointer out of the buffer
    while ((uintptr_t)buffer & 0b11) {
        buffer++;
    }
    // Convince the compiler that we aligned the pointer correctly and return
    return __builtin_assume_aligned(buffer, 4);
}

void set_send_trans_state(struct rn2483_desc_t *inst, int n,
                          enum rn2483_send_trans_state state)
{
    unsigned int offset = (RN2483_SEND_TRANSACTION_SIZE * n);
    inst->send_transactions &= ~(RN2483_SEND_TRANSACTION_MASK << offset);
    inst->send_transactions |= ((state & RN2483_SEND_TRANSACTION_MASK) <<
                                offset);
}

uint8_t find_send_trans(struct rn2483_desc_t *inst,
                        enum rn2483_send_trans_state state)
{
    uint8_t id = 0;
    for (; id < RN2483_NUM_SEND_TRANSACTIONS; id++) {
        if (rn2483_get_send_state(inst, id) == state) {
            break;
        }
    }
    return id;
}


// MARK: Initialization State Handlers
static int rn2483_case_reset (struct rn2483_desc_t *inst)
{
    // Before sending reset command perform auto-baud detection sequence
    if (!inst->cmd_ready) {
        // Wait for UART TX buffer to be empty before starting reset
        if (!sercom_uart_out_buffer_empty(inst->uart)) {
            return 0;
        }
        // Send break condition to trigger auto-baud detection sequence
        sercom_uart_send_break(inst->uart, RN2483_AUTO_BAUD_BREAK_LENGTH);
        // Send 0x55 in order to set baud rate
        sercom_uart_put_char(inst->uart, 0x55);
        // Only perform auto-baud sequence once
        inst->cmd_ready = 1;

    }

    // Handle writing of command
    enum rn2483_state_result res = handle_state(inst, RN2483_CMD_RESET,
                                                RN2483_RSP_RESET_OK, 0,
                                                RN2483_WRITE_WDT);

    if (res == RN2483_RES_CONTINUE) {
        // Skip any null characters at the beginning of the line
        // Sometimes when the radio module is reset there will be a framing
        // error that can show up as a null character
        char *b = inst->buffer;
        if (*b == '\0') {
            b++;
        }
        // Parse version string
        uint8_t ret = parse_version(inst, b);
        if ((ret == 0) && (inst->version < RN2483_MINIMUM_FIRMWARE)) {
            // Version number too low
            inst->state = RN2483_FAILED;
            return 0;
        } else if (ret == 0) {
            // Successfully parsed version and it is high enough
            return 1;
        }
    } else if (res == RN2483_RES_WAIT) {
        return 0;
    }

    // If we make it here we got some kind of bad response
    if (inst->reset_try_count >= RN2483_MAX_NUM_RESETS) {
        // Give up
        inst->state = RN2483_FAILED;
    } else {
        // Try again
        inst->reset_try_count++;
        inst->state = RN2483_RESET;
        inst->cmd_ready = 1;
    }

    return 0;
}

static int rn2483_case_write_wdt (struct rn2483_desc_t *inst)
{
    // Handle writing of command and reception of response
    if (handle_state(inst, RN2483_CMD_WDT, RN2483_RSP_OK,
                     RN2483_RSP_OK_LEN, RN2483_PAUSE_MAC) !=
                    RN2483_RES_CONTINUE) {
        return 0;
    }
    return 1;
}

static int rn2483_case_pause_mac (struct rn2483_desc_t *inst)
{
    // Handle writing of command and reception of response
    if (handle_state(inst, RN2483_CMD_PAUSE_MAC, RN2483_RSP_PAUSE_MAC,
                     RN2483_RSP_PAUSE_MAC_LEN, RN2483_WRITE_MODE) !=
                    RN2483_RES_CONTINUE) {
        return 0;
    }
    return 1;
}

static int rn2483_case_write_mode (struct rn2483_desc_t *inst)
{
    // Handle writing of command and reception of response
    if (handle_state(inst, RN2483_CMD_MODE, RN2483_RSP_OK,
                     RN2483_RSP_OK_LEN, RN2483_WRITE_FREQ) !=
                    RN2483_RES_CONTINUE) {
        return 0;
    }
    return 1;
}

static int rn2483_case_write_freq (struct rn2483_desc_t *inst)
{
    // Update command if required
    if (!inst->cmd_ready) {
        // Create command
        strcpy(inst->buffer, RN2483_CMD_FREQ);
        size_t i = strlen(RN2483_CMD_FREQ);
        utoa(inst->settings->freq, inst->buffer + i, 10);
        i = strlen(inst->buffer);
        *(inst->buffer + i + 0) = '\r';
        *(inst->buffer + i + 1)  = '\n';
        *(inst->buffer + i + 2)  = '\0';
        inst->cmd_ready = 1;
    }
    // Determine which state we need to go to next
    enum rn2483_state next = ((inst->state == RN2483_UPDATE_FREQ) ?
                                    RN2483_IDLE : RN2483_WRITE_PWR);
    // Handle writing of command and reception of response
    if (handle_state(inst, inst->buffer, RN2483_RSP_OK, RN2483_RSP_OK_LEN,
                     next) != RN2483_RES_CONTINUE) {
        return 0;
    }
    return 1;
}

static int rn2483_case_write_pwr (struct rn2483_desc_t *inst)
{
    // Update command if required
    if (!inst->cmd_ready) {
        // Create command
        strcpy(inst->buffer, RN2483_CMD_PWR);
        size_t i = strlen(RN2483_CMD_PWR);
        itoa(inst->settings->power, inst->buffer + i, 10);
        i = strlen(inst->buffer);
        *(inst->buffer + i + 0) = '\r';
        *(inst->buffer + i + 1)  = '\n';
        *(inst->buffer + i + 2)  = '\0';
        inst->cmd_ready = 1;
    }
    // Handle writing of command and reception of response
    if (handle_state(inst, inst->buffer, RN2483_RSP_OK,
                     RN2483_RSP_OK_LEN, RN2483_WRITE_SF) !=
                    RN2483_RES_CONTINUE) {
        return 0;
    }
    return 1;
}

static int rn2483_case_write_sf (struct rn2483_desc_t *inst)
{
    // Update command if required
    if (!inst->cmd_ready) {
        // Create command
        strcpy(inst->buffer, RN2483_CMD_SF);
        size_t i = strlen(RN2483_CMD_SF);
        switch (inst->settings->spreading_factor) {
            case RN2483_SF_SF7:
                strcpy(inst->buffer + i, RN2483_STR_SF_7);
                break;
            case RN2483_SF_SF8:
                strcpy(inst->buffer + i, RN2483_STR_SF_8);
                break;
            case RN2483_SF_SF9:
                strcpy(inst->buffer + i, RN2483_STR_SF_9);
                break;
            case RN2483_SF_SF10:
                strcpy(inst->buffer + i, RN2483_STR_SF_10);
                break;
            case RN2483_SF_SF11:
                strcpy(inst->buffer + i, RN2483_STR_SF_11);
                break;
            case RN2483_SF_SF12:
                strcpy(inst->buffer + i, RN2483_STR_SF_12);
                break;
        }
        inst->cmd_ready = 1;
    }
    // Handle writing of command and reception of response
    if (handle_state(inst, inst->buffer, RN2483_RSP_OK,
                     RN2483_RSP_OK_LEN, RN2483_WRITE_CRC) !=
                    RN2483_RES_CONTINUE) {
        return 0;
    }
    return 1;
}

static int rn2483_case_write_crc (struct rn2483_desc_t *inst)
{
    // Update command if required
    if (!inst->cmd_ready) {
        // Create command
        strcpy(inst->buffer, RN2483_CMD_CRC);
        size_t i = strlen(RN2483_CMD_CRC);
        if (inst->settings->crc) {
            strcpy(inst->buffer + i, RN2483_STR_ON);
        } else {
            strcpy(inst->buffer + i, RN2483_STR_OFF);
        }
        inst->cmd_ready = 1;
    }
    // Handle writing of command and reception of response
    if (handle_state(inst, inst->buffer, RN2483_RSP_OK,
                     RN2483_RSP_OK_LEN, RN2483_WRITE_IQI) !=
                    RN2483_RES_CONTINUE) {
        return 0;
    }
    return 1;
}

static int rn2483_case_write_iqi (struct rn2483_desc_t *inst)
{
    // Update command if required
    if (!inst->cmd_ready) {
        // Create command
        strcpy(inst->buffer, RN2483_CMD_IQI);
        size_t i = strlen(RN2483_CMD_IQI);
        if (inst->settings->invert_qi) {
            strcpy(inst->buffer + i, RN2483_STR_ON);
        } else {
            strcpy(inst->buffer + i, RN2483_STR_OFF);
        }
        inst->cmd_ready = 1;
    }
    // Handle writing of command and reception of response
    if (handle_state(inst, inst->buffer, RN2483_RSP_OK,
                     RN2483_RSP_OK_LEN, RN2483_WRITE_CR) !=
                    RN2483_RES_CONTINUE) {
        return 0;
    }
    return 1;
}

static int rn2483_case_write_cr (struct rn2483_desc_t *inst)
{
    // Update command if required
    if (!inst->cmd_ready) {
        // Create command
        strcpy(inst->buffer, RN2483_CMD_CR);
        size_t i = strlen(RN2483_CMD_CR);
        switch (inst->settings->coding_rate) {
            case RN2483_CR_4_5:
                strcpy(inst->buffer + i, RN2483_STR_CR_4_5);
                break;
            case RN2483_CR_4_6:
                strcpy(inst->buffer + i, RN2483_STR_CR_4_6);
                break;
            case RN2483_CR_4_7:
                strcpy(inst->buffer + i, RN2483_STR_CR_4_7);
                break;
            case RN2483_CR_4_8:
                strcpy(inst->buffer + i, RN2483_STR_CR_4_8);
                break;
        }
        inst->cmd_ready = 1;
    }
    // Handle writing of command and reception of response
    if (handle_state(inst, inst->buffer, RN2483_RSP_OK,
                     RN2483_RSP_OK_LEN, RN2483_WRITE_SYNC) !=
                    RN2483_RES_CONTINUE) {
        return 0;
    }
    return 1;
}

static int rn2483_case_write_sync (struct rn2483_desc_t *inst)
{
    // Update command if required
    if (!inst->cmd_ready) {
        // Create command
        strcpy(inst->buffer, RN2483_CMD_SYNC);
        size_t i = strlen(RN2483_CMD_SYNC);
        utoa(inst->settings->sync_byte, inst->buffer + i, 10);
        i = strlen(inst->buffer);
        *(inst->buffer + i + 0) = '\r';
        *(inst->buffer + i + 1)  = '\n';
        *(inst->buffer + i + 2)  = '\0';
        inst->cmd_ready = 1;
    }
    // Handle writing of command and reception of response
    if (handle_state(inst, inst->buffer, RN2483_RSP_OK,
                     RN2483_RSP_OK_LEN, RN2483_WRITE_BW) !=
                    RN2483_RES_CONTINUE) {
        return 0;
    }
    return 1;
}

static int rn2483_case_write_bw (struct rn2483_desc_t *inst)
{
    // Update command if required
    if (!inst->cmd_ready) {
        // Create command
        strcpy(inst->buffer, RN2483_CMD_BW);
        size_t i = strlen(RN2483_CMD_BW);
        switch (inst->settings->bandwidth) {
            case RN2483_BW_125:
                strcpy(inst->buffer + i, RN2483_STR_BW125);
                break;
            case RN2483_BW_250:
                strcpy(inst->buffer + i, RN2483_STR_BW250);
                break;
            case RN2483_BW_500:
                strcpy(inst->buffer + i, RN2483_STR_BW500);
                break;
        }
        inst->cmd_ready = 1;
    }
    // Handle writing of command and reception of response
    if (handle_state(inst, inst->buffer, RN2483_RSP_OK,
                     RN2483_RSP_OK_LEN, RN2483_WRITE_PRLEN) !=
                    RN2483_RES_CONTINUE) {
        return 0;
    }
    return 1;
}

static int rn2483_case_write_prlen (struct rn2483_desc_t *inst)
{
    // Update command if required
    if (!inst->cmd_ready) {
        // Create command
        strcpy(inst->buffer, RN2483_CMD_PRLEN);
        size_t i = strlen(RN2483_CMD_PRLEN);
        utoa(inst->settings->preamble_length, inst->buffer + i, 10);
        i = strlen(inst->buffer);
        *(inst->buffer + i + 0) = '\r';
        *(inst->buffer + i + 1)  = '\n';
        *(inst->buffer + i + 2)  = '\0';
        inst->cmd_ready = 1;
    }
    // Handle writing of command and reception of response
    if (handle_state(inst, inst->buffer, RN2483_RSP_OK,
                     RN2483_RSP_OK_LEN, RN2483_IDLE) !=
                    RN2483_RES_CONTINUE) {
        return 0;
    }
    return 1;
}

// MARK: Idle State Handler

static int rn2483_case_idle (struct rn2483_desc_t *inst)
{
    /* If the settings or frequency are dirty start updating them */
    if (inst->settings_dirty) {
        // Jump to first initialization state
        inst->state = RN2483_WRITE_WDT;
        inst->settings_dirty = 0;
        inst->frequency_dirty = 0;
        return 1;
    } else if (inst->frequency_dirty) {
        // Jump to update frequency state
        inst->state = RN2483_UPDATE_FREQ;
        inst->frequency_dirty = 0;
        return 1;
    }

    /* Check if enough time has elapsed that we should mark our inputs
        dirty */
    if (RN2483_GPIO_UPDATE_PERIOD &&
        ((millis - inst->last_polled) > RN2483_GPIO_UPDATE_PERIOD)) {
        inst->last_polled = millis;
        rn2483_poll_gpio(inst);
    }
    
    /* Check for pins with dirty modes */
    for (inst->current_pin = 0; inst->current_pin < RN2483_NUM_PINS;
         inst->current_pin++) {
        if (inst->pins[inst->current_pin].mode_dirty) {
            break;
        }
    }
    if (inst->current_pin < RN2483_NUM_PINS) {
        // There is a pin with a dirty mode, update it
        inst->state = RN2483_SET_PIN_MODE;
        // Handle next state right away
        return 1;
    }
    
    /* Check for pins with dirty values */
    for (inst->current_pin = 0; inst->current_pin < RN2483_NUM_PINS;
         inst->current_pin++) {
        if (inst->pins[inst->current_pin].value_dirty) {
            break;
        }
    }
    if (inst->current_pin < RN2483_NUM_PINS) {
        // There is a pin with a dirty value, update it
        if (inst->pins[inst->current_pin].mode ==
            RN2483_PIN_MODE_OUTPUT) {
            inst->state = RN2483_SET_PINDIG;
        } else {
            inst->state = RN2483_GET_PIN_VALUE;
        }
        // Handle next state right away
        return 1;
    }
    
    /* Check if we need to be sending anything */
    if (inst->send_buffer != NULL) {
        inst->state = RN2483_SEND;
        return 1;
    }
    
    /* Start a reception if continuous receiving is enabled */
    if (inst->receive) {
        inst->state = RN2483_RECEIVE;
        return 1;
    }
    
    return 0;
}

// MARK: Send State Handlers

static int rn2483_case_send (struct rn2483_desc_t *inst)
{
    if (!inst->waiting_for_line) {
        // Continue sending command
        uint8_t cmd_len = (uint8_t)strlen(RN2483_CMD_TX);
        uint8_t data_len = (uint8_t)inst->send_length * 2;
        if (inst->position < cmd_len) {
            // Still sending command
            inst->position += sercom_uart_put_string(inst->uart,
                                                     (RN2483_CMD_TX +
                                                      inst->position));
            if (inst->position < cmd_len) {
                // Didn't finish sending command, uart buffer must be full
                return 0;
            }
        }
        
        // Send data
        while (inst->position < (cmd_len + data_len)) {
            uint8_t data_pos = inst->position - cmd_len;
            uint8_t i = data_pos / 2;
            uint8_t shift = (data_pos & 1) ? 0 : 4;
            
            uint8_t data = (inst->send_buffer[i] >> shift) & 0xF;
            char str[2];
            str[0] = (data < 10) ? ('0' + data) : ('A' + data - 10);
            str[1] = '\0';
            
            uint8_t sent = sercom_uart_put_string(inst->uart, str);
            
            if (sent == 0) {
                // Character was not sent, uart buffer must be full
                return 0;
            }
            
            inst->position++;
        }
        
        // Sending line terminator
        if (inst->position == (cmd_len + data_len)) {
            // Sending "\r\n"
            inst->position += sercom_uart_put_string(inst->uart, "\r\n");
            
            if (inst->position < (cmd_len + data_len + 2)) {
                // uart buffer must be full
                return 0;
            }
        } else {
            // Just sending "\n"
            inst->position += sercom_uart_put_string(inst->uart, "\n");
            
            if (inst->position < (cmd_len + data_len + 2)) {
                // uart buffer must be full
                return 0;
            }
        }
        
        // Done sending line
        inst->waiting_for_line = 1;
        inst->send_buffer = NULL;
        
        // Find send transaction and update state
        uint8_t id = find_send_trans(inst, RN2483_SEND_TRANS_PENDING);
        set_send_trans_state(inst, id, RN2483_SEND_TRANS_WRITTEN);
        
        return 0;
    } else if (sercom_uart_has_line(inst->uart)) {
        // Clear output position for next transaction
        inst->position = 0;
        // Get the received line
        sercom_uart_get_line(inst->uart, inst->buffer, RN2483_BUFFER_LEN);
        
        if (!strncmp(inst->buffer, RN2483_RSP_OK, RN2483_RSP_OK_LEN)) {
            // Success! Wait for second response
            inst->state = RN2483_SEND_WAIT;
        } else {
            // Something went wrong, go back to idle state
            inst->state = RN2483_IDLE;
            // Got a response
            inst->waiting_for_line = 0;
            // Find transaction number for transaction to be updated
            uint8_t id = find_send_trans(inst, RN2483_SEND_TRANS_WRITTEN);
            // Update transaction state
            set_send_trans_state(inst, id, RN2483_SEND_TRANS_FAILED);
        }
        return 1;
    }
    return 0;
}

static int rn2483_case_send_wait (struct rn2483_desc_t *inst)
{
    // Wait for second response and return to idle
    if (handle_state(inst, inst->buffer, RN2483_RSP_TX_OK, RN2483_RSP_TX_OK_LEN,
                     RN2483_IDLE) == RN2483_RES_CONTINUE) {
        // Success! Sending is complete
        // Find transaction number for transaction to be updated
        uint8_t id = find_send_trans(inst, RN2483_SEND_TRANS_WRITTEN);
        // Update transaction state
        set_send_trans_state(inst, id, RN2483_SEND_TRANS_DONE);
    } else if (inst->state == RN2483_FAILED) {
        // Sending failed, go back to idle
        inst->state = RN2483_IDLE;
        // Find transaction number for transaction to be updated
        uint8_t id = find_send_trans(inst, RN2483_SEND_TRANS_WRITTEN);
        // Update transaction state
        set_send_trans_state(inst, id, RN2483_SEND_TRANS_FAILED);
    } else {
        // Still waiting
        return 0;
    }
    return 1;
}

// MARK: Receive State Handlers

static int rn2483_case_receive (struct rn2483_desc_t *inst)
{
    // Update command if required
    if (!inst->cmd_ready) {
        // Create command
        strcpy(inst->buffer, RN2483_CMD_RX);
        size_t i = strlen(RN2483_CMD_RX);
        if (inst->version >= RN2483_MIN_FW_RXSTOP) {
            // If we support rxstop we can start receiving indefinitely
            utoa(0, inst->buffer + i, 10);
        } else {
            // If we do not support rxstop we need to use a window
            utoa(RN2483_RX_WINDOW_SIZE, inst->buffer + i, 10);
        }
        i = strlen(inst->buffer);
        *(inst->buffer + i + 0) = '\r';
        *(inst->buffer + i + 1)  = '\n';
        *(inst->buffer + i + 2)  = '\0';
        inst->cmd_ready = 1;
    }
    // Record whether we need to stop receiving right away (if possible) when we
    // get the first response to the receive command
    uint8_t abort = inst->state == RN2483_RECEIVE_ABORT;
    // Handle writing of command and reception of response
    if (handle_state(inst, inst->buffer, RN2483_RSP_OK, RN2483_RSP_OK_LEN,
                     RN2483_RX_OK_WAIT) == RN2483_RES_CONTINUE) {
        if (abort && (inst->version >= RN2483_MIN_FW_RXSTOP)) {
            // We need to stop receiving right away
            inst->state = RN2483_RXSTOP;
            return 1;
        }
        // Got first response to receive command
        // Receive has started, wait for the second response
    } else if (inst->state == RN2483_FAILED) {
        // Failed to start receiving
        inst->receive_callback(inst, inst->callback_context, NULL, 0, 0, 0);
        inst->receive = 0;
        // Go back to idle
        inst->state = RN2483_IDLE;
    }
    return 0;
}

static int handle_rx_timeout(struct rn2483_desc_t *inst)
{
    if (!inst->receive) {
        // This is a one-off receive, notify caller that receive failed
        inst->receive_callback(inst, inst->callback_context, NULL, 0, 0, 0);
    }
    // If we are in continuous receive mode we don't call the callback every
    // time a receive times out, we just start receiving again.

    // Go back to idle
    inst->state = RN2483_IDLE;

    // Could be ready to send another command or start receiving again
    // right away, continue directly to next state
    return 1;
}

static int rn2483_case_rx_ok_wait (struct rn2483_desc_t *inst)
{
    // Check if we have received a full rx ok message or rx failed message
    if (sercom_uart_has_delim(inst->uart, ' ')) {
        // Get the message we received
        sercom_uart_get_line_delim(inst->uart, ' ', inst->buffer,
                                   RN2483_BUFFER_LEN);
        // Double check that it is actually the correct message
        if (strncmp(inst->buffer, RN2483_RSP_RX_OK, RN2483_RSP_RX_OK_LEN)) {
            // Not the correct response, assume that we probably timed out
            return handle_rx_timeout(inst);
        }
        // Set up the rx info structure in the buffer
        struct rn2483_rx_info *const rx_info =
                                        (struct rn2483_rx_info*)inst->buffer;
        rx_info->have_leftover = 0;
        rx_info->length = 0;
        // We got the correct response! Start parsing the data from the message
        inst->state = RN2483_RX_DATA_WAIT;
        return 1;
    } else if (sercom_uart_has_line(inst->uart)) {
        // We have received a full line without any spaces in it. This is an
        // error message of some kind because the rx ok message is always
        // followed by a space.

        // Flush the line
        sercom_uart_get_line(inst->uart, inst->buffer, RN2483_BUFFER_LEN);
        // Our receive probably timed out
        return handle_rx_timeout(inst);
    }
    // We have not received a response yet, try again later
    return 0;
}

static int rn2483_case_rx_data_wait (struct rn2483_desc_t *inst)
{
    struct rn2483_rx_info *const rx_info = (struct rn2483_rx_info*)inst->buffer;
    // Calculate where we should be putting the next received byte
    const uint16_t offset = RN2483_RX_DATA_OFFSET + rx_info->length;

    // Keep trying to get more bytes and parse them until we run out of bytes in
    // the uart's rx buffer
    uint8_t chars_in;
    uint8_t new_char;
    char rx_data[3];
    do {
        // Get up to the next two bytes of data from the uart in buffer
        sercom_uart_get_string(inst->uart, rx_data, 2);
        chars_in = strlen(rx_data);

        if (chars_in == 0) {
            // No new data to parse
            break;
        }

        // Skip spaces and determine how many new usable char we have
        if ((chars_in == 1) && (rx_data[0] == ' ')) {
            // The only char we got was a space, no new data to parse
            break;
        } else if ((chars_in == 2) && (rx_data[0] == ' ') &&
                   (rx_data[1] == ' ')) {
            // The only two new chars we got where spaces, no new data to parse
            // (but since we got a full two chars there could be more chars in
            // the uart buffer)
            continue;
        } else if ((chars_in == 2) && (rx_data[0] == ' ')) {
            // The first char we got is a space but the second one is good,
            // shift the second char over and pretend we never received the
            // first one
            rx_data[0] = rx_data[1];
            new_char = 1;
        } else if ((chars_in == 2) && (rx_data[1] == ' ')) {
            // The second char we got is a space, set new_chars to 1 and pretend
            // we never received the second one
            new_char = 1;
        } else {
            // All of the chars we got from the buffer are not spaces
            new_char = chars_in;
        }

        if (!rx_info->have_leftover && (new_char == 1)) {
            // We only have one char to work with, just store it as a leftover
            // for next time
            rx_info->leftover = rx_data[0];
            rx_info->have_leftover = 1;
            break;
        }

        // At this point we have two chars to work with. We should be able to
        // parse out a byte of received data (or find the end of the line).

        // Identify our high and low chars
        char high_char;
        char low_char;
        if (rx_info->have_leftover) {
            high_char = rx_info->leftover;
            low_char = rx_data[0];
            if (new_char == 2) {
                // We have a char left over for next time
                rx_info->leftover = rx_data[1];
            } else {
                rx_info->have_leftover = 0;
            }
        } else {
            high_char = rx_data[0];
            low_char = rx_data[1];
        }

        // Check if we have a newline
        if ((high_char == '\r') && (low_char == '\n')) {
            // That's all the data, get the SNR now
            inst->state = RN2483_GET_SNR;
            return 1;
        } else if (!isxdigit(high_char) || !isxdigit(low_char)) {
            // If our pair of chars is not a newline and either of our chars are
            // not a hexadecimal digit a few different things are could be going
            // on:
            //      - We received an odd number of hexadecimal digits before the
            //        newline so we are now looking at a valid digit and a
            //        carriage return together
            //      - We received a character that is neither a valid digit, a
            //        carriage return nor a newline
            //      - The carriage return and newline are backwards
            //      - We received a a carriage return and then something after
            //        it that was not a newline
            // No matter what it is, something has gone very wrong and we can't
            // keep parsing the data. Because we don't know what the radio might
            // send next (we could still be in the middle of a line) we need to
            // go straight to the failed state.
            inst->state = RN2483_FAILED;
            return 0;
        } else if (offset >= RN2483_BUFFER_LEN) {
            // We have a valid byte to parse, but there is no space left in the
            // buffer. We don't have a lot of options here, but in the interest
            // of not crashing the radio driver if we receive a packet that is
            // too big we are going to just ignore any bytes that don't fit in
            // the buffer and pretend that they never happened
            continue;
        } else {
            // We have a valid byte to parse and space for it in the buffer
            inst->buffer[offset] = 0;
            uint8_t ret = parse_nibble(high_char,
                                       (uint8_t*)&inst->buffer[offset], 4);
            ret |= parse_nibble(low_char, (uint8_t*)&inst->buffer[offset], 0);
            if (ret != 0) {
                // This should never happen because we already checked that
                // both of our bytes are valid hex chars
                inst->state = RN2483_FAILED;
                return 0;
            }
            rx_info->length++;
        }
    } while (chars_in == 2);

    return 0;
}

static int rn2483_case_get_snr (struct rn2483_desc_t *inst)
{
    if (handle_state(inst, RN2483_CMD_SNR, RN2483_RSP_OK, 0,
                     RN2483_GET_RSSI) != RN2483_RES_CONTINUE) {
        return 0;
    }
    /* Got the SNR from the radio */
    // Parse received SNR
    int8_t snr = (int8_t)strtol(inst->buffer, NULL, 10);
    // Save SNR in buffer so that we can get it after we have the
    // RSSI
    struct rn2483_rx_info *const rx_info = (struct rn2483_rx_info*)inst->buffer;
    rx_info->snr = snr;
    // Continue directly to next state
    return 1;
}

static int rn2483_case_get_rssi (struct rn2483_desc_t *inst)
{
    int8_t rssi = INT8_MIN;
    if (inst->version >= RN2483_MIN_FW_RSSI) {
        if (handle_state(inst, RN2483_CMD_RSSI, RN2483_RSP_OK, 0,
                         RN2483_IDLE) != RN2483_RES_CONTINUE) {
            return 0;
        }
        /* Got the RSSI from the radio */
        // Parse received RSSI
        rssi = (int8_t)strtol(inst->buffer, NULL, 10);
    }
    
    // Get rx info from buffer
    struct rn2483_rx_info *const rx_info = (struct rn2483_rx_info*)inst->buffer;

    // Call receive callback
    int ret = inst->receive_callback(inst, inst->callback_context,
                                     (uint8_t*)inst->buffer +
                                                        RN2483_RX_DATA_OFFSET,
                                     rx_info->length, rx_info->snr, rssi);
    
    // Receive is finished
    if (!ret) {
        inst->receive = 0;
    }
    
    return 1;
}

static int rn2483_case_rxstop (struct rn2483_desc_t *inst)
{
    // Make note of whether we need to continue on to get the SNR once we have
    // gotten the ok response from the rxstop command
    uint8_t received = inst->state == RN2483_RXSTOP_RECEIVED;
    
    // Send rxstop command and get response
    if (handle_state(inst, RN2483_CMD_RXSTOP, RN2483_RSP_OK, 0,
                     RN2483_IDLE) != RN2483_RES_CONTINUE) {
        return 0;
    }
    
    // Check response
    if (!strncmp(inst->buffer, RN2483_RSP_OK, RN2483_RSP_OK_LEN)) {
        // Got ok response from rxstop command
        if (received) {
            // Need to parse received data
            inst->state = RN2483_GET_SNR;
        } else {
            // Need to get the error response to rx command
            inst->state = RN2483_RXSTOP_GET_ERROR;
            // Store current time so we know how long to wait for
            *get_word_ptr(inst) = millis;
        }
    } else if (!strncmp(inst->buffer, RN2483_RSP_RX_OK, RN2483_RSP_RX_OK_LEN)) {
        // Received a packet
        // We still need to get the ok response to the rxstop command before we
        // can continue on to parsing the received data
        inst->state = RN2483_RXSTOP_RECEIVED;
        inst->waiting_for_line = 1;
    } else {
        // Receive failed
        // We still need to get the ok response to the rxstop command
        inst->state = RN2483_RXSTOP;
        inst->waiting_for_line = 1;
    }
    
    return 1;
}

static int rn2483_case_rxstop_get_error (struct rn2483_desc_t *inst)
{
    if (sercom_uart_has_line(inst->uart)) {
        // Get the received line
        sercom_uart_get_line(inst->uart, inst->buffer, RN2483_BUFFER_LEN);
        
        if (!strncmp(inst->buffer, RN2483_RSP_RX_ERR, RN2483_RSP_RX_ERR_LEN)) {
            // Got error response
            inst->state = RN2483_IDLE;
            return 1;
        } else {
            // Got something unexpected
            inst->state = RN2483_FAILED;
            return 0;
        }
    } else if ((millis - *get_word_ptr(inst)) > RN2483_RXSTOP_WAIT_TIME) {
        // Done waiting for error
        inst->state = RN2483_IDLE;
        return 1;
    }
    
    return 0;
}

// MARK: GPIO State Handlers

static int rn2483_case_set_pin_mode (struct rn2483_desc_t *inst)
{
    // Update command if required
    if (!inst->cmd_ready) {
        // Create command
        strcpy(inst->buffer, RN2483_CMD_SET_PINMODE);
        size_t i = strlen(inst->buffer);
        strcpy(inst->buffer + i, RN2483_PIN_NAMES[inst->current_pin]);
        i = strlen(inst->buffer);
        switch (inst->pins[inst->current_pin].mode) {
            case RN2483_PIN_MODE_OUTPUT:
                strcpy(inst->buffer + i, RN2483_STR_PIN_MODE_DIGOUT);
                break;
            case RN2483_PIN_MODE_INPUT:
                strcpy(inst->buffer + i, RN2483_STR_PIN_MODE_DIGIN);
                break;
            case RN2483_PIN_MODE_ANALOG:
                strcpy(inst->buffer + i, RN2483_STR_PIN_MODE_ANA);
                break;
        }
        inst->cmd_ready = 1;
    }
    // Handle writing of command and reception of response
    if (handle_state(inst, inst->buffer, RN2483_RSP_OK,
                     RN2483_RSP_OK_LEN, RN2483_IDLE) == RN2483_RES_CONTINUE) {
        // Pin's mode is now clean
        inst->pins[inst->current_pin].mode_dirty = 0;
        // Handle next state right away
        return 1;
    }
    return 0;
}

static int rn2483_case_set_pindig (struct rn2483_desc_t *inst)
{
    // Update command if required
    if (!inst->cmd_ready) {
        // Create command
        strcpy(inst->buffer, RN2483_CMD_SET_PINDIG);
        size_t i = strlen(inst->buffer);
        strcpy(inst->buffer + i, RN2483_PIN_NAMES[inst->current_pin]);
        i = strlen(inst->buffer);
        if (inst->pins[inst->current_pin].value) {
            strcpy(inst->buffer + i, RN2483_STR_PINSTATE_HIGH);
        } else {
            strcpy(inst->buffer + i, RN2483_STR_PINSTATE_LOW);
        }
        inst->cmd_ready = 1;
    }
    // Handle writing of command and reception of response
    if (handle_state(inst, inst->buffer, RN2483_RSP_OK,
                     RN2483_RSP_OK_LEN, RN2483_IDLE) == RN2483_RES_CONTINUE) {
        // Pin's value is now clean
        inst->pins[inst->current_pin].value_dirty = 0;
        // Handle next state right away
        return 1;
    }
    return 0;
}

static int rn2483_case_get_pin_value (struct rn2483_desc_t *inst)
{
    // Update command if required
    if (!inst->cmd_ready) {
        // Create command
        switch (inst->pins[inst->current_pin].mode) {
            case RN2483_PIN_MODE_INPUT:
                strcpy(inst->buffer, RN2483_CMD_GET_PINDIG);
                break;
            case RN2483_PIN_MODE_ANALOG:
                strcpy(inst->buffer, RN2483_CMD_GET_PINANA);
                break;
            default:
                // This should not happen, go back to idle
                inst->state = RN2483_IDLE;
                return 0;
        }
        size_t i = strlen(inst->buffer);
        strcpy(inst->buffer + i, RN2483_PIN_NAMES[inst->current_pin]);
        i = strlen(inst->buffer);
        *(inst->buffer + i + 0) = '\r';
        *(inst->buffer + i + 1)  = '\n';
        *(inst->buffer + i + 2)  = '\0';
        inst->cmd_ready = 1;
    }
    // Handle writing of command and reception of response
    if (handle_state(inst, inst->buffer, RN2483_RSP_OK, 0,
                     RN2483_IDLE) == RN2483_RES_CONTINUE) {
        // Parse and store received value
        uint16_t value = (uint16_t) strtoul(inst->buffer, NULL, 10);
        inst->pins[inst->current_pin].value = value;
        // Pin value is no longer dirty
        inst->pins[inst->current_pin].value_dirty = 0;
        // Handle next state right away
        return 1;
    }
    return 0;
}

static int rn2483_case_failed (struct rn2483_desc_t *inst)
{
    // This should not happen
    return 0;
}


// MARK: State Handlers Table

const rn2483_state_handler_t rn2483_state_handlers[] = {
    rn2483_case_reset,              // RN2483_RESET
    rn2483_case_write_wdt,          // RN2483_WRITE_WDT
    rn2483_case_pause_mac,          // RN2483_PAUSE_MAC
    rn2483_case_write_mode,         // RN2483_WRITE_MODE
    rn2483_case_write_freq,         // RN2483_WRITE_FREQ
    rn2483_case_write_freq,         // RN2483_UPDATE_FREQ
    rn2483_case_write_pwr,          // RN2483_WRITE_PWR
    rn2483_case_write_sf,           // RN2483_WRITE_SF
    rn2483_case_write_crc,          // RN2483_WRITE_CRC
    rn2483_case_write_iqi,          // RN2483_WRITE_IQI
    rn2483_case_write_cr,           // RN2483_WRITE_CR
    rn2483_case_write_sync,         // RN2483_WRITE_SYNC
    rn2483_case_write_bw,           // RN2483_WRITE_BW
    rn2483_case_write_prlen,        // RN2483_WRITE_PRLEN
    rn2483_case_idle,               // RN2483_IDLE
    rn2483_case_send,               // RN2483_SEND
    rn2483_case_send_wait,          // RN2483_SEND_WAIT
    rn2483_case_receive,            // RN2483_RECEIVE
    rn2483_case_receive,            // RN2483_RECEIVE_ABORT
    rn2483_case_rx_ok_wait,         // RN2483_RX_OK_WAIT
    rn2483_case_rx_data_wait,       // RN2483_RX_DATA_WAIT
    rn2483_case_get_snr,            // RN2483_GET_SNR
    rn2483_case_get_rssi,           // RN2483_GET_RSSI
    rn2483_case_rxstop,             // RN2483_RXSTOP
    rn2483_case_rxstop,             // RN2483_RXSTOP_RECEIVED
    rn2483_case_rxstop_get_error,   // RN2483_RXSTOP_GET_ERROR
    rn2483_case_set_pin_mode,       // RN2483_SET_PIN_MODE
    rn2483_case_set_pindig,         // RN2483_SET_PINDIG
    rn2483_case_get_pin_value,      // RN2483_GET_PIN_VALUE
    rn2483_case_failed              // RN2483_FAILED
};

