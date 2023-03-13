/**
 * @file mpu9250.h
 * @desc Driver for MPU9250 IMU
 * @author Samuel Dewan
 * @date 2021-09-29
 * Last Author:
 * Last Edited On:
 */

#ifndef mpu9250_h
#define mpu9250_h

#include "global.h"

#include "sercom-i2c.h"
#include "gpio.h"
#include "src/telemetry/telemetry.h"


#define MPU9250_BUFFER_LENGTH   128


/** MPU9250 sample rate */
enum ak8963_odr {
    /** 8 Hz */
    AK8963_ODR_8HZ,
    /** 100 Hz */
    AK8963_ODR_100HZ
};

/** MPU9250 gyroscope full scale range */
enum mpu9250_gyro_fsr {
    /** +/-250 degrees per second */
    MPU9250_GYRO_FSR_250DPS,
    /** +/-500 degrees per second */
    MPU9250_GYRO_FSR_500DPS,
    /** +/-1000 degrees per second */
    MPU9250_GYRO_FSR_1000DPS,
    /** +/-2000 degrees per second */
    MPU9250_GYRO_FSR_2000DPS
};

/** MPU9250 accelerometer full scale range */
enum mpu9250_accel_fsr {
    /** +/-2 g */
    MPU9250_ACCEL_FSR_2G,
    /** +/-4 g */
    MPU9250_ACCEL_FSR_4G,
    /** +/-8 g */
    MPU9250_ACCEL_FSR_8G,
    /** +/-16 g */
    MPU9250_ACCEL_FSR_16G
};

/** MPU9250 gyroscope low pass filter 3dB bandwidth */
enum mpu9250_gyro_bw {
    /** 5 Hz */
    MPU9250_GYRO_BW_5HZ,
    /** 10 Hz */
    MPU9250_GYRO_BW_10HZ,
    /** 20 Hz */
    MPU9250_GYRO_BW_20HZ,
    /** 41 Hz */
    MPU9250_GYRO_BW_41HZ,
    /** 92 Hz */
    MPU9250_GYRO_BW_92HZ,
    /** 184 Hz */
    MPU9250_GYRO_BW_184HZ,
    /** 250 Hz */
    MPU9250_GYRO_BW_250HZ
};

/** MPU9250 gyroscope low pass filter 3dB bandwidth */
enum mpu9250_accel_bw {
    /** 5.05 Hz */
    MPU9250_ACCEL_BW_5HZ,
    /** 10.2 Hz */
    MPU9250_ACCEL_BW_10HZ,
    /** 21.2 Hz */
    MPU9250_ACCEL_BW_21HZ,
    /** 44.8 Hz */
    MPU9250_ACCEL_BW_45HZ,
    /** 99 Hz */
    MPU9250_ACCEL_BW_99HZ,
    /** 218.1 Hz */
    MPU9250_ACCEL_BW_218HZ,
    /** 420 Hz */
    MPU9250_ACCEL_BW_420HZ
};


