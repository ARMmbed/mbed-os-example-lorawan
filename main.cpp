/**
 * Copyright (c) 2017, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>

#include "lorawan/LoRaWANInterface.h"
#include "lorawan/system/lorawan_data_structures.h"
#include "events/EventQueue.h"

#include "trace_helper.h"
#include "lora_radio_helper.h"

#define TEST_MODE_DEACTIVE         0x00
#define TEST_MODE_ACTIVE           0x01
#define TEST_MODE_CONFIRMED        0x02
#define TEST_MODE_UNCONFIRMED      0x03
#define TEST_MODE_ECHO             0x04
#define TEST_MODE_LINK_CHECK_REQ   0x05
#define TEST_MODE_TRIG_JOIN_REQ    0x06
#define TEST_MODE_CONT_WAVE        0x07
#define TEST_MODE_ABORT_MAX_CNT    192 // Max. number of unanswered frames after which test mode is aborted
#define DL_FCNT_LENGTH             2 //bytes
#define COMPLIANCE_TEST_PORT       224
/*
 * Sets up an application dependent transmission timer in ms.
 * Used only when Duty Cycling is off for testing
 */
#define TX_TIMER                        5000

using namespace events;

typedef enum {
    TEST_MODE_IDLE=0,
    TEST_MODE_ACTIVATED,
    TEST_MODE_DEACTIVATED,
    TEST_MODE_APP_FUNC_ECHO_SERV,
    TEST_MODE_APP_FUNC_DL_CNT,
    TEST_MODE_CONFIRMED_MSGS,
    TEST_MODE_APP_OTAA
} test_suit_type;

static int current_test_state = TEST_MODE_IDLE;
static uint16_t downlink_counter = 0;
static uint16_t received_packet_length = 0;
static int tx_dispatcher_id = -1;

// Max payload size can be LORAMAC_PHY_MAXPAYLOAD.
// This example only communicates with much shorter messages (<30 bytes).
// If longer messages are used, these buffers must be changed accordingly.
uint8_t tx_buffer[30];
uint8_t rx_buffer[30];


/**
 * Maximum number of events for the event queue.
 * 10 is the safe number for the stack events, however, if application
 * also uses the queue for whatever purposes, this number should be increased.
 */
#define MAX_NUMBER_OF_EVENTS            10

/**
 * Maximum number of retries for CONFIRMED messages before giving up
 */
#define CONFIRMED_MSG_RETRY_COUNTER     3

/**
* This event queue is the global event queue for both the
* application and stack. To conserve memory, the stack is designed to run
* in the same thread as the application and the application is responsible for
* providing an event queue to the stack that will be used for ISR deferment as
* well as application information event queuing.
*/
static EventQueue ev_queue(MAX_NUMBER_OF_EVENTS * EVENTS_EVENT_SIZE);

/**
 * Event handler.
 *
 * This will be passed to the LoRaWAN stack to queue events for the
 * application which in turn drive the application.
 */
static void lora_event_handler(lorawan_event_t event);

/**
 * Constructing Mbed LoRaWANInterface and passing it down the radio object.
 */
static LoRaWANInterface lorawan(radio);

/**
 * Application specific callbacks
 */
static lorawan_app_callbacks_t callbacks;

static lorawan_connect_t conn;

static lorawan_status_t re_connect(bool OTAA);

const char *any_packet = "ANY_16_BYTE_LONG";
bool is_any_packet = true;
bool use_otaa = true;

/**
 * Entry point for application
 */
