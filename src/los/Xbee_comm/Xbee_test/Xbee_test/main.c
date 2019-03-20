/*
 * Xbee_test.c
 * This program tests the xbee module on the arduino 2560.
 * It will be used to send wireless commands from the laptop to the arduino
 * which will trigger certain leds. The triggering of these leds can be
 * replaced by functions that trigger the motor or other functions in a later fase.
 * Created: 18-3-2019 11:57:29
 * Author : mgjho
 */
#define F_CPU 16000000
#define BAUD 9600
#define BRC ((F_CPU/16/baud) -1 )
 
// Inclusions:
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <util/delay.h>



//Functions in the program:
void init_usart();
void init_hardware();
int leescommand();

int main(void)
{
   init_usart();
   init_hardware();
   
   int command;
	
    while (1) 
    {
		command = leescommand();
		if (command==119 || command==87){ // als de invoer gelijk is aan E of e
			PORTB ^= (1<<PINB0);			// Zet de bovenste led aan
		}
		if (command==68 || command==100 ){	// Wanneer invoer gelijk is aan ASCII d of D
			PORTB ^= (1<<PINB1 );			// Zet meest oostelijke led aan
		}
		if (command == 83 || command == 115){	// Wanneer invoer gelijk is aan ASCII S of s
			PORTB ^= (1<<PINB2);				// Zet de onderste led aan
		}
		if (command == 65 || command == 97){	// Wanneer invoer gelijk is aan ASCII A of a
			PORTB ^= (1<<PINB3);				// Zet westelijke led aan
		}
    }
}

void init_usart(){
	 UCSR0A = 0; // USART status and control register0A uit
	 UCSR0B = (1 << RXEN0); // Enable de USART Reciever
	 UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); /* 8 data bits, 1 stop bit */
	 UBRR0H = 00;
	 UBRR0L = 103; //baudrade 9600 bij
}

void init_hardware(){
	DDRB = 0b00001111; // Portb Pinb 50/51/52/53 set on output (leds)
}

int leescommand(){
	int getal;
	while(~UCSR0A & (1<<RXC0)); 
	getal = UDR0;
	return getal;
}