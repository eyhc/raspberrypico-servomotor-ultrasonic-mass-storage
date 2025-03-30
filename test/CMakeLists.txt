add_executable(test-servo test_servo.c)
target_link_libraries(test-servo servomotor pico_stdlib)
pico_enable_stdio_usb(test-servo 1)
pico_add_extra_outputs(test-servo)

