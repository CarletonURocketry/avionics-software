/**
 * @file kx134-1211-states.c
 * @desc Driver state machine for KX134-1211 accelerometer
 * @author Samuel Dewan
 * @date 2021-07-10
 * Last Author:
 * Last Edited On:
 */

#include "kx134-1211-states.h"
#include "kx134-1211-registers.h"

// MARK: Constants
#define KX134_1211_POWER_ON_DELAY           MS_TO_MILLIS(50)
#define KX134_1211_SW_RESET_DELAY           MS_TO_MILLIS(5)
#define KX134_1211_ST_ENABLE_DELAY          MS_TO_MILLIS(50)

#define KX134_1211_ST_RSP_MIN               100 // (0.1 * 1000)
#define KX134_1211_ST_RSP_TYP               500 // (0.5 * 1000)
#define KX134_1211_ST_RSP_MAX               900 // (0.9 * 1000)


// MARK: Helpers

static uint8_t get_odcntl_reg_val(struct kx134_1211_desc_t *inst)
{
    uint8_t odcntl;

    switch (inst->odr) {
        case KX134_1211_ODR_781:
            odcntl = KX134_1211_ODCNTL_OSA_0_781;
            break;
        case KX134_1211_ODR_1563:
            odcntl = KX134_1211_ODCNTL_OSA_1_563;
            break;
        case KX134_1211_ODR_3125:
            odcntl = KX134_1211_ODCNTL_OSA_3_125;
            break;
        case KX134_1211_ODR_6250:
            odcntl = KX134_1211_ODCNTL_OSA_6_25;
            break;
        case KX134_1211_ODR_12500:
            odcntl = KX134_1211_ODCNTL_OSA_12_5;
            break;
        case KX134_1211_ODR_25000:
            odcntl = KX134_1211_ODCNTL_OSA_25;
            break;
        case KX134_1211_ODR_50000:
            odcntl = KX134_1211_ODCNTL_OSA_50;
            break;
        case KX134_1211_ODR_100000:
            odcntl = KX134_1211_ODCNTL_OSA_100;
            break;
        case KX134_1211_ODR_200000:
            odcntl = KX134_1211_ODCNTL_OSA_200;
            break;
        case KX134_1211_ODR_400000:
            odcntl = KX134_1211_ODCNTL_OSA_400;
            break;
        case KX134_1211_ODR_800000:
            odcntl = KX134_1211_ODCNTL_OSA_800;
            break;
        case KX134_1211_ODR_1600000:
            odcntl = KX134_1211_ODCNTL_OSA_1600;
            break;
        case KX134_1211_ODR_3200000:
            odcntl = KX134_1211_ODCNTL_OSA_3200;
            break;
        case KX134_1211_ODR_6400000:
            odcntl = KX134_1211_ODCNTL_OSA_6400;
            break;
        case KX134_1211_ODR_12800000:
            odcntl = KX134_1211_ODCNTL_OSA_12800;
            break;
        case KX134_1211_ODR_25600000:
            odcntl = KX134_1211_ODCNTL_OSA_25600;
            break;
        default:
            odcntl = 0;
    }

    odcntl |= (((inst->rolloff == KX134_1211_LOW_PASS_ROLLOFF_9) ?
                KX134_1211_ODCNTL_LPRO_IR_CFF_ODR_9 :
                KX134_1211_ODCNTL_LPRO_IR_CFF_ODR_2) |
               KX134_1211_ODCNTL_FSTUP);

    return odcntl;
}


enum do_state_result {
    DO_STATE_RESULT_DONE,
    DO_STATE_RESULT_LATER
};

static enum do_state_result do_state(struct kx134_1211_desc_t *inst,
                                     uint32_t delay, uint16_t bytes_out,
                                     uint16_t bytes_in)
{
    if ((delay != 0) && !inst->delay_done) {
        if ((millis - inst->init_delay_start_time) < delay) {
            // Still waiting
            return DO_STATE_RESULT_LATER;
        }
    }

    if (!inst->spi_in_progress) {
        // Need to start SPI transaction
        uint8_t const ret = sercom_spi_start(inst->spi_inst, &inst->t_id,
                                             KX134_1211_BAUDRATE,
                                             inst->cs_pin_group,
                                             inst->cs_pin_mask, inst->buffer,
                                             bytes_out, inst->buffer, bytes_in);

        inst->spi_in_progress = ret == 0;
        return DO_STATE_RESULT_LATER;
    } else {
        // SPI transaction is done, clean up for next time
        inst->delay_done = 0;
        inst->cmd_ready = 0;
        inst->spi_in_progress = 0;

        return DO_STATE_RESULT_DONE;
    }
}



