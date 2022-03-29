/**
 * @file mpu9250-states.c
 * @desc Driver state machine for MPU9250 IMU
 * @author Samuel Dewan
 * @date 2021-09-29
 * Last Author:
 * Last Edited On:
 */

#include "mpu9250-states.h"

#include "mpu9250-registers.h"
#include "ak8963-registers.h"

#include "mpu9250-self-test.h"

// MARK: Constants

#define MPU9250_RESET_WAIT_PERIOD       MS_TO_MILLIS(2)
#define MPU9250_CLOCK_WAIT_PERIOD       MS_TO_MILLIS(100)
#define MPU9250_AG_ST_NUM_SAMPS         200
#define MPU9250_AG_CAL_NUM_SAMPS        200
#define MPU9250_AG_ST_STABILIZE_PERIOD  MS_TO_MILLIS(20)
#define MPU9250_MAG_POLL_PERIOD         MS_TO_MILLIS(1)

#define MPU9250_AG_ACC_SAMPLE_LEN       12


// MARK: Helpers

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

/**
 *  Calculate the number of samples that should be read from the FIFO after
 *  having read the FIFO_COUNT register. This function takes into account the
 *  amount of buffer space available and how many samples are required to be
 *  read for the current operation. It updates the samples_to_read field in the
 *  instance descriptor to indicate how many samples should be read from the
 *  FIFO. The extra_samples field will be updated to indicate how many more
 *  samples are available in the FIFO above samples_to_read.
 *
 *  @note This function should only be called when FIFO_COUNTH and FIFO_COUNTL
 *        have just been read into buffer.
 *
 *  @param inst Driver instance
 *  @param samples_left The number of samples that need to be collected for the
 *                      current operation or 0 to read as many samples as
 *                      possible
 *  @param sample_length The number of bytes in each sample
 */
static inline void calc_samples_to_read(struct mpu9250_desc_t *const inst,
                                        uint8_t const samples_left,
                                        uint8_t const sample_length)
{
    uint16_t const fifo_count = ((((uint16_t)(inst->buffer[0] & 0x1fU)) << 8) |
                                 (uint16_t)inst->buffer[1]);
    uint16_t const fifo_samples = fifo_count / sample_length;
    uint8_t const max_samples = MPU9250_BUFFER_LENGTH / sample_length;

    if (fifo_samples > max_samples) {
        inst->samples_to_read = max_samples;
    } else {
        inst->samples_to_read = fifo_samples;
    }

    if ((samples_left != 0) && (inst->samples_to_read > samples_left)) {
        inst->samples_to_read = inst->samples_left;
    }

    inst->extra_samples = fifo_samples - inst->samples_to_read;
}

/**
 *  Determine how many sample periods the driver should wait before read from
 *  the FIFO. This function takes into account the size of the buffer, the
 *  number of samples required for the current operation and the number of
 *  samples that we already know to be available in the FIFO.
 *
 *  @param required The number of samples that need to be read for the current
 *                  operation or 0 to read as many samples as possible
 *  @param available The number of samples known to be available in the FIFO
 *                   already
 *  @param sample_length The number of bytes in a sample
 *  @return The number of sample periods that the driver should wait before
 *          reading from the FIFO again
 */
static inline uint8_t calc_num_samples_to_wait_for(uint8_t const required,
                                                   uint8_t const available,
                                                   uint8_t const sample_length)
{
    uint8_t const max = MPU9250_BUFFER_LENGTH / sample_length;
    uint8_t to_read = required;
    if ((to_read > max) || (to_read == 0)) {
        to_read = max;
    }

    if (available >= to_read) {
        return 0;
    } else {
        return to_read - available;
    }
}

void parse_mpu9250_data(struct mpu9250_desc_t *const inst,
                        const uint8_t *const s)
{
    inst->last_accel_x = (int16_t)((((uint16_t)s[0]) << 8) | (uint16_t)s[1]);
    inst->last_accel_y = (int16_t)((((uint16_t)s[2]) << 8) | (uint16_t)s[3]);
    inst->last_accel_z = (int16_t)((((uint16_t)s[4]) << 8) | (uint16_t)s[5]);

    inst->last_temp = (int16_t)((((uint16_t)s[6]) << 8) | (uint16_t)s[7]);

    inst->last_gyro_x = (int16_t)((((uint16_t)s[8]) << 8) | (uint16_t)s[9]);
    inst->last_gyro_y = (int16_t)((((uint16_t)s[10]) << 8) | (uint16_t)s[11]);
    inst->last_gyro_z = (int16_t)((((uint16_t)s[12]) << 8) | (uint16_t)s[13]);

    int16_t const mag_x = (int16_t)((uint16_t)s[14] | (((uint16_t)s[15]) << 8));
    int16_t const mag_y = (int16_t)((uint16_t)s[16] | (((uint16_t)s[17]) << 8));
    int16_t const mag_z = (int16_t)((uint16_t)s[18] | (((uint16_t)s[19]) << 8));

    inst->last_mag_x = mag_adjust_sensitivity(mag_x, inst->mag_asa[0]);
    inst->last_mag_y = mag_adjust_sensitivity(mag_y, inst->mag_asa[1]);
    inst->last_mag_z = mag_adjust_sensitivity(mag_z, inst->mag_asa[2]);
    inst->last_mag_overflow = !!(inst->buffer[20] & AK8963_ST2_HOFL);
}


enum state_helper_result {
    /** Operation for this state is complete */
    STATE_DONE,
    /** Operation is ongoing, need to wait longer */
    STATE_WAIT,
    /** Try operation again later */
    STATE_RETRY,
    /** Failed to start and I2C transaction, maybe try again later */
    STATE_COULD_NOT_START_I2C,
    /** Operation failed */
    STATE_FAILED
};

/**
 *  Handle an I2C transaction.
 *
 *  @param inst MPU9250 driver instance
 *  @param addr I2C address of device to communicate with
 *  @param reg Register address
 *  @param length Number of bytes to transfer
 *  @param buffer Buffer for data to be transferred
 *  @param write_reg If zero transaction will be a read, otherwise it will be a
 *                   write
 *  @param allow_retry If non-zero retries will be counted, otherwise failures
 *                     will result in going right to the failed state
 */
static enum state_helper_result do_reg_state(struct mpu9250_desc_t *const inst,
                                             uint8_t const addr,
                                             uint8_t const reg,
                                             uint16_t const length,
                                             uint8_t *const buffer,
                                             int const write_reg,
                                             int const allow_retry)
{
    if (inst->i2c_in_progress) {
        // I2C transaction has finished!
        inst->i2c_in_progress = 0;

        enum i2c_transaction_state const state = sercom_i2c_transaction_state(
                                                                inst->i2c_inst,
                                                                inst->t_id);
        sercom_i2c_clear_transaction(inst->i2c_inst, inst->t_id);

        if (state != I2C_STATE_DONE) {
            // Failed
            if (allow_retry) {
                inst->retry_count++;
                return STATE_RETRY;
            } else {
                return STATE_FAILED;
            }
        }

        inst->retry_count = 0;
        inst->cmd_ready = 0;
        return STATE_DONE;
    }

    // Start I2C transaction
    if (write_reg) {
        inst->i2c_in_progress = !sercom_i2c_start_reg_write(inst->i2c_inst,
                                                            &inst->t_id, addr,
                                                            reg, buffer,
                                                            length);
    } else {
        inst->i2c_in_progress = !sercom_i2c_start_reg_read(inst->i2c_inst,
                                                           &inst->t_id, addr,
                                                           reg, buffer, length);
    }

    return inst->i2c_in_progress ? STATE_WAIT : STATE_COULD_NOT_START_I2C;
}




