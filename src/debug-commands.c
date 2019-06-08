/**
 * @file debug-commmands.c
 * @desc Functions to be run from CLI for debuging purposes.
 * @author Samuel Dewan
 * @date 2019-09-22
 * Last Author: Samuel Dewan
 * Last Edited On: 2019-04-18
 */

#include "debug-commands.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "global.h"
#include "config.h"

#include "wdt.h"

#include "sercom-i2c.h"
#include "sercom-spi.h"
#include "mcp23s17-registers.h"
#include "adc.h"

#include "ms5611.h"


#define DEBUG_VERSION_NAME  "version"
#define DEBUG_VERSION_HELP  "Get software version information.\n"

static void debug_version (uint8_t argc, char **argv,
                           struct console_desc_t *console)
{
    console_send_str(console, VERSION_STRING);
    console_send_str(console, BUILD_STRING);
    console_send_str(console, "Configuration: ");
    console_send_str(console, CONFIG_STRING);
}



#define DEBUG_DID_NAME  "did"
#define DEBUG_DID_HELP  "Get device identification information.\n"

static void debug_did (uint8_t argc, char **argv,
                       struct console_desc_t *console)
{
    char str[11];
    
    str[0] = '0';
    str[1] = 'x';
    
    console_send_str(console, "Serial Number: ");
    utoa(*((uint32_t*)0x0080A00C), str + 2, 16);
    console_send_str(console, str);
    utoa(*((uint32_t*)0x0080A040), str + 2, 16);
    console_send_str(console, str+2);
    utoa(*((uint32_t*)0x0080A044), str + 2, 16);
    console_send_str(console, str+2);
    utoa(*((uint32_t*)0x0080A048), str + 2, 16);
    console_send_str(console, str+2);
    
    console_send_str(console, "\n\nDevice Identification: ");
    utoa(DSU->DID.reg, str + 2, 16);
    console_send_str(console, str);
    
    console_send_str(console, "\n\tPROCESSOR: ");
    utoa(DSU->DID.bit.PROCESSOR, str + 2, 16);
    console_send_str(console, str);
    
    console_send_str(console, "\n\tFAMILY: ");
    utoa(DSU->DID.bit.FAMILY, str + 2, 16);
    console_send_str(console, str);
    
    console_send_str(console, "\n\tSERIES: ");
    utoa(DSU->DID.bit.SERIES, str + 2, 16);
    console_send_str(console, str);
    
    console_send_str(console, "\n\tDIE: ");
    utoa(DSU->DID.bit.DIE, str + 2, 16);
    console_send_str(console, str);
    
    console_send_str(console, "\n\tREVISION: ");
    utoa(DSU->DID.bit.REVISION, str + 2, 16);
    console_send_str(console, str);
    str[2] = ' ';
    str[3] = '(';
    str[4] = (char)(DSU->DID.bit.REVISION + 'A');
    str[5] = ')';
    str[6] = 0;
    console_send_str(console, str + 2);
    
    console_send_str(console, "\n\tDEVSEL: ");
    utoa(DSU->DID.bit.DEVSEL, str + 2, 16);
    console_send_str(console, str);
    
    console_send_str(console, "\nCPUID: ");
    utoa(SCB->CPUID, str + 2, 16);
    console_send_str(console, str);
    
    console_send_str(console, "\n");
}


#define DEBUG_I2C_SCAN_NAME  "i2c-scan"
#define DEBUG_I2C_SCAN_HELP  "Scan for devices on the I2C bus.\n"

static void debug_i2c_scan (uint8_t argc, char **argv,
                           struct console_desc_t *console)
{
    // Start a scan of the I2C bus
    uint8_t i2c_t_id;
    sercom_i2c_start_scan(&i2c_g, &i2c_t_id);
    
