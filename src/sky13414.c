/**
 * @file sky13414.c
 * @desc Driver for SKY13414 antenna switch
 * @author Samuel Dewan
 * @date 2020-03-14
 * Last Author:
 * Last Edited On:
 */

#include "sky13414.h"

#define V1_VALUE(state) (!!(state & 0b100))
#define V2_VALUE(state) (!!(state & 0b010))
#define V3_VALUE(state) (!!(state & 0b001))


void init_sky13414(struct sky13414_desc_t *inst, struct rn2483_desc_t *radio,
                   enum rn2483_pin v1, enum rn2483_pin v2, enum rn2483_pin v3)
{
    // Store configuration information
    inst->radio = radio;
    inst->v1_pin = v1;
    inst->v2_pin = v2;
    inst->v3_pin = v3;
    // Set the pin modes of the GPIO pins
    rn2483_set_pin_mode(inst->radio, inst->v1_pin, RN2483_PIN_MODE_OUTPUT);
    rn2483_set_pin_mode(inst->radio, inst->v2_pin, RN2483_PIN_MODE_OUTPUT);
    rn2483_set_pin_mode(inst->radio, inst->v3_pin, RN2483_PIN_MODE_OUTPUT);
    // Set a real state and update outputs
    sky13414_set(inst, SKY13414_SHUTDOWN);
}

void sky13414_set(struct sky13414_desc_t *inst, enum sky13414_state state)
{
    rn2483_set_output(inst->radio, inst->v1_pin, V1_VALUE(state));
    rn2483_set_output(inst->radio, inst->v2_pin, V2_VALUE(state));
    rn2483_set_output(inst->radio, inst->v3_pin, V3_VALUE(state));
    inst->state = state;
}

enum sky13414_state sky13414_current_state(struct sky13414_desc_t *inst)
{
    return inst->state;
}