enum mpu9250_state {
    /** Read from WHO_AM_I register and verify value (should be 0x71) */
    MPU9250_READ_AG_WAI,

// ##### Reset accel/gyro #####
    /**  Write to PWR_MGMT_1 with H_RESET set */
    MPU9250_SOFT_RESET,
    /** Write to PWR_MGMT_1 with CLKSEL = 1 to switch to PLL clocked from gyro
        osc then wait for 100 ms to make sure that the clock is stable */
    MPU9250_SELECT_CLOCK,
// Note: next state after MPU9250_SELECT_CLOCK is MPU9250_AG_ST_CONFIG_SENSORS,
//       the following states (ACC states and MPU9250_USER_REST) are sequences
//       which are jumped back to from other states

// ##### Accel/Gyro sample accumulation sequence #####
    /** Write to USER_CTRL to reset and enable FIFO module */
    MPU9250_SAMP_ACC_EN_FIFO,
    /** Write to FIFO_EN to enable writing of gyro x, y and z and accel data to
        FIFO */
    MPU9250_SAMP_ACC_CONFIG_FIFO,
    /** Wait for as many samples as we can fit in our buffer to be stored in
        FIFO */
    MPU9250_SAMP_ACC_WAIT,
    /** Read FIFO_COUNT to check how many samples have been accumulated */
    MPU9250_SAMP_ACC_READ_COUNT,
    /** Read back the samples we have so far and sum them up */
    MPU9250_SAMP_ACC_READ_SAMPLES,
// Note: MPU9250_SAMP_ACC_WAIT, MPU9250_SAMP_ACC_READ_COUNT and
//       MPU9250_SAMP_ACC_READ_SAMPLES are repeated until we get all the samples
//       we want
    /** Write to FIFO_EN to disable writing of gyro x, y and z and accel data to
        FIFO */
    MPU9250_SAMP_ACC_DECONFIG_FIFO,

// ##### User reset sequence #####
    /** Write to USER_CTRL to reset FIFO, I2C master and sensors, leave FIFO and
        I2C master disabled */
    MPU9250_USER_REST,


// ##### Do accel/gyro self test #####
    /** Write to CONFIG, GYRO_CONFIG, ACCEL_CONFIG and ACCEL_CONFIG_2: accel and
        gyro DPLF configs to 2 and to zero out everything else */
    MPU9250_AG_ST_CONFIG_SENSORS,
// Do Accel/Gyro sample accumulation sequence to accumulate 200 samples
    /** Write to GYRO_CONFIG and ACCEL_CONFIG to enable self test on all axes
        and wait 20 ms for sensor output to stabilize */
    MPU9250_AG_ST_ENABLE_ST,
// Do Accel/Gyro sample accumulation sequence to subtract 200 samples
    /** Read SELF_TEST_*_GYRO into buffer[0-2] */
    MPU9250_AG_ST_READ_ST_GYRO_OTP,
    /** Read SELF_TEST_*_ACCEL into buffer[3-5] and check self test result */
    MPU9250_AG_ST_READ_ST_ACCEL_OTP,
// Do user reset sequence

// ##### Reset magnetometer #####
    /** Write to INT_PIN_CFG to enable I2C bypass */
    MPU9250_ENABLE_I2C_BYPASS,
    /** Read magnetometer WAI (should be 0x48) */
    MPU9250_READ_MAG_WAI,
    /** Write to CNTL2 to reset magnetometer */
    MPU9250_RESET_MAG,
// ##### Read magnetometer sensitivity adjustment registers #####
    /** Write to CNTL1 to enter fuse ROM access mode */
    MPU9250_MAG_SENS_ROM_ACC_MODE,
    /** Read ASAX, ASAY and ASAZ */
    MPU9250_MAG_SENS_READ,
    /** Write to CNTL1 to enter power down mode */
    MPU9250_MAG_POWER_DOWN,
// ##### Self test magnetometer #####
    /** Set SELF bit in ASTC register */
    MPU9250_MAG_ST_ENABLE,
    /** Write to CNTL1 to enter self test mode with 16 bit output */
    MPU9250_MAG_ST_ENTER_ST_MODE,
    /** Read ST1 to check if data ready, wait 1 ms if not, repeat until it is */
    MPU9250_MAG_ST_POLL,
    /** Read data from HXL to ST2, check self test result */
    MPU9250_MAG_ST_READ,
    /** Clear SELF bit in ASTC register */
    MPU9250_MAG_ST_DISABLE,
// Do MPU9250_MAG_POWER_DOWN

// ##### Calibrate accel/gyro #####
    /** Write to INT_ENABLE to disable all interrupts (in case we are
        re-calibrating after having already been running for a while) */
    MPU9250_AG_CAL_DISABLE_INT,
    /** Write to FIFO_EN to disable writing of any data to FIFO */
    MPU9250_AG_CAL_DECONFIG_FIFO,
// Do user reset sequence
    /** Write SMPLRT_DIV, CONFIG, GYRO_CONFIG, ACCEL_CONFIG and ACCEL_CONFIG_2
        to sample at 1 KHz with a 184 Hz LPF for gyro and a 218.1 Hz LPF for
        accel, FSR of 250 degrees per second for gyro, 2 g for accel */
    MPU9250_AG_CAL_CONFIG_SENSORS,
// Do Accel/Gyro sample accumulation sequence to accumulate 200 samples
    /** Calculate offset values and write XG_OFFSET_H through ZG_OFFSET_L */
    MPU9250_AG_CAL_WRITE_GYRO_OFFS,
    /** Read XA_OFFSET_H through ZA_OFFSET_L (so that we can preserve the unused
        bits in these registers) */
    MPU9250_AG_CAL_READ_ACCEL_OFFS,
    /** Write XA_OFFSET_H through ZA_OFFSET_L */
    MPU9250_AG_CAL_WRITE_ACCEL_OFFS,
// Do user reset sequence

// ##### Initialize magnetometer for normal operation #####
    /** Write CNTL1 to select 8 or 100 Hz continuous mode with 16 bit
        resolution */
    MPU9250_MAG_ENABLE,

// ##### Configure accel/gyro to read magnetometer #####
    /** Write I2C_MST_CTRL, I2C_SLV0_ADDR, I2C_SLV0_REG and I2C_SLV0_CTRL to
        read 7 bytes from magnetometer starting at HXL, I2C master configured
        for 400 KHz clock and to delay data ready interrupt until external
        sensor data is ready */
    MPU9250_CONFIG_I2C_MST,
    /** Write to USER_CTRL to enable I2C master (and enable FIFO for FIFO driven
        operation) */
    MPU9250_ENABLE_I2C_MST_AND_FIFO,

// ##### Initialize accel/gyro for normal operation #####
    /** Write to SMPLRT_DIV, CONFIG, GYRO_CONFIG, ACCEL_CONFIG and
        ACCEL_CONFIG_2 to configure DLPFs and sample rate */
    MPU9250_AG_CONFIG_SENSORS,
// For interrupt driven operation:
    /** Write to INT_PIN_CFG and INT_ENABLE to enable clearing of interrupt
        status when any register is read (leave I2C bypass enabled as well) and
        to enable raw data ready interrupt */
    MPU9250_AG_CONFIG_INT,
// For FIFO driven operation:
    /** Write to FIFO_EN to enable writing of gyro x, y and z, accel, temp and
        I2C slave 0 data to FIFO */
    MPU9250_AG_CONFIG_FIFO,

// ##### Normal operation (interrupt driven) #####
    /** Reading data is handled by callbacks */
    MPU9250_RUNNING,

// ##### Normal operation (FIFO driven) #####
    /** Wait for samples to be written into FIFO */
    MPU9250_FIFO_WAIT,
    /** Read FIFO count */
    MPU9250_FIFO_READ_COUNT,
    /** Read samples from FIFO */
    MPU9250_FIFO_READ,

// ##### Failure states #####
    /** Driver failed */
    MPU9250_FAILED,
    /** Accel/gyro WAI not recognized */
    MPU9250_FAILED_AG_WAI,
    /** Magnetometer WAI not recognized */
    MPU9250_FAILED_MAG_WAI,
    /** Accel/gyro Self test failed */
    MPU9250_FAILED_AG_SELF_TEST,
    /** Magnetometer Self test failed */
    MPU9250_FAILED_MAG_SELF_TEST
};


