/**
 * @file evsys.h
 * @desc Event System Driver
 * @author Samuel Dewan
 * @date 2019-04-27
 * Last Author:
 * Last Edited On:
 */

#ifndef evsys_h
#define evsys_h

#include "global.h"

#define EVSYS_CHANNEL_DISABLED 0xFF

/** Edge detection setting for event channels with synchronous paths */
enum evsys_edge {
    /** No event output when using the resynchronized or synchronous path */
    EVSYS_EDGE_NO_EVT_OUTPUT = EVSYS_CHANNEL_EDGSEL_NO_EVT_OUTPUT_Val,
    /** Event detection only on the rising edge of the signal from the event
     generator when using the resynchronized or synchronous path */
    EVSYS_EDGE_RISING = EVSYS_CHANNEL_EDGSEL_RISING_EDGE_Val,
    /** Event detection only on the falling edge of the signal from the event
     generator when using the resynchronized or synchronous path */
    EVSYS_EDGE_FALLING = EVSYS_CHANNEL_EDGSEL_FALLING_EDGE_Val,
    /** Event detection on rising and falling edges of the signal from the event
     generator when using the resynchronized or synchronous path */
    EVSYS_EDGE_BOTH = EVSYS_CHANNEL_EDGSEL_BOTH_EDGES_Val
};

/** Event propagation path type */
enum evsys_path {
    /** Synchronous path (use when event and channel are on the same clock
        domain) */
    EVSYS_PATH_SYNCHRONOUS = EVSYS_CHANNEL_PATH_SYNCHRONOUS_Val,
    /** Resynchronized path (use when event and channel are on distinct clock
        domains) */
    EVSYS_PATH_RESYNCHRONIZED = EVSYS_CHANNEL_PATH_RESYNCHRONIZED_Val,
    /** Asynchronous path */
    EVSYS_PATH_ASYNCHRONOUS = EVSYS_CHANNEL_PATH_ASYNCHRONOUS_Val
};

/**
 *  Initilize the event system.
 */
extern void init_evsys (void);

/**
 *  Configure the mutiplexer for an event user.
 *
 *  @param user The event user for which the multiplexer should be configured
 *  @param channel The channel to which the even user's multiplexer should be
 *                 set or EVSYS_CHANNEL_DISABLED if the user should not be
 *                 connected to any event channel
 */
extern void evsys_configure_user_mux (uint8_t user, uint8_t channel);

/**
 *  Configure an event channel.
 *
 *  @param channel The channel to be configured
 *  @param generator The event generator for the channel
 *  @param clock_mask Mask for the Generic Clock Generator to be used by the
 *                    channel's Ceneric Clock
 *  @param path The propagation path type for the event channel
 *  @param edge The edge detection type for the channel (applicable only if a
 *              synchronous or resynchronized path is used)
 */
extern void evsys_configure_channel (uint8_t channel, uint8_t generator,
                                     uint32_t clock_mask, enum evsys_path path,
                                     enum evsys_edge edge);

/**
 *  Trigger an EVSYS event from software.
 *
 *  @param channel The channel on which the event should be triggered
 */
extern void evsys_software_event (uint8_t channel);

#endif /* evsys_h */
