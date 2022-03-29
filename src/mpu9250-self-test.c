/**
 * @file mpu9250-self-test.c
 * @desc Self checking functions for MPU9250 IMU
 * @author Samuel Dewan
 * @date 2021-09-29
 * Last Author:
 * Last Edited On:
 */

#include "mpu9250-self-test.h"

#include <stdlib.h>
#include <math.h>


#define USE_TABLE_FOR_TRIM


// MARK: Helpers

#ifdef USE_TABLE_FOR_TRIM
//  #! /usr/bin/env python3
//  l = [round(2620 * (1.01 ** i)) for i in range(255)]
//  for i in range(0, len(l), 8):
//      print(*l[i:i+8], sep=', ')
static const uint16_t st_trim_val[255] = {
    2620, 2646, 2673, 2699, 2726, 2754, 2781, 2809,
    2837, 2865, 2894, 2923, 2952, 2982, 3012, 3042,
    3072, 3103, 3134, 3165, 3197, 3229, 3261, 3294,
    3327, 3360, 3394, 3428, 3462, 3496, 3531, 3567,
    3602, 3638, 3675, 3711, 3749, 3786, 3824, 3862,
    3901, 3940, 3979, 4019, 4059, 4100, 4141, 4182,
    4224, 4266, 4309, 4352, 4396, 4440, 4484, 4529,
    4574, 4620, 4666, 4713, 4760, 4807, 4855, 4904,
    4953, 5003, 5053, 5103, 5154, 5206, 5258, 5310,
    5363, 5417, 5471, 5526, 5581, 5637, 5693, 5750,
    5808, 5866, 5925, 5984, 6044, 6104, 6165, 6227,
    6289, 6352, 6415, 6480, 6544, 6610, 6676, 6743,
    6810, 6878, 6947, 7016, 7087, 7157, 7229, 7301,
    7374, 7448, 7523, 7598, 7674, 7751, 7828, 7906,
    7985, 8065, 8146, 8227, 8310, 8393, 8477, 8561,
    8647, 8733, 8821, 8909, 8998, 9088, 9179, 9271,
    9363, 9457, 9552, 9647, 9744, 9841, 9940, 10039,
    10139, 10241, 10343, 10447, 10551, 10657, 10763, 10871,
    10979, 11089, 11200, 11312, 11425, 11539, 11655, 11771,
    11889, 12008, 12128, 12249, 12372, 12496, 12621, 12747,
    12874, 13003, 13133, 13264, 13397, 13531, 13666, 13803,
    13941, 14080, 14221, 14363, 14507, 14652, 14799, 14947,
    15096, 15247, 15399, 15553, 15709, 15866, 16025, 16185,
    16347, 16510, 16675, 16842, 17011, 17181, 17353, 17526,
    17701, 17878, 18057, 18238, 18420, 18604, 18790, 18978,
    19168, 19360, 19553, 19749, 19946, 20146, 20347, 20551,
    20756, 20964, 21173, 21385, 21599, 21815, 22033, 22253,
    22476, 22701, 22928, 23157, 23389, 23622, 23859, 24097,
    24338, 24582, 24827, 25076, 25326, 25580, 25836, 26094,
    26355, 26618, 26885, 27153, 27425, 27699, 27976, 28256,
    28538, 28824, 29112, 29403, 29697, 29994, 30294, 30597,
    30903, 31212, 31524, 31839, 32158, 32479, 32804
};
#else
/**
 *  Calculate coef * (1.01 ^ n) using exponentiation by squaring with 20.12
 *  fixed point math.
 *
 *  @param n Exponent
 *  @param coef Coefficient
 */
static uint32_t fixed_point_pow_coef(uint8_t n, uint16_t const coef)
{
    if (n == 0) {
        return 1;
    }

    // 1.01 * 2^12 = 4136.96 -> 4137
    uint32_t x = 4137;
    uint32_t y = (1u << 12);

    while (n > 1) {
        if (n & 0b1) {
            // n is odd
            y = ((x * y) + (1u << 11)) >> 12;
            x = ((x * x) + (1u << 11)) >> 12;
        } else {
            // n is even
            x = ((x * x) + (1u << 11)) >> 12;
        }
        n = n >> 1;
    }

    uint32_t const power = ((x * y) + (1u << 11)) >> 12;
    return ((power * coef) + (1u << 11)) >> 12;
}
#endif

