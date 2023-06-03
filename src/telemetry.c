/**
 * @file telemetry.c
 * @desc Service to send telemetry
 * @author Samuel Dewan
 * @date 2019-06-11
 * Last Author:
 * Last Edited On:
 */

#include "telemetry.h"

#include "telemetry-formats.h"
#include "radio-packet-layout.h"

#include "variant.h"
#include "board.h"
#include "gnss-xa1110.h"
#include "kx134-1211.h"
#include "mpu9250.h"
#include "ms5611.h"


#define STATUS_LOG_PERIOD           1000
#define STATUS_TRANSMIT_PERIOD      5000
#define ALTITUDE_TRANSMIT_PERIOD    1000
#define GNSS_LOC_TRANSMIT_PERIOD    5000
#define GNSS_META_TRANSMIT_PERIOD   30000
#define GNSS_META_LOG_PERIOD        1000
#define IMU_TRANSMIT_PERIOD         2500



#ifdef ENABLE_ALTIMETER
/**
 *  Marshal payload for MS5611 altitude data.
 *
 *  @param pl_buf Buffer where payload should be marshaled
 *  @param arg Pointer to MS5611 driver instance
 */
static void telemetry_marshal_ms5611_altitude(uint32_t *pl_buf, void *arg);
#endif

#ifdef ENABLE_GNSS
/**
 *  Marshal payload for GNSS location data.
 *
 *  @param pl_buf Buffer where payload should be marshaled
 *  @param arg Pointer to gnss driver instance
 */
static void telemetry_marshal_gnss_loc(uint32_t *pl_buf, void *arg);
/**
 *  Marshal payload for GNSS metadata.
 *
 *  @param pl_buf Buffer where payload should be marshaled
 *  @param arg Pointer to gnss driver instance
 */
static void telemetry_marshal_gnss_metadata(uint32_t *pl_buf, void *arg);
#endif

#ifdef ENABLE_IMU
/**
 *  Marshal MPU9250 acceleration data into generic acceleration data payload.
 *
 *  @param pl_buf Buffer where payload should be marshaled
 *  @param arg Pointer to IMU driver instance
 */
static void telemetry_marshal_mpu9250_acceleration(uint32_t *pl_buf, void *arg);

/**
 *  Marshal MPU9250 angular velocity data into generic angular velocity data
 *  payload.
 *
 *  @param pl_buf Buffer where payload should be marshaled
 *  @param arg Pointer to IMU driver instance
 */
static void telemetry_marshal_mpu9250_angular_velocity(uint32_t *pl_buf,
                                                       void *arg);
#endif

/**
 *  Marshal software status payload.
 *
 *  @param pl_buf Buffer where payload should be marshaled
 *  @param arg Not used
 */
static void telemetry_marshal_status(uint32_t *pl_buf, void *arg);




/**
 *  Generic function to write telemetry data to logging service and radio. This
 *  function create the required headers and called the provided marshaling
 *  function to create the payload.
 *
 *  @param inst Telemetry service instance descriptor
 *  @param log_samp Whether the sample should be logged to the SD card
 *  @param send_samp Whether the sample should be send with the radio
 *  @param transmit_period How often this type of sample is send with the radio
 *  @param pl_len The length of the payload to be posted
 *  @param marshal_func Function to marshal the payload
 *  @param marshal_func_arg Argument passed to marshaling function
 *  @param subtype Data block subtype to be used in headers
 *
 *  @return 0 if successful, a non-zero value otherwise
 */
static int telemetry_post_internal(struct telemetry_service_desc_t *const inst,
                                   uint8_t log_samp, uint8_t send_samp,
                                   uint32_t const transmit_period,
                                   size_t const pl_len,
                                   void (*const marshal_func)(uint32_t*, void*),
                                   void *const marshal_func_arg,
                                   enum radio_block_data_subtype subtype)
{
    log_samp = log_samp && (inst->logging != NULL);
    send_samp = send_samp && (inst->logging != NULL);

    if (!log_samp && !send_samp) {
        return 0;
    }

    // Calculate size of block
    uint16_t const total_bytes = LOGGING_BLOCK_HEADER_LENGTH + pl_len;

    // Get a buffer from the logging service if possible
    uint8_t *buffer;

    if (log_samp) {
        int const log_checkout_ret = log_checkout(inst->logging, &buffer,
                                                  total_bytes);
        if (log_checkout_ret != 0) {
            log_samp = 0;
        }
    }

