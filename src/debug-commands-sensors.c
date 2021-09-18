/**
 * @file debug-commands-sensors.c
 * @desc Commands for debugging CLI
 * @author Samuel Dewan
 * @date 2020-08-11
 * Last Author:
 * Last Edited On:
 */

#include "debug-commands-sensors.h"

#include "debug-commands.h"

#include "variant.h"
#include "board.h"
#include "wdt.h"
#include "sercom-i2c.h"
#include "mcp23s17-registers.h"
#include "gnss-xa1110.h"
#include "ms5611.h"
#include "kx134-1211.h"

// MARK: Altimeter PROM

void debug_alt_prom (uint8_t argc, char **argv, struct console_desc_t *console)
{
    uint8_t i2c_t;
    uint16_t data;

    char str[6];

    // 0: Factory data and setup
    uint8_t cmd = 0b10100000;
    sercom_i2c_start_generic(&i2c0_g, &i2c_t, 0b1110110, &cmd, 1, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c0_g, i2c_t)) {
        sercom_i2c_service(&i2c0_g);
        wdt_pat();
    }
    console_send_str(console, "0: 0x");
    utoa(__builtin_bswap16(data), str, 16);
    console_send_str(console, str);
    console_send_str(console, " (Factory data and setup)\n");
    sercom_i2c_clear_transaction(&i2c0_g, i2c_t);

    // C1: Pressure Sensitivity
    //sercom_i2c_start_reg_read(&i2c0_g, &i2c_t, 0b1110110, 0b10100100, (uint8_t*)&data, 2);
    cmd = 0b10100010;
    sercom_i2c_start_generic(&i2c0_g, &i2c_t, 0b1110110, &cmd, 1, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c0_g, i2c_t)) {
        sercom_i2c_service(&i2c0_g);
        wdt_pat();
    }
    console_send_str(console, "C1: ");
    utoa(__builtin_bswap16(data), str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Pressure sensitivity)\n");
    sercom_i2c_clear_transaction(&i2c0_g, i2c_t);

    // C2: Pressure offset
    sercom_i2c_start_reg_read(&i2c0_g, &i2c_t, 0b1110110, 0b10100100, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c0_g, i2c_t)) {
        sercom_i2c_service(&i2c0_g);
        wdt_pat();
    }
    console_send_str(console, "C2: ");
    utoa(__builtin_bswap16(data), str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Pressure offset)\n");
    sercom_i2c_clear_transaction(&i2c0_g, i2c_t);

    // C3: Temperature coefficient of pressure sensitivity
    sercom_i2c_start_reg_read(&i2c0_g, &i2c_t, 0b1110110, 0b10100110, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c0_g, i2c_t)) {
        sercom_i2c_service(&i2c0_g);
        wdt_pat();
    }
    console_send_str(console, "C3: ");
    utoa(__builtin_bswap16(data), str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Temperature coefficient of pressure sensitivity)\n");
    sercom_i2c_clear_transaction(&i2c0_g, i2c_t);

    // C4: Temperature coefficient of pressure offset
    sercom_i2c_start_reg_read(&i2c0_g, &i2c_t, 0b1110110, 0b10101000, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c0_g, i2c_t)) {
        sercom_i2c_service(&i2c0_g);
        wdt_pat();
    }
    console_send_str(console, "C4: ");
    utoa(__builtin_bswap16(data), str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Temperature coefficient of pressure offset)\n");
    sercom_i2c_clear_transaction(&i2c0_g, i2c_t);

    // C5: Reference temperature
    sercom_i2c_start_reg_read(&i2c0_g, &i2c_t, 0b1110110, 0b10101010, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c0_g, i2c_t)) {
        sercom_i2c_service(&i2c0_g);
        wdt_pat();
    }
    console_send_str(console, "C5: ");
    utoa(__builtin_bswap16(data), str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Reference temperature)\n");
    sercom_i2c_clear_transaction(&i2c0_g, i2c_t);

    // C6: Temperature coefficient of the temperature
    sercom_i2c_start_reg_read(&i2c0_g, &i2c_t, 0b1110110, 0b10101100, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c0_g, i2c_t)) {
        sercom_i2c_service(&i2c0_g);
        wdt_pat();
    }
    console_send_str(console, "C6: ");
    utoa(__builtin_bswap16(data), str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Temperature coefficient of the temperature)\n");
    sercom_i2c_clear_transaction(&i2c0_g, i2c_t);

    // 7: Serial code and CRC
    cmd = 0b10100111;
    sercom_i2c_start_generic(&i2c0_g, &i2c_t, 0b1110110, &cmd, 1, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c0_g, i2c_t)) {
        sercom_i2c_service(&i2c0_g);
        wdt_pat();
    }
    console_send_str(console, "7: 0x");
    utoa(__builtin_bswap16(data), str, 16);
    console_send_str(console, str);
    console_send_str(console, " (Serial code and CRC)\n");
    sercom_i2c_clear_transaction(&i2c0_g, i2c_t);
}

