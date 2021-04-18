#!/bin/sh
avr-gcc -mmcu=attiny4313 -Wall -Os -o magdaq.elf magdaq.c
avr-objcopy -j .text -j .data -O ihex magdaq.elf magdaq.hex
