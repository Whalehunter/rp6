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
#include <avr/interrupt.h>

/*#define SL1 (1<<PINC6) // Red Right Led1
#define SL2 (1<<PINC5) // Red Right led2
#define SL3 (1<<PINC4) // Green Right Led3
#define SL4 (1<<PINB7) // Green Left Led1
#define SL5 (1<<PINB1) // Red Left Led2
#define SL6 (1<<PINB0) // Red Left Led3
*/
uint64_t timert(int x);
void toggle_links();
void toggle_rechts();


int main(void)
{
	sei();
	DDRB |= 0b10000000; // Stel pb7 in als output
	DDRC |= 0b00010000; // Stel pc4 in als output
	
	// Instellen van Timer compare registers 
	TIMSK = (1<<TOIE0); // Timer overflow interrupt bitje
	TCCR0 |= (1<<COM00); // Timer control register COM00 - toggle oc0a on compare match
	TCCR0 = (1<<CS02) | (1<<CS00); // Stelt de prescaler in op 1024. Dan moet de OCA op 78.
	OCR0 = 78; // OUtput compare ingesteld op 78 (80000/1024)
	
	
	while (1) 
    {
		if (timert(0)%1000==0){
			toggle_links();	
			toggle_rechts();
		}
		/*if (timert(0)%100==0){
			PORTB &= ~(1<<PINB7);
			PORTC &= ~(1<<PINC4); 
		}*/
	}
}


void toggle_links(){
		PORTB ^= (1<<PINB7);
}

void toggle_rechts(){
		PORTC ^= (1<<PINC4);
}


uint64_t timert(int x)
{
	static uint64_t i =0;
	if (x){
		return i++;
	}
	return i;
	
}

ISR(TIMER0_OVF_vect) // Interrupt Service Routine 
{
	timert(1);
}