#include <avr/io.h>
#include <avr/interrupt.h>

#define SLA 56

char x, y;

void i2c_slave_init() {
	TWSR = 0;
	TWAR = (SLA<<1); // move address into Address Register
	TWCR = (1<<TWEA) | (1<<TWIE) | (1<<TWEN);
}

ISR(TWI_vect) {
	switch(TWSR) {
		// Slave Receiver mode
		case 0x60: // Own SLA+W has been received; ACK has been returned
		break;
		case 0x80: // Addressed with own SLA+W; data received; ACK send
		x = TWDR; break;
		case 0xA0: // A STOP or repeated START condition has been received
		y = x; break;
		// Slave Transmitter mode
		case 0xA8: // Own SLA+R has been received; ACK has been returned
		i2cFunctions(y); break;
		case 0xB8: // Data byte in TWDR has been send; ACK has been received
		i2cFunctions(y); break;
		case 0xC0: // Data byte in TWDR has been send; NACK has been received
		y = 0; break;
	}
	TWCR |= (1<<TWINT); // Clear TWINT Flag
}
void i2cFunctions(char a){
	switch(a){
		case 'a':
		TWDR = '<'; break;
		case 'd':
		TWDR = '>'; break;
		case 'w':
		TWDR = '^'; break;
		case 's':
		TWDR = '.'; break;
		default: break;
	}
}
void main(void) {
	sei(); // enable general interrupt
	i2c_slave_init();
	while (1) {
	}
}