    if (!log_samp && send_samp) {
        // Use a buffer on the stack so that we can still send data over
        // radio
        buffer = alloca(total_bytes);
    } else if (!log_samp) {
        return 1;
    }

    // Copy data into buffer
    uint32_t *const pl = (uint32_t*)__builtin_assume_aligned(buffer +
                                                LOGGING_BLOCK_HEADER_LENGTH, 4);
    marshal_func(pl, marshal_func_arg);

    // Send via radio
    if (send_samp) {
        // Create radio block header
        radio_block_marshal_header(buffer, total_bytes, 0,
                                   RADIO_DEVICE_ADDRESS_GROUND_STATION,
                                   RADIO_BLOCK_TYPE_DATA, subtype);
        // Send radio block header
        radio_send_block(inst->radio, buffer, total_bytes,
                         transmit_period, transmit_period * 2);
    }

    // Log to sd card
    if (log_samp) {
        // Create logging block header
        logging_block_marshal_header(buffer, LOGGING_BLOCK_CLASS_TELEMETRY,
                                     subtype, total_bytes);
        // Checkin buffer
        log_checkin(inst->logging, buffer);
    }

    return 0;
}







void init_telemetry_service (struct telemetry_service_desc_t *inst,
                             struct logging_desc_t *logging,
                             struct radio_transport_desc *radio)
{
    // Zero out all the pointers to sensor descriptors and last polled times
    memset(inst, 0, sizeof(struct telemetry_service_desc_t));

    inst->logging = logging;
    inst->radio = radio;
}

void telemetry_service(struct telemetry_service_desc_t *inst)
{
#ifdef ENABLE_ALTIMETER
    if (inst->ms5611_alt != NULL) {
        // Post acceleration
        uint32_t const alt_time = ms5611_get_last_reading_time(
                                                            inst->ms5611_alt);
        uint8_t const log_alt = alt_time != inst->last_ms5611_alt_log_time;
        uint8_t const send_alt = ((alt_time -
                                   inst->last_ms5611_alt_radio_time) >
                                  ALTITUDE_TRANSMIT_PERIOD);
        telemetry_post_internal(inst, log_alt, send_alt,
                                ALTITUDE_TRANSMIT_PERIOD,
                                sizeof(struct telem_altitude),
                                telemetry_marshal_ms5611_altitude,
                                inst->ms5611_alt, RADIO_DATA_BLOCK_ALTITUDE);
        inst->last_ms5611_alt_log_time = alt_time;
        if (send_alt) {
            inst->last_ms5611_alt_radio_time = alt_time;
        }
    }
#endif

#ifdef ENABLE_GNSS
    // Post GNSS information
    if (inst->gnss != NULL) {
        // Location
        uint32_t const fix_time = inst->gnss->last_fix;
        uint8_t const log_loc = fix_time != inst->last_gnss_loc_log_time;
        uint8_t const send_loc = ((fix_time - inst->last_gnss_loc_radio_time) >
                                  GNSS_LOC_TRANSMIT_PERIOD);
        telemetry_post_internal(inst, log_loc, send_loc,
                                GNSS_LOC_TRANSMIT_PERIOD,
                                sizeof(struct telem_gnss_loc),
                                telemetry_marshal_gnss_loc,
                                inst->gnss, RADIO_DATA_BLOCK_GNSS);
        inst->last_gnss_loc_log_time = fix_time;
        if (send_loc) {
            inst->last_gnss_loc_radio_time = fix_time;
        }

        // Metadata
        uint32_t const meta_time = inst->gnss->last_gsv;
        uint8_t const log_meta = ((meta_time -
                                    inst->last_gnss_meta_log_time) >
                                   GNSS_META_LOG_PERIOD);
        uint8_t const send_meta = ((meta_time -
                                    inst->last_gnss_meta_radio_time) >
                                   GNSS_META_TRANSMIT_PERIOD);

        uint8_t const num_sats = (inst->gnss->num_gps_sats_in_view +
                                  inst->gnss->num_glonass_sats_in_view);
        size_t const sat_length = (num_sats *
                                   sizeof(struct telem_gnss_meta_sat_info));

        telemetry_post_internal(inst, log_meta, send_meta,
                                GNSS_META_TRANSMIT_PERIOD,
                                sizeof(struct telem_gnss_meta) + sat_length,
                                telemetry_marshal_gnss_metadata,
                                inst->gnss, RADIO_DATA_BLOCK_GNSS_META);
        if (log_meta) {
            inst->last_gnss_meta_log_time = meta_time;
        }
        if (send_meta) {
            inst->last_gnss_meta_radio_time = meta_time;
        }

        // If we have a fix, set the flight's timestamp
        if ((inst->gnss->fix_type == GNSS_FIX_2D) ||
            (inst->gnss->fix_type == GNSS_FIX_3D)) {
            logging_set_timestamp(inst->logging, inst->gnss->utc_time);
        }
    }
#endif

#ifdef ENABLE_IMU
    if (inst->mpu9250_imu != NULL) {
        // Post acceleration
        uint32_t const imu_time = mpu9250_get_last_time(inst->mpu9250_imu);
        uint8_t const send_imu = ((imu_time - inst->last_mpu9250_radio_time) >
                                  IMU_TRANSMIT_PERIOD);
        telemetry_post_internal(inst, 0, send_imu, IMU_TRANSMIT_PERIOD,
                                sizeof(struct telem_acceleration),
                                telemetry_marshal_mpu9250_acceleration,
                                inst->mpu9250_imu,
                                RADIO_DATA_BLOCK_ACCELERATION);
        telemetry_post_internal(inst, 0, send_imu, IMU_TRANSMIT_PERIOD,
                                sizeof(struct telem_angular_velocity),
                                telemetry_marshal_mpu9250_angular_velocity,
                                inst->mpu9250_imu,
                                RADIO_DATA_BLOCK_ANGULAR_VELOCITY);
        if (send_imu) {
            inst->last_mpu9250_radio_time = imu_time;
        }
    }
#endif

    uint8_t const log_status = ((millis - inst->last_status_log_time) >
                                STATUS_LOG_PERIOD);
    uint8_t const send_status = ((millis - inst->last_status_radio_time) >
                                 STATUS_TRANSMIT_PERIOD);
    telemetry_post_internal(inst, log_status, send_status,
                            STATUS_TRANSMIT_PERIOD, sizeof(struct telem_status),
                            telemetry_marshal_status, NULL,
                            RADIO_DATA_BLOCK_STATUS);
    if (log_status) {
        inst->last_status_log_time = millis;
    }

    if (send_status) {
        inst->last_status_radio_time = millis;
    }
}





