/**
 * MIT License
 *
 * Copyright (c) 2023 rppicomidi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
#include <stdio.h>
#include "midi_service_stream_handler.h"
#include "btstack.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/btstack_cyw43.h"
#include "pico-w-ble-midi-server-demo.h"
#include "pico-w-ble-midi-server-demo-cli.h"
// This is Bluetooth LE only
#define APP_AD_FLAGS 0x06
const uint8_t adv_data[] = {
    // Flags general discoverable
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, APP_AD_FLAGS,
    // Service class list
    0x11, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_128_BIT_SERVICE_CLASS_UUIDS, 0x00, 0xc7, 0xc4, 0x4e, 0xe3, 0x6c, 0x51, 0xa7, 0x33, 0x4b, 0xe8, 0xed, 0x5a, 0x0e, 0xb8, 0x03,
};
const uint8_t adv_data_len = sizeof(adv_data);

const uint8_t scan_resp_data[] = {
    // Name
    0x0E, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'B', 'L', 'E', '-', 'M', 'I', 'D', 'I', ' ', 'D', 'e', 'm', 'o',
};
const uint8_t scan_resp_data_len = sizeof(scan_resp_data);

static hci_con_handle_t con_handle = HCI_CON_HANDLE_INVALID;

void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(size);
    UNUSED(channel);
    bd_addr_t local_addr;
    uint8_t event_type;
    switch(packet_type) {
        case HCI_EVENT_PACKET:
            event_type = hci_event_packet_get_type(packet);
            switch(event_type){
                case BTSTACK_EVENT_STATE:
                    if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) return;
                    gap_local_bd_addr(local_addr);
                    printf("BTstack up and running on %s.\n", bd_addr_to_str(local_addr));

                    // setup advertisements
                    uint16_t adv_int_min = 800;
                    uint16_t adv_int_max = 800;
                    uint8_t adv_type = 0;
                    bd_addr_t null_addr;
                    memset(null_addr, 0, 6);
                    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
                    assert(adv_data_len <= 31); // ble limitation
                    gap_advertisements_set_data(adv_data_len, (uint8_t*) adv_data);
                    assert(scan_resp_data_len <= 31); // ble limitation
                    gap_scan_response_set_data(scan_resp_data_len, (uint8_t*) scan_resp_data);
                    gap_advertisements_enable(1);

                    break;
                case HCI_EVENT_DISCONNECTION_COMPLETE:
                    printf("Disconnected\r\n");
                    break;
                case HCI_EVENT_GATTSERVICE_META:
                    switch(hci_event_gattservice_meta_get_subevent_code(packet)) {
                        case GATTSERVICE_SUBEVENT_SPP_SERVICE_CONNECTED:
                            con_handle = gattservice_subevent_spp_service_connected_get_con_handle(packet);
                            break;
                        case GATTSERVICE_SUBEVENT_SPP_SERVICE_DISCONNECTED:
                            printf("demo: I got GATTSERVICE_SUBEVENT_SPP_SERVICE_DISCONNECTED event\r\n");
                            con_handle = HCI_CON_HANDLE_INVALID;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

static void process_send_command(uint8_t* byte_array, uint8_t nbytes)
{
    if (con_handle != HCI_CON_HANDLE_INVALID) {
        printf("Sending MIDI bytes: ");
        printf_hexdump(byte_array, nbytes);
        midi_service_stream_write(con_handle, nbytes, byte_array);
    }
    else {
        printf("Not connected yet\r\n");
    }
}

// TODO static btstack_packet_callback_registration_t sm_event_callback_registration;
int main()
{
    stdio_init_all();
    // initialize CYW43 driver architecture (will enable BT if/because CYW43_ENABLE_BLUETOOTH == 1)
    if (cyw43_arch_init()) {
        printf("failed to initialise cyw43_arch\n");
        return -1;
    }
    l2cap_init();

    // setup SM: Just works
    sm_init();

    att_server_init(profile_data, NULL, NULL);
    //sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    //sm_set_authentication_requirements(SM_AUTHREQ_SECURE_CONNECTION | SM_AUTHREQ_BONDING);
    // register for SM events
    //sm_event_callback_registration.callback = &packet_handler;
    //sm_add_event_handler(&sm_event_callback_registration);
    midi_service_stream_init(packet_handler);

    // turn on bluetooth
    hci_power_control(HCI_POWER_ON);
    pico_w_ble_midi_server_demo_cli_init(process_send_command);
    for(;;) {
        //sleep_ms(1000); // TODO process incoming and outgoing MIDI packets from USB, etc.
        if (con_handle != HCI_CON_HANDLE_INVALID) {
            uint16_t timestamp;
            uint8_t mes[3];
            uint8_t nread = midi_service_stream_read(con_handle, sizeof(mes), mes, &timestamp);
            if (nread != 0) {
                printf("ts:%u  MIDI:", timestamp);
                printf_hexdump(mes, nread);
            }
        }
        pico_w_ble_midi_server_demo_cli_task();
    }
    return 0;
}