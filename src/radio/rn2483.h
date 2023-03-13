/**
 * @file rn2483.h
 * @desc Driver for RN2483 LoRa radio
 * @author Samuel Dewan
 * @date 2019-06-07
 * Last Author:
 * Last Edited On:
 */

#ifndef rn2483_h
#define rn2483_h

#include "global.h"

#include "sercom-uart.h"

// Buffer length is enough send a 128 byte packet:
//      (128 * 2) + tx cmd length (9)
#define RN2483_BUFFER_LEN   265

#define RN2483_FREQ_MIN 433050000UL
#define RN2483_FREQ_MAX 434790000UL

#define RN2483_PWR_MIN  -3
#define RN2483_PWR_MAX  14

#define RN2483_NUM_PINS 18

/** Period in milliseconds at which inputs should be polled, if 0 inputs will
 not be polled automatically */
#define RN2483_GPIO_UPDATE_PERIOD 0

struct rn2483_desc_t;

/**
 *  RN2483 Bandwidth
 */
enum rn2483_state {
    RN2483_RESET,
    RN2483_WRITE_WDT,
    RN2483_PAUSE_MAC,
    RN2483_WRITE_MODE,
    RN2483_WRITE_FREQ,
    RN2483_UPDATE_FREQ,
    RN2483_WRITE_PWR,
    RN2483_WRITE_SF,
    RN2483_WRITE_CRC,
    RN2483_WRITE_IQI,
    RN2483_WRITE_CR,
    RN2483_WRITE_SYNC,
    RN2483_WRITE_BW,
    RN2483_WRITE_PRLEN,
    RN2483_IDLE,
    RN2483_SEND,
    RN2483_SEND_WAIT,
    RN2483_RECEIVE,
    RN2483_RECEIVE_ABORT,
    RN2483_RX_OK_WAIT,
    RN2483_RX_DATA_WAIT,
    RN2483_GET_SNR,
    RN2483_GET_RSSI,
    RN2483_RXSTOP,
    RN2483_RXSTOP_RECEIVED,
    RN2483_RXSTOP_GET_ERROR,
    RN2483_SET_PIN_MODE,
    RN2483_SET_PINDIG,
    RN2483_GET_PIN_VALUE,
    RN2483_FAILED
};

/**
 *  Status code returned when starting a radio operation
 */
enum rn2483_operation_result {
    /** Operation was started successfully */
    RN2483_OP_SUCCESS,
    /** Radio is busy */
    RN2483_OP_BUSY,
    /** Data size is too long */
    RN2483_OP_TOO_LONG,
    /** Radio driver cannot start operation from its current state */
    RN2483_OP_BAD_STATE
};


/**
 *  RN2483 Spreading Factor
 */
enum rn2483_sf {
    RN2483_SF_SF7,
    RN2483_SF_SF8,
    RN2483_SF_SF9,
    RN2483_SF_SF10,
    RN2483_SF_SF11,
    RN2483_SF_SF12
};

/**
 *  RN2483 Coding Rate
 */
enum rn2483_cr {
    RN2483_CR_4_5,
    RN2483_CR_4_6,
    RN2483_CR_4_7,
    RN2483_CR_4_8
};

/**
 *  RN2483 Bandwidth
 */
enum rn2483_bw {
    RN2483_BW_125,
    RN2483_BW_250,
    RN2483_BW_500
};

/**
 *  RN2483 GPIO pin
 */
enum rn2483_pin {
    RN2483_GPIO0 = 0,
    RN2483_GPIO1 = 1,
    RN2483_GPIO2 = 2,
    RN2483_GPIO3 = 3,
    RN2483_GPIO4 = 4,
    RN2483_GPIO5 = 5,
    RN2483_GPIO6 = 6,
    RN2483_GPIO7 = 7,
    RN2483_GPIO8 = 8,
    RN2483_GPIO9 = 9,
    RN2483_GPIO10 = 10,
    RN2483_GPIO11 = 11,
    RN2483_GPIO12 = 12,
    RN2483_GPIO13 = 13,
    RN2483_UART_CTS,
    RN2483_UART_RTS,
    RN2483_TEST0,
    RN2483_TEST1
};

/**
 *  RN2483 GPIO pin mode
 */
enum rn2483_pin_mode {
    RN2483_PIN_MODE_OUTPUT,
    RN2483_PIN_MODE_INPUT,
    RN2483_PIN_MODE_ANALOG
};

/**
 *  State of a send transaction
 */
