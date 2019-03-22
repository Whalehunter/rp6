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
uint8_t timert(int x);
void toggle_links();
void toggle_rechts();
void knipper_licht_uit();

#define DELAYWAARDE 20

int main(void)
{
    sei();
    DDRB = 0b10000000; // Stel pb7 in als output
    DDRC = 0b00010000; // Stel pc4 in als output

    // Instellen van Timer compare registers
    TIMSK = (1<<OCIE0); // Timer overflow interrupt bitje
    TCCR0 = (1<<COM00) | (1<<WGM01); // Timer control register COM00 - toggle oc0a on compare match | WGM on ctc mode
    TCCR0 |= (1<<CS02) | (1<<CS00); // Stelt de prescaler in op 1024. Dan moet de OCA op 77.
    OCR0 = 77; // OUtput compare ingesteld op 77 (80000/1024)
    knipper_licht_uit();

    while (1) {

        toggle_rechts();
    }
    return 0;
}

void toggle_links(){
    if (timert(0)){
        PORTB ^= 0b10000000;
    }
}

void toggle_rechts(){
    if (timert(0)){
        PORTC ^= 0b00010000;
    }

    /*static int aan = 1;
      static int temp =1;

      if (timert(0) && aan && temp){
      PORTC |= (1<<PINC4);
      aan =0;
      }
      if (timert(0) && !aan && !temp){
      PORTC &= ~(1<<PINC4);
      aan =1;
      }
      temp = !temp;*/
}

void knipper_licht_uit() {
    /* Links uit */
    PORTB &= ~(1<<PINB7);
    /* Rechts uit */
    PORTC &= ~(1<<PINC4);
}

uint8_t timert(int x)
{
    static uint8_t delay = 0;

    if (x) {
        delay++;
    } else if (delay >= DELAYWAARDE) {
        delay = 0;
        return 1;
    }

    return 0;
}

ISR(TIMER0_COMP_vect) // Interrupt Service Routine
{
    timert(1);
}
