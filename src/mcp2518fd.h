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

#define MCP2518FD_BAUD_RATE 100 // need to set the baud rate, figure out how to find it

enum mcp2518fd_mode {
    /** The value is the value to set register REQOP to for each mode */
    /** The REQOP value does not alway correspond with the OPMOD register showing current mode */
    MCP2518FD_CONFIGURATION_MODE, // = 0b100,
    /** Config mode is required to change some settings and to switch between most modes */
    /** enum State {Working = 1, Failed = 0, Freezed = 0}; */

    /** NORMAL MODES */
    MCP2518FD_NORMAL_FD_MODE, // = 0b000,
    /** Mode to handle CAN FD and CAN 2.0 messages */
    MCP2518FD_NORMAL_20_MODE, // = 0b110,
    /** Supports only CAN 2.0 messages */
    MCP2518FD_SLEEP_MODE, // = 0b001,
    /** Low power mode preserving registers and RAM */
    MCP2518FD_LOW_POWER_MODE,
    /** Ultra low power mode, leaving only logic for wake up */ 

    /** DEBUG MODES */
    MCP2518FD_LISTEN_ONLY_MODE,
    MCP2518FD_RESTRICTED_MODE,
    /** Can only recieve and acknowledge valid frames */
    MCP2518FD_INTERNAL_LOOPBACK_MODE,
    MCP2518FD_EXTERNAL_LOOPBACK_MODE
    /** Loopback modes allow transmission of messages from transmit FIFOs to recieve FIFOs */

};

struct mcp2518fd_config {
    enum OSC;
    /** Configures the oscillator */
    
    uint8_t spi_mode:1;
    /** determines whether SPI is 0,0 mode or 1,1 */ 

    uint8_t PLLEN:1;
    /** Set to multiply 4MHz clock by 10 */
    uint8_t SCLKDIV:1;
    /** Set to divide the system clock by 2 */
};

struct spi_out_buffer{
    uint8_t buffer[12];
    uint8_t spi_transaction_id;
    uint8_t empty:1;
};

struct mcp2518fd_desc {
    /** SPI instance used to communicate with the device */ 
    struct sercom_spi_desc_t *spi_inst;
    /** The pin mask for the GPIO pins for the SPI chip select */
    uint32_t cs_pin_mask;
    /** The GPIO pin group for SPI chip select */
    uint8_t cs_pin_group;

    // uint8_t spi_transaction_id;

    uint8_t next_out_buffer;
    // could make this a point to the next buffer and not an index, but not sure
    uint8_t num_out_buffers;

    struct spi_out_buffer spi_out_buffer[8];
    // How many out buffers do we need?
    // Should we make the buffers as big as they need to be? Need to actually be 8 + the spi commands etc
};