struct mpu9250_desc_t {
    /** I2C instance used by this sensor */
    struct sercom_i2c_desc_t *i2c_inst;

    /** Telemetry service instance */
    struct telemetry_service_desc_t *telem;

    /** Buffer used for I2C transaction data */
    uint8_t buffer[MPU9250_BUFFER_LENGTH];

    /** Pointer to buffer to be used when reading samples from sensor, could be
        a buffer provided by the telemetry service or this driver instances own
        buffer */
    uint8_t *telem_buffer;

    /** The millis value when we started waiting for something */
    uint32_t wait_start;

    /** Values used when averaging samples for self test and offset
        calibration */
    int32_t accel_accumulators[3];
    /** Values used when averaging samples for self test and offset
        calibration */
    int32_t gyro_accumulators[3];

    /** Records time of interrupt before a sample is read from the chip */
    uint32_t next_sample_time;

    uint32_t last_sample_time;
    int16_t last_accel_x;
    int16_t last_accel_y;
    int16_t last_accel_z;
    int16_t last_gyro_x;
    int16_t last_gyro_y;
    int16_t last_gyro_z;
    int16_t last_temp;
    int16_t last_mag_x;
    int16_t last_mag_y;
    int16_t last_mag_z;

    /** Magnetometer sensitivity adjustment values */
    uint8_t mag_asa[3];

