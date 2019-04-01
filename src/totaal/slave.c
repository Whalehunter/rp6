#define F_CPU 8000000
#ifndef __AVR_ATmega32__
#define __AVR_ATmega32__
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include "rp6.h"

#define SLA 56

void i2c_init();
void init_motors();
void init_usart();
void drive(char x);
uint8_t timert(int x);
void init_leds();
void i2c_send(char data);

static RP6_Full rp6 = {{0,0,0},	/* Speed */
		       'w',	/* Direction */
		       0,
		       {0,0}};	/* Updates */

int main(void){
	cli();			/* Disable global interrupts */
        init_motors();
        init_leds();
        i2c_init();
        sei();			/* Enable global interrupts */

        while(1) {
		if (rp6.update.speed) {
			RP6_SetCurrentSpeed(&rp6);
		}
		RP6_Execute(&rp6);
	}
        return 0;
}

ISR(TWI_vect) {

        switch(TWSR) {
        case 0x80: // Addressed with own SLA+W; data received; ACK send
                drive(TWDR);
		break;
        case 0xA8: // Own SLA+R has been received; ACK has been returned
        case 0xB8: // Data byte in TWDR has been send; ACK has been received
		i2c_send(RP6_GetDirection(&rp6));
		break;
        case 0xC0: // Data byte in TWDR has been send; NACK has been received
		drive('0');
		break;
        }
        TWCR |= (1<<TWINT); // Clear TWINT Flag
}

void i2c_init() {
        TWSR = 0;
        TWAR = (SLA<<1); // move address into Address Register
        TWCR = (1<<TWEA) | (1<<TWIE) | (1<<TWEN);
}

void i2c_send(char data) {
	TWDR = data;
}

void init_usart(){
        UCSRA = 0; // USART status and control registerA uit
        UCSRB = (1 << RXEN) | (1 << RXCIE); // Enable de USART Reciever
        UCSRC = (1 << UCSZ1) | (1 << UCSZ0); /* 8 data bits, 1 stop bit */
        UBRRH = 00;
        UBRRL = 12; //baudrade 38.4K. voor robotloader
}

void drive(char x) {
	switch(x) {
	case 'w':
	case 'a':
	case 's':
	case 'd':
		RP6_SetDirection(&rp6, x);
		break;
	case '0':
	case '1':
	case '2':
	case '3':
		RP6_SetSpeed(&rp6, x);
		break;
	}
}

void init_motors() {
        TCNT1 = 0;

        TCCR1A |= (1 << COM1A1)|(1 << COM1B1)|(1 << WGM11); // set output to high on compare + pwm 9-bit
        TCCR1B |= (1 << WGM13)|(1 << CS10); // no prescaler + pwm, phase & frequency correct

        ICR1 = 0x00FF; // set interrupt
        DDRC |= (1 << PINC2) | (1 << PINC3);
}

void init_leds(){
        DDRB |= (1 << PIN7); /* PB7 als OUTPUT */
        DDRC |= (1 << PINC4); /* PC4 als OUTPUT */
        TIMSK = (1 << OCIE0); /* Timer overflow ENABLE */
        TCCR0 |= (1 << COM00) | (1<<WGM01); // CTC ENABLE
        TCCR0 |= (1<<CS02) | (1<<CS00); // Prescaler: 1024
        OCR0 = 77; // Output compare: 77 (80000/1024)
}

ISR(TIMER0_COMP_vect) // Interrupt Service Routine
{
	rp6.blinkerCount++;
}
