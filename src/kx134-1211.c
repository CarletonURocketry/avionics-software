/**
 * @file kx134-1211.c
 * @desc Driver for KX134-1211 accelerometer
 * @author Samuel Dewan
 * @date 2021-07-09
 * Last Author:
 * Last Edited On:
 */

#include "kx134-1211.h"
#include "kx134-1211-states.h"
#include "kx134-1211-registers.h"

// Interrupt handling functions
static void kx134_1211_int1_callback(void *context, union gpio_pin_t pin,
                                     uint8_t value);


// MARK: Public Functions

void init_kx134_1211 (struct kx134_1211_desc_t *inst,
                      struct sercom_spi_desc_t *spi_inst,
                      uint8_t cs_pin_group, uint32_t cs_pin_mask,
                      union gpio_pin_t int1_pin,
                      union gpio_pin_t int2_pin,
                      enum kx134_1211_range range,
                      enum kx134_1211_low_pass_rolloff rolloff,
                      enum kx134_1211_odr odr,
                      enum kx134_1211_resolution resolution)
{
    // Configure instance descriptor
    inst->spi_inst = spi_inst;
    inst->telem = NULL;
    inst->cs_pin_group = cs_pin_group;
    inst->cs_pin_mask = cs_pin_mask;
    inst->state = KX134_1211_POWER_ON;
    inst->en_next_state = KX134_1211_FAILED;
    inst->range = range;
    inst->rolloff = rolloff;
    inst->odr = odr;
    inst->resolution = resolution;
    inst->last_reading_time = 0;
    inst->delay_done = 0;
    inst->cmd_ready = 0;
    inst->spi_in_progress = 0;

    // Configure interrupt pin
    gpio_set_pin_mode(int1_pin, GPIO_PIN_INPUT);
    gpio_enable_interrupt(int1_pin, GPIO_INTERRUPT_RISING_EDGE, 0,
                          kx134_1211_int1_callback, inst);

    // Reset process will start in service function after delay
    inst->init_delay_start_time = millis;

    // Calculate sensitivity
    uint16_t max_counts;
    if (inst->resolution == KX134_1211_RES_8_BIT) {
        max_counts = INT8_MAX + 1;
    } else {
        max_counts = INT16_MAX + 1;
    }

    switch (inst->range) {
        case KX134_1211_RANGE_8G:
            inst->sensitivity = max_counts / 8;
            break;
        case KX134_1211_RANGE_16G:
            inst->sensitivity = max_counts / 16;
            break;
        case KX134_1211_RANGE_32G:
            inst->sensitivity = max_counts / 32;
            break;
        case KX134_1211_RANGE_64G:
            inst->sensitivity = max_counts / 64;
            break;
        default:
            inst->sensitivity = 0;
            break;
    }
}

void kx134_1211_service (struct kx134_1211_desc_t *inst)
{
    int do_next_state;
    do {
        // Check for ongoing SPI transaction
        if (inst->spi_in_progress &&
            !sercom_spi_transaction_done(inst->spi_inst, inst->t_id)) {
            // Waiting for an SPI transaction to complete
            return;
        }

        do_next_state = kx134_1211_state_handlers[inst->state](inst);
    } while (do_next_state);
}


// MARK: Callbacks

static void kx134_1211_int1_callback(void *context, union gpio_pin_t pin,
                                     uint8_t value)
{
    struct kx134_1211_desc_t *const inst = (struct kx134_1211_desc_t *)context;

    inst->next_reading_time = millis;

    // Read from sample buffer
    inst->buffer[0] = KX134_1211_REG_BUF_READ | KX134_1211_READ;

    uint16_t const in_length = ((inst->resolution == KX134_1211_RES_8_BIT) ?
                                (KX134_1211_SAMPLE_THRESHOLD_8BIT * 3) :
                                (KX134_1211_SAMPLE_THRESHOLD_16BIT * 6));

    // Try to get a buffer from the telemetry service to put the data into
    uint8_t *buffer = NULL;
    if (inst->telem != NULL) {
        buffer = telemetry_post_kx134_accel(inst->telem,
                                            inst->next_reading_time, inst->odr,
                                            inst->range, inst->rolloff,
                                            inst->resolution, in_length);
    }

    if (buffer == NULL) {
        buffer = inst->buffer;
    } else {
        inst->telem_buffer = buffer;
        inst->telem_buffer_write = 1;
    }

    sercom_spi_start_with_cb(inst->spi_inst, &inst->t_id, KX134_1211_BAUDRATE,
                             inst->cs_pin_group, inst->cs_pin_mask,
                             inst->buffer, 1, buffer, in_length,
                             kx134_1211_spi_callback, inst);
}

void kx134_1211_spi_callback(void *context)
{
    struct kx134_1211_desc_t *const inst = (struct kx134_1211_desc_t *)context;

    // We have read to the sample buffer
    uint8_t *const buffer = inst->telem_buffer_write ? inst->telem_buffer :
                                                       inst->buffer;

    // Save last reading
    inst->last_reading_time = inst->next_reading_time;
    if (inst->resolution == KX134_1211_RES_8_BIT) {
        // 8-Bit samples
        int8_t *const samples = ((int8_t*)buffer +
                                 ((KX134_1211_SAMPLE_THRESHOLD_8BIT - 1) * 3));
        inst->last_x = samples[0];
        inst->last_y = samples[1];
        inst->last_z = samples[2];
    } else {
        // 16-Bit samples
        int16_t *const samples = ((int16_t *)__builtin_assume_aligned(buffer, 2) +
                                  ((KX134_1211_SAMPLE_THRESHOLD_16BIT - 1) * 3));
        inst->last_x = samples[0];
        inst->last_y = samples[1];
        inst->last_z = samples[2];
    }

    // Checkin telemetry buffer if we used one
    if (inst->telem_buffer_write) {
        telemetry_finish_kx134_accel(inst->telem, inst->telem_buffer);
        inst->telem_buffer_write = 0;
    }
}
