/**
 * @file mcp23s17.h
 * @desc Driver for Microchip MCP23S17 IO Expander
 * @author Samuel Dewan
 * @date 2019-03-14
 * Last Author: Samuel Dewan
 * Last Edited On: 2019-03-15
 */

#ifndef mcp23s17_h
#define mcp23s17_h

#include "global.h"

#include "mcp23s17-registers.h"

#include "sercom-spi.h"

/** The maximum number of interrupts which may be enabled */
#define MCP23S17_MAX_NUM_INTERRUPTS  4

/** Baud rate used to communicate with IO expander */
#define MCP23S17_BAUD_RATE      8000000UL

#define MCP23S17_MODE_INPUT     1
#define MCP23S17_MODE_OUTPUT    0

#define MCP23S17_VALUE_HIGH     1
#define MCP23S17_VALUE_LOW      0

#define MCP23S17_PULL_UP_ENABLED    1
#define MCP23S17_PULL_UP_DISABLED   0

/**
 *  Description of an MCP23S17 IO expander.
 */
struct mcp23s17_desc_t;

/** Represents ony of the two ports on an MCP23S17 */
enum mcp23s17_port {
    MCP23S17_PORT_A,
    MCP23S17_PORT_B
};

/** Represents a pin on an MCP23S17 */
union mcp23s17_pin_t {
    struct {
        uint8_t pin:3;                  /** The pin number for this pin */
        enum mcp23s17_port port:1;      /** The port that this pin is in */
        uint8_t :4;                     /** RESERVED */
    };
    uint8_t value;
};

/** Represents an interrupt trigger type on the MCP23S17 */
enum mcp23s17_interrupt_type {
    /** Interrupt triggered if the value of the pin changes in any direction */
    MCP23S17_INT_ON_CHANGE,
    /** Interrupt triggered if the value of the pin changes from high to low */
    MCP23S17_INT_FALLING_EDGE,
    /** Interrupt triggered if the value of the pin changes from low to high */
    MCP23S17_INT_RISING_EDGE
};

/** Represents the possible SPI transaction states for the MCP23S17 driver */
enum mcp23s17_transaction_state {
    /** No SPI transaction in progress */
    MCP23S17_SPI_NONE,
    /** An SPI transaction to refresh the cached GPIO register values is
        ongoing */
    MCP23S17_SPI_GPIO,
    /** An SPI transaction to refresh the cached interrupts register values is
        ongoing */
    MCP23S17_SPI_INTERRUPTS,
    /** Another SPI transaction is in progress */
    MCP23S17_SPI_OTHER
};

/** Type of function called when an interrupt occures */
typedef void (*mcp23s17_int_callback)(struct mcp23s17_desc_t *inst,
                                      union mcp23s17_pin_t pin, uint8_t value);

// Ignoring warnings about inefficient alignment for this struct
#pragma GCC diagnostic ignored "-Wattributes"

struct mcp23s17_desc_t {
    /** Period with which the input registers should be polled automatically */
    uint32_t poll_period;
    /** Stores the last time at which the GPIO registers where polled */
    uint32_t last_polled;
    /** Callback function for interrupts */
    mcp23s17_int_callback interrupt_callback;
    /** SPI instance used to communicate with device */
    struct sercom_spi_desc_t *spi_inst;
    /** Mask for devices chip select pin */
    uint32_t cs_pin_mask;
    /** Group in which devices chip select pin is located */
    uint8_t cs_pin_group;
    
    /** Opcode, located here to provide easy writing of the entire register
        cache to the IO expander */
    uint8_t opcode;
    /** Register address, located here to provide easy writing of the entire
        register cache to the IO expander */
    uint8_t reg_addr;
    /** Cache of device register values */
    struct mcp23s17_register_map registers;
    
    /** Buffer used in SPI transaction to write only the OLAT register */
    uint8_t spi_out_buffer[4];
    /** ID to keep track of SPI transactions */
    uint8_t spi_transaction_id;
    
    /** Flag that indicates that a the cached interupt flag and value register
     values need to updated from the device */
    uint8_t interrupts_dirty:1;
    /** Flag that indicates that a the cached input register values need to
     updated from the device */
    uint8_t gpio_dirty:1;
    /** Flag that indicates that the configuration registers on the device need
        to be updated from the cache */
    uint8_t config_dirty:1;
    /** Flag that indicates that the output latch registers on the device need
        to be updated from the cache */
    uint8_t olat_dirty:1;
    /** Flag used to unsure that the service function is not executed in an
        interrupt while it is already being run in the main thread */
    uint8_t service_lock:1;
    
