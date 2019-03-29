/*
 * Afstand_RP6_1.0.c
 *
 * Created: 29-3-2019 16:06:41
 * Author : mgjho
 */ 

#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>

#define uint16_t Targ_ENC_A;
#define uint16_t Targ_ENC_B;
void init_ENC_interrupt(){
	PORTB |= (1<<PINB4);
	DDRD &= ~((1<<PD2) | (1<<PD3));
	PORTD |= (1<<PD2) |(1<<PD3);
	MCUCR |= (1<<ISC10) | (1<<ISC00);
	GICR |= (1<<INT0) | (1<<INT1);
}

void disableDistance(){
	Targ_ENC_A = 0XFFFF;
	Targ_ENC_B = 0XFFFF;
}

void setDistance(uint16_t distance_cm){
	ENC_L_ticks = 0;
	ENC_R_ticks = 0;

	if(distance_cm != 0){
		transmistChar('$');
		writeIntDecimal((uint16_t)(distance_cm / 0.025));
		Targ_ENC_A = (uint16_t)(distance_cm / 0,025);
		Targ_ENC_B = (uint16_t)(distance_cm / 0,025);
	}
	else{
		disableDistance();
	}
}
