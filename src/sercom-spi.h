/**
 * @file sercom-spi.h
 * @desc SERCOM SPI mode driver which allows interupt or DMA driven transfers.
 * @author Samuel Dewan
 * @date 2019-01-02
 * Last Author:
 * Last Edited On:
 */

#ifndef sercom_spi_h
#define sercom_spi_h

#include "global.h"

#include "transaction-queue.h"

#define SERCOM_SPI_TRANSACTION_QUEUE_LENGTH 16


/**
 *  State for an SPI transaction.
 */
struct sercom_spi_transaction_t {
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
    
    /** The synchronous clock frequency for this transaction.  */
    uint32_t baudrate;
    
    /** The mask for the chip select pin of the peripheral. */
    uint32_t cs_pin_mask;
    /** The group index of the chip select pin for the peripheral. */
    uint8_t cs_pin_group:2;
    
    /** Flag set if the receive stage has been initialized. */
    uint8_t rx_started:1;
};

/**
 *  State for a SERCOM SPI driver instance
 */
struct sercom_spi_desc_t {
    /** Registers for the SERCOM hardware of this SPI instance. */
    Sercom *sercom;
    
    /** Frequency of the SERCOM code clock, used to calculate baud rates. */
    uint32_t core_frequency;
    
    /** Memory for transaction queue. */
    struct transaction_t transactions[SERCOM_SPI_TRANSACTION_QUEUE_LENGTH];
    /** Memory for transaction state information. */
    struct sercom_spi_transaction_t states[SERCOM_SPI_TRANSACTION_QUEUE_LENGTH];
    /** Queue of SPI transactions. */
    struct transaction_queue_t queue;

    /** The instance number of the SERCOM hardware of this SPI instance. */
    uint8_t sercom_instnum;
    
    /** Index of the DMA channel used for transmitting. */
    uint8_t tx_dma_chan:4;
    /** Index of the DMA channel used for recieving. */
    uint8_t rx_dma_chan:4;
    /** Flag which is set if DMA should be used for transmitting. */
    uint8_t tx_use_dma:1;
    /** Flag which is set if DMA should be used for recieving. */
    uint8_t rx_use_dma:1;
    /** Flag used to unsure that the service function is not executed in an
        interrupt while it is already being run in the main thread */
    uint8_t service_lock:1;
};


/**
 *  Initilize a SERCOM instance for use as an SPI master.
 *
 *  @param descriptor The descriptor to be populated for this SPI instance.
 *  @param sercom Pointer to the SERCOM instance's registers.
 *  @param core_freq The frequency of the core clock for the SERCOM instance.
 *  @param core_clock_mask The mask for the generator to be used for the SERCOM
 *                         core clock;
 *  @param tx_dma_channel The DMA channel to be used for transmition or a
 *                        negative value for interupt driven transmition.
 *  @param rx_dma_channel The DMA channel to be used for reception or a
 *                        negative value for interupt driven reception.
 */
extern void init_sercom_spi(struct sercom_spi_desc_t *descriptor,
                            Sercom *sercom, uint32_t core_freq,
                            uint32_t core_clock_mask, int8_t tx_dma_channel,
                            int8_t rx_dma_channel);

/**
 *  Send and recieve data on the SPI bus.
 *
 *  @param spi_inst The SPI instance to use.
 *  @param trans_id The identifer for the created transaction will be
 *                  placed here.
 *  @param baudrate The baudrate to be used for the transaction.
 *  @param cs_pin_group The group index of the chip select pin of the
 *                      peripheral.
 *  @param cs_pin_mask The mask for the chip select pin of the peripheral.
 *  @param out_buffer The buffer from which data should be sent.
 *  @param out_length The number of bytes to be sent.
 *  @param in_buffer The buffer where recieved data will be placed.
 *  @param in_length The number of bytes to be recieved.
 *
 *  @return 0 if transaction is successfuly queued.
 */
extern uint8_t sercom_spi_start(struct sercom_spi_desc_t *spi_inst,
                                uint8_t *trans_id, uint32_t baudrate,
                                uint8_t cs_pin_group, uint32_t cs_pin_mask,
                                uint8_t *out_buffer, uint16_t out_length,
                                uint8_t * in_buffer, uint16_t in_length);

/**
 *  Check if an SPI transaction in the queue is complete.
 *
 *  @param spi_inst The SPI instance from which the queue should be used.
 *  @param trans_id The ID of the transaction to check.
 *
 *  @return A non-zero value if the tranaction has finished, 0 otherwise.
 */
extern uint8_t sercom_spi_transaction_done(struct sercom_spi_desc_t *spi_inst,
                                           uint8_t trans_id);

/**
 *  Remove and SPI transaction from the queue.
 *
 *  @param spi_inst The SPI instance from which the queue should be used.
 *  @param trans_id The ID of the transaction to clear.
 *
 *  @return 0 if transaction was successfuly cleared.
 */
extern uint8_t sercom_spi_clear_transaction(struct sercom_spi_desc_t *spi_inst,
                                            uint8_t trans_id);

#endif /* sercom_spi_h */
