#include <avr/io.h>
#include <avr/interrupt.h>
#include "../../includes/rp6aansluitingen.h"
/*
 * Rijden - tst 2
 */

uint16_t mleft_speed;
uint16_t mright_speed;

void speed() {

}

int main(void) {
    /* Init ports */
    DDRA = 0x00;
    PORTA = 0x00;

    DDRB = 0x54;
    PORTB = 0x00;

    DDRC = 0x46;
    PORTC = 0xAC;

    DDRD = 0xF2;
    PORTD = 0x01;

    /* Disable global interrupts */
    cli();

    /* Enable reset button */
    PORTB &= ~(1 << PINB5);
    DDRB |= (1 << PINB5);

    /* Init external interrupts */
    MCUCR = (0 << ISC11) | (1 << ISC10) | (0 << ISC01) | (1 << ISC00);
    GICR = (1 << INT2) | (1 << INT1) | (1 << INT0);
    MCUCSR = (0 << ISC2);


    /* Initialsize timer 0 - 100 microseconds for Delays/Stopwatches */
    TCCR0 =   (0 << WGM00) | (1 << WGM01)
        | (0 << COM00) | (0 << COM01)
        | (0 << CS02)  | (1 << CS01) | (0 << CS00);
    OCR0  = 99;

    /* Init timer1 - PWM */
    TCCR1A = (0 << WGM10) | (1 << WGM11) | (1 << COM1A1) | (1 << COM1B1);
    TCCR1B = (1 << WGM13) | (0 << WGM12) | (1 << CS10);
    ICR1 = 210;
    OCR1AL = 0;
    OCR1BL = 0;

    /* Motor forward */
    PORTC &= ~(1 << PINC2);
    PORTC &= ~(1 << PINC3);

    /* Init timer interrupts */
    TIMSK = (1 << OCIE0);

    /* Enable global interrupts */
    sei();

    /* Power ON */
    PORTB |= (1 << PINB4);

    /* Set speed */
    speed(40, 20);
}
