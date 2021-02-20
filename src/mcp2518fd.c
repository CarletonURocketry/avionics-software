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
    if (sercom_spi_transaction_done(inst->spi_inst, inst->spi_out_buffer))
    {
        /* code */
    }

    // need to add the way that it checks if transactions are done. 
}

void mcp2518fd_reset(struct mcp2518fd_desc *inst)
{
    uint8_t buffer_index = inst->next_out_buffer;
    *((inst->spi_out_buffer[buffer_index]).buffer) = 0x0000;
    inst->spi_out_buffer[buffer_index].empty = 0;
    set_next_out_buffer(inst);

    sercom_spi_start(inst->spi_inst, &(inst->spi_out_buffer[buffer_index].spi_transaction_id),
                     MCP2518FD_BAUD_RATE, inst->cs_pin_group, inst->cs_pin_mask,
                     &(inst->spi_out_buffer[buffer_index].buffer), 16, 0, 0);
    // is this the right way to not recieve?
    // Is the out length in bits or in bytes? I've assumed bits.
}

void set_next_out_buffer(struct mcp2518fd_desc *inst)
{
    if (inst->spi_out_buffer[inst->next_out_buffer].empty)
    {
        return;
    }
    else
    {
        while (!(inst->spi_out_buffer[inst->next_out_buffer].empty))
        {
            (inst->next_out_buffer)++;
            if (inst->next_out_buffer == inst->num_out_buffers)
            {
                inst->next_out_buffer = 0;
            }
            /** really need some error handling if all the buffers are full */
        }
        return;
    }
}