// MARK: IMU Who Am I

void debug_imu_wai (uint8_t argc, char **argv, struct console_desc_t *console)
{
    uint8_t i2c_t;
    uint8_t data;
    char str[9];

    // Who Am I
    sercom_i2c_start_reg_read(&i2c0_g, &i2c_t, 0b1101000, 0x75, &data, 2);
    while (!sercom_i2c_transaction_done(&i2c0_g, i2c_t)) {
        sercom_i2c_service(&i2c0_g);
        wdt_pat();
    }
    console_send_str(console, "Who Am I: 0x");
    utoa(data, str, 16);
    console_send_str(console, str);
    console_send_str(console, " (Gyroscope and Accelerometer)\n");
    sercom_i2c_clear_transaction(&i2c0_g, i2c_t);
}

// MARK: Altimeter

void debug_alt (uint8_t argc, char **argv, struct console_desc_t *console)
{
#ifndef ENABLE_ALTIMETER
    console_send_str(console, "Altimeter is not enabled in compile time "
                     "configuration.\n");
    return;
#else
    char str[16];

    console_send_str(console, "PROM Values:\n");
    // C1: Pressure Sensitivity
    console_send_str(console, "C1: ");
    utoa(altimeter_g.prom_values[0], str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Pressure sensitivity)\n");

    // C2: Pressure offset
    console_send_str(console, "C2: ");
    utoa(altimeter_g.prom_values[1], str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Pressure offset)\n");

    // C3: Temperature coefficient of pressure sensitivity
    console_send_str(console, "C3: ");
    utoa(altimeter_g.prom_values[2], str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Temperature coefficient of pressure sensitivity)\n");

    wdt_pat();

    // C4: Temperature coefficient of pressure offset
    console_send_str(console, "C4: ");
    utoa(altimeter_g.prom_values[3], str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Temperature coefficient of pressure offset)\n");

    // C5: Reference temperature
    console_send_str(console, "C5: ");
    utoa(altimeter_g.prom_values[4], str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Reference temperature)\n");

    // C6: Temperature coefficient of the temperature
    console_send_str(console, "C6: ");
    utoa(altimeter_g.prom_values[5], str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Temperature coefficient of the temperature)\n");

    wdt_pat();

    // Last reading time
    uint32_t last_reading_time = ms5611_get_last_reading_time(&altimeter_g);
    console_send_str(console, "\nLast reading at ");
    utoa(last_reading_time, str, 10);
    console_send_str(console, str);
    console_send_str(console, " (");
    utoa(millis - last_reading_time, str, 10);
    console_send_str(console, str);
    console_send_str(console, "  milliseconds ago)\n");

    // Pressure
    console_send_str(console, "Pressure: ");
    debug_print_fixed_point(console, altimeter_g.pressure, 2);
    console_send_str(console, " mbar (");
    utoa(altimeter_g.d1, str, 10);
    console_send_str(console, str);
    console_send_str(console, ", p0 = ");
    debug_print_fixed_point(console, (int32_t)(altimeter_g.p0 * 100), 2);

    wdt_pat();

    // Temperature
    console_send_str(console, " mbar)\nTemperature: ");
    debug_print_fixed_point(console, altimeter_g.temperature, 2);
    console_send_str(console, " C (");
    utoa(altimeter_g.d2, str, 10);
    console_send_str(console, str);

    // Altitude
    int32_t altitude = (int32_t)(altimeter_g.altitude * 100);
    console_send_str(console, ")\nAltitude: ");
    debug_print_fixed_point(console, altitude, 2);
    console_send_str(console, " m\n");
#endif
}

