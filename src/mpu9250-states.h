/**
 * @file mpu9250-states.h
 * @desc Driver state machine for MPU9250 IMU
 * @author Samuel Dewan
 * @date 2021-09-29
 * Last Author:
 * Last Edited On:
 */

#ifndef mpu9250_states_h
#define mpu9250_states_h

#include "mpu9250.h"

typedef int (*mpu9250_state_handler_t)(struct mpu9250_desc_t *inst);


#define MPU9250_SAMPLE_LEN              21


/**
 *  Parse a sample from the MPU9250 to populate last sensor data fields.
 *
 *  This function takes a 21 byte raw sample from the IMU and populate the
 *  following fields:
 *      - last_accel_x, last_accel_y, last_accel_z
 *      - last_temp
 *      - last_gyro_x, last_gyro_y, last_gyro_z
 *      - last_mag_x, last_mag_y, last_mag_z
 *      - last_mag_overflow
 *
 *  @param inst Driver instance
 *  @param s Pointer to raw sample data from IMU
 */
extern void parse_mpu9250_data(struct mpu9250_desc_t *const inst,
                               const uint8_t *const s);



/**
 *  Array of functions for handling FSM states.
 *
 *  Each state handler returns 0 if the service function should return or 1 if
 *  the service function should call the handler for the next state immediately.
 */
extern const mpu9250_state_handler_t mpu9250_state_handlers[];

#endif /* mpu9250_states_h */
