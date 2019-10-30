/**
 * @file sd.c
 * @brief Containing function implementations for communicating with the SD
 *          Card
 *
 * Sources for implementation:
 *  -- https://electronics.stackexchange.com/questions/77417
 *  -- https://openlabpro.com/guide/interfacing-microcontrollers-with-sd-card/
 *  -- https://nerdclub-uk.blogspot.com/2012/11/how-spi-works-with-sd-card.html
 *  -- https://openlabpro.com/guide/raw-sd-readwrite-using-pic-18f4550/
 *
 * Supposedly, the proper way to initalize a card over SPI is to:
 *      1. Set the clock speed to 400kHz or less
 *      2. Hold the CS line low and send 80 clock pulses (with command 0xFF)
 *      3. Send the "soft reset" command CMD0
 *      4. Wait for the card to respond "ok" with the value 0x01
 *      5. Send in the "initialize card" command CMD1
 *      6. Repeat sending CMD1 until card responds with "ok" value 0x00
 *      7. Set sector size using CMD16 with parameter 512
 *      8. Turn off CRC requirement by sending CMD59
 *      9. Next time the card responds with "ok" value it is ready
 *      10. Ramp up clock speed back to normal.
 *  But this might be outdated/not work 100% of the time. Also, newer cards can
 *  usually deal with MHz clock speeds just fine so down-clocking isn't
 *  necessary. The stackexchange question had the most recently written process
 *  so that is what was followed in reality.
 *
 *  Supposedly, according to the spec, all SD/SDHC/SDXC cards should respond to
 *  CMD55/ACMD41 to initalize and CMD1 should only be used as a fallback.
 *
 *  We should also make sure CS is low at least before and after each CMD is
 *  sent since SD cards are selfish and may assume it's the only SPI device
 *  selected all the time?
 */

#include "config.h"
#include "sd.h"

/**
 * sd_send_cmd()
 * @brief Sets up the buffer for sending commands to the SD card and then
 * sends the data.
 *
 * @param cmd The single byte command to send to the SD card.
 * @param arg The four byte argument to send to the SD card.
 * @param crc The crc checksum to send (necessary during initialization,
 *              otherwise optional).
 * @param initFlag A simple flag which will tell the program whether or not to
 *              set the CS pin (the CS pin must remain low when setting card
 *              into SPI mode).
 *
 * @return The byte that the SD card sent in response.
 */
static inline uint8_t sd_send_cmd(uint8_t cmd, uint32_t arg, uint8_t crc,
                                  uint8_t initFlag)
{
    uint8_t transactionId;
    uint8_t receiveBuffer;
    uint8_t sendBuffer[7];
    uint16_t sendBufferLength = sizeof(sendBuffer);
    uint16_t receiveBufferLength = sizeof(receiveBuffer);

    sendBuffer[0] = cmd | 0x40; // Every command byte sent must have bit 6 set
    sendBuffer[1] = arg >> 24;
    sendBuffer[2] = arg >> 16;
    sendBuffer[3] = arg >> 8;
    sendBuffer[4] = arg;
    sendBuffer[5] = crc; // Usually 0x00 during normal (i.e. not init) operation
    // Provides 8 clock cycles necessary to allow the card to complete the
    // operation according to the SD Card spec
    sendBuffer[6] = 0xFF;

    if (initFlag) {
        sercom_spi_start(&spi_g, &transactionId, SD_BAUDRATE, 0xFF,
                SD_CS_PIN_MASK, sendBuffer, sendBufferLength, &receiveBuffer,
                receiveBufferLength);
        while (!sercom_spi_transaction_done(&spi_g, transactionId));
    }
    else {
        sercom_spi_start(&spi_g, &transactionId, SD_BAUDRATE, SD_CS_PIN_GROUP,
                SD_CS_PIN_MASK, sendBuffer, sendBufferLength, &receiveBuffer,
                receiveBufferLength);
        while (!sercom_spi_transaction_done(&spi_g, transactionId));
    }
    return receiveBuffer;
}

/**
 * init()
 * @brief Initializes the SD card into SPI mode.
 *
 * @return Either 0 (success) or 1 (failure)
 */