// MARK: Alt Tare Now

void debug_alt_tare_now (uint8_t argc, char **argv,
                         struct console_desc_t *console)
{
#ifndef ENABLE_ALTIMETER
    console_send_str(console, "Altimeter is not enabled in compile time "
                     "configuration.\n");
    return;
#else
    ms5611_tare_now(&altimeter_g);
#endif
}

// MARK: Alt Tare Next

void debug_alt_tare_next (uint8_t argc, char **argv,
                          struct console_desc_t *console)
{
#ifndef ENABLE_ALTIMETER
    console_send_str(console, "Altimeter is not enabled in compile time "
                     "configuration.\n");
    return;
#else
    ms5611_tare_next(&altimeter_g);
#endif
}

// MARK: GNSS

struct xtm
{
    uint32_t year;
    int8_t mon;
    int8_t day;
    int8_t hour;
    int8_t min;
    int8_t sec;
};

#define YEAR_TO_DAYS(y) ((y)*365 + (y)/4 - (y)/100 + (y)/400)

/**
 *  Convert unix time to time struct.
 *
 *  From: https://stackoverflow.com/a/1275638/10914765
 *
 *  @param unixtime Unix time to be converted
 *  @param tm The structure to store the result in
 */
static void untime(uint32_t unixtime, struct xtm *tm)
{
    /* First take out the hour/minutes/seconds - this part is easy. */

    tm->sec = unixtime % 60;
    unixtime /= 60;

    tm->min = unixtime % 60;
    unixtime /= 60;

    tm->hour = unixtime % 24;
    unixtime /= 24;

    /* unixtime is now days since 01/01/1970 UTC
     * Rebaseline to the Common Era
     */

    unixtime += 719499;

    /* Roll forward looking for the year.  This could be done more efficiently
     * but this will do.  We have to start at 1969 because the year we calculate
     * here runs from March - so January and February 1970 will come out as 1969
     * here.
     */
    for (tm->year = 1969; unixtime > YEAR_TO_DAYS(tm->year + 1) + 30;
         tm->year++);

    /* OK we have our "year", so subtract off the days accounted for by full
     * years.
     */
    unixtime -= YEAR_TO_DAYS(tm->year);

    /* unixtime is now number of days we are into the year (remembering that
     * March 1 is the first day of the "year" still).
     */

    /* Roll forward looking for the month.  1 = March through to 12 =
     * February.
     */
    for (tm->mon = 1; (tm->mon < 12 && unixtime >
                       (uint32_t)(367 * (tm->mon + 1) / 12)); tm->mon++);

    /* Subtract off the days accounted for by full months */
    unixtime -= 367 * tm->mon / 12;

    /* unixtime is now number of days we are into the month */

    /* Adjust the month/year so that 1 = January, and years start where we
     * usually expect them to.
     */
    tm->mon += 2;
    if (tm->mon > 12) {
        tm->mon -= 12;
        tm->year++;
    }

    tm->day = unixtime;
}

/**
 *  Print a time structure.
 *
 *  @param console The console on which the time should be printed
 *  @param time The time to be printed
 */
