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

#ifndef APP_LORA_RADIO_HELPER_H_
#define APP_LORA_RADIO_HELPER_H_

#include "lorawan/LoRaRadio.h"

#define SX1272   0xFF
#define SX1276   0xEE
#define SX126X   0xDD

#if (MBED_CONF_APP_LORA_RADIO == SX1272)
#include "SX1272_LoRaRadio.h"
SX1272_LoRaRadio radio(MBED_CONF_APP_LORA_SPI_MOSI,
                       MBED_CONF_APP_LORA_SPI_MISO,
                       MBED_CONF_APP_LORA_SPI_SCLK,
                       MBED_CONF_APP_LORA_CS,
                       MBED_CONF_APP_LORA_RESET,
                       MBED_CONF_APP_LORA_DIO0,
                       MBED_CONF_APP_LORA_DIO1,
                       MBED_CONF_APP_LORA_DIO2,
                       MBED_CONF_APP_LORA_DIO3,
                       MBED_CONF_APP_LORA_DIO4,
                       MBED_CONF_APP_LORA_DIO5,
                       MBED_CONF_APP_LORA_RF_SWITCH_CTL1,
                       MBED_CONF_APP_LORA_RF_SWITCH_CTL2,
                       MBED_CONF_APP_LORA_TXCTL,
                       MBED_CONF_APP_LORA_RXCTL,
                       MBED_CONF_APP_LORA_ANT_SWITCH,
                       MBED_CONF_APP_LORA_PWR_AMP_CTL,
                       MBED_CONF_APP_LORA_TCXO);

#elif (MBED_CONF_APP_LORA_RADIO == SX1276)
#include "SX1276_LoRaRadio.h"
SX1276_LoRaRadio radio(MBED_CONF_APP_LORA_SPI_MOSI,
                       MBED_CONF_APP_LORA_SPI_MISO,
                       MBED_CONF_APP_LORA_SPI_SCLK,
                       MBED_CONF_APP_LORA_CS,
                       MBED_CONF_APP_LORA_RESET,
                       MBED_CONF_APP_LORA_DIO0,
                       MBED_CONF_APP_LORA_DIO1,
                       MBED_CONF_APP_LORA_DIO2,
                       MBED_CONF_APP_LORA_DIO3,
                       MBED_CONF_APP_LORA_DIO4,
                       MBED_CONF_APP_LORA_DIO5,
                       MBED_CONF_APP_LORA_RF_SWITCH_CTL1,
                       MBED_CONF_APP_LORA_RF_SWITCH_CTL2,
                       MBED_CONF_APP_LORA_TXCTL,
                       MBED_CONF_APP_LORA_RXCTL,
                       MBED_CONF_APP_LORA_ANT_SWITCH,
                       MBED_CONF_APP_LORA_PWR_AMP_CTL,
                       MBED_CONF_APP_LORA_TCXO);

#elif (MBED_CONF_APP_LORA_RADIO == SX126X)
#include "SX126X_LoRaRadio.h"
SX126X_LoRaRadio radio(MBED_CONF_APP_LORA_SPI_MOSI,
                       MBED_CONF_APP_LORA_SPI_MISO,
                       MBED_CONF_APP_LORA_SPI_SCLK,
                       MBED_CONF_APP_LORA_CS,
                       MBED_CONF_APP_LORA_RESET,
                       MBED_CONF_APP_LORA_DIO1,
                       MBED_CONF_APP_LORA_BUSY,
                       MBED_CONF_APP_LORA_FREQ_SEL,
                       MBED_CONF_APP_LORA_DEV_SEL,
                       MBED_CONF_APP_LORA_XTAL_SEL,
                       MBED_CONF_APP_LORA_ANT_SWITCH);

#else
#error "Unknown LoRa radio specified (SX126X, SX1272, SX1276 are valid)"
#endif

#endif /* APP_LORA_RADIO_HELPER_H_ */
