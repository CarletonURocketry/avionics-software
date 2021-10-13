//
//  sdhc-test.c
//  index
//
//  Created by Samuel Dewan on 2021-07-28.
//  Copyright © 2021 Samuel Dewan. All rights reserved.
//

#include "sdhc-test.h"
#include "sd-commands.h"


#define SDHC_ADMA2_DESC_ACT_NOP_Val     0b00
#define SDHC_ADMA2_DESC_ACT_TRAN_Val    0b10
#define SDHC_ADMA2_DESC_ACT_LINK_Val    0b11

#define SDHC_ADMA2_DESC_VALID       (1 << 0)
#define SDHC_ADMA2_DESC_END         (1 << 1)
#define SDHC_ADMA2_DESC_INTERRUPT   (1 << 2)
#define SDHC_ADMA2_DESC_ACT(x)      ((x & 0x3) << 4)
#define SDHC_ADMA2_DESC_ACT_NOP     SDHC_ADMA2_DESC_ACT(SDHC_ADMA2_DESC_ACT_NOP_Val)
#define SDHC_ADMA2_DESC_ACT_TRAN    SDHC_ADMA2_DESC_ACT(SDHC_ADMA2_DESC_ACT_TRAN_Val)
#define SDHC_ADMA2_DESC_ACT_LINK    SDHC_ADMA2_DESC_ACT(SDHC_ADMA2_DESC_ACT_LINK_Val)

struct sdhc_adma2_descriptor32 {
    union {
        struct {
            uint16_t valid:1;
            uint16_t end:1;
            uint16_t interrupt:1;
            uint16_t RESERVED:1;
            uint16_t act:2;
            uint16_t RESERVED1:10;
        } bits;
        uint16_t raw;
    } attributes;
    uint16_t    length;
    uint32_t    address;
};



void init_sdhc_test(void)
{
    /* Configure Pin MUX */
    PORT->Group[0].PMUX[4].bit.PMUXE = 0x8;      // PA08: SDCMD
    PORT->Group[0].PINCFG[8].bit.PMUXEN = 0b1;
    PORT->Group[0].PMUX[4].bit.PMUXO = 0x8;      // PA09: SDDAT0
    PORT->Group[0].PINCFG[9].bit.PMUXEN = 0b1;
    PORT->Group[0].PMUX[5].bit.PMUXO = 0x8;      // PA10: SDDAT1
    PORT->Group[0].PINCFG[10].bit.PMUXEN = 0b1;
    PORT->Group[0].PMUX[5].bit.PMUXO = 0x8;      // PA11: SDDAT2
    PORT->Group[0].PINCFG[11].bit.PMUXEN = 0b1;
    PORT->Group[1].PMUX[5].bit.PMUXO = 0x8;      // PB10: SDDAT3
    PORT->Group[1].PINCFG[10].bit.PMUXEN = 0b1;
    PORT->Group[1].PMUX[5].bit.PMUXO = 0x8;      // PB11: SDCK
    PORT->Group[1].PINCFG[11].bit.PMUXEN = 0b1;
    PORT->Group[1].PMUX[6].bit.PMUXE = 0x8;      // PB12: SDCD
    PORT->Group[1].PINCFG[12].bit.PMUXEN = 0b1;

    /* Enable Bus Clock for SDHC0 */
    MCLK->AHBMASK.reg |= MCLK_AHBMASK_SDHC0;

    /* Select Generic Clock for SDHC0 (GLCK5 at 100 MHz from DPLL1) */
    do {
        GCLK->PCHCTRL[SDHC0_GCLK_ID].reg = GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN_GCLK5;
    } while(!GCLK->PCHCTRL[SDHC0_GCLK_ID].bit.CHEN);

    // Slow clock is already selected by initial clock configuration code

    /* Reset SDHC instance */
    SDHC0->SRR.bit.SWRSTALL = 1;
    while (SDHC0->SRR.bit.SWRSTALL);

    /* Configure SDHC instance */
    // Start with a 400 KHz clock, will increase later
    const uint16_t clk_setting = 100000000UL / 400000UL / 2;
    SDHC0->CCR.reg = (SDHC_CCR_SDCLKFSEL(clk_setting & 0xFF) |
                      SDHC_CCR_USDCLKFSEL((clk_setting >> 8) & 0x3) |
                      SDHC_CCR_INTCLKEN);
    // Wait for clock to become stable
    while (!SDHC0->CCR.bit.INTCLKS);
    // 1 bit mode, no high speed for now, use 32 bit ADMA2
    SDHC0->HC1R.reg = SDHC_HC1R_DMASEL_32BIT;
    // Configure timeout value
    SDHC0->TCR.reg = SDHC_TCR_DTCVAL(13);
}



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
               ((command == 12) ? SDHC_CR_CMDTYP_ABORT :
                SDHC_CR_CMDTYP_NORMAL) |
               SDHC_CR_CMDIDX(command));

    return cr;
}

