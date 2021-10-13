/**
 * @file sdhc-states.c
 * @desc Driver state machine for SD host controller
 * @author Samuel Dewan
 * @date 2021-07-01
 * Last Author:
 * Last Edited On:
 */

#include "sdhc-states.h"
#include "sd-commands.h"

#include <string.h>


enum sdhc_cmd_rsp_type {
    /** No response */
    SDHC_CMD_RSP_TYPE_NONE,
    /** 48 bit response */
    SDHC_CMD_RSP_TYPE_R1,
    /** 48 bit response with busy signal */
    SDHC_CMD_RSP_TYPE_R1B,
    /** 136 bit response */
    SDHC_CMD_RSP_TYPE_R2,
    /** 48 bit response, no CRC */
    SDHC_CMD_RSP_TYPE_R3,
    /** 48 bit response */
    SDHC_CMD_RSP_TYPE_R6,
    /** 48 bit response */
    SDHC_CMD_RSP_TYPE_R7
};

// MARK: Helpers

static inline void sdhc_enable_cmd_interrupts(struct sdhc_desc_t *inst,
                                              int enable_transfer_wait,
                                              int enable_data_interrupts)
{
    // Enable command complete interrupt and transfer complete interrupt if
    // requested
    inst->sdhc->NISTER.reg |= (SDHC_NISTER_CMDC |
                               (enable_transfer_wait ? SDHC_NISTER_TRFC : 0));
    inst->sdhc->NISIER.reg |= (SDHC_NISIER_CMDC |
                               (enable_transfer_wait ? SDHC_NISIER_TRFC : 0));
    // Enable command error interrupts
    inst->sdhc->EISTER.reg |= (SDHC_EISTER_CMDTEO |
                               SDHC_EISTER_CMDCRC |
                               SDHC_EISTER_CMDEND |
                               SDHC_EISTER_CMDIDX);
    inst->sdhc->EISIER.reg |= (SDHC_EISIER_CMDTEO |
                               SDHC_EISIER_CMDCRC |
                               SDHC_EISIER_CMDEND |
                               SDHC_EISIER_CMDIDX);

    if (enable_data_interrupts) {
        // Enable data error interrupts
        inst->sdhc->EISTER.reg |= (SDHC_EISTER_DATTEO |
                                   SDHC_EISTER_DATCRC |
                                   SDHC_EISTER_DATEND |
                                   SDHC_EISTER_ACMD |
                                   SDHC_EISTER_ADMA);
        inst->sdhc->EISIER.reg |= (SDHC_EISIER_DATTEO |
                                   SDHC_EISIER_DATCRC |
                                   SDHC_EISIER_DATEND |
                                   SDHC_EISIER_ACMD |
                                   SDHC_EISIER_ADMA);
    } else if (enable_transfer_wait) {
        // Enable data timeout error interrupt
        inst->sdhc->EISTER.reg |= SDHC_EISTER_DATTEO;
        inst->sdhc->EISIER.reg |= SDHC_EISIER_DATTEO;
    }
}

static inline void sdhc_disable_cmd_interrupts(struct sdhc_desc_t *inst)
{
    // Disable command complete interrupt
    inst->sdhc->NISIER.reg &= ~SDHC_NISIER_CMDC;
    inst->sdhc->NISTER.reg &= ~SDHC_NISTER_CMDC;
    // Disable command error interrupts
    inst->sdhc->EISIER.reg &= ~(SDHC_EISIER_CMDTEO |
                                SDHC_EISIER_CMDCRC |
                                SDHC_EISIER_CMDEND |
                                SDHC_EISIER_CMDIDX);
    inst->sdhc->EISTER.reg &= ~(SDHC_EISTER_CMDTEO |
                                SDHC_EISTER_CMDCRC |
                                SDHC_EISTER_CMDEND |
                                SDHC_EISTER_CMDIDX);
}

static inline void sdhc_disable_transfer_interrupts(struct sdhc_desc_t *inst)
{
    // Disable transfer complete interrupt
    inst->sdhc->NISIER.reg &= ~SDHC_NISIER_TRFC;
    inst->sdhc->NISTER.reg &= ~SDHC_NISTER_TRFC;
    // Disable data error interrupts
    inst->sdhc->EISIER.reg &= ~(SDHC_EISIER_DATTEO |
                                SDHC_EISIER_DATCRC |
                                SDHC_EISIER_DATEND |
                                SDHC_EISIER_ACMD |
                                SDHC_EISIER_ADMA);
    inst->sdhc->EISTER.reg &= ~(SDHC_EISTER_DATTEO |
                                SDHC_EISTER_DATCRC |
                                SDHC_EISTER_DATEND |
                                SDHC_EISTER_ACMD |
                                SDHC_EISTER_ADMA);
}


enum sdhc_substate_rsp {
    /** Operation is complete */
    SDHC_SUBSTATE_RSP_DONE,
    /** State handler should return 0 and call substate handler again later */
    SDHC_SUBSTATE_RSP_LATER,
    /** State handler should call substate handler again right away */
    SDHC_SUBSTATE_RSP_AGAIN,
    /** Timed out waiting for command response */
    SDHC_SUBSTATE_RSP_CMD_TIMEOUT,
    /** CRC error in command response */
    SDHC_SUBSTATE_RSP_CMD_CRC_ERROR,
    /** Tried to drive CMD line to 1 but it stayed at 0 */
    SDHC_SUBSTATE_RSP_CMD_LINE_CONFLICT,
    /** ID mismatch or end bit error in command response */
    SDHC_SUBSTATE_RSP_CMD_RSP_ERROR,
    /** Timed out waiting for data or busy signal */
    SDHC_SUBSTATE_RSP_TRAN_TIMEOUT,
    /** CRC error in data response */
    SDHC_SUBSTATE_RSP_TRAN_CRC_ERROR,
    /** End bit error in data response */
    SDHC_SUBSTATE_RSP_TRAN_RSP_ERROR,
    /** ACMD error */
    SDHC_SUBSTATE_RSP_ACMD_ERROR,
    /** ADMA error */
    SDHC_SUBSTATE_RSP_ADMA_ERROR,
    /** Other failure */
    SDHC_SUBSTATE_RSP_FAILED
};

static enum sdhc_substate_rsp sdhc_parse_error_flags(SDHC_EISTR_Type eistr)
{
    if (eistr.bit.CMDTEO && eistr.bit.CMDCRC) {
        return SDHC_SUBSTATE_RSP_CMD_LINE_CONFLICT;
    } else if (eistr.bit.CMDTEO) {
        return SDHC_SUBSTATE_RSP_CMD_TIMEOUT;
    } else if (eistr.bit.CMDCRC) {
        return SDHC_SUBSTATE_RSP_CMD_CRC_ERROR;
    } else if (eistr.bit.CMDEND || eistr.bit.CMDIDX) {
        return SDHC_SUBSTATE_RSP_CMD_RSP_ERROR;
    } else if (eistr.bit.DATTEO) {
        return SDHC_SUBSTATE_RSP_TRAN_TIMEOUT;
    } else if (eistr.bit.DATCRC) {
        return SDHC_SUBSTATE_RSP_TRAN_CRC_ERROR;
    } else if (eistr.bit.DATEND) {
        return SDHC_SUBSTATE_RSP_TRAN_RSP_ERROR;
    } else if (eistr.bit.ACMD) {
        return SDHC_SUBSTATE_RSP_ACMD_ERROR;
    } else if (eistr.bit.ADMA) {
        return SDHC_SUBSTATE_RSP_ADMA_ERROR;
    }
    return SDHC_SUBSTATE_RSP_FAILED;
}

