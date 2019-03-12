/**
 * @file debug-commmands.c
 * @desc Functions to be run from CLI for debuging purposes.
 * @author Samuel Dewan
 * @date 2019-09-22
 * Last Author: Samuel Dewan
 * Last Edited On: 2019-09-23
 */

#include "debug-commands.h"

#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "config.h"

#include "sercom-i2c.h"



#define DEBUG_VERSION_NAME  "version"
#define DEBUG_VERSION_HELP  "Get software version information."

static void debug_version (uint8_t argc, char **argv,
                           struct console_desc_t *console)
{
    console_send_str(console, VERSION_STRING);
    console_send_str(console, BUILD_STRING);
    console_send_str(console, "Configuration: ");
    console_send_str(console, CONFIG_STRING);
}



#define DEBUG_DID_NAME  "did"
#define DEBUG_DID_HELP  "Get device identification information."

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
#define DEBUG_I2C_SCAN_HELP  "Scan for devices on the I2C bus."

static void debug_i2c_scan (uint8_t argc, char **argv,
                           struct console_desc_t *console)
{
    // Start a scan of the I2C bus
    uint8_t i2c_t_id;
    sercom_i2c_start_scan(&i2c_g, &i2c_t_id);
    
    // Wait for scan to complete
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t_id));
    
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
#define DEBUG_ALT_PROM_HELP  "Read data from altimeter PROM."

static void debug_alt_prom (uint8_t argc, char **argv,
                            struct console_desc_t *console)
{
    uint8_t i2c_t;
    uint16_t data;
    
    char str[6];
    
    // 0: Factory data and setup
    uint8_t cmd = 0b10100000;
    sercom_i2c_start_generic(&i2c_g, &i2c_t, 0b1110110, &cmd, 1, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) sercom_i2c_service(&i2c_g);
    console_send_str(console, "0: 0x");
    utoa(data, str, 16);
    console_send_str(console, str);
    console_send_str(console, " (Factory data and setup)\n");
    sercom_i2c_clear_transaction(&i2c_g, i2c_t);
    
    // C1: Pressure Sensitivity
    //sercom_i2c_start_reg_read(&i2c_g, &i2c_t, 0b1110110, 0b10100100, (uint8_t*)&data, 2);
    cmd = 0b10100100;
    sercom_i2c_start_generic(&i2c_g, &i2c_t, 0b1110110, &cmd, 1, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) sercom_i2c_service(&i2c_g);
    console_send_str(console, "C1: ");
    utoa(data, str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Pressure sensitivity)\n");
    sercom_i2c_clear_transaction(&i2c_g, i2c_t);
    
    // C2: Pressure offset
    sercom_i2c_start_reg_read(&i2c_g, &i2c_t, 0b1110110, 0b10100010, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) sercom_i2c_service(&i2c_g);
    console_send_str(console, "C2: ");
    utoa(data, str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Pressure offset)\n");
    sercom_i2c_clear_transaction(&i2c_g, i2c_t);
    
    // C3: Temperature coefficient of pressure sensitivity
    sercom_i2c_start_reg_read(&i2c_g, &i2c_t, 0b1110110, 0b10100110, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) sercom_i2c_service(&i2c_g);
    console_send_str(console, "C3: ");
    utoa(data, str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Temperature coefficient of pressure sensitivity)\n");
     sercom_i2c_clear_transaction(&i2c_g, i2c_t);
    
    // C4: Temperature coefficient of pressure offset
    sercom_i2c_start_reg_read(&i2c_g, &i2c_t, 0b1110110, 0b10100001, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) sercom_i2c_service(&i2c_g);
    console_send_str(console, "C4: ");
    utoa(data, str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Temperature coefficient of pressure offset)\n");
    sercom_i2c_clear_transaction(&i2c_g, i2c_t);
    
    // C5: Reference temperature
    sercom_i2c_start_reg_read(&i2c_g, &i2c_t, 0b1110110, 0b10100101, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) sercom_i2c_service(&i2c_g);
    console_send_str(console, "C5: ");
    utoa(data, str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Reference temperature)\n");
    sercom_i2c_clear_transaction(&i2c_g, i2c_t);
    
    // C6: Temperature coefficient of the temperature
    sercom_i2c_start_reg_read(&i2c_g, &i2c_t, 0b1110110, 0b10100011, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) sercom_i2c_service(&i2c_g);
    console_send_str(console, "C6: ");
    utoa(data, str, 10);
    console_send_str(console, str);
    console_send_str(console, " (Temperature coefficient of the temperature)\n");
    sercom_i2c_clear_transaction(&i2c_g, i2c_t);
    
    // 7: Serial code and CRC
    cmd = 0b10100111;
    sercom_i2c_start_generic(&i2c_g, &i2c_t, 0b1110110, &cmd, 1, (uint8_t*)&data, 2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) sercom_i2c_service(&i2c_g);
    console_send_str(console, "7: 0x");
    utoa(data, str, 16);
    console_send_str(console, str);
    console_send_str(console, " (Serial code and CRC)\n");
    sercom_i2c_clear_transaction(&i2c_g, i2c_t);
}


#define DEBUG_IMU_WAI_NAME  "imu-wai"
#define DEBUG_IMU_WAI_HELP  "Read IMU Who Am I register."

static void debug_imu_wai (uint8_t argc, char **argv,
                           struct console_desc_t *console)
{
    uint8_t i2c_t;
    uint8_t data;
    char str[9];
    
    // Who Am I
    sercom_i2c_start_reg_read(&i2c_g, &i2c_t, 0b1101000, 0x75, &data, 2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) sercom_i2c_service(&i2c_g);
    console_send_str(console, "Who Am I: 0x");
    utoa(data, str, 16);
    console_send_str(console, str);
    console_send_str(console, " (Gyroscope and Accelerometer)\n");
    sercom_i2c_clear_transaction(&i2c_g, i2c_t);
}


const uint8_t debug_commands_num_funcs = 5;
const struct cli_func_desc_t debug_commands_funcs[] = {
    {.func = debug_version, .name = DEBUG_VERSION_NAME, .help_string = DEBUG_VERSION_HELP},
    {.func = debug_did, .name = DEBUG_DID_NAME, .help_string = DEBUG_DID_HELP},
    {.func = debug_i2c_scan, .name = DEBUG_I2C_SCAN_NAME, .help_string = DEBUG_I2C_SCAN_HELP},
    {.func = debug_alt_prom, .name = DEBUG_ALT_PROM_NAME, .help_string = DEBUG_ALT_PROM_HELP},
    {.func = debug_imu_wai, .name = DEBUG_IMU_WAI_NAME, .help_string = DEBUG_IMU_WAI_HELP}
};
