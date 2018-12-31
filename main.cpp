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
#include "HeapBlockDevice.h"

// Application helpers
#include "DummySensor.h"
#include "trace_helper.h"
#include "lora_radio_helper.h"

#include "MulticastControlPackage.h"
#include "ClockSyncControlPackage.h"
#include "FragmentationControlPackage.h"

#if MBED_MEM_TRACING_ENABLED
#include "mbed_mem_trace.h"
#endif

using namespace events;

// Max payload size can be LORAMAC_PHY_MAXPAYLOAD.
// This example only communicates with much shorter messages (<30 bytes).
// If longer messages are used, these buffers must be changed accordingly.
uint8_t tx_buffer[100];
uint8_t rx_buffer[100];

/*
 * Sets up an application dependent transmission timer in ms. Used only when Duty Cycling is off for testing
 */
#define TX_TIMER                        10000

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
 * Dummy pin for dummy sensor
 */
#define PC_9                            0

/**
 * Dummy sensor class object
 */
DS1820  ds1820(PC_9);

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

static frag_bd_opts_t *bd_cb_handler(uint8_t frag_index, uint32_t desc);

/**
 * Constructing Mbed LoRaWANInterface and passing it down the radio object.
 */
static LoRaWANInterface lorawan(radio);
static ClockSyncControlPackage clk_sync_plugin;
static MulticastControlPackage mcast_plugin;
static FragmentationControlPackage frag_plugin;
uint8_t gen_app_key[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                           0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x1F};

mcast_controller_cbs_t mcast_cbs;
clk_sync_response_t *sync_resp = NULL;
mcast_ctrl_response_t *mcast_resp = NULL;
frag_ctrl_response_t *frag_resp = NULL;

/**
 * Application specific callbacks
 */
static lorawan_app_callbacks_t callbacks;
int app_mssage_dispatcher_id = -1;
bool clock_sync_request_ongoing = false;
bool clock_sync_magic_test_ongoing = false;
bool clock_sync_test_ongoing = false;
bool class_c_mode = false;

static int start_message_dispatcher(void);
static int stop_message_dispatcher(int id);
static void send_message();
static void test_state_machine(uint8_t new_state);
typedef enum {
    IDLE=0,
    CLOCK_SYNC_REQ,
    CLOCK_SYNC_MAGIC_TEST,
    SENDING_CLOCK_SYNC_MAGIC_TEST_RESP,
    MCAST_MAGIC_TEST,
    SENDING_MCAST_MAGIC_TEST_RESP,
    FRAG_MAGIC_TEST,
    SENDING_FRAG_MAGIC_TEST_RESP,
    NORMAL_TRAFFIC
} state;

/**
 * Helper function to print memory usage statistics
 */
#if MBED_HEAP_STATS_ENABLED
void printHeapStats(const char *prefix = "") {
    mbed_stats_heap_t heap_stats;
    mbed_stats_heap_get(&heap_stats);

    printf("\n%s Heap stats: %lu / %lu (max=%lu)\n", prefix, heap_stats.current_size, heap_stats.reserved_size, heap_stats.max_size);
}
#else
void printHeapStats(const char *prefix = "")
{
  (void) prefix;
}
#endif

static uint8_t previous_state = IDLE;
static BlockDevice *mem;
static FragBDWrapper *flash;
static frag_bd_opts_t bd_opts;

static frag_bd_opts_t *bd_cb_handler(uint8_t frag_index, uint32_t desc)
{
    printHeapStats("BD_CALLBACK");
    printf("Creating BD for session %d with desc: %lu", frag_index, desc);
    bd_opts.redundancy_max = 40;
    bd_opts.offset = 0;
    bd_opts.fasm = new FragAssembler();
    mem = new HeapBlockDevice(2 * 512, 512);
    flash = new FragBDWrapper(mem);
    bd_opts.bd = flash;
    printHeapStats("BD_CALLBACK");
    return &bd_opts;
}

