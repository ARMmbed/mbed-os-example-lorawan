/**
 * Copyright (c) 2017 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Temperature.h"

/**
 *
 * For the test_otaa() -method
 * May be removed when not needed anymore..
 */
lorawan_connect_t my_connect_params;
lorawan_session_t session;

/**
 *
 * Using radio chip SX1272
 */
static SX1272_LoRaRadio Radio(LORA_MOSI, LORA_MISO, LORA_SCK, LORA_NSS, LORA_RESET,
                              LORA_DIO0, LORA_DIO1, LORA_DIO2, LORA_DIO3, LORA_DIO4,
                              LORA_DIO5, NC, NC, LORA_TXCTL, LORA_RXCTL, NC, NC);

LoRaWANInterface lorawan(&Radio);


void Temperature::test_otaa()
{
    /*
     * Just some random values for the test case
     */
    uint8_t my_dev_eui[] = {0x01, 0x45, 0xB8, 0xDF, 0x01, 0x45, 0xB8, 0xDF};
    uint8_t my_app_eui[] = {0x45, 0xB8, 0xB8, 0xDF, 0x45, 0xB8, 0xB8, 0xDF};
    uint8_t my_app_key[] = {0x45, 0x45, 0xB8, 0xB8, 0xB8, 0xDF, 0x01, 0x45, 0x45, 0xB8, 0xB8, 0xB8, 0xDF, 0x01, 0x45, 0x45};

    /*
     * Set parameters to my_connect_params struct
     */
    my_connect_params.connect_type = LORAWAN_CONNECTION_OTAA;
    my_connect_params.connect.connect_otaa.dev_eui = my_dev_eui;
    my_connect_params.connect.connect_otaa.app_eui = my_app_eui;
    my_connect_params.connect.connect_otaa.app_key = my_app_key;
}

nsapi_error_t Temperature::program1()
{
    session.session_id = 1;

    //test_otaa();
    lorawan.connect();

    //lorawan->connect(&my_connect_params);

    return NSAPI_ERROR_OK;
}

void Temperature::lorawan_rx_message_handler(int8_t *session_id)
{
    lora_mac_rx_message_t rx_message;
    lorawan_session_t session;

    //TODO: Add needed rx-message handling by type.
    session.session_id = *session_id;
    lorawan.receive(&session, &rx_message);

    if (rx_message.type == LORAMAC_RX_MLME_CONFIRM) {

    } else if (rx_message.type == LORAMAC_RX_MCPS_CONFIRM) {

    } else {
        switch (rx_message.rx_message.mcps_indication.port) {
            case 1:
                /* no break */
                /* Fall through */
            case 2:
                if (rx_message.rx_message.mcps_indication.buffer_size == 1) {
                }
                break;
            default:
                break;
        }
    }
}

void Temperature::lorawan_state_machine_handler(device_states_t *device_new_state)
{
    lora_mac_tx_message_t message;
    uint8_t AppData[64];

    switch (*device_new_state) {
        case DEVICE_STATE_INIT:
            break;
        case DEVICE_STATE_JOIN:
            break;
        case DEVICE_STATE_SEND:
            break;
        case DEVICE_STATE_CYCLE:
            message.type = LORAMAC_TX_CONFIRMED_DATA;
            AppData[0] = 'H';
            AppData[1] = 'e';
            AppData[2] = 'l';
            AppData[3] = 'l';
            AppData[4] = 'o';
            AppData[5] = ' ';
            AppData[6] = 'W';
            AppData[7] = 'o';
            AppData[8] = 'r';
            AppData[9] = 'l';
            AppData[10] = 'd';
            AppData[11] = '!';
            message.message_u.confirmed.f_buffer = AppData;
            message.message_u.confirmed.f_buffer_size = 12;
            message.message_u.confirmed.f_port = LORAWAN_APP_PORT;
            message.message_u.confirmed.nb_trials = 3;

            lorawan.send(&session, &message);
            break;

        default:
            break;
    }
}
