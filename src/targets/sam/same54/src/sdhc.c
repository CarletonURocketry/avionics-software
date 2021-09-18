/**
 * @file sdhc.c
 * @desc Driver for SD host controller
 * @author Samuel Dewan
 * @date 2021-07-01
 * Last Author:
 * Last Edited On:
 */

#include "sdhc.h"

#include "sdhc-states.h"

//
//
//  Helpers for Public Functions
//
//

struct sdhc_desc_t *sdhc_interrupt_info[SDHC_INST_NUM];

const enum peripheral_generic_clock sdhc_generic_clocks[SDHC_INST_NUM] = {
#ifdef SDHC0
    PERPH_GCLK_SDHC0,
#endif
#ifdef SDHC1
    PERPH_GCLK_SDHC1
#endif
};

const enum peripheral_bus_clock sdhc_bus_clocks[SDHC_INST_NUM] = {
#ifdef SDHC0
    PERPH_BUS_CLK_SDHC0_AHB,
#endif
#ifdef SDHC1
    PERPH_BUS_CLK_SDHC1_AHB
#endif
};

const IRQn_Type sdhc_irqs[SDHC_INST_NUM] = {
#ifdef SDHC0
    SDHC0_IRQn,
#endif
#ifdef SDHC1
    SDHC1_IRQn
#endif
};

static int get_sdhc_instance_num(Sdhc *sdhc)
{
#ifdef SDHC0
    if (sdhc == SDHC0) {
        return 0;
    }
#endif
#ifdef SDHC1
    if (sdhc == SDHC1) {
        return 1;
    }
#endif
    return -1;
}

//
//
//  Public Functions
//
//