static SDHC_CR_Type sdhc_get_cr_val(uint8_t command, enum sdhc_cmd_rsp_type rsp,
                                    int data)
{
    SDHC_CR_Type cr;

    switch (rsp) {
        case SDHC_CMD_RSP_TYPE_NONE:
            cr.reg = (SDHC_CR_RESPTYP_NONE | SDHC_CR_CMDCCEN_DISABLE |
                      SDHC_CR_CMDICEN_DISABLE);
            break;
        case SDHC_CMD_RSP_TYPE_R1:
        case SDHC_CMD_RSP_TYPE_R6:
        case SDHC_CMD_RSP_TYPE_R7:
            cr.reg = (SDHC_CR_RESPTYP_48_BIT | SDHC_CR_CMDCCEN_ENABLE |
                      SDHC_CR_CMDICEN_ENABLE);
            break;
        case SDHC_CMD_RSP_TYPE_R1B:
            cr.reg = (SDHC_CR_RESPTYP_48_BIT_BUSY | SDHC_CR_CMDCCEN_ENABLE |
                      SDHC_CR_CMDICEN_ENABLE);
            break;
        case SDHC_CMD_RSP_TYPE_R2:
            cr.reg = (SDHC_CR_RESPTYP_136_BIT | SDHC_CR_CMDCCEN_ENABLE |
                      SDHC_CR_CMDICEN_DISABLE);
            break;
        case SDHC_CMD_RSP_TYPE_R3:
            cr.reg = (SDHC_CR_RESPTYP_48_BIT | SDHC_CR_CMDCCEN_DISABLE |
                      SDHC_CR_CMDICEN_DISABLE);
            break;
    };

    cr.reg |= ((data ? SDHC_CR_DPSEL_DATA : SDHC_CR_DPSEL_NO_DATA) |
               ((command == SD_CMD12) ? SDHC_CR_CMDTYP_ABORT :
                                        SDHC_CR_CMDTYP_NORMAL) |
               SDHC_CR_CMDIDX(command));

    return cr;
}


static enum sdhc_substate_rsp sdhc_handle_command(struct sdhc_desc_t *inst,
                                                  uint8_t command, uint32_t arg,
                                                  enum sdhc_cmd_rsp_type rsp)
{
    switch (inst->substate) {
        case SDHC_SUBSTATE_START:
            // Make sure that eveything is idle
            if (inst->sdhc->PSR.bit.CMDINHC || inst->sdhc->PSR.bit.CMDINHD) {
                return SDHC_SUBSTATE_RSP_LATER;
            }
            // Enable required interrupts
            sdhc_enable_cmd_interrupts(inst, rsp == SDHC_CMD_RSP_TYPE_R1B, 0);
            // Configure registers for command
            inst->sdhc->ARG1R.reg = arg;
            inst->sdhc->TMR.reg = 0;
            inst->sdhc->CR = sdhc_get_cr_val(command, rsp, 0);
            // Wait for command to finish
            inst->waiting_for_interrupt = 1;
            inst->substate = SDHC_SUBSTATE_CMD_WAIT;
            return SDHC_SUBSTATE_RSP_LATER;
        case SDHC_SUBSTATE_CMD_WAIT:
            return SDHC_SUBSTATE_RSP_LATER;
        case SDHC_SUBSTATE_CMD_DONE:
            // Disable command interrutps
            sdhc_disable_cmd_interrupts(inst);
            // Check if we need to wait for busy signal
            if (rsp == SDHC_CMD_RSP_TYPE_R1B) {
                // Need to wait for busy signal (transfer stage)
                inst->waiting_for_interrupt = 1;
                inst->substate = SDHC_SUBSTATE_TRAN_WAIT;
                return SDHC_SUBSTATE_RSP_LATER;
            }
            // All done! The response, if any, is in the response registers
            break;
        case SDHC_SUBSTATE_TRAN_WAIT:
            return SDHC_SUBSTATE_RSP_LATER;
        case SDHC_SUBSTATE_TRAN_DONE:
            // Disable transfer interrupts
            sdhc_disable_transfer_interrupts(inst);
            // All done! The response, if any, is in the response registers
            break;
        case SDHC_SUBSTATE_CMD_ERROR:
        case SDHC_SUBSTATE_TRAN_ERROR:
            inst->substate = 0;
            // Save error flags
            SDHC_EISTR_Type const eistr = inst->sdhc->EISTR;
            // Clear error flags
            inst->sdhc->EISTR.reg = 0xFFFF;
            // Disable interrupts
            sdhc_disable_cmd_interrupts(inst);
            sdhc_disable_transfer_interrupts(inst);
            // Check error flags
            return sdhc_parse_error_flags(eistr);
        default:
            inst->substate = 0;
            return SDHC_SUBSTATE_RSP_FAILED;
    }

    inst->substate = 0;
    return SDHC_SUBSTATE_RSP_DONE;
}