int telemetry_post_msg(struct telemetry_service_desc_t *const inst,
                       const enum telemetry_msg_severity severity,
                       const char *const msg)
{
    // Calculate the number of bytes that we need
    size_t const string_len = strlen(msg);
    // Round up to make sure the message is a multiple of 4 bytes long
    size_t const msg_len = (string_len + 3) & ~0x3;
    size_t const total_bytes = msg_len + 4 + LOGGING_BLOCK_HEADER_LENGTH;

    // Get a buffer from the logging service if possible
    uint8_t *buffer;

    int log_checkout_ret = 1;
    if (inst->logging != NULL) {
        log_checkout_ret = log_checkout(inst->logging, &buffer, total_bytes);
    }

    if ((log_checkout_ret != 0) && (severity <= TELEM_MSG_SEVERITY_CRIT) &&
        (inst->radio != NULL)) {
        // Use a buffer on the stack so that we can still send message over
        // radio
        buffer = alloca(total_bytes);
    } else if (log_checkout_ret != 0) {
        return 1;
    }

    // Zero out the last word of our buffer as it may contain padding bytes
    uint32_t *last_word_p = (uint32_t*)__builtin_assume_aligned((buffer +
                                                        (total_bytes - 4)), 4);
    *last_word_p = 0;

    // Place mission time at start of buffer
    *((uint32_t*)__builtin_assume_aligned(buffer + LOGGING_BLOCK_HEADER_LENGTH,
                                          4)) = millis;

    // Copy message into buffer
    memcpy(buffer + LOGGING_BLOCK_HEADER_LENGTH + 4, msg, string_len);

    // Pad with nul characters if needed
    for (size_t i = string_len; i < msg_len; i++) {
        buffer[LOGGING_BLOCK_HEADER_LENGTH + i] = '\0';
    }

    if ((severity <= TELEM_MSG_SEVERITY_CRIT) && (inst->radio != NULL)) {
        // Create radio block header
        radio_block_marshal_header(buffer, total_bytes, 0,
                                   RADIO_DEVICE_ADDRESS_GROUND_STATION,
                                   RADIO_BLOCK_TYPE_DATA,
                                   RADIO_DATA_BLOCK_DEBUG);
        // Send radio block
        radio_send_block(inst->radio, buffer, total_bytes, 500, 0);
    }

    // Check if we only made it here by allocating on the stack
    if (log_checkout_ret != 0) {
        return 0;
    }
    // Create logging block header
    logging_block_marshal_header(buffer, LOGGING_BLOCK_CLASS_DIAG,
                                 LOGGING_DIAG_TYPE_MSG, total_bytes);

    // Checking logging buffer
    log_checkin(inst->logging, buffer);

    return 0;
}