// MARK: State Handlers

/** Read from WHO_AM_I register and verify value (should be 0x71) */
static int mpu9250_handle_read_ag_wai(struct mpu9250_desc_t *const inst)
{
    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_WHO_AM_I,
                                                      1, inst->buffer, 0, 1);

    if (res == STATE_RETRY) {
        // Failed to read from sensor over I2C bus
        inst->state = MPU9250_FAILED;
        return 0;
    } else if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Check WAI value
    if (inst->buffer[0] != MPU9250_WHO_AM_I_VAL) {
        inst->state = MPU9250_FAILED_AG_WAI;
        return 0;
    }

    // Move to next state
    inst->state = MPU9250_SOFT_RESET;
    return 1;
}


/**  Write to PWR_MGMT_1 with H_RESET set */
static int mpu9250_handle_soft_reset(struct mpu9250_desc_t *const inst)
{
    if (inst->post_cmd_wait) {
        // Post command wait
        if ((millis - inst->wait_start) < MPU9250_RESET_WAIT_PERIOD) {
            // Still waiting
            return 0;
        }
        // Done, move to next state
        inst->post_cmd_wait = 0;
        inst->state = MPU9250_SELECT_CLOCK;
        return 1;
    } else if (!inst->cmd_ready) {
        /* Marshal command to set H_RESET bit in PWR_MGMT_1 register */
        inst->buffer[0] = MPU9250_PWR_MGMT_1_H_RESET;
        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_PWR_MGMT_1,
                                                      1, inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Prepare for post command wait
    inst->post_cmd_wait = 1;
    inst->wait_start = millis;
    return 0;
}


/** Write to PWR_MGMT_1 with CLKSEL = 1 to switch to PLL clocked from gyro osc
    then wait for 100 ms to make sure that the clock is stable */
static int mpu9250_handle_select_clock(struct mpu9250_desc_t *const inst)
{
    if (inst->post_cmd_wait) {
        // Post command wait
        if ((millis - inst->wait_start) < MPU9250_CLOCK_WAIT_PERIOD) {
            // Still waiting
            return 0;
        }
        // Done, move to next state
        inst->post_cmd_wait = 0;
        inst->state = MPU9250_AG_ST_CONFIG_SENSORS;
        return 1;
    } else if (!inst->cmd_ready) {
        /* Marshal command to set CLKSEL to 1 in PWR_MGMT_1 register */
        inst->buffer[0] = MPU9250_PWR_MGMT_1_CLKSEL_AUTO;
        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_PWR_MGMT_1,
                                                      1, inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Prepare for post command wait
    inst->post_cmd_wait = 1;
    inst->wait_start = millis;
    return 0;
}


/** Write to USER_CTRL to reset and enable FIFO module */
static int mpu9250_handle_samp_acc_en_fifo(struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to reset and enable FIFO */
        inst->buffer[0] = (MPU9250_USER_CTRL_FIFO_RST |
                           MPU9250_USER_CTRL_FIFO_EN);
        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_USER_CTRL,
                                                      1, inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = MPU9250_SAMP_ACC_CONFIG_FIFO;
    return 1;
}


/** Write to FIFO_EN to enable writing of gyro x, y and z and accel data to
    FIFO */
static int mpu9250_handle_samp_acc_config_fifo(
                                            struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to enable writing of gyro x, y and z and accel data
           to FIFO */
        inst->buffer[0] = (MPU9250_FIFO_EN_ACCEL |
                           MPU9250_FIFO_EN_GYRO_ZOUT |
                           MPU9250_FIFO_EN_GYRO_YOUT |
                           MPU9250_FIFO_EN_GYRO_XOUT);
        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_FIFO_EN,
                                                      1, inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = MPU9250_SAMP_ACC_WAIT;
    return 1;
}


/** Wait for as many samples as we can fit in our buffer to be stored in FIFO */
static int mpu9250_handle_samp_acc_wait(struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        inst->wait_start = millis;

        // Calculate how many samples we need to wait for
        inst->samples_to_read = calc_num_samples_to_wait_for(inst->samples_left,
                                                0, MPU9250_AG_ACC_SAMPLE_LEN);
        // Add an extra millisecond of wait time, otherwise we would probably
        // almost always read one less sample than we want.
        inst->samples_to_read++;

        inst->cmd_ready = 1;
    }

    // Since we sample at 1 KHz for self test and calibration the wait time in
    // milliseconds is equal to the number of samples to be read

    if ((millis - inst->wait_start) < MS_TO_MILLIS(inst->samples_to_read)) {
        return 0;
    }

    // Move to next state
    inst->wait_start = millis;
    inst->state = MPU9250_SAMP_ACC_READ_COUNT;
    return 1;
}


/** Read FIFO_COUNT to check how many samples have been accumulated */
static int mpu9250_handle_samp_acc_read_count(struct mpu9250_desc_t *const inst)
{
    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_FIFO_COUNTH,
                                                      2, inst->buffer, 0, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Calculate how many samples we can read from the FIFO
    calc_samples_to_read(inst, inst->samples_left, MPU9250_AG_ACC_SAMPLE_LEN);

    // Move to next state
    inst->state = MPU9250_SAMP_ACC_READ_SAMPLES;
    return 1;
}


/** Read back the samples we have so far and sum them up */
static int mpu9250_handle_samp_acc_read_samples(
                                            struct mpu9250_desc_t *const inst)
{
    uint16_t const read_len = inst->samples_to_read * MPU9250_AG_ACC_SAMPLE_LEN;
    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_FIFO_R_W,
                                                      read_len, inst->buffer, 0,
                                                      0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Add the samples to the accumulators
    int16_t const sign = inst->acc_subtract ? -1 : 1;

    for (int i = 0; i < inst->samples_to_read; i++) {
        uint16_t const off = i * MPU9250_AG_ACC_SAMPLE_LEN;

        uint16_t const accel_x = ((((uint16_t)inst->buffer[off + 0]) << 8) |
                                  (uint16_t)inst->buffer[off + 1]);
        uint16_t const accel_y = ((((uint16_t)inst->buffer[off + 2]) << 8) |
                                  (uint16_t)inst->buffer[off + 3]);
        uint16_t const accel_z = ((((uint16_t)inst->buffer[off + 4]) << 8) |
                                  (uint16_t)inst->buffer[off + 5]);

        uint16_t const gyro_x = ((((uint16_t)inst->buffer[off + 6]) << 8) |
                                 (uint16_t)inst->buffer[off + 7]);
        uint16_t const gyro_y = ((((uint16_t)inst->buffer[off + 8]) << 8) |
                                 (uint16_t)inst->buffer[off + 9]);
        uint16_t const gyro_z = ((((uint16_t)inst->buffer[off + 10]) << 8) |
                                  (uint16_t)inst->buffer[off + 11]);

        inst->accel_accumulators[0] += sign * ((int16_t)accel_x);
        inst->accel_accumulators[1] += sign * ((int16_t)accel_y);
        inst->accel_accumulators[2] += sign * ((int16_t)accel_z);

        inst->gyro_accumulators[0] += sign * ((int16_t)gyro_x);
        inst->gyro_accumulators[1] += sign * ((int16_t)gyro_y);
        inst->gyro_accumulators[2] += sign * ((int16_t)gyro_z);
    }

    // Update count of samples left to read
    inst->samples_left -= inst->samples_to_read;

    if (inst->samples_left == 0) {
        // All done!
        inst->state = MPU9250_SAMP_ACC_DECONFIG_FIFO;
    } else {
        // Calculate wait period
        inst->samples_to_read = calc_num_samples_to_wait_for(inst->samples_left,
                                                    inst->extra_samples,
                                                    MPU9250_AG_ACC_SAMPLE_LEN);

        if (inst->samples_to_read == 0) {
            // There is already a full buffer worth of samples ready to go, jump
            // right to reading FIFO count
            inst->state = MPU9250_SAMP_ACC_READ_COUNT;
            return 1;
        }

        inst->samples_to_read++;
        // Set cmd_ready so that wait state will use already recorded wait start
        // time and the wait period we just calculated
        inst->cmd_ready = 1;
        // Go to wait state
        inst->state = MPU9250_SAMP_ACC_WAIT;
    }

    return 1;
}


/** Write to FIFO_EN to disable writing of gyro x, y and z and accel data to
    FIFO */
static int mpu9250_handle_samp_acc_deconfig_fifo(
                                            struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to disable writing of gyro x, y and z and accel data
            to FIFO */
        inst->buffer[0] = 0;
        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_FIFO_EN,
                                                      1, inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state (done with accumulation sequence)
    inst->state = inst->next_state;
    inst->next_state = MPU9250_FAILED;
    return 1;
}


/** Write to USER_CTRL to reset FIFO, I2C master and sensors, leave FIFO and
    I2C master disabled */
static int mpu9250_handle_user_reset(struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to reset reset FIFO, I2C master and sensors */
        inst->buffer[0] = (MPU9250_USER_CTRL_SIG_COND_RST |
                           MPU9250_USER_CTRL_I2C_MST_RST |
                           MPU9250_USER_CTRL_FIFO_RST);
        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_USER_CTRL,
                                                      1, inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state (done with user reset sequence)
    inst->state = inst->next_state;
    inst->next_state = MPU9250_FAILED;
    return 1;
}


/** Write to CONFIG, GYRO_CONFIG, ACCEL_CONFIG and ACCEL_CONFIG_2: accel and
    gyro DPLF configs to 2 and to zero out everything else */
static int mpu9250_handle_ag_st_config_sensors(
                                            struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to configure sensors for self test */
        // CONFIG
        inst->buffer[0] = MPU9250_CONFIG_DLPF_CFG(2);
        // GYRO_CONFIG
        inst->buffer[1] = MPU9250_GYRO_CONFIG_GYRO_FS_SEL_250;
        // ACCEL_CONFIG
        inst->buffer[2] = MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_2;
        // ACCEL_CONFIG_2
        inst->buffer[3] = MPU9250_ACCEL_CONFIG_2_A_DLPFCFG(2);

        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_CONFIG, 4,
                                                      inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Clear sample accumulators
    inst->accel_accumulators[0] = 0;
    inst->accel_accumulators[1] = 0;
    inst->accel_accumulators[2] = 0;
    inst->gyro_accumulators[0] = 0;
    inst->gyro_accumulators[1] = 0;
    inst->gyro_accumulators[2] = 0;

    // Start Accel/Gyro sample accumulation sequence to subtract 200 samples
    inst->samples_left = MPU9250_AG_ST_NUM_SAMPS;
    inst->acc_subtract = 1;
    inst->next_state = MPU9250_AG_ST_ENABLE_ST;

    inst->state = MPU9250_SAMP_ACC_EN_FIFO;
    return 1;
}


/** Write to GYRO_CONFIG and ACCEL_CONFIG to enable self test on all axes
    and wait 20 ms for sensor output to stabilize */
static int mpu9250_handle_ag_st_enable_st(struct mpu9250_desc_t *const inst)
{
    if (inst->post_cmd_wait) {
        if ((millis - inst->wait_start) < MPU9250_AG_ST_STABILIZE_PERIOD) {
            // Still waiting
            return 0;
        }

        // Start Accel/Gyro sample accumulation sequence to add 200 samples
        inst->samples_left = MPU9250_AG_ST_NUM_SAMPS;
        inst->acc_subtract = 0;
        inst->next_state = MPU9250_AG_ST_READ_ST_GYRO_OTP;

        inst->post_cmd_wait = 0;
        inst->state = MPU9250_SAMP_ACC_EN_FIFO;
        return 1;
    } else if (!inst->cmd_ready) {
        /* Marshal command to configure sensors for self test */
        // GYRO_CONFIG
        inst->buffer[0] = (MPU9250_GYRO_CONFIG_ZGYRO_CTEN |
                           MPU9250_GYRO_CONFIG_YGYRO_CTEN |
                           MPU9250_GYRO_CONFIG_XGYRO_CTEN);
        // ACCEL_CONFIG
        inst->buffer[1] = (MPU9250_ACCEL_CONFIG_AZ_ST_EN |
                           MPU9250_ACCEL_CONFIG_AY_ST_EN |
                           MPU9250_ACCEL_CONFIG_AX_ST_EN);

        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_GYRO_CONFIG,
                                                      2, inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Prepare for post command wait
    inst->post_cmd_wait = 1;
    inst->wait_start = millis;
    return 0;
}


/** Read SELF_TEST_*_GYRO into buffer[0-2] */
static int mpu9250_handle_ag_st_read_st_gyro_otp(
                                            struct mpu9250_desc_t *const inst)
{
    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_ST_X_GYRO, 3,
                                                      inst->buffer, 0, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move on to reading accelerometer factory trim values and calculating
    // result
    inst->state = MPU9250_AG_ST_READ_ST_ACCEL_OTP;
    return 1;
}


/** Read SELF_TEST_*_ACCEL into buffer[3-5] and check self test result */
static int mpu9250_handle_ag_st_read_st_accel_otp(
                                            struct mpu9250_desc_t *const inst)
{
    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_ST_X_ACCEL, 3,
                                                      inst->buffer + 3, 0, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Calculate averages
    int16_t const gxst = inst->gyro_accumulators[0] / MPU9250_AG_ST_NUM_SAMPS;
    int16_t const gyst = inst->gyro_accumulators[1] / MPU9250_AG_ST_NUM_SAMPS;
    int16_t const gzst = inst->gyro_accumulators[2] / MPU9250_AG_ST_NUM_SAMPS;

    int16_t const axst = inst->accel_accumulators[0] / MPU9250_AG_ST_NUM_SAMPS;
    int16_t const ayst = inst->accel_accumulators[1] / MPU9250_AG_ST_NUM_SAMPS;
    int16_t const azst = inst->accel_accumulators[2] / MPU9250_AG_ST_NUM_SAMPS;

    // Get OTP values
    uint8_t const gxst_otp = inst->buffer[0];
    uint8_t const gyst_otp = inst->buffer[1];
    uint8_t const gzst_otp = inst->buffer[2];

    uint8_t const axst_otp = inst->buffer[3];
    uint8_t const ayst_otp = inst->buffer[4];
    uint8_t const azst_otp = inst->buffer[5];

    // Check self test response for each axis
    int st_failed = 0;
    st_failed = st_failed || mpu9250_check_gyro_st(gxst, gxst_otp);
    st_failed = st_failed || mpu9250_check_gyro_st(gyst, gyst_otp);
    st_failed = st_failed || mpu9250_check_gyro_st(gzst, gzst_otp);

    st_failed = st_failed || mpu9250_check_accel_st(axst, axst_otp);
    st_failed = st_failed || mpu9250_check_accel_st(ayst, ayst_otp);
    st_failed = st_failed || mpu9250_check_accel_st(azst, azst_otp);

    if (st_failed) {
        inst->state = MPU9250_FAILED_AG_SELF_TEST;
        return 0;
    }

    // Reset sensors and FIFO before moving on to resetting magnetometer
    inst->state = MPU9250_USER_REST;
    inst->next_state = MPU9250_ENABLE_I2C_BYPASS;
    return 1;
}


/** Write to INT_PIN_CFG to enable I2C bypass */
static int mpu9250_handle_enable_i2c_bypass(struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to enable I2C bypass */
        inst->buffer[0] = MPU9250_INT_PIN_CFG_BYPASS_EN;
        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_INT_PIN_CFG,
                                                      1, inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = MPU9250_READ_MAG_WAI;
    return 1;
}


/** Read magnetometer WAI (should be 0x48) */
static int mpu9250_handle_read_mag_wai(struct mpu9250_desc_t *const inst)
{
    enum state_helper_result const res = do_reg_state(inst, AK8963_I2C_ADDR,
                                                      AK8963_REG_WIA, 1,
                                                      inst->buffer, 0, 1);

    if (res == STATE_RETRY) {
        // Failed to read from sensor over I2C bus
        inst->state = MPU9250_FAILED;
        return 0;
    } else if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Check WAI value
    if (inst->buffer[0] != AK8963_WHO_AM_I_VAL) {
        inst->state = MPU9250_FAILED_MAG_WAI;
        return 0;
    }

    // Move to next state
    inst->state = MPU9250_RESET_MAG;
    return 1;
}


/** Write to CNTL2 to reset magnetometer */
static int mpu9250_handle_reset_mag(struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to reset magnetometer */
        inst->buffer[0] = AK8963_CNTL2_SRST;
        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, AK8963_I2C_ADDR,
                                                      AK8963_REG_CNTL2, 1,
                                                      inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = MPU9250_MAG_SENS_ROM_ACC_MODE;
    return 1;
}


/** Write to CNTL1 to enter fuse ROM access mode */
static int mpu9250_handle_mag_sens_rom_acc_mode(
                                            struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to enter fuse ROM access mode */
        inst->buffer[0] = AK8963_CNTL1_MODE_FUSE_ROM_ACCESS;
        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, AK8963_I2C_ADDR,
                                                      AK8963_REG_CNTL1, 1,
                                                      inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = MPU9250_MAG_SENS_READ;
    return 1;
}


/** Read ASAX, ASAY and ASAZ */
static int mpu9250_handle_mag_sens_read(struct mpu9250_desc_t *const inst)
{
    enum state_helper_result const res = do_reg_state(inst, AK8963_I2C_ADDR,
                                                      AK8963_REG_ASAX, 3,
                                                      inst->buffer, 0, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    inst->mag_asa[0] = inst->buffer[0];
    inst->mag_asa[1] = inst->buffer[2];
    inst->mag_asa[2] = inst->buffer[3];

    // Move to next state
    inst->state = MPU9250_MAG_POWER_DOWN;
    inst->next_state = MPU9250_MAG_ST_ENABLE;
    return 1;
}


/** Write to CNTL1 to enter power down mode */
static int mpu9250_handle_mag_power_down(struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to enter power down mode */
        inst->buffer[0] = AK8963_CNTL1_MODE_POWER_DOWN;
        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, AK8963_I2C_ADDR,
                                                      AK8963_REG_CNTL1, 1,
                                                      inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state (end of magnetometer power down sequence)
    inst->state = inst->next_state;
    inst->next_state = MPU9250_FAILED;
    return 1;
}


/** Set SELF bit in ASTC register */
static int mpu9250_handle_mag_st_enable(struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to enable self test */
        inst->buffer[0] = AK8963_ASTC_SELF;
        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, AK8963_I2C_ADDR,
                                                      AK8963_REG_ASTC, 1,
                                                      inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = MPU9250_MAG_ST_ENTER_ST_MODE;
    return 1;
}


/** Write to CNTL1 to enter self test mode with 16 bit output */
static int mpu9250_handle_mag_st_enter_st_mode(
                                            struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to enter self test mode */
        inst->buffer[0] = (AK8963_CNTL1_MODE_SELF_TEST |
                           AK8963_CNTL1_BIT_16);
        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, AK8963_I2C_ADDR,
                                                      AK8963_REG_CNTL1, 1,
                                                      inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = MPU9250_MAG_ST_POLL;
    return 1;
}


/** Read ST1 to check if data ready, wait 1 ms if not, repeat until it is */
static int mpu9250_handle_mag_st_poll(struct mpu9250_desc_t *const inst)
{
    if (inst->post_cmd_wait) {
        // Wait between polls
        if ((millis - inst->wait_start) < MPU9250_MAG_POLL_PERIOD) {
            // Still waiting
            return 0;
        }
        // Done, poll again
        inst->post_cmd_wait = 0;
    }

    enum state_helper_result const res = do_reg_state(inst, AK8963_I2C_ADDR,
                                                      AK8963_REG_ST1, 1,
                                                      inst->buffer, 0, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    if (inst->buffer[0] & AK8963_ST1_DRDY) {
        // Data ready, move to next state
        inst->state = MPU9250_MAG_ST_READ;
        return 1;
    }

    // Prepare for inter-poll wait
    inst->post_cmd_wait = 1;
    inst->wait_start = millis;
    return 0;
}


/** Read data from HXL to HZH, check self test result */
static int mpu9250_handle_mag_st_read(struct mpu9250_desc_t *const inst)
{
    enum state_helper_result const res = do_reg_state(inst, AK8963_I2C_ADDR,
                                                      AK8963_REG_HXL, 7,
                                                      inst->buffer, 0, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Unpack samples
    int16_t const hx = (int16_t)((((uint16_t)inst->buffer[1]) << 8) |
                                 (uint16_t)inst->buffer[0]);
    int16_t const hy = (int16_t)((((uint16_t)inst->buffer[3]) << 8) |
                                 (uint16_t)inst->buffer[2]);
    int16_t const hz = (int16_t)((((uint16_t)inst->buffer[5]) << 8) |
                                 (uint16_t)inst->buffer[4]);

    // Check self test result
    int st_failed = 0;
    st_failed = st_failed || mpu9250_check_mag_st(0, hx, inst->mag_asa[0]);
    st_failed = st_failed || mpu9250_check_mag_st(1, hy, inst->mag_asa[1]);
    st_failed = st_failed || mpu9250_check_mag_st(2, hz, inst->mag_asa[2]);

    if (st_failed) {
        inst->state = MPU9250_FAILED_MAG_SELF_TEST;
        return 0;
    }

    // Move to next state
    inst->state = MPU9250_MAG_ST_DISABLE;
    return 1;
}


/** Clear SELF bit in ASTC register */
static int mpu9250_handle_mag_st_disable(struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to disable self test */
        inst->buffer[0] = 0;
        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, AK8963_I2C_ADDR,
                                                      AK8963_REG_ASTC, 1,
                                                      inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = MPU9250_MAG_POWER_DOWN;
    inst->next_state = MPU9250_AG_CAL_DISABLE_INT;
    return 1;
}


/** Write to INT_ENABLE to disable all interrupts (in case we are re-calibrating
    after having already been running for a while) */
static int mpu9250_handle_ag_cal_disable_int(struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to disable interrupts */
        inst->buffer[0] = 0;
        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_INT_ENABLE, 1,
                                                      inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = MPU9250_AG_CAL_DECONFIG_FIFO;
    return 1;
}


/** Write to FIFO_EN to disable writing of any data to FIFO */
static int mpu9250_handle_ag_cal_deconfig_fifo(
                                            struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to disable interrupts */
        inst->buffer[0] = 0;
        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_FIFO_EN, 1,
                                                      inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = MPU9250_USER_REST;
    inst->next_state = MPU9250_AG_CAL_CONFIG_SENSORS;
    return 1;
}


/** Write SMPLRT_DIV, CONFIG, GYRO_CONFIG, ACCEL_CONFIG and ACCEL_CONFIG_2 to
    sample at 1 KHz with a 184 Hz LPF for gyro and a 218.1 Hz LPF for accel, FSR
    of 250 degrees per second for gyro, 2 g for accel */
static int mpu9250_handle_ag_cal_config_sensors(
                                            struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to configure sensors for offset calibration */
        // SMPLRT_DIV
        inst->buffer[0] = 0;
        // CONFIG
        inst->buffer[1] = MPU9250_CONFIG_DLPF_CFG(1);
        // GYRO_CONFIG
        inst->buffer[2] = MPU9250_GYRO_CONFIG_GYRO_FS_SEL_250;
        // ACCEL_CONFIG
        inst->buffer[3] = MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_16;
        // ACCEL_CONFIG_2
        inst->buffer[4] = MPU9250_ACCEL_CONFIG_2_A_DLPFCFG(1);

        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_SMPLRT_DIV, 5,
                                                      inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Clear sample accumulators
    inst->accel_accumulators[0] = 0;
    inst->accel_accumulators[1] = 0;
    inst->accel_accumulators[2] = 0;
    inst->gyro_accumulators[0] = 0;
    inst->gyro_accumulators[1] = 0;
    inst->gyro_accumulators[2] = 0;

    // Start Accel/Gyro sample accumulation sequence to subtract 200 samples
    inst->samples_left = MPU9250_AG_CAL_NUM_SAMPS;
    inst->acc_subtract = 0;
    inst->next_state = MPU9250_AG_CAL_WRITE_GYRO_OFFS;

    inst->state = MPU9250_SAMP_ACC_EN_FIFO;
    return 1;
}


/** Calculate offset values and write XG_OFFSET_H through ZG_OFFSET_L */
static int mpu9250_handle_ag_cal_write_gyro_offs(
                                            struct mpu9250_desc_t *const inst)
{
    // See: InvenSense MPU Hardware Offset Registers Application Note

    if (!inst->cmd_ready) {
        /* Calculate gyro offsets */

        // Calculate averages and divide by 4 to get values in required units of
        // 32.8 LSB/dps (+-1000 dps FSR)
        inst->gyro_accumulators[0] /= (MPU9250_AG_CAL_NUM_SAMPS * 4);
        inst->gyro_accumulators[1] /= (MPU9250_AG_CAL_NUM_SAMPS * 4);
        inst->gyro_accumulators[2] /= (MPU9250_AG_CAL_NUM_SAMPS * 4);

        // Negate offsets
        int16_t const x_off = (int16_t)(-inst->gyro_accumulators[0]);
        int16_t const y_off = (int16_t)(-inst->gyro_accumulators[1]);
        int16_t const z_off = (int16_t)(-inst->gyro_accumulators[2]);

        // XG_OFFSET_H
        inst->buffer[0] = (uint8_t)(((uint16_t)x_off) >> 8);
        // XG_OFFSET_L
        inst->buffer[1] = (uint8_t)(((uint16_t)x_off) & 0xff);
        // YG_OFFSET_H
        inst->buffer[2] = (uint8_t)(((uint16_t)y_off) >> 8);
        // YG_OFFSET_L
        inst->buffer[3] = (uint8_t)(((uint16_t)y_off) & 0xff);
        // ZG_OFFSET_H
        inst->buffer[4] = (uint8_t)(((uint16_t)z_off) >> 8);
        // ZG_OFFSET_L
        inst->buffer[5] = (uint8_t)(((uint16_t)z_off) & 0xff);

        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_XG_OFFSET_H,
                                                      6, inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = MPU9250_AG_CAL_READ_ACCEL_OFFS;
    return 1;
}


/** Read XA_OFFSET_H through ZA_OFFSET_L (so that we can preserve the unused
    bits in these registers) */
static int mpu9250_handle_ag_cal_read_accel_offs(
                                            struct mpu9250_desc_t *const inst)
{
    // Note that XA_OFFSET_H, YA_OFFSET_H and ZA_OFFSET_H are not contiguous,
    // there is an extra register between XA_OFFSET_H and YA_OFFSET_H and
    // between YA_OFFSET_H and ZA_OFFSET_H. These registers are undocumented
    // and always seem to read as 0. We do eight byte reads and writes for the
    // accelerometer offsets and preserve the in between registers.
    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_XA_OFFSET_H,
                                                      8, inst->buffer, 0, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = MPU9250_AG_CAL_WRITE_ACCEL_OFFS;
    return 1;
}


/** Write XA_OFFSET_H through ZA_OFFSET_L */
static int mpu9250_handle_ag_cal_write_accel_offs(
                                            struct mpu9250_desc_t *const inst)
{
    // See: InvenSense MPU Hardware Offset Registers Application Note

    if (!inst->cmd_ready) {
        /* Get factory accelerometer biases */
        int16_t x_off = (int16_t)((((uint16_t)inst->buffer[0]) << 8) |
                                  (uint16_t)inst->buffer[1]);
        int16_t y_off = (int16_t)((((uint16_t)inst->buffer[3]) << 8) |
                                  (uint16_t)inst->buffer[4]);
        int16_t z_off = (int16_t)((((uint16_t)inst->buffer[6]) << 8) |
                                  (uint16_t)inst->buffer[7]);

        // Bit 0 of the factory value is preserved as it is used for temperature
        // compensation
        uint8_t const x_off_lsb = inst->buffer[1] & 1;
        uint8_t const y_off_lsb = inst->buffer[4] & 1;
        uint8_t const z_off_lsb = inst->buffer[7] & 1;

        /* Calculate accel offsets */

        // Calculate averages
        inst->accel_accumulators[0] /= MPU9250_AG_CAL_NUM_SAMPS;
        inst->accel_accumulators[1] /= MPU9250_AG_CAL_NUM_SAMPS;
        inst->accel_accumulators[2] /= MPU9250_AG_CAL_NUM_SAMPS;

        // Remove gravity from z-axis
        if (inst->accel_accumulators[2] > 0) {
            inst->accel_accumulators[2] -= 2048;
        } else {
            inst->accel_accumulators[2] += 2048;
        }

        // Subtract measured offsets from factory offsets
        if (__builtin_sub_overflow(x_off, inst->accel_accumulators[0],
                                   &x_off)) {
            x_off = INT16_MIN;
        }
        if (__builtin_sub_overflow(y_off, inst->accel_accumulators[1],
                                   &y_off)) {
            y_off = INT16_MIN;
        }
        if (__builtin_sub_overflow(z_off, inst->accel_accumulators[2],
                                   &z_off)) {
            z_off = INT16_MIN;
        }

        // XA_OFFS_H
        inst->buffer[0] = (uint8_t)(((uint16_t)x_off) >> 8);
        // XA_OFFS_L
        inst->buffer[1] = (uint8_t)(((uint16_t)x_off) & 0xfe) & x_off_lsb;
        // YA_OFFS_H
        inst->buffer[3] = (uint8_t)(((uint16_t)y_off) >> 8);
        // YA_OFFS_L
        inst->buffer[4] = (uint8_t)(((uint16_t)y_off) & 0xfe) & y_off_lsb;
        // ZA_OFFS_H
        inst->buffer[6] = (uint8_t)(((uint16_t)z_off) >> 8);
        // ZA_OFFS_L
        inst->buffer[7] = (uint8_t)(((uint16_t)z_off) & 0xfe) & z_off_lsb;

        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_XA_OFFSET_H,
                                                      8, inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = MPU9250_USER_REST;
    inst->next_state = MPU9250_MAG_ENABLE;
    return 1;
}


/** Write CNTL1 to select 8 or 100 Hz continuous mode with 16 bit resolution */
static int mpu9250_handle_mag_enable(struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to enter appropriate continuous mode with 16 bit
           resolution */
        inst->buffer[0] = AK8963_CNTL1_BIT_16;
        if (inst->mag_odr == AK8963_ODR_8HZ) {
            inst->buffer[0] |= AK8963_CNTL1_MODE_CONTINUOUS1; // 8 Hz
        } else {
            inst->buffer[0] |= AK8963_CNTL1_MODE_CONTINUOUS2; // 100 Hz
        }

        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, AK8963_I2C_ADDR,
                                                      AK8963_REG_CNTL1, 1,
                                                      inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = MPU9250_CONFIG_I2C_MST;
    return 1;
}


/** Write I2C_MST_CTRL, I2C_SLV0_ADDR, I2C_SLV0_REG and I2C_SLV0_CTRL to read 7
    bytes from magnetometer starting at HXL, I2C master configured for 400 KHz
    clock and to delay data ready interrupt until external sensor data is
    ready */
static int mpu9250_handle_config_i2c_mst(struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        // I2C_MST_CTRL
        inst->buffer[0] = (MPU9250_I2C_MST_CTRL_I2C_MST_CLK_400 |
                           MPU9250_I2C_MST_CTRL_WAIT_FOR_ES);
        // I2C_SLV0_ADDR
        inst->buffer[1] = (MPU9250_I2C_SLV0_ADDR_I2C_ID_0(AK8963_I2C_ADDR) |
                           MPU9250_I2C_SLV0_ADDR_I2C_SLV0_RNW_READ);
        // I2C_SLV0_REG
        inst->buffer[2] = AK8963_REG_HXL;
        // I2C_SLV0_CTRL
        inst->buffer[3] = (MPU9250_I2C_SLV0_CTRL_I2C_SLV0_LENG(7) |
                           MPU9250_I2C_SLV0_CTRL_I2C_SLV0_EN);

        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_I2C_MST_CTRL,
                                                      4, inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = MPU9250_ENABLE_I2C_MST_AND_FIFO;
    return 1;
}


/** Write to USER_CTRL to enable I2C master (and enable FIFO for FIFO driven
    operation) */
static int mpu9250_handle_enable_i2c_mst_and_fifo(
                                            struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        // USER_CTRL
        inst->buffer[0] = MPU9250_USER_CTRL_I2C_MST_EN;
        if (inst->use_fifo) {
            inst->buffer[0] |= MPU9250_USER_CTRL_FIFO_EN;
        }

        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_USER_CTRL, 1,
                                                      inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = MPU9250_AG_CONFIG_SENSORS;
    return 1;
}



/** Write to SMPLRT_DIV, CONFIG, GYRO_CONFIG, ACCEL_CONFIG and ACCEL_CONFIG_2 to
    configure DLPFs and sample rate */
static int mpu9250_handle_ag_config_sensors( struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        // SMPLRT_DIV
        inst->buffer[0] = inst->odr;
        // CONFIG
        inst->buffer[1] = 0;
        switch (inst->gyro_bw) {
            case MPU9250_GYRO_BW_5HZ:
                inst->buffer[1] |= MPU9250_CONFIG_DLPF_CFG(6);
                break;
            case MPU9250_GYRO_BW_10HZ:
                inst->buffer[1] |= MPU9250_CONFIG_DLPF_CFG(5);
                break;
            case MPU9250_GYRO_BW_20HZ:
                inst->buffer[1] |= MPU9250_CONFIG_DLPF_CFG(4);
                break;
            case MPU9250_GYRO_BW_41HZ:
                inst->buffer[1] |= MPU9250_CONFIG_DLPF_CFG(3);
                break;
            case MPU9250_GYRO_BW_92HZ:
                inst->buffer[1] |= MPU9250_CONFIG_DLPF_CFG(2);
                break;
            case MPU9250_GYRO_BW_184HZ:
                inst->buffer[1] |= MPU9250_CONFIG_DLPF_CFG(1);
                break;
            case MPU9250_GYRO_BW_250HZ:
                inst->buffer[1] |= MPU9250_CONFIG_DLPF_CFG(0);
                break;
        }
        // GYRO_CONFIG
        inst->buffer[2] = 0;
        switch (inst->gyro_fsr) {
            case MPU9250_GYRO_FSR_250DPS:
                inst->buffer[2] |= MPU9250_GYRO_CONFIG_GYRO_FS_SEL_250;
                break;
            case MPU9250_GYRO_FSR_500DPS:
                inst->buffer[2] |= MPU9250_GYRO_CONFIG_GYRO_FS_SEL_500;
                break;
            case MPU9250_GYRO_FSR_1000DPS:
                inst->buffer[2] |= MPU9250_GYRO_CONFIG_GYRO_FS_SEL_1000;
                break;
            case MPU9250_GYRO_FSR_2000DPS:
                inst->buffer[2] |= MPU9250_GYRO_CONFIG_GYRO_FS_SEL_2000;
                break;
        }
        // ACCEL_CONFIG
        inst->buffer[3] = 0;
        switch (inst->accel_fsr) {
            case MPU9250_ACCEL_FSR_2G:
                inst->buffer[3] |= MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_2;
                break;
            case MPU9250_ACCEL_FSR_4G:
                inst->buffer[3] |= MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_4;
                break;
            case MPU9250_ACCEL_FSR_8G:
                inst->buffer[3] |= MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_8;
                break;
            case MPU9250_ACCEL_FSR_16G:
                inst->buffer[3] |= MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_16;
                break;
        }
        // ACCEL_CONFIG_2
        inst->buffer[4] = 0;
        switch (inst->accel_bw) {
            case MPU9250_ACCEL_BW_5HZ:
                inst->buffer[4] |= MPU9250_ACCEL_CONFIG_2_A_DLPFCFG(6);
                break;
            case MPU9250_ACCEL_BW_10HZ:
                inst->buffer[4] |= MPU9250_ACCEL_CONFIG_2_A_DLPFCFG(5);
                break;
            case MPU9250_ACCEL_BW_21HZ:
                inst->buffer[4] |= MPU9250_ACCEL_CONFIG_2_A_DLPFCFG(4);
                break;
            case MPU9250_ACCEL_BW_45HZ:
                inst->buffer[4] |= MPU9250_ACCEL_CONFIG_2_A_DLPFCFG(3);
                break;
            case MPU9250_ACCEL_BW_99HZ:
                inst->buffer[4] |= MPU9250_ACCEL_CONFIG_2_A_DLPFCFG(2);
                break;
            case MPU9250_ACCEL_BW_218HZ:
                inst->buffer[4] |= MPU9250_ACCEL_CONFIG_2_A_DLPFCFG(1);
                break;
            case MPU9250_ACCEL_BW_420HZ:
                inst->buffer[4] |= MPU9250_ACCEL_CONFIG_2_A_DLPFCFG(7);
                break;
        }

        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_SMPLRT_DIV, 5,
                                                      inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    if (inst->use_fifo) {
        inst->state = MPU9250_AG_CONFIG_FIFO;
    } else {
        inst->state = MPU9250_AG_CONFIG_INT;
    }
    return 1;
}


/** Write to INT_PIN_CFG and INT_ENABLE to enable clearing of interrupt status
    when any register is read (leave I2C bypass enabled as well) and to enable
    raw data ready interrupt */
static int mpu9250_handle_ag_config_int(struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        // INT_PIN_CFG
        inst->buffer[0] = (MPU9250_INT_PIN_CFG_BYPASS_EN |
                           MPU9250_INT_PIN_CFG_ANYRD_2CLEAR);
        // INT_ENABLE
        inst->buffer[1] = MPU9250_INT_ENABLE_RAW_RDY_EN;

        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_INT_PIN_CFG,
                                                      2, inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = MPU9250_RUNNING;
    return 1;
}


/** Write to FIFO_EN to enable writing of gyro x, y and z, accel, temp and I2C
    slave 0 data to FIFO */
static int mpu9250_handle_ag_config_fifo(struct mpu9250_desc_t *const inst)
{
    if (!inst->cmd_ready) {
        // FIFO_EN
        inst->buffer[0] = (MPU9250_FIFO_EN_SLV_0 |
                           MPU9250_FIFO_EN_ACCEL |
                           MPU9250_FIFO_EN_GYRO_ZOUT |
                           MPU9250_FIFO_EN_GYRO_YOUT |
                           MPU9250_FIFO_EN_GYRO_XOUT |
                           MPU9250_FIFO_EN_TEMP_OUT);

        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_FIFO_EN, 1,
                                                      inst->buffer, 1, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->wait_start = millis;
    inst->samples_to_read = calc_num_samples_to_wait_for(0, 0,
                                                         MPU9250_SAMPLE_LEN);
    inst->state = MPU9250_FIFO_WAIT;
    return 1;
}


/** Reading data is handled by callbacks */
static int mpu9250_handle_running(struct mpu9250_desc_t *const inst)
{
    return 0;
}


/** Wait for samples to be written into FIFO */
static int mpu9250_handle_fifo_wait(struct mpu9250_desc_t *const inst)
{
    uint32_t const wait_period = (((uint32_t)inst->samples_to_read * 1000) /
                                  mpu9250_get_ag_odr(inst)) + 1;
    if ((millis - inst->wait_start) < wait_period) {
        // Not done waiting
        return 0;
    }

    inst->wait_start = millis;
    inst->next_sample_time = inst->wait_start;
    inst->state = MPU9250_FIFO_READ_COUNT;
    return 1;
}


/** Read FIFO count */
static int mpu9250_handle_fifo_read_count(struct mpu9250_desc_t *const inst)
{
    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_FIFO_COUNTH,
                                                      2, inst->buffer, 0, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Calculate how many samples we can read from the FIFO
    calc_samples_to_read(inst, 0, MPU9250_SAMPLE_LEN);

    // Move to next state
    inst->state = MPU9250_FIFO_READ;
    return 1;
}


/** Read samples from FIFO */
static int mpu9250_handle_fifo_reads(struct mpu9250_desc_t *const inst)
{
    uint16_t const read_len = inst->samples_to_read * MPU9250_SAMPLE_LEN;

    if (!inst->cmd_ready) {
        // Try to get a buffer from the telemetry service to put the data into
        inst->telem_buffer = NULL;
        if (inst->telem != NULL) {
            inst->telem_buffer = telemetry_post_mpu9250_imu(inst->telem,
                                                        inst->next_sample_time,
                                                        inst->odr,
                                                        inst->mag_odr,
                                                        inst->accel_fsr,
                                                        inst->gyro_fsr,
                                                        inst->accel_bw,
                                                        inst->gyro_bw,
                                                        read_len);
        }

        if (inst->telem_buffer == NULL) {
            inst->telem_buffer = inst->buffer;
        } else {
            inst->telemetry_buffer_checked_out = 1;
        }

        inst->cmd_ready = 1;
    }

    enum state_helper_result const res = do_reg_state(inst, inst->mpu9250_addr,
                                                      MPU9250_REG_FIFO_R_W,
                                                      read_len,
                                                      inst->telem_buffer, 0, 0);

    if (res != STATE_DONE) {
        // Not done yet
        return 0;
    }

    // Take the last sample and record it in the instance descriptor
    off_t const off = (inst->samples_to_read - 1) * MPU9250_SAMPLE_LEN;
    const uint8_t *const sample = inst->telem_buffer + off;
    parse_mpu9250_data(inst, sample);
    inst->last_sample_time = inst->next_sample_time;

    // Check in telemetry service buffer if we used one
    if (inst->telemetry_buffer_checked_out) {
        telemetry_finish_mpu9250_imu(inst->telem, inst->telem_buffer);
        inst->telemetry_buffer_checked_out = 0;
    }

    // Calculate how many samples we need to wait for
    inst->samples_to_read = calc_num_samples_to_wait_for(0, inst->extra_samples,
                                                         MPU9250_SAMPLE_LEN);

    // Go to wait state
    inst->state = MPU9250_FIFO_WAIT;
    return 1;
}




static int mpu9250_handle_failed(struct mpu9250_desc_t *const inst)
{
    return 0;
}





const mpu9250_state_handler_t mpu9250_state_handlers[] = {
    mpu9250_handle_read_ag_wai,             // MPU9250_READ_AG_WAI
// ##### Reset accel/gyro #####
    mpu9250_handle_soft_reset,              // MPU9250_SOFT_RESET
    mpu9250_handle_select_clock,            // MPU9250_SELECT_CLOCK
// Note: next state after MPU9250_SELECT_CLOCK is MPU9250_AG_ST_CONFIG_SENSORS
// ##### Accel/Gyro sample accumulation sequence ####
    mpu9250_handle_samp_acc_en_fifo,        // MPU9250_SAMP_ACC_EN_FIFO
    mpu9250_handle_samp_acc_config_fifo,    // MPU9250_SAMP_ACC_CONFIG_FIFO
    mpu9250_handle_samp_acc_wait,           // MPU9250_SAMP_ACC_WAIT
    mpu9250_handle_samp_acc_read_count,     // MPU9250_SAMP_ACC_READ_COUNT
    mpu9250_handle_samp_acc_read_samples,   // MPU9250_SAMP_ACC_READ_SAMPLES
    mpu9250_handle_samp_acc_deconfig_fifo,  // MPU9250_SAMP_ACC_DECONFIG_FIFO
// ##### User reset sequence #####
    mpu9250_handle_user_reset,              // MPU9250_USER_REST
// ##### Do accel/gyro self test #####
    mpu9250_handle_ag_st_config_sensors,    // MPU9250_AG_ST_CONFIG_SENSORS
// Do Accel/Gyro sample accumulation sequence to accumulate 200 samples
    mpu9250_handle_ag_st_enable_st,         // MPU9250_AG_ST_ENABLE_ST
// Do Accel/Gyro sample accumulation sequence to subtract 200 samples
    mpu9250_handle_ag_st_read_st_gyro_otp,  // MPU9250_AG_ST_READ_ST_GYRO_OTP
    mpu9250_handle_ag_st_read_st_accel_otp, // MPU9250_AG_ST_READ_ST_ACCEL_OTP
// Do user reset sequence
// ##### Reset magnetometer #####
    mpu9250_handle_enable_i2c_bypass,       // MPU9250_ENABLE_I2C_BYPASS
    mpu9250_handle_read_mag_wai,            // MPU9250_READ_MAG_WAI
    mpu9250_handle_reset_mag,               // MPU9250_RESET_MAG
// ##### Read magnetometer sensitivity adjustment registers #####
    mpu9250_handle_mag_sens_rom_acc_mode,   // MPU9250_MAG_SENS_ROM_ACC_MODE
    mpu9250_handle_mag_sens_read,           // MPU9250_MAG_SENS_READ
    mpu9250_handle_mag_power_down,          // MPU9250_MAG_POWER_DOWN
// ##### Self test magnetometer #####
    mpu9250_handle_mag_st_enable,           // MPU9250_MAG_ST_ENABLE
    mpu9250_handle_mag_st_enter_st_mode,    // MPU9250_MAG_ST_ENTER_ST_MODE
    mpu9250_handle_mag_st_poll,             // MPU9250_MAG_ST_POLL
    mpu9250_handle_mag_st_read,             // MPU9250_MAG_ST_READ
    mpu9250_handle_mag_st_disable,          // MPU9250_MAG_ST_DISABLE
// Do MPU9250_MAG_POWER_DOWN
// ##### Calibrate accel/gyro #####
    mpu9250_handle_ag_cal_disable_int,      // MPU9250_AG_CAL_DISABLE_INT
    mpu9250_handle_ag_cal_deconfig_fifo,    // MPU9250_AG_CAL_DECONFIG_FIFO
// Do user reset sequence
    mpu9250_handle_ag_cal_config_sensors,   // MPU9250_AG_CAL_CONFIG_SENSORS
// Do Accel/Gyro sample accumulation sequence to accumulate 200 samples
    mpu9250_handle_ag_cal_write_gyro_offs,  // MPU9250_AG_CAL_WRITE_GYRO_OFFS
    mpu9250_handle_ag_cal_read_accel_offs,  // MPU9250_AG_CAL_READ_ACCEL_OFFS
    mpu9250_handle_ag_cal_write_accel_offs, // MPU9250_AG_CAL_WRITE_ACCEL_OFFS
// Do user reset sequence
// ##### Initialize magnetometer for normal operation #####
    mpu9250_handle_mag_enable,              // MPU9250_MAG_ENABLE
// ##### Configure accel/gyro to read magnetometer #####
    mpu9250_handle_config_i2c_mst,          // MPU9250_CONFIG_I2C_MST
    mpu9250_handle_enable_i2c_mst_and_fifo, // MPU9250_ENABLE_I2C_MST_AND_FIFO
// ##### Initialize accel/gyro for normal operation #####
    mpu9250_handle_ag_config_sensors,       // MPU9250_AG_CONFIG_SENSORS
// For interrupt driven operation:
    mpu9250_handle_ag_config_int,           // MPU9250_AG_CONFIG_INT
// For FIFO driven operation:
    mpu9250_handle_ag_config_fifo,          // MPU9250_AG_CONFIG_FIFO

// ##### Normal operation (interrupt driven) #####
    mpu9250_handle_running,                 // MPU9250_RUNNING
// ##### Normal operation (FIFO driven) #####
    mpu9250_handle_fifo_wait,               // MPU9250_FIFO_WAIT
    mpu9250_handle_fifo_read_count,         // MPU9250_FIFO_READ_COUNT
    mpu9250_handle_fifo_reads,              // MPU9250_FIFO_READ

// ##### Failure states #####
    mpu9250_handle_failed,                  // MPU9250_FAILED
    mpu9250_handle_failed,                  // MPU9250_FAILED_AG_WAI
    mpu9250_handle_failed,                  // MPU9250_FAILED_MAG_WAI
    mpu9250_handle_failed,                  // MPU9250_FAILED_AG_SELF_TEST
    mpu9250_handle_failed                   // MPU9250_FAILED_MAG_SELF_TEST
};
