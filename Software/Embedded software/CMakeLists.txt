# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(wristband)

target_sources(app PRIVATE src/main.c src/as7058_drv.c)
target_sources(app PRIVATE src/main.c src/flash_drv.c)
target_sources(app PRIVATE src/main.c src/usb_drv.c)
target_sources(app PRIVATE src/main.c src/shell.c)
include(${ZEPHYR_BASE}/samples/subsys/usb/common/common.cmake)