static enum sdhc_substate_rsp sdhc_handle_data(struct sdhc_desc_t *inst,
                                               uint8_t command, uint32_t arg,
                                               uint16_t block_count,
                                               uint16_t block_size,
                                               uint32_t dma_addr, int write)
{
    switch (inst->substate) {
        case SDHC_SUBSTATE_START:
            // Make sure that eveything is idle
            if (inst->sdhc->PSR.bit.CMDINHC || inst->sdhc->PSR.bit.CMDINHD) {
                return SDHC_SUBSTATE_RSP_LATER;
            }
            // Make sure that SD clock is on
            inst->sdhc->CCR.bit.SDCLKEN = 1;
            // Enable required interrupts
            sdhc_enable_cmd_interrupts(inst, 1, 1);
            // Configure ADMA2 descriptor
            uint32_t const len = block_count * block_size;
            if (len < 65536) {
                inst->adma2_desc.length = len;
            } else if (len == 65536) {
                inst->adma2_desc.length = 0;
            } else {
                return SDHC_SUBSTATE_RSP_FAILED;
            }
            inst->adma2_desc.address = dma_addr;
            inst->adma2_desc.attributes.raw = (SDHC_ADMA2_DESC_VALID |
                                               SDHC_ADMA2_DESC_END |
                                               SDHC_ADMA2_DESC_ACT_TRAN);
            // Set ADMA2 descriptor base address
            inst->sdhc->ASAR[0].reg = (uint32_t)&inst->adma2_desc;
            // Configure registers for command
            unsigned const multi = block_count > 1;
            unsigned const cmd23 = multi && inst->cmd23_supported;
            uint16_t const acmden = (cmd23 ? SDHC_TMR_ACMDEN_CMD23 :
                                     multi ? SDHC_TMR_ACMDEN_CMD12 :
                                             SDHC_TMR_ACMDEN_DISABLED);
            if (cmd23) {
                inst->sdhc->SSAR.CMD23.ARG2 = block_count;
            }
            inst->sdhc->BSR.reg = SDHC_BSR_BLOCKSIZE(block_size);
            inst->sdhc->BCR.reg = SDHC_BCR_BCNT(block_count);
            inst->sdhc->ARG1R.reg = arg;
            inst->sdhc->TMR.reg = (SDHC_TMR_DMAEN_ENABLE |
                                   (multi << SDHC_TMR_BCEN_Pos) |
                                   acmden |
                                   (write ? SDHC_TMR_DTDSEL_WRITE :
                                            SDHC_TMR_DTDSEL_READ) |
                                   (multi << SDHC_TMR_MSBSEL_Pos));
            inst->sdhc->CR = sdhc_get_cr_val(command, SDHC_CMD_RSP_TYPE_R1, 1);
            // Wait for command to finish
            inst->waiting_for_interrupt = 1;
            inst->substate = SDHC_SUBSTATE_CMD_WAIT;
            return SDHC_SUBSTATE_RSP_LATER;
        case SDHC_SUBSTATE_CMD_WAIT:
            return SDHC_SUBSTATE_RSP_LATER;
        case SDHC_SUBSTATE_CMD_DONE:
            // Disable command interrutps
            sdhc_disable_cmd_interrupts(inst);
            // Transfering data
            inst->waiting_for_interrupt = 1;
            inst->substate = SDHC_SUBSTATE_TRAN_WAIT;
            return SDHC_SUBSTATE_RSP_LATER;
        case SDHC_SUBSTATE_TRAN_WAIT:
            return SDHC_SUBSTATE_RSP_LATER;
        case SDHC_SUBSTATE_TRAN_DONE:
            // Disable transfer interrupts
            sdhc_disable_transfer_interrupts(inst);
            // All done! The data has been placed in the destination.
            break;
        case SDHC_SUBSTATE_CMD_ERROR:
        case SDHC_SUBSTATE_TRAN_ERROR:
            inst->substate = 0;
            // Save error flags
            SDHC_EISTR_Type const eistr = inst->sdhc->EISTR;
            // Clear error flags
            inst->sdhc->EISTR.reg = 0xFFFF;
            // Disable interrupts
            sdhc_disable_cmd_interrupts(inst);
            sdhc_disable_transfer_interrupts(inst);
            // Check error flags
            return sdhc_parse_error_flags(eistr);
        default:
            inst->substate = 0;
            return SDHC_SUBSTATE_RSP_FAILED;
    }

    inst->substate = 0;
    return SDHC_SUBSTATE_RSP_DONE;
}

static enum sdhc_substate_rsp sdhc_handle_read(struct sdhc_desc_t *inst,
                                               uint8_t command, uint32_t arg,
                                               uint16_t block_count,
                                               uint16_t block_size,
                                               uint8_t *destination)
{
    return sdhc_handle_data(inst, command, arg, block_count, block_size,
                            (uint32_t)destination, 0);
}

static enum sdhc_substate_rsp sdhc_handle_write(struct sdhc_desc_t *inst,
                                                uint8_t command, uint32_t addr,
                                                uint16_t block_count,
                                                uint8_t const *source)
{
    return sdhc_handle_data(inst, command, addr, block_count, SD_BLOCK_LENGTH,
                            (uint32_t)source, 1);
}


static inline int sdhc_retry(struct sdhc_desc_t *inst, uint8_t count, int abort,
                             enum sdhc_state fail_state)
{
    inst->init_retry_count++;
    if (inst->init_retry_count > count) {
        inst->state = fail_state;
        return 0;
    } else if (abort) {
        inst->abort_recovery_state = inst->state;
        inst->state = SDHC_ABORT;
        inst->cmd_start_time = millis;
    }
    // Try again
    return 1;
}

static inline int sdhc_init_retry(struct sdhc_desc_t *inst)
{
    return sdhc_retry(inst, SDHC_NUM_INIT_RETRIES, 0,
                      SDHC_TOO_MANY_INIT_RETRIES);
}

static inline int sdhc_init_abort_retry(struct sdhc_desc_t *inst)
{
    return sdhc_retry(inst, SDHC_NUM_INIT_RETRIES, 1,
                      SDHC_TOO_MANY_INIT_RETRIES);
}

static inline int sdhc_op_retry(struct sdhc_desc_t *inst)
{
    return sdhc_retry(inst, SDHC_NUM_OP_RETRIES, 0, SDHC_FAILED);
}

static inline int sdhc_op_abort_retry(struct sdhc_desc_t *inst)
{
    return sdhc_retry(inst, SDHC_NUM_OP_RETRIES, 1, SDHC_FAILED);
}



// MARK: Handlers
static int sdhc_state_handler_not_present(struct sdhc_desc_t *inst)
{
    if (inst->sdhc->PSR.bit.CARDINS) {
        // Card inserted go to first initialization state
        inst->state = SDHC_RESET;
        // Disable card inserted interrupt, enable card remove interrupt
        inst->sdhc->NISTER.reg = SDHC_NISTER_CREM;
        inst->sdhc->NISIER.reg = SDHC_NISIER_CREM;
        // Enable SD card clock
        inst->sdhc->CCR.bit.SDCLKEN = 1;
        // Enable SD bus power
        inst->sdhc->PCR.reg = SDHC_PCR_SDBVSEL_3V3 | SDHC_PCR_SDBPWR;
        // Go right into next state
        return 1;
    } else {
        return 0;
    }
}

static int sdhc_state_handler_idle(struct sdhc_desc_t *inst)
{
    // Make sure that SD clock is off
    inst->sdhc->CCR.bit.SDCLKEN = 0;
    return 0;
}

static int sdhc_state_handler_app_cmd(struct sdhc_desc_t *inst)
{
    union sd_rca_arg const arg = { .rca = inst->rca };
    enum sdhc_substate_rsp res;
    res = sdhc_handle_command(inst, SD_CMD55, arg.raw, SDHC_CMD_RSP_TYPE_R1);
    union sd_card_status_rsp rsp;

    switch (res) {
        case SDHC_SUBSTATE_RSP_DONE:
            rsp = sd_get_card_status_rsp(inst->sdhc->RR);

            if (rsp.error || rsp.cc_error || rsp.illegal_comand ||
                rsp.com_crc_error || rsp.card_is_locked || !rsp.app_cmd) {
                // Command failed try again later
                // TODO: Better error handling
                return 0;
            }

            // Success! Ready to move on to next state.
            inst->state = inst->acmd_state;
            inst->acmd_state = SDHC_FAILED;
            return 1;
        case SDHC_SUBSTATE_RSP_LATER:
            return 0;
        case SDHC_SUBSTATE_RSP_AGAIN:
            return 1;
        case SDHC_SUBSTATE_RSP_CMD_TIMEOUT:
        case SDHC_SUBSTATE_RSP_CMD_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_CMD_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_FAILED:
            // Try again later
            // TODO: Better error handling
            return 0;
        default:
            // This shouldn't happen
            inst->state = SDHC_FAILED;
            return 0;
    }
}