    // Wait for scan to complete
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t_id)) wdt_pat();
    
    // Check if scan completed successfully
    switch (sercom_i2c_transaction_state(&i2c_g, i2c_t_id)) {
        case I2C_STATE_BUS_ERROR:
            console_send_str(console, "Scan failed: Bus Error\n");
            sercom_i2c_clear_transaction(&i2c_g, i2c_t_id);
            return;
        case I2C_STATE_ARBITRATION_LOST:
            console_send_str(console, "Scan failed: Arbitration Lost\n");
            sercom_i2c_clear_transaction(&i2c_g, i2c_t_id);
            return;
        case I2C_STATE_SLAVE_NACK:
            console_send_str(console, "Scan failed: Slave NACK\n");
            sercom_i2c_clear_transaction(&i2c_g, i2c_t_id);
            return;
        default:
            // Success!
            break;
    }
    
    // Print scan results
    char str[8];
    
    console_send_str(console, "Avaliable Devices:\n");
    for (int i = 0; i < 128; i++) {
        if (sercom_i2c_device_avaliable(&i2c_g, i2c_t_id, i)) {
            console_send_str(console, "0b");
            utoa(i, str, 2);
            for (int j = strlen(str); j < 7; j++) {
                console_send_str(console, "0");
            }
            console_send_str(console, str);
            
            console_send_str(console, " (0x");
            utoa(i, str, 16);
            if (strlen(str) == 1) {
                console_send_str(console, "0");
            }
            console_send_str(console, str);
            console_send_str(console, ")\n");
        }
    }
    
    // Clear transaction
    sercom_i2c_clear_transaction(&i2c_g, i2c_t_id);
}


#define DEBUG_ALT_PROM_NAME  "alt-prom"
#define DEBUG_ALT_PROM_HELP  "Read data from altimeter PROM.\n"

static void debug_alt_prom (uint8_t argc, char **argv,
                            struct console_desc_t *console)
{
    uint8_t i2c_t;
    uint16_t data;
    
    char str[6];
    
    // 0: Factory data and setup
    uint8_t cmd = 0b10100000;
    sercom_i2c_start_generic(&i2c_g, &i2c_t, 0b1110110, &cmd, 1, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) {
        sercom_i2c_service(&i2c_g);
        wdt_pat();
    }
    console_send_str(console, "0: 0x");
    utoa(data, str, 16);
    console_send_str(console, str);
    console_send_str(console, " (Factory data and setup)\n");
    sercom_i2c_clear_transaction(&i2c_g, i2c_t);
    
    // C1: Pressure Sensitivity
    //sercom_i2c_start_reg_read(&i2c_g, &i2c_t, 0b1110110, 0b10100100, (uint8_t*)&data, 2);
    cmd = 0b10100100;
    sercom_i2c_start_generic(&i2c_g, &i2c_t, 0b1110110, &cmd, 1, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) {
        sercom_i2c_service(&i2c_g);
        wdt_pat();
    }
    console_send_str(console, "C1: ");
    utoa(data, str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Pressure sensitivity)\n");
    sercom_i2c_clear_transaction(&i2c_g, i2c_t);
    
    // C2: Pressure offset
    sercom_i2c_start_reg_read(&i2c_g, &i2c_t, 0b1110110, 0b10100010, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) {
        sercom_i2c_service(&i2c_g);
        wdt_pat();
    }
    console_send_str(console, "C2: ");
    utoa(data, str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Pressure offset)\n");
    sercom_i2c_clear_transaction(&i2c_g, i2c_t);
    
    // C3: Temperature coefficient of pressure sensitivity
    sercom_i2c_start_reg_read(&i2c_g, &i2c_t, 0b1110110, 0b10100110, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) {
        sercom_i2c_service(&i2c_g);
        wdt_pat();
    }
    console_send_str(console, "C3: ");
    utoa(data, str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Temperature coefficient of pressure sensitivity)\n");
     sercom_i2c_clear_transaction(&i2c_g, i2c_t);
    
    // C4: Temperature coefficient of pressure offset
    sercom_i2c_start_reg_read(&i2c_g, &i2c_t, 0b1110110, 0b10100001, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) {
        sercom_i2c_service(&i2c_g);
        wdt_pat();
    }
    console_send_str(console, "C4: ");
    utoa(data, str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Temperature coefficient of pressure offset)\n");
    sercom_i2c_clear_transaction(&i2c_g, i2c_t);
    
    // C5: Reference temperature
    sercom_i2c_start_reg_read(&i2c_g, &i2c_t, 0b1110110, 0b10100101, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) {
        sercom_i2c_service(&i2c_g);
        wdt_pat();
    }
    console_send_str(console, "C5: ");
    utoa(data, str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Reference temperature)\n");
    sercom_i2c_clear_transaction(&i2c_g, i2c_t);
    
