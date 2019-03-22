#define F_CPU 8000000

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <util/delay.h>


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



int main (void){
	// Initialisers
	cli();
	init();
	init_usart();
	sei();
	
	
	while(1)
	{
		rijden();
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

void rijden(){
	uint8_t command = leescommand();
	if (command == '1')
	{
		snelheid = 50;
	}
	if (command == '2')
	{
		snelheid = 125;
	}
	if (command == '3')
	{
		snelheid = 150;
	}
	
	
	if(command == 'w') // ga VOORUIT als invoer w is
	{
		PORTC &= (1 << PINC2);
		PORTC &= (1 << PINC3);  // |= is achteruit op beide, &= is vooruit
		OCR1A = snelheid;//dit zet de motoren aan
		OCR1B  = snelheid;
	}
	
	if (command == 'a') // ga LINKSAF als invoer a is
	{
		PORTC |= (1 << PINC2);   //Defined dat hij achteruit gaat
		PORTC &= ~(1 << PINC3);  // |= is achteruit op beide, &= is vooruit
		OCR1A = snelheid;//dit zet de motoren aan
		OCR1B  = snelheid;
	}
	if (command == 's') // ga ACHTERUIT als invoer s is
	{
		PORTC |= (1 << PINC2);   //Defined dat hij achteruit gaat
		PORTC |= (1 << PINC3);  // |= is achteruit op beide, &= is vooruit
		OCR1A = snelheid;//dit zet de motoren aan
		OCR1B  = snelheid;
	}
	
	if (command == 'd') // ga RECHTSAF als invoer d is
	{
		PORTC &= ~(1 << PINC2);   //Defined dat hij achteruit gaat
		PORTC |= (1 << PINC3);  // |= is achteruit op beide, &= is vooruit
		OCR1A = 0x50;//dit zet de motoren aan
		OCR1B  = 0x50;
	}
	if (command == 'q') // q for quit
	{
		OCR1B = 0x00; // Uitzetten van de motoren
		OCR1A = 0x00;
	}
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
