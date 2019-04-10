/**
 * sd.c
 * Containing function implementations for communicating with the SD Card
 *
 *
 */

#include "config.h"
#include "sd.h"
#include "sercom_spi.h"

/**
 * To initialize the SD card, set CS high for 74 clock cycles to put the SD
 * card into SPI mode. Then set the pin to low and send CMD0 (set idle state).
 */
uint8_t init()
{
    uint8_t retCode = 0;
    uint8_t *transactionId;
    uint8_t *sendBuffer;
    uint8_t *receiveBuffer;
    uint16_t sendBufferLength = 1;
    uint16_t receiveBufferLength = 1;
    while (retCode != SD_CARD_ERROR_CMD0) {
        for (uint8_t i = 0; i < 10; i++) {
            retCode = 1;
            while (retCode == 1) {
                *sendBuffer = 0xFF;
                retCode = sercom_spi_start(spi_g, transactionId, SD_BAUDRATE,
                        0xFF, SD_CS_PIN_MASK, sendBuffer, sendBufferLength,
                        receiveBuffer, receiveBufferLength);
            }
        }
        sendBuffer = CMD0;
        retCode = sercom_spi_start(spi_g, transactionId, SD_BAUDRATE,
                SD_CS_PIN_GROUP, SD_CS_PIN_MASK, sendBuffer, sendBufferLength,
                receiveBuffer, receiveBufferLength);
    }
    while (retCode != SD_CARD_ERROR_CMD8) {
        sendBuffer = CMD8;
        retCode = sercom_spi_start(spi_g, transactionId, SD_BAUDRATE,
                SD_CS_PIN_GROUP, SD_CS_PIN_MASK, sendBuffer, sendBufferLength,
                receiveBuffer, receiveBufferLength);
    }
}

uint8_t write_block(uint32_t blockNumber, const uint8_t* src)
{
// write one block's worth of data to the SD card where src points to the
// array of bytes to be written
}

uint8_t read_block(uint32_t blockNumber)
{
// read one block's worth of data to the SD card (may not be necessary)
}
