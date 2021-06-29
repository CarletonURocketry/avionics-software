/**
 * @file sd.c
 * @desc Module for writing data to an SD Card
 *
 * Sources for implementation:
 *  -- https://electronics.stackexchange.com/questions/77417
 *  -- https://openlabpro.com/guide/interfacing-microcontrollers-with-sd-card/
 *  -- https://nerdclub-uk.blogspot.com/2012/11/how-spi-works-with-sd-card.html
 *  -- https://openlabpro.com/guide/raw-sd-readwrite-using-pic-18f4550/
 *  -- SD Card Physical Layer Spec (see repo wiki)
 *      ^^^ THIS IS THE BEST RESOURCE ^^^
 *
 *  **IMPORTANT NOTE:** If using an older card, the initialization steps must be
 *  executed while the microprocessor/controller is running at a slower clock
 *  rate (100-400 KHz). Newer cards can withstand MHz clocks but older ones will
 *  complain. After initialization is complete, the clock speed may be switched
 *  to a higher one.
 *
 * Supposedly, the proper way to initalize a card over SPI is to:
 *      1. Set the clock speed to 400kHz or less if old card
 *      2. Hold the CS line low and send 80 clock pulses (with bytes 0xFF)
 *      3. Send the "soft reset" command CMD0
 *      4. Wait for the card to respond "ok" with the value 0x01 (0xFF is also
 *          acceptable and indicates the card was in a strange state)
 *      5. Initialize the card:
 *          5a. Send CMD55 followed by ACMD41, if response is 0x05, this is an
 *              old card and CMD1 must be used (step 5b). If response 0x01 for
 *              CMD55 then continue, if response 0x00 for ACMD41 then continue,
 *              if response 0x01 for ACMD41 then repeat this step.
 *          5b. Send in the "initialize card" command CMD1 and repeat this until
 *              the card responds with 0x00.
 *      6. Set sector size using CMD16 with parameter 512
 *      7. Turn off CRC requirement by sending CMD59
 *      8. Next time the card responds with "ok" value it is ready
 *      9. Ramp up clock speed back to normal if step 1 was necessary.
 *
 *  We should also assert CS low at least before and after each CMD is sent
 *  since SD cards are selfish and may assume it's the only SPI device selected
 *  all the time.
 */

#include "board.h"

#include "sdspi.h"
#include "sdspi-states.h"
#include "sd.h"

#ifdef ENABLE_SDSPI

/**
 * init_sdspi()
 *
 * @brief Initializes the SD card.
 *
 * @param sd_desc_t The instance of the sd card struct
 */
void init_sdpsi(struct sdspi_desc_t *inst, struct sercom_spi_desc_t *spi_inst,
                uint32_t cs_pin_mask, uint8_t cs_pin_group,
                union gpio_pin_t card_detect_pin)
{
    /* Store SPI information */
    inst->spi_inst = spi_inst;
    inst->cs_pin_mask = cs_pin_mask;
    inst->cs_pin_group = cs_pin_group;

    /* Configure card detect pin */
    inst->card_detect_pin = card_detect_pin;
    gpio_set_pin_mode(card_detect_pin, GPIO_PIN_INPUT);
    gpio_set_pull(card_detect_pin, GPIO_PULL_HIGH);
    inst->card_present = 0;

    /** Initialize driver state */
    inst->state = SDSPI_NOT_PRESENT;
    inst->substate = 0;
    inst->acmd_state = SDSPI_FAILED;
    inst->spi_in_progress = 0;
    inst->spi_session_open = 0;
    inst->bytes_in = 0;
    inst->init_retry_count = 0;
    inst->v1_card = 0;

    // Clear operation state
    inst->op_addr = 0;
    inst->callback = NULL;
    inst->cb_context = NULL;
    inst->read_buffer = NULL;
    inst->write_data = NULL;
    inst->block_count = 0;
    inst->blocks_done = 0;

    /** Run service function to get started on initilization of card */
    sdspi_service(inst);
}

/**
 * sdspi_service()
 *
 * @brief Handles the different states of the SD card service.
 *
 * @param sd_desc_t The instance of the sd card struct
 */
void sdspi_service(struct sdspi_desc_t *inst)
{
    int do_next_state = 1;
    while (do_next_state) {
        // Check for ongoing SPI transaction
        if (inst->spi_in_progress &&
            !sercom_spi_transaction_done(inst->spi_inst, inst->spi_tid)) {
            // Waiting for an SPI transaction to complete
            return;
        }

        do_next_state = sdspi_state_handlers[inst->state](inst);
    }
}