/** Send CMD12 to abort failed operation */
static int sdhc_state_handler_abort(struct sdhc_desc_t *inst)
{
    if ((millis - inst->cmd_start_time) < 5) {
        return 0;
    }

    enum sdhc_substate_rsp res;
    res = sdhc_handle_command(inst, SD_CMD12, 0, SDHC_CMD_RSP_TYPE_R1B);
    union sd_card_status_rsp rsp;

    switch (res) {
        case SDHC_SUBSTATE_RSP_DONE:
            rsp = sd_get_card_status_rsp(inst->sdhc->RR);

            if (rsp.error || rsp.cc_error || rsp.illegal_comand ||
                rsp.com_crc_error || rsp.card_is_locked || !rsp.app_cmd) {
                // Command failed, unrecoverable error
                inst->state = SDHC_FAILED;
                return 0;
            }

            // Success! Ready to move on to next state.
            inst->state = inst->abort_recovery_state;
            inst->abort_recovery_state = SDHC_FAILED;
            return 1;
        case SDHC_SUBSTATE_RSP_LATER:
            return 0;
        case SDHC_SUBSTATE_RSP_AGAIN:
            return 1;
        default:
            // Command failed, unrecoverable error
            inst->state = SDHC_FAILED;
            return 0;
    }
}



/** Send CMD0 */
static int sdhc_state_handler_reset(struct sdhc_desc_t *inst)
{
    enum sdhc_substate_rsp res;
    res = sdhc_handle_command(inst, SD_CMD0, 0, SDHC_CMD_RSP_TYPE_NONE);

    switch (res) {
        case SDHC_SUBSTATE_RSP_DONE:
            // Success! Ready to move on to next state.
            inst->init_retry_count = 0;
            inst->state = SDHC_CHECK_VOLTAGE;
            return 1;
        case SDHC_SUBSTATE_RSP_LATER:
            return 0;
        case SDHC_SUBSTATE_RSP_AGAIN:
            return 1;
        case SDHC_SUBSTATE_RSP_CMD_LINE_CONFLICT:
        case SDHC_SUBSTATE_RSP_CMD_TIMEOUT:
        case SDHC_SUBSTATE_RSP_CMD_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_CMD_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_FAILED:
            // Try again later
            return sdhc_init_retry(inst);
        default:
            // This shouldn't happen
            inst->state = SDHC_FAILED;
            return 0;
    }
}

/** Send CMD8 */
static int sdhc_state_handler_check_voltage(struct sdhc_desc_t *inst)
{
    union sd_cmd8_arg const arg = {
        .check_pattern = 0xAA,
        .voltage_supplied = SD_VHS_27_36
    };
    enum sdhc_substate_rsp res;
    res = sdhc_handle_command(inst, SD_CMD8, arg.raw, SDHC_CMD_RSP_TYPE_R7);
    union sd_cmd8_rsp rsp;

    switch (res) {
        case SDHC_SUBSTATE_RSP_DONE:
            rsp = sd_get_cmd8_rsp(inst->sdhc->RR);

            if ((rsp.check_pattern_echo != 0xAA) ||
                (rsp.voltage_accepted != SD_VHS_27_36)) {
                // Command failed
                return sdhc_init_retry(inst);
            }

            inst->v1_card = 0;

            // Success! Ready to move on to next state.
            inst->init_retry_count = 0;
            inst->state = SDHC_APP_CMD;
            inst->acmd_state = SDHC_CHECK_OCR;
            return 1;
        case SDHC_SUBSTATE_RSP_LATER:
            return 0;
        case SDHC_SUBSTATE_RSP_AGAIN:
            return 1;
        case SDHC_SUBSTATE_RSP_CMD_TIMEOUT:
            // This is a SD Version 1 card
            inst->v1_card = 1;
            // Ready to move to next state
            inst->init_retry_count = 0;
            inst->state = SDHC_APP_CMD;
            inst->acmd_state = SDHC_CHECK_OCR;
            return 1;
        case SDHC_SUBSTATE_RSP_CMD_LINE_CONFLICT:
        case SDHC_SUBSTATE_RSP_CMD_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_CMD_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_FAILED:
            // Try again later
            return sdhc_init_retry(inst);
        default:
            // This shouldn't happen
            inst->state = SDHC_FAILED;
            return 0;
    }
}

/** Send send ACMD41 to read OCR and check supported voltage ranges */
static int sdhc_state_handler_check_ocr(struct sdhc_desc_t *inst)
{
    enum sdhc_substate_rsp res;
    // arg = 0 for inquiry
    res = sdhc_handle_command(inst, SD_ACMD41, 0, SDHC_CMD_RSP_TYPE_R3);
    union sd_acmd41_rsp rsp;

    switch (res) {
        case SDHC_SUBSTATE_RSP_DONE:
            rsp = sd_get_acmd41_rsp(inst->sdhc->RR);

            // Check if the card works at 3.3 volts
            if (!(rsp.volt_range_3V2_3V3 || rsp.volt_range_3V3_3V4)) {
                // This card does not support our voltage range
                inst->state = SDHC_UNUSABLE_CARD;
                return 0;
            }

            // Success! Ready to move on to next state.
            inst->init_retry_count = 0;
            inst->state = SDHC_APP_CMD;
            inst->acmd_state = SDHC_INITIALIZE;
            return 1;
        case SDHC_SUBSTATE_RSP_LATER:
            return 0;
        case SDHC_SUBSTATE_RSP_AGAIN:
            return 1;
        case SDHC_SUBSTATE_RSP_CMD_LINE_CONFLICT:
        case SDHC_SUBSTATE_RSP_CMD_TIMEOUT:
        case SDHC_SUBSTATE_RSP_CMD_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_CMD_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_FAILED:
            // Try again later
            inst->state = SDHC_APP_CMD;
            inst->acmd_state = SDHC_CHECK_OCR;
            return sdhc_init_retry(inst);
        default:
            // This shouldn't happen
            inst->state = SDHC_FAILED;
            return 0;
    }
}