    uint8_t samples_to_read;
    uint8_t extra_samples;

    uint8_t samples_left;

    /** Sensor I2C address */
    uint8_t mpu9250_addr;


    /** I2C transaction id */
    uint8_t t_id;

    uint8_t retry_count;

    /** Value to be loaded into sample rate register to set ODR */
    uint8_t odr;

    /** ODR for magnetometer */
    enum ak8963_odr mag_odr:1;
    /** Full scale range for gyroscope */
    enum mpu9250_gyro_fsr gyro_fsr:2;
    /** Full scale range for accelerometer */
    enum mpu9250_accel_fsr accel_fsr:2;
    /** LPF bandwidth for gyroscope */
    enum mpu9250_gyro_bw gyro_bw:7;
    /** LPF bandwidth for accelerometer */
    enum mpu9250_accel_bw accel_bw:7;

    /** Driver current state */
    enum mpu9250_state state:6;
    /** Driver state to continue to after subsequence */
    enum mpu9250_state next_state:6;

    /** Flag set to indicate that the drive should wait for the FIFO to fill and
        read data in larger chunks rather than reading each sample using the
        interrupt */
    uint8_t use_fifo:1;
    /** Flag to indicate that we the register values to be sent in the current
        state have been marshaled */
    uint8_t cmd_ready:1;
    /** Flag to indicate that we are waiting for an I2C transaction */
    uint8_t i2c_in_progress:1;
    /** Flag to indicate that we are doing a wait after an I2C transaction is
        done */
    uint8_t post_cmd_wait:1;
    /** Flag to indicate that samples should be subtracted by accumulation
        sequence */
    uint8_t acc_subtract:1;
    /** Bit that indicates the last magnetometer sample is not valid because the
        magnetic sensor overflowed */
    uint8_t last_mag_overflow:1;
    /** Flag to indicate that we currently have buffer checked out from the
        telemetry service */
    uint8_t telemetry_buffer_checked_out:1;
    /** Flag to indicate that an I2C transaction initiated from an interrupt is
        in progress (separate from i2c_in_progress to avoid affecting FSM) */
    uint8_t async_i2c_in_progress:1;
};




extern int init_mpu9250(struct mpu9250_desc_t *inst,
                        struct sercom_i2c_desc_t *i2c_inst, uint8_t i2c_addr,
                        union gpio_pin_t int_pin,
                        enum mpu9250_gyro_fsr gyro_fsr,
                        enum mpu9250_gyro_bw gyro_bw,
                        enum mpu9250_accel_fsr accel_fsr,
                        enum mpu9250_accel_bw accel_bw, uint16_t ag_odr,
                        enum ak8963_odr mag_odr, int use_fifo);

static inline void mpu9250_register_telem(struct mpu9250_desc_t *inst,
                                        struct telemetry_service_desc_t *telem)
{
    inst->telem = telem;
}

extern void mpu9250_service(struct mpu9250_desc_t *inst);






/**
 *  Get the sensitivity of the gyroscope.
 *
 *  @param inst The MPU9250 driver instance
 *
 *  @return Gyroscope sensitivity in LSB/1000 dps
 */
static inline uint32_t mpu9250_gyro_sensitivity(const struct mpu9250_desc_t *inst)
{
    switch (inst->gyro_fsr) {
        case MPU9250_GYRO_FSR_250DPS:
            // sensitivity = (2^15) / 250 = 131.072 LSB/dps
            return 131072;
        case MPU9250_GYRO_FSR_500DPS:
            // sensitivity = (2^15) / 500 = 65.536 LSB/dps
            return 65536;
        case MPU9250_GYRO_FSR_1000DPS:
            // sensitivity = (2^15) / 1000 = 32.768 LSB/dps
            return 32768;
        case MPU9250_GYRO_FSR_2000DPS:
            // sensitivity = (2^15) / 2000 = 16.384 LSB/dps
            return 16384;
    }
}