static void print_time (struct console_desc_t *console, struct xtm *time)
{
    char str[6];

    utoa(time->year, str, 10);
    console_send_str(console, str);
    console_send_str(console, time->mon < 9 ? "-0" : "-");
    utoa(time->mon, str, 10);
    console_send_str(console, str);
    console_send_str(console, time->day < 9 ? "-0" : "-");
    utoa(time->day, str, 10);
    console_send_str(console, str);

    console_send_str(console, " ");

    utoa(time->hour, str, 10);
    console_send_str(console, str);
    console_send_str(console, time->min < 9 ? ":0" : ":");
    utoa(time->min, str, 10);
    console_send_str(console, str);
    console_send_str(console, time->sec < 9 ? ":0" : ":");
    utoa(time->sec, str, 10);
    console_send_str(console, str);
}

void debug_gnss (uint8_t argc, char **argv, struct console_desc_t *console)
{
#ifndef ENABLE_GNSS
    console_send_str(console, "GNSS is not enabled in compile time "
                     "configuration.\n");
    return;
#endif

    char str[8];

    /* Timestamps */
    console_send_str(console, "Timestamps\n\tLast sentence at ");
    utoa(gnss_xa1110_descriptor.last_sentence, str, 10);
    console_send_str(console, str);
    console_send_str(console, " (");
    utoa(millis - gnss_xa1110_descriptor.last_sentence, str, 10);
    console_send_str(console, str);

    console_send_str(console, " milliseconds ago)\n");
    console_send_str(console, "\tLast fix at ");
    utoa(gnss_xa1110_descriptor.last_fix, str, 10);
    console_send_str(console, str);
    console_send_str(console, " (");
    utoa(millis - gnss_xa1110_descriptor.last_fix, str, 10);
    console_send_str(console, str);

    console_send_str(console, " milliseconds ago)\n\tLast metadata at ");
    utoa(gnss_xa1110_descriptor.last_meta, str, 10);
    console_send_str(console, str);
    console_send_str(console, " (");
    utoa(millis - gnss_xa1110_descriptor.last_meta, str, 10);
    console_send_str(console, str);

#ifdef GNSS_STORE_IN_VIEW_SAT_INFO
    console_send_str(console, " milliseconds ago)\n\tLast gsv at ");
    utoa(gnss_xa1110_descriptor.last_gsv, str, 10);
    console_send_str(console, str);
    console_send_str(console, " (");
    utoa(millis - gnss_xa1110_descriptor.last_gsv, str, 10);
    console_send_str(console, str);
#endif

    /* Fix */
    console_send_str(console, " milliseconds ago)\nGNSS Fix\n\t");
    int32_t lat_seconds = gnss_xa1110_descriptor.latitude;
    uint8_t lat_dir = lat_seconds >= 0;
    lat_seconds *= lat_dir ? 1 : -1;
    uint32_t lat_degrees = lat_seconds / 600000;
    lat_seconds -= lat_degrees * 600000;
    uint32_t lat_minutes = lat_seconds / 10000;
    lat_seconds -= lat_minutes * 10000;
    lat_seconds *= 6;

    int32_t lon_seconds = gnss_xa1110_descriptor.longitude;
    uint8_t lon_dir = lon_seconds >= 0;
    lon_seconds *= lon_dir ? 1 : -1;
    uint32_t lon_degrees = lon_seconds / 600000;
    lon_seconds -= lon_degrees * 600000;
    uint32_t lon_minutes = lon_seconds / 10000;
    lon_seconds -= lon_minutes * 10000;
    lon_seconds *= 6;

    utoa(lat_degrees, str, 10);
    console_send_str(console, str);
    console_send_str(console, "°");
    utoa(lat_minutes, str, 10);
    console_send_str(console, str);
    console_send_str(console, "'");
    debug_print_fixed_point(console, lat_seconds, 3);
    console_send_str(console, lat_dir ? "\"N " : "\"S ");

    utoa(lon_degrees, str, 10);
    console_send_str(console, str);
    console_send_str(console, "°");
    utoa(lon_minutes, str, 10);
    console_send_str(console, str);
    console_send_str(console, "'");
    debug_print_fixed_point(console, lon_seconds, 3);
    console_send_str(console, lon_dir ? "\"E (" : "\"W (");

    utoa(gnss_xa1110_descriptor.latitude, str, 10);
    console_send_str(console, str);
    console_send_str(console, ", ");
    utoa(gnss_xa1110_descriptor.longitude, str, 10);
    console_send_str(console, str);

    /* UTC Time */
    console_send_str(console, ")\nUTC Time\n\t");
    struct xtm time;
    untime(gnss_xa1110_descriptor.utc_time, &time);
    print_time(console, &time);

    /* Additional GNSS Data */
    console_send_str(console, "\nAdditional GNSS Data\n\tAltitude: ");
    debug_print_fixed_point(console, gnss_xa1110_descriptor.altitude, 3);
    // Speed over ground
    console_send_str(console, " m\n\tSpeed over ground: ");
    debug_print_fixed_point(console, gnss_xa1110_descriptor.speed, 2);
    // Course over ground
    console_send_str(console, " knots\n\tCourse over ground: ");
    debug_print_fixed_point(console, gnss_xa1110_descriptor.course, 2);

    /* Metadata */
    console_send_str(console, "°\nMetadata\n\tNumber of satellites in "
                     "use: ");
    utoa(gnss_xa1110_descriptor.num_sats_in_use, str, 10);
    console_send_str(console, str);
#ifdef GNSS_STORE_IN_USE_SAT_SVS
    if (gnss_xa1110_descriptor.gps_sats_in_use) {
        console_send_str(console, "\n\t\tGPS PRNs: ");
    }
    for (uint8_t i = 0; i < 32; i++) {
        if (gnss_xa1110_descriptor.gps_sats_in_use & (1 << i)) {
            utoa(i + GPS_SV_OFFSET, str, 10);
            console_send_str(console, str);
            console_send_str(console, " ");
        }
    }
    if (gnss_xa1110_descriptor.glonass_sats_in_use) {
        console_send_str(console, "\n\t\tGLONASS SVs: ");
    }
    for (uint8_t i = 0; i < 32; i++) {
        if (gnss_xa1110_descriptor.glonass_sats_in_use & (1 << i)) {
            utoa(i + GLONASS_SV_OFFSET, str, 10);
            console_send_str(console, str);
            console_send_str(console, " ");
        }
    }
#endif
    console_send_str(console, "\n\tPDOP: ");
    debug_print_fixed_point(console, gnss_xa1110_descriptor.pdop, 2);
    console_send_str(console, "\n\tHDOP: ");
    debug_print_fixed_point(console, gnss_xa1110_descriptor.hdop, 2);
    console_send_str(console, "\n\tVDOP: ");
    debug_print_fixed_point(console, gnss_xa1110_descriptor.vdop, 2);
    console_send_str(console, "\n\tAntenna: Unknown\n");
    switch (gnss_xa1110_descriptor.antenna) {
        case GNSS_ANTENNA_UNKOWN:
            console_send_str(console, "\n\tAntenna: Unknown\n");
            break;
        case GNSS_ANTENNA_INTERNAL:
            console_send_str(console, "\n\tAntenna: Internal\n");
            break;
        case GNSS_ANTENNA_EXTERNAL:
            console_send_str(console, "\n\tAntenna: External\n");
            break;
    }
    console_send_str(console, "\tFix: ");
    switch (gnss_xa1110_descriptor.fix_type) {
        case GNSS_FIX_UNKOWN:
            console_send_str(console, "Unknown\n");
            break;
        case GNSS_FIX_NOT_AVAILABLE:
            console_send_str(console, "Not Available\n");
            break;
        case GNSS_FIX_2D:
            console_send_str(console, "2D\n");
            break;
        case GNSS_FIX_3D:
            console_send_str(console, "3D\n");
            break;
    }
    console_send_str(console, "\tQuality: ");
    switch (gnss_xa1110_descriptor.fix_quality) {
        case GNSS_QUALITY_INVALID:
            console_send_str(console, "Invalid\n");
            break;
        case GNSS_QUALITY_GPS_FIX:
            console_send_str(console, "GPS Fix\n");
            break;
        case GNSS_QUALITY_DGPS_FIX:
            console_send_str(console, "Differential GPS Fix\n");
            break;
        case GNSS_QUALITY_PPS_FIX:
            console_send_str(console, "PPS Fix\n");
            break;
        case GNSS_QUALITY_REAL_TIME_KINEMATIC:
            console_send_str(console, "Real Time Kinematic\n");
            break;
        case GNSS_QUALITY_FLOAT_RTK:
            console_send_str(console, "Float RTK\n");
            break;
        case GNSS_QUALITY_DEAD_RECKONING:
            console_send_str(console, "Dead Reckoning\n");
            break;
        case GNSS_QUALITY_MANUAL_INPUT:
            console_send_str(console, "Manual Input\n");
            break;
        case GNSS_QUALITY_SIMULATION:
            console_send_str(console, "Simulation\n");
            break;
    }

#ifdef GNSS_STORE_IN_VIEW_SAT_INFO
    console_send_str(console, "\tGPS satellites in view: ");
    utoa(gnss_xa1110_descriptor.num_gps_sats_in_view, str, 10);
    console_send_str(console, str);
    console_send_str(console, "\n");

    for (uint8_t i = 0; i < gnss_xa1110_descriptor.num_gps_sats_in_view; i++) {
        console_send_str(console, "\t\tSat ");
        utoa(i + 1, str, 10);
        console_send_str(console, str);
        console_send_str(console, ": (PRN: ");
        utoa((gnss_xa1110_descriptor.in_view_gps_satellites[i].prn +
              GPS_SV_OFFSET), str, 10);
        console_send_str(console, str);
        console_send_str(console, ", Elevation: ");
        utoa(gnss_xa1110_descriptor.in_view_gps_satellites[i].elevation, str,
             10);
        console_send_str(console, str);
        console_send_str(console, "°, Azimuth: ");
        utoa(gnss_xa1110_descriptor.in_view_gps_satellites[i].azimuth, str, 10);
        console_send_str(console, str);
        console_send_str(console, "°, SNR: ");
        utoa(gnss_xa1110_descriptor.in_view_gps_satellites[i].snr, str, 10);
        console_send_str(console, str);
        console_send_str(console, " dB-Hz)\n");
    }

    console_send_str(console, "\tGLONASS satellites in view: ");
    utoa(gnss_xa1110_descriptor.num_glonass_sats_in_view, str, 10);
    console_send_str(console, str);
    console_send_str(console, "\n");

    for (uint8_t i = 0; i < gnss_xa1110_descriptor.num_glonass_sats_in_view;
         i++) {
        console_send_str(console, "\t\tSat ");
        utoa(i + 1, str, 10);
        console_send_str(console, str);
        console_send_str(console, ": (ID: ");
        utoa((gnss_xa1110_descriptor.in_view_glonass_satellites[i].sat_id +
              GLONASS_SV_OFFSET), str, 10);
        console_send_str(console, str);
        console_send_str(console, ", Elevation: ");
        utoa(gnss_xa1110_descriptor.in_view_glonass_satellites[i].elevation,
             str, 10);
        console_send_str(console, str);
        console_send_str(console, "°, Azimuth: ");
        utoa(gnss_xa1110_descriptor.in_view_glonass_satellites[i].azimuth, str,
             10);
        console_send_str(console, str);
        console_send_str(console, "°, SNR: ");
        utoa(gnss_xa1110_descriptor.in_view_glonass_satellites[i].snr, str, 10);
        console_send_str(console, str);
        console_send_str(console, " dB-Hz)\n");
    }
#endif
}