    // C6: Temperature coefficient of the temperature
    sercom_i2c_start_reg_read(&i2c_g, &i2c_t, 0b1110110, 0b10100011, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) {
        sercom_i2c_service(&i2c_g);
        wdt_pat();
    }
    console_send_str(console, "C6: ");
    utoa(data, str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Temperature coefficient of the temperature)\n");
    sercom_i2c_clear_transaction(&i2c_g, i2c_t);
    
    // 7: Serial code and CRC
    cmd = 0b10100111;
    sercom_i2c_start_generic(&i2c_g, &i2c_t, 0b1110110, &cmd, 1, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) {
        sercom_i2c_service(&i2c_g);
        wdt_pat();
    }
    console_send_str(console, "7: 0x");
    utoa(data, str, 16);
    console_send_str(console, str);
    console_send_str(console, " (Serial code and CRC)\n");
    sercom_i2c_clear_transaction(&i2c_g, i2c_t);
}


#define DEBUG_IMU_WAI_NAME  "imu-wai"
#define DEBUG_IMU_WAI_HELP  "Read IMU Who Am I register.\n"

static void debug_imu_wai (uint8_t argc, char **argv,
                           struct console_desc_t *console)
{
    uint8_t i2c_t;
    uint8_t data;
    char str[9];
    
    // Who Am I
    sercom_i2c_start_reg_read(&i2c_g, &i2c_t, 0b1101000, 0x75, &data, 2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) {
        sercom_i2c_service(&i2c_g);
        wdt_pat();
    }
    console_send_str(console, "Who Am I: 0x");
    utoa(data, str, 16);
    console_send_str(console, str);
    console_send_str(console, " (Gyroscope and Accelerometer)\n");
    sercom_i2c_clear_transaction(&i2c_g, i2c_t);
}

#define DEBUG_IO_EXP_REGS_NAME  "io-exp-regs"
#define DEBUG_IO_EXP_REGS_HELP  "Read MCP23S17 registers.\n"\
                                "Usage: io-exp-regs [address]\n"

static void debug_print_byte_with_pad (struct console_desc_t *console,
                                       const char *line_start, uint8_t byte,
                                       const char *line_end)
{
    char str[9];
    
    console_send_str(console, line_start);
    
    utoa(byte, str, 2);
    for (uint8_t i = strlen(str); i < 8; i++) {
        console_send_str(console, "0");
    }
    console_send_str(console, str);
    
    console_send_str(console, line_end);
}

static void debug_io_exp_regs (uint8_t argc, char **argv,
                               struct console_desc_t *console)
{
    uint8_t address = 0;
    
    if (argc > 2) {
        console_send_str(console, DEBUG_IO_EXP_REGS_HELP);
        return;
    } else if (argc == 2) {
        char* end;
        
        uint32_t addr = strtoul(argv[1], &end, 0);
        if ((*end != '\0') || (addr > 7)) {
            console_send_str(console, DEBUG_IO_EXP_REGS_HELP);
            return;
        }
        
        address = (uint8_t)addr;
    }
    
    uint8_t command[] = {((MCP23S17_ADDR | address) << 1) | 1, 0};
    struct mcp23s17_register_map registers;
    
    
    uint8_t t_id;
    uint8_t s = sercom_spi_start(&spi_g, &t_id, 8000000UL,
                                 IO_EXPANDER_CS_PIN_GROUP,
                                 IO_EXPANDER_CS_PIN_MASK, command, 2,
                                 (uint8_t*)&registers, 10);
    
    if (s) {
        console_send_str(console, "Failed to queue SPI transaction.\n");
        return;
    }
    
    while (!sercom_spi_transaction_done(&spi_g, t_id)) wdt_pat();
    sercom_spi_clear_transaction(&spi_g, t_id);
    
    command[1] = 0x0B;
    s = sercom_spi_start(&spi_g, &t_id, 8000000UL, IO_EXPANDER_CS_PIN_GROUP,
                         IO_EXPANDER_CS_PIN_MASK, command, 2,
                         (uint8_t*)&registers.IOCON, 11);
    
    if (s) {
        console_send_str(console, "Failed to queue SPI transaction.\n");
        return;
    }
    
