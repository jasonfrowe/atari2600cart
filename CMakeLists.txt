cmake_minimum_required(VERSION 3.21)

# Pull in SDK (must be before project)
# include(pico_sdk_import.cmake)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# project(PICO_ROM CXX C ASM)
project(PICO_ROM CXX C ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


# Initialize the SDK
pico_sdk_init()

add_executable(pico_rom)

target_sources(pico_rom PRIVATE rom.c)

target_link_libraries(pico_rom PRIVATE
        pico_stdlib
        hardware_pio
        hardware_adc
        )

pico_add_extra_outputs(pico_rom)

# Enable USB output, disable UART output.
# pico_enable_stdio_usb(pico_rom 1)
# pico_enable_stdio_uart(pico_rom 0)
