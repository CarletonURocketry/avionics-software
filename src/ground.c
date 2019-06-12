/**
 * @file ground.c
 * @desc Service to receive and relay telemetry packets
 * @author Samuel Dewan
 * @date 2019-06-11
 * Last Author:
 * Last Edited On:
 */

#include "ground.h"

#include "console.h"


static struct console_desc_t *ground_console_g;

static struct rn2483_desc_t *ground_radio_g;

static uint8_t ready_to_send_g;

static enum rn2483_operation_result ground_op_status_g;



static void ground_radio_recv_callback (struct rn2483_desc_t *inst,
                                       void *context, uint8_t *data,
                                       uint8_t length, int8_t snr)
{
    if (length && ready_to_send_g) {
        console_send_bytes(ground_console_g, data, length);
        console_send_bytes(ground_console_g, (uint8_t*)&snr, 1);
    }
    ground_op_status_g =  rn2483_receive(ground_radio_g, 0,
                                         ground_radio_recv_callback, NULL);
}

static void console_ready (struct console_desc_t *console, void *context)
{
    ready_to_send_g = 1;
}


void init_ground_service(struct console_desc_t *out_console,
                         struct rn2483_desc_t *radio)
{
    ground_console_g = out_console;
    ground_radio_g = radio;
    ready_to_send_g = 0;
    
    ground_op_status_g = rn2483_receive(ground_radio_g, 0,
                                        ground_radio_recv_callback, NULL);
    
    console_set_init_callback(ground_console_g, console_ready, NULL);
}

void ground_service (void)
{
    if (ground_op_status_g != RN2483_OP_SUCCESS) {
        ground_op_status_g = rn2483_receive(ground_radio_g, 0,
                                            ground_radio_recv_callback, NULL);
    }
}
