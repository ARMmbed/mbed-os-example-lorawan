# Example LoRaWAN application for Mbed-OS

This is an example application based on `Mbed-OS` LoRaWAN protocol APIs. The Mbed-OS LoRaWAN stack implementation is compliant with LoRaWAN v1.0.2 specification. 

## Getting started

This application can work with any Network Server if you have correct credentials for the said Network Server. 

### Download the application

```sh
$ mbed import mbed-os-example-lorawan
$ cd mbed-os-example-lorawan

#OR

$ git clone git@github.com:ARMmbed/mbed-os-example-lorawan.git
$ cd mbed-os-example-lorawan
$ mbed deploy
```

### Selecting radio

Mbed OS provides inherent support for a variety of modules. If your device is one of the those modules, you may skip this part. The correct radio type and pin set is already provided for the modules in the `target-overrides` field. For more information on supported modules, please refer to the [module support section](#module-support)

If you are using an Mbed Enabled radio shield such as [Mbed SX1276 shield LoRa](https://os.mbed.com/components/SX1276MB1xAS/) or [Mbed SX1272 LoRa shield ](https://os.mbed.com/components/SX1272MB2xAS/) with any Mbed Enabled board, this part is relevant. You can use any Mbed Enabled board that comes with an arduino form factor.

Please select your radio type by modifying the `lora-radio` field and providing a pin set if it is different from the default. For example:

```json
"lora-radio": {
    "help": "Which radio to use (options: SX1272,SX1276)",
    "value": "SX1272"
},
```

### Add network credentials

Open the file `mbed_app.json` in the root directory of your application. This file contains all the user specific configurations your application and the Mbed OS LoRaWAN stack need. Network credentials are typically provided by LoRa network provider.

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

In addition to that, you need to provide `Application Session Key`, `Network Session Key` and `Device Address`. For example:

```json
"lora.appskey": "{ YOUR_APPLICATION_SESSION_KEY }",
"lora.nwkskey": "{ YOUR_NETWORK_SESSION_KEY }",
"lora.device-address": " YOUR_DEVICE_ADDRESS_IN_HEX  " 
```

## Configuring the application

The Mbed OS LoRaWAN stack provides a lot of configuration controls to the application through the Mbed OS configuration system. The previous section discusses some of these controls. This section highlights some useful features that you can configure.

### Selecting a PHY

The LoRaWAN protocol is subject to various country specific regulations concerning radio emissions. That's why the Mbed OS LoRaWAN stack provides a `LoRaPHY` class that you can use to implement any region specific PHY layer. Currently, the Mbed OS LoRaWAN stack provides 10 different country specific implementations of `LoRaPHY` class. Selection of a specific PHY layer happens at compile time. By default, the Mbed OS LoRaWAN stack uses `EU 868 MHz` PHY. An example of selecting a PHY can be:

```josn
        "phy": {
            "help": "LoRa PHY region. 0 = EU868 (default), 1 = AS923, 2 = AU915, 3 = CN470, 4 = CN779, 5 = EU433, 6 = IN865, 7 = KR920, 8 = US915, 9 = US915_HYBRID",
            "value": "0"
        },
```

### Duty cycling

LoRaWAN v1.0.2 specifcation is exclusively duty cycle based. This application comes with duty cycle enabled by default. In other words, the Mbed OS LoRaWAN stack enforces duty cycle. The stack keeps track of transmissions on the channels in use and schedules transmissions on channels that become available in the shortest time possible. We recommend you keep duty cycle on for compliance with your country specific regulations. 

However, you can define a timer value in the application, which you can use to perform a periodic uplink when the duty cycle is turned off. Such a setup should be used only for testing or with a large enough timer value. For example:

```josn 
"target_overrides": {
	"*": {
		"lora.duty-cycle-on": false
		},
	}
}
```

## Module support

Here is a nonexhaustive list of boards and modules that we have tested with the Mbed OS LoRaWAN stack.

- MultiTech mDot.
- MultiTech xDot.
- LTEK_FF1705.
- Advantech Wise 1510.
- ST B-L072Z-LRWAN1 LoRa®Discovery kit (with muRata radio chip).

## Compiling the application

Use Mbed CLI commands to generate a binary for the application.
For example:

```sh
$ mbed compile -m YOUR_TARGET -t ARM
```

## Running the application

Drag and drop the application binary from `BUILD/YOUR_TARGET/ARM/mbed-os-example-lora.bin` to your Mbed enabled target hardware, which appears as a USB device on your host machine. 

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

## [Optional] Adding trace library
To enable Mbed trace, add to your `mbed_app.json` the following fields:

```json
    "target_overrides": {
        "*": {
            "target.features_add": ["COMMON_PAL"],
            "mbed-trace.enable": true
            }
     }
```
The trace is disabled by default to save RAM and reduce main stack usage (see chapter Memory optimization).

**Please note that some targets with small RAM size (e.g. DISCO_L072CZ_LRWAN1 and MTB_MURATA_ABZ) mbed traces cannot be enabled without increasing the default** `"main_stack_size": 1024`**.**

## [Optional] Memory optimization 

Using `Arm CC compiler` instead of `GCC` reduces `3K` of RAM. Currently the application takes about `15K` of static RAM with Arm CC, which spills over for the platforms with `20K` of RAM because you need to leave space, about `5K`, for dynamic allocation. So if you reduce the application stack size, you can barely fit into the 20K platforms.

For example, add the following into `config` section in your `mbed_app.json`:

```
"main_stack_size": {
    "value": 2048
}
```

Essentially you can make the whole application with Mbed LoRaWAN stack in 6K if you drop the RTOS from Mbed OS and use a smaller standard C/C++ library like new-lib-nano. Please find instructions [here](https://os.mbed.com/blog/entry/Reducing-memory-usage-with-a-custom-prin/).
 

For more information, please follow this [blog post](https://os.mbed.com/blog/entry/Reducing-memory-usage-by-tuning-RTOS-con/).
