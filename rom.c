/*
* PicoROM
* Simulate a ROM chip (e.g. 28c256) with a Raspberry Pi Pico.
* Nick Bild (nick.bild@gmail.com)
* August 2021
*/

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "pin_definitions.h"
#include "romheader.h"
#include <stdlib.h>
#include "hardware/adc.h"

void setup_gpio();
int get_requested_address();
void put_data_on_bus(int);
void setup_rom_contents();
void data_bus_input();
void data_bus_output();

uint romsize = 4096;
uint8_t rom_contents[32768] = { 0 };
uint8_t cart = 0;
uint bankswitch = 0;

uint16_t addr;
uint16_t romoffset = 0;

uint16_t ce;
uint16_t last_ce;

void roms_00();
void roms_01();
void roms_02();
void roms_03();
void roms_04();
void roms_05();
void roms_06();
void roms_07();
void roms_08();
void roms_09();
void roms_10();
void roms_11();
void roms_12();
void roms_13();
void roms_14();
void roms_15();

int main() {

    // Set system clock speed.
    // 400 MHz
    // vreg_set_voltage(VREG_VOLTAGE_1_30);
    // set_sys_clock_pll(1600000000, 4, 1);
    // set_sys_clock_pll(1200000000, 3, 1);
    set_sys_clock_khz(270000, true);

    // Turn on built in LED to see that we have power
    const uint LED_PIN = 25;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    // A12 Cartridge, Data-ready pin (0 == data on address)
    const uint CE_PIN = 27;
    gpio_init(CE_PIN);
    gpio_set_dir(CE_PIN, GPIO_IN);
    // gpio_pull_up(CE_PIN);

    // Read in DIPs to get cart to use
    gpio_init(20);
    gpio_set_dir(20, GPIO_IN);
    if (gpio_get(20)){
        cart+=1;
    } 

    gpio_init(21);
    gpio_set_dir(21, GPIO_IN);
    if (gpio_get(21)){
        cart+=2;
    }

    gpio_init(22);
    gpio_set_dir(22, GPIO_IN);
    if (gpio_get(22)){
        cart+=4;
    }

    // gpio_init(26);
    // gpio_set_dir(26, GPIO_IN);
    // if (gpio_get(26)){
    //     cart+=8;
    // }
    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);
    // const float conversion_factor = 3.3f / (1 << 12);
    uint16_t result = adc_read();
    if (result  > 100){
        cart+=8;
    }

    // Specify contents of emulated ROM.
    // roms_01();
    if (cart == 0){
        roms_00();
    } else if (cart == 1){
        roms_01();
    } else if (cart == 2){
        roms_02();
    } else if (cart == 3){
        roms_03();
    } else if (cart == 4){
        roms_04();
    } else if (cart == 5){
        roms_05();
    } else if (cart == 6){
        roms_06();
    } else if (cart == 7){
        roms_07();
    } else if (cart == 8){
        roms_08();
    } else if (cart == 9){
        roms_09();
    } else if (cart == 10){
        roms_10();
    } else if (cart == 11){
        roms_11();
    } else if (cart == 12){
        roms_12();
    } else if (cart == 13){
        roms_13();
    } else if (cart == 14){
        roms_14();
    } else if (cart == 15){
        roms_15();
    }

    if (romsize == 8192){
        bankswitch = 1; // F8 bankswitching
    } else if (romsize == 16384){
        bankswitch = 2; // F6 bankswitching
    } else if (romsize == 32768){
        bankswitch = 3; // F4 bankswitching
    }

    last_ce = 1;
    
    // GPIO setup for address and data bus
    setup_gpio();
    
    // Continually check address lines and
    // put associated data on bus.
    while (true) {
        // put_data_on_bus(get_requested_address());

        ce = gpio_get(CE_PIN);
        if (ce != last_ce) {
            last_ce = ce;
            if (last_ce) {
                data_bus_input();
            } else{
                data_bus_output();
            }
        }
        if (last_ce == 0){
            addr = get_requested_address();
            put_data_on_bus(addr + romoffset);

            if (bankswitch == 1 && addr == 4088){ //F8 bankswitching (8K)
                romoffset = 0;
            } else if (bankswitch == 1 && addr == 4089){
                romoffset = 4096;
            } else if (bankswitch == 2 && addr == 4086){ //F6 bankswitching (16K)
                romoffset = 0;
            } else if (bankswitch == 2 && addr == 4087){ 
                romoffset = 4096;
            } else if (bankswitch == 2 && addr == 4088){ 
                romoffset = 8192;
            } else if (bankswitch == 2 && addr == 4089){ 
                romoffset = 12288;
            } else if (bankswitch == 3 && addr == 4084){ //F4 bankswitching (32K)
                romoffset = 0;
            } else if (bankswitch == 3 && addr == 4085){ 
                romoffset = 4096;
            } else if (bankswitch == 3 && addr == 4086){ 
                romoffset = 8192;
            } else if (bankswitch == 3 && addr == 4087){ 
                romoffset = 12288;
            } else if (bankswitch == 3 && addr == 4088){ 
                romoffset = 16384;
            } else if (bankswitch == 3 && addr == 4089){ 
                romoffset = 20480;
            } else if (bankswitch == 3 && addr == 4090){ 
                romoffset = 24576;
            } else if (bankswitch == 3 && addr == 4091){ 
                romoffset = 28672;
            }

        }

    }
}

