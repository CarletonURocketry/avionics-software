/**
 * @file mcp2518fd.c
 * @desc Driver for Microchip MCP2518FD SPI CAN Controller
 * @author Grant Wilson 
 * @date 2021-01-31
 * Last Author: Grant Wilson 
 * Last Edited On: 2021-01-31
 */

#include "sercom-spi.h"
// #include "global.h"

struct mcp2518fd_desc {
    /** SPI instance used to communicate with the device */ 
    struct sercom_spi_desc_t *spi_inst;
    /** The pin mask for the GPIO pins for the SPI chip select */
    uint32_t cs_pin_mask;
    /** The GPIO pin group for SPI chip select */
    uint8_t cs_pin_group;
};