/** Send send ACMD41 to set supported voltage ranges and start initialization */
static int sdhc_state_handler_initialize(struct sdhc_desc_t *inst)
{
    if (!inst->init_cmd_started) {
        inst->cmd_start_time = millis;
        inst->init_cmd_started = 1;
    }

    union sd_acmd41_arg const arg = {
        .volt_range_3V2_3V3 = 1,
        .volt_range_3V3_3V4 = 1,
        .xpc = 1,
        .hcs = !inst->v1_card
    };
    enum sdhc_substate_rsp res;
    res = sdhc_handle_command(inst, SD_ACMD41, arg.raw, SDHC_CMD_RSP_TYPE_R3);
    union sd_acmd41_rsp rsp;

    switch (res) {
        case SDHC_SUBSTATE_RSP_DONE:
            rsp = sd_get_acmd41_rsp(inst->sdhc->RR);

            if (!rsp.busy) {
                if ((millis - inst->cmd_start_time) >
                        SDHC_ACMD41_INIT_TIMEOUT) {
                    // Timeout expired
                    inst->state = SDHC_INIT_TIMEOUT;
                    return 0;
                }

                // Retry
                inst->init_retry_count++;
                inst->state = SDHC_APP_CMD;
                inst->acmd_state = SDHC_INITIALIZE;
                return 0;
            }

            inst->block_addressed = rsp.ccs;

            // Success! Ready to move on to next state.
            inst->init_retry_count = 0;
            inst->state = SDHC_GET_CID;
            return 1;
        case SDHC_SUBSTATE_RSP_LATER:
            return 0;
        case SDHC_SUBSTATE_RSP_AGAIN:
            return 1;
        case SDHC_SUBSTATE_RSP_CMD_LINE_CONFLICT:
        case SDHC_SUBSTATE_RSP_CMD_TIMEOUT:
        case SDHC_SUBSTATE_RSP_CMD_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_CMD_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_FAILED:
            // Try again later
            inst->state = SDHC_APP_CMD;
            inst->acmd_state = SDHC_CHECK_OCR;
            return sdhc_init_retry(inst);
        default:
            // This shouldn't happen
            inst->state = SDHC_FAILED;
            return 0;
    }
}

/** Send CMD2 to get card ID and go into IDENT state */
static int sdhc_state_handler_get_cid(struct sdhc_desc_t *inst)
{
    enum sdhc_substate_rsp res;
    res = sdhc_handle_command(inst, SD_CMD2, 0, SDHC_CMD_RSP_TYPE_R2);
    //union sd_cid_reg rsp;

    switch (res) {
        case SDHC_SUBSTATE_RSP_DONE:
            //rsp = sd_get_cid_rsp(inst->sdhc->RR);

            // Success! Ready to move on to next state.
            inst->init_retry_count = 0;
            inst->state = SDHC_GET_RCA;
            return 1;
        case SDHC_SUBSTATE_RSP_LATER:
            return 0;
        case SDHC_SUBSTATE_RSP_AGAIN:
            return 1;
        case SDHC_SUBSTATE_RSP_CMD_LINE_CONFLICT:
        case SDHC_SUBSTATE_RSP_CMD_TIMEOUT:
        case SDHC_SUBSTATE_RSP_CMD_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_CMD_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_FAILED:
            // Try again later
            return sdhc_init_retry(inst);
        default:
            // This shouldn't happen
            inst->state = SDHC_FAILED;
            return 0;
    }
}

/** Send CMD3 to get Relative Address (RCA) for card */
static int sdhc_state_handler_get_rca(struct sdhc_desc_t *inst)
{
    enum sdhc_substate_rsp res;
    res = sdhc_handle_command(inst, SD_CMD3, 0, SDHC_CMD_RSP_TYPE_R6);
    union sd_cmd3_rsp rsp;

    switch (res) {
        case SDHC_SUBSTATE_RSP_DONE:
            rsp = sd_get_cmd3_rsp(inst->sdhc->RR);

            if (rsp.error || rsp.illegal_comand || rsp.com_crc_error ||
                (rsp.rca == 0)) {
                // Command failed try again later
                return sdhc_init_retry(inst);
            }

            inst->rca = rsp.rca;

            // Success! Ready to move on to next state.
            inst->init_retry_count = 0;
            inst->state = SDHC_READ_CSD;
            return 1;
        case SDHC_SUBSTATE_RSP_LATER:
            return 0;
        case SDHC_SUBSTATE_RSP_AGAIN:
            return 1;
        case SDHC_SUBSTATE_RSP_CMD_LINE_CONFLICT:
        case SDHC_SUBSTATE_RSP_CMD_TIMEOUT:
        case SDHC_SUBSTATE_RSP_CMD_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_CMD_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_FAILED:
            // Try again later
            return sdhc_init_retry(inst);
        default:
            // This shouldn't happen
            inst->state = SDHC_FAILED;
            return 0;
    }
}

/** Send CMD9 to read Card Specific Data */
static int sdhc_state_handler_set_read_csd(struct sdhc_desc_t *inst)
{
    union sd_rca_arg const arg = { .rca = inst->rca };
    enum sdhc_substate_rsp res;
    res = sdhc_handle_command(inst, SD_CMD9, arg.raw, SDHC_CMD_RSP_TYPE_R2);
    union {
        union sd_csd_1_reg v1;
        union sd_csd_2_reg v2;
    } rsp;

    switch (res) {
        case SDHC_SUBSTATE_RSP_DONE:
            rsp.v2 = sd_get_csd_2_rsp(inst->sdhc->RR);

            if (rsp.v2.csd_structure == 0b01) {
                // This is v2 of the CSD register
                inst->card_capacity = SD_CSD_2_BLOCKS(rsp.v2);
            } else if (rsp.v2.csd_structure == 0b00) {
                // This is v1 of the CSD register
                inst->card_capacity = SD_CSD_1_BLOCKS(rsp.v1);
            } else {
                // Unkown CSD register layout
                inst->state = SDHC_UNUSABLE_CARD;
                return 0;
            }

            // Success! Ready to move on to next state.
            inst->init_retry_count = 0;
            inst->state = SDHC_SELECT;
            return 1;
        case SDHC_SUBSTATE_RSP_LATER:
            return 0;
        case SDHC_SUBSTATE_RSP_AGAIN:
            return 1;
        case SDHC_SUBSTATE_RSP_CMD_LINE_CONFLICT:
        case SDHC_SUBSTATE_RSP_CMD_TIMEOUT:
        case SDHC_SUBSTATE_RSP_CMD_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_CMD_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_FAILED:
            // Try again later
            return sdhc_init_retry(inst);
        default:
            // This shouldn't happen
            inst->state = SDHC_FAILED;
            return 0;
    }
}