#ifdef ENABLE_ALTIMETER
static void telemetry_marshal_ms5611_altitude(uint32_t *pl_buf, void *arg)
{
    struct ms5611_desc_t *const ms5611 = (struct ms5611_desc_t*)arg;
    struct telem_altitude *const pl = (struct telem_altitude*)pl_buf;

    pl->measurement_time = ms5611_get_last_reading_time(ms5611);
    pl->pressure = ms5611_get_pressure(ms5611);
    pl->temperature = ms5611_get_temperature(ms5611) * 10;
    pl->altitude = (int32_t)(ms5611_get_altitude(ms5611) * 1000.0f);
}
#endif

#ifdef ENABLE_GNSS
static void telemetry_marshal_gnss_loc(uint32_t *pl_buf, void *arg)
{
    const struct gnss *const gnss = (const struct gnss*)arg;
    struct telem_gnss_loc *const pl = (struct telem_gnss_loc*)pl_buf;

    pl->fix_time = gnss->last_fix;
    pl->lat = gnss->latitude;
    pl->lon = gnss->longitude;
    pl->utc_time = gnss->utc_time;
    pl->altitude = gnss->altitude;
    pl->speed = gnss->speed;
    pl->course = gnss->course;
    pl->pdop = gnss->pdop;
    pl->hdop = gnss->hdop;
    pl->vdop = gnss->vdop;
    pl->sats = gnss->num_sats_in_use;
    pl->type = gnss->fix_type;
}

static void telemetry_marshal_gnss_metadata(uint32_t *pl_buf, void *arg)
{
    const struct gnss *const gnss = (const struct gnss*)arg;
    struct telem_gnss_meta *const pl = (struct telem_gnss_meta*)pl_buf;

    pl->mission_time = gnss->last_meta;
    pl->gps_sats_in_use = gnss->gps_sats_in_use;
    pl->glonass_sats_in_use = gnss->glonass_sats_in_use;

    uint8_t const num_sats = (gnss->num_gps_sats_in_view +
                              gnss->num_glonass_sats_in_view);

    for (uint8_t i = 0; i < gnss->num_gps_sats_in_view; i++) {
        pl->sats[i].elevation = gnss->in_view_gps_satellites[i].elevation;
        pl->sats[i].snr = gnss->in_view_gps_satellites[i].snr;
        pl->sats[i].sat_id = gnss->in_view_gps_satellites[i].prn;
        pl->sats[i].azimuth = gnss->in_view_gps_satellites[i].azimuth;
        pl->sats[i].type = TELEM_GNSS_META_SAT_GPS;
    }

    for (uint8_t i = gnss->num_gps_sats_in_view; i < num_sats; i++) {
        uint8_t const gs = i - gnss->num_gps_sats_in_view;
        pl->sats[i].elevation = gnss->in_view_glonass_satellites[gs].elevation;
        pl->sats[i].snr = gnss->in_view_glonass_satellites[gs].snr;
        pl->sats[i].sat_id = gnss->in_view_glonass_satellites[gs].sat_id;
        pl->sats[i].azimuth = gnss->in_view_glonass_satellites[gs].azimuth;
        pl->sats[i].type = TELEM_GNSS_META_SAT_GLONASS;
    }
}
#endif

#ifdef ENABLE_IMU
static void telemetry_marshal_mpu9250_acceleration(uint32_t *pl_buf, void *arg)
{
    const struct mpu9250_desc_t *const imu = (const struct mpu9250_desc_t*)arg;
    struct telem_acceleration *const pl = (struct telem_acceleration*)pl_buf;

    pl->measurement_time = mpu9250_get_last_time(imu);
    pl->fsr = mpu9250_get_accel_fsr(imu);
    pl->x = mpu9250_get_accel_x(imu);
    pl->y = mpu9250_get_accel_y(imu);
    pl->z = mpu9250_get_accel_z(imu);
}