    debug_print_byte_with_pad(console, "   IODIRA: 0b", registers.IODIR[0].reg, "\n");
    debug_print_byte_with_pad(console, "   IODIRB: 0b", registers.IODIR[1].reg, "\n\n");
    debug_print_byte_with_pad(console, "    IPOLA: 0b", registers.IPOL[0].reg, "\n");
    debug_print_byte_with_pad(console, "    IPOLB: 0b", registers.IPOL[1].reg, "\n\n");
    wdt_pat();
    debug_print_byte_with_pad(console, " GPINTENA: 0b", registers.GPINTEN[0].reg, "\n");
    debug_print_byte_with_pad(console, " GPINTENB: 0b", registers.GPINTEN[1].reg, "\n\n");
    debug_print_byte_with_pad(console, "  DEFVALA: 0b", registers.DEFVAL[0].reg, "\n");
    debug_print_byte_with_pad(console, "  DEFVALB: 0b", registers.DEFVAL[1].reg, "\n\n");
    wdt_pat();
    debug_print_byte_with_pad(console, "  INTCONA: 0b", registers.INTCON[0].reg, "\n");
    debug_print_byte_with_pad(console, "  INTCONB: 0b", registers.INTCON[1].reg, "\n\n");
    
    while (!sercom_spi_transaction_done(&spi_g, t_id)) wdt_pat();
    sercom_spi_clear_transaction(&spi_g, t_id);
    
    debug_print_byte_with_pad(console, "   IOCON: 0b", registers.IOCON.reg, "\n\n");
    debug_print_byte_with_pad(console, "   GPPUA: 0b", registers.GPPU[0].reg, "\n");
    debug_print_byte_with_pad(console, "   GPPUB: 0b", registers.GPPU[1].reg, "\n\n");
    wdt_pat();
    debug_print_byte_with_pad(console, "   INTFA: 0b", registers.INTF[0].reg, "\n");
    debug_print_byte_with_pad(console, "   INTFB: 0b", registers.INTF[1].reg, "\n\n");
    debug_print_byte_with_pad(console, " INTCAPA: 0b", registers.INTCAP[0].reg, "\n");
    wdt_pat();
    debug_print_byte_with_pad(console, " INTCAPB: 0b", registers.INTCAP[1].reg, "\n\n");
    debug_print_byte_with_pad(console, "   GPIOA: 0b", registers.GPIO[0].reg, "\n");
    debug_print_byte_with_pad(console, "   GPIOB: 0b", registers.GPIO[1].reg, "\n\n");
    wdt_pat();
    debug_print_byte_with_pad(console, "   OLATA: 0b", registers.OLAT[0].reg, "\n");
    debug_print_byte_with_pad(console, "   OLATB: 0b", registers.OLAT[1].reg, "\n");
}

#define DEBUG_TEMP_NAME  "temp"
#define DEBUG_TEMP_HELP  "Read internal temperature sensor and the NVM "\
                         "temperature log row.\n"

static void debug_temp (uint8_t argc, char **argv,
                        struct console_desc_t *console)
{
    // Read tempurature log values from
    uint8_t room_temp_val_int = (uint8_t)(((*((uint32_t*)
                                    NVMCTRL_FUSES_ROOM_TEMP_VAL_INT_ADDR)) &
                                        NVMCTRL_FUSES_ROOM_TEMP_VAL_INT_Msk) >>
                                          NVMCTRL_FUSES_ROOM_TEMP_VAL_INT_Pos);
    uint8_t room_temp_val_dec = (uint8_t)(((*((uint32_t*)
                                    NVMCTRL_FUSES_ROOM_TEMP_VAL_DEC_ADDR)) &
                                        NVMCTRL_FUSES_ROOM_TEMP_VAL_DEC_Msk) >>
                                          NVMCTRL_FUSES_ROOM_TEMP_VAL_DEC_Pos);
    uint8_t hot_temp_val_int = (uint8_t)(((*((uint32_t*)
                                        NVMCTRL_FUSES_HOT_TEMP_VAL_INT_ADDR)) &
                                        NVMCTRL_FUSES_HOT_TEMP_VAL_INT_Msk) >>
                                          NVMCTRL_FUSES_HOT_TEMP_VAL_INT_Pos);
    uint8_t hot_temp_val_dec = (uint8_t)(((*((uint32_t*)
                                        NVMCTRL_FUSES_HOT_TEMP_VAL_DEC_ADDR)) &
                                        NVMCTRL_FUSES_HOT_TEMP_VAL_DEC_Msk) >>
                                          NVMCTRL_FUSES_HOT_TEMP_VAL_DEC_Pos);
    int8_t room_int1v_val = (int8_t)(((*((uint32_t*)
                                        NVMCTRL_FUSES_ROOM_INT1V_VAL_ADDR)) &
                                          NVMCTRL_FUSES_ROOM_INT1V_VAL_Msk) >>
                                         NVMCTRL_FUSES_ROOM_INT1V_VAL_Pos);
    int8_t hot_int1v_val = (int8_t)(((*((uint32_t*)
                                        NVMCTRL_FUSES_HOT_INT1V_VAL_ADDR)) &
                                          NVMCTRL_FUSES_HOT_INT1V_VAL_Msk) >>
                                         NVMCTRL_FUSES_HOT_INT1V_VAL_Pos);
    
