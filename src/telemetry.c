//
//  telemetry.c
//  index
//
//  Created by Samuel Dewan on 2019-06-11.
//  Copyright © 2019 Samuel Dewan. All rights reserved.
//

#include "telemetry.h"

#include "telemetry-format.h"




static struct rn2483_desc_t *telemetry_radio_g;

static struct ms5611_desc_t *telemetry_altimeter_g;

static uint32_t rate_g;
static uint32_t last_time_g;

uint8_t telemetry_paused = 0;

static struct telemetry_api_frame packet_g;

static uint8_t send_transaction;
static uint8_t send_in_progress;


void init_telemetry_service (struct rn2483_desc_t *radio,
                             struct ms5611_desc_t *altimeter,
                             uint32_t telemetry_rate)
{
    telemetry_radio_g = radio;
    telemetry_altimeter_g = altimeter;
    rate_g = telemetry_rate;
    
    packet_g.start_delimiter = 0x52;
    packet_g.payload_type = 0;
    packet_g.length = sizeof packet_g.payload;
    packet_g.end_delimiter = 0xcc;
}

void telemetry_service (void)
{
    if (send_in_progress) {
        if (rn2483_get_send_state(telemetry_radio_g, send_transaction) ==
                RN2483_SEND_TRANS_PENDING) {
            return;
        }
        rn2483_clear_send_transaction(telemetry_radio_g, send_transaction);
        send_in_progress = 0;
    }
    
    if (((millis - last_time_g) >= rate_g) && !telemetry_paused) {
        last_time_g = millis;
        
        packet_g.payload.mission_time = millis;
        packet_g.payload.altimeter_temp = ms5611_get_temperature(telemetry_altimeter_g);
        packet_g.payload.altimeter_altitude = ms5611_get_altitude(telemetry_altimeter_g);
        
        packet_g.payload.gps_utc_time = gnss_xa1110_descriptor.utc_time;
        packet_g.payload.gps_latitude = gnss_xa1110_descriptor.latitude;
        packet_g.payload.gps_longitude = gnss_xa1110_descriptor.longitude;
        packet_g.payload.gps_altitude = gnss_xa1110_descriptor.altitude;
        packet_g.payload.gps_speed = gnss_xa1110_descriptor.speed;
        packet_g.payload.gps_course = gnss_xa1110_descriptor.course;
        packet_g.payload.flags.gps_data_valid = gnss_xa1110_descriptor.last_fix != 0;
        
        rn2483_send(telemetry_radio_g, (uint8_t*)(&packet_g), sizeof packet_g,
                    &send_transaction);
    }
}