// MARK: State Handlers

/**
 *  Wait for device to boot (typical 20 ms, max 50 ms) then write 0 to
 *  mysterious register 0x7f.
 */
static int kx134_1211_handle_power_on(struct kx134_1211_desc_t *inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to write 0 to register 0x7f */
        // Write to 0x7f
        inst->buffer[0] = KX134_1211_REG_MYSTERY_RST | KX134_1211_WRITE;
        inst->buffer[1] = 0;

        inst->cmd_ready = 1;
    }

    enum do_state_result const res = do_state(inst, KX134_1211_POWER_ON_DELAY,
                                              2, 0);

    if (res != DO_STATE_RESULT_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = KX134_1211_CLEAR_CNTL2;
    return 1;
}


/**
 *  Clear CNTL2 register.
 */
static int kx134_1211_handle_clear_cntl2(struct kx134_1211_desc_t *inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to write 0 to CNTL2 */
        // Write to CNTL2
        inst->buffer[0] = KX134_1211_REG_CNTL2 | KX134_1211_WRITE;
        inst->buffer[1] = 0;

        inst->cmd_ready = 1;
    }

    enum do_state_result const res = do_state(inst, 0, 2, 0);

    if (res != DO_STATE_RESULT_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = KX134_1211_SOFTWARE_RESET;
    return 1;
}


/**
 *  Write 0x80 to CNTL2 to initiate software reset.
 */
static int kx134_1211_handle_software_reset(struct kx134_1211_desc_t *inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to write 0x8 to CNTL2 */
        // Write to CNTL2
        inst->buffer[0] = KX134_1211_REG_CNTL2 | KX134_1211_WRITE;
        inst->buffer[1] = KX134_1211_CNTL2_SRST;

        inst->cmd_ready = 1;
    }

    enum do_state_result const res = do_state(inst, 0, 2, 0);

    if (res != DO_STATE_RESULT_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->init_delay_start_time = millis;
    inst->state = KX134_1211_CHECK_WAI;
    return 1;
}


/**
 *  Wait for software reset to complete (minimum 2 ms) then check Who Am I
 *  register (should be 0x46).
 */
static int kx134_1211_handle_check_wai(struct kx134_1211_desc_t *inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to read from Who Am I register */
        // Read from Who Am I
        inst->buffer[0] = KX134_1211_REG_WHO_AM_I | KX134_1211_READ;

        inst->cmd_ready = 1;
    }

    enum do_state_result const res = do_state(inst, KX134_1211_SW_RESET_DELAY,
                                              1, 1);

    if (res != DO_STATE_RESULT_DONE) {
        // Not done yet
        return 0;
    }

    // Check WAI value
    if (inst->buffer[0] != KX134_1211_WHO_AM_I_VAL) {
        inst->state = KX134_1211_FAILED_WAI;
        return 0;
    }

    // Move to next state
    inst->state = KX134_1211_CHECK_COTR;
    return 1;
}


/**
 *  Check command test response register (should be 0x55)
 */
static int kx134_1211_handle_check_cotr(struct kx134_1211_desc_t *inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to read from command test response register  */
        // Read from COTR
        inst->buffer[0] = KX134_1211_REG_COTR | KX134_1211_READ;

        inst->cmd_ready = 1;
    }

    enum do_state_result const res = do_state(inst, 0, 1, 1);

    if (res != DO_STATE_RESULT_DONE) {
        // Not done yet
        return 0;
    }

    // Check COTR value
    if (inst->buffer[0] != KX134_1211_COTR_DEFAULT_VAL) {
        inst->state = KX134_1211_FAILED_COTR;
        return 0;
    }

    // Move to next state
    inst->state = KX134_1211_ENABLE_ACCEL;
    inst->en_next_state = KX134_1211_READ_ST_OFF;
    return 1;
}


/**
 *  Write CNTL1 to enable accelerometer.
 */
