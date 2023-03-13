/**
 * @file mpu9250-self-test.h
 * @desc Self checking functions for MPU9250 IMU
 * @author Samuel Dewan
 * @date 2021-09-29
 * Last Author:
 * Last Edited On:
 */

#ifndef mpu9250_self_test_h
#define mpu9250_self_test_h

#include <stdint.h>

/**
 *  Given the measured self test value for a gyroscope axis (G*_ST_OS - G*_OS)
 *  and the factory trim value, test if sensor passes self test.
 *
 *  @param gst The measured self test value (value with self test enable minus
 *             value without self test enabled averaged over 200 samples)
 *  @param gst_otp_code Factory trim value from OTP memory
 *  @return 0 if self test passes, a non-zero value otherwise
 */
extern int mpu9250_check_gyro_st(int16_t gst, uint8_t gst_otp_code);

/**
 *  Given the measured self test value for a accelerometer axis
 *  (A*_ST_OS - A*_OS) and the factory trim value, test if sensor passes self
 *  test.
 *
 *  @param ast The measured self test value (value with self test enable minus
 *             value without self test enabled averaged over 200 samples)
 *  @param ast_otp_code Factory trim value from OTP memory
 *  @return 0 if self test passes, a non-zero value otherwise
 */
extern int mpu9250_check_accel_st(int16_t ast, uint8_t ast_otp_code);

extern int mpu9250_check_mag_st(int axis, int16_t h, uint8_t asa);

#endif /* mpu9250_self_test_h */
