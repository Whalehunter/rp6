CC=avr-gcc
MMCU=atmega32
MASTER=atmega2560
OBJ = slave.o rp6.o

slave:
	$(CC) -g -Os -mmcu=$(MMCU) -c slave.c
	$(CC) -g -mmcu=$(MMCU) -o slave.elf slave.o
	avr-objcopy -j .text -j .data -O ihex slave.elf slave.hex

master:
	$(CC) -g -Os -mmcu=$(MASTER) -c master.c
	$(CC) -g -mmcu=$(MASTER) -o master.elf master.o
	avr-objcopy -j .text -j .data -O ihex master.elf master.hex
	avrdude -p $(MASTER) -c wiring -P /dev/ttyACM0 -b 115200 -D -U flash:w:master.hex