static int kx134_1211_handle_enable_accel(struct kx134_1211_desc_t *inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to enable accelerometer */
        // Write to CNTL1
        inst->buffer[0] = KX134_1211_REG_CNTL1 | KX134_1211_WRITE;

        switch (inst->range) {
            case KX134_1211_RANGE_8G:
                inst->buffer[1] = KX134_1211_CNTL1_GSEL_8G;
                break;
            case KX134_1211_RANGE_16G:
                inst->buffer[1] = KX134_1211_CNTL1_GSEL_16G;
                break;
            case KX134_1211_RANGE_32G:
                inst->buffer[1] = KX134_1211_CNTL1_GSEL_32G;
                break;
            case KX134_1211_RANGE_64G:
                inst->buffer[1] = KX134_1211_CNTL1_GSEL_64G;
                break;
        }
        inst->buffer[1] |= (KX134_1211_CNTL1_RES_HIGH |
                            KX134_1211_CNTL1_PC1);

        inst->cmd_ready = 1;
    }

    enum do_state_result const res = do_state(inst, 0, 2, 0);

    if (res != DO_STATE_RESULT_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->init_delay_start_time = millis;
    inst->state = inst->en_next_state;
    inst->en_next_state = KX134_1211_FAILED;
    return 1;
}


/**
 *  Wait for accelerometer to be ready (min 21.6 ms at ODR = 50, round up to
 *  50 ms to make sure (since 1/ODR = 20 ms)) then take a reading with self test
 *  off.
 */
static int kx134_1211_handle_read_st_off(struct kx134_1211_desc_t *inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to read from data out registers */
        // Read starting from XOUT_L
        inst->buffer[0] = KX134_1211_REG_XOUT_L | KX134_1211_READ;

        inst->cmd_ready = 1;
    }

    enum do_state_result const res = do_state(inst, KX134_1211_ST_ENABLE_DELAY,
                                              1, 6);

    if (res != DO_STATE_RESULT_DONE) {
        // Not done yet
        return 0;
    }

    // Store acceleration values
    inst->last_x = *((int16_t*)(__builtin_assume_aligned(&inst->buffer[0], 2)));
    inst->last_y = *((int16_t*)(__builtin_assume_aligned(&inst->buffer[2], 2)));
    inst->last_z = *((int16_t*)(__builtin_assume_aligned(&inst->buffer[4], 2)));

    // Move to next state
    inst->state = KX134_1211_DISABLE_ACCEL;
    inst->en_next_state = KX134_1211_ENABLE_SELF_TEST;
    return 1;
}


/**
 *  Write CNTL1 to disable accelerometer.
 */
static int kx134_1211_handle_disable_accel(struct kx134_1211_desc_t *inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to write 0 to CNTL1 */
        // Write to CNTL1
        inst->buffer[0] = KX134_1211_REG_CNTL1 | KX134_1211_WRITE;
        inst->buffer[1] = 0;

        inst->cmd_ready = 1;
    }

    enum do_state_result const res = do_state(inst, 0, 2, 0);

    if (res != DO_STATE_RESULT_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = inst->en_next_state;
    inst->en_next_state = KX134_1211_FAILED;
    return 1;
}


/**
 *  Write 0xCA to SELF_TEST register to enable self test.
 */
static int kx134_1211_handle_enable_self_test(struct kx134_1211_desc_t *inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to write 0xCA to SELF_TEST */
        // Write to SELF_TEST
        inst->buffer[0] = KX134_1211_REG_SELF_TEST | KX134_1211_WRITE;
        inst->buffer[1] = KX134_1211_REG_SELF_TEST_ENABLE_VAL;

        inst->cmd_ready = 1;
    }

    enum do_state_result const res = do_state(inst, 0, 2, 0);

    if (res != DO_STATE_RESULT_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = KX134_1211_ENABLE_ACCEL;
    inst->en_next_state = KX134_1211_READ_ST_ON;
    return 1;
}


/**
 *  Wait for accelerometer to be ready (min 21.6 ms at ODR = 50, round up to
 *  50 ms to make sure (since 1/ODR = 20 ms)) then take a reading with self test
 *  on.
 */