/**
 *  Get the sensitivity of the accelerometer.
 *
 *  @param inst The MPU9250 driver instance
 *
 *  @return Accelerometer sensitivity in LSB/g
 */
static inline uint16_t mpu9250_accel_sensitivity(const struct mpu9250_desc_t *inst)
{
    switch (inst->accel_fsr) {
        case MPU9250_ACCEL_FSR_2G:
            // sensitivity = (2^15) / 2 = 16384 LSB/g
            return 16384;
        case MPU9250_ACCEL_FSR_4G:
            // sensitivity = (2^15) / 4 = 8192 LSB/g
            return 8192;
        case MPU9250_ACCEL_FSR_8G:
            // sensitivity = (2^15) / 8 = 4096 LSB/g
            return 4096;
        case MPU9250_ACCEL_FSR_16G:
            // sensitivity = (2^15) / 16 = 2048 LSB/g
            return 2048;
    }
}

/**
 *  Get the sensitivity of the magnetometer.
 *
 *  @param inst The MPU9250 driver instance
 *
 *  @return Magnetometer sensitivity in LSB per mT
 */
static inline uint16_t mpu9250_mag_sensitivity(const struct mpu9250_desc_t *inst)
{
    // Sensitivity in 16 bit mode is 0.15 microT/LSB -> 6 and 2/3 LSB/microT
    return 6666;
}

/**
 *  Get the sample rate for the accelerometer and gyroscope.
 *
 *  @param inst The MPU9250 driver instance
 *
 *  @return Accelerometer and gyroscope ODR in Hz
 */
static inline uint16_t mpu9250_get_ag_odr(const struct mpu9250_desc_t *inst)
{
    return (uint16_t)1000 / ((uint16_t)(inst->odr) + 1);
}

/**
 *  Get the full scale range for the accelerometer.
 *
 *  @param inst The MPU9250 driver instance
 *
 *  @return The full scale range in g.
 */
static inline uint8_t mpu9250_get_accel_fsr(const struct mpu9250_desc_t *inst)
{
    switch (inst->accel_fsr) {
        case MPU9250_ACCEL_FSR_2G:
            return 2;
        case MPU9250_ACCEL_FSR_4G:
            return 4;
        case MPU9250_ACCEL_FSR_8G:
            return 8;
        case MPU9250_ACCEL_FSR_16G:
            return 16;
    }
}

/**
 *
 */
static inline uint16_t mpu9250_get_gyro_fsr(const struct mpu9250_desc_t *inst)
{
    switch (inst->gyro_fsr) {
        case MPU9250_GYRO_FSR_250DPS:
            return 250;
        case MPU9250_GYRO_FSR_500DPS:
            return 500;
        case MPU9250_GYRO_FSR_1000DPS:
            return 1000;
        case MPU9250_GYRO_FSR_2000DPS:
            return 2000;
    }
}



/**
 *  Get the time of the most recent measurement.
 *
 *  @param inst The MPU9250 driver instance
 *
 *  @return The value of millis for the most recent measurement
 */
static inline uint32_t mpu9250_get_last_time (const struct mpu9250_desc_t *inst)
{
    return inst->last_sample_time;
}

/**
 *  Get the most recent x axis acceleration measurement.
 *
 *  @param inst The MPU9250 driver instance
 *
 *  @return The most recent x axis acceleration measurement
 */
static inline int16_t mpu9250_get_accel_x(const struct mpu9250_desc_t *inst)
{
    return inst->last_accel_x;
}

/**
 *  Get the most recent y axis acceleration measurement.
 *
 *  @param inst The MPU9250 driver instance
 *
 *  @return The most recent y axis acceleration measurement
 */
static inline int16_t mpu9250_get_accel_y(const struct mpu9250_desc_t *inst)
{
    return inst->last_accel_y;
}

/**
 *  Get the most recent z axis acceleration measurement.
 *
 *  @param inst The MPU9250 driver instance
 *
 *  @return The most recent z axis acceleration measurement
 */
static inline int16_t mpu9250_get_accel_z(const struct mpu9250_desc_t *inst)
{
    return inst->last_accel_z;
}

/**
 *  Get the most recent temperature measurement.
 *
 *  @param inst The MPU9250 driver instance
 *
 *  @return The most recent temperature measurement in millidegrees celsius
 */