static void telemetry_marshal_mpu9250_angular_velocity(uint32_t *pl_buf,
                                                       void *arg)
{
    const struct mpu9250_desc_t *const imu = (const struct mpu9250_desc_t*)arg;
    struct telem_angular_velocity *const pl =
                                        (struct telem_angular_velocity*)pl_buf;

    pl->measurement_time = mpu9250_get_last_time(imu);
    pl->fsr = mpu9250_get_gyro_fsr(imu);
    pl->x = mpu9250_get_gyro_x(imu);
    pl->y = mpu9250_get_gyro_y(imu);
    pl->z = mpu9250_get_gyro_z(imu);
}
#endif

static void telemetry_marshal_status(uint32_t *pl_buf, void *arg)
{
    struct telem_status *const pl = (struct telem_status*)pl_buf;

    // Set the entire payload to zero to make sure that any fields which are not
    // populated will be zeroed
    memset(pl, 0, sizeof(struct telem_status));

    pl->time = millis;
#ifdef ENABLE_DEPLOYMENT
    pl->deployment_state = deployment_get_state(&deployment_g);
#endif
#ifdef ENABLE_LOGGING
    pl->sd_blocks_recorded = log_get_curr_flight_blocks(&logging_g);
    pl->sd_checkouts_missed = log_get_num_missed_checkouts(&logging_g);
    pl->sd_state = log_get_sd_funcs(&logging_g)->get_status(
                                                log_get_sd_desc(&logging_g));
#endif
#ifdef ENABLE_IMU
    if (imu_g.state < MPU9250_RUNNING) {
        pl->imu_state = TELEM_SENSOR_STATUS_INITIALIZING;
    } else if (imu_g.state < MPU9250_FAILED) {
        pl->imu_state = TELEM_SENSOR_STATUS_RUNNING;
    } else if ((imu_g.state == MPU9250_FAILED_AG_SELF_TEST) ||
               (imu_g.state == MPU9250_FAILED_MAG_SELF_TEST)) {
        pl->imu_state = TELEM_SENSOR_STATUS_SELF_TEST_FAILED;
    } else {
        pl->imu_state = TELEM_SENSOR_STATUS_FAILED;
    }
#endif
#ifdef ENABLE_ALTIMETER
    if (altimeter_g.state < MS5611_IDLE) {
        pl->altimeter_state = TELEM_SENSOR_STATUS_INITIALIZING;
    } else if (altimeter_g.state < MS5611_FAILED) {
        pl->altimeter_state = TELEM_SENSOR_STATUS_RUNNING;
    } else {
        pl->altimeter_state = TELEM_SENSOR_STATUS_FAILED;
    }
#endif
#ifdef ENABLE_KX134_1211
    if (kx134_g.state < KX134_1211_RUNNING) {
        pl->kx134_state = TELEM_SENSOR_STATUS_INITIALIZING;
    } else if (kx134_g.state < KX134_1211_FAILED) {
        pl->kx134_state = TELEM_SENSOR_STATUS_RUNNING;
    } else if (kx134_g.state == KX134_1211_FAILED_SELF_TEST) {
        pl->kx134_state = TELEM_SENSOR_STATUS_SELF_TEST_FAILED;
    } else {
        pl->kx134_state = TELEM_SENSOR_STATUS_FAILED;
    }
#endif
}