static int kx134_1211_handle_read_st_on(struct kx134_1211_desc_t *inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to read from data out registers */
        // Read starting from XOUT_L
        inst->buffer[0] = KX134_1211_REG_XOUT_L | KX134_1211_READ;

        inst->cmd_ready = 1;
    }

    enum do_state_result const res = do_state(inst, KX134_1211_ST_ENABLE_DELAY,
                                              1, 6);

    if (res != DO_STATE_RESULT_DONE) {
        // Not done yet
        return 0;
    }

    // Parse acceleration values
    int16_t const st_x = *((int16_t*)(__builtin_assume_aligned(&inst->buffer[0],
                                                               2)));
    int16_t const st_y = *((int16_t*)(__builtin_assume_aligned(&inst->buffer[2],
                                                               2)));
    int16_t const st_z = *((int16_t*)(__builtin_assume_aligned(&inst->buffer[4],
                                                               2)));

    // Calculate self test response for each axis
    uint32_t const rsp_x = (((uint32_t)(st_x - inst->last_x) * 1000) /
                            inst->sensitivity);
    uint32_t const rsp_y = (((uint32_t)(st_y - inst->last_y) * 1000) /
                            inst->sensitivity);
    uint32_t const rsp_z = (((uint32_t)(st_z - inst->last_z) * 1000) /
                            inst->sensitivity);

    // Check that self test responses are within acceptable range
    if (((rsp_x < KX134_1211_ST_RSP_MIN) || (rsp_x > KX134_1211_ST_RSP_MAX)) ||
        ((rsp_y < KX134_1211_ST_RSP_MIN) || (rsp_y > KX134_1211_ST_RSP_MAX)) ||
        ((rsp_z < KX134_1211_ST_RSP_MIN) || (rsp_z > KX134_1211_ST_RSP_MAX))) {
        // Self test failed
        inst->en_next_state = KX134_1211_FAILED_SELF_TEST;
    } else {
        // Self test passed
        inst->en_next_state = KX134_1211_CONFIG_BUFFER;
    }

    // Move to next state
    inst->state = KX134_1211_DISABLE_ACCEL;
    return 1;
}


/**
 *  Write SELF_TEST, BUF_CNTL1 and BUF_CNTL2 to disable self test and configure
 *  buffer in stream mode.
 */
static int kx134_1211_handle_config_buffer(struct kx134_1211_desc_t *inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to write to SELF_TEST, BUF_CNTL1 and BUF_CNTL2 */
        // Write starting at SELF_TEST
        inst->buffer[0] = KX134_1211_REG_SELF_TEST | KX134_1211_WRITE;
        // SELF_TEST
        inst->buffer[1] = 0;
        // BUF_CNTL1: Sample threshold
        inst->buffer[2] = ((inst->resolution == KX134_1211_RES_8_BIT) ?
                           KX134_1211_SAMPLE_THRESHOLD_8BIT :
                           KX134_1211_SAMPLE_THRESHOLD_16BIT);
        // BUF_CNTL2
        // Configure in stream mode with correct resolution and enable
        inst->buffer[3] = (KX134_1211_BUF_CNTL2_BM_STREAM |
                           ((inst->resolution == KX134_1211_RES_8_BIT) ?
                            KX134_1211_BUF_CNTL2_BRES_8 :
                            KX134_1211_BUF_CNTL2_BRES_16) |
                           KX134_1211_BUF_CNTL2_BUFE);

        inst->cmd_ready = 1;
    }

    enum do_state_result const res = do_state(inst, 0, 4, 0);

    if (res != DO_STATE_RESULT_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = KX134_1211_CONFIG;
    return 1;
}


/**
 *  Write CNTL2 through CNTL6, ODCNTL and INC1 though INC6 to configure sensor.
 */
