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
		if (command==117||command==87){ // als de invoer gelijk is aan w of W
			PORTB |= (1<<PINB0); // Zet de bovenste led aan
		}
		if (command==68 || command==100 ){
			PORTB |= (1<<PINB1 );
		}
		if (command == 83 || command == 115){
			PORTB |= (1<<PINB2);
		}
		if (command == 65 || command == 97){
			PORTB |= (1<<PINB3);
		}
		PORTB |=(1<<PINB0);
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
	
	
	/* while(1){
		/* Wacht totdat de Usart Recieve Complete bit (in het USCR0A register) laag is.
		Dit gebeurt wanneer de ongelezen data in de UDR0 gelezen is. Wanneer er ongelezen data in zit gaat de bit omhoog en zal hij verder gaan naar buffer
		Als deze data wordt gelezen gaat de bit weer omlaag en blijft deze laag tot er nieuwe data is.
		*///while(~UCSR0A & (1<<RXC0)); 
		/*buffer[i] = UDR0;
		
		buffer[i+1] = '\0'; // 1 plek na het laatst ingelezen char wordt gevuld met '\0' zodat de string goed afgesloten is
		break;
	}
	getal = atoi(buffer); // Ascii to integer conversie (van de buffer)
	return getal;*/
}