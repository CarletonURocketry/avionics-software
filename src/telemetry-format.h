/**
 * @file telemetry-format.h
 * @desc Structs used to marshal telemetry packets
 * @author Samuel Dewan
 * @date 2019-06-11
 * Last Author:
 * Last Edited On:
 */

#ifndef telemetry_format_h
#define telemetry_format_h

// Ignore warnings in this file about inefficient alignment
#pragma GCC diagnostic ignored "-Wattributes"
#pragma GCC diagnostic ignored "-Wpacked"

struct telemetry_frame {
    uint32_t mission_time;
    
    struct {
        uint16_t parachute_deployed:1;
        uint16_t gps_data_valid:1;
        
        uint16_t RESERVED:8;
        
        uint16_t state_standby:1;
        uint16_t state_pre_flight:1;
        uint16_t state_powered_ascent:1;
        uint16_t state_coasting_ascent:1;
        uint16_t state_descents:1;
        uint16_t state_recovery:1;
    } flags;
    
    uint16_t adc_data[8];
    
    uint16_t accel_x;
    uint16_t accel_y;
    uint16_t accel_z;
    uint16_t accel_temp;
    
    uint32_t altimeter_temp;
    float altimeter_altitude;
    
    uint32_t gps_utc_time;
    int32_t gps_latitude;
    int32_t gps_longitude;
    int16_t gps_speed;
    int16_t gps_course;
    int32_t gps_altitude;
} __attribute__ ((packed));



struct telemetry_api_frame {
    uint8_t start_delimiter;            // 0x52
    uint8_t payload_type;
    
    uint16_t length;
    
    struct telemetry_frame payload;
    
    uint8_t end_delimiter;              // 0xCC
} __attribute__ ((packed));

// Stop ignoring warnings about inefficient alignment
#pragma GCC diagnostic pop

#endif /* telemetry_format_h */