    uint16_t room_adc_val = (uint16_t)(((*((uint32_t*)
                                         NVMCTRL_FUSES_ROOM_ADC_VAL_ADDR)) &
                                        NVMCTRL_FUSES_ROOM_ADC_VAL_Msk) >>
                                       NVMCTRL_FUSES_ROOM_ADC_VAL_Pos);
    uint16_t hot_adc_val = (uint16_t)(((*((uint32_t*)
                                           NVMCTRL_FUSES_HOT_ADC_VAL_ADDR)) &
                                        NVMCTRL_FUSES_HOT_ADC_VAL_Msk) >>
                                       NVMCTRL_FUSES_HOT_ADC_VAL_Pos);
    
    char str[8];
    
    console_send_str(console, "Temperature (course): ");
    int16_t t_course = adc_get_temp_course();
    itoa(t_course / 100, str, 10);
    console_send_str(console, str);
    console_send_str(console, ".");
    utoa(abs(t_course % 100), str, 10);
    console_send_str(console, str);
    console_send_str(console, " C\nTemperature (fine): ");
    int16_t t_fine = adc_get_temp_fine();
    itoa(t_fine / 100, str, 10);
    console_send_str(console, str);
    console_send_str(console, ".");
    utoa(abs(t_fine % 100), str, 10);
    console_send_str(console, str);
    
    wdt_pat();
    
    console_send_str(console, " C\n\nTemperature Log Row:");
    
    console_send_str(console, "\n     ROOM_TEMP_VAL: ");
    utoa(room_temp_val_int, str, 10);
    console_send_str(console, str);
    console_send_str(console, ".");
    utoa(room_temp_val_dec, str, 10);
    console_send_str(console, str);
    
    console_send_str(console, " C\n      HOT_TEMP_VAL: ");
    utoa(hot_temp_val_int, str, 10);
    console_send_str(console, str);
    console_send_str(console, ".");
    utoa(hot_temp_val_dec, str, 10);
    console_send_str(console, str);
    
    wdt_pat();
    
    console_send_str(console, " C\n    ROOM_INT1V_VAL: ");
    console_send_str(console, (room_int1v_val >= 0) ? "1." : "0.");
    utoa((room_int1v_val >= 0) ? ((unsigned int)room_int1v_val) :
         ((unsigned int)(1000 + room_int1v_val)), str, 10);
    console_send_str(console, str);
    
    console_send_str(console, " V\n     HOT_INT1V_VAL: ");
    console_send_str(console, (hot_int1v_val >= 0) ? "1." : "0.");
    utoa((hot_int1v_val >= 0) ? ((unsigned int)hot_int1v_val) :
         ((unsigned int)(1000 + hot_int1v_val)), str, 10);
    console_send_str(console, str);
    
    wdt_pat();
    
    console_send_str(console, " V\n      ROOM_ADC_VAL: 0x");
    utoa(room_adc_val, str, 16);
    console_send_str(console, str);
    
    console_send_str(console, "\n       HOT_ADC_VAL: 0x");
    utoa(hot_adc_val, str, 16);
    console_send_str(console, str);
    console_send_str(console, "\n");
}


#define DEBUG_ANALOG_NAME  "analog"
#define DEBUG_ANALOG_HELP  "Print values of analog inputs.\nUsage: analog "\
                           "[pin numbering]\nPin numbering should be one of"\
                           "internal or header.\n"

