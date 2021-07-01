# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0


add_library(mbed-mytarget INTERFACE)

target_sources(mbed-mytarget
    INTERFACE
        PeripheralPins.c
        system_clock.c
)

target_include_directories(mbed-mytarget
    INTERFACE
        .
)

   target_link_libraries(mbed-mytarget INTERFACE mbed-stm32wb55xx)
