/**
 * @file telemetry-formats.h
 * @desc Definitions for data block payloads
 * @author Samuel Dewan
 * @date 2021-08-20
 * Last Author:
 * Last Edited On:
 */

#ifndef telemetry_formats_h
#define telemetry_formats_h

#include <stdint.h>

//
//
//  Status
//
//
enum telem_sensor_status {
    TELEM_SENSOR_STATUS_NONE = 0x0,
    TELEM_SENSOR_STATUS_INITIALIZING = 0x1,
    TELEM_SENSOR_STATUS_RUNNING = 0x2,
    TELEM_SENSOR_STATUS_SELF_TEST_FAILED = 0x3,
    TELEM_SENSOR_STATUS_FAILED = 0x4
};

struct telem_status {
    uint32_t time;
    uint32_t RESERVED:16;
    uint32_t kx134_state:3;
    uint32_t altimeter_state:3;
    uint32_t imu_state:3;
    uint32_t sd_state:3;
    uint32_t deployment_state:4;
    uint32_t sd_blocks_recorded;
    uint32_t sd_checkouts_missed;
};

//
//
//  Altitude
//
//
struct telem_altitude {
    uint32_t measurement_time;
    int32_t pressure;
    int32_t temperature;
    int32_t altitude;
};

//
//
//  Acceleration
//
//
struct telem_acceleration {
    uint32_t measurement_time;
    uint8_t fsr;
    uint8_t RESERVED;
    uint16_t x;
    uint16_t y;
    uint16_t z;
};

//
//
//  Angular Velocity
//
//
struct telem_angular_velocity {
    uint32_t measurement_time;
    uint16_t fsr;
    uint16_t x;
    uint16_t y;
    uint16_t z;
};

//
//
//  GNSS
//
//
struct telem_gnss_loc {
    uint32_t fix_time;
    int32_t lat;
    int32_t lon;
    uint32_t utc_time;
    int32_t altitude;
    int16_t speed;
    int16_t course;
    uint16_t pdop;
    uint16_t hdop;
    uint16_t vdop;
    uint8_t sats;
    uint8_t type:2;
};


enum telem_gnss_meta_sat_type {
    TELEM_GNSS_META_SAT_GPS = 0,
    TELEM_GNSS_META_SAT_GLONASS = 1
};

struct telem_gnss_meta {
    uint32_t mission_time;
    uint32_t gps_sats_in_use;
    uint32_t glonass_sats_in_use;
    struct telem_gnss_meta_sat_info {
        uint32_t elevation:8;
        uint32_t snr:8;
        uint32_t sat_id:5;
        uint32_t azimuth:9;
        uint32_t RESERVED:1;
        uint32_t type:1;
    } sats[];
};

//
//
//  MPU9250 IMU
//
//
struct telem_mpu9250_imu_pl_head {
    uint32_t measurement_time;
    uint32_t ag_sr_div:8;
    uint32_t mag_odr:1;
    uint32_t accel_fsr:2;
    uint32_t gyro_fsr:2;
    uint32_t accel_bw:3;
    uint32_t gyro_bw:3;
    uint32_t RESERVED:13;
    uint8_t data[] __attribute__((aligned(2)));
};

//
//
//  KX134-1211 Accelerometer
//
//
struct telem_kx124_accel_pl_head {
    uint32_t measurement_time;
    uint16_t odr:4;
    uint16_t range:2;
    uint16_t roll:1;
    uint16_t res:1;
    uint16_t RESERVED:6;
    uint16_t padding:2;
    uint8_t data[] __attribute__((aligned(2)));
};

#endif /* telemetry_formats_h */
