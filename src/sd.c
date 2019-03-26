/**
 * sd.c
 * Containing function implementations for communicating with the SD Card
 *
 *
 */

#include "sd.h"

uint8_t init()
{
    // Must send >=74 clock cycles with CS high to tell SD card to use SPI mode
    chip_select_high();
    for (uint8_t i = 0; i < 10; i++) {
        // Send over spi bus 0xFF
    }
    chip_select_low()

}

void chip_select_high(void)
{
// set CS pin high
}

void chip_select_low(void)
{
// set CS pin low
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