static void debug_analog_print_channel (struct console_desc_t *console,
                                        uint8_t channel, int32_t parsed_value,
                                        uint32_t scale, const char *name,
                                        const char *unit)
{
    char str[10];
    
    if (*name == '\0') {
        if (channel < 10) {
            console_send_str(console, "  ");
        } else {
            console_send_str(console, " ");
        }
        utoa(channel, str, 10);
        console_send_str(console, str);
    } else {
        console_send_str(console, name);
    }
    console_send_str(console, ": 0x");
    
    utoa(adc_get_value(channel), str, 16);
    console_send_str(console, str);
    
    if (*unit != '\0') {
        console_send_str(console, " (");
        itoa(parsed_value / scale, str, 10);
        console_send_str(console, str);
        console_send_str(console, ".");
        utoa(abs(parsed_value % scale), str, 10);
        console_send_str(console, str);
        console_send_str(console, " ");
        console_send_str(console, unit);
        console_send_str(console, ")\n");
    } else {
        console_send_str(console, "\n");
    }
}

static void debug_analog (uint8_t argc, char **argv,
                          struct console_desc_t *console)
{
    uint8_t numbering = 0;
    if ((argc == 2) && !strcasecmp(argv[1], "internal")) {
        numbering = 1;
    } else if ((argc == 2) && !strcasecmp(argv[1], "header")) {
        numbering = 0;
    } else if (argc != 1) {
        console_send_str(console, DEBUG_ANALOG_HELP);
        return;
    }
    
    uint32_t channel_mask = adc_get_channel_mask();
    
    if (numbering) {
        // Use internal pin numbers
        
        // Iterate over all enabled external channels
        uint32_t int_chans = channel_mask & 0xFFFFF;
        
        while (int_chans != 0) {
            uint32_t t = int_chans & -int_chans;
            
            uint8_t i = __builtin_ctzl(int_chans);
            uint16_t volts = adc_get_value_millivolts(i);
            debug_analog_print_channel(console, i, (uint32_t)volts, 1000, "",
                                       "V");
            
            int_chans ^= t;
            
            wdt_pat();
        }
    } else {
        // Use header pin numbers
        uint8_t analog_header_chans[] = HEADER_ANALOG_PINS;
        // Iterate over all of the analog header pins
        for (uint8_t i = 0; i < NUM_ANALOG_HEADER_PINS; i++) {
            uint16_t volts = adc_get_value_millivolts(analog_header_chans[i]);
            debug_analog_print_channel(console, analog_header_chans[i],
                                       (uint32_t)volts, 1000, "", "V");
            
            wdt_pat();
        }
    }
    
    
    // Iterate over enabled internal channels
    channel_mask &= 0x1F000000;
    
    while (channel_mask != 0) {
        uint32_t t = channel_mask & -channel_mask;
        int i = __builtin_ctzl(channel_mask);
        
        if (i == ADC_INPUTCTRL_MUXPOS_TEMP_Val) {
            int16_t temp = adc_get_temp_fine();
            debug_analog_print_channel(console, i, (uint32_t)temp, 100,
                                       "Temperature", "C");
        } else if (i == ADC_INPUTCTRL_MUXPOS_BANDGAP_Val) {
            uint16_t volts = adc_get_value_millivolts(i);
            debug_analog_print_channel(console, i, (uint32_t)volts, 1000,
                                       "Bandgap", "V");
        } else if (i == ADC_INPUTCTRL_MUXPOS_SCALEDCOREVCC_Val) {
            int16_t volts = adc_get_core_vcc();
            debug_analog_print_channel(console, i, (uint32_t)volts, 1000,
                                       "Core VCC", "V");
        } else if (i == ADC_INPUTCTRL_MUXPOS_SCALEDIOVCC_Val) {
            int16_t volts = adc_get_io_vcc();
            debug_analog_print_channel(console, i, (uint32_t)volts, 1000,
                                       "IO VCC", "V");
        } else if (i == ADC_INPUTCTRL_MUXPOS_DAC_Val) {
            uint16_t volts = adc_get_value_millivolts(i);
            debug_analog_print_channel(console, i, (uint32_t)volts, 1000,
                                       "DAC", "V");
        } else {
            uint16_t volts = adc_get_value_millivolts(i);
            debug_analog_print_channel(console, i, (uint32_t)volts, 1000, "",
                                       "V");
        }
        
        channel_mask ^= t;
        wdt_pat();
    }
}

#define DEBUG_ALT_NAME  "alt-test"
#define DEBUG_ALT_HELP  "Print most recent values from altimeter."