static inline void sdhc_enable_cmd_interrupts(int enable_transfer_wait,
                                              int enable_data_interrupts)
{
    // Enable command complete interrupt and transfer complete interrupt if
    // requested
    SDHC0->NISTER.reg |= (SDHC_NISTER_CMDC |
                          (enable_transfer_wait ? SDHC_NISTER_TRFC : 0));
    SDHC0->NISIER.reg |= (SDHC_NISIER_CMDC |
                          (enable_transfer_wait ? SDHC_NISIER_TRFC : 0));
    // Enable command error interrupts
    SDHC0->EISTER.reg |= (SDHC_EISTER_CMDTEO |
                          SDHC_EISTER_CMDCRC |
                          SDHC_EISTER_CMDEND |
                          SDHC_EISTER_CMDIDX);
    SDHC0->EISIER.reg |= (SDHC_EISIER_CMDTEO |
                          SDHC_EISIER_CMDCRC |
                          SDHC_EISIER_CMDEND |
                          SDHC_EISIER_CMDIDX);

    if (enable_data_interrupts) {
        // Enable data error interrupts
        SDHC0->EISTER.reg |= (SDHC_EISTER_DATTEO |
                              SDHC_EISTER_DATCRC |
                              SDHC_EISTER_DATEND |
                              SDHC_EISTER_ACMD |
                              SDHC_EISTER_ADMA);
        SDHC0->EISIER.reg |= (SDHC_EISIER_DATTEO |
                              SDHC_EISIER_DATCRC |
                              SDHC_EISIER_DATEND |
                              SDHC_EISIER_ACMD |
                              SDHC_EISIER_ADMA);
    } else if (enable_transfer_wait) {
        // Enable data timeout error interrupt
        SDHC0->EISTER.reg |= SDHC_EISTER_DATTEO;
        SDHC0->EISIER.reg |= SDHC_EISIER_DATTEO;
    }
}

static int sdhc_do_cmd(uint8_t command, uint32_t arg,
                       enum sdhc_cmd_rsp_type rsp)
{
    /* Make sure that eveything is idle */
    while (SDHC0->PSR.bit.CMDINHC || SDHC0->PSR.bit.CMDINHD);
    /* Enable required interrupts */
    sdhc_enable_cmd_interrupts(rsp == SDHC_CMD_RSP_TYPE_R1B, 0);
    /* Configure registers for command */
    SDHC0->ARG1R.reg = arg;
    SDHC0->TMR.reg = 0;
    SDHC0->CR = sdhc_get_cr_val(command, rsp, 0);
    /* Wait for command to finish */
    while (!SDHC0->NISTR.bit.CMDC);

    if (SDHC0->NISTR.bit.ERRINT) {
        // Error
        return 1;
    }

    /* Wait for the busy signal if we need to */
    if (rsp == SDHC_CMD_RSP_TYPE_R1B) {
        while (!SDHC0->NISTR.bit.TRFC);

        if (SDHC0->NISTR.bit.ERRINT) {
            // Error
            return 1;
        }
    }

    /* All done! The response, if any, is in the response registers */
    return 0;
}

