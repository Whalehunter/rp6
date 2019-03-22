


#define F_CPU 8000000

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#define SLA 56


//TERMINAL COMMUNICATIE
#define BAUD_LOW 38400
#define UBRR_BAUD_LOW ((F_CPU/(16*BAUD_LOW))-1)
#define BAUD_HIGH 500000
#define UBRR_BAUD_HIGH ((F_CPU/(16*BAUD_HIGH))-1)

//TERMINAL COMMUNICATIE
#define HEX 16
#define DEC 10
#define OCT 8
#define BIN 2

// Declarations of functions
int timer_counter(uint64_t inc);
void init();
void goLinks();
void goRechts();
void goAchteruit();
void goVooruit();
int bumpers();
int  BumperStatusLinks();
int BumperStatusRechts();
void init_usart();
int leescommand();
void rijden();


int snelheid;
char x, y;



int main (void){
	// Initialisers
	cli();
	init();
	init_usart();
	sei();
	i2c_slave_init();

	while(1)
	{
		OCR1B = 0x00; // Uitzetten van de motoren
		OCR1A = 0x00;
	}
	
	return 0;
}


// Functions used:
void init_usart(){
	UCSRA = 0; // USART status and control registerA uit
	UCSRB = (1 << RXEN); // Enable de USART Reciever
	UCSRC = (1 << UCSZ1) | (1 << UCSZ0); /* 8 data bits, 1 stop bit */
	UBRRH = 00;
	UBRRL = 12; //baudrade 38.4K. voor robotloader
}


int leescommand(){
	int getal;
	while(~UCSRA & (1<<RXC));
	getal = UDR;
	return getal;
}

void vooruit(){
	PORTC &= (1 << PINC2);
	PORTC &= (1 << PINC3);  // |= is achteruit op beide, &= is vooruit
	OCR1A = 0x50;//dit zet de motoren aan
	OCR1B  = 0x50;
}

void links(){
	PORTC |= (1 << PINC2);   //Defined dat hij achteruit gaat
	PORTC &= ~(1 << PINC3);  // |= is achteruit op beide, &= is vooruit
	OCR1A = 0x50;//dit zet de motoren aan
	OCR1B  = 0x50;
}

void rechts(){
	PORTC &= ~(1 << PINC2);   //Defined dat hij achteruit gaat
	PORTC |= (1 << PINC3);  // |= is achteruit op beide, &= is vooruit
	OCR1A = 0x50;//dit zet de motoren aan
	OCR1B  = 0x50;
}

void achteruit(){
	PORTC |= (1 << PINC2);   //Defined dat hij achteruit gaat
	PORTC |= (1 << PINC3);  // |= is achteruit op beide, &= is vooruit
	OCR1A = 0x50;//dit zet de motoren aan
	OCR1B  = 0x50;
}


int timer_counter(uint64_t inc) {
	static uint64_t i = 1;
	if (inc)
	return i++;
	return i;
}


void init()
{
	sei();
	TCNT1 = 0x00;
	TCCR0 |= (1 << CS00)|(1 << CS01);

	TCCR1A |= (1 << COM1A1)|(1 << COM1B1)|(1 << WGM11);
	TCCR1B |= (1 << WGM13)|(1 << CS10);

	ICR1 = 0x00FF;
	TIMSK |= (1 << TOIE0);
	DDRC |= (1 << PINC2) | (1 << PINC3);
}



ISR(TIMER1_COMPA_vect)
{
	timer_counter(1);
}
ISR(TIMER1_COMPB_vect)
{
	timer_counter(1);
}


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
		TWDR = '<'; links(); break;
		case 'd':
		TWDR = '>'; rechts(); break;
		case 'w':
		TWDR = '^'; vooruit(); break;
		case 's':
		TWDR = '.'; achteruit(); break;
		default: break;
	}
}