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

// for reading from the can bus, how do I return it to whatever wants to read it?

// should I maybe have made it just one set of buffers and have a bit for if it is a send or a recieve? probably

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
    for (uint8_t i = 0; i < inst->num_out_buffers; i++)
    {
        if (!inst->spi_out_buffer[i].empty && sercom_spi_transaction_done(inst->spi_inst, inst->spi_out_buffer[i].spi_transaction_id))
        {
            inst->spi_out_buffer[i].empty = 1;
            sercom_spi_clear_transaction(inst, inst->spi_out_buffer[i].spi_transaction_id);
        }
    }
    for (uint8_t i = 0; i < inst->num_in_buffers; i++)
    {
        if (!inst->spi_in_buffer[i].empty && sercom_spi_transaction_done(inst->spi_inst, inst->spi_in_buffer[i].spi_transaction_id))
        {
            *(inst->spi_in_buffer[i].completed) = 1;
            inst->spi_in_buffer[i].empty = 1;
            sercom_spi_clear_transaction(inst, inst->spi_in_buffer[i].spi_transaction_id);
        }
    }
    // all duplicate code, could be fixed by reducing the number of buffers, but will check before we do that
}

/**
 * Should only be reset after entering configuration mode
 */
void mcp2518fd_reset(struct mcp2518fd_desc *inst)
{
    uint8_t buffer_index = inst->next_out_buffer;
    *((inst->spi_out_buffer[buffer_index]).buffer) = 0x0000;
    inst->spi_out_buffer[buffer_index].empty = 0;
    mcp2518fd_set_next_buffer(inst);

    sercom_spi_start(inst->spi_inst, &(inst->spi_out_buffer[buffer_index].spi_transaction_id),
                     MCP2518FD_BAUD_RATE, inst->cs_pin_group, inst->cs_pin_mask,
                     &(inst->spi_out_buffer[buffer_index].buffer), 16, 0, 0);
    // is this the right way to not recieve?
    // Is the out length in bits or in bytes? I've assumed bits.
}

void mcp2518fd_enter_config(struct mcp2518fd_desc *inst)
{
    return; // not written yet
}

/**
 * Writes to the SFR / RAM of the CAN chip
 * @param address the address on the CAN chip to write to (12 bits)
 * @param message pointer to the message content
 * @param message_data_length the length of the data in bytes to be written to the sfr / ram
 */
void mcp2518fd_write(struct mcp2518fd_desc *inst, uint16_t address, uint8_t *message, uint8_t message_data_length)
{
    uint8_t buffer_index = inst->next_out_buffer;
    inst->spi_out_buffer[buffer_index].empty = 0;
    mcp2518fd_set_next_buffer(inst);
    inst->spi_out_buffer[buffer_index].buffer[0] = 0b0010 << 12 | address;
    inst->spi_out_buffer[buffer_index].buffer[4] = *message;
    uint8_t whole_message_length = 4 + message_data_length;
    sercom_spi_start(inst->spi_inst, &(inst->spi_out_buffer[buffer_index].spi_transaction_id),
                     MCP2518FD_BAUD_RATE, inst->cs_pin_group, inst->cs_pin_mask,
                     &(inst->spi_out_buffer[buffer_index].buffer), whole_message_length, 0, 0);
    // now here we are assuming spi transaction length is measured in bytes, which is it?
    return;
}

/** 
 * Writes to the SFR / RAM where a CRC is done on the message
 */
void mcp2518fd_write_crc(struct mcp2518fd_desc *inst)
{
    return; // not written
}

/**
 * Writes to the SFR / RAM but checks the CRC before writing
 */
void mcp2518fd_write_safe(struct mcp2518fd_desc *inst)
{
    return; // not written
}

/**
 * Reads from the SFR / RAM on the CAN chip
 * @param read_data_length number of bytes to read starting at address
 * @param address address on CAN chip to start reading from 
 * @param read_out address of where to store read data
 */
void mcp2518fd_read(struct mcp2518fd_desc *inst, uint8_t address, uint8_t *read_out, uint8_t read_data_length)
{
    uint8_t buffer_index = inst->next_in_buffer;
    inst->spi_in_buffer[buffer_index].empty = 0;
    mcp2518fd_set_next_buffer(inst);
    inst->spi_out_buffer[buffer_index].buffer[0] = 0b0011 << 12 | address;
    sercom_spi_start(inst->spi_inst, &(inst->spi_in_buffer[buffer_index].spi_transaction_id),
                     MCP2518FD_BAUD_RATE, inst->cs_pin_group, inst->cs_pin_mask,
                     &(inst->spi_in_buffer[buffer_index].in_buffer), 4,
                     read_out, read_data_length);
    return;
}

/** 
 * Reads from the SFR / RAM and calculates the CRC
 */
void mcp2518fd_read_crc(struct mcp2518fd_desc *inst)
{
    return; // not written
}

void mcp2518fd_set_next_buffer(struct mcp2518fd_desc *inst)
{
    if (!inst->spi_out_buffer[inst->next_out_buffer].empty)
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
    }
    if (!inst->spi_in_buffer[inst->next_out_buffer].empty)
    {
        while (!(inst->spi_in_buffer[inst->next_in_buffer].empty))
        {
            (inst->next_in_buffer)++;
            if (inst->next_in_buffer == inst->num_in_buffers)
            {
                inst->next_in_buffer = 0;
            }
            /** really need some error handling if all the buffers are full */
        }
    }
    return;
}

void mcp2518fd_CAN_send(struct mcp2518fd_desc *inst, uint8_t *message, uint8_t message_length)
{
    return; // not written yet
}
