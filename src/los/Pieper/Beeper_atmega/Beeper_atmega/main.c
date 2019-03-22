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

void init_pieper();

int main(void)
{
	init_pieper();
	
	while (1)
	{
		/*for (int i=0;i<250;i++){
			PORTL |= (1<<PINL2);
			_delay_us(2000);
			PORTL &= ~(1<<PINL2);
			_delay_us(2000);
		}
		for (int i=0;i<300;i++){
			PORTL |= (1<<PINL2);
			_delay_us(1500);
			PORTL &= ~(1<<PINL2);
			_delay_us(1500);
		}*/

		
	}
}

void init_pieper(){
	DDRL = 0xFF;
	
}