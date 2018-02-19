# Example LoRaWAN application for Mbed-OS

This is an example application based on `Mbed-OS` LoRaWAN protocol APIs. The Mbed-OS LoRaWAN stack implementation is compliant with LoRaWAN v1.0.2 specification.

## Getting started

This application can work with any Network Server if you have correct credentials for the said Network Server.

### Download the application

```sh
$ mbed import mbed-os-example-lora
$ cd mbed-os-example-lora

#OR

$ git clone git@github.com:ARMmbed/mbed-os-example-lora.git
$ cd mbed-os-example-lora
$ mbed deploy
```

### Add network credentials

Open the file `mbed_app.json` in the root directory of your application.
This file contains all the user specific configurations your application and the Mbed-OS LoRaWAN stack needs.

#### For OTAA

Please add `Device EUI`, `Application EUI` and `Application Key` needed for Over-the-air-activation(OTAA). For example:

```json

"lora.device-eui": "{ YOUR_DEVICE_EUI }",
"lora.application-eui": "{ YOUR_APPLICATION_EUI }",
"lora.application-key": "{ YOUR_APPLICATION_KEY }"
```

#### For ABP

For Activation-By-Personalization (ABP) connection method, modify the `mbed_app.json` to enable ABP. You can do it by simply turning off OTAA. For example:

```json
"lora.over-the-air-activation": false,
```

In addition to that you need to provide  `Application Session Key`, `Network Session Key` and `Device Address`. For example:

```json
"lora.appskey": "{ YOUR_APPLICATION_SESSION_KEY }",
"lora.nwkskey": "{ YOUR_NETWORK_SESSION_KEY }",
"lora.device-address": "{ YOUR_DEVICE_ADDRESS } "
```
## Configuring the application

Mbed-OS LoRaWAN stack provides a lot of configuration controls to the application via Mbed-OS config system. Some of such controls are discussed in the previous section. In this section we will highlight some useful features that can be configured.

### Selecting a PHY

LoRaWAN protocol is subjected to various country specific regulations concerning radio emissions. That's why Mbed-OS LoRaWAN stack provides a `LoRaPHY` class which can be used to implement any region specific PHY layer. Currently, Mbed-OS LoRaWAN stack provides 10 different country-specific implementations of `LoRaPHY` class. Selection of a specific PHY layer happens at compile time. Bu default, the Mbed-OS LoRaWAN stack uses `EU 868 MHz` PHY. An example of selecting a PHY can be:

```josn
"phy": {
            "help": ["Select LoRa PHY layer. See README.md for more information. Default: 0 = LORA_PHY_EU868",
            "                                                                             1 = LORA_PHY_AS923",
            "                                                                             2 = LORA_PHY_AU915",
            "                                                                             3 = LORA_PHY_CN470",
            "                                                                             4 = LORA_PHY_CN779",
            "                                                                             5 = LORA_PHY_EU433",
            "                                                                             6 = LORA_PHY_IN865",
            "                                                                             7 = LORA_PHY_KR920",
            "                                                                             8 = LORA_PHY_US915",
            "                                                                             9 = LORA_PHY_US915_HYBRID"],
            "value": "0"
        },
```

### Duty cycling

LoRaWAN v1.0.2 specifcation is exclusively duty cycle based. This application comes with duty cycle enabled by default, i.e., the Mbed-OS LoRaWAN stack enforces duty cycle. The stack keep track of transmissions on the channels in use and hence schedules transmissions on channels which become available in the shortest time possible.  We recommend to keep duty cycle on for compliance to your country specific regulation.

However, user can define a timer value using `mbed_app.json` which can be used to perform a periodic uplink when the duty cycle is turned off. Such a setup should be used only for testing or with a large enough timer value. For example:

```json
"config": {
    "tx-timer": 10000
},
"target_overrides": {
    "*": {
        "lora.duty-cycle-on": false
    }
}
```

### Tracing

To view debug information from the LoRaWAN stack and the radio drivers you can enable tracing. Note that this has an effect on memory usage, which is why it's disabled by default. Tracing is only available if Mbed RTOS is present.

To enable tracing, add the following to `mbed_app.json` under `target_overrides.*`:

```json
            "target.features_add": ["COMMON_PAL"],
            "mbed-trace.enable": true,
```

## Board & Module support

Mbed-OS provides drivers for Semtech SX1272 and SX1276  LoRa radios. If you have an Mbed enabled radio shield like [Mbed SX1276 shield LoRa](https://os.mbed.com/components/SX1276MB1xAS/)  or [Mbed SX1272 LoRa shield ](https://os.mbed.com/components/SX1272MB2xAS/) , you can virtually use any Mbed-enabled board. If you have a module which have a built-in SX1272 or SX1276 radio, you can still use the drivers  provided by Mbed-OS. All you need to do is to construct a proper `LoRaRadio` object with correct pinout for your board. Please refer to `lora_radio_helper.cpp` which helps your application to construct a `LoRaRadio` object.

Here is a non-exhaustive list of boards and modules which we have tested with Mbed-OS LoRaWAN stack.

- Any Mbed-enabled board with Mbed LoRa radio shield (SX1272 or SX1276)
- MultiTech mDot
- MultiTech xDot
- ST B-L072Z-LRWAN1 LoRa®Discovery kit (with muRata radio chip)

## Compiling the application

Use Mbed CLI commands to generate a binary for the application.
For example:

```sh
$ mbed compile -m YOUR_TARGET -t ARM
```

## Running the application

Drag and drop the application binary from `BUILD/YOUR_TARGET/ARM/mbed-os-example-lora.bin` yo your Mbed enabled target hardware which appears as USB device on your host machine.

Attach a serial console emulator of your choice (for example, PuTTY, Minicom or screen) to your USB device. Set the baudrate to 115200 bit/s, and reset your board by pressing the reset button.

You should see an output similar to this:

```
Mbed LoRaWANStack initialized

 CONFIRMED message retries : 3

 Adaptive data  rate (ADR) - Enabled

 Connection - In Progress ...

 Connection - Successful

 Dummy Sensor Value = 2.1

 25 bytes scheduled for transmission

 Message Sent to Network Server

```