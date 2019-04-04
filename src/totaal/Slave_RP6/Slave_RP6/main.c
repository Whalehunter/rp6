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
void init_update_interval();
void init_ENC_interrupt();
int BumperStatusLinks();
int BumperStatusRechts();

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

/* Wheel settings RP6 ********************************************************/
typedef struct {
	int right;
	int left;
} RP6_Distance;

/* Full settings RP6 *********************************************************/
typedef struct {
	RP6_Speed speed;
	RP6_Distance distance;
	char dir;
	int blinkerCount;
	RP6_Update update;
} RP6_Full;

void RP6_SetDistance(RP6_Full * RP6);	       /* Update RP6 afstand */
void RP6_SetBlinker(RP6_Full * RP6); /* Update RP6 knipperlicht */
void RP6_SetSpeed(RP6_Full * RP6, char speed); /* Update RP6 speed geleidelijk */
void RP6_SetDirection(RP6_Full * RP6, char direction); /* Update RP6 richting */
char RP6_GetDirection(RP6_Full * RP6);                 /* Get current direction of RP6 */
void RP6_SetCurrentSpeed(RP6_Full * RP6);        /* Set acceleration for next item */

/*****************************************************************************/
/*      De "Execute" functies zijn in princepe alleen voor RP6_Execute.      */
/*****************************************************************************/
void RP6_Execute(RP6_Full * RP6);                /* Execute RP6 settings */
void RP6_Execute_Direction(RP6_Full * RP6);      /* Execute drive direction */
void RP6_Execute_Blinker(RP6_Full * RP6);        /* Execute blinker toggles */
void RP6_Execute_Speed(RP6_Full * RP6); /* Execute RP6 speed */

void RP6_SetBlinker(RP6_Full * RP6) {
	RP6->blinkerCount = 20;
}

void RP6_SetSpeed(RP6_Full * RP6, char speed) {
	int newSpeed;

	switch(speed){
		case '1':
		newSpeed = 50;
		break;
		case '2':
		newSpeed = 125;
		break;
		case '3':
		newSpeed = 175;
		break;
		case 'q':
		newSpeed = 0;
		break;
	}

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
		} else if (RP6->speed.cur == 0) {
		RP6->speed.cur = 10;
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

static RP6_Full rp6 = {{0,0,0}, /* Speed */
'w',     /* Direction */
0,       /* Blinker counter */
{0,0,0}};  /* Updates */





int main(void){
	cli();                  /* Disable global interrupts */
	init_motors();
	init_leds();
	i2c_init();
	init_update_interval();
	sei();                  /* Enable global interrupts */

	while(1) {
	RP6_Execute(&rp6);
	BumperStatusLinks();
	BumperStatusRechts();
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
		//i2c_send(RP6_GetDirection(&rp6));
		i2c_send(55);
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
	UBRRL = 12; //baudrate 38.4K. voor robotloader
}

void drive(char x) {
	switch(x) {
		case 'w':
		case 'a':
		case 's':
		case 'd':
		RP6_SetDirection(&rp6, x);
		break;
		case '1':
		case '2':
		case '3':
		RP6_SetSpeed(&rp6, x);
		break;
		case 'q':
		RP6_SetSpeed(&rp6, x);
		break;
		case '8':
		rp6.speed.next = 0;
		rp6.speed.cur = 0;
		RP6_Execute_Speed(&rp6);
		break;
		case 'm':;
	}
}

void init_update_interval() {
	TCCR2 |= (1 << CS22) | (1 << CS21) | (1 << CS20);
	TIMSK |= (1 << TOIE2);
}

void init_motors() {
	TCNT1 = 0;

	TCCR1A |= (1 << COM1A1)|(1 << COM1B1)|(1 << WGM11); // set output to high on compare + pwm 9-bit
	TCCR1B |= (1 << WGM13)|(1 << CS10); // no prescaler + pwm, phase & frequency correct

	ICR1 = 0x00FF; // set interrupt
	DDRC |= (1 << PINC2) | (1 << PINC3);
}

void init_leds(){
	DDRB |= (1 << PINB7); /* PB7 als OUTPUT */
	DDRC |= (1 << PINC4); /* PC4 als OUTPUT */
	TIMSK = (1 << OCIE0); /* Timer overflow ENABLE */
	TCCR0 |= (1 << COM00) | (1<<WGM01); // CTC ENABLE
	TCCR0 |= (1<<CS02) | (1<<CS00); // Prescaler: 1024
	OCR0 = 77; // Output compare: 77 (80000/1024)
}

ISR(TIMER2_OVF_vect) {
	if (rp6.update.speed) {
		RP6_SetCurrentSpeed(&rp6);
		RP6_Execute_Speed(&rp6);
	}
}

ISR(TIMER0_COMP_vect) // Interrupt Service Routine
{
	rp6.blinkerCount++;
}


void init_ENC_interrupt(){
	PORTB |= (1<<PINB4);
	DDRD &= ~((1<<PD2) | (1<<PD3)); // Instellen van PD2 en PD3
	PORTD |= (1<<PD2) |(1<<PD3);
	MCUCR |= (1<<ISC10) | (1<<ISC00);
	GICR |= (1<<INT0) | (1<<INT1);  // Interrupt
}


void setDistance(RP6_Full * RP6){
	i2c_send((RP6->distance.left + RP6->distance.right) * 0.05); /* TODO: Converteren naar iets dat werkelijk verzonden kan worden */
}

ISR(INT0_vect) {
	rp6.distance.left++;
}

ISR(INT1_vect) {
	rp6.distance.right++;
}



int BumperStatusLinks(RP6_Full * RP6)
{

	int DDRBNu = DDRB;
	int PORTBNu = PORTB;
	DDRB &= ~(1 << PINB0);		// input
	PORTB &= ~(1 << PINB0);		// leegmaken van de pin
	asm("nop");

	int BumperTriggerLinks = PINB & (1 << PINB0);

	DDRB = DDRBNu;
	PORTB = PORTBNu;
	
	if (BumperTriggerLinks)
	{
		RP6_SetDirection(&rp6, 'd');
		RP6_SetSpeed(&rp6, '1');
		return 1;
	}
	return 0;
}


int BumperStatusRechts(RP6_Full * RP6)
{

	int DDRCNu = DDRC;
	int PORTCNu = PORTC;
	DDRC &= ~(1 << PINC6);
	PORTC &= ~(1 << PINC6);
	asm("nop");

	int BumperTriggerRechts = PINC & (1 << PINC6);

	DDRC = DDRCNu;
	PORTC = PORTCNu;
	if (BumperTriggerRechts)
	{
		RP6_SetDirection(&rp6, 'a');
		RP6_SetSpeed(&rp6, '1');
		return 1;
	}
	return 0;
}

