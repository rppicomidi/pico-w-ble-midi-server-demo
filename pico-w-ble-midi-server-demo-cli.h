#pragma once
#include <stdint.h>
#include "embedded_cli.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef void (*process_send_cb_t)(uint8_t* byte_array, uint8_t nbytes);
EmbeddedCli* pico_w_ble_midi_server_demo_cli_init(process_send_cb_t process_send_cb);
void pico_w_ble_midi_server_demo_cli_task();
#ifdef __cplusplus
}
#endif
