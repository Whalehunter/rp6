/*
 * bumpers.c
 *
 * Created: 16-3-2019 23:54:01
 * Author : Björn van Lieshout
 code voor gebruik bumpers
 */ 

#include <avr/io.h>
#include "functiesBjorn.h"



int bumperLinks();
{
/*
LED SL6 is portb
Bumpers links zijn verbonden aan: PORTB, PINB, DDRB, PINB0
//Situatie: Wanneer de bumper wordt ingedrukt moet hij automatisch achteruit rijden. 
led sl6 en sl3 zijn statusleds voor de bumpers. Wanneer deze 1 zijn moet de robot achteruit rijden 
*/	
	PORTB &= ~(1 << PINB0);
	DDRB &= ~(1 << PINB0);
	
	uint8_t triggerLinks = PINB & PINB0;
	
		if(	PORTB &= (1 << PINB0) &&  DDRB &= ~(1 << PINB0)) 
		{
		PORTB &= (1 << PINB0);
		DDRB &= (1 << PINB0);
		}
		return triggerLinks;


	
}
	int bumperRechts();
{
/*
LED SL3 is portc
zelfde comment als bij bumperLinks
*/	
PORTC &= ~(1 << PINC6);
DDRC &= ~(1 << PINC6);

uint8_t triggerRechts = PINC & PINC6;

if(	PORTC &= (1 << PINC6) &&  DDRC &= ~(1 << PINC6))
{
PORTC &= ~(1 << PINC6);
DDRC &= ~(1 << PINC6);
}
return triggerLinks;


		
}
	
	
int returnBumperTrigger();
{

		return bumperLinks() || bumperRechts();
    
}

