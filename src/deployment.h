/**
 * @file deployment.h
 * @desc Service which handles deployment of parachute
 * @author Samuel Dewan
 * @date 2022-06-10
 * Last Author:
 * Last Edited On:
 */

#ifndef deployment_h
#define deployment_h

#include "src/sensors/ms5611.h"
#include "src/sensors/mpu9250.h"

enum deployment_service_state {
    DEPLOYMENT_STATE_IDLE = 0x0,
    DEPLOYMENT_STATE_ARMED,
    DEPLOYMENT_STATE_POWERED_ASCENT,
    DEPLOYMENT_STATE_COASTING_ASCENT,
    DEPLOYMENT_STATE_DEPLOYING,
    DEPLOYMENT_STATE_DESCENT,
    DEPLOYMENT_STATE_RECOVERY
};

struct deployment_service_desc_t {
    enum deployment_service_state state;
    struct ms5611_desc_t *ms5611_alt;
    struct mpu9250_desc_t *mpu9250_imu;
    union {
        float max_altitude;
        float last_altitude;
    };
    union {
        uint32_t last_sample_time;
        uint32_t deployment_time;
    };
    union {
        uint8_t decending_sample_count;
        uint8_t landing_sample_count;
    };
};


/**
 *  Initialize the deployment service.
 *
 *  @param inst A deployment service instance descriptor
 *  @param ms6511_alt Altimeter instance
 *  @param mpu9250_imu IMU instance
 */
extern void init_deployment(struct deployment_service_desc_t *inst,
                            struct ms5611_desc_t *ms5611_alt,
                            struct mpu9250_desc_t *mpu9250_imu);

/**
 *  Deployment service function to be called in each iteration of the main loop.
 *
 *  @param inst A deployment service instance descriptor
 */
extern void deployment_service(struct deployment_service_desc_t *inst);


/**
 *  Get state of deployment services.
 */
static inline enum deployment_service_state deployment_get_state(
                            const struct deployment_service_desc_t *const inst)
{
    return inst->state;
}


#endif /* deployment_h */
