cmake_minimum_required(VERSION 3.20.0)

set(ZEPHYR_TOOLCHAIN_VARIANT 0.13.2)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app)

target_sources(app PRIVATE
    src/main.cpp
    src/ble/common.c
    src/ble/ble.c
    src/ble/service/info.c
    src/ble/service/current_time.c
    src/ble/service/accel.c
    src/ble/service/random.c
)