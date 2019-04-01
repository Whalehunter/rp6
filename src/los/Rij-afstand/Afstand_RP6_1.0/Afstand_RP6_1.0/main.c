/*
 * Afstand_RP6_1.0.c
 *
 * Created: 29-3-2019 16:06:41
 * Author : mgjho
 */ 
 

#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>

#define F_CPU 8000000
#define BAUD 9600


#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>

void init_ENC_interrupt();
void initUSART();
void init();
void setDistance();
void writeChar(char c);
void writeInt(int i);
void writeString(char st[]);

int main (void)
{
	init_ENC_interrupt();
	init();
	initUSART();
	sei();
	PORTC &= ~(1 << PINC2);
	PORTC &= ~(1 << PINC3);
	OCR1A = 50;
	OCR1B = 50;

	while(1)
	{
		
	}
	return 0;
}


void init_ENC_interrupt(){
	PORTB |= (1<<PINB4);
	DDRD &= ~((1<<PD2) | (1<<PD3)); // Instellen van PD2 en PD3
	PORTD |= (1<<PD2) |(1<<PD3);
	MCUCR |= (1<<ISC10) | (1<<ISC00);
	GICR |= (1<<INT0) | (1<<INT1);  // Interrupt
}


void setDistance(uint16_t links, uint16_t rechts){
	static long long int edges_links = 0;
	static long long int edges_rechts = 0;

	if (links != 0)
	{
		edges_links = links;
	}
	if (rechts != 0)
	{
		edges_rechts = rechts;
	}
	long long int avg_edges = (edges_links+edges_rechts)/2;
	int distance = avg_edges * 0.025;
	static long long int last_distance = 0;
	if (last_distance != distance){
		writeInt(distance);
		last_distance = distance;
	}
	
}

ISR(INT0_vect)
{
	static long long int edges_links = 0;
	edges_links++;
	setDistance(edges_links, 0);
}

ISR(INT1_vect)
{
	static long long int edges_rechts = 0;
	edges_rechts++;
	setDistance(0 , edges_rechts);
}

void writeChar(char c) {
	while(~UCSRA & (1 << UDRE));
	UDR = c;
}

void writeString(char st[]) {
	for(uint8_t i = 0 ; st[i] != 0 ; i++) {
		writeChar(st[i]);
	}
}

void writeInt(volatile int i) {
	char buffer[10000];
	itoa(i,buffer,10);
	writeString(buffer);
	writeChar('\n');
}

void init()
{
	TCNT1 = 0;

	TCCR1A |= (1 << COM1A1)|(1 << COM1B1)|(1 << WGM11); // set output to high on compare + pwm 9-bit
	TCCR1B |= (1 << WGM13)|(1 << CS10); // no prescaler + pwm, phase & frequency correct

	ICR1 = 0x00FF; // set interrupt
	DDRC |= (1 << PINC2) | (1 << PINC3);
}

void initUSART() {
	UCSRA = 0;
	UCSRB = (1 << TXEN); // | (1 << RXEN) | (1 << RXCIE); // enable USART Transmitter
	UCSRC = (1 << UCSZ1) | (1 << UCSZ0); // 8 data bits, 1 stop bit
	UBRRH = 00;
	UBRRL = 12; // baudrate 38.4 k
}