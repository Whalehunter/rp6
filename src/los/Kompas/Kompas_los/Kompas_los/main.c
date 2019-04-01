/*
 * Kompas_los.c
 *
 * Created: 26-3-2019 12:11:12
 * Author : mgjho
 */ 


#define F_CPU 16000000UL
#define F_SCL 100000UL

#include <util/delay.h>
#include <avr/io.h>
//#include "usart.h"
#define COMPASS_AD 0x60 // I2C adress of kompas

volatile uint8_t input[5];
volatile uint16_t tabel[10] = {65,66,67,68,69,70,71,72,73,74};
volatile uint8_t teller; 

void init_compass();
uint8_t getCompassRegister(uint8_t aRegister);
uint16_t getBearing();
void init_usart();



int main(void)
{
    init_compass();
	//init_usart();
	
    while (1) 
    {
		uint16_t robot_bearing = getBearing();
		//transmitByte(robot_bearing); // Functie transmit byte nodig
    }
}

void init_compass(){
	TWSR = 0;
	TWAR = (COMPASS_AD <<1);
	TWCR |= (1<<TWEA) | (1<<TWIE) | (1<<TWEN);
}

ISR(TWI_vect){
	static uint8_t data[5];
	switch (TWSR){
		//slave receive mode
		case 0x60: // Own SLA+W has been received; ACK has been returned
		teller = 0;
		break;
		case 0x80: // Adressed with own SLA+W; data received; ACK 
		data[teller] = TWDR;
		teller++;
		break;
		case 0xA0: // A stop or repeated START condition has been received 
		for (int i = 0; i<5; i++){
			input[i] = data[i];
		}
		break;
		
		//Slave Transmitter mode
		case 0xA8:// Own SLA+R has been received; ACK has been returned
		teller = 0;
		TWDR = tabel[teller];
		teller++;
		break;
		case 0xB8: // Data type in TWDR has been send: ACK has been received
		TWDR = tabel[teller];
		teller ++;
		break;
		case 0xC0: // Data type in TWDR has been send; NACK has been received
		break;
		}
		TWCR |= (1<<TWINT); // Clear TWINT flag
}

uint8_t getCompassRegister(uint8_t aRegister){
	// START conditie
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<< TWEN);
	while (!(TWCR & (1<<TWINT)));
	
	// Set I2C adress
	
	TWDR = (COMPASS_AD <<1);
	TWCR = (1<< TWINT) | (1<< TWEN);
	
	while (!(TWCR & (1<<TWINT)));
	
	// Select register 
	TWDR = aRegister;
	// Start write operation
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	
	// STOP condition
	TWCR = (1<< TWINT) | (1<< TWSTO) | (1<<TWEN);
	
	// START conditie
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	
	// Set I2C adress
	TWDR = (COMPASS_AD << 1 ) + 1; // Nu gaan lezen
	TWCR = (1 << TWINT) | (1<<TWEN);
	
	while (!(TWCR & (1<<TWINT)));
	
	TWCR = (1<<TWINT) | (1<< TWEN);
	while (!(TWCR & (1<<TWINT)));
	
	uint8_t received_byte = TWDR;
	uint8_t status_byte = TWSR;
	
	// STOP condition
	TWCR = (1<< TWINT) | (1<<TWSTO) | (1<<TWEN);
	
	return received_byte;
}

uint16_t getBearing(){
	// Voor hoge nauwkeurigheid register 2 en 3, HIGH byte first
	
	uint16_t bearing = 0;
	bearing = getCompassRegister(2);
	bearing = (bearing << 8); // register 2 naar high byte
	bearing = bearing + getCompassRegister(3);
	return bearing;
}

int main(void) {
	printf("hallo")
	init_Compass();
	while (1)
	{
	uint16_t robot_bearing = getBearing();
	transmitByte(robot_bearing);
		
void USART_print(char line[]){
	uint8_t count = 0
	while(line[count]! = 0){
		transmitByte(line[count]);
		count++;
	}

void USART_printline(char line[]){
	USART_print(line);
	transmitByte(10);
	transmitByte(13);
	
void USART_print_int(uint16_t value){
	char str [10];
	printf(str, "%d",value);
	USART_print(str);
	