static int sdhc_do_transfer(uint8_t command, uint32_t arg, uint16_t block_count,
                            uint16_t block_size, uint8_t *buffer, int write)
{
    /* Make sure that eveything is idle */
    while (SDHC0->PSR.bit.CMDINHC || SDHC0->PSR.bit.CMDINHD);
    /* Make sure that SD clock is on */
    SDHC0->CCR.bit.SDCLKEN = 1;
    /* Enable required interrupts */
    sdhc_enable_cmd_interrupts(1, 1);
    /* Configure an ADMA2 descriptor */
    struct sdhc_adma2_descriptor32 adma2_desc = { 0 };
    uint32_t const len = block_count * block_size;
    if (len < 65536) {
        adma2_desc.length = len;
    } else if (len == 65536) {
        adma2_desc.length = 0;
    } else {
        return 1;
    }
    adma2_desc.address = (uint32_t)buffer;
    adma2_desc.attributes.raw = (SDHC_ADMA2_DESC_VALID |
                                 SDHC_ADMA2_DESC_END |
                                 SDHC_ADMA2_DESC_ACT_TRAN);
    /* Set ADMA2 descriptor base address */
    SDHC0->ASAR[0].reg = (uint32_t)&adma2_desc;
    /* Configure registers for command */
    unsigned const multi = block_count > 1;
    SDHC0->SSAR.CMD23.ARG2 = block_count;
    SDHC0->BSR.reg = SDHC_BSR_BLOCKSIZE(block_size);
    SDHC0->BCR.reg = SDHC_BCR_BCNT(block_count);
    SDHC0->ARG1R.reg = arg;
    SDHC0->TMR.reg = (SDHC_TMR_DMAEN_ENABLE |
                      (multi << SDHC_TMR_BCEN_Pos) |
                      SDHC_TMR_ACMDEN_CMD23 |
                      (write ? SDHC_TMR_DTDSEL_WRITE :
                               SDHC_TMR_DTDSEL_READ) |
                      (multi << SDHC_TMR_MSBSEL_Pos));
    SDHC0->CR = sdhc_get_cr_val(command, SDHC_CMD_RSP_TYPE_R1, 1);
    /* Wait for command to finish */
    while (!SDHC0->NISTR.bit.CMDC);

    if (SDHC0->NISTR.bit.ERRINT) {
        // Error
        return 1;
    }

    /* Wait transfer to finish */
    while (!SDHC0->NISTR.bit.TRFC);

    if (SDHC0->NISTR.bit.ERRINT) {
        // Error
        return 1;
    }

    /* All done! */
    return 0;
}