extern int32_t mpu9250_get_temperature(const struct mpu9250_desc_t *inst);

/**
 *  Get the most recent x axis angular velocity measurement.
 *
 *  @param inst The MPU9250 driver instance
 *
 *  @return The most recent x axis angular velocity measurement
 */
static inline int16_t mpu9250_get_gyro_x(const struct mpu9250_desc_t *inst)
{
    return inst->last_gyro_x;
}

/**
 *  Get the most recent y axis angular velocity measurement.
 *
 *  @param inst The MPU9250 driver instance
 *
 *  @return The most recent y axis angular velocity measurement
 */
static inline int16_t mpu9250_get_gyro_y(const struct mpu9250_desc_t *inst)
{
    return inst->last_gyro_y;
}

/**
 *  Get the most recent z axis angular velocity measurement.
 *
 *  @param inst The MPU9250 driver instance
 *
 *  @return The most recent z axis angular velocity measurement
 */
static inline int16_t mpu9250_get_gyro_z(const struct mpu9250_desc_t *inst)
{
    return inst->last_gyro_z;
}

/**
 *  Get the most recent x axis magnetic flux density measurement.
 *
 *  @param inst The MPU9250 driver instance
 *
 *  @return The most recent x axis magnetic flux density measurement
 */
static inline int16_t mpu9250_get_mag_x(const struct mpu9250_desc_t *inst)
{
    return inst->last_mag_x;
}

/**
 *  Get the most recent y axis magnetic flux density measurement.
 *
 *  @param inst The MPU9250 driver instance
 *
 *  @return The most recent y axis magnetic flux density measurement
 */
static inline int16_t mpu9250_get_mag_y(const struct mpu9250_desc_t *inst)
{
    return inst->last_mag_y;
}

/**
 *  Get the most recent z axis magnetic flux density measurement.
 *
 *  @param inst The MPU9250 driver instance
 *
 *  @return The most recent z axis magnetic flux density measurement
 */
static inline int16_t mpu9250_get_mag_z(const struct mpu9250_desc_t *inst)
{
    return inst->last_mag_z;
}

/**
 *  Get whether the magnetometer overflowed during the most recent measurement.
 *
 *  @param inst The MPU9250 driver instance
 *
 *  @return Whether the magnetometer overflowed during the most recent
 *          measurement
 */
static inline int mpu9250_get_mag_overflow(const struct mpu9250_desc_t *inst)
{
    return inst->last_mag_overflow;
}








//
//  Telemetry related functions defined in telemetry.c
//

/**
 *  Post data from MPU9250 IMU to telemetry service.
 *
 *  @param inst Telemetry service instance
 *  @param time Mission time for data being posted
 *  @param ag_sr_div Sample rate division register value
 *  @param accel_fsr Acceleration full scale range
 *  @param gyro_frs Angular velocity full scale range
 *  @param accel_bw Accelerometer 3 dB bandwidth
 *  @param gyro_bw Gyroscope 3 dB bandwidth
 *  @param sensor_payload_length Amount of sensor data to be posted in bytes
 *
 *  @return Pointer to a buffer into which sensor data can be copied or NULL if
 *          a buffer cannot be provided
 */
extern uint8_t *telemetry_post_mpu9250_imu(
                                           struct telemetry_service_desc_t *inst,
                                           uint32_t time, uint8_t ag_sr_div,
                                           enum ak8963_odr mag_odr,
                                           enum mpu9250_accel_fsr accel_fsr,
                                           enum mpu9250_gyro_fsr gyro_frs,
                                           enum mpu9250_accel_bw accel_bw,
                                           enum mpu9250_gyro_bw gyro_bw,
                                           uint16_t sensor_payload_length);

/**
 *  Indicate to telemetry service that MPU9250 data has been copied
 *  into buffer received from telemetry_post_mpu9250_imu().
 *
 *  @param inst Telemetry service instance
 *  @param buffer The buffer into which data has been copied.
 */
extern int telemetry_finish_mpu9250_imu(struct telemetry_service_desc_t *inst,
                                        uint8_t *buffer);


#endif /* mpu9250_h */
