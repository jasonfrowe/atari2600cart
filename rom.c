/*
* PicoMultiRom for ATARI 2600+
* Jason Rowe (jasonfrowe@gmail.com)
* Based on PicoROM by Nick Bild (nick.bild@gmail.com)
* and Otaku-flash by karrika (https://github.com/karrika/Otaku-flash)
* January 2024
*/

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "pin_definitions.h"
// #include "romheader.h"
#include <stdlib.h>
#include "hardware/adc.h"

void setup_gpio(); // uses pin defintions and sets address and data bus
int get_requested_address(); // get address from address bus
void put_data_on_bus(uint8_t); // puts data on bus
void setup_cart();  // Will fetch the rom contents

void (*fpBankSwitching)(uint16_t); // function pointer
void BankSwitching_none(uint16_t);
void BankSwitching_F8(uint16_t); // 8K
void BankSwitching_F6(uint16_t); // 16K
void BankSwitching_F4(uint16_t); // 32K


// Variables about the ROM
uint romsize = 4096; // Will be set by setup_cart
uint8_t rom_contents[32768] = { 0 }; // Will be set by setup_cart
uint8_t cart = 0; // Will be set by GPIO pins
uint bankswitch = 0; // Will be set by setup_cart

uint16_t addr;  // address read from address lines
uint16_t romoffset = 0; // offset for bankswitching

uint16_t ce;  // chip-enable value
uint16_t last_ce; // last value to monitor changes in ce 


int main() {

    // Set system clock speed.
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

    setup_cart(); // Get rom contents and setup bankswitching
    
    last_ce = 1;
    
    // GPIO setup for address and data bus
    setup_gpio();
    
    // Continually check address lines and
    // put associated data on bus.
    while (true) {

        ce = gpio_get(CE_PIN);
        if (ce != last_ce) {
            last_ce = ce;
            if (last_ce) {
                gpio_set_dir_in_masked(0xFF); // data_bus_input();
            } else{
                gpio_set_dir_out_masked(0xFF); // data_bus_output();
            }
        }
        if (last_ce == 0){
            addr = get_requested_address();
            put_data_on_bus(rom_contents[addr + romoffset]);

            // bankswitching function
            (*fpBankSwitching)(addr);

        }

    }
}

// Function to set up ROM from DIP-switches

void setup_cart() {
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
    if (cart == 0){
        #include "roms_00.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 1){
        #include "roms_01.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 2){
        #include "roms_02.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 3){
        #include "roms_03.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 4){
        #include "roms_04.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 5){
        #include "roms_05.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 6){
        #include "roms_06.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 7){
        #include "roms_07.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 8){
        #include "roms_08.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 9){
        #include "roms_09.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 10){
        #include "roms_10.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 11){
        #include "roms_11.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 12){
        #include "roms_12.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 13){
        #include "roms_13.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 14){
        #include "roms_14.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 15){
        #include "roms_15.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    }

    if (romsize == 4096){
        bankswitch = 0; // No bankswitching
        fpBankSwitching = &BankSwitching_none;
    } else if (romsize == 8192){
        bankswitch = 1; // F8 bankswitching
        fpBankSwitching = &BankSwitching_F8;
    } else if (romsize == 16384){
        bankswitch = 2; // F6 bankswitching
        fpBankSwitching = &BankSwitching_F6;
    } else if (romsize == 32768){
        bankswitch = 3; // F4 bankswitching
        fpBankSwitching = &BankSwitching_F4;
    }

}

// Bank Switching Functions 

void BankSwitching_none(uint16_t addr_in) {
    romoffset = 0;
}

void BankSwitching_F8(uint16_t addr_in) {
    if (addr_in == 4088){
        romoffset = 0;
    } else if (addr_in == 4089){
        romoffset = 4096;
    }
}

void BankSwitching_F6(uint16_t addr_in) {
    if ((addr_in >= 4086) && (addr_in <= 4089)){
        romoffset = 4096 * (addr_in - 4086);
    }
}


void BankSwitching_F4(uint16_t addr_in) {

    if ((addr_in >= 4084) && (addr_in <= 4091)){
        romoffset = 4096 * (addr_in - 4084);
    }
}


// GPIO Setup Functions 

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
    // Return only first 12 bits from address bus.
    return ((gpio_get_all() >> 8) & 4095);
}

void put_data_on_bus(uint8_t rom_cont) {
    // 8-bits onto the data bus
    gpio_put_masked(255, rom_cont);
}

