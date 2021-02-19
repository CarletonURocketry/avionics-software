/**
 * @file mcp2518fd.c
 * @desc Driver for Microchip MCP2518FD SPI CAN Controller
 * @author Grant Wilson 
 * @date 2021-01-31
 * Last Author: Grant Wilson 
 * Last Edited On: 2021-01-31
 */

#include "mcp2518fd.h"
#include "mcp2518fd-registers.h"


/**
 * @param descriptor The pointer to where the descriptor will be stored
 */
void init_mcp2518fd(struct mcp2518fd_desc *descriptor, struct sercom_spi_desc_t *spi_inst,
                    uint32_t cs_pin_mask, uint8_t cs_pin_group)
{
    /* Store SPI settings */ 
    descriptor->spi_inst = spi_inst;
    descriptor->cs_pin_group = cs_pin_group;
    descriptor->cs_pin_mask = cs_pin_mask;
    
    mcp2518fd_service(descriptor);
}

void mcp2518fd_service(struct mcp2518fd_desc *inst)
{

}