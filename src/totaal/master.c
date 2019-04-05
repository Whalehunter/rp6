#define F_CPU 16000000 		/* Stond op 16e7 */
#define F_SCL 100000
#define SLA 56 // (Slave ID (RP6))
#ifndef __AVR_ATmega2560__
#define __AVR_ATmega2560__
#endif

#include <avr/io.h>
#include <avr/interrupt.h>

typedef struct {
	uint8_t command;
	uint8_t type;
	uint8_t receivedBuffer[100];
} Arduino_I2C;

typedef struct {
	uint16_t start;
	uint16_t end;
} Arduino_Sonar;

typedef struct {
	uint8_t aan;
	uint8_t temp;
} Arduino_Pieper;

typedef struct {
	Arduino_I2C i2c;
	Arduino_Sonar sonar;
	Arduino_Pieper pieper;
} Arduino_Full;

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

void USART_Write_EOL() {
	while (~UCSR0A & (1 << UDRE0));
	UDR0 = 0x0D;
	while (~UCSR0A & (1 << UDRE0));
	UDR0 = 0x0A;
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
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); /* Start transmission */
	while (!(TWCR & (1 << TWINT))); /* Wait for end transmission */
}

void I2C_Send(uint8_t data) {
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN); /* Start transmission */
	while (!(TWCR & (1 << TWINT)));    /* Wait for end transmission */
}

uint8_t I2C_Ack(char type[]) {
	if (type[0] == 'N') {	/* No Ack: Slave stops sending data */
		TWCR = (1 << TWINT) | (1 << TWEN);
	} else {		/* ACK: Slave can send more data */
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	}
	while (!(TWCR & (1 << TWINT)));
	return TWDR;
}

void I2C_Stop() {
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
	while (!(TWCR & (1 << TWINT)));
}

void I2C_Write(uint8_t x) {
	I2C_Start();
	I2C_Send((SLA << 1) + 0); /* Write data */
	I2C_Send(x);
	I2C_Stop();
}

void I2C_Read() {
	I2C_Start();
	I2C_Send((SLA << 1) + 1); /* Read data */
	uint8_t y = I2C_Ack("NACK");
	I2C_Stop();
	USART_Write(y);
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
	TCCR0B |= (1 << CS00); 	/* Enable no prescaler TCNT0 */
}

void Sonar_Pulse() {
	DDRL |= (1 << PL0); // PL0 = ICP4
	PORTL |= (1 << PL0);

	/* Delay 2 us */
	TCNT0 = 0;
	while (32 < TCNT0); 	/* 1 us = 16 (F_CPU = 16MHz) */

	PORTL &= ~(1 << PL0);
	DDRL &= ~(1 << PL0);
	TIMSK4 |= (1 << ICIE4); // enable input capture interrupt
}

/*****************************************************************************/
/*                                   Pieper                                  */
/*****************************************************************************/

void Pieper_Init() {
	TCCR2A = (1 << COM2A0) | (1 << WGM21); /* OC2A on compare match & CTC */
	OCR2A = 8;
	TCCR3A = (1 << COM3A1) | (1 << COM3A0);
	OCR3A = 31250;		/* 500ms output compare */
	TIMSK3 = (1 << OCIE3A);
}

void Pieper_Aan() {
	TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);
	TCCR3B = (1 << CS32) | (1 << WGM32);
}

void Pieper_Uit() {
	TCCR2B = (1 << CS00);
	TCCR3B = (1 << CS00);
	DDRB &= ~(1 << PINB4);
}

/*****************************************************************************/
/*             Hier begint de Arduino struct & de ISR definities             */
/*****************************************************************************/

Arduino_Full arduino = {{0, 0, {}},
			{0, 0},
			{1, 1}};

/*****************************************************************************/
/*                              USART interrupt                              */
/*****************************************************************************/

ISR(USART0_RX_vect) {
	arduino.i2c.type = 0;
        arduino.i2c.command = UDR0;
        switch(UDR0){
        case 'a':
        case 'w':
        case 'd':
        case 'q':
	case '0':
                I2C_Start();	/* Roep TWI_vect aan */
		pieper_uit();
		break;
        case 's':
                I2C_Start();	/* Roep TWI_vect aan */
		pieper_aan();
		break;
        case '1':
        case '2':
        case '3':
                I2C_Start();
		break;
        case 'p':
                sonar = !sonar;
		break;
        }
}

/*****************************************************************************/
/*                               I2C Interrupt                             */
/*****************************************************************************/

ISR(TWI_vect) {
	volatile uint8_t TWI_STATUS = (TWSR & 0xF8);

	switch(TWI_STATUS) {

		/* SLA+W & SLA+R beide hier **********************************/
	case 0x08: 		/* Start condition has been transmitted */
		I2C_Send((SLA << 1) + arduino.i2c.type);
		break;

		/* SLA+W vanaf hier ******************************************/
	case 0x18:		/* SLA+W has been transmitted, ACK has been received */
		I2C_Send(arduino.i2c.command);
		break;
	case 0x20:		/* SLA+W has been transmitted, NOT ACK has been received */
		/* I2C_Stop(); */
		break;
	case 0x28:		/* Data byte has been transmitted, ACK has been received */
		I2C_Stop();
		break;
	case 0x30:		/* Data byte has been transmitted, NOT ACK has been received */
		break;

		/* SLA+R vanaf hier ******************************************/
	case 0x40:		/* SLA+R has been transmitted, ACK has been received */
		break;
	case 0x48:		/* SLA+R has been transmitted, NOT ACK has been received */
		break;
	case 0x50:		/* Data has been received, ACK has been transmitted */
		break;
	case 0x58:		/* Data has been received, NOT ACK has been transmitted */
		USART_Write(TWDR);
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
		uint8_t distance = (((arduino.sonar.end - arduino.sonar.start) * 0.2704) - 14);
		if (distance <= 8) {
			arduino.i2c.command = '8';
			arduino.i2c.type = 0;
			I2C_Start();
		}
		TIMSK4 &= ~(1 << ICIE4); /* Disable ICR interrupt */
	}
}

/*****************************************************************************/
/*                              Pieper interrupt                             */
/*****************************************************************************/

ISR(TIMER3_COMPA_vect) {
	if (arduino.pieper.aan)
		TCCR2B = (1 << CS00);
	else
		TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);

	arduino.pieper.aan = !arduino.pieper.aan;
}

int main(void) {

	while(1)
		;
        return 0;
}
