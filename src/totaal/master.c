#define F_CPU 16000000 		/* Stond op 16e7 */
#define F_SCL 100000
#define SLA 56 // (Slave ID (RP6))
#ifndef __AVR_ATmega2560__
#define __AVR_ATmega2560__
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define USARTBUFLEN 100
#define I2CBUFLEN 2

typedef struct {
	uint8_t type;
	uint8_t buf[I2CBUFLEN];
	volatile uint8_t bufIndex;
} Arduino_I2C;

typedef struct {
	uint16_t start;
	uint16_t end;
	uint8_t print;
} Arduino_Sonar;

typedef struct {
	uint8_t buf[USARTBUFLEN];
	volatile uint8_t bufIndex;
	uint8_t isParkour;
} Arduino_USART;

typedef struct {
	volatile uint16_t travelDistance;
	volatile uint8_t direction;
	volatile uint8_t speed;
} Parkour;

typedef struct {
	Arduino_USART usart;
	Arduino_I2C i2c;
	Arduino_Sonar sonar;
	uint8_t pieper;
	/* Parkour parkour; */
} Arduino_Full;

void uitoa(uint16_t value, uint8_t* buffer)
{
	uint8_t i, printed = 0;
	uint16_t digit, divisor;

	if (value) {
		for(i = 0, divisor = 10000; i < 5u; i++) {
			digit = value/divisor;
			if(digit || printed) {
				*buffer++ = '0' + digit;
				value -= digit*divisor;
				printed = 1;
			}
			divisor /= 10;
		}
	} else {
		*buffer++ = '0';
	}

    *buffer = '\0';
}

/*****************************************************************************/
/*                                   USART                                   */
/*****************************************************************************/

void USART_Init() {
	UCSR0A = 0;
	UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0); // enable USART Transmitter
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 data bits, 1 stop bit
	UBRR0H = 00;
	UBRR0L = 103; // baudrate 9600
}

void USART_Write(uint8_t c) {
	while (~UCSR0A & (1 << UDRE0));
	UDR0 = c;
}

void USART_WriteEOL() {
	while (~UCSR0A & (1 << UDRE0));
	UDR0 = 0x0D;
	while (~UCSR0A & (1 << UDRE0));
	UDR0 = 0x0A;
}

void USART_WriteString(uint8_t s[]) {
	for (int i = 0; s[i] != 0; i++) {
		USART_Write(s[i]);
	}
	USART_WriteEOL();
}

void USART_WriteDistance(Arduino_Full * ARDUINO) {
	uint16_t num = (ARDUINO->i2c.buf[0] << 8) | ARDUINO->i2c.buf[1];
	uint8_t i, distanceString[] = "De afgelegde afstand:", totalDistance[10];

	uitoa(num, totalDistance);
	USART_WriteString(distanceString);
	for (i = 0; totalDistance[i] != 0; ++i) {
		USART_Write(totalDistance[i]);
	}

	USART_Write('c');
	USART_Write('m');
	USART_WriteEOL();
}

void USART_ResetBuffer(Arduino_Full * ard) {
	for (int i = 0; i < USARTBUFLEN; i++) {
		ard->usart.buf[i] = 0;
	}
	ard->usart.bufIndex = 0;
}

/*****************************************************************************/
/*                                    I2C                                    */
/*****************************************************************************/

void I2C_Init() {
	PORTD = 0x03;		/* Pull up resistors voor SDA & SCL */
	TWSR = 0;
	TWBR = (uint8_t)(((F_CPU / F_SCL) - 16) / 2);
	TWCR = (1 << TWEN);
}

void I2C_Start() {
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN) | (1 << TWIE); /* Start transmission */
}

void I2C_SetData(uint8_t b) {
	TWDR = b;
}

void I2C_Ack(char type[]) {
	if (type[0] == 'N') {	/* No Ack: Slave stops sending data */
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
	} else {		/* ACK: Slave can send more data */
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE);
	}
}

