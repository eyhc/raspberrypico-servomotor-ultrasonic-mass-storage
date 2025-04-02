cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD_HEADER_DIRS ${CMAKE_SOURCE_DIR})
set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)
set(PICO_DEFAULT_BOOT_STAGE2 boot2_generic_03h)

include(pico-sdk/pico_sdk_init.cmake)

project(pico-servo-ultrasonic)

pico_sdk_init()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(servomotor servomotor.c)
target_link_libraries(servomotor hardware_pwm hardware_gpio)

add_library(ultrasonic ultrasonic.c)
target_link_libraries(ultrasonic hardware_gpio pico_stdlib pico_time)

add_subdirectory(test)

#add_executable(pico-servo-ultrasonic main.c)
#target_link_libraries(pico-servo-ultrasonic servomotor pico_stdlib)
#pico_enable_stdio_usb(pico-servo-ultrasonic 1)
#pico_add_extra_outputs(pico-servo-ultrasonic)
