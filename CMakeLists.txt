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

# Run from RAM only. Flash is too slow for heavy overclocking.
#set(PICO_DEFAULT_BINARY_TYPE copy_to_ram)

add_executable(pico_rom)

target_sources(pico_rom PRIVATE rom.c roms_00.c roms_01.c roms_02.c roms_03.c
roms_04.c roms_05.c roms_06.c roms_07.c roms_08.c roms_09.c roms_10.c roms_11.c 
roms_12.c roms_13.c roms_14.c roms_15.c)

target_link_libraries(pico_rom PRIVATE
        pico_stdlib
        hardware_pio
        hardware_adc
        )

pico_add_extra_outputs(pico_rom)

# Enable USB output, disable UART output.
# pico_enable_stdio_usb(pico_rom 1)
# pico_enable_stdio_uart(pico_rom 0)

# Slow down flash during boot for heavy overclocking.
#pico_define_boot_stage2(slower_boot2 ${PICO_DEFAULT_BOOT_STAGE2_FILE})
#target_compile_definitions(slower_boot2 PRIVATE PICO_FLASH_SPI_CLKDIV=4)
#pico_set_boot_stage2(pico_rom slower_boot2)