void I2C_Stop() {
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO) | (1 << TWIE);
}

/*****************************************************************************/
/*                                   SONAR                                   */
/*****************************************************************************/

void Sonar_Init() {
	TCCR4B |= (1 << ICES4)| (1 << CS12) | (1 << WGM12) | (1 << ICNC4); //  Input Capture on rising edge, IC noise canceler 1, prescaler 256, CTC mode (OCR TOP)
	TIMSK4 |= (1 << OCIE1A); // enable OCR interrupt
	TIMSK4 &= ~(1 << ICIE4); // disable input capture interrupt
	OCR4A = 31250; // 2Hz frequency

	/* Use timer0 to count microseconds in replacement of _delay_us() */
	/* TCCR0B |= (1 << CS00); 	/\* Enable no prescaler TCNT0 *\/ */
}

uint16_t Sonar_GetDistance(Arduino_Full * a) {
	uint16_t distance = (((a->sonar.end - a->sonar.start) * 0.2704) - 14);
	if (distance > 255) {
		return 255;
	}

	return distance;
}

void Sonar_Pulse() {
	DDRL |= (1 << PL0); // PL0 = ICP4
	PORTL |= (1 << PL0);

	/* Delay 2 us */
	/* TCNT0 = 0; */
	/* while (32 < TCNT0); 	/\* 1 us = 16 (F_CPU = 16MHz) *\/ */
	_delay_us(2);
	PORTL &= ~(1 << PL0);
	DDRL &= ~(1 << PL0);
	TIMSK4 |= (1 << ICIE4); // enable input capture interrupt
}

/*****************************************************************************/
/*                                   Pieper                                  */
/*****************************************************************************/

void Pieper_Aan() {
	DDRB |= (1 << PB4);
	TCCR2A = (1 << COM2A0) | (1 << WGM21); /* OC2A on compare match & CTC */
	TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);
	OCR2A = 8;

	TCCR3A = (1 << COM3A1) | (1 << COM3A0);
	TCCR3B = (1 << CS32) | (1 << WGM32);
	OCR3A = 31250;		/* 500ms output compare */
	TIMSK3 = (1 << OCIE3A);
}

void Pieper_Uit() {
	TCCR2B = (1 << CS00);
	TCCR3B = (1 << CS00);
	DDRB &= ~(1 << PINB4);
}

/*****************************************************************************/
/*             Hier begint de Arduino struct & de ISR definities             */
/*****************************************************************************/

Arduino_Full arduino = {{"", 0, 0}, 	/* USART */
			{0, "", 0},	/* I2C */
			{0, 0, 0},	/* SONAR */
			1};		/* PIEPER */

/*****************************************************************************/
/*                              USART interrupt                              */
/*****************************************************************************/

ISR(USART0_RX_vect) {
	char x = UDR0;
	arduino.i2c.type = 0;	/* I2C SLA+W modus */
	switch(x){
	case 'a':
	case 'w':
	case 'd':
	case 'q':
		USART_ResetBuffer(&arduino);
		arduino.usart.buf[0] = x;
		I2C_Start();	/* Roep TWI_vect aan */
		Pieper_Uit();
		break;
	case 's':
		USART_ResetBuffer(&arduino);
		arduino.usart.buf[0] = x;
		I2C_Start();	/* Roep TWI_vect aan */
		Pieper_Aan();
		break;
	case '1':
	case '2':
	case '3':
		USART_ResetBuffer(&arduino);
		arduino.usart.buf[0] = x;
		I2C_Start();
		break;
	case 'x':
		arduino.i2c.type = 1; /* I2C SLA+R modus */
		I2C_Start();
		break;
	case 'p':
		arduino.sonar.print = !arduino.sonar.print;
	}
}

/*****************************************************************************/
/*                               I2C Interrupt                             */
/*****************************************************************************/

