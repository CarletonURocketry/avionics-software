/**
 * @file deployment.c
 * @desc Service which handles deployment of parachute
 * @author Samuel Dewan
 * @date 2022-06-10
 * Last Author:
 * Last Edited On:
 */

#include "deployment.h"

#include <math.h>

#include "variant.h"
#include "gpio.h"


void init_deployment(struct deployment_service_desc_t *const inst,
                     struct ms5611_desc_t *const ms5611_alt,
                     struct mpu9250_desc_t *const mpu9250_imu)
{
    inst->state = DEPLOYMENT_STATE_IDLE;

    inst->ms5611_alt = ms5611_alt;
    inst->mpu9250_imu = mpu9250_imu;
    inst->max_altitude = 0.0f;
    inst->last_sample_time = 0;
    inst->decending_sample_count = 0;
    inst->first_previous_altitude = 0.0f;
}




static inline int is_armed(void)
{
#ifdef ARMED_SENSE_PIN
    return gpio_get_input(ARMED_SENSE_PIN) == 1;
#else
    return 1;
#endif
}

static inline int test_abs_acceleration(
                                const struct mpu9250_desc_t *const mpu9250_imu,
                                uint32_t threashold)
{
    const uint16_t x = mpu9250_get_accel_x(mpu9250_imu);
    const uint16_t y = mpu9250_get_accel_y(mpu9250_imu);
    const uint16_t z = mpu9250_get_accel_z(mpu9250_imu);

    int64_t abs = ((int64_t)((int32_t)x * x) + (int64_t)((int32_t)y * y) +
                   (int64_t)((int32_t)z * z));
    threashold *= mpu9250_accel_sensitivity(mpu9250_imu);

    return abs > (threashold * threashold);
}

static inline int is_decending(struct deployment_service_desc_t *const inst)
{

#ifdef ENABLE_DEPLOYMENT_SERVICE
    // Check if we have a new sample
    const uint32_t alt_time = ms5611_get_last_reading_time(inst->ms5611_alt);
    if (alt_time <= inst->last_sample_time) {
        return 0;
    }
    inst->last_sample_time = alt_time;
  
    //Compares the new recorded altitude to the previously recorded altitude 
   const uint32_t alt_time = ms5611_get_last_reading_time(inst->ms5611_alt);
    if (alt_time <= inst->last_sample_time) {
        return 0;
    }
    inst->last_sample_time = alt_time;

    const float altitude = ms5611_get_altitude(inst->ms5611_alt);
    if (altitude >= inst->first_previous_altitude)  { 
        
        inst->descending_sample_count = 0;
        //resets the descending sample count if the new altitude is greater than the previous altitude 
        return 0;
    else {

        inst->descending_sample_count++
        //increments the descending sample count if the new altiude is less than the previous one
    }
    inst->first_previous_altitude = altitude; 
    // the new altitude is reparamaterized as the old altitude 
    return (inst->decending_sample_count >
        DEPLOYMENT_DESCENDING_SAMPLE_THREASHOLD);
        //if the sequence of samples where the altitude is not greater than the previous altitude reaches a threshold, will return 1
    }
#else
    return 0;
#endif
}


void deployment_service(struct deployment_service_desc_t *const inst)
{
#ifdef ENABLE_DEPLOYMENT_SERVICE
    switch (inst->state) {
        case DEPLOYMENT_STATE_IDLE:
            if (is_armed()) {
                inst->state = DEPLOYMENT_STATE_ARMED;
            }
            break;
        case DEPLOYMENT_STATE_ARMED:
            inst->last_altitude = ms5611_get_altitude(inst->ms5611_alt);
            if (test_abs_acceleration(inst->mpu9250_imu,
                                DEPLOYMENT_POWERED_ASCENT_ACCEL_THREASHOLD) ||
                inst->last_altitude >
                                DEPLOYMENT_POWERED_ASCENT_ALT_THREASHOLD) {
                inst->state = DEPLOYMENT_STATE_POWERED_ASCENT;
            }
            break;
        case DEPLOYMENT_STATE_POWERED_ASCENT:
            inst->last_altitude = ms5611_get_altitude(inst->ms5611_alt);
            if (!test_abs_acceleration(inst->mpu9250_imu,
                                DEPLOYMENT_COASTING_ASCENT_ACCEL_THREASHOLD) ||
                inst->last_altitude >
                                DEPLOYMENT_COASTING_ASCENT_ALT_THREASHOLD) {
                if (inst->last_altitude >
                    DEPLOYMENT_COASTING_ASCENT_ALT_MINIMUM) {

                    inst->state = DEPLOYMENT_STATE_COASTING_ASCENT;
                }
            }
            break;
        case DEPLOYMENT_STATE_COASTING_ASCENT:
            if (is_decending(inst)) {
                gpio_set_output(EMATCH_1_PIN, 1);
                gpio_set_output(EMATCH_2_PIN, 1);
                inst->deployment_time = millis;
                inst->state = DEPLOYMENT_STATE_DEPLOYING;
            }
            break;
        case DEPLOYMENT_STATE_DEPLOYING:
            if ((millis - inst->deployment_time) >
                    DEPLOYMENT_EMATCH_FIRE_DURATION) {
                gpio_set_output(EMATCH_1_PIN, 0);
                gpio_set_output(EMATCH_2_PIN, 0);
                inst->state = DEPLOYMENT_STATE_DESCENT;
            }
            break;
        case DEPLOYMENT_STATE_DESCENT:
            if (is_landed(inst)) {
                inst->state = DEPLOYMENT_STATE_RECOVERY;
            }
            break;
        case DEPLOYMENT_STATE_RECOVERY:
            break;
        default:
            break;
    }
#else
    return;
#endif
}
