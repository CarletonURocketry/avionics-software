/**
 * @file telemetry.h
 * @desc Service to send telemetry
 * @author Samuel Dewan
 * @date 2019-06-11
 * Last Author:
 * Last Edited On:
 */

#ifndef telemetry_h
#define telemetry_h

#include "global.h"
#include "variant.h"
#include "board.h"

#include "logging.h"
#include "radio-transport.h"

#include "adc.h"
#include "ms5611.h"
#include "gnss-xa1110.h"
#include "mpu9250.h"


struct telemetry_service_desc_t {
    // Telemetry destinations
    /** Logging service instance for writing to SD card */
    struct logging_desc_t *logging;
    /** Radio instance for transmitting to ground */
    struct radio_transport_desc *radio;

    // Polled sensor drivers
#ifdef ENABLE_ALTIMETER
    /** MS5611 altimeter driver instance descriptor */
    struct ms5611_desc_t *ms5611_alt;
    /** The timestamp for the last altimeter data point that was logged */
    uint32_t last_ms5611_alt_log_time;
    /** The timestamp for the last altimeter data point that was transmitted */
    uint32_t last_ms5611_alt_radio_time;
#endif

#ifdef ENABLE_GNSS
    /** GNSS driver instance descriptor */
    struct gnss *gnss;
    /** The timestamp for the last GNSS location data point that was logged */
    uint32_t last_gnss_loc_log_time;
    /** The timestamp for the last GNSS location data point that was
        transmitted */
    uint32_t last_gnss_loc_radio_time;
    /** The timestamp for the last GNSS metadata data point that was logged */
    uint32_t last_gnss_meta_log_time;
    /** The timestamp for the last GNSS metadata data point that was
        transmitted */
    uint32_t last_gnss_meta_radio_time;
#endif

#ifdef ENABLE_IMU
    /** IMU driver instance descriptor */
    struct mpu9250_desc_t *mpu9250_imu;
    /** The timestamp for the last IMU data that was transmitted */
    uint32_t last_mpu9250_radio_time;
#endif

    /** The last time software status data was logged */
    uint32_t last_status_log_time;
    /** The last time software status data was transmitted */
    uint32_t last_status_radio_time;
};


/**
 *  Initialize the telemetry service.
 *
 *  @param inst Telemetry service instance descriptor to be initialized
 *  @param logging Logging service instance (or NULL for no SD card logging)
 *  @param radio Radio transport descriptor (or NULL for radio transmission)
 */
extern void init_telemetry_service(struct telemetry_service_desc_t *inst,
                                   struct logging_desc_t *logging,
                                   struct radio_transport_desc *radio);

/**
 *  Telemetry service function to be run in each iteration of main loop.
 *
 *  @param inst The telemetry service instance for which the service function
 *              should be run
 */
extern void telemetry_service(struct telemetry_service_desc_t *inst);



// Message logging

/**
 *  Severity levels which dictate how a message to be logged should be handled.
 */
enum telemetry_msg_severity {
    /** Critical - Sends via radio as well as logging to SD card */
    TELEM_MSG_SEVERITY_CRIT = 2,
    /** Error */
    TELEM_MSG_SEVERITY_ERR = 3,
    /** Warning */
    TELEM_MSG_SEVERITY_WARN = 4,
    /** Information */
    TELEM_MSG_SEVERITY_INFO = 6,
    /** Low level debugging info */
    TELEM_MSG_SEVERITY_DEBUG = 7
};

/**
 *  Post a string message.
 *
 *  @param inst Telemetry service instance
 *  @param severity Message severity
 *  @param msg The message to be posted
 *
 *  @return 0 if successful
 */
extern int telemetry_post_msg(struct telemetry_service_desc_t *inst,
                              enum telemetry_msg_severity severity,
                              const char *msg);


// Functions to register instance descriptors for polled sensors

#ifdef ENABLE_ALTIMETER
/**
 *  Register an MS5611 altimeter driver instance for use by the telemetry
 *  service.
 *
 *  @param inst Telemetry service instance
 *  @param ms5611_alt The MS5611 altimeter driver instance
 */
static inline void telemetry_register_ms5611_alt(
                                        struct telemetry_service_desc_t *inst,
                                        struct ms5611_desc_t *ms5611_alt)
{
    inst->ms5611_alt = ms5611_alt;
}
#endif

#ifdef ENABLE_GNSS
/**
 *  Register an GNSS driver instance for use by the telemetry service.
 *
 *  @param inst Telemetry service instance
 *  @param gnss The GNSS driver instance
 */
static inline void telemetry_register_gnss(struct telemetry_service_desc_t *inst,
                                           struct gnss *gnss)
{
    inst->gnss = gnss;
}
#endif

#ifdef ENABLE_IMU
static inline void telemetry_register_imu(struct telemetry_service_desc_t *inst,
                                          struct mpu9250_desc_t *mpu9250_imu)
{
    inst->mpu9250_imu = mpu9250_imu;
}
#endif


// Functions to post data from pushed sensors


#endif /* telemetry_h */
