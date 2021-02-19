/**
 * @file mcp2518fd.c
 * @desc Registers for Microchip MCP2518FD SPI CAN Controller
 * @author Grant Wilson 
 * @date 2021-01-31
 * Last Author: Grant Wilson 
 * Last Edited On: 2021-01-31
 */

union mcp2518fd_REG
{
    struct 
    {
        uint16_t CRC;
        uint16_t RESERVED:6;
        // ...
    };
    uint32_t ref;

    // make sure it adds up to the total, 
    
};
