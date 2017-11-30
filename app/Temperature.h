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

#ifndef APP_TEMPERATURE_H_
#define APP_TEMPERATURE_H_

#include "LoRaWANInterface.h"
#include "mbed.h"
#include "radio/SX1272_LoRaRadio.h"

class Temperature {
public:
    void test_otaa();
    nsapi_error_t program1();
    static void lorawan_state_machine_handler(device_states_t *device_new_state);

private:
    lora_events_t _callback;

    static void lorawan_rx_message_handler(int8_t *session_id);
};
#endif /* APP_TEMPERATURE_H_ */