/**
 *  Convert an ST_CODE value retrieved from the OTP memory of the sensor into
 *  the ST_OTP trim value.
 *
 *  See section 3.2 of InvenSense application note AN-MPU-9250A-03 (MPU-9250
 *  Accelerometer, Gyroscope and Compass Self-Test Implementation).
 *
 *  @param st_code The ST_CODE value retrieved from the sensors OTP memory
 *  @return The calculated ST_OTP value to be used in the self test procedure
 */
static inline int32_t calc_st_trim_from_code(uint8_t const st_code)
{
    // See application note: MPU-9250 Accelerometer, Gyroscope and Compass
    // Self-Test Implementation

#ifdef USE_TABLE_FOR_TRIM
    return (int32_t)st_trim_val[st_code - 1];
#else
    // st_otp = (2620 / (2 ^ (Full Scale Range Value))) * (1.01 ^ (st_code - 1))
    // The full scale range value is always zero for self test:
    //      st_otp = 2620 * (1.01 ^ (st_code - 1))
    return fixed_point_pow_coef(st_code - 1, 2620);
#endif
}


// MARK: Gyroscope

int mpu9250_check_gyro_st(int16_t const gst, uint8_t const gst_otp_code)
{
    // See application note: MPU-9250 Accelerometer, Gyroscope and Compass
    // Self-Test Implementation

    if (gst_otp_code != 0) {
        // Pass iff (GST / GST_OTP) > 0.5
        int32_t const double_gst = 2 * (int32_t)gst;
        int32_t const gst_otp = calc_st_trim_from_code(gst_otp_code);
        return !(double_gst > gst_otp);
    } else {
        // Pass iff |GST| >= 60 dps
        // 60 degrees per second = 7861 (131 LSB/dps)
        return !(abs(gst) >= 7861);
    }
}


// MARK: Accelerometer

int mpu9250_check_accel_st(int16_t const ast, uint8_t const ast_otp_code)
{
    // See application note: MPU-9250 Accelerometer, Gyroscope and Compass
    // Self-Test Implementation

    if (ast_otp_code != 0) {
        // Pass iff 0.5 < (AST / AST_OTP) < 1.5
        int32_t const ast_otp = calc_st_trim_from_code(ast_otp_code);
        int32_t const frac = (1000 * (int32_t)ast) / ast_otp;
        return !((500 < frac) && (frac < 1500));
    } else {
        // Pass iff 225 millig <= |AST| <= 675 millig
        // 225 millig = 3686 (16384 LSB/g)
        // 675 millig = 11059 (16384 LSB/g)
        return !((3686 < abs(ast)) && (abs(ast) < 11059));
    }
}


// MARK: Magnetometer

/**
 *  Adjust a magnetometer reading using the sensitivity adjustment values from
 *  the magnetometer's ROM.
 *
 *  See section 8.3.11 of AK8963 datasheet.
 *
 *  @param h The measurement to be adjusted
 *  @param asa The sensitivity adjustment value
 *  @return The adjusted sensor output
 */
static inline int32_t mag_adjust_sensitivity(int16_t const h, uint8_t const asa)
{
    return (((int32_t)h * ((int32_t)asa - 128)) / 256) + (int32_t)h;
}

int mpu9250_check_mag_st(int const axis, int16_t const h,  uint8_t const asa)
{
    // See application note: MPU-9250 Accelerometer, Gyroscope and Compass
    // Self-Test Implementation

    int32_t const ha = mag_adjust_sensitivity(h, asa);

    if ((axis == 0) || (axis == 1)) {
        // X or Y
        // Pass iff -200 <= H <= 200
        return !((-200 <= ha) && (ha <= 200));
    } else {
        // Z
        // Pass iff -3200 <= H <= 800
        return !((-3200 <= ha) && (ha <= -800));
    }
}