/** Send CMD7 to select card and go into TRAN state */
static int sdhc_state_handler_select(struct sdhc_desc_t *inst)
{
    union sd_rca_arg const arg = { .rca = inst->rca };
    enum sdhc_substate_rsp res;
    res = sdhc_handle_command(inst, SD_CMD7, arg.raw, SDHC_CMD_RSP_TYPE_R1);
    union sd_card_status_rsp rsp;

    switch (res) {
        case SDHC_SUBSTATE_RSP_DONE:
            rsp = sd_get_card_status_rsp(inst->sdhc->RR);

            if (rsp.error || rsp.cc_error || rsp.illegal_comand ||
                rsp.com_crc_error || rsp.card_is_locked) {
                // Command failed try again later
                return sdhc_init_retry(inst);
            }

            // We can increase our clock speed to 25 MHz now
            inst->sdhc->CCR.bit.SDCLKEN = 0;
            const uint16_t clk_setting = inst->clock_freq / SDHC_CLK_NORMAL / 2;
            inst->sdhc->CCR.reg = (SDHC_CCR_SDCLKFSEL(clk_setting & 0xFF) |
                                   SDHC_CCR_USDCLKFSEL((clk_setting >> 8) &
                                                       0x3) |
                                   SDHC_CCR_INTCLKEN);
            while (!inst->sdhc->CCR.bit.INTCLKS);
            inst->sdhc->CCR.bit.SDCLKEN = 1;

            // Success! Ready to move on to next state.
            inst->init_retry_count = 0;
            inst->state = SDHC_SET_HIGH_SPEED;
            return 1;
        case SDHC_SUBSTATE_RSP_LATER:
            return 0;
        case SDHC_SUBSTATE_RSP_AGAIN:
            return 1;
        case SDHC_SUBSTATE_RSP_CMD_LINE_CONFLICT:
        case SDHC_SUBSTATE_RSP_CMD_TIMEOUT:
        case SDHC_SUBSTATE_RSP_CMD_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_CMD_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_TRAN_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_TRAN_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_FAILED:
            // Try again later
            return sdhc_init_retry(inst);
        case SDHC_SUBSTATE_RSP_TRAN_TIMEOUT:
        case SDHC_SUBSTATE_RSP_ADMA_ERROR:
            // Abort command and retry
            return sdhc_init_abort_retry(inst);
        default:
            // This shouldn't happen
            inst->state = SDHC_FAILED;
            return 0;
    }
}

/** Send CMD6 to switch to high speed mode */
static int sdhc_state_handler_set_high_speed(struct sdhc_desc_t *inst)
{
    union sd_cmd6_arg const arg = {
        .function_group_1 = SD_FG1_ACCESS_MODE_SDR25,
        .function_group_2 = SD_FG_NO_CHANGE,
        .function_group_3 = SD_FG_NO_CHANGE,
        .function_group_4 = SD_FG_NO_CHANGE,
        .function_group_5 = SD_FG_NO_CHANGE,
        .function_group_6 = SD_FG_NO_CHANGE,
        .mode = 1
    };
    enum sdhc_substate_rsp res;
    res = sdhc_handle_read(inst, SD_CMD6, arg.raw, 1,
                           sizeof(union sd_switch_function_status_rsp),
                           inst->buffer);
    union sd_card_status_rsp rsp;

    switch (res) {
        case SDHC_SUBSTATE_RSP_DONE:
            // Check CMD response
            rsp = sd_get_card_status_rsp(inst->sdhc->RR);

            if (rsp.error || rsp.cc_error || rsp.illegal_comand ||
                rsp.com_crc_error || rsp.card_is_locked) {
                // Command failed try again later
                return sdhc_init_retry(inst);
            }

            // Check status
            union sd_switch_function_status_rsp status;
            status = sd_swap_switch_func_status(inst->buffer);

            if (status.function_group_1 == SD_FG1_ACCESS_MODE_SDR25) {
                // Success! We need to switch to high speed mode now.
                inst->sdhc->HC1R.bit.HSEN = 1;
                // We can increase our clock speed to 50 MHz now.
                inst->sdhc->CCR.bit.SDCLKEN = 0;
                const uint16_t clk_setting = (inst->clock_freq /
                                              SDHC_CLK_HIGH_SPEED / 2);
                inst->sdhc->CCR.reg = (SDHC_CCR_SDCLKFSEL(clk_setting & 0xFF) |
                                       SDHC_CCR_USDCLKFSEL((clk_setting >> 8) &
                                                           0x3) |
                                       SDHC_CCR_INTCLKEN);
                while (!inst->sdhc->CCR.bit.INTCLKS);
                inst->sdhc->CCR.bit.SDCLKEN = 1;
            } else if (status.function_group_1_info &
                       (1 << SD_FG1_ACCESS_MODE_SDR25)) {
                // This card says it can support a 50 MHz clock but it didn't
                // let us enable it. Try again.
                return sdhc_init_retry(inst);
            }

            // Ready to move on to next state.
            inst->init_retry_count = 0;
            inst->state = SDHC_APP_CMD;
            //inst->acmd_state = SDHC_SET_4_BIT;
            inst->acmd_state = SDHC_READ_SCR;
            return 1;
        case SDHC_SUBSTATE_RSP_LATER:
            return 0;
        case SDHC_SUBSTATE_RSP_AGAIN:
            return 1;
        case SDHC_SUBSTATE_RSP_CMD_LINE_CONFLICT:
        case SDHC_SUBSTATE_RSP_CMD_TIMEOUT:
        case SDHC_SUBSTATE_RSP_CMD_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_CMD_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_TRAN_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_TRAN_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_FAILED:
            // Try again later
            return sdhc_init_retry(inst);
        case SDHC_SUBSTATE_RSP_TRAN_TIMEOUT:
        case SDHC_SUBSTATE_RSP_ACMD_ERROR:
        case SDHC_SUBSTATE_RSP_ADMA_ERROR:
            // Abort and retry
            return sdhc_init_abort_retry(inst);
        default:
            // This shouldn't happen
            inst->state = SDHC_FAILED;
            return 0;
    }
}

/** Send ACMD6 to switch to a 4 bit wide bus */
static int sdhc_state_handler_set_4_bit(struct sdhc_desc_t *inst)
{
    union sd_acmd6_arg const arg = { .bus_width = SD_BUS_WIDTH_4 };
    enum sdhc_substate_rsp res;
    res = sdhc_handle_command(inst, SD_ACMD6, arg.raw, SDHC_CMD_RSP_TYPE_R1);
    union sd_card_status_rsp rsp;

    switch (res) {
        case SDHC_SUBSTATE_RSP_DONE:
            rsp = sd_get_card_status_rsp(inst->sdhc->RR);

            if (rsp.error || rsp.illegal_comand || rsp.com_crc_error ||
                rsp.out_of_range) {
                // Command failed try again later
                return sdhc_init_retry(inst);
            }

            // We need to switch to a 4 bit bus now.
            inst->sdhc->HC1R.bit.DW = SDHC_HC1R_DW_4BIT_Val;

            // Success! Ready to move on to next state.
            inst->init_retry_count = 0;
            inst->state = SDHC_APP_CMD;
            inst->acmd_state = SDHC_READ_SCR;
            return 1;
        case SDHC_SUBSTATE_RSP_LATER:
            return 0;
        case SDHC_SUBSTATE_RSP_AGAIN:
            return 1;
        case SDHC_SUBSTATE_RSP_CMD_LINE_CONFLICT:
        case SDHC_SUBSTATE_RSP_CMD_TIMEOUT:
        case SDHC_SUBSTATE_RSP_CMD_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_CMD_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_FAILED:
            // Try again later
            inst->state = SDHC_APP_CMD;
            inst->acmd_state = SDHC_SET_4_BIT;
            return sdhc_init_retry(inst);
        default:
            // This shouldn't happen
            inst->state = SDHC_FAILED;
            return 0;
    }
}

