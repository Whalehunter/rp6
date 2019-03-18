/*
 * Ledtests_rp6.c
 *
 * Created: 11-3-2019 14:50:12
 * Author : mgjho
 */ 
#define F_CPU 8000000
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>

/*#define SL1 (1<<PINC6) // Red Right Led1
#define SL2 (1<<PINC5) // Red Right led2
#define SL3 (1<<PINC4) // Green Right Led3
#define SL4 (1<<PINB7) // Green Left Led1
#define SL5 (1<<PINB1) // Red Left Led2
#define SL6 (1<<PINB0) // Red Left Led3
*/



int main(void)
{
	PORTB |= 0b10000000; // Stel pb7 in als output
	PORTC |= 0b00010000; // Stel pc4 in als output
	
	// Instellen van Timer compare registers
	WGM01 = 0; // Stel waveform generation moden op normal mode     
	TIMSK = (1<<TOIE0); // Timer overflow interrupt bitje
	TCCR0 |= (1<<COM00); // Timer control register COM00 - toggle oc0a on compare match
	TCCR0 = (1<<CS02) | (1<<CS00); // Stelt de prescaler in op 1024. Dan moet de OCA op 78.
	OCR0 = 78; // OUtput compare ingesteld op 78 (80000/1024)
	
	
	while (1) 
    {
		ledknipper_links();
		_delay_ms(1000);
		ledknipper_rechts();
		_delay_ms(1000);
    }
}


void ledknipper_links(){
	for (int i=0; i<8; i++){
		PORTB ^= (1<<PINB7);
		TIMER0_OVF_vect
	}
	PORTB &= ~(1<<PINB7);
}

void ledknipper_rechts(){
	for (int i=0; i<8;i++){
		PORTC ^= (1<<PINC4);
		TIMER0_OVF_vect
	}
	PORTC &= ~(1<<PINC4); 
}


void timer(int i)
{
	TIMSK = (1<<TOIE0); // Timer overflow interrupt bitje
	TCCR0 |= (1<<COM00); // Timer control register COM00 - toggle oc0a on compare match
	
}

ISR(TIMER0_OVF_vect) // Interrupt Service Routine 
{
	
	
}