#include <unittest.h>

/*
 *  Stubs for symbols used by rn2483_service that can be used by tests for
 *  functions other than rn2483_service.
 *
 *  This file is ment to be included into other tests. The tests that it is
 *  included into must have the global variable "static struct rn2483_desc_t
 *  radio_descriptor;".
 */

static int sercom_uart_has_line_call_count;
static uint8_t sercom_uart_has_line_retval;

uint8_t sercom_uart_has_line (struct sercom_uart_desc_t *uart)
{
    sercom_uart_has_line_call_count++;
    return sercom_uart_has_line_retval;
}

static enum rn2483_state executed_state;

#define STATE_STUB(name, state) static int name (struct rn2483_desc_t *inst) \
{\
    executed_state = state;\
    return 0;\
}

STATE_STUB(state_reset_stub, RN2483_RESET);
STATE_STUB(state_write_wdt_stub, RN2483_WRITE_WDT);
STATE_STUB(state_pause_mac_stub, RN2483_PAUSE_MAC);
STATE_STUB(state_write_mode_stub, RN2483_WRITE_MODE);
STATE_STUB(state_write_freq_stub, RN2483_WRITE_FREQ);
STATE_STUB(state_update_freq_stub, RN2483_UPDATE_FREQ);
STATE_STUB(state_write_pwr_stub, RN2483_WRITE_PWR);
STATE_STUB(state_write_sf_stub, RN2483_WRITE_SF);
STATE_STUB(state_write_crc_stub, RN2483_WRITE_CRC);
STATE_STUB(state_write_iqi_stub, RN2483_WRITE_IQI);
STATE_STUB(state_write_cr_stub, RN2483_WRITE_CR);
STATE_STUB(state_write_sync_stub, RN2483_WRITE_SYNC);
STATE_STUB(state_write_bw_stub, RN2483_WRITE_BW);
STATE_STUB(state_write_prlen_stub, RN2483_WRITE_PRLEN);
STATE_STUB(state_idle_stub, RN2483_IDLE);
STATE_STUB(state_send_stub, RN2483_SEND);
STATE_STUB(state_send_wait_stub, RN2483_SEND_WAIT);
STATE_STUB(state_receive_stub, RN2483_RECEIVE);
STATE_STUB(state_receive_wait_stub, RN2483_RECEIVE_ABORT);
STATE_STUB(state_rx_ok_wait_stub, RN2483_RX_OK_WAIT);
STATE_STUB(state_rx_data_wait_stub, RN2483_RX_DATA_WAIT);
STATE_STUB(state_get_snr_stub, RN2483_GET_SNR);
STATE_STUB(state_get_rssi_stub, RN2483_GET_RSSI);
STATE_STUB(state_rxstop_stub, RN2483_RXSTOP);
STATE_STUB(state_rxstop_received_stub, RN2483_RXSTOP_RECEIVED);
STATE_STUB(state_rxstop_get_error_stub, RN2483_RXSTOP_GET_ERROR);
STATE_STUB(state_set_pin_mode_stub, RN2483_SET_PIN_MODE);
STATE_STUB(state_set_pindig_stub, RN2483_SET_PINDIG);
STATE_STUB(state_get_pin_value_stub, RN2483_GET_PIN_VALUE);
STATE_STUB(state_failed_stub, RN2483_FAILED);

const rn2483_state_handler_t rn2483_state_handlers[] = {
    state_reset_stub,
    state_write_wdt_stub,
    state_pause_mac_stub,
    state_write_mode_stub,
    state_write_freq_stub,
    state_update_freq_stub,
    state_write_pwr_stub,
    state_write_sf_stub,
    state_write_crc_stub,
    state_write_iqi_stub,
    state_write_cr_stub,
    state_write_sync_stub,
    state_write_bw_stub,
    state_write_prlen_stub,
    state_idle_stub,
    state_send_stub,
    state_send_wait_stub,
    state_receive_stub,
    state_receive_wait_stub,
    state_rx_ok_wait_stub,
    state_rx_data_wait_stub,
    state_get_snr_stub,
    state_get_rssi_stub,
    state_rxstop_stub,
    state_rxstop_received_stub,
    state_rxstop_get_error_stub,
    state_set_pin_mode_stub,
    state_set_pindig_stub,
    state_get_pin_value_stub,
    state_failed_stub
};
