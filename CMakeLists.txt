# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.18.2 FATAL_ERROR)

# TODO: @mbed-os-tools MBED_ROOT and MBED_CONFIG_PATH should probably come from mbedtools
set(MBED_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os CACHE INTERNAL "")
set(MBED_CONFIG_PATH ${CMAKE_CURRENT_SOURCE_DIR}/.mbedbuild CACHE INTERNAL "")
set(APP_TARGET mbed-os-example-lorawan)

include(${MBED_ROOT}/tools/cmake/app.cmake)

add_subdirectory(${MBED_ROOT})

add_executable(${APP_TARGET})

mbed_configure_app_target(${APP_TARGET})

mbed_set_mbed_target_linker_script(${APP_TARGET})

project(${APP_TARGET})

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
        mbed-mbedtls
)

mbed_set_post_build(${APP_TARGET})

option(VERBOSE_BUILD "Have a verbose build process")
if(VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
