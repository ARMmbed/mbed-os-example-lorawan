# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13)

# Mbed OS system root path - that probably needs to come from tools
# TODO: @mbed-os-tools
set(MBED_OS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os CACHE INTERNAL "")

# TODO: @mbed-os-tools This needs to come from tools
# Build system provides "component" list. This property is defined in mbed_app.json
# to select the shield to use.
# DISCO_L475VG_IOT01A
set_property(GLOBAL
  PROPERTY MBED_OS_TARGET_LABELS
    SX1276;
)

# Include standard Mbed definitions
include(${MBED_OS_ROOT}/cmake/app.cmake)

project(mbed-os-example-lorawan)

# uncomment below to have a verbose build process
#set(CMAKE_VERBOSE_MAKEFILE ON)

# Sources for main application
target_sources(app
    PRIVATE
        main.cpp
        trace_helper.cpp
)
target_include_directories(mbed-os
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
