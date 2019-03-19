rijden:
	avr-gcc -g -Os -mmcu=atmega32 -c rijden.c
	avr-gcc -g -mmcu=atmega32 -o rijden.elf rijden.o
	avr-objcopy -j .text -j .data -O ihex rijden.elf rijden.hex
