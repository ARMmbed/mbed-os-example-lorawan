# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.19.0 FATAL_ERROR)

set(MBED_PATH ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os CACHE INTERNAL "")
set(MBED_CONFIG_PATH ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")
set(APP_TARGET mbed-os-example-lorawan)

include(${MBED_PATH}/tools/cmake/app.cmake)

project(${APP_TARGET})

add_subdirectory(${MBED_PATH})

add_executable(${APP_TARGET})

# Provide Mbed OS with the header file it needs to configure Mbed TLS for LoRa
target_include_directories(${APP_TARGET}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(${APP_TARGET}
    PRIVATE
        main.cpp
        trace_helper.cpp
)

target_link_libraries(${APP_TARGET}
    PRIVATE
        mbed-os
        mbed-lorawan
)

# Custom target
add_library(mbed-k64f-sx126x INTERFACE)

target_link_libraries(mbed-k64f-sx126x INTERFACE mbed-k64f)

mbed_set_post_build(${APP_TARGET})

option(VERBOSE_BUILD "Have a verbose build process")
if(VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