static int kx134_1211_handle_config(struct kx134_1211_desc_t *inst)
{
    if (!inst->cmd_ready) {
        /* Marshal command to write to CNTL2 through CNTL6, ODCNTL and INC1
           though INC6 */
        // Write starting at CNTL2
        inst->buffer[0] = KX134_1211_REG_CNTL2 | KX134_1211_WRITE;
        // CNTL2 (disable all tile axes)
        inst->buffer[1] = 0;
        // CNTL3 (do not change)
        inst->buffer[2] = KX134_1211_REG_CNTL3_RST_VAL;
        // CNTL4 (do not change)
        inst->buffer[3] = KX134_1211_REG_CNTL4_RST_VAL;
        // CNTL5 (do not change)
        inst->buffer[4] = KX134_1211_REG_CNTL5_RST_VAL;
        // CNTL6 (do not change)
        inst->buffer[5] = KX134_1211_REG_CNTL6_RST_VAL;
        // ODCNTL (configure ODR and low-pass filter)
        inst->buffer[6] = get_odcntl_reg_val(inst);
        // INC1 (configure int pin 1 as enabled, pulsed and active high)
        inst->buffer[7] = (KX134_1211_INC1_IEA1_HIGH |
                           KX134_1211_INC1_IEN1 |
                           KX134_1211_INC1_PW1_50U);
        // INC2 (disable wake up and back to sleep interrupts)
        inst->buffer[8] = 0;
        // INC3 (disable tap/double tap interrupts)
        inst->buffer[9] = 0;
        // INC4 (route watermark interrupt to pin 1)
        inst->buffer[10] = KX134_1211_INC4_WMI1;
        // INC5 (do not change)
        inst->buffer[11] = KX134_1211_REG_INC5_RST_VAL;
        // INC6 (do not route any interrupts to pin 2)
        inst->buffer[12] = 0;

        inst->cmd_ready = 1;
    }

    enum do_state_result const res = do_state(inst, 0, 13, 0);

    if (res != DO_STATE_RESULT_DONE) {
        // Not done yet
        return 0;
    }

    // Move to next state
    inst->state = KX134_1211_ENABLE_ACCEL;
    inst->en_next_state = KX134_1211_RUNNING;
    return 1;
}


static int kx134_1211_handle_running(struct kx134_1211_desc_t *inst)
{
    return 0;
}

int kx134_1211_handle_read_buffer(struct kx134_1211_desc_t *inst)
{
    inst->last_reading_time = millis;

    // TODO: Check out a buffer of some kind from logging service

    inst->buffer[0] = KX134_1211_REG_BUF_READ | KX134_1211_READ;

    uint16_t const in_length = ((inst->resolution == KX134_1211_RES_8_BIT) ?
                                (KX134_1211_SAMPLE_THRESHOLD_8BIT * 3) :
                                (KX134_1211_SAMPLE_THRESHOLD_16BIT * 6));

    uint8_t const ret = sercom_spi_start_with_cb(inst->spi_inst, &inst->t_id,
                                                 KX134_1211_BAUDRATE,
                                                 inst->cs_pin_group,
                                                 inst->cs_pin_mask,
                                                 inst->buffer, 1, inst->buffer,
                                                 in_length,
                                                 kx134_1211_spi_callback, inst);

    if (ret == 0) {
        // Success fully queued SPI transaction, end of transaction will be
        // handled in 
        inst->state = KX134_1211_RUNNING;
    }

    return 0;
}


static int kx134_1211_handle_failed(struct kx134_1211_desc_t *inst)
{
    return 0;
}





const kx134_1211_state_handler_t kx134_1211_state_handlers[] = {
    kx134_1211_handle_power_on,                 // KX134_1211_POWER_ON
    kx134_1211_handle_clear_cntl2,              // KX134_1211_CLEAR_CNTL2
    kx134_1211_handle_software_reset,           // KX134_1211_SOFTWARE_RESET
    kx134_1211_handle_check_wai,                // KX134_1211_CHECK_WAI
    kx134_1211_handle_check_cotr,               // KX134_1211_CHECK_COTR
    kx134_1211_handle_enable_accel,             // KX134_1211_ENABLE_ACCEL
    kx134_1211_handle_read_st_off,              // KX134_1211_READ_ST_OFF
    kx134_1211_handle_disable_accel,            // KX134_1211_DISABLE_ACCEL
    kx134_1211_handle_enable_self_test,         // KX134_1211_ENABLE_SELF_TEST
    kx134_1211_handle_read_st_on,               // KX134_1211_READ_ST_ON
    kx134_1211_handle_config_buffer,            // KX134_1211_CONFIG_BUFFER
    kx134_1211_handle_config,                   // KX134_1211_CONFIG
    kx134_1211_handle_running,                  // KX134_1211_RUNNING
    kx134_1211_handle_failed,                   // KX134_1211_FAILED
    kx134_1211_handle_failed,                   // KX134_1211_FAILED_WAI
    kx134_1211_handle_failed,                   // KX134_1211_FAILED_COTR
    kx134_1211_handle_failed,                   // KX134_1211_FAILED_SELF_TEST
};

