#define F_CPU 8000000
#ifndef __AVR_ATmega32__
#define __AVR_ATmega32__
#endif

#include <avr/io.h>
#include <avr/interrupt.h>

#define SLA 56
#define BLINK 32

void i2c_init();
void init_motors();
void init_usart();
void init_leds();
void init_bumpers();
void init_encoder_interrupt();
void drive(char x);
uint8_t timert(int x);
void i2c_SetData(char data);
void init_update_interval();

#define KEERAFSTAND 15

/* If a setting needs updating in while loop *********************************/
typedef struct {
        int speed;
        int dir;
	int bumped;
} RP6_Update;

/* Bumper settings RP6 *******************************************************/
typedef struct {
	uint8_t kant;
	uint16_t travelDistanceBump;
	uint16_t travelDistanceTurn;
	uint16_t travelDistanceSecondTurn;
	uint16_t travelDistanceForward;
	uint8_t currentPosition;
} RP6_Bumper;

/* Speed settings RP6 ********************************************************/
typedef struct {
        int cur;
        int prev;
        int next;
} RP6_Speed;

/* Distance driven RP6 *******************************************************/
typedef struct {
	uint32_t left;
	uint32_t right;
	uint16_t cm;
} RP6_Distance;

/* Full settings RP6 *********************************************************/
typedef struct {
        RP6_Speed speed;
        char dir;
        int blinkerCount;
        RP6_Update update;
	RP6_Distance distance;
	RP6_Bumper bump;
} RP6_Full;

void RP6_SetDistance(RP6_Full * RP6);			/* Update RP6 travel distance */
void RP6_SetBlinker(RP6_Full * RP6);			/* Update RP6 knipperlicht */
void RP6_SetSpeed(RP6_Full * RP6, char speed); 		/* Update RP6 speed geleidelijk */
void RP6_SetDirection(RP6_Full * RP6, char direction);	/* Update RP6 richting */
char RP6_GetDirection(RP6_Full * RP6);			/* Get current direction of RP6 */
void RP6_SetCurrentSpeed(RP6_Full * RP6);		/* Set acceleration for next item */

/*****************************************************************************/
/*      De "Execute" functies zijn in principe alleen voor RP6_Execute.      */
/*****************************************************************************/
void RP6_Execute(RP6_Full * RP6);			/* Execute RP6 settings */
void RP6_Execute_Direction(RP6_Full * RP6);		/* Execute drive direction */
void RP6_Execute_Blinker(RP6_Full * RP6);		/* Execute blinker toggles */
void RP6_Execute_Speed(RP6_Full * RP6);			/* Execute RP6 speed */

void RP6_SetDistance(RP6_Full * RP6) {
	RP6->distance.cm = (RP6->distance.left + RP6->distance.right) * 0.0125;
}

