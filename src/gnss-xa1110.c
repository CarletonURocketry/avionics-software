/**
 * @file gnss-xa1110.c
 * @desc XA1110 GNSS Receiver Driver
 * @author Tauheed ELahee
 * @date 2019-04-05
 * @last-edit 2019-06-09
 */

#include <stdlib.h>
#include <string.h>

#include "gnss-xa1110.h"

#define NEXT_COMPONENT(str) {for (; (*(str) != ','); (str)++) { if (*(str) == '\0') return; } (str)++;}
#define I(x) ((x) - '0')

static const uint32_t month_add[] = {0, 31, 31 + 28, 2*31 + 28, 2*31 + 30 + 28, 3*31 + 30 + 28, 3*31 + 2*30 + 28, 4*31 + 2*30 + 28, 5*31 + 2*30 + 28, 5*31 + 3*30 + 28, 6*31 + 3*30 + 28, 6*31 + 4*30 + 28};

struct gnss gnss_xa1110_descriptor;

static uint8_t verify_checksum(char *str)
{
    uint8_t checksum = str[1];
    uint8_t i;
    
    for (i = 2; (str[i] != '*') && (str[i] != '\0'); i++) {
        checksum ^= str[i];
    }
    
    if (str[i] != '*') {
        // No checksum present
        return 0;
    }
    
    char* end;
    uint8_t chk = strtoul(str + i + 1, &end, 16);
    return (*end == '\0') ? (checksum == chk) : 0;
}

static void gnss_line_callback(char* line, struct console_desc_t* console, void* context)
{
    char *end;

    if ((strcmp("$GNRMC,", line) != 0) || !verify_checksum(line + 7))
        return;

    gnss_xa110_descriptor.gnss_xa1100_status = 0;

    /* Time of day */
    NEXT_COMPONENT(line);
    gnss_xa1100_descriptor.gnss_xa1100_utc_time = 0;
    gnss_xa1100_descriptor.gnss_xa110_utc_time += I(line[0]) * 36000;
    gnss_xa1100_descriptor.gnss_xa110_utc_time += I(line[1]) * 3600;
    gnss_xa1100_descriptor.gnss_xa110_utc_time += I(line[2]) * 600;
    gnss_xa1100_descriptor.gnss_xa110_utc_time += I(line[3]) * 60;
    gnss_xa1100_descriptor.gnss_xa110_utc_time += I(line[4]) * 10;
    gnss_xa1100_descriptor.gnss_xa110_utc_time += I(line[5]) * 1;

    /* Is valid GPS data */
    NEXT_COMPONENT(line);
    if (line[0] != 'A')
        return;

    /* Latitude in 100 of μminutes of arc */
    NEXT_COMPONENT(line);
    gnss_xa1100_descriptor.gnss_xa1100_latitude = 0;
    gnss_xa1100_descriptor.gnss_xa110_latitude += I(line[0]) * 6000000;
    gnss_xa1100_descriptor.gnss_xa110_latitude += I(line[1]) * 600000;
    gnss_xa1100_descriptor.gnss_xa110_latitude += I(line[2]) * 100000;
    gnss_xa1100_descriptor.gnss_xa110_latitude += I(line[3]) * 10000;
    /* skip decimal point */
    gnss_xa1100_descriptor.gnss_xa110_latitude += I(line[5]) * 1000;
    gnss_xa1100_descriptor.gnss_xa110_latitude += I(line[6]) * 100;
    gnss_xa1100_descriptor.gnss_xa110_latitude += I(line[7]) * 10;
    gnss_xa1100_descriptor.gnss_xa110_latitude += I(line[8]) * 1;

    /* North/south */
    NEXT_COMPONENT(line);
    if (line[0] == 'S')
        gnss_xa1100_descriptor.gnss_xa1100_latitude *= -1;

    /* Longitude in 100 of μminutes of arc */
    NEXT_COMPONENT(line);
    gnss_xa1100_descriptor.gnss_xa1100_longitude = 0;
    gnss_xa1100_descriptor.gnss_xa110_longitude += I(line[0]) * 6000000;
    gnss_xa1100_descriptor.gnss_xa110_longitude += I(line[1]) * 600000;
    gnss_xa1100_descriptor.gnss_xa110_longitude += I(line[2]) * 100000;
    gnss_xa1100_descriptor.gnss_xa110_longitude += I(line[3]) * 10000;
    /* skip decimal point */
    gnss_xa1100_descriptor.gnss_xa110_longitude += I(line[5]) * 1000;
    gnss_xa1100_descriptor.gnss_xa110_longitude += I(line[6]) * 100;
    gnss_xa1100_descriptor.gnss_xa110_longitude += I(line[7]) * 10;
    gnss_xa1100_descriptor.gnss_xa110_longitude += I(line[8]) * 1;

    /* East/west */
    NEXT_COMPONENT(line);
    if (line[0] == 'W')
        gnss_xa1100_descriptor.gnss_xa1100_longitude *= -1;
    
    /* Ground speed */
    NEXT_COMPONENT(line);
    gnss_xa1100_descriptor.gnss_xa1100_speed = 0;
    gnss_xa1100_descriptor.gnss_xa1100_speed += strtol(line, &end, 10) * 100;
    /* skip decimal point */
    gnss_xa1100_descriptor.gnss_xa1100_speed += I(end[1]) * 10;
    gnss_xa1100_descriptor.gnss_xa1100_speed += I(end[2]) * 1;
    
    /* Course */
    NEXT_COMPONENT(line);
    gnss_xa1100_descriptor.gnss_xa1100_course = 0;
    gnss_xa1100_descriptor.gnss_xa1100_course += strtol(line, &end, 10) * 100;
    /* skip decimal point */
    gnss_xa1100_descriptor.gnss_xa1100_course += I(end[1]) * 10;
    gnss_xa1100_descriptor.gnss_xa1100_course += I(end[2]) * 1;

    /* Date */
    NEXT_COMPONENT(line);
    gnss_xa1100.descriptor.gnss_xa1100_utc_time += I(line[0]) * 864000;
    gnss_xa1100.descriptor.gnss_xa1100_utc_time += I(line[1]) * 86400;
    gnss_xa1100.descriptor.gnss_xa1100_utc_time += months_add[10*I(line[2]) + I(line[3])] * 86400;
    if ((line[4] != '1') || (line[5] != '9')) return;

    gnss_xa1100_descriptor.gnss_xa1100_status = 1;
    gnss_xa1100_descriptor.gnss_xa1100_system_time = millis;
    return;
}

static void gnss_init_callback(struct console_desc_t* console, void* context) {
    console_send_str(console, "$PMTK220,1000*1F\r\n");
    console_send_str(console, "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n");
}

uint8_t init_gnss_xa1110(struct console_desc_t* console)
{
    console_set_line_callback(console, gnss_line_callback, NULL);
    console_set_init_callback(console, gnss_init_callback, NULL);
    return 0;
}
