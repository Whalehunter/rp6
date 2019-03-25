#define F_CPU 8000000

#include <avr/io.h>
#include <avr/interrupt.h>

// Declarations of functions
void init();
void init_usart();
int leescommand();
void rijden(char x);
//int leessnelheid();



int main (void){
    // Initialisers
    cli();
    init();
    init_usart();
    sei();


    while(1)
        {
            /* rijdex(); */
        }
    return 0;
}


// Functions used:
void init_usart(){
    UCSRA = 0; // USART status and control registerA uit
    UCSRB = (1 << RXEN) | (1 << RXCIE); // Enable de USART Reciever
    UCSRC = (1 << UCSZ1) | (1 << UCSZ0); /* 8 data bits, 1 stop bit */
    UBRRH = 00;
    UBRRL = 12; //baudrade 38.4K. voor robotloader
}


int leescommand(){
    while(~UCSRA & (1<<RXC));
    return UDR;
}

void rijden (char x) {

    switch(x) {
    case 'w':
        PORTC &= ~(1 << PINC2);
        PORTC &= ~(1 << PINC3);
        break;
    case 'a':
        PORTC |= (1 << PINC2);
        PORTC &= ~(1 << PINC3);
        break;
    case 's':
        PORTC |= (1 << PINC2);
        PORTC |= (1 << PINC3);
        break;
    case 'd':
        PORTC &= ~(1 << PINC2);
        PORTC |= (1 << PINC3);
        break;
    case '1':
        OCR1A = 50;
        OCR1B = 50;
        break;
    case '2':
        OCR1A = 125;
        OCR1B = 125;
        break;
    case '3':
        OCR1A = 150;
        OCR1B = 150;
        break;
    case 'q':
        OCR1A = 0;
        OCR1B = 0;
    }
}

/*
  void rijden(){
  char command = leescommand();
  //uint8_t commands = leessnelheid();
  static int snelheid = 100;

  if (command == '1')
  {
  snelheid = 50;
  }
  if (command == '2')
  {
  snelheid = 125;
  }
  if (command == '3')
  {
  snelheid = 150;
  }


  if(command == 'w') // ga VOORUIT als invoer w is
  {
  PORTC &= (1 << PINC2);
  PORTC &= (1 << PINC3);  // |= is achteruit op beide, &= is vooruit
  OCR1A = snelheid;//dit zet de motoren aan
  OCR1B  = snelheid;
  }

  if (command == 'a') // ga LINKSAF als invoer a is
  {
  PORTC |= (1 << PINC2);   //Defined dat hij achteruit gaat
  PORTC &= ~(1 << PINC3);  // |= is achteruit op beide, &= is vooruit
  OCR1A = snelheid;//dit zet de motoren aan
  OCR1B  = snelheid;
  }
  if (command == 's') // ga ACHTERUIT als invoer s is
  {
  PORTC |= (1 << PINC2);   //Defined dat hij achteruit gaat
  PORTC |= (1 << PINC3);  // |= is achteruit op beide, &= is vooruit
  OCR1A = snelheid;//dit zet de motoren aan
  OCR1B  = snelheid;
  }

  if (command == 'd') // ga RECHTSAF als invoer d is
  {
  PORTC &= ~(1 << PINC2);   //Defined dat hij achteruit gaat
  PORTC |= (1 << PINC3);  // |= is achteruit op beide, &= is vooruit
  OCR1A = snelheid;//dit zet de motoren aan
  OCR1B  = snelheid;
  }
  if (command == 'q') // q for quit
  {
  OCR1B = 0x00; // Uitzetten van de motoren
  OCR1A = 0x00;
  }
  }
*/
void init()
{
    TCNT1 = 0;

    TCCR1A |= (1 << COM1A1)|(1 << COM1B1)|(1 << WGM11); // set output to high on compare + pwm 9-bit
    TCCR1B |= (1 << WGM13)|(1 << CS10); // no prescaler + pwm, phase & frequency correct

    ICR1 = 0x00FF; // set interrupt
    DDRC |= (1 << PINC2) | (1 << PINC3);
}

ISR(USART_RXC_vect) {
    rijden(UDR);
}
