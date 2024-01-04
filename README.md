# atari2600cart
DIY Atari 2600+ Cart

Using a Raspberry Pi PICO to emulate a 28C256 32KB EEPROM which is then interfaced to an Atari 2600 cartridge card.

The ROM emulator is based on PicoROM (https://github.com/nickbild/picoROM) and the cartridge PCB is from https://www.pcbway.com/project/shareproject/ATARI_2600_MULTI_GAME_CARTRIDGE_PCB.html.

The cart allows for 16 different cartridges of 4K, 8K, 16K and 32K sizes with standard bankswitching.  A reset switch is wired, which allows for hot-swapping cartridges on the 2600+.

User must supply their own roms_00.c -- roms_15.c that contain the game ROMs.  An example, roms_ex.c is provided.  