static void switch_to_class_A_helper()
{
    printf("\r\n Class C session life time expired \r\n");
    lorawan.enable_adaptive_datarate();
    printf("\r\n ADR enabled \r\n");
    lorawan.restore_rx2_frequency();
    printf("\r\n Restoring RX2 frequency to default \r\n");
    printf("\r\n Restoring class A session \r\n");
    lorawan.set_device_class(CLASS_A);

    class_c_mode = false;

    test_state_machine(FRAG_MAGIC_TEST);

/*    if (app_mssage_dispatcher_id == -1) {
        app_mssage_dispatcher_id = start_message_dispatcher();
    }*/

}

static void switch_to_class_C_helper(uint8_t life_time,
                                     uint8_t dr,
                                     uint32_t dl_freq)
{
    lorawan.cancel_sending();
    lorawan.disable_adaptive_datarate();
    if (lorawan.set_datarate(dr) != LORAWAN_STATUS_OK) {
        printf("\r\n Failed to set up data rate %d for Class C session \r\n", dr);
    } else {
        printf("\r\n Class C session data rate: %d \r\n", dr);
    }

    if (lorawan.set_rx2_frequency(dl_freq) != LORAWAN_STATUS_OK) {
        printf("\r\n Failed to set up frequency %lu for Class C session \r\n", dl_freq);
    } else {
        printf("\r\n Class C session frequency: %lu \r\n", dl_freq);
    }

    lorawan.set_device_class(CLASS_C);

    ev_queue.call_in(life_time * 1000, switch_to_class_A_helper);
}

static void switch_class(uint8_t device_class,
                         uint32_t time_to_switch,
                         uint8_t life_time,
                         uint8_t dr,
                         uint32_t dl_freq)
{
    if (device_class != CLASS_C) {
        return;
    }

    class_c_mode = true;
    app_mssage_dispatcher_id = stop_message_dispatcher(app_mssage_dispatcher_id);
    printf("\r\n Switching to class C in %lu seconds \r\n", time_to_switch);
    printf("\r\n Class C session lifetime %d seconds \r\n", life_time);
    ev_queue.call_in(time_to_switch * 1000, switch_to_class_C_helper, life_time, dr, dl_freq);

}

static lorawan_status_t check_params_validity(uint8_t dr, uint32_t dl_freq)
{
    return lorawan.verify_multicast_freq_and_dr(dl_freq, dr);
}
static int start_message_dispatcher(void) {
    // send a message every 10 secs
    return ev_queue.call_every(TX_TIMER, send_message);
}

static int stop_message_dispatcher(int id) {
    ev_queue.cancel(id);
    return -1;
}

static const uint32_t magic_sequence_clk_sync = 0x01010101;
static const uint32_t magic_sequence_mcast = 0x02020202;
static const uint32_t magic_sequence_frag = 0x03030303;

static lorawan_time_t get_gps_time()
{
    return lorawan.get_current_gps_time();

}

static void set_gps_time(lorawan_time_t time)
{
    lorawan.set_current_gps_time(time);
}

/**
 * Entry point for application
 */
int main (void)
{
#if MBED_MEM_TRACING_ENABLED
    mbed_mem_trace_set_callback(mbed_mem_trace_default_callback);
#endif

    // setup tracing
    setup_trace();

    //activate clock sync control plug-in
    clk_sync_plugin.activate_clock_sync_package(mbed::callback(get_gps_time),
                                                mbed::callback(set_gps_time));

    // activate multicast plugin
    mcast_plugin.activate_multicast_control_package(gen_app_key, 16);

    //set callbacks for multicast control package
    mcast_cbs.switch_class = mbed::callback(switch_class);
    mcast_cbs.check_params_validity = mbed::callback(check_params_validity);
    mcast_cbs.get_gps_time = mbed::callback(get_gps_time);

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

    // Enable adaptive data rate
    if (lorawan.enable_adaptive_datarate() != LORAWAN_STATUS_OK) {
        printf("\r\n enable_adaptive_datarate failed! \r\n");
        return -1;
    }

    printf("\r\n Adaptive data  rate (ADR) - Enabled \r\n");

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

static void test_state_machine(uint8_t new_state)
{
    if (new_state == IDLE || new_state == previous_state) {
        return;
    }

    switch (new_state) {
        case CLOCK_SYNC_REQ: {
            sync_resp = clk_sync_plugin.request_clock_sync(true);

            if (sync_resp) {
                clock_sync_request_ongoing = true;
                lorawan.send(CLOCK_SYNC_PORT, sync_resp->data, sync_resp->size,
                MSG_UNCONFIRMED_FLAG);
            }

            break;
        }

        case CLOCK_SYNC_MAGIC_TEST: {
            write_four_bytes(magic_sequence_clk_sync, tx_buffer);
            int16_t ret = lorawan.send(
                    MBED_CONF_LORA_APP_PORT,
                    tx_buffer,
                    4,
                    MSG_UNCONFIRMED_FLAG);
            if (ret == 4) {
                printf("\r\n Sent Magic Sequence for clock sync test \r\n");
                for (int i = 0; i < 4; i++) {
                    printf("%x", tx_buffer[i]);
                }
                printf("\r\n");
            }

            sync_resp = NULL;

            break;
        }

        case SENDING_CLOCK_SYNC_MAGIC_TEST_RESP: {
            if (sync_resp) {
                printf("\r\n sync_resp->data: ");
                for (int i = 0; i < sync_resp->size; i++) {
                      printf("%x", sync_resp->data[i]);
                }
                printf("\r\n sync_resp->size: %d \r\n", sync_resp->size);
                lorawan.send(CLOCK_SYNC_PORT, sync_resp->data, sync_resp->size,
                             MSG_UNCONFIRMED_FLAG);
            }
            break;
        }

        case MCAST_MAGIC_TEST: {
            write_four_bytes(magic_sequence_mcast, tx_buffer);
            int16_t ret = lorawan.send(MBED_CONF_LORA_APP_PORT, tx_buffer, 4,
                                       MSG_UNCONFIRMED_FLAG);
            if (ret == 4) {
                printf("\r\n Sent Magic Sequence for Multicast test \r\n");
                for (int i = 0; i < 4; i++) {
                    printf("%x", tx_buffer[i]);
                }
                printf("\r\n");
            }

            sync_resp = NULL;
            mcast_resp = NULL;

            break;
        }

        case SENDING_MCAST_MAGIC_TEST_RESP: {
            if (mcast_resp) {
                printf("\r\n mcast_resp->data: ");
                for (int i = 0; i < mcast_resp->size; i++) {
                    printf("%x", mcast_resp->data[i]);
                }
                printf("\r\n mcast_resp->size: %d \r\n", mcast_resp->size);

                lorawan.send(MULTICAST_CONTROL_PORT,
                             mcast_resp->data,
                             mcast_resp->size,
                             MSG_UNCONFIRMED_FLAG);
            }
            mcast_resp = NULL;
            break;
        }

        case FRAG_MAGIC_TEST: {
            //skip temporarily
            return;
            write_four_bytes(magic_sequence_frag, tx_buffer);
            int16_t ret = lorawan.send(MBED_CONF_LORA_APP_PORT, tx_buffer, 4,
                                       MSG_UNCONFIRMED_FLAG);
            if (ret == 4) {
                printf("\r\n Sent Magic Sequence for Frag test \r\n");
                for (int i = 0; i < 4; i++) {
                    printf("%x", tx_buffer[i]);
                }
                printf("\r\n");
            }

            frag_resp = NULL;
            break;
        }

        case SENDING_FRAG_MAGIC_TEST_RESP: {
            //skip temporarily
            return;
            if (frag_resp && frag_resp->type == FRAG_CMD_RESP) {

                printf("\r\n frag_resp->data: ");
                for (int i = 0; i < frag_resp->cmd_ans.size; i++) {
                    printf("%x", frag_resp->cmd_ans.data[i]);
                }
                printf("\r\n frag_resp->cmd_ans.size: %d \r\n",
                       frag_resp->cmd_ans.size);

                lorawan.send(FRAGMENTATION_CONTROL_PORT, frag_resp->cmd_ans.data,
                             frag_resp->cmd_ans.size,
                             MSG_UNCONFIRMED_FLAG);

            }

            frag_resp = NULL;
            break;
        }

        default:
            new_state = IDLE;
            break;
    }

    previous_state = new_state;

}

/**
 * Sends a message to the Network Server
 */
static void send_message()
{
    uint16_t packet_len;
    packet_len = sprintf((char*) tx_buffer, "PING");
    lorawan.send(MBED_CONF_LORA_APP_PORT, tx_buffer, packet_len,
    MSG_UNCONFIRMED_FLAG);

    memset(tx_buffer, 0, sizeof(tx_buffer));
}

/**
 * Receive a message from the Network Server
 */
static void receive_message()
{
    int16_t retcode;
    int flags;
    uint8_t port;
    retcode = lorawan.receive(rx_buffer,
                              sizeof(rx_buffer), port, flags);

    if (retcode < 0) {
        printf("\r\n receive() - Error code %d \r\n", retcode);
        return;
    }

    printf(" Data:");

    for (uint8_t i = 0; i < retcode; i++) {
        printf("%x", rx_buffer[i]);
    }

    printf("\r\n Data Length: %d\r\n", retcode);

    if (port == CLOCK_SYNC_PORT) {
        sync_resp = clk_sync_plugin.parse(rx_buffer, retcode);
    } else if (port == MULTICAST_CONTROL_PORT) {
        mcast_resp = mcast_plugin.parse(rx_buffer, retcode,
                                        lorawan.get_multicast_addr_register(),
                                        &mcast_cbs);
        lorawan.set_system_time_utc(37);
    } else if (port == FRAGMENTATION_CONTROL_PORT) {
        lorawan_rx_metadata md;
        lorawan.get_rx_metadata(md);
        // Descriptor 'FOTA' = 0x464f5441
        frag_resp = frag_plugin.parse(rx_buffer, retcode, flags, md.dev_addr,
                                      mbed::callback(bd_cb_handler),
                                      lorawan.get_multicast_addr_register(),
                                      0x464f5441);

        if (frag_resp->type == FRAG_SESSION_STATUS && frag_resp->status == FRAG_SESSION_COMPLETE) {
            char buffer[1024];

            if (flash->read(buffer, 0, 512) == BD_ERROR_OK) {
                printf("%s", buffer);
            }

        }

        printHeapStats("FRAG_RESP");

        test_state_machine(SENDING_FRAG_MAGIC_TEST_RESP);
    }

    memset(rx_buffer, 0, sizeof(rx_buffer));
}

/**
 * Event handler
 */
static void lora_event_handler(lorawan_event_t event)
{
    switch (event) {
        case CONNECTED:
            printf("\r\n Connection - Successful \r\n");
            printHeapStats("CONNECTED");
            //test_state_machine(FRAG_MAGIC_TEST);
            test_state_machine(CLOCK_SYNC_REQ);
            break;
        case DISCONNECTED:
            ev_queue.break_dispatch();
            printf("\r\n Disconnected Successfully \r\n");
            break;
        case TX_DONE:
            printf("\r\n Message Sent to Network Server \r\n");
            if (app_mssage_dispatcher_id == -1 && class_c_mode == false) {
                app_mssage_dispatcher_id = start_message_dispatcher();
            }
            break;
        case TX_TIMEOUT:
        case TX_ERROR:
        case TX_CRYPTO_ERROR:
        case TX_SCHEDULING_ERROR:
            printf("\r\n Transmission Error - EventCode = %d \r\n", event);
            break;
        case RX_DONE:
            printf("\r\n Received message from Network Server \r\n");
            receive_message();
            if (app_mssage_dispatcher_id > 0) {
                app_mssage_dispatcher_id = stop_message_dispatcher(app_mssage_dispatcher_id);
            }

            if (previous_state != NORMAL_TRAFFIC || previous_state != SENDING_MCAST_MAGIC_TEST_RESP) {
                test_state_machine(previous_state + 1);
            }

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
            break;
        default:
            MBED_ASSERT("Unknown Event");
    }
}

// EOF