uint8_t *telemetry_post_kx134_accel(struct telemetry_service_desc_t *inst,
                                    uint32_t time, enum kx134_1211_odr odr,
                                    enum kx134_1211_range range,
                                    enum kx134_1211_low_pass_rolloff roll,
                                    enum kx134_1211_resolution res,
                                    uint16_t sensor_payload_length)
{
    if (inst->logging == NULL) {
        return NULL;
    }

    // Calculate the number of bytes that we need
    size_t const subhead_size = offsetof(struct telem_kx124_accel_pl_head,
                                         data);
    uint16_t payload_bytes;
    uint16_t total_bytes;

    int ret = __builtin_add_overflow(sensor_payload_length, subhead_size + 3,
                                     &payload_bytes);
    if (ret) {
        return NULL;
    }
    // Make sure that payload_bytes is a multiple of 4 (note that we added an
    // extra 3 bytes on above, that's part of rounding up to the nearest 4
    // bytes)
    payload_bytes &= ~0x3;

    ret = __builtin_add_overflow(payload_bytes, LOGGING_BLOCK_HEADER_LENGTH,
                                 &total_bytes);
    if (ret) {
        return NULL;
    }

    // Checkout a buffer
    uint8_t *buffer;
    ret = log_checkout(inst->logging, &buffer, total_bytes);
    if (ret != 0) {
        return NULL;
    }

    // Create the block header
    logging_block_marshal_header(buffer, LOGGING_BLOCK_CLASS_TELEMETRY,
                                 RADIO_DATA_BLOCK_KX134_1211_ACCEL,
                                 total_bytes);

    // Create payload header
    uint8_t *const pl = buffer + LOGGING_BLOCK_HEADER_LENGTH;
    struct telem_kx124_accel_pl_head *const pl_head =
            (struct telem_kx124_accel_pl_head*)__builtin_assume_aligned(pl, 4);
    pl_head->measurement_time = time;
    pl_head->odr = odr;
    pl_head->range = range;
    pl_head->roll = roll;
    pl_head->res = res;
    pl_head->padding = (payload_bytes - subhead_size) - sensor_payload_length;

    // Zero out the last word of the buffer as it could contain some padding
    uint32_t *const last_word_p =
        (uint32_t*)__builtin_assume_aligned((buffer + (total_bytes - 4)), 4);
    *last_word_p = 0;

    return pl + subhead_size;
}

int telemetry_finish_kx134_accel(struct telemetry_service_desc_t *inst,
                                 uint8_t *buffer)
{
    if (inst->logging == NULL) {
        return 1;
    }

    return log_checkin(inst->logging, buffer);
}



uint8_t *telemetry_post_mpu9250_imu(struct telemetry_service_desc_t *inst,
                                    uint32_t time, uint8_t ag_sr_div,
                                    enum ak8963_odr mag_odr,
                                    enum mpu9250_accel_fsr accel_fsr,
                                    enum mpu9250_gyro_fsr gyro_fsr,
                                    enum mpu9250_accel_bw accel_bw,
                                    enum mpu9250_gyro_bw gyro_bw,
                                    uint16_t sensor_payload_length)
{
    if (inst->logging == NULL) {
        return NULL;
    }

    // Calculate the number of bytes that we need
    size_t const subhead_size = offsetof(struct telem_mpu9250_imu_pl_head,
                                         data);
    uint16_t payload_bytes;
    uint16_t total_bytes;

    int ret = __builtin_add_overflow(sensor_payload_length, subhead_size + 3,
                                     &payload_bytes);
    if (ret) {
        return NULL;
    }
    // Make sure that payload_bytes is a multiple of 4 (note that we added an
    // extra 3 bytes on above, that's part of rounding up to the nearest 4
    // bytes)
    payload_bytes &= ~0x3;

    ret = __builtin_add_overflow(payload_bytes, LOGGING_BLOCK_HEADER_LENGTH,
                                 &total_bytes);
    if (ret) {
        return NULL;
    }

    // Checkout a buffer
    uint8_t *buffer;
    ret = log_checkout(inst->logging, &buffer, total_bytes);
    if (ret != 0) {
        return NULL;
    }

    // Create the block header
    logging_block_marshal_header(buffer, LOGGING_BLOCK_CLASS_TELEMETRY,
                                 RADIO_DATA_BLOCK_MPU9250_IMU, total_bytes);

    // Create payload header
    uint8_t *const pl = buffer + LOGGING_BLOCK_HEADER_LENGTH;
    struct telem_mpu9250_imu_pl_head *const pl_head =
            (struct telem_mpu9250_imu_pl_head*)__builtin_assume_aligned(pl, 4);
    pl_head->measurement_time = time;
    pl_head->ag_sr_div = ag_sr_div;
    pl_head->mag_odr = mag_odr;
    pl_head->accel_fsr = accel_fsr;
    pl_head->gyro_fsr = gyro_fsr;
    pl_head->accel_bw = accel_bw;
    pl_head->gyro_bw = gyro_bw;

    // Zero out the last word of the buffer as it could contain some padding
    uint32_t *const last_word_p =
        (uint32_t*)__builtin_assume_aligned((buffer + (total_bytes - 4)), 4);
    *last_word_p = 0;

    return pl + subhead_size;
}

int telemetry_finish_mpu9250_imu(struct telemetry_service_desc_t *inst,
                                 uint8_t *buffer)
{
    if (inst->logging == NULL) {
        return 1;
    }

    return log_checkin(inst->logging, buffer);
}