enum rn2483_send_trans_state {
    /** This transaction is not valid */
    RN2483_SEND_TRANS_INVALID,
    /** The data had not yet been send to the radio */
    RN2483_SEND_TRANS_PENDING,
    /** Data has been sent to radio (buffer is no longer needed) */
    RN2483_SEND_TRANS_WRITTEN,
    /** Radio has transmitted data */
    RN2483_SEND_TRANS_DONE,
    /** Radio has failed to transmit data */
    RN2483_SEND_TRANS_FAILED
};

/**
 *  Type for callback function used for receiving data.
 *
 *  Returns 0 to stop receiving, a non-zero value to continue.
 */
typedef int (*rn2483_recv_callback)(struct rn2483_desc_t *inst, void *context,
                                    uint8_t *data, uint8_t length, int8_t snr,
                                    int8_t rssi);

#define RN2483_PIN_DESC_VALUE(x)    (x & 0x3FF)
#define RN2483_PIN_DESC_MODE(x)     ((x & 0x3) << 10)
#define RN2483_PIN_DESC_MODE_DIRTY  (1 << 12)
#define RN2483_PIN_DESC_VALUE_DIRTY (1 << 13)
#define RN2483_PIN_DESC_MODE_EXP    (1 << 14)

#define RN2483_VER_NUM_MAJOR_BITS   5
#define RN2483_VER_NUM_MAJOR_POS    11
#define RN2483_VER_NUM_MAJOR_MASK   (((((uint16_t)1) <<\
                                            RN2483_VER_NUM_MAJOR_BITS) - 1) <<\
                                                RN2483_VER_NUM_MAJOR_POS)
#define RN2483_VER_NUM_MINOR_BITS   5
#define RN2483_VER_NUM_MINOR_POS    6
#define RN2483_VER_NUM_MINOR_MASK   (((((uint16_t)1) <<\
                                            RN2483_VER_NUM_MINOR_BITS) - 1) <<\
                                                RN2483_VER_NUM_MINOR_POS)
#define RN2483_VER_NUM_REV_BITS     6
#define RN2483_VER_NUM_REV_POS      0
#define RN2483_VER_NUM_REV_MASK     (((((uint16_t)1) <<\
                                            RN2483_VER_NUM_REV_BITS) - 1) <<\
                                                RN2483_VER_NUM_REV_POS)
#define RN2483_VERSION(ma, mi, r) ((((uint16_t)ma << RN2483_VER_NUM_MAJOR_POS)\
                                                & RN2483_VER_NUM_MAJOR_MASK)\
                                 | (((uint16_t)mi << RN2483_VER_NUM_MINOR_POS)\
                                                & RN2483_VER_NUM_MINOR_MASK)\
                                 | (((uint16_t)r << RN2483_VER_NUM_REV_POS)\
                                                & RN2483_VER_NUM_REV_MASK))

#define RN2483_SEND_TRANSACTION_SIZE    3
#define RN2483_NUM_SEND_TRANSACTIONS    4
#define RN2483_SEND_TRANSACTION_MASK    ((1 << 3) - 1)

/**
 *  Describes all relevant RN2483 radio settings.
 */
struct rn2483_lora_settings_t {
    /** Centre frequency */
    uint32_t freq:31;
    /** Whether the I and Q streams should be inverted */
    uint32_t invert_qi:1;
    /** Preamble length */
    uint16_t preamble_length;
    /** Sync word */
    uint8_t sync_byte;
    /** Power level  */
    int8_t power;
    /** LoRa spreading factor */
    enum rn2483_sf spreading_factor:3;
    /** LoRa coding rate */
    enum rn2483_cr coding_rate:2;
    /** Bandwidth */
    enum rn2483_bw bandwidth:2;
    /** Whether a CRC should be added to the data */
    uint8_t crc:1;
};

/**
 *  Descriptor for RN2483 radio module driver instance.
 */
struct rn2483_desc_t {
    /** UART instance to which the radio is connected */
    struct sercom_uart_desc_t *uart;
    
    /** Callback function to be called when data is received */
    rn2483_recv_callback receive_callback;
    
    /** Context variable for callback */
    void *callback_context;
    
    /** Pointer to buffer of data to be send */
    const uint8_t *send_buffer;

    /** Radio configuration information */
    struct rn2483_lora_settings_t *settings;

    union {
        /** Stores the last time at which the GPIO registers where polled */
        uint32_t last_polled;
        /** Number of times the driver has attempted to reset the radio
            module */
        uint32_t reset_try_count;
    };
    
    /** Buffer used for marshaling commands and receiving responses */
    char buffer[RN2483_BUFFER_LEN];
    