void RP6_SetBlinker(RP6_Full * RP6) {
        RP6->blinkerCount = BLINK;
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
	if (RP6->update.bumped) {
		RP6_SetDistance(RP6);

		if (RP6_GetDirection(RP6) != '.' && RP6->bump.currentPosition == 0) {
			RP6_SetDirection(RP6, 's');
			RP6->bump.travelDistanceBump = RP6->distance.cm;
			RP6->bump.currentPosition = 1;
		} else if (RP6->distance.cm - RP6->bump.travelDistanceBump > 15 && RP6->bump.currentPosition == 1) {
			if (RP6->bump.kant == 'r') {
				RP6_SetDirection(RP6, 'a');
			} else if (RP6->bump.kant == 'l'){
				RP6_SetDirection(RP6, 'd');
			}
			RP6->bump.travelDistanceTurn = RP6->distance.cm;
			RP6->bump.currentPosition = 2;
		} else if (RP6->distance.cm - RP6->bump.travelDistanceTurn > KEERAFSTAND && RP6->bump.currentPosition == 2) {
			RP6_SetDirection(RP6, 'w');
			RP6->bump.travelDistanceForward = RP6->distance.cm;
			RP6->bump.currentPosition = 3;
		} else if (RP6->distance.cm - RP6->bump.travelDistanceForward > KEERAFSTAND && RP6->bump.currentPosition == 3) {
			if (RP6->bump.kant == 'r') {
				RP6_SetDirection(RP6, 'd');
			} else {
				RP6_SetDirection(RP6, 'a');
			}
			RP6->bump.travelDistanceSecondTurn = RP6->distance.cm;
			RP6->bump.currentPosition = 4;
		} else if (RP6->distance.cm - RP6->bump.travelDistanceSecondTurn > KEERAFSTAND && RP6->bump.currentPosition == 4) {
			RP6_SetDirection(RP6, 'w');
			RP6->bump.currentPosition = 0;
			RP6->update.bumped = 0;
		}
	}

	if (RP6->update.dir) {
		RP6_Execute_Direction(RP6);
	}
	if (RP6->blinkerCount > BLINK) {
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
	if (RP6->speed.cur > 0 && (RP6->dir == 'a' || RP6->dir == 'd')) {
		if (RP6->dir == 'a') {
			PORTC ^= (1 << PINC4);  /* Toggle left blinker */
			PORTB &= ~(1 << PINB7); /* Disable right blinker */
		} else {
			PORTC &= ~(1 << PINC4); /* Disable left blinker */
			PORTB ^= (1 << PINB7);  /* Toggle right blinker */
		}
	} else {
                PORTC &= ~(1 << PINC4); /* Disable left blinker */
                PORTB &= ~(1 << PINB7); /* Disable right blinker */
	}

	RP6->blinkerCount = 0;
}

void RP6_Execute_Speed(RP6_Full * RP6) {
        OCR1A = RP6->speed.cur;
        OCR1B = RP6->speed.cur;
}

static RP6_Full rp6 = {{0,0,0},	 /* Speed */
                       'w',	 /* Direction */
                       0,	 /* Blinker counter */
                       {0,0,0},  /* Updates */
		       {0,0,0}, /* afstand links & rechts */
		       {0,0,0,0,0,0}};	/* Bumper */

void bumper() {
	if (!rp6.update.bumped && ((PINC & (1 << PINC6)) || (PINB & (1 << PINB0)))) {
		if ((PINC & (1 << PINC6))) {
			rp6.bump.kant = 'r';
		} else {
			rp6.bump.kant = 'l';
		}
		rp6.update.bumped = 1;
	}
}

int main(void){
        cli();                  /* Disable global interrupts */
        init_motors();
        init_leds();
        i2c_init();
        init_update_interval();
	init_encoder_interrupt();
	init_bumpers();
        sei();                  /* Enable global interrupts */

	while(1) {
                RP6_Execute(&rp6);
		bumper();
        }
        return 0;
}

ISR(TWI_vect) {
        switch(TWSR) {
		/*************************************************************/
		/*                           SLA+W                           */
		/*************************************************************/
	case 0x60:		/* SLA+W received, ACK send */
        case 0x80:		/* Data received, ACK send */
                drive(TWDR);
		/**
		 * TODO:
		 * Data schrijven naar array
		 */
                break;
	case 0xA0:    /* STOP received */
		/**
		 * TODO:
		 * Uitvoeren van received bytes
		 * drive(shit)
		 */
		break;
		/*************************************************************/
		/*                           SLA+R                           */
		/*************************************************************/
        case 0xA8:	    /* SLA+R received, ACK send */
		RP6_SetDistance(&rp6);
		i2c_SetData(rp6.distance.cm >> 8); /* !!!!!!!!!!!!!!!!!!!!!!!!!! */
		break;
        case 0xB8:	    /* Data transmitted, ACK received */
		i2c_SetData(rp6.distance.cm & 0x00FF); /* !!!!!!!!!!!!!!!!!! */
                break;
        case 0xC0:		/* Data transmitted, NACK received */
                /* drive('0'); */
                break;
        }
        TWCR |= (1<<TWINT); // Clear TWINT Flag
}

void i2c_init() {
        TWSR = 0;
        TWAR = (SLA<<1); // move address into Address Register
        TWCR = (1<<TWEA) | (1<<TWIE) | (1<<TWEN);
}

void i2c_SetData(char data) {
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
        case '1':
        case '2':
        case '3':
        case 'q':
                RP6_SetSpeed(&rp6, x);
                break;
	case '8':
		rp6.speed.next = 0;
		rp6.speed.cur = 0;
		RP6_Execute_Speed(&rp6);
		break;
        }
}

void init_bumpers() {
	DDRB &= ~(1 << PINB0);
	DDRC &= ~(1 << PINC6);
}

void init_update_interval() {
        TCCR2 |= (1 << CS22) | (1 << CS21) | (1 << CS20);
        TIMSK |= (1 << TOIE2);
}

void init_motors() {
        TCNT1 = 0;
        TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11); // set output to high on compare + pwm 9-bit
        TCCR1B |= (1 << WGM13) | (1 << CS10); // no prescaler + pwm, phase & frequency correct
        DDRC |= (1 << PINC2) | (1 << PINC3);
        ICR1 = 0x00FF; // set interrupt
}

void init_leds(){
        DDRB |= (1 << PINB7); /* PB7 als OUTPUT */
        DDRC |= (1 << PINC4); /* PC4 als OUTPUT */
        TIMSK = (1 << OCIE0); /* Timer overflow ENABLE */
        TCCR0 |= (1 << COM00) | (1 << WGM01); // CTC ENABLE
        TCCR0 |= (1 << CS02) | (1 << CS00); // Prescaler: 1024
        OCR0 = 244; // 16 blinkercount nu nodig om ~1hz te hebben
}

void init_encoder_interrupt() {
	PORTB |= (1 << PINB4);
	DDRD &= ~(1 << PIND2) & ~(1 << PIND3); /* !!!!!!!!!!!!!!!!!!!!!!!!!! */
	PORTD |= (1 << PIND2) | (1 << PIND3);
	MCUCR |= (1 << ISC10) | (1 << ISC00);
	GICR |= (1 << INT0) | (1 << INT1);
}

ISR(TIMER2_OVF_vect) {
        if (rp6.update.speed) {
                RP6_SetCurrentSpeed(&rp6);
                RP6_Execute_Speed(&rp6);
        }
}

ISR(TIMER0_COMP_vect)
{
        rp6.blinkerCount += 1;
}

ISR(INT0_vect) {
	rp6.distance.left++;
}

ISR(INT1_vect) {
	rp6.distance.right++;
}
