#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 8000000

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

int timer_counter(uint64_t inc) {
	static uint64_t i = 1;
	if (inc)
	return i++;
	return i;
}

void cycleDelay(volatile unsigned long cd){
	while(cd--);
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
	
	int i = 10;
	
	while (1)
	{
		if(i == 20) 
		{
				PORTC &= (1 << PINC2);   //Defined of hij achteruit gaat
				PORTC &= (1 << PINC3);  // |= is achteruit op beide, &= is vooruit
				OCR1A = 0x50;//dit zet de motoren aan
				OCR1B  = 0x50;
		}
		if (i == 10)
		{
			PORTC |= (1 << PINC2);   //Defined dat hij achteruit gaat
			PORTC &= ~(1 << PINC3);  // |= is achteruit op beide, &= is vooruit
			OCR1A = 0x50;//dit zet de motoren aan
			OCR1B  = 0x50;
		}
		
	}
	
		
	/*
	PORTC &= (1 << PINC2);   //Defined of hij achteruit gaat
	PORTC &= (1 << PINC3);  // |= is achteruit op beide, &= is vooruit
	OCR1A = 0x50;//dit zet de motoren aan
	OCR1B  = 0x50;
    */
	
	
	/* Snelheden:
	0xB4 = 180, TOPSNELHEID
	0X7D = 125, GEMIDDELDE SNELHEID
	0x0050 = 50, LANGZAAM
	*/	
}


void goLinks()
{
	DDRC |= (1 << PINC2) | (1 << PINC3);
	PORTC  &= (1 << PINC3);
	PORTC &= (1 << PINC2);
	

	PORTC &= (1 << PINC2);   //Defined of hij achteruit gaat
	PORTC &= (1 << PINC3);  // |= is achteruit op beide, &= is vooruit
	OCR1A = 0x7D;//dit zet de motoren aan met de snelheid
	OCR1B  = 0x50;
	
}
void goRechts()
{
		
	PORTC &= (1 << PINC2);   //Defined of hij achteruit gaat
	PORTC &= (1 << PINC3);  // |= is achteruit op beide, &= is vooruit
	OCR1A = 0x50;//dit zet de motoren aan
	OCR1B  = 0x7D;
}
void goAchteruit()
{
	PORTC |= (1 << PINC2);   //Defined of hij achteruit gaat
	PORTC |= (1 << PINC3);  // |= is achteruit op beide, &= is vooruit
	OCR1A = 0x50;//dit zet de motoren aan
	OCR1B  = 0x50;
}
void goVooruit()
{
	PORTC &= (1 << PINC2);   //Defined of hij achteruit gaat
	PORTC &= (1 << PINC3);  // |= is achteruit op beide, &= is vooruit
	OCR1A = 0x50;//dit zet de motoren aan
	OCR1B  = 0x50;
}


ISR(TIMER1_COMPA_vect)
{
timer_counter(1);
}
ISR(TIMER1_COMPB_vect)
{
timer_counter(1);
}

/*
void MotorSturen()
{
	DDRC |= (1 << PINC2) | (1 << PINC3);
	PORTC  &= (1 << PINC3);
	PORTC &= (1 << PINC2);
	
	DDRD |= 0xff;
	DDRC = 0xff;
}
*/

int BumperStatusLinks()
{
	int DDRBNu = DDRB;
	int PORTBNu= PORTB;
	DDRB &= (1 << PINB0);
	PORTB &= (1 << PINB0);
	
    cycleDelay(10);
    asm("nop");
	int BumperTriggerLinks = PINB & (1 << PINB0);

	DDRB = DDRBNu;
	PORTB = PORTBNu;
	
	if (BumperTriggerLinks)
	{
		return 1;
	}
	return 0;
}

int BumperStatusRechts()
{
	int DDRCNu = DDRC;
	int PORTCNu = PORTC;
	DDRC &= ~(1 << PINC6);
	PORTC &= ~(1 << PINC6);
	cycleDelay(10);
	asm("nop");
    
	int BumperTriggerRechts = PINC & (1 << PINC6);

	DDRC = DDRCNu;
	PORTC = PORTCNu;
	if (BumperTriggerRechts)
	{
		return 1;
	}
	return 0;
}

void writeChar(char a)
{
	while (!(UCSRA & (1 << UDRE)));
	UDR = (uint8_t) a;
}

void writeString(char * string)
{
	while ( * string)
	writeChar( * string++);
}

int bumpers (void)
{
		while (1)
	{
		cycleDelay(100);
		if (BumperStatusLinks())
		{
				writeString("Linker  bumper getriggerd. Ga achteruit!\n");
		}
		else if (BumperStatusRechts())
		{
		    	writeString("Rechter\n");
		}
	}
	return 0;
}

int main()
{
	cli();
	init();
    bumpers();
	//goLinks();
	

	
	sei();
}