int main (void)
{
    if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
        printf("\r\n Duty Cycle must be off for Compliance Testing. Exiting \r\n");
        return EXIT_FAILURE;
    }

    // setup tracing
    setup_trace();

    // stores the status of a call to LoRaWAN protocol
    lorawan_status_t retcode;

    // Initialize LoRaWAN stack
    if (lorawan.initialize(&ev_queue) != LORAWAN_STATUS_OK) {
        printf("\r\n LoRa initialization failed! \r\n");
        return -1;
    }

    printf("\r\n Mbed LoRaWANStack initialized \r\n");

    // prepare application callbacks
    callbacks.events = mbed::callback(lora_event_handler);
    lorawan.add_app_callbacks(&callbacks);

    // Set number of retries in case of CONFIRMED messages
    if (lorawan.set_confirmed_msg_retries(CONFIRMED_MSG_RETRY_COUNTER)
                                          != LORAWAN_STATUS_OK) {
        printf("\r\n set_confirmed_msg_retries failed! \r\n\r\n");
        return -1;
    }

    printf("\r\n CONFIRMED message retries : %d \r\n",
           CONFIRMED_MSG_RETRY_COUNTER);

    // Enable adaptive data rate
    if (lorawan.enable_adaptive_datarate() != LORAWAN_STATUS_OK) {
        printf("\r\n enable_adaptive_datarate failed! \r\n");
        return -1;
    }

    printf("\r\n Adaptive data  rate (ADR) - Enabled \r\n");

    is_any_packet = true;
    retcode = lorawan.connect();

    if (retcode == LORAWAN_STATUS_OK ||
        retcode == LORAWAN_STATUS_CONNECT_IN_PROGRESS) {
    } else {
        printf("\r\n Connection error, code = %d \r\n", retcode);
        return -1;
    }

    printf("\r\n Connection - In Progress ...\r\n");

    // make your event queue dispatching events forever
    ev_queue.dispatch_forever();

    return 0;
}

static lorawan_status_t re_connect(bool OTAA)
{
    if (OTAA) {
        const static uint8_t dev_eui[] = MBED_CONF_LORA_DEVICE_EUI;
        const static uint8_t app_eui[] = MBED_CONF_LORA_APPLICATION_EUI;
        const static uint8_t app_key[] = MBED_CONF_LORA_APPLICATION_KEY;
        conn.connect_type = LORAWAN_CONNECTION_OTAA;
        conn.connection_u.otaa.app_eui = const_cast<uint8_t *>(app_eui);
        conn.connection_u.otaa.app_key = const_cast<uint8_t *>(app_key);
        conn.connection_u.otaa.dev_eui = const_cast<uint8_t *>(dev_eui);
        conn.connection_u.otaa.nb_trials = MBED_CONF_LORA_NB_TRIALS;
        return lorawan.connect(conn);
    }

    return lorawan.connect();
}

void fill_in_dl_cnt(uint8_t *buffer, int size, bool increment = false)
{
    uint16_t dl_cnt = downlink_counter;
    if (!buffer || size < DL_FCNT_LENGTH) {
        return;
    }

    if (increment) {
        dl_cnt += 1;
    }

    buffer[1] = dl_cnt & 0xFF;
    buffer[0] = (dl_cnt >> 8) & 0xFF;

    printf("Fill-in DL_CNT: %d%d\n", buffer[0], buffer[1]);
}

uint16_t test_app_func_echo()
{
    if (received_packet_length > sizeof(tx_buffer) / sizeof(uint8_t)) {
        printf("Abort: Response can't fit into buffer\n");
        return 0;
    }

    tx_buffer[0] = TEST_MODE_ECHO;
    uint16_t packet_length = 1;

    printf("Echo Response: %d", tx_buffer[0]);

    for (int i = 1; i < received_packet_length; i++) {
        tx_buffer[i] = (rx_buffer[i] + 1) % 256;
        printf("%d", tx_buffer[i]);
        packet_length++;
    }

    printf("\n");

    return packet_length;
}

uint16_t get_test_payload(int test_suit, uint8_t *buffer)
{
    uint16_t payload_size = DL_FCNT_LENGTH;

    switch (test_suit) {
        case TEST_MODE_IDLE:
            payload_size = sprintf((char *) buffer, any_packet);
            is_any_packet = true;
            break;
        case TEST_MODE_ACTIVATED:
        case TEST_MODE_CONFIRMED_MSGS:
            // After entering test mode, we need to send 2 bytes payload
            // containing downlink frame counter in big-endian format
            fill_in_dl_cnt(tx_buffer, DL_FCNT_LENGTH);
            break;
        case TEST_MODE_DEACTIVATED:
            // This will indicate send_message() routine to stop dispatching
            payload_size = 0;
            break;
        case TEST_MODE_APP_FUNC_ECHO_SERV:
            payload_size = test_app_func_echo();
            break;
        case TEST_MODE_APP_FUNC_DL_CNT:
            fill_in_dl_cnt(tx_buffer, DL_FCNT_LENGTH);
            break;
        case TEST_MODE_APP_OTAA:
            payload_size = 0;
            use_otaa = true;
            // reconnection is being handled in the event handler
            lorawan.disconnect();
            break;

        default:
            payload_size = 0;
            break;
    }

    return payload_size;
}

/**
 * Sends a message to the Network Server
 */
static void send_message()
{
    uint16_t packet_len;
    int16_t retcode;

    packet_len = get_test_payload(current_test_state, tx_buffer);

    if (packet_len == 0) {
        printf("\r\n Test Mode Deactivated .. Hang-on \r\n");
        return;
    }

    retcode = lorawan.send(is_any_packet ? MBED_CONF_LORA_APP_PORT : COMPLIANCE_TEST_PORT,
                           tx_buffer, packet_len,
                           current_test_state == TEST_MODE_CONFIRMED_MSGS ? MSG_CONFIRMED_FLAG :MSG_UNCONFIRMED_FLAG);

    if (retcode < 0) {
        retcode == LORAWAN_STATUS_WOULD_BLOCK ? printf("send - WOULD BLOCK\r\n")
                : printf("\r\n send() - Error code %d \r\n", retcode);
        return;
    }

    is_any_packet = false;
    current_test_state = TEST_MODE_ACTIVATED;

    printf("\r\n %d bytes scheduled for transmission \r\n", retcode);
    memset(tx_buffer, 0, sizeof(tx_buffer));
}

/**
 * Receive a message from the Network Server
 */
static void receive_message()
{
    int16_t retcode;
    retcode = lorawan.receive(COMPLIANCE_TEST_PORT, rx_buffer,
                              sizeof(rx_buffer),
                              MSG_CONFIRMED_FLAG|MSG_UNCONFIRMED_FLAG);

    if (retcode < 0) {
        printf("\r\n receive() - Error code %d \r\n", retcode);
        return;
    }

    printf(" Data:");

    for (uint8_t i = 0; i < retcode; i++) {
        printf("%x", rx_buffer[i]);
    }

    received_packet_length = retcode;

    printf("\r\n Data Length: %d\r\n", retcode);

    downlink_counter++;

    switch (rx_buffer[0]) {
        case TEST_MODE_DEACTIVE:
            downlink_counter = 0;
            current_test_state = TEST_MODE_DEACTIVATED;
            break;
        case TEST_MODE_ACTIVE:
            downlink_counter = 0;
            current_test_state = TEST_MODE_ACTIVATED;
            break;
        case TEST_MODE_ECHO:
            if (rx_buffer[1] == 0x01) {
                current_test_state = TEST_MODE_APP_FUNC_ECHO_SERV;
            }
            break;
        case TEST_MODE_UNCONFIRMED:
            current_test_state = TEST_MODE_APP_FUNC_DL_CNT;
            break;
        case TEST_MODE_CONFIRMED:
            current_test_state = TEST_MODE_CONFIRMED_MSGS;
            break;
        case TEST_MODE_TRIG_JOIN_REQ:
            current_test_state = TEST_MODE_APP_OTAA;
            break;
        default:
            break;
    }
}

/**
 * Event handler
 */
static void lora_event_handler(lorawan_event_t event)
{
    switch (event) {
        case CONNECTED:
            printf("\r\n Connection - Successful \r\n");
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message();
            } else {
                if (tx_dispatcher_id == -1) {
                    tx_dispatcher_id = ev_queue.call_every(TX_TIMER, send_message);
                }
            }

            break;
        case DISCONNECTED:
            ev_queue.cancel(tx_dispatcher_id);
            tx_dispatcher_id = -1;
            downlink_counter = 0;
            current_test_state = TEST_MODE_IDLE;
            printf("Reconnecting ...\n");
            re_connect(use_otaa);
            break;
        case TX_DONE:
            printf("\r\n Message Sent to Network Server \r\n");
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message();
            }
            break;
        case TX_TIMEOUT:
        case TX_ERROR:
        case TX_CRYPTO_ERROR:
        case TX_SCHEDULING_ERROR:
            printf("\r\n Transmission Error - EventCode = %d \r\n", event);
            // try again
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message();
            }
            break;
        case RX_DONE:
            printf("\r\n Received message from Network Server \r\n");
            receive_message();
            break;
        case RX_TIMEOUT:
        case RX_ERROR:
            printf("\r\n Error in reception - Code = %d \r\n", event);
            break;
        case JOIN_FAILURE:
            printf("\r\n OTAA Failed - Check Keys \r\n");
            break;
        case UPLINK_REQUIRED:
            printf("\r\n Uplink required by NS \r\n");
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message();
            }
            break;
        default:
            MBED_ASSERT("Unknown Event");
    }
}

// EOF
