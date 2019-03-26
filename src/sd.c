/**
 * sd.c
 * Containing function implementations for communicating with the SD Card
 *
 *
 */

#include "sd.h"

/**
 * To initialize the SD card, set CS high for 74 clock cycles to put the SD
 * card into SPI mode. Then set the pin to low and send CMD0 (set idle state).
 *
 */
uint8_t init()
{
    // Must set MOSI high for>=74 clock cycles with CS high to tell SD card to
    // use SPI mode
    chip_select_high();
    for (uint8_t i = 0; i < 10; i++) {
        // Send over spi bus 0xFF
        // send cs pin group FF for this transaction only otherwise send
        // value defined in config.h
    }
    chip_select_low()
    // Send command 0
    // Check if response is 0b00000001 (no errors), MOSI still set high
    // Response must be received in 16 clock cycles of the SD card
    // If response not that, re-reset the card
    // If response good, send CMD8
    // Check the first 8 bit received to see if successful... next 32 bits
    // are just SD information so ignore those
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