    /** Cache for GPIO pin states */
    union {
        struct {
            /** The digital or analog value for this pin */
            uint16_t value:10;
            /** Mode for this pin */
            enum rn2483_pin_mode mode:2;
            /** Indicates whether the mode for this pin has been changes since
                it was last written to the module */
            uint16_t mode_dirty:1;
            /** Indicates whether the locally cached value for this pin needs to
                be updated from the module (for an input) or written to the
                module (for an output) */
            uint16_t value_dirty:1;
            /** Indicates whether the mode for this pin has been explicitly set,
                which would indicate that application code cares about this pin
                and if it is an input it should be polled automatically */
            uint16_t mode_explicit:1;
        };
        uint16_t raw;
    } pins[RN2483_NUM_PINS];
    
    /** Module firmware version*/
    uint16_t version;
    
    /** Information about send transactions */
    uint16_t send_transactions:12;
    /** Flag that indicates when new settings need to be written to the radio */
    uint16_t settings_dirty:1;
    /** Flag that indicates when a new frequency needs to be written to the
        radio */
    uint16_t frequency_dirty:1;
    /** Whether the radio should return to receiving after sending any other
        command */
    uint8_t receive:1;

    /** Length of data in send buffer */
    uint8_t send_length;
    
    /** Pointer for sending commands over multiple calls to service if UART
        buffer becomes full */
    uint8_t position;
    
    /** Pin which is the target of the current GPIO command */
    enum rn2483_pin current_pin:8;
    
    /** Current state of driver */
    enum rn2483_state state:5;
    /** Whether a new line needs to be received before the driver can
        continue */
    uint8_t waiting_for_line:1;
    /** Whether the command to be sent next has been marshaled */
    uint8_t cmd_ready:1;
};

/**
 *  Initialize instance of RN2483 radio driver.
 *
 *  @param inst The instance of the driver to be initialized
 *  @param uart The uart instance used to communicate with the radio
 *  @param settings Pointer to structure containing LoRa settings
 */
extern void init_rn2483 (struct rn2483_desc_t *inst,
                         struct sercom_uart_desc_t *uart,
                         struct rn2483_lora_settings_t *settings);

/**
 *  Service to be run in each iteration of the main loop.
 *
 *  @param inst The driver instance for which the service should be run
 */
extern void rn2483_service (struct rn2483_desc_t *inst);

/**
 *  Send data via radio.
 *
 *  @param inst Driver instance
 *  @param data The data to be sent
 *  @param length The number of bytes to be sent
 *  @param transaction_id Pointer to where transaction ID should be stored
 *
 *  @return Operation status
 */
extern enum rn2483_operation_result rn2483_send (struct rn2483_desc_t *inst,
                                                 const uint8_t *data,
                                                 uint8_t length,
                                                 uint8_t *transaction_id);

/**
 *  Get the current state of a send transaction.
 *
 *  @param inst Driver instance
 *  @param transaction_id ID of transaction for which state should be found
 *
 *  @return The state of the transaction
 */
extern enum rn2483_send_trans_state rn2483_get_send_state (
                                                    struct rn2483_desc_t *inst,
                                                    uint8_t transaction_id);

/**
 *  Clear an entry in the send transaction table.
 *
 *  @param inst Driver instance
 *  @param transaction_id ID of transaction to be cleared
 */
extern void rn2483_clear_send_transaction (struct rn2483_desc_t *inst,
                                           uint8_t transaction_id);

/**
 *  Start receiving data from radio. Radio will be put into receive mode
 *  whenever another operation is not in progress until a packet has been
 *  received.
 *
 *  @param inst Driver instance
 *  @param callback Function to be called when data is received
 *  @param context Context variable to be provided to callback
 *
 *  @return RN2483_OP_SUCCESS on success, RN2483_OP_BUSY if the radio is already
 *          receiving or RN2483_OP_BAD_STATE if the radio driver is in a failed
 *          state
 */
extern enum rn2483_operation_result rn2483_receive (struct rn2483_desc_t *inst,
                                                    rn2483_recv_callback callback,
                                                    void *context);

/**
 *  Cancel an ongoing receive operation.
 *
 *  @param inst Driver instance
 *
 *  @return RN2483_OP_SUCCESS if operation is being canceled, RN2483_OP_BAD_STATE
 *          if radio is not currently receiving or if radio driver is in a
 *          failed state
 */
enum rn2483_operation_result rn2483_receive_stop (struct rn2483_desc_t *inst);

/**
 *  Set the center frequency in an RN2483 settings structure.
 *
 *  @param s Pointer to the settings structure to be modified
 *  @param frequency New Centre frequency in hertz, from 433050000 to 434790000
 */
