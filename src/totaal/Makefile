CC=avr-gcc
MMCU=atmega32
DEPS = rp6.h
OBJ = slave.o rp6.o

%.o: %.c $(DEPS)
	$(CC) -g -Os -mmcu=$(MMCU) -c $^

slave.elf: $(OBJ)
	$(CC) -g -mmcu=$(MMCU) -o $@ $^

slave.hex: slave.elf
	avr-objcopy -j .text -j .data -O ihex $^ $@