static void print_fixed_point (struct console_desc_t *console, int32_t value,
                               uint8_t decimal_places)
{
    uint32_t scale = pow(10, decimal_places);
    
    char str[10];
    int32_t whole = value / scale;
    itoa(whole, str, 10);
    console_send_str(console, str);
    console_send_str(console, ".");
    
    int32_t frac = abs(value - (whole * scale));
    itoa(frac, str, 10);
    for (int i = strlen(str); i < decimal_places; i++) {
        console_send_str(console, "0");
    }
    console_send_str(console, str);
}

static void debug_alt (uint8_t argc, char **argv,
                       struct console_desc_t *console)
{
#ifndef ENABLE_ALTIMETER
    console_send_str(console, "Altimeter is not enabled in compile time "
                        "configuration.\n");
    return;
#endif
    char str[16];
    
    console_send_str(console, "PROM Values:\n");
    // C1: Pressure Sensitivity
    console_send_str(console, "C1: ");
    utoa(altimeter_g.prom_values[0], str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Pressure sensitivity)\n");
    
    // C2: Pressure offset
    console_send_str(console, "C2: ");
    utoa(altimeter_g.prom_values[1], str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Pressure offset)\n");
    
    // C3: Temperature coefficient of pressure sensitivity
    console_send_str(console, "C3: ");
    utoa(altimeter_g.prom_values[2], str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Temperature coefficient of pressure sensitivity)\n");
    
    // C4: Temperature coefficient of pressure offset
    console_send_str(console, "C4: ");
    utoa(altimeter_g.prom_values[3], str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Temperature coefficient of pressure offset)\n");
    
    // C5: Reference temperature
    console_send_str(console, "C5: ");
    utoa(altimeter_g.prom_values[4], str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Reference temperature)\n");
    
    // C6: Temperature coefficient of the temperature
    console_send_str(console, "C6: ");
    utoa(altimeter_g.prom_values[5], str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Temperature coefficient of the temperature)\n");
    
    // Last reading time
    uint32_t last_reading_time = ms5611_get_last_reading_time(&altimeter_g);
    console_send_str(console, "Last reading at ");
    utoa(last_reading_time, str, 10);
    console_send_str(console, str);
    console_send_str(console, " (");
    utoa(millis - last_reading_time, str, 10);
    console_send_str(console, str);
    console_send_str(console, "  milliseconds ago)\n");
    
    // Pressure
    console_send_str(console, "Pressure: ");
    print_fixed_point(console, altimeter_g.pressure, 2);
    console_send_str(console, str);
    
    // Temperature
    console_send_str(console, " mbar\nTemperature: ");
    print_fixed_point(console, altimeter_g.temperature, 2);
    console_send_str(console, str);
    
    // Altitude
    int32_t altitude = (int32_t)(altimeter_g.altitude * 100);
    console_send_str(console, " C\nAltitude: ");
    print_fixed_point(console, altitude, 2);
    console_send_str(console, str);
    console_send_str(console, " m\n");
}



const uint8_t debug_commands_num_funcs = 8;
const struct cli_func_desc_t debug_commands_funcs[] = {
    {.func = debug_version, .name = DEBUG_VERSION_NAME, .help_string = DEBUG_VERSION_HELP},
    {.func = debug_did, .name = DEBUG_DID_NAME, .help_string = DEBUG_DID_HELP},
    {.func = debug_i2c_scan, .name = DEBUG_I2C_SCAN_NAME, .help_string = DEBUG_I2C_SCAN_HELP},
    {.func = debug_alt_prom, .name = DEBUG_ALT_PROM_NAME, .help_string = DEBUG_ALT_PROM_HELP},
    {.func = debug_imu_wai, .name = DEBUG_IMU_WAI_NAME, .help_string = DEBUG_IMU_WAI_HELP},
    {.func = debug_io_exp_regs, .name = DEBUG_IO_EXP_REGS_NAME, .help_string = DEBUG_IO_EXP_REGS_HELP},
    {.func = debug_temp, .name = DEBUG_TEMP_NAME, .help_string = DEBUG_TEMP_HELP},
    {.func = debug_analog, .name = DEBUG_ANALOG_NAME, .help_string = DEBUG_ANALOG_HELP},
    {.func = debug_alt, .name = DEBUG_ALT_NAME, .help_string = DEBUG_ALT_HELP}
};
