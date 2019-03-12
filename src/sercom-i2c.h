/**
 * @file sercom-i2c.h
 * @desc SERCOM I2C master mode driver with DMA support.
 * @author Samuel Dewan
 * @date 2019-01-25
 * Last Author:
 * Last Edited On:
 */

#ifndef sercom_i2c_h
#define sercom_i2c_h

#include "global.h"

#include "transaction-queue.h"

#define I2C_ADDRESS_MASK 0xFE

#define SERCOM_I2C_TRANSACTION_QUEUE_LENGTH 12

/**
 *  I2C speed mode
 */
enum i2c_mode {
    /** 100 kHz */
    I2C_MODE_STANDARD,
    /** 400 kHz */
    I2C_MODE_FAST,
    /** 1 MHz MHz */
    I2C_MODE_FAST_PLUS,
    /** 3.4 MHz MHz */
    I2C_MODE_HIGH_SPEED
};

 /**
  * I2C Transaction type
  */
enum i2c_transaction_type {
    /** Generic Transaction */
    I2C_TRANSACTION_GENERIC,
    //  Send out_length bytes from out_buffer
    //  Receive in_lenth bytes to in_buffer
    
    /** Register Write */
    I2C_TRANSACTION_REG_WRITE,
    //  Send register address byte from register_address
    //  Send data_length bytes from buffer
    
    /** Register Read */
    I2C_TRANSACTION_REG_READ,
    //  Send register address byte from register_address
    //  Receive data_length bytes to buffer
    
    /** Bus Scan */
    I2C_TRANSACTION_SCAN
    //  Send every possible devices address, If the address is acked, set a bit
    //  in the 128 bit field made up from scan[0] and scan[1];
};

/**
 *  I2C transaction state
 */
enum i2c_transaction_state {
    /** Initial state, transaction is not yet started */
    I2C_STATE_PENDING,
    /** Sending device address for register read or write transaction */
    I2C_STATE_REG_ADDR,
    /** Transmitting data to slave */
    I2C_STATE_TX,
    /** Waiting for the bus to become idle before starting recieve stage */
    I2C_STATE_WAIT_FOR_RX,
    /** Receiving data from slave */
    I2C_STATE_RX,
    /** Waiting for the bus to become idle before ending transaction */
    I2C_STATE_WAIT_FOR_DONE,
    /** Transaction finished */
    I2C_STATE_DONE,
    /** Error occured on I2C bus, transaction aborted */
    I2C_STATE_BUS_ERROR,
    /** Lost arbitration on I2C bus, transaction aborted */
    I2C_STATE_ARBITRATION_LOST,
    /** The slave did not ACK it's address or a byte which was sent to it,
     transaction aborted */
    I2C_STATE_SLAVE_NACK
};

/**
 *  State for an I2C transaction.
 */
struct sercom_i2c_transaction_t {
    /** Description of data to be send or received. */
    union {
        /** Data for a generic transaction with arbitrary length transmit and
         *  recieve stages */
        struct {
            /** The buffer from which data is sent. */
            uint8_t *out_buffer;
            /** The buffer into which recieved data is placed. */
            uint8_t *in_buffer;
            /** The number of bytes to be sent. */
            uint16_t out_length;
            /** The number of bytes to be recieved. */
            uint16_t in_length;
            
            /** The number of bytes which have been sent. */
            uint16_t bytes_out;
            /** The number of bytes which have been recieved. */
            uint16_t bytes_in;
        } generic;
        
        /** Data for a transaction to or from a specific device register. */
        struct {
            /** The buffer from/to which data is sent or received. */
            uint8_t *buffer;
            
            /** The number of bytes to be sent or recieved. */
            uint16_t data_length;
            /** The number of bytes which have been sent or received. */
            uint16_t position;
            
            /** The address of the device register to be written/read. */
            uint8_t register_address;
            
            uint8_t RESERVED[7];
        } reg;
        
        /** Data for a bus scan operation. */
        struct {
            uint64_t results[2];
        } scan;
    };
    
    /** The address for the peripheral. */
    uint8_t dev_address;
    
    /** Whether DMA should be used for the transmit stage of this transaction */
    uint8_t dma_out:1;
    /** Whether DMA should be used for the recieve stage of this transaction */
    uint8_t dma_in:1;
    
    /** The type of this transaction */
    enum i2c_transaction_type type:2;
    // WARNING: Using an enum as a bit field may not be supported on compilers
    //          other than GCC (but I'm doing it because GCC gives nice warnings
    //          if the field is too narrow)
    
    /** Current state of this transaction */
    enum i2c_transaction_state state:4;
};

/**
 *  State for a SERCOM I2C driver instance
 */
struct sercom_i2c_desc_t {
    /** Registers for the SERCOM hardware of this I2C instance. */
    Sercom *sercom;
    
    /** Memory for transaction queue. */
    struct transaction_t transactions[SERCOM_I2C_TRANSACTION_QUEUE_LENGTH];
    /** Memory for transaction state information. */
    struct sercom_i2c_transaction_t states[SERCOM_I2C_TRANSACTION_QUEUE_LENGTH];
    /** Queue of I2C transactions. */
    struct transaction_queue_t queue;
    
    /** DMA descriptor used as second descriptor in DMA transactions. */
    DmacDescriptor dma_desc;
    
    /** The instance number of the SERCOM hardware of this I2C instance. */
    uint8_t sercom_instnum;
    
    /** Index of the DMA channel used. */
    uint8_t dma_chan:4;
    /** Flag which is set if DMA should be used. */
    uint8_t use_dma:1;
    