ISR(TWI_vect) {
	switch(TWSR & 0xF8) {

	/* SLA+W & SLA+R beide hier **********************************/
	case 0x08: 		/* Start condition has been transmitted */
		I2C_SetData((SLA << 1) + arduino.i2c.type);
		I2C_Ack("ACK");
		break;

		/*************************************************************/
		/*                           SLA+W                           */
		/*************************************************************/
	case 0x18:		/* SLA+W has been transmitted, ACK has been received */
		I2C_SetData(arduino.usart.buf[0]);
		if (arduino.usart.isParkour) {
			I2C_Ack("ACK");	/* kan ook nack zijn */
		} else {
			I2C_Ack("NACK");
		}
		break;
	case 0x28:		/* Data byte has been transmitted, ACK has been received */
		if (arduino.usart.bufIndex == USARTBUFLEN - 1 || !arduino.usart.buf[arduino.usart.bufIndex]){
			I2C_Stop();
		} else {
			arduino.usart.bufIndex++;
			I2C_SetData(arduino.usart.buf[arduino.usart.bufIndex]);
			I2C_Ack("ACK");
		}
		break;
	case 0x30:		/* Data byte has been transmitted, NOT ACK has been received */
		I2C_Stop();
		break;

		/*************************************************************/
		/*                           SLA+R                           */
		/*************************************************************/
	case 0x40:		/* SLA+R has been transmitted, ACK has been received */
		I2C_Ack("ACK");
		break;
	case 0x48:		/* SLA+R has been transmitted, NOT ACK has been received */
		break;
	case 0x50:		/* Data has been received, ACK has been transmitted */
		arduino.i2c.buf[arduino.i2c.bufIndex++] = TWDR;
		if (arduino.i2c.bufIndex < I2CBUFLEN) {
			I2C_Ack("ACK");
		} else {
			I2C_Ack("NACK");
			arduino.i2c.bufIndex = 0;
		}
		break;
	case 0x58:		/* Data has been received, NOT ACK has been transmitted */
		USART_WriteDistance(&arduino);
		I2C_Stop();
		break;
	}
}

/*****************************************************************************/
/*                              SONAR Interrupts                             */
/*****************************************************************************/

ISR(TIMER4_COMPA_vect) {
	Sonar_Pulse();
}

ISR(TIMER4_CAPT_vect) {
	if (TCCR4B & (1 << ICES4)) {    /* Rising edge */
		arduino.sonar.start = ICR4;
		TCCR4B &= ~(1 << ICES4);
	} else {		        /* Falling edge */
		TCCR4B |= (1 << ICES4);
		arduino.sonar.end = ICR4;
		uint16_t dist = Sonar_GetDistance(&arduino);
		if (dist <= 8) {
			arduino.usart.buf[0] = '8';
			arduino.i2c.type = 0;
			I2C_Start();
		}
		if (arduino.sonar.print) {
			uint8_t str[] = "Afstand tot object:";
			USART_WriteString(str);
			uint8_t distance[10];
			uitoa(dist, distance);

			for (int i = 0; distance[i] != 0; ++i) {
				USART_Write(distance[i]);
			}
			USART_Write('c');
			USART_Write('m');
			USART_WriteEOL();
		}
		TIMSK4 &= ~(1 << ICIE4); /* Disable ICR interrupt */
	}
}

/*****************************************************************************/
/*                              Pieper interrupt                             */
/*****************************************************************************/

ISR(TIMER3_COMPA_vect) {
	static uint8_t temp = 1;

	if (arduino.pieper && temp) {
		TCCR2B = (1 << CS00);
		arduino.pieper = !arduino.pieper;
	} else if (!arduino.pieper && !temp) {
		TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);
		arduino.pieper = !arduino.pieper;
	}

	temp = !temp;
}

int main(void) {
	cli();
	USART_Init();
	I2C_Init();
	Sonar_Init();
	sei();

	while(1)
		;
        return 0;
}
