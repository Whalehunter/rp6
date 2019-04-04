CC=avr-gcc
MMCU=atmega32
OBJ = slave.o rp6.o

build:
	$(CC) -g -Os -mmcu=$(MMCU) -c slave.c
	$(CC) -g -mmcu=$(MMCU) -o slave.elf slave.o
	avr-objcopy -j .text -j .data -O ihex slave.elf slave.hex
