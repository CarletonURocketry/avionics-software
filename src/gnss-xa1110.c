/**
 * @file gnss-xa1110.c
 * @desc XA1110 GNSS Receiver Driver
 * @author Tauheed ELahee
 * @date 2019-04-05
 * @last-edit 2019-05-21
 */

#include "gnss-xa1110.h"

struct {
  uint32_t gnss_xa1110_system_time;
  uint32_t gnss_xa1110_utc_time;
  int32_t gnss_xa1110_latitude;
  int32_t gnss_xa1110_longitude;
  int16_t gnss_xa1110_speed;
  int16_t gnss_xa1110_course;
  uint8_t gnss_xa1110_status;
} gnss_xa1110_descriptor;

struct console_desc_t* console_g;

static uint8_t verify_checksum(char *str)
{
    uint8_t checksum = str[1];
    uint8_t i;
    
    for (i = 2; (str[i] != '*') && (str[i] != '\0'); i++) {
        checksum ^= str[i];
    }
    
    if (str[i] != '*') {
        // No cheksum present
        return 0;
    }
    
    char* end;
    uint8_t chk = strtoul(str + i + 1, &end, 16);
    return (*end == '\0') ? (checksum == chk) : 0;
}

void gnss_line_callback(char* line, struct console_desc_t* console, void* context)
{
    // parsing part update the gnss_descriptor struct
}

void gnss_init_callback(struct console_desc_t* console, void* context) {
    // make gnss_init callback function
}

void init_gnss_xa1110(struct console_desc_t* console)
{
    console_g = console;
    console_set_line_callback (console_g, gnss_line_callback, NULL);
    console_set_init_callback(console_g, gnss_init_callback, NULL);
    // put console_set_init_callback here
}

// Do not need gnss_service

uint32_t gnss_xa1110_retrieve_sytem_time(void)
{
  return gnss_xa1110_descriptor.gnss_xa1110_system_time;
}

uint32_t gnss_xa1110_retrieve_utc_time(void)
{
  return gnss_xa1110_descriptor.gnss_xa1110_utc_time;
}

uint32_t gnss_xa1110_retrieve_latitude(void)
{
  return gnss_xa1110_descriptor.gnss_xa1110_latitude;
}

uint32_t gnss_xa1110_retrieve_longitude(void)
{
  return gnss_xa1110_descriptor.gnss_xa1110_longitude;
}

uint16_t gnss_xa1110_retrieve_speed(void)
{
  return gnss_xa1110_descriptor.gnss_xa1110_speed;
}

uint16_t gnss_xa1110_retrieve_course(void)
{
  return gnss_xa1110_descriptor.gnss_xa1110_course;
}

uint8_t gnss_xa1110_retrieve_status(void)
{
  return gnss_xa1110_descriptor.gnss_xa1110_status;
}
