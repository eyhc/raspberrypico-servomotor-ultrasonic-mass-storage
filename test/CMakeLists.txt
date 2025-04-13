cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD_HEADER_DIRS ${CMAKE_SOURCE_DIR}/..)
set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)
set(PICO_DEFAULT_BOOT_STAGE2 boot2_generic_03h)
set(PICO_SDK_PATH ../pico-sdk)

include(../pico-sdk/pico_sdk_init.cmake)

project(pico-servo-ultrasonic-tests)

pico_sdk_init()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PICO_SDK_PATH}/lib/tinyusb/lib/fatfs/source   # fatfs lib
)

add_library(servomotor ../servomotor.c)
target_link_libraries(servomotor hardware_pwm hardware_gpio)

add_library(ultrasonic ../ultrasonic.c)
target_link_libraries(ultrasonic hardware_gpio pico_stdlib pico_time)

add_library(config ../config.c)
target_link_libraries(config hardware_flash pico_stdlib pico_flash)

add_executable(test-servo test_servo.c)
target_link_libraries(test-servo servomotor pico_stdlib)
pico_enable_stdio_usb(test-servo 1)
pico_add_extra_outputs(test-servo)

add_executable(test-ultrasonic test_ultrasonic.c)
target_link_libraries(test-ultrasonic ultrasonic pico_stdlib)
pico_enable_stdio_usb(test-ultrasonic 1)
pico_add_extra_outputs(test-ultrasonic)

add_executable(test-config test_config.c)
target_link_libraries(test-config config pico_stdlib)
pico_enable_stdio_usb(test-config 1)
pico_add_extra_outputs(test-config)
