/*
 * Beeper_atmega.c
 *
 * Created: 21-3-2019 16:31:02
 * Author : mgjho
 */ 

#define F_CPU 16000000
#define BAUD 9600
#define BRC ((F_CPU/16/baud) -1 )

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define DELAYWAARDE2 1000

void init_pieper();
uint8_t timert2(int x);
void frequentie();

int main(void)
{
	init_pieper();

	
	while (1)
	{
		
	}
}

void init_pieper(){
	DDRL = 0xFF;
	//Instellen van de timer voor de frequentie/toonhoogte:
	TIMSK2 = (1<<TOIE2); // Timer overflow interrupt bitje
	TIFR2 = (1<<OCF2A);
	TCCR2A |= (1<<COM2A1) | (1<<COM2A0); // Timer control register COM00 - toggle oc0a on compare match | WGM on ctc mode
	TCCR2A |= (1<<CS22) | (1<<CS21) | (1<<CS20); // Stelt de prescaler in op 1024. Dan moet de OCA op 77.
	OCR2A = 15; // OUtput compare ingesteld op 77 (80000/1024)
}

uint8_t timert2(int x)
{
	static uint8_t delay = 0;

	if (x) {
		delay++;
		} else if (delay >= DELAYWAARDE2 ) {
		delay = 0;
		return 1;
	}
	return 0;
}

ISR(TIMER2_COMPA_vect) // Interrupt Service Routine
{
	//timert2(1);
	frequentie();
}

void frequentie(){
	PORTL ^= (1<<PINL2);
}