/** Send ACMD51 to read SD Card Configuration Register (SCR) */
static int sdhc_state_handler_read_scr(struct sdhc_desc_t *inst)
{
    enum sdhc_substate_rsp res;
    res = sdhc_handle_read(inst, SD_ACMD51, 0, 1, sizeof(union sd_scr_reg),
                           inst->buffer);
    union sd_card_status_rsp rsp;

    switch (res) {
        case SDHC_SUBSTATE_RSP_DONE:
            // Check CMD response
            rsp = sd_get_card_status_rsp(inst->sdhc->RR);

            if (rsp.error || rsp.cc_error || rsp.illegal_comand ||
                rsp.com_crc_error || rsp.card_is_locked || !rsp.app_cmd) {
                // Command failed try again later
                return sdhc_init_retry(inst);
            }

            // Check SCR register
            union sd_scr_reg scr;
            scr = sd_swap_scr(inst->buffer);

            inst->cmd23_supported = !!(scr.command_support & (0b10));

            // Ready to move on to next state.
            inst->init_retry_count = 0;
            inst->state = SDHC_SET_BLOCK_LEN;
            return 1;
        case SDHC_SUBSTATE_RSP_LATER:
            return 0;
        case SDHC_SUBSTATE_RSP_AGAIN:
            return 1;
        case SDHC_SUBSTATE_RSP_CMD_LINE_CONFLICT:
        case SDHC_SUBSTATE_RSP_CMD_TIMEOUT:
        case SDHC_SUBSTATE_RSP_CMD_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_CMD_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_TRAN_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_TRAN_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_FAILED:
            // Try again later
            return sdhc_init_retry(inst);
        case SDHC_SUBSTATE_RSP_TRAN_TIMEOUT:
        case SDHC_SUBSTATE_RSP_ACMD_ERROR:
        case SDHC_SUBSTATE_RSP_ADMA_ERROR:
            // Abort and retry
            return sdhc_init_abort_retry(inst);
        default:
            // This shouldn't happen
            inst->state = SDHC_FAILED;
            return 0;
    }
}

/** Send CMD16 to set block length */
static int sdhc_state_handler_set_block_len(struct sdhc_desc_t *inst)
{
    enum sdhc_substate_rsp res;
    res = sdhc_handle_command(inst, SD_CMD16, SD_BLOCK_LENGTH,
                              SDHC_CMD_RSP_TYPE_R1);
    union sd_card_status_rsp rsp;

    switch (res) {
        case SDHC_SUBSTATE_RSP_DONE:
            rsp = sd_get_card_status_rsp(inst->sdhc->RR);

            if (rsp.error || rsp.cc_error || rsp.illegal_comand ||
                rsp.com_crc_error || rsp.card_is_locked) {
                // Command failed try again later
                return sdhc_init_retry(inst);
            }

            // Success! Ready to move on to next state.
            inst->init_retry_count = 0;
            inst->state = SDHC_INIT_DONE;
            return 1;
        case SDHC_SUBSTATE_RSP_LATER:
            return 0;
        case SDHC_SUBSTATE_RSP_AGAIN:
            return 1;
        case SDHC_SUBSTATE_RSP_CMD_LINE_CONFLICT:
        case SDHC_SUBSTATE_RSP_CMD_TIMEOUT:
        case SDHC_SUBSTATE_RSP_CMD_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_CMD_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_FAILED:
            // Try again later
            return sdhc_init_retry(inst);
        default:
            // This shouldn't happen
            inst->state = SDHC_FAILED;
            return 0;
    }
}

static int sdhc_state_handler_init_done(struct sdhc_desc_t *inst)
{
//    // Lengthen command timeout to 500 ms (assuming that slow clock is
//    // 32.768 KHz)
//    inst->sdhc->TCR.reg = SDHC_TCR_DTCVAL(13);
//    // Disable forcing clock to run for an extra 8 cycles after transaction
//    inst->sdhc->CC2R.reg = SDHC_CC2R_FSDCLKD_DISABLE;
    // Go to idle state
    inst->state = SDHC_IDLE;
    return 0;
}




/** Send CMD17 to read a single block or CMD18 to read multiple blocks */
static int sdhc_state_handler_read(struct sdhc_desc_t *inst)
{
    enum sdhc_substate_rsp res;
    res = sdhc_handle_read(inst, (inst->block_count > 1) ? SD_CMD18 : SD_CMD17,
                           inst->op_addr, inst->block_count, SD_BLOCK_LENGTH,
                           inst->read_buffer);
    union sd_card_status_rsp rsp;

    switch (res) {
        case SDHC_SUBSTATE_RSP_DONE:
            // Check CMD response
            rsp = sd_get_card_status_rsp(inst->sdhc->RR);

            if (rsp.error || rsp.cc_error || rsp.illegal_comand ||
                rsp.com_crc_error || rsp.card_is_locked) {
                // Command failed
                inst->callback(inst->cb_context, SD_OP_FAILED, 0);
            }

            // Success
            inst->callback(inst->cb_context, SD_OP_SUCCESS, inst->block_count);

            // Go back to idle
            inst->op_retry_count = 0;
            inst->state = SDHC_IDLE;
            return 1;
        case SDHC_SUBSTATE_RSP_LATER:
            return 0;
        case SDHC_SUBSTATE_RSP_AGAIN:
            return 1;
        case SDHC_SUBSTATE_RSP_CMD_LINE_CONFLICT:
        case SDHC_SUBSTATE_RSP_CMD_TIMEOUT:
        case SDHC_SUBSTATE_RSP_CMD_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_CMD_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_TRAN_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_TRAN_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_FAILED:
            // Try again later
            //sdhc_op_retry(inst);
            //break;
        case SDHC_SUBSTATE_RSP_TRAN_TIMEOUT:
        case SDHC_SUBSTATE_RSP_ACMD_ERROR:
        case SDHC_SUBSTATE_RSP_ADMA_ERROR:
            // Abort and retry
            sdhc_op_abort_retry(inst);
            break;
        default:
            // This shouldn't happen
            inst->state = SDHC_FAILED;
            return 0;
    }

    // Error case
    if (inst->state == SDHC_FAILED) {
        inst->callback(inst->cb_context, SD_OP_FAILED, 0);
        inst->state = SDHC_IDLE;
    }
    return 1;
}