// MARK: KX134 Who Am I

void debug_kx134_wai (uint8_t argc, char **argv, struct console_desc_t *console)
{
#ifdef SPI1_SERCOM_INST
    uint8_t tid;
    uint8_t data[6] = { (1 << 7) };
    char str[9];

    // Who Am I
    sercom_spi_start(&spi1_g, &tid, 10000000, KX134_1211_CS_PIN_GROUP,
                     KX134_1211_CS_PIN_MASK, data, 1, data, 6);
    while (!sercom_spi_transaction_done(&spi1_g, tid)) {
        wdt_pat();
    }
    console_send_str(console, "Manufacturer ID: \"");
    memcpy(str, data, 4);
    str[4] = '\0';
    console_send_str(console, str);
    console_send_str(console, "\"\nWho Am I: 0x");
    utoa(data[4], str, 16);
    console_send_str(console, str);
    console_send_str(console, "\nSilicon ID: 0x");
    utoa(data[5], str, 16);
    console_send_str(console, str);
    console_send_str(console, "\n");
    sercom_spi_clear_transaction(&spi1_g, tid);
#else
    console_send_str(console, "SPI1 not enabled in board configuration.\n");
#endif
}

// MARK: KX134 Test

void debug_kx134_test (uint8_t argc, char **argv,
                       struct console_desc_t *console)
{
#ifdef ENABLE_KX134_1211
    switch (kx134_g.state) {
        case KX134_1211_RUNNING:
            break;
        case KX134_1211_FAILED:
            console_send_str(console, "Failed\n");
            return;
        case KX134_1211_FAILED_WAI:
            console_send_str(console, "Failed: WAI invalid\n");
            return;
        case KX134_1211_FAILED_COTR:
            console_send_str(console, "Failed: COTR invalid\n");
            return;
        case KX134_1211_FAILED_SELF_TEST:
            console_send_str(console, "Failed: Self Test Failed\n");
            return;
        default:
            console_send_str(console, "Initializing...\n");
            return;
    }