int sdhc_test_init_card(void)
{
    int ret;
    uint32_t arg;
    uint16_t rca;
    uint16_t clk_setting;
    uint8_t buffer[512];

    /* Enable SD card clock */
    SDHC0->CCR.bit.SDCLKEN = 1;
    /* Enable SD bus power */
    SDHC0->PCR.reg = SDHC_PCR_SDBVSEL_3V3 | SDHC_PCR_SDBPWR;

    /* Send command 0 */
    ret = sdhc_do_cmd(SD_CMD0, 0, SDHC_CMD_RSP_TYPE_NONE);
    if (ret != 0) {
        return 1;
    }

    /* Send command 8 */
    arg = (union sd_cmd8_arg){
        .check_pattern = 0xAA,
        .voltage_supplied = SD_VHS_27_36
    }.raw;
    for (;;) {
        ret = sdhc_do_cmd(SD_CMD8, arg, SDHC_CMD_RSP_TYPE_R7);
        if (ret != 0) {
            return 1;
        }

        union sd_cmd8_rsp rsp = sd_get_cmd8_rsp(SDHC0->RR);
        if ((rsp.check_pattern_echo == 0xAA) &&
            (rsp.voltage_accepted == SD_VHS_27_36)) {
            break;
        }
    }

    /* Initialize */
    arg = (union sd_acmd41_arg){
        .volt_range_3V2_3V3 = 1,
        .volt_range_3V3_3V4 = 1,
        .xpc = 1,
        .hcs = 1
    }.raw;
    for (;;) {
        /* Send command 55 */
        ret = sdhc_do_cmd(SD_CMD55, 0, SDHC_CMD_RSP_TYPE_R1);
        if (ret != 0) {
            return 1;
        }

        /* Send send ACMD41 to set supported voltage ranges and init */
        ret = sdhc_do_cmd(SD_ACMD41, arg, SDHC_CMD_RSP_TYPE_R3);
        if (ret != 0) {
            return 1;
        }

        //union sd_acmd41_rsp rsp = (union sd_acmd41_rsp){ .raw = SDHC0->RR[0].reg };
        //if (rsp.busy) {
        //volatile uint32_t test = SDHC0->RR[0].reg;
        if (SDHC0->RR[0].reg & (UINT32_C(1) << 31)) {
            break;
        }

        for (volatile unsigned i = 0; i < 10000; i++);
    }

    /* Send CMD2 to get card ID and go into IDENT state */
    ret = sdhc_do_cmd(SD_CMD2, 0, SDHC_CMD_RSP_TYPE_R2);
    if (ret != 0) {
        return 1;
    }

    /* Send CMD3 to get Relative Address (RCA) for card */
    ret = sdhc_do_cmd(SD_CMD3, 0, SDHC_CMD_RSP_TYPE_R6);
    if (ret != 0) {
        return 1;
    }

    union sd_cmd3_rsp rsp = sd_get_cmd3_rsp(SDHC0->RR);
    rca = rsp.rca;

    // Normally send CMD9 here to read Card Specific Data, skipped because we
    // are using a known card for this test.

    /* Send CMD7 to select card and go into TRAN state */
    arg = (union sd_rca_arg){ .rca = rca }.raw;
    ret = sdhc_do_cmd(SD_CMD7, arg, SDHC_CMD_RSP_TYPE_R1);
    if (ret != 0) {
        return 1;
    }

    /* Increase clock speed to 25 MHz */
    SDHC0->CCR.bit.SDCLKEN = 0;
    clk_setting = 100000000 / 25000000 / 2;
    SDHC0->CCR.reg = (SDHC_CCR_SDCLKFSEL(clk_setting & 0xFF) |
                      SDHC_CCR_USDCLKFSEL((clk_setting >> 8) & 0x3) |
                      SDHC_CCR_INTCLKEN);
    while (!SDHC0->CCR.bit.INTCLKS);
    SDHC0->CCR.bit.SDCLKEN = 1;

    /* Send CMD6 to switch to high speed mode */
    arg = (union sd_cmd6_arg){
        .function_group_1 = SD_FG1_ACCESS_MODE_SDR25,
        .function_group_2 = SD_FG_NO_CHANGE,
        .function_group_3 = SD_FG_NO_CHANGE,
        .function_group_4 = SD_FG_NO_CHANGE,
        .function_group_5 = SD_FG_NO_CHANGE,
        .function_group_6 = SD_FG_NO_CHANGE,
        .mode = 1
    }.raw;
    ret = sdhc_do_transfer(SD_CMD6, arg, 1,
                           sizeof(union sd_switch_function_status_rsp), buffer,
                           0);
    if (ret != 0) {
        return 1;
    }

    /* Switch to high speed mode and increase clock speed to 50 MHz */
    SDHC0->HC1R.bit.HSEN = 1;
    SDHC0->CCR.bit.SDCLKEN = 0;
    clk_setting = 100000000 / 50000000 / 2;
    SDHC0->CCR.reg = (SDHC_CCR_SDCLKFSEL(clk_setting & 0xFF) |
                      SDHC_CCR_USDCLKFSEL((clk_setting >> 8) & 0x3) |
                      SDHC_CCR_INTCLKEN);
    while (!SDHC0->CCR.bit.INTCLKS);
    SDHC0->CCR.bit.SDCLKEN = 1;

    /* Send command 55 */
    ret = sdhc_do_cmd(SD_CMD55, 0, SDHC_CMD_RSP_TYPE_R1);
    if (ret != 0) {
        return 1;
    }

    /* Send ACMD6 to switch to a 4 bit wide bus  */
    arg = (union sd_acmd6_arg){ .bus_width = SD_BUS_WIDTH_4 }.raw;
    ret = sdhc_do_cmd(SD_ACMD6, arg, SDHC_CMD_RSP_TYPE_R1);
    if (ret != 0) {
        return 1;
    }

    /* We need to switch to a 4 bit bus now */
    SDHC0->HC1R.bit.DW = SDHC_HC1R_DW_4BIT_Val;

    // Normally send ACMD51 here to read SD Card Configuration Register (SCR),
    // skipped because we are using a known card for this test.

    /* Send CMD16 to set block length */
    ret = sdhc_do_cmd(SD_CMD16, 512, SDHC_CMD_RSP_TYPE_R1);
    if (ret != 0) {
        return 1;
    }

    SDHC0->CCR.bit.SDCLKEN = 0;
    return 0;
}


int sdhc_test_read(uint32_t address, uint16_t num_blocks,
                   uint8_t *buffer)
{
    int ret = sdhc_do_transfer((num_blocks > 1) ? SD_CMD18 : SD_CMD17, address,
                               num_blocks, 512, buffer, 0);
    SDHC0->CCR.bit.SDCLKEN = 0;
    return ret;
}

int sdhc_test_write(uint32_t address, uint16_t num_blocks,
                    uint8_t *buffer)
{
    int ret = sdhc_do_transfer((num_blocks > 1) ? SD_CMD25 : SD_CMD24, address,
                               num_blocks, 512, buffer, 1);
    SDHC0->CCR.bit.SDCLKEN = 0;
    return ret;
}