static inline void rn2483_settings_set_freq (struct rn2483_lora_settings_t *s,
                                             uint32_t frequency)
{
    if (frequency > RN2483_FREQ_MAX) {
        s->freq = RN2483_FREQ_MAX;
    } else if (frequency < RN2483_FREQ_MIN) {
        s->freq = RN2483_FREQ_MIN;
    } else {
        s->freq = frequency;
    }
}

/**
 *  Get the center frequency from an RN2483 settings structure.
 *
 *  @param s Pointer to the settings structure
 *
 *  @return The configured center frequency in hertz.
 */
static inline uint32_t rn2483_settings_get_freq (struct rn2483_lora_settings_t *s)
{
    return s->freq;
}

/**
 *  Set the RF related settings in an RN2483 settings structure.
 *
 *  @param s Pointer to the settings structure to be modified
 *  @param power Power level in dBm, from -3 to 14
 *  @param spreading_factor LoRa spreading factor
 *  @param coding_rate LoRa coding rate
 *  @param bandwidth Bandwidth
 */
static inline void rn2483_settings_set_rf (struct rn2483_lora_settings_t *s,
                                           int8_t power,
                                           enum rn2483_sf spreading_factor,
                                           enum rn2483_cr coding_rate,
                                           enum rn2483_bw bandwidth)
{
    if (power > RN2483_PWR_MAX) {
        s->power = RN2483_PWR_MAX;
    } else if (power < RN2483_PWR_MIN) {
        s->power = RN2483_PWR_MIN;
    } else {
        s->power = power;
    }
    s->spreading_factor = spreading_factor;
    s->coding_rate = coding_rate;
    s->bandwidth = bandwidth;
}

/**
 *  Get the RF related settings from an RN2483 settings structure.
 *
 *  @param s Pointer to the settings structure
 *  @param power Pointer to where power level in dBm will be stored
 *  @param spreading_factor Pointer to where LoRa spreading factor will be
 *                          stored
 *  @param coding_rate Pointer to where LoRa coding rate will be stored
 *  @param bandwidth Pointer to where bandwidth will be stored
 */
static inline void rn2483_settings_get_rf (struct rn2483_lora_settings_t *s,
                                           int8_t *power,
                                           enum rn2483_sf *spreading_factor,
                                           enum rn2483_cr *coding_rate,
                                           enum rn2483_bw *bandwidth)
{
    *power = s->power;
    *spreading_factor = s->spreading_factor;
    *coding_rate = s->coding_rate;
    *bandwidth = s->bandwidth;
}

/**
 *  Set the synchronization settings in an RN2483 settings structure.
 *
 *  @param s Pointer to the settings structure to be modified
 *  @param send_crc Whether a CRC should be added to the data
 *  @param invert_qi Whether the I and Q streams should be inverted
 *  @param sync_byte Sync word
 *  @param preamble_length Length of preamble in symbols
 */
static inline void rn2483_settings_set_sync (struct rn2483_lora_settings_t *s,
                                             uint8_t send_crc,
                                             uint8_t invert_qi,
                                             uint8_t sync_byte,
                                             uint16_t preamble_length)
{
    s->crc = send_crc;
    s->invert_qi = invert_qi;
    s->sync_byte = sync_byte;
    s->preamble_length = preamble_length;
}

/**
 *  Get the synchronization settings from an RN2483 settings structure.
 *
 *  @param s Pointer to the settings structure
 *  @param send_crc Pointer to where whether a CRC should be added to the data
 *                  will be stored
 *  @param invert_qi Pointer to where whether the I and Q streams should be
 *                   inverted will be stored
 *  @param sync_byte Pointer to where the sync word will be stored
 *  @param preamble_length Pointer to where the length of preamble in symbols
 *                         will be stored
 */
static inline void rn2483_settings_get_sync (struct rn2483_lora_settings_t *s,
                                             uint8_t *send_crc,
                                             uint8_t *invert_qi,
                                             uint8_t *sync_byte,
                                             uint16_t *preamble_length)
{
    *send_crc = s->crc;
    *invert_qi = s->invert_qi;
    *sync_byte = s->sync_byte;
    *preamble_length = s->preamble_length;
}

/**
 *  Indicates to the radio driver that the settings structure for a radio has
 *  changed and that all of the radio settings (including the frequency) need to
 *  be sent to the radio again.
 *
 *  @note The settings will be updated after any ongoing transmission or
 *        reception that is already in the data stage has completed but before
 *        any new transmission or reception starts.
 *
 *  @param inst The instance for which the settings need to be updated.
 */