/** Send CMD24 to write a single block or CMD25 to write multiple blocks */
static int sdhc_state_handler_write(struct sdhc_desc_t *inst)
{
    enum sdhc_substate_rsp res;
    res = sdhc_handle_write(inst, (inst->block_count > 1) ? SD_CMD25 : SD_CMD24,
                           inst->op_addr, inst->block_count, inst->write_data);
    union sd_card_status_rsp rsp;

    switch (res) {
        case SDHC_SUBSTATE_RSP_DONE:
            // Check CMD response
            rsp = sd_get_card_status_rsp(inst->sdhc->RR);

            if (rsp.error || rsp.cc_error || rsp.illegal_comand ||
                rsp.com_crc_error || rsp.card_is_locked) {
                // Command failed, check how many blocks where written
                inst->state = SDHC_APP_CMD;
                inst->acmd_state = SDHC_GET_NUM_BLOCKS_WRITTEN;
                return 1;
            }

            // Success
            inst->callback(inst->cb_context, SD_OP_SUCCESS, inst->block_count);

            // Go back to idle
            inst->op_retry_count = 0;
            inst->state = SDHC_IDLE;
            return 1;
        case SDHC_SUBSTATE_RSP_LATER:
            return 0;
        case SDHC_SUBSTATE_RSP_AGAIN:
            return 1;
        case SDHC_SUBSTATE_RSP_CMD_LINE_CONFLICT:
        case SDHC_SUBSTATE_RSP_CMD_TIMEOUT:
        case SDHC_SUBSTATE_RSP_CMD_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_CMD_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_TRAN_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_TRAN_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_FAILED:
            // Try again later
            //sdhc_op_retry(inst);
            //break;
        case SDHC_SUBSTATE_RSP_TRAN_TIMEOUT:
        case SDHC_SUBSTATE_RSP_ACMD_ERROR:
        case SDHC_SUBSTATE_RSP_ADMA_ERROR:
            // Abort and retry
            sdhc_op_abort_retry(inst);
            break;
        default:
            // This shouldn't happen
            inst->state = SDHC_FAILED;
            return 0;
    }
    // Error case
    if (inst->state == SDHC_FAILED) {
        inst->state = SDHC_APP_CMD;
        inst->acmd_state = SDHC_GET_NUM_BLOCKS_WRITTEN;
    }
    return 1;
}

/** Send ACMD22 to read number of blocks successfully written */
static int sdhc_state_handler_get_num_blocks_written(struct sdhc_desc_t *inst)
{
    enum sdhc_substate_rsp res;
    res = sdhc_handle_read(inst, SD_ACMD22, 0, 1, sizeof(uint32_t),
                           inst->buffer);
    union sd_card_status_rsp rsp;

    switch (res) {
        case SDHC_SUBSTATE_RSP_DONE:
            // Check CMD response
            rsp = sd_get_card_status_rsp(inst->sdhc->RR);

            if (rsp.error || rsp.cc_error || rsp.illegal_comand ||
                rsp.com_crc_error || rsp.card_is_locked || !rsp.app_cmd) {
                // Command failed try again later
                return sdhc_op_retry(inst);
            }

            // Get number of blocks written and call operation callback
            uint32_t *buf = (uint32_t*)__builtin_assume_aligned(inst->buffer,
                                                                4);
            uint32_t const num_blocks = __builtin_bswap32(*buf);
            inst->callback(inst->cb_context, SD_OP_FAILED, num_blocks);

            // Go back to idle
            inst->op_retry_count = 0;
            inst->state = SDHC_IDLE;
            return 1;
        case SDHC_SUBSTATE_RSP_LATER:
            return 0;
        case SDHC_SUBSTATE_RSP_AGAIN:
            return 1;
        case SDHC_SUBSTATE_RSP_CMD_LINE_CONFLICT:
        case SDHC_SUBSTATE_RSP_CMD_TIMEOUT:
        case SDHC_SUBSTATE_RSP_CMD_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_CMD_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_TRAN_CRC_ERROR:
        case SDHC_SUBSTATE_RSP_TRAN_RSP_ERROR:
        case SDHC_SUBSTATE_RSP_FAILED:
            // Try again later
            inst->state = SDHC_APP_CMD;
            inst->acmd_state = SDHC_GET_NUM_BLOCKS_WRITTEN;
            sdhc_op_retry(inst);
            break;
        case SDHC_SUBSTATE_RSP_TRAN_TIMEOUT:
        case SDHC_SUBSTATE_RSP_ACMD_ERROR:
        case SDHC_SUBSTATE_RSP_ADMA_ERROR:
            // Abort
            inst->abort_recovery_state = SDHC_IDLE;
            inst->state = SDHC_ABORT;
            inst->cmd_start_time = millis;
            inst->callback(inst->cb_context, SD_OP_FAILED, 0);
            return 1;
        default:
            // This shouldn't happen
            inst->state = SDHC_FAILED;
            return 0;
    }
    // Error
    if (inst->state == SDHC_FAILED) {
        inst->state = SDHC_IDLE;
        inst->callback(inst->cb_context, SD_OP_FAILED, 0);
    }
    return 1;
}





static int sdhc_state_handler_failed(struct sdhc_desc_t *inst)
{
    // Make sure that SD clock is off
    inst->sdhc->CCR.bit.SDCLKEN = 0;
    return 0;
}




// MARK: Handlers Table
const sdhc_state_handler_t sdhc_state_handlers[] = {
    sdhc_state_handler_not_present,             // SDHC_NOT_PRESENT
    sdhc_state_handler_idle,                    // SDHC_IDLE
    sdhc_state_handler_app_cmd,                 // SDHC_APP_CMD
    sdhc_state_handler_abort,                   // SDHC_ABORT

    sdhc_state_handler_reset,                   // SDHC_RESET
    sdhc_state_handler_check_voltage,           // SDHC_CHECK_VOLTAGE
    sdhc_state_handler_check_ocr,               // SDHC_CHECK_OCR
    sdhc_state_handler_initialize,              // SDHC_INITIALIZE
    sdhc_state_handler_get_cid,                 // SDHC_GET_CID
    sdhc_state_handler_get_rca,                 // SDHC_GET_RCA
    sdhc_state_handler_set_read_csd,            // SDHC_READ_CSD
    sdhc_state_handler_select,                  // SDHC_SELECT
    sdhc_state_handler_set_high_speed,          // SDHC_SET_HIGH_SPEED
    sdhc_state_handler_set_4_bit,               // SDHC_SET_4_BIT
    sdhc_state_handler_read_scr,                // SDHC_READ_SCR
    sdhc_state_handler_set_block_len,           // SDHC_SET_BLOCK_LEN
    sdhc_state_handler_init_done,               // SDHC_INIT_DONE

    sdhc_state_handler_read,                    // SDHC_READ

    sdhc_state_handler_write,                   // SDHC_WRITE
    sdhc_state_handler_get_num_blocks_written,  // SDHC_GET_NUM_BLOCKS_WRITTEN

    sdhc_state_handler_failed,                  // SDHC_UNUSABLE_CARD
    sdhc_state_handler_failed,                  // SDHC_TOO_MANY_INIT_RETRIES
    sdhc_state_handler_failed,                  // SDHC_INIT_TIMEOUT
    sdhc_state_handler_failed                   // SDHC_FAILED
};