    char str[16];

    // Print time since last reading
    uint32_t const last_reading_time = kx134_1211_get_last_time(&kx134_g);
    console_send_str(console, "Last reading at ");
    utoa(last_reading_time, str, 10);
    console_send_str(console, str);
    console_send_str(console, " (");
    utoa(MILLIS_TO_MS(millis - last_reading_time), str, 10);
    console_send_str(console, str);
    console_send_str(console, "  milliseconds ago)\n");

    // Print sensitivity
    uint16_t const sensitivity = kx134_1211_get_sensitivity(&kx134_g);
    console_send_str(console, "Sensitivity: ");
    utoa(sensitivity, str, 10);
    console_send_str(console, str);
    console_send_str(console, " LSB/g\n");

    // X
    int16_t const x = kx134_1211_get_last_x(&kx134_g);
    int32_t const x_g = (x * 10000) / sensitivity;
    console_send_str(console, "X: ");
    debug_print_fixed_point(console, x_g, 4);
    console_send_str(console, " g (");
    utoa(x, str, 10);
    console_send_str(console, str);
    console_send_str(console, ")\n");

    // Y
    int16_t const y = kx134_1211_get_last_y(&kx134_g);
    int32_t const y_g = (y * 10000) / sensitivity;
    console_send_str(console, "Y: ");
    debug_print_fixed_point(console, y_g, 4);
    console_send_str(console, " g (");
    utoa(y, str, 10);
    console_send_str(console, str);
    console_send_str(console, ")\n");

    // Z
    int16_t const z = kx134_1211_get_last_z(&kx134_g);
    int32_t const z_g = (z * 10000) / sensitivity;
    console_send_str(console, "Z: ");
    debug_print_fixed_point(console, z_g, 4);
    console_send_str(console, " g (");
    utoa(z, str, 10);
    console_send_str(console, str);
    console_send_str(console, ")\n");
#else
    console_send_str(console, "KX134 not enabled in board configuration.\n");
#endif
}