void data_bus_input() {
    // Data pins.
    gpio_set_dir(D0, GPIO_IN);
    gpio_set_dir(D1, GPIO_IN);
    gpio_set_dir(D2, GPIO_IN);
    gpio_set_dir(D3, GPIO_IN);
    gpio_set_dir(D4, GPIO_IN);
    gpio_set_dir(D5, GPIO_IN);
    gpio_set_dir(D6, GPIO_IN);
    gpio_set_dir(D7, GPIO_IN);
}

void data_bus_output() {
    // Data pins.
    gpio_set_dir(D0, GPIO_OUT);
    gpio_set_dir(D1, GPIO_OUT);
    gpio_set_dir(D2, GPIO_OUT);
    gpio_set_dir(D3, GPIO_OUT);
    gpio_set_dir(D4, GPIO_OUT);
    gpio_set_dir(D5, GPIO_OUT);
    gpio_set_dir(D6, GPIO_OUT);
    gpio_set_dir(D7, GPIO_OUT);
}

void setup_gpio() {
    // Address pins.
    gpio_init(A0);
    gpio_set_dir(A0, GPIO_IN);
    gpio_init(A1);
    gpio_set_dir(A1, GPIO_IN);
    gpio_init(A2); 
    gpio_set_dir(A2, GPIO_IN);
    gpio_init(A3);
    gpio_set_dir(A3, GPIO_IN);
    gpio_init(A4);
    gpio_set_dir(A4, GPIO_IN);
    gpio_init(A5);
    gpio_set_dir(A5, GPIO_IN);
    gpio_init(A6);
    gpio_set_dir(A6, GPIO_IN);
    gpio_init(A7);
    gpio_set_dir(A7, GPIO_IN);
    gpio_init(A8);
    gpio_set_dir(A8, GPIO_IN);
    gpio_init(A9);
    gpio_set_dir(A9, GPIO_IN);
    gpio_init(A10);
    gpio_set_dir(A10, GPIO_IN);
    gpio_init(A11);
    gpio_set_dir(A11, GPIO_IN);
    // gpio_init(A12);
    // gpio_set_dir(A12, GPIO_IN);
    // gpio_init(A13);
    // gpio_set_dir(A13, GPIO_IN);
    // gpio_init(A14);
    // gpio_set_dir(A14, GPIO_IN);

    // Data pins.
    gpio_init(D0);
    gpio_set_dir(D0, GPIO_OUT);
    gpio_init(D1);
    gpio_set_dir(D1, GPIO_OUT);
    gpio_init(D2);
    gpio_set_dir(D2, GPIO_OUT);
    gpio_init(D3);
    gpio_set_dir(D3, GPIO_OUT);
    gpio_init(D4);
    gpio_set_dir(D4, GPIO_OUT);
    gpio_init(D5);
    gpio_set_dir(D5, GPIO_OUT);
    gpio_init(D6);
    gpio_set_dir(D6, GPIO_OUT);
    gpio_init(D7);
    gpio_set_dir(D7, GPIO_OUT);
}

int get_requested_address() {
    // Return only first 12 bits.
    // return gpio_get_all() & 32767;
    return ((gpio_get_all() >> 8) & 4095);
}

void put_data_on_bus(int address) {
    // int data = rom_contents[address];

    // gpio mask = 8355840; // i.e.: 11111111000000000000000
    // Shift data 15 bits to put it in correct position to match data pin defintion.
    gpio_put_masked(255, rom_contents[address]);
}