uint8_t init_sd_card(void)
{
    uint8_t oldCardFlag = 0;
    uint8_t softResetCount = 0;
    uint8_t response = 0xFF;

    // Put SD card in SPI mode
    for (uint8_t i = 0; i < 10; i++) {
        sd_send_cmd(0xFF, 0xFFFFFFFF, 0xFF, 1);
    }
    // Repeat until soft reset successful or a reasonable number of times
    // since it is possible to not get a valid response here but have the next
    // steps work just fine
    while (response != 0x01 || softResetCount < 20) {
        response = sd_send_cmd(CMD0, 0x00000000, 0x95, 0);
        softResetCount++;
    }

    response = sd_send_cmd(CMD8, 0x000001AA, 0x87, 0);

    // Apparently most cards require this to be repeated at least once...
    // ...so repeat it 3 times to be really sure since it is also apprently
    // common to have to wait a few hundred ms if device is just after power on.
    for (uint8_t i = 0; i < 3; i++) {
        if (oldCardFlag)
            response = sd_send_cmd(CMD1, 0x00000000, 0xF9, 0);
        else {
            response = sd_send_cmd(CMD55, 0x00000000, 0x65, 0);
            // If this response is given, we have an old card that must use CMD1
            if (response == 0x05) {
                response = sd_send_cmd(CMD1, 0x00000000, 0xF9, 0);
                oldCardFlag = 1;
            }
            else {
                response = sd_send_cmd(ACMD41, 0x40000000, 0x77, 0);
                // If this response is given, we need to send CMD55 again
                if (response == 0x01) {
                    i--;
                    continue;
                }
            }
        }
    }
    // Response should be 0x00 indicating the SD card is ready, if not,
    // initialization has failed
    if (response == 0x00) {
        // Set the R/W block size to 512 bytes with CMD16
        // Try 3 times, if success return 0 immediately
        for (uint8_t i = 0; i < 3; i++) {
            response = sd_send_cmd(CMD16, SD_BLOCKSIZE, 0xFF, 0);
            if (response == 0x00)
                return 0;
        }
    }
    // In all error cases, return 1
    return 1;
}

/**
 * write_block()
 * @brief Write a single block to the SD card.
 *
 * @param blockAddr The address of the block to write to.
 * @param src A pointer to the data which will be written to the card.
 *
 * @return 0 on success, 1 on error.
 */
uint8_t write_block(uint32_t blockAddr, uint8_t* src)
{
    /* NOTE: This may be difficult to do because the specification and
     * many of the resources I found required that we wait for responses
     * and for things to complete on the SD card. Depending on how the SPI
     * driver schedules things, writes could fail because the SD card
     * expects to have exclusive control of the SPI bus when it is being
     * commanded to do something.
     */
    uint8_t transactionId;
    uint8_t response = 0xFF;
    uint16_t sendBufferLength = SD_BLOCKSIZE;
    uint16_t responseLength = sizeof(response);
    uint8_t dummyByte = 0xFF;
    uint8_t writeBeginByte = 0xFE;

    // Send CMD24 (the single block write command)
    response = sd_send_cmd(CMD24, blockAddr, 0x00, 0);
    // If not immediately successful, exit because we can't afford keep
    // trying for a single write command.
    if (response != 0x00) {
        return 1;
    }

    // Send one dummy byte before sending the data.
    sercom_spi_start(&spi_g, &transactionId, SD_BAUDRATE, SD_CS_PIN_GROUP,
                SD_CS_PIN_MASK, &dummyByte, 1, &response,
                responseLength);
    while (!sercom_spi_transaction_done(&spi_g, transactionId));

    // First byte sent MUST be 0xFE to enable writing of a single block.
    sercom_spi_start(&spi_g, &transactionId, SD_BAUDRATE, SD_CS_PIN_GROUP,
                SD_CS_PIN_MASK, &writeBeginByte, 1, &response,
                responseLength);
    while (!sercom_spi_transaction_done(&spi_g, transactionId));

    // Write the block.
    sercom_spi_start(&spi_g, &transactionId, SD_BAUDRATE, SD_CS_PIN_GROUP,
                SD_CS_PIN_MASK, src, sendBufferLength, &response,
                responseLength);
    while (!sercom_spi_transaction_done(&spi_g, transactionId));

    // Send two more dummy bytes after sending the data.
    for (uint8_t i = 0; i < 2; i++) {
        sercom_spi_start(&spi_g, &transactionId, SD_BAUDRATE, SD_CS_PIN_GROUP,
                    SD_CS_PIN_MASK, &dummyByte, 1, &response,
                    responseLength);
        while (!sercom_spi_transaction_done(&spi_g, transactionId));
    }

    // It is (apparently) mandatory to send CMD13 after every block write
    // presumably because this returns the status of the card.
    response = sd_send_cmd(CMD13, 0x00000000, 0xFF, 0);

    // Check if write was successful.
    if (response == 0x00)
        return 0;
    else
        return 1;
}