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

#include "SX1272_LoRaRadio.h"
#include "SX1276_LoRaRadio.h"

/**
 * Constructing a LoRaRadio object for Semtech SX1272 radio
 * available on MultiTech mDot platform
 * https://os.mbed.com/platforms/MTS-mDot-F411/
 */
#if TARGET_MTS_MDOT_F411RE
static SX1272_LoRaRadio radio(LORA_MOSI, LORA_MISO, LORA_SCK, LORA_NSS, LORA_RESET,
                              LORA_DIO0, LORA_DIO1, LORA_DIO2, LORA_DIO3, LORA_DIO4, LORA_DIO5,
                              NC, NC, LORA_TXCTL, LORA_RXCTL, NC, NC);
#endif //TARGET_MTS_MDOT_F411RE

/**
 * Constructing a LoRaRadio object for Semtech SX1272 radio
 * available on MultiTech xDot platform
 * https://os.mbed.com/platforms/MTS-xDot-L151CC/
 */
#if defined(TARGET_XDOT_L151CC) || defined(TARGET_MTB_MTS_XDOT)
static SX1272_LoRaRadio radio(LORA_MOSI, LORA_MISO, LORA_SCK, LORA_NSS, LORA_RESET,
                              LORA_DIO0, LORA_DIO1, LORA_DIO2, LORA_DIO3, LORA_DIO4, NC,
                              NC, NC, NC, NC, NC, NC);
#endif

/**
 * Constructing a LoRaRadio object for Semtech 1276 radio available on
 * Mbed LoRa radio shield which can be mounted on any board supporting
 * Arduino form factor.
 * https://os.mbed.com/components/SX1276MB1xAS/
 */
#if TARGET_K64F
#define LORA_SPI_MOSI     D11
#define LORA_SPI_MISO     D12
#define LORA_SPI_SCLK     D13
#define LORA_CS           D10
#define LORA_RESET        A0
#define LORA_DIO0         D2
#define LORA_DIO1         D3
#define LORA_DIO2         D4
#define LORA_DIO3         D5
#define LORA_DIO4         D8
#define LORA_DIO5         D9
#define LORA_ANT_SWITCH   A4

static SX1276_LoRaRadio radio(LORA_SPI_MOSI, LORA_SPI_MISO, LORA_SPI_SCLK, LORA_CS, LORA_RESET,
                              LORA_DIO0, LORA_DIO1, LORA_DIO2, LORA_DIO3, LORA_DIO4, LORA_DIO5,
                              NC, NC, NC, NC, LORA_ANT_SWITCH, NC, NC);
#endif //TARGET_K64F

/**
 * Constructing a LoRaRadio object for Semtech SX1276 radio
 * available on WISE-1510 module
 * http://www.advantech.com/products/ed549ce6-ff1b-4f36-a350-2ecabeb2418a/wise-1510/mod_5904a91b-0a62-4d34-97f6-b5c6b2c1ac91
 */
#if TARGET_MTB_ADV_WISE_1510
#define LORA_SPI_MOSI   PB_5
#define LORA_SPI_MISO   PB_4
#define LORA_SPI_SCK    PB_3
#define LORA_CS         PA_15
#define LORA_RESET      PC_14
#define LORA_DIO0       PC_13
#define LORA_DIO1       PB_8
#define LORA_DIO2       PB_7
#define LORA_DIO3       PD_2
#define LORA_DIO4       PC_11
#define LORA_DIO5       PC_10
#define LORA_ANT_SWITCH PC_15

static SX1276_LoRaRadio radio(LORA_SPI_MOSI, LORA_SPI_MISO, LORA_SPI_SCK, LORA_CS, LORA_RESET,
                              LORA_DIO0, LORA_DIO1, LORA_DIO2, LORA_DIO3, LORA_DIO4, LORA_DIO5,
                              NC, NC, NC, NC, LORA_ANT_SWITCH, NC, NC);
#endif //TARGET_MTB_ADV_WISE_1510

/**
 * Constructing a LoRaRadio object for Semtech 1276 radio
 * available on muRata LoRa modules
 * https://www.murata.com/en-eu/products/lpwa/lora
 * https://os.mbed.com/platforms/ST-Discovery-LRWAN1/
 */
#if defined(TARGET_DISCO_L072CZ_LRWAN1) || defined(TARGET_MTB_MURATA_ABZ)
#define LORA_SPI_MOSI   PA_7
#define LORA_SPI_MISO   PA_6
#define LORA_SPI_SCLK   PB_3
#define LORA_CS         PA_15
#define LORA_RESET      PC_0
#define LORA_DIO0       PB_4
#define LORA_DIO1       PB_1
#define LORA_DIO2       PB_0
#define LORA_DIO3       PC_13
#define LORA_ANT_RX     PA_1
#define LORA_ANT_TX     PC_2
#define LORA_ANT_BOOST  PC_1
#define LORA_TCXO       PA_12   // 32 MHz

static SX1276_LoRaRadio radio(LORA_SPI_MOSI, LORA_SPI_MISO, LORA_SPI_SCLK, LORA_CS, LORA_RESET,
                              LORA_DIO0, LORA_DIO1, LORA_DIO2, LORA_DIO3, NC, NC,
                              NC, NC, LORA_ANT_TX, LORA_ANT_RX, NC, LORA_ANT_BOOST, LORA_TCXO);
#endif

/**
 * Returns a reference the LoRaRadio object
 */
LoRaRadio& get_lora_radio()
{
    return radio;
}
