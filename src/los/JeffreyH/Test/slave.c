#define F_CPU 8000000

#include <avr/io.h>
#include <avr/interrupt.h>

#define SLA 56

// Declarations of functions
void i2c_slave_init();
void init();
void init_usart();
// int leescommand();
void rijden(char x);
//int leessnelheid();



int main (void){
    // Initialisers
    cli();
    init();
    //init_usart();
    sei();
	i2c_slave_init();


    while(1)
        {
        }
    return 0;
}

ISR(TWI_vect) {
	char x,y;
	
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
		rijden(y); break;
		case 0xB8: // Data byte in TWDR has been send; ACK has been received
		rijden(y); break;
		case 0xC0: // Data byte in TWDR has been send; NACK has been received
		y = 0; break;
	}
	TWCR |= (1<<TWINT); // Clear TWINT Flag
}

// Functions used:
void i2c_slave_init() {
	TWSR = 0;
	TWAR = (SLA<<1); // move address into Address Register
	TWCR = (1<<TWEA) | (1<<TWIE) | (1<<TWEN);
}

void init_usart(){
    UCSRA = 0; // USART status and control registerA uit
    UCSRB = (1 << RXEN) | (1 << RXCIE); // Enable de USART Reciever
    UCSRC = (1 << UCSZ1) | (1 << UCSZ0); /* 8 data bits, 1 stop bit */
    UBRRH = 00;
    UBRRL = 12; //baudrade 38.4K. voor robotloader
}

void rijden (char x) {

    switch(x) {
    case 'w':
        PORTC &= ~(1 << PINC2);
        PORTC &= ~(1 << PINC3);
		TWDR = '^';
        break;
    case 'a':
        PORTC |= (1 << PINC2);
        PORTC &= ~(1 << PINC3);
		TWDR = '<';
        break;
    case 's':
        PORTC |= (1 << PINC2);
        PORTC |= (1 << PINC3);
		TWDR = '.';
        break;
    case 'd':
        PORTC &= ~(1 << PINC2);
        PORTC |= (1 << PINC3);
		TWDR = '>';
        break;
    case '1':
        OCR1A = 50;
        OCR1B = 50;
        break;
    case '2':
        OCR1A = 125;
        OCR1B = 125;
        break;
    case '3':
        OCR1A = 150;
        OCR1B = 150;
        break;
    case 'q':
        OCR1A = 0;
        OCR1B = 0;
    }
}


void init()
{
    TCNT1 = 0;

    TCCR1A |= (1 << COM1A1)|(1 << COM1B1)|(1 << WGM11); // set output to high on compare + pwm 9-bit
    TCCR1B |= (1 << WGM13)|(1 << CS10); // no prescaler + pwm, phase & frequency correct

    ICR1 = 0x00FF; // set interrupt
    DDRC |= (1 << PINC2) | (1 << PINC3);
}

/*ISR(USART_RXC_vect) {
    rijden(UDR);
}*/

