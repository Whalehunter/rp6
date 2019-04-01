#define F_CPU 8000000
#ifndef __AVR_ATmega32__
#define __AVR_ATmega32__
#endif

#include <avr/io.h>
#include <avr/interrupt.h>

#define SLA 56

void i2c_init();
void init_motors();
void init_usart();
void drive(char x);
uint8_t timert(int x);
void init_leds();
void i2c_send(char data);

/* If a setting needs updating in while loop *********************************/
typedef struct {
	int speed;
	int dir;
} RP6_Update;

/* Speed settings RP6 ********************************************************/
typedef struct {
	int cur;
	int prev;
	int next;
} RP6_Speed;

/* Full settings RP6 *********************************************************/
typedef struct {
	RP6_Speed speed;
	char dir;
	int blinkerCount;
	RP6_Update update;
} RP6_Full;

void RP6_SetBlinker(RP6_Full * RP6); /* Update RP6 knipperlicht */
void RP6_SetSpeed(RP6_Full * RP6, char speed); /* Update RP6 speed geleidelijk */
void RP6_SetDirection(RP6_Full * RP6, char direction); /* Update RP6 richting */
char RP6_GetDirection(RP6_Full * RP6);		       /* Get current direction of RP6 */
void RP6_SetCurrentSpeed(RP6_Full * RP6);	 /* Set acceleration for next item */

/*****************************************************************************/
/*      De "Execute" functies zijn in princepe alleen voor RP6_Execute.      */
/*****************************************************************************/
void RP6_Execute(RP6_Full * RP6);		 /* Execute RP6 settings */
void RP6_Execute_Direction(RP6_Full * RP6);	 /* Execute drive direction */
void RP6_Execute_Blinker(RP6_Full * RP6);	 /* Execute blinker toggles */
void RP6_Execute_Speed(RP6_Full * RP6); /* Execute RP6 speed */

void RP6_SetBlinker(RP6_Full * RP6) {
        RP6->blinkerCount = 20;
}

void RP6_SetSpeed(RP6_Full * RP6, char speed) {
        int newSpeed = 50 * (int)speed;
        if (RP6->speed.next != newSpeed) {
                RP6->speed.next = newSpeed;
                RP6->update.speed = 1;
        }
}

void RP6_SetDirection(RP6_Full * RP6, char direction) {
        if (direction != RP6->dir) {
                RP6->dir = direction;
                RP6->update.dir = 1;
                RP6_SetBlinker(RP6);
        }
}

char RP6_GetDirection(RP6_Full * RP6) {
        switch(RP6->dir) {
        case 'w': return '^';
        case 'a': return '<';
        case 's': return '.';
        case 'd': return '>';
        default: return '?';
        }
}

void RP6_SetCurrentSpeed(RP6_Full * RP6) {
        int inc = 1;
        if (RP6->speed.cur > RP6->speed.next) {
                inc = -1;
        }

        RP6->speed.cur += inc;

        if (RP6->speed.cur == RP6->speed.next) {
                RP6->update.speed = 0;
        }
}

void RP6_Execute(RP6_Full * RP6) {
        if (RP6->update.dir) {
                RP6_Execute_Direction(RP6);
        }
        if (RP6->blinkerCount > 20) {
                RP6_Execute_Blinker(RP6);
        }
        if (RP6->update.speed) {
                RP6_Execute_Speed(RP6);
        }
}

void RP6_Execute_Direction(RP6_Full * RP6) {
        switch(RP6->dir) {
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
        }
}

void RP6_Execute_Blinker(RP6_Full * RP6) {
        switch(RP6->dir) {
        case 'a':
                PORTC ^= (1 << PINC4); /* Toggle left blinker */
                PORTB &= ~(1 << PINB7); /* Disable right blinker */
                break;
        case 'd':
                PORTC &= ~(1 << PINC4); /* Disable left blinker */
                PORTB ^= (1 << PINB7);  /* Toggle right blinker */
                break;
        default:
                PORTC &= ~(1 << PINC4); /* Disable left blinker */
                PORTB &= ~(1 << PINB7); /* Disable right blinker */
                break;
        }
}

void RP6_Execute_Speed(RP6_Full * RP6) {
        OCR1A = RP6->speed.cur;
        OCR1B = RP6->speed.cur;
}

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
