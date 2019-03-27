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

void pieper_aan();
void pieper_uit();

int main(void)
{
	sei();
	pieper_aan();
	
	while (1)
	{
		
	}
}

void pieper_aan(){
	DDRB = 0xFF; // PORTB op output instellen, PB4 (pin10) is een OCRA pin
	
	//Instellen van de timer2 voor de frequentie/toonhoogte:
	TCCR2A = (1<<COM2A0)| (1<<WGM21); //set oc2a on compare match | WGM on ctc mode
	TCCR2B = (1<<CS22) | (1<<CS21) | (1<<CS20); // Stelt de prescaler in op 1024. Dan moet de OCA op 15.
	OCR2A = 8; // OUtput compare ingesteld op
	
	// Instellen van de 16bit timer3
	TCCR3A = (1<<COM3A1) | (1<<COM3A0) ;
	TCCR3B = (1<<CS32) | (1<<WGM32); // Prescaler op 256
	OCR3A = 31250; // Output compare on 31250 (500ms)
	TIMSK3 = (1<<OCIE3A);
}

void pieper_uit(){
	TCCR2B = (1<<CS00); // uitzetten prescalers en dus timers
	TCCR3B= (1<<CS00);
}

ISR(TIMER3_COMPA_vect){
		static bool aan = true;
		static bool temp = true;
	
	if (aan&&temp){
		TCCR2B = (1<<CS00);
		aan=!aan;
	}
	if (!aan&&!temp){
		TCCR2B = (1<<CS22) | (1<<CS21) | (1<<CS20); 
		aan=!aan;
	}
	temp=!temp;
}