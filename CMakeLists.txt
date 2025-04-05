cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD_HEADER_DIRS ${CMAKE_SOURCE_DIR})
set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)
set(PICO_DEFAULT_BOOT_STAGE2 boot2_generic_03h)

include(pico-sdk/pico_sdk_init.cmake)

project(pico-servo-ultrasonic)

pico_sdk_init()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_LIST_DIR}                       # tusb_config.h
    ${PICO_SDK_PATH}/lib/tinyusb/lib/fatfs/source   # fatfs lib
)

add_library(servomotor servomotor.c)
target_link_libraries(servomotor hardware_pwm hardware_gpio)

add_library(ultrasonic ultrasonic.c)
target_link_libraries(ultrasonic hardware_gpio pico_stdlib pico_time)

add_library(inih inih/ini.c)
target_link_libraries(inih pico_stdlib)

add_library(config config.c)
target_link_libraries(config hardware_flash pico_stdlib pico_flash inih)

add_library(
    ram_disk
    ${PICO_SDK_PATH}/lib/tinyusb/lib/fatfs/source/ff.c
    ${PICO_SDK_PATH}/lib/tinyusb/lib/fatfs/source/ffsystem.c
    ${PICO_SDK_PATH}/lib/tinyusb/lib/fatfs/source/ffunicode.c
    ram_disk.c
)

add_library(usb_msc usb_descriptors.c tusb_msc.c)
target_link_libraries(usb_msc pico_stdlib pico_unique_id tinyusb_device tinyusb_board ram_disk)
target_compile_definitions(usb_msc PRIVATE TUSB_CONFIG_FILE="tusb_config_msc.h")

add_executable(pico-servo-ultrasonic main.c)
target_link_libraries(pico-servo-ultrasonic
    servomotor
    ultrasonic
    config
    usb_msc
    pico_stdlib
)
pico_enable_stdio_usb(pico-servo-ultrasonic 1)
pico_add_extra_outputs(pico-servo-ultrasonic)
