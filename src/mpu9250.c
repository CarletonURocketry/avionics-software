/**
 * @file mpu9250.c
 * @desc Driver for MPU9250 IMU
 * @author Samuel Dewan
 * @date 2021-09-29
 * Last Author:
 * Last Edited On:
 */

#include "mpu9250.h"
#include "mpu9250-states.h"
#include "mpu9250-registers.h"


// Interrupt handling functions
static void mpu9250_int_callback(void *context, union gpio_pin_t pin,
                                 uint8_t value);
static void mpu9250_i2c_callback(enum i2c_transaction_state const state,
                                 void *const context);



// MARK: Public Functions

int init_mpu9250(struct mpu9250_desc_t *inst,
                 struct sercom_i2c_desc_t *i2c_inst, uint8_t i2c_addr,
                 union gpio_pin_t int_pin, enum mpu9250_gyro_fsr gyro_fsr,
                 enum mpu9250_gyro_bw gyro_bw, enum mpu9250_accel_fsr accel_fsr,
                 enum mpu9250_accel_bw accel_bw, uint16_t ag_odr,
                 enum ak8963_odr mag_odr, int use_fifo)
{
    // Initialize instance descriptor
    inst->i2c_inst = i2c_inst;
    inst->mpu9250_addr = i2c_addr;
    inst->wait_start = 0;
    memset(inst->accel_accumulators, 0, 3 * sizeof(int32_t));
    memset(inst->gyro_accumulators, 0, 3 * sizeof(int32_t));
    memset(inst->mag_asa, 0, 3);
    inst->samples_to_read = 0;
    inst->extra_samples = 0;
    inst->samples_left = 0;
    inst->t_id = 0;
    inst->retry_count = 0;
    inst->state = MPU9250_READ_AG_WAI;
    inst->next_state = MPU9250_FAILED;
    inst->cmd_ready = 0;
    inst->i2c_in_progress = 0;
    inst->post_cmd_wait = 0;
    inst->acc_subtract = 0;
    inst->telemetry_buffer_checked_out = 0;
    inst->telem = NULL;

    // Store settings
    inst->mag_odr = mag_odr;
    inst->gyro_fsr = gyro_fsr;
    inst->accel_fsr = accel_fsr;
    inst->gyro_bw = gyro_bw;
    inst->accel_bw = accel_bw;

    int16_t const odr_reg_val = (1000 / ag_odr) - 1;;
    if ((odr_reg_val < 0) || (odr_reg_val > 255)) {
        // Invalid ODR
        return 1;
    }
    inst->odr = (uint8_t)odr_reg_val;

    inst->use_fifo = !!use_fifo;

    // Configure interrupt pin
    if (!use_fifo) {
        int ret = gpio_set_pin_mode(int_pin, GPIO_PIN_INPUT);
        if (ret != 0) {
            inst->state = MPU9250_FAILED;
            return 1;
        }

        ret = gpio_enable_interrupt(int_pin, GPIO_INTERRUPT_RISING_EDGE, 0,
                                    mpu9250_int_callback, inst);
        if (ret != 0) {
            inst->state = MPU9250_FAILED;
            return 1;
        }
    }

    // Clear last sample fields
    inst->last_sample_time = 0;
    inst->last_accel_x = 0;
    inst->last_accel_y = 0;
    inst->last_accel_z = 0;
    inst->last_gyro_x = 0;
    inst->last_gyro_y = 0;
    inst->last_gyro_z = 0;
    inst->last_temp = 0;
    inst->last_mag_x = 0;
    inst->last_mag_y = 0;
    inst->last_mag_z = 0;
    inst->last_mag_overflow = 0;

    return 0;
}

void mpu9250_service(struct mpu9250_desc_t *inst)
{
    int do_next_state;
    do {
        // Check for ongoing I2C transaction
        if (inst->i2c_in_progress &&
            !sercom_i2c_transaction_done(inst->i2c_inst, inst->t_id)) {
            // Waiting for an I2C transaction to complete
            return;
        }

        do_next_state = mpu9250_state_handlers[inst->state](inst);
    } while (do_next_state);
}

int32_t mpu9250_get_temperature(const struct mpu9250_desc_t *inst)
{
    int32_t const t_val = (int32_t)inst->last_temp - MPU9250_TEMP_ROOM_OFFSET;
    return ((1000 * t_val) / MPU9250_TEMP_SENSITIVITY) + 21000;
}








// MARK: Callbacks

static void mpu9250_int_callback(void *const context,
                                 union gpio_pin_t const pin,
                                 uint8_t const value)
{
    struct mpu9250_desc_t *const inst = (struct mpu9250_desc_t *)context;

    if (inst->state != MPU9250_RUNNING) {
        return;
    }

    inst->next_sample_time = millis;

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
                                                        MPU9250_SAMPLE_LEN);
    }

    if (inst->telem_buffer == NULL) {
        inst->telem_buffer = inst->buffer;
    } else {
        inst->telemetry_buffer_checked_out = 1;
    }

    inst->async_i2c_in_progress = !sercom_i2c_start_reg_read_with_cb(
                                                    inst->i2c_inst, &inst->t_id,
                                                    inst->mpu9250_addr,
                                                    MPU9250_REG_ACCEL_XOUT_H,
                                                    inst->telem_buffer,
                                                    MPU9250_SAMPLE_LEN,
                                                    mpu9250_i2c_callback, inst);

    if ((!inst->async_i2c_in_progress) && (inst->telem_buffer != NULL)) {
        // Failed to start I2C transaction
        memset(inst->telem_buffer, 0, MPU9250_SAMPLE_LEN);
        telemetry_finish_mpu9250_imu(inst->telem, inst->telem_buffer);
        inst->telemetry_buffer_checked_out = 0;
    }
}

static void mpu9250_i2c_callback(enum i2c_transaction_state const state,
                                 void *const context)
{
    struct mpu9250_desc_t *const inst = (struct mpu9250_desc_t *)context;

    if (state != I2C_STATE_DONE) {
        // Something went wrong!
        return;
    } else if (!inst->async_i2c_in_progress) {
        return;
    }

    inst->async_i2c_in_progress = 0;
    parse_mpu9250_data(inst, inst->telem_buffer);
    inst->last_sample_time = inst->next_sample_time;

    // Check in telemetry buffer if we used one
    if (inst->telemetry_buffer_checked_out) {
        telemetry_finish_mpu9250_imu(inst->telem, inst->telem_buffer);
        inst->telemetry_buffer_checked_out = 0;
    }
}
