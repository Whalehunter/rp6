/*
 * ATMega 2560, Master of RP6
 *
 * 
 * Auteurs: Bjorn, Jeffrey M, Maeve, Martijn, Jelmer, Jeffrey H
 * Bronnen:
 */ 

#define F_CPU 16000000
#define F_SCL 100000
#define SLA 56 // (Slave ID (RP6))

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void sonarSensor(); // sensor to measure and display distance to objects
void printDistance(); // print distance to objects
void initUSART(); // set up USART
void initADC(); // set up ADC
void initMaster(); // set up master register
void i2c_start(); // start i2c transmission
void i2c_send_byte(uint8_t data); // send byte over i2c
uint8_t i2c_read_ack(); // read incoming ACK
uint8_t i2c_read_nack(); // read incoming NACK
void i2c_stop(); // send stop byte
void i2c_write(char x); // write (SLA + 0) to slaves
void i2c_read(); // read slave response (SLA + 1)
void lineFeed(); // often re-used code to simulate "enter" in USART

int b = 0; // variable used to store distance to object
int sonar = 0;
char x, y;

int main (void) {
	initUSART();
	initADC();
	initMaster();

	TCCR1A = 0x00;
	TCCR1B = (1 << CS10) | (1 << CS11); // start timer with prescaler 64
	
	sei(); // enable global interrupt
	
	while(1){
		sonarSensor();
	}
	
	return 0;
}

ISR(TIMER1_OVF_vect) {
	// placeholder timer overflow interrupt
}

ISR(USART0_RX_vect){
	x = UDR0;
	switch(x){
		case 'a':
		i2c_write(x); i2c_read(); break;
		case 'w':
		i2c_write(x); i2c_read(); break;
		case 'd':
		i2c_write(x); i2c_read(); break;
		case 's':
		i2c_write(x); i2c_read(); break;
		case 'p':
		sonar = !sonar; break;
		default:
		i2c_write(x); break;
	}
}

void sonarSensor(){
	uint32_t minloops = 0;
	uint32_t maxloops = 400000;

	TCNT1 = 0;
	for (int i = 0; i < 128; i++) {
		while (64000 > TCNT1);
	}

	DDRB |= (1 << PB5); // PB5 on output (send pulse)
	PORTB &= ~(1 << PB5); // clear PB5
	_delay_us(2); // _delay_us(2)

	PORTB |= (1 << PB5); // input trigger pulse
	_delay_us(5); // _delay_us(5)

	PORTB &= ~(1 << PB5); // clear PB5
	DDRB &= ~(1 << PB5); // PB5 on input (listen to pulse echo)

	while (!(PINB & (1 << PB5))) { // wait until PB5 is high
		if (minloops == maxloops) {
			break; // return if timeout
		}
		minloops++;
	}

	if (minloops < maxloops) {
		TCNT1 = 0;
		while ((PINB & (1 << PB5)));
		b = (TCNT1);
		if (sonar == 1){
			printDistance(b / 15);
		}
	}
}

void initUSART() {
	UCSR0A = 0;
	UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0); // enable USART Transmitter
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 data bits, 1 stop bit
	
	UBRR0H = 00;
	UBRR0L = 103; // baudrate 9600
}

void initADC() {
	ADMUX |= (1 << REFS1); // reference voltage
	ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); // ADC clock prescaler /128
	ADCSRA |= (1 << ADEN);
	ADCSRA |= (1 << ADSC | 1 << ADIF); // start ADC conversion
}

void initMaster(void) {
	PORTD = 0x03;//PORTC = 0x30 voor arduino uno; // pull-up resistors for SDA & SCL (00110000
	TWSR = 0;
	TWBR = (uint8_t)(((F_CPU / F_SCL) - 16 ) / 2);
	TWCR = (1 << TWEN);
}

void i2c_start(void) {
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // transmit START condition
	while ( !(TWCR & (1 << TWINT)) ); // wait for end of transmission
}

void i2c_send_byte(uint8_t data) {
	TWDR = (data);
	TWCR = (1 << TWINT) | (1 << TWEN); // start transmission
	while ( !(TWCR & (1 << TWINT)) ); // wait for end of transmission
}

uint8_t i2c_read_ack(void) {
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA); // start TWI with ACK after reception
	while ( !(TWCR & (1 << TWINT)) ); // wait for end of transmission
	return TWDR; // return received data from TWDR
}

uint8_t i2c_read_nack(void) {
	TWCR = (1 << TWINT) | (1 << TWEN); // start receiving without acknowledging reception
	while ( !(TWCR & (1 << TWINT)) ); // wait for end of transmission
	return TWDR; // return received data from TWDR
}

void i2c_stop(void) {
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); // transmit STOP condition
	while ( !(TWCR & (1 << TWINT)) ); // wait for end of transmission
}

void i2c_write(char x) {
	i2c_start(); // Start new transmission
	i2c_send_byte((SLA << 1) + 0); // SLA = slave address, + 0 = master is writing data (write), doesn't expect a response
	i2c_send_byte(x); // Send data + wait for end of transmission
	i2c_stop; // Send stop signal (TWSTO)
}

void i2c_read() {
	i2c_start(); // Start new transmission
	i2c_send_byte((SLA << 1) + 1); // SLA = slave address, + 1 = master is expecting data (read)
	//y = i2c_read_ack(); // Read transmission and send ACK to confirm successful data transfer and store in variable
	y = i2c_read_nack(); // Read transmission without sending ACK to inform slave to stop sending and store in variable
	i2c_stop; // Send stop signal (TWSTO)
  
	while (~UCSR0A & (1 << UDRE0));
	UDR0 = y; // print data
}

int readADC(int analogInput) {
	ADMUX = (0xf0 & ADMUX) | analogInput;
	ADCSRA |= (1 << ADSC);
	while (~ADCSRA & (1 << ADIF));
	return ADC;
}

void printDistance(uint16_t x) {
	char danger[] = {'D','A','N','G','E','R'};
	char buffer[16];
	itoa(x, buffer, 10);
	for (int i = 0; buffer[i] != 0; i++) {
		while (~UCSR0A & (1 << UDRE0));
		UDR0 = buffer[i];
	}
	while (~UCSR0A & (1 << UDRE0));
	UDR0 = 'c';
	while (~UCSR0A & (1 << UDRE0));
	UDR0 = 'm';
	if(x <= 8){
		while (~UCSR0A & (1 << UDRE0));
		UDR0 = '-';
		while (~UCSR0A & (1 << UDRE0));
		UDR0 = '-';
		for(int y = 0; y < 6; y++){
			while (~UCSR0A & (1 << UDRE0));
			UDR0 = danger[y];
		}
	}
	lineFeed();
}

void lineFeed() {
	while (~UCSR0A & (1 << UDRE0));
	UDR0 = 0x0D;
	while (~UCSR0A & (1 << UDRE0));
	UDR0 = 0x0A;
}