extern void rn2483_update_settings(struct rn2483_desc_t *inst);

/**
 *  Indicates to the radio driver that the frequency in the settings structure
 *  for a radio has changed and that it needs to be sent to the radio again.
 *
 *  @note The frequency will be updated after any ongoing transmission or
 *        reception that is already in the data stage has completed but before
 *        any new transmission or reception starts.
 *
 *  @param inst The instance for which the frequency needs to be updated.
 */
extern void rn2483_update_frequency_settings(struct rn2483_desc_t *inst);

/**
 *  Poll the radio modules for updates on any pins which have been set as
 *  inputs.
 *
 *  @param inst Driver instance
 */
extern void rn2483_poll_gpio(struct rn2483_desc_t *inst);

/**
 *  Poll the radio modules for updates on a specific input pin.
 *
 *  @param inst Driver instance
 *  @param pin The pin which should be polled
 */
static inline void rn2483_poll_gpio_pin(struct rn2483_desc_t *inst,
                                        enum rn2483_pin pin)
{
    // Set value dirty
    inst->pins[pin].value_dirty = 1;
    // Run the service to start the update right away if possible
    rn2483_service(inst);
}

/**
 *  Check if the radio module is in the process of being polled for updates to
 *  input pin values.
 *
 *  @param inst Driver instance
 *
 *  @return 1 if polling is in progress, 0 otherwise
 */
extern uint8_t rn2483_poll_gpio_in_progress(struct rn2483_desc_t *inst);

/**
 *  Check if the radio module is in the process of being polled for an update to
 *  the value for a specific input pin.
 *
 *  @param inst Driver instance
 *  @param pin The pin which should be checked
 *
 *  @return 1 if polling is in progress, 0 otherwise
 */
static inline uint8_t rn2483_poll_gpio_pin_in_progress(
                                                    struct rn2483_desc_t *inst,
                                                    enum rn2483_pin pin)
{
    return inst->pins[pin].value_dirty;
}

/**
 *  Configure the mode for a pin.
 *
 *  @param inst Driver instance
 *  @param pin The pin for which the mode should be set
 *  @param mode 0 for output, any other value for input
 *
 *  @return 0 if the mode was set successfully, a non-zero value otherwise
 */
extern uint8_t rn2483_set_pin_mode(struct rn2483_desc_t *inst,
                                   enum rn2483_pin pin,
                                   enum rn2483_pin_mode mode);

/**
 *  Get the current mode of a pin.
 *
 *  @param inst Driver instance
 *  @param pin The pin for which the mode should be found
 *
 *  @return The current mode of the pin
 */
static inline enum rn2483_pin_mode rn2483_get_pin_mode(
                                                    struct rn2483_desc_t *inst,
                                                    enum rn2483_pin pin)
{
    return inst->pins[pin].mode;
}

/**
 *  Get the value from a pin which is configured as an input.
 *
 *  @param inst Driver instance
 *  @param pin The pin for which the value should be returned
 *
 *  @return The value of the pin, 0 for logic low, 1 for logic high
 */
static inline uint8_t rn2483_get_input(struct rn2483_desc_t *inst,
                                       enum rn2483_pin pin)
{
    return (uint8_t)!!inst->pins[pin].value;
}

/**
 *  Set the value for a pin which is configured as an output.
 *
 *  @param inst Driver instance
 *  @param pin The pin for which the value should be set
 *  @param value New value for the pin, 0 for logic low, logic high otherwise
 */
extern void rn2483_set_output(struct rn2483_desc_t *inst, enum rn2483_pin pin,
                              uint8_t value);

/**
 *  Toggle the value for a pin which is configured as an output.
 *
 *  @param inst Driver instance
 *  @param pin The pin for which the value should be toggled
 */
extern void rn2483_toggle_output(struct rn2483_desc_t *inst,
                                 enum rn2483_pin pin);

/**
 *  Get the value from a pin which is configured as an analog input.
 *
 *  @param inst Driver instance
 *  @param pin The pin for which the value should be returned
 *
 *  @return A value ranging from 0 to 1023 representing the analog value of the
 *          pin from 0v to VDD or 0xFFFF if the pin is not configured as an
 *          analog input
 */
static inline uint16_t rn2483_get_analog(struct rn2483_desc_t *inst,
                                         enum rn2483_pin pin)
{
    if (inst->pins[pin].mode == RN2483_PIN_MODE_ANALOG) {
        return inst->pins[pin].value;
    } else {
        return 0xFFFF;
    }
}

#endif /* rn2483_h */