    /** Flag used to indicate that the next transaction is staled waiting for
        the bus to become free */
    uint8_t wait_for_idle:1;
};


/**
 *  Initilize a SERCOM instance for use as an I2C master.
 *
 *  @param descriptor The descriptor to be populated for this I2C instance.
 *  @param sercom Pointer to the SERCOM instance's registers.
 *  @param core_freq The frequency of the core clock for the SERCOM instance.
 *  @param core_clock_mask The mask for the generator to be used for the SERCOM
 *                         core clock;
 *  @param mode The speed mode for the I2C interface
 *  @param dma_channel The DMA channel to be used or a negative value for
 *                     interupt driven transactions.
 */
extern void init_sercom_i2c(struct sercom_i2c_desc_t *descriptor,
                            Sercom *sercom, uint32_t core_freq,
                            uint32_t core_clock_mask, enum i2c_mode mode,
                            int8_t dma_channel);

/**
 *  Send and recieve data on the I2C bus.
 *
 *  @param i2c_inst The I2C instance to use.
 *  @param trans_id The identifer for the created transaction will be
 *                  placed here.
 *  @param dev_address The address of the peripheral to comunicate with.
 *  @param out_buffer The buffer from which data should be sent.
 *  @param out_length The number of bytes to be sent.
 *  @param in_buffer The buffer where recieved data will be placed.
 *  @param in_length The number of bytes to be recieved.
 *
 *  @return 0 if transaction is successfuly queued.
 */
extern uint8_t sercom_i2c_start_generic(struct sercom_i2c_desc_t *i2c_inst,
                                        uint8_t *trans_id, uint8_t dev_address,
                                        uint8_t *out_buffer,
                                        uint16_t out_length, uint8_t *in_buffer,
                                        uint16_t in_length);

/**
 *  Write a register on a peripheral on the I2C bus.
 *
 *  @param i2c_inst The I2C instance to use.
 *  @param trans_id The identifer for the created transaction will be
 *                  placed here.
 *  @param dev_address The address of the peripheral to comunicate with.
 *  @param register_address The address of the register to be writen
 *  @param data The buffer from which data should be sent.
 *  @param length The number of bytes to be sent.
 *
 *  @return 0 if transaction is successfuly queued.
 */
extern uint8_t sercom_i2c_start_reg_write(struct sercom_i2c_desc_t *i2c_inst,
                                          uint8_t *trans_id,
                                          uint8_t dev_address,
                                          uint8_t register_address,
                                          uint8_t *data, uint16_t length);

/**
 *  Read a register on a peripheral on the I2C bus.
 *
 *  @param i2c_inst The I2C instance to use.
 *  @param trans_id The identifer for the created transaction will be
 *                  placed here.
 *  @param dev_address The address of the peripheral to comunicate with.
 *  @param register_address The address of the register to be read
 *  @param data The buffer where recieved data will be placed.
 *  @param length The number of bytes to be recieved.
 *
 *  @return 0 if transaction is successfuly queued.
 */
extern uint8_t sercom_i2c_start_reg_read(struct sercom_i2c_desc_t *i2c_inst,
                                         uint8_t *trans_id, uint8_t dev_address,
                                         uint8_t register_address,
                                         uint8_t *data, uint16_t length);

/**
 *  Scan to determine all of the attached addresses on the I2C bus.
 *
 *  @param i2c_inst The I2C instance to use.
 *  @param trans_id The identifer for the created transaction will be
 *                  placed here.
 *
 *  @return 0 if transaction is successfuly queued.
 */
extern uint8_t sercom_i2c_start_scan(struct sercom_i2c_desc_t *i2c_inst,
                                     uint8_t *trans_id);

/**
 *  Check if an I2C transaction in the queue is complete.
 *
 *  @param i2c_inst The I2C instance from which the queue should be used.
 *  @param trans_id The ID of the transaction to check.
 *
 *  @return A non-zero value if the tranaction has finished, 0 otherwise.
 */
extern uint8_t sercom_i2c_transaction_done(struct sercom_i2c_desc_t *i2c_inst,
                                           uint8_t trans_id);

/**
 *  Get the current state of an I2C transaction.
 *
 *  @param i2c_inst The I2C instance from which the queue should be used.
 *  @param trans_id The ID of the transaction to check.
 *
 *  @return The transaction's state.
 */
extern enum i2c_transaction_state sercom_i2c_transaction_state(
                                            struct sercom_i2c_desc_t *i2c_inst,
                                            uint8_t trans_id);

/**
 *  Remove and I2C transaction from the queue.
 *
 *  @param i2c_inst The I2C instance from which the queue should be used.
 *  @param trans_id The ID of the transaction to clear.
 *
 *  @return 0 if transaction was successfuly cleared.
 */
extern uint8_t sercom_i2c_clear_transaction(struct sercom_i2c_desc_t *i2c_inst,
                                            uint8_t trans_id);

/**
 *  Check if a device was found in a scan.
 *
 *  @param i2c_inst The I2C instance from which the queue should be used.
 *  @param trans_id The ID of a complete, uncleared scan transaction.
 *  @param address The address to check for.
 *
 *  @return 0 If the device is not avaliable, a possitive number otherwise
 */
extern uint8_t sercom_i2c_device_avaliable(struct sercom_i2c_desc_t *i2c_inst,
                                           uint8_t trans_id, uint8_t address);

/**
 *  Service run in each iteration of the main loop.
 *
 *  @param i2c_inst The I2C instance for which the service should be run.
 */
extern void sercom_i2c_service (struct sercom_i2c_desc_t *i2c_inst);

#endif /* sercom_i2c_h */