enum sdspi_status sdspi_get_status(struct sdspi_desc_t *inst)
{
    switch (inst->state) {
        case SDSPI_NOT_PRESENT:
            return SDSPI_STATUS_NO_CARD;
        case SDSPI_INIT_CYCLES:
        case SDSPI_SOFT_RESET:
        case SDSPI_SEND_HOST_VOLT_INFO:
        case SDSPI_SET_CRC:
        case SDSPI_NEXT_CMD_APP_SPECIFIC:
        case SDSPI_INIT_CARD:
        case SDSPI_INIT_V1_CARD:
        case SDSPI_READ_OCR:
        case SDSPI_READ_CSD:
        case SDSPI_READ_CSD_READ_BLOCK:
        case SDSPI_SET_BLOCK_LENGTH:
            return SDSPI_STATUS_INITIALIZING;
        case SDSPI_UNUSABLE_CARD:
            return SDSPI_STATUS_UNUSABLE_CARD;
        case SDSPI_TOO_MANY_INIT_RETRIES:
            return SDSPI_STATUS_TOO_MANY_INIT_RETRIES;
        case SDSPI_FAILED:
            return SDSPI_STATUS_FAILED;
        default:
            return SDSPI_STATUS_READY;
    }
}



static int sdspi_start_op(struct sdspi_desc_t *inst, uint32_t addr,
                          uint32_t num_blocks, sd_op_cb_t cb, void *context)
{
    if (inst->state != SDSPI_IDLE) {
        // Either we are not done initializing the card, there is another
        // operation ongoing or the driver is in a failed state
        return 1;
    }

    if (num_blocks == 0) {
        return 1;
    }

    // Convert address to byte address if nessesary
    if (!inst->block_addressed && __builtin_mul_overflow(addr, SDSPI_BLOCK_SIZE,
                                                         &addr)) {
        // Address overflowed
        return 1;
    }

    // Check that address if valid
    if (addr >= inst->card_capacity) {
        return 1;
    }

    // Set up operation state
    inst->op_addr = addr;
    inst->callback = cb;
    inst->cb_context = context;
    inst->block_count = num_blocks;
    inst->blocks_done = 0;

    return 0;
}




static int sdspi_read(sd_desc_ptr_t inst, uint32_t addr, uint32_t num_blocks,
                      uint8_t *buffer, sd_op_cb_t cb, void *context)
{
    int const ret = sdspi_start_op(inst.sdspi, addr, num_blocks, cb, context);

    if (ret != 0) {
        return ret;
    }

    inst.sdspi->read_buffer = buffer;

    // Jump to correct driver state to start operation
    inst.sdspi->state = SDSPI_START_READ;

    // Run the service function to get started right away
    sdspi_service(inst.sdspi);

    return 0;
}

static int sdspi_write(sd_desc_ptr_t inst, uint32_t addr, uint32_t num_blocks,
                       uint8_t const *data, sd_op_cb_t cb, void *context)
{
    int const ret = sdspi_start_op(inst.sdspi, addr, num_blocks, cb, context);

    if (ret != 0) {
        return ret;
    }

    inst.sdspi->write_data = data;

    // Jump to correct driver state to start operation
    inst.sdspi->state = SDSPI_START_WRITE;

    // Run the service function to get started right away
    sdspi_service(inst.sdspi);

    return 0;
}

// This function is for the generic SD driver status, sdspi_get_status() is for
// sdspi specific status.
static enum sd_status sdspi_get_sd_status(sd_desc_ptr_t inst)
{
    switch (sdspi_get_status(inst.sdspi)) {
        case SDSPI_STATUS_NO_CARD:
            return SD_STATUS_NOT_PRESENT;
        case SDSPI_STATUS_INITIALIZING:
            return SD_STATUS_INITIALIZING;
        case SDSPI_STATUS_READY:
            return SD_STATUS_READY;
        case SDSPI_STATUS_UNUSABLE_CARD:
        case SDSPI_STATUS_TOO_MANY_INIT_RETRIES:
        case SDSPI_STATUS_FAILED:
        default:
            return SD_STATUS_FAILED;
    }
}



struct sd_funcs const sdspi_sd_funcs = {
    .read = &sdspi_read,
    .write = &sdspi_write,
    .get_status = sdspi_get_sd_status
};

#endif // ENABLE_SDSPI
