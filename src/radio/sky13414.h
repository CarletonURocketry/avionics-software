/**
 * @file sky13414.h
 * @desc Driver for SKY13414 antenna switch
 * @author Samuel Dewan
 * @date 2020-03-14
 * Last Author:
 * Last Edited On:
 */

#ifndef sky13414_h
#define sky13414_h

#include "global.h"
#include "radio/rn2483.h"

/**
 *  Possible states of SKY13414 switch.
 */
enum sky13414_state {
    /** Antenna pin connected to RF1 pin */
    SKY13414_RF1 = 0b000,
    /** Antenna pin connected to RF2 pin */
    SKY13414_RF2 = 0b001,
    /** Antenna pin connected to RF3 pin */
    SKY13414_RF3 = 0b010,
    /** Antenna pin connected to RF4 pin */
    SKY13414_RF4 = 0b011,
    /** Antenna pin internally disconnected */
    SKY13414_NONE = 0b100,
    /** Antenna pin connected to 50 Ohm load */
    SKY13414_50_OHM = 0b101,
    /** Antenna switch is in shutdown mode */
    SKY13414_SHUTDOWN = 0b111
};

/**
 *  Descriptor for SKY13414 antenna switch driver instance.
 */
struct sky13414_desc_t {
    struct rn2483_desc_t *radio;
    enum sky13414_state  state:3;
    enum rn2483_pin  v1_pin:5;
    enum rn2483_pin  v2_pin:5;
    enum rn2483_pin  v3_pin:5;
};

/**
 *  Initialize an instance of the SKY13414 antenna switch driver.
 *
 *  @param inst Pointer to the instance structure to be initialized
 *  @param radio Pointer to the RN2483 driver instance on which the GPIO pins
 *               are located
 *  @param v1 RN2483 GPIO pin connected to the V1 pin of the antenna switch
 *  @param v2 RN2483 GPIO pin connected to the V2 pin of the antenna switch
 *  @param v3 RN2483 GPIO pin connected to the V3 pin of the antenna switch
 */
extern void init_sky13414(struct sky13414_desc_t *inst,
                          struct rn2483_desc_t *radio, enum rn2483_pin v1,
                          enum rn2483_pin v2, enum rn2483_pin v3);

/**
 *  Change the state of a SKY13414 antenna switch.
 *
 *  @param inst Driver instance for antenna switch
 *  @param state New state for antenna switch
 */
extern void sky13414_set(struct sky13414_desc_t *inst,
                         enum sky13414_state state);

/**
 *  Get the current state of a SKY13414 antenna switch.
 *
 *  @param inst Driver instance for antenna switch
 *
 *  @return The current state of the switch
 */
extern enum sky13414_state sky13414_current_state(struct sky13414_desc_t *inst);

#endif /* sky13414_h */