    /** The current SPI transaction state */
    enum mcp23s17_transaction_state transaction_state:2;
} __attribute__((packed));

// Stop ignoring warnings about inefficient alignment
#pragma GCC diagnostic pop


/**
 *  Initilize an MCP23S17 GPIO expander.
 *
 *  @param descriptor The instance descriptor to be initilized
 *  @param address The 3 bit selectable address of the IO expander
 *  @param spi_inst The SPI instance used to communicate with the device
 *  @param poll_period The period in milliseconds with which the device should
 *                     be polled for updated input data, a period of 0 means
 *                     that automatic polling will not occur
 *  @param cs_pin_mask The mask for the devices chip select pin
 *  @param cs_pin_group The group in which the devices chip select pin is
 *                      located
 */
extern void init_mcp23s17(struct mcp23s17_desc_t *descriptor, uint8_t address,
                          struct sercom_spi_desc_t *spi_inst,
                          uint32_t poll_period, uint32_t cs_pin_mask,
                          uint8_t cs_pin_group);

/**
 *  Service to be run in each iteration of the main loop.
 *
 *  @param inst The descriptor for which the service should be run
 */
extern void mcp23s17_service(struct mcp23s17_desc_t *inst);

/**
 *  Poll the IO expander for updates to the input registers
 *
 *  @param inst The descriptor for the IO expander
 */
static inline void mcp23s17_poll(struct mcp23s17_desc_t *inst)
{
    // Mark the GPIO registers as needing to be fetched
    inst->gpio_dirty = 1;
    // Start fetching the registers immediately if possible
    mcp23s17_service(inst);
}

/**
 *  Check if the IO expander is in the process of being polled for updates to
 *  the input registers.
 *
 *  @param inst The descriptor for the IO expander
 *
 *  @return 1 if polling is in progress, 0 otherwise
 */
static inline uint8_t mcp23s17_poll_in_progress(struct mcp23s17_desc_t *inst)
{
    return inst->gpio_dirty || (inst->transaction_state != MCP23S17_SPI_GPIO);
}

/**
 *  Configure the mode for a pin.
 *
 *  @param inst The descriptor for the IO expander
 *  @param pin The pin for which the mode should be set
 *  @param mode 0 for output, any other value for input
 */
extern void mcp23s17_set_pin_mode(struct mcp23s17_desc_t *inst,
                                  union mcp23s17_pin_t pin, uint8_t mode);

/**
 *  Get the value from a pin which is configured as an input.
 *
 *  @param inst The descriptor for the IO expander
 *  @param pin The pin for which the value should be returned
 *
 *  @return The value of the pin, 0 for logic low, 1 for logic high
 */
static inline uint8_t mcp23s17_get_input(struct mcp23s17_desc_t *inst,
                                         union mcp23s17_pin_t pin)
{
    return !!(inst->registers.GPIO[pin.port].reg & (1 << pin.pin));
}

/**
 *  Set the value for a pin which is configured as an output.
 *
 *  @param inst The descriptor for the IO expander
 *  @param pin The pin for which the value should be set
 *  @param value New value for the pin, 0 for logic low, logic high otherwise
 */
extern void mcp23s17_set_output(struct mcp23s17_desc_t *inst,
                                union mcp23s17_pin_t pin, uint8_t value);

/**
 *  Enable or disable pull-up for a pin which is configured as an input.
 *
 *  @param inst The descriptor for the IO expander
 *  @param pin The pin for which the pull-up should be set
 *  @param value 0 to disable pull-up, any other value to enable
 */
extern void mcp23s17_set_pull_up(struct mcp23s17_desc_t *inst,
                                 union mcp23s17_pin_t pin, uint8_t value);

/**
 *  Enable an interrupt for a pin which is configured as an input.
 *
 *  @param inst The descriptor for the IO expander
 *  @param pin The pin for which the interrupt should be enabled
 *  @param type The trigger type for the interrupt
 */
extern void mcp23s17_enable_interrupt(struct mcp23s17_desc_t *inst,
                                      union mcp23s17_pin_t pin,
                                      enum mcp23s17_interrupt_type type);

/**
 *  Disable the interrupt for a pin.
 *
 *  @param inst The descriptor for the IO expander
 *  @param pin The pin for which the interrupt should be disabled
 */
extern void mcp23s17_disable_interrupt(struct mcp23s17_desc_t *inst,
                                       union mcp23s17_pin_t pin);

/**
 *  Function to be called on a falling edge of the IO expander's interrupt pin.
 *
 *  @param inst The descriptor for the IO expander
 */
extern void mcp23s17_handle_interrupt(struct mcp23s17_desc_t *inst);

#endif /* mcp23s17_h */
