# atari2600cart
DIY Atari 2600+ Cart

Using a Raspberry Pi PICO to emulate a 28C256 32KB EEPROM which is then interfaced to an Atari 2600 cartridge card.

The ROM emulator is based on PicoROM (https://github.com/nickbild/picoROM) and the cartridge PCB is from https://www.pcbway.com/project/shareproject/ATARI_2600_MULTI_GAME_CARTRIDGE_PCB.html.

The cart allows for 16 different cartridges of 4K, 8K, 16K and 32K sizes with standard bankswitching.  A reset switch is wired, which allows for hot-swapping cartridges on the 2600+.

A helper script 'make_romheaders.py' is provided that will read in ROMS from the roms/ directory and create the necesssary roms_XX.h headers.

![Alt Final Product in an Atari 2600+](pics/IMG_6257.png)


## Build Log

I'm going to put together a build walk through to help those that may want to create their own Pico Multi-Cart for the 2600+

![Alt Picture of Required Materials](pics/IMG_6307.png)