void init_sdhc(struct sdhc_desc_t *inst, Sdhc *sdhc, uint32_t clock_freq,
               uint32_t clock_mask)
{
    /* Find instance number for SDHC */
    const int inst_num = get_sdhc_instance_num(sdhc);
    if (inst_num < 0) {
        return;
    }

    /* Initialize driver instance descriptor */
    inst->sdhc = sdhc;
    inst->clock_freq = clock_freq;
    inst->state = SDHC_NOT_PRESENT;
    inst->acmd_state = SDHC_FAILED;
    inst->abort_recovery_state = SDHC_FAILED;
    inst->substate = 0;
    inst->rca = 0;
    inst->waiting_for_interrupt = 0;
    inst->init_cmd_started = 0;
    inst->card_capacity = 0;
    inst->init_retry_count = 0;
    inst->v1_card = 0;
    inst->block_addressed = 0;
    inst->cmd23_supported = 0;
    // Clear operation state
    inst->op_addr = 0;
    inst->callback = NULL;
    inst->cb_context = NULL;
    inst->read_buffer = NULL;
    inst->write_data = NULL;
    inst->block_count = 0;

    /* Enable bus clock for SDHC instance */
    enable_bus_clock(sdhc_bus_clocks[inst_num]);
    /* Select generic clock for SDHC instance */
    set_perph_generic_clock(sdhc_generic_clocks[inst_num], clock_mask);

    /* Reset SDHC instance */
    sdhc->SRR.bit.SWRSTALL = 1;
    while (sdhc->SRR.bit.SWRSTALL);

    /* Configure SDHC instance */
    // Start with a 400 KHz clock, will increase later
    const uint16_t clk_setting = clock_freq / SDHC_CLK_INIT / 2;
    sdhc->CCR.reg = (SDHC_CCR_SDCLKFSEL(clk_setting & 0xFF) |
                     SDHC_CCR_USDCLKFSEL((clk_setting >> 8) & 0x3) |
                     SDHC_CCR_INTCLKEN);
    // Wait for clock to become stable
    while (!sdhc->CCR.bit.INTCLKS);
    // 1 bit mode, no high speed for now, use 32 bit ADMA2
    sdhc->HC1R.reg = SDHC_HC1R_DMASEL_32BIT;
    // Configure timeout value
    sdhc->TCR.reg = SDHC_TCR_DTCVAL(13);

    /* Initialize interrupt info table */
    sdhc_interrupt_info[inst_num] = inst;

    /* Configure interrupts */
    NVIC_SetPriority(sdhc_irqs[inst_num], SDHC_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(sdhc_irqs[inst_num]);

    /* Check if a Card is Present */
    if (sdhc->PSR.bit.CARDINS) {
        // Card is inserted, jump straight into first initialization state
        sdhc_service(inst);
    } else {
        // Need to wait for card to be inserted
        inst->waiting_for_interrupt = 1;
        sdhc->NISTER.reg = SDHC_NISTER_CINS;
        sdhc->NISIER.reg = SDHC_NISIER_CINS;
    }
}

void sdhc_service(struct sdhc_desc_t *inst)
{
    int do_next_state = 1;
    while (do_next_state) {
        if (inst->waiting_for_interrupt) {
            return;
        }

        do_next_state = sdhc_state_handlers[inst->state](inst);
    }
}

enum sdhc_status sdhc_get_status(struct sdhc_desc_t *inst)
{
    switch (inst->state) {
        case SDHC_NOT_PRESENT:
            return SDHC_STATUS_NO_CARD;
        case SDHC_APP_CMD:
        case SDHC_ABORT:
            // Not sure where these two states should go
        case SDHC_RESET:
        case SDHC_CHECK_VOLTAGE:
        case SDHC_CHECK_OCR:
        case SDHC_INITIALIZE:
        case SDHC_GET_CID:
        case SDHC_GET_RCA:
        case SDHC_SELECT:
        case SDHC_SET_HIGH_SPEED:
        case SDHC_SET_4_BIT:
        case SDHC_READ_CSD:
        case SDHC_READ_SCR:
        case SDHC_SET_BLOCK_LEN:
        case SDHC_INIT_DONE:
            return SDHC_STATUS_INITIALIZING;
        case SDHC_IDLE:
        case SDHC_READ:
        case SDHC_WRITE:
        case SDHC_GET_NUM_BLOCKS_WRITTEN:
            return SDHC_STATUS_READY;
        case SDHC_UNUSABLE_CARD:
            return SDHC_STATUS_UNUSABLE_CARD;
        case SDHC_TOO_MANY_INIT_RETRIES:
            return SDHC_STATUS_TOO_MANY_INIT_RETRIES;
        case SDHC_INIT_TIMEOUT:
            return SDHC_STATUS_INIT_TIMEOUT;
        case SDHC_FAILED:
        default:
            return SDHC_STATUS_FAILED;
    }
}

//
//
//  SD Interface Functions
//
//

static inline int sdhc_start_op(struct sdhc_desc_t *inst, uint32_t addr,
                                uint32_t num_blocks, sd_op_cb_t cb,
                                void *context)
{
    if (inst->state != SDHC_IDLE) {
        // Either we are not done initializing the card, there is another
        // operation ongoing or the driver is in a failed state
        return 1;
    }

    if ((num_blocks == 0) ||
        (__builtin_mul_overflow_p(num_blocks, SD_BLOCK_LENGTH,
                                  inst->adma2_desc.length))) {
        // Too few or too many blocks
        return 1;
    }

    // Convert address to byte address if nessesary
    if (!inst->block_addressed && __builtin_mul_overflow(addr, SD_BLOCK_LENGTH,
                                                         &addr)) {
        // Address overflowed
        return 1;
    }

    // Check that address if valid
    if (__builtin_add_overflow_p(addr, num_blocks, inst->card_capacity) ||
        ((addr + num_blocks) > inst->card_capacity)) {
        return 1;
    }

    // Set up operation state
    inst->op_addr = addr;
    inst->callback = cb;
    inst->cb_context = context;
    inst->block_count = num_blocks;

    return 0;
}

static int sdhc_read(sd_desc_ptr_t inst, uint32_t addr, uint32_t num_blocks,
                     uint8_t *buffer, sd_op_cb_t cb, void *context)
{
    int const ret = sdhc_start_op(inst.sdhc, addr, num_blocks, cb, context);

    if (ret != 0) {
        return ret;
    }

    inst.sdhc->read_buffer = buffer;

    // Jump to correct driver state to start operation
    inst.sdhc->state = SDHC_READ;

    // Run the service function to get started right away
    sdhc_service(inst.sdhc);

    return 0;
}

static int sdhc_write(sd_desc_ptr_t inst, uint32_t addr, uint32_t num_blocks,
                      uint8_t const *data, sd_op_cb_t cb, void *context)
{
    int const ret = sdhc_start_op(inst.sdhc, addr, num_blocks, cb, context);

    if (ret != 0) {
        return ret;
    }

    inst.sdhc->write_data = data;

    // Jump to correct driver state to start operation
    inst.sdhc->state = SDHC_WRITE;

    // Run the service function to get started right away
    sdhc_service(inst.sdhc);

    return 0;
}

static enum sd_status sdhc_get_sd_status(sd_desc_ptr_t inst)
{
    switch (sdhc_get_status(inst.sdhc)) {
        case SDHC_STATUS_NO_CARD:
            return SD_STATUS_NOT_PRESENT;
        case SDHC_STATUS_INITIALIZING:
            return SD_STATUS_INITIALIZING;
        case SDHC_STATUS_READY:
            return SD_STATUS_READY;
        case SDHC_STATUS_UNUSABLE_CARD:
        case SDHC_STATUS_TOO_MANY_INIT_RETRIES:
        case SDHC_STATUS_INIT_TIMEOUT:
        case SDHC_STATUS_FAILED:
        default:
            return SD_STATUS_FAILED;
    }
}

static uint32_t sdhc_get_num_blocks(sd_desc_ptr_t inst)
{
    return inst.sdhc->card_capacity;
}


struct sd_funcs const sdhc_sd_funcs = {
    .read = &sdhc_read,
    .write = &sdhc_write,
    .get_status = sdhc_get_sd_status,
    .get_num_blocks = sdhc_get_num_blocks
};


//
//
//  Interrupt Service Routines
//
//

static void sdhc_isr(struct sdhc_desc_t *inst)
{
    // Card inserted
    if (inst->sdhc->NISTR.bit.CINS) {
        // Clear interrupt
        inst->sdhc->NISTR.reg = SDHC_NISTR_CINS;
        // Card inserted go to first initialization state
        inst->waiting_for_interrupt = 0;
    }

    // Card removed
    if (inst->sdhc->NISTR.bit.CREM) {
        // Clear interrupt
        inst->sdhc->NISTR.reg = SDHC_NISTR_CREM;
        // Disable clock and SD bus power
        inst->sdhc->CCR.bit.SDCLKEN = 0;
        inst->sdhc->PCR.bit.SDBPWR = 0;
        inst->sdhc->SRR.reg = SDHC_SRR_SWRSTCMD;
        inst->sdhc->SRR.reg = SDHC_SRR_SWRSTDAT;
        inst->sdhc->NISTR.reg = 0xFF;
        inst->sdhc->EISTR.reg = 0xFF;
        inst->sdhc->EISTER.reg = 0;
        inst->sdhc->EISIER.reg = 0;
        // Call callback if an operation is ongoing
        if ((inst->state == SDHC_READ) | (inst->state == SDHC_WRITE) |
            (inst->state == SDHC_GET_NUM_BLOCKS_WRITTEN)) {
            if (inst->callback != NULL) {
                inst->callback(inst, SD_OP_FAILED, 0);
            }
        }
        // Reset driver state
        inst->state = SDHC_NOT_PRESENT;
        inst->waiting_for_interrupt = 1;
        inst->sdhc->NISTER.reg = SDHC_NISTER_CINS;
        inst->sdhc->NISIER.reg = SDHC_NISIER_CINS;
        return;
    }

    // Error
    if (inst->sdhc->NISTR.bit.ERRINT) {
        if (inst->sdhc->EISTR.reg & (SDHC_EISTR_CMDTEO |
                                     SDHC_EISTR_CMDCRC |
                                     SDHC_EISTR_CMDEND |
                                     SDHC_EISTR_CMDIDX)) {
            // Reset CMD line
            inst->sdhc->SRR.reg = SDHC_SRR_SWRSTCMD;
            // A command error occured (substate handler must clear error bits)
            inst->substate = SDHC_SUBSTATE_CMD_ERROR;
            inst->waiting_for_interrupt = 0;
        } else if (inst->sdhc->EISTR.reg & (SDHC_EISTR_DATTEO |
                                            SDHC_EISTR_DATCRC |
                                            SDHC_EISTR_DATEND |
                                            SDHC_EISTR_ACMD |
                                            SDHC_EISTR_ADMA)) {
            // Reset DAT line
            inst->sdhc->SRR.reg = SDHC_SRR_SWRSTDAT;
            // A transfer error occured (substate handler must clear error bits)
            inst->substate = SDHC_SUBSTATE_TRAN_ERROR;
            inst->waiting_for_interrupt = 0;
        }
    }

    // Command complete
    if (inst->sdhc->NISTR.bit.CMDC) {
        // Clear flag
        inst->sdhc->NISTR.reg = SDHC_NISTR_CMDC;
        // Update substate
        inst->substate = SDHC_SUBSTATE_CMD_DONE;
        inst->waiting_for_interrupt = 0;
    }

    // Transfer complete
    if (inst->sdhc->NISTR.bit.TRFC) {
        // Clear flag
        inst->sdhc->NISTR.reg = SDHC_NISTR_TRFC;
        // Update substate
        inst->substate = SDHC_SUBSTATE_TRAN_DONE;
        inst->waiting_for_interrupt = 0;
    }


    int const ret = sdhc_state_handlers[inst->state](inst);
    if (ret) {
        (void)sdhc_state_handlers[inst->state](inst);
    }
}

#ifdef SDHC0
void SDHC0_Handler(void)
{
    sdhc_isr(sdhc_interrupt_info[0]);
}
#endif

#ifdef SDHC1
void SDHC1_Handler(void)
{
    sdhc_isr(sdhc_interrupt_info[1]);
}
#endif
