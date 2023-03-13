/**
 * @file debug-commands-sensors.h
 * @desc Commands for debugging CLI
 * @author Samuel Dewan
 * @date 2020-08-11
 * Last Author:
 * Last Edited On:
 */

#ifndef debug_commands_sensors_h
#define debug_commands_sensors_h

#include <stdint.h>
#include "cli.h"


#define DEBUG_ALT_PROM_NAME  "alt-prom"
#define DEBUG_ALT_PROM_HELP  "Read data from altimeter PROM."

extern void debug_alt_prom (uint8_t argc, char **argv,
                            struct console_desc_t *console);


#define DEBUG_IMU_WAI_NAME  "imu-wai"
#define DEBUG_IMU_WAI_HELP  "Read IMU Who Am I register."

extern void debug_imu_wai (uint8_t argc, char **argv,
                           struct console_desc_t *console);


#define DEBUG_ALT_NAME  "alt-test"
#define DEBUG_ALT_HELP  "Print most recent values from altimeter."


extern void debug_alt (uint8_t argc, char **argv,
                       struct console_desc_t *console);


#define DEBUG_ALT_TARE_NOW_NAME "alt-tare-now"
#define DEBUG_ALT_TARE_NOW_HELP "Tare altimeter to most recently measured "\
                                "pressure"

extern void debug_alt_tare_now (uint8_t argc, char **argv,
                                struct console_desc_t *console);


#define DEBUG_ALT_TARE_NEXT_NAME    "alt-tare-next"
#define DEBUG_ALT_TARE_NEXT_HELP    "Tare altimeter to next measured pressure"

extern void debug_alt_tare_next (uint8_t argc, char **argv,
                                 struct console_desc_t *console);


#define DEBUG_GNSS_NAME  "gnss"
#define DEBUG_GNSS_HELP  "Print GNSS info"

extern void debug_gnss (uint8_t argc, char **argv,
                        struct console_desc_t *console);


#define DEBUG_KX134_WAI_NAME    "kx134-wai"
#define DEBUG_KX134_WAI_HELP    "Read KX134 Accelerometer Who Am I register."

extern void debug_kx134_wai (uint8_t argc, char **argv,
                            struct console_desc_t *console);


#define DEBUG_KX134_TEST_NAME   "kx134-test"
#define DEBUG_KX134_TEST_HELP   "Print information from KX124-1211 driver."

extern void debug_kx134_test (uint8_t argc, char **argv,
                              struct console_desc_t *console);

#define DEBUG_MPU9250_WAI_NAME  "mpu9250-wai"
#define DEBUG_MPU9250_WAI_HELP  "Read MPU-9250 IMU Who Am I register."

extern void debug_mpu9250_wai (uint8_t argc, char **argv,
                               struct console_desc_t *console);

#define DEBUG_MPU9250_TEST_NAME "mpu9250-test"
#define DEBUG_MPU9250_TEST_HELP "Print information from MPU9250 driver."

extern void debug_mpu9250_test (uint8_t argc, char **argv,
                                struct console_desc_t *console);

#endif /* debug_commands_sensors_h */
