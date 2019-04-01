#define F_CPU 8000000

#include <avr/io.h>
#include <avr/interrupt.h>

#define SLA 56
#define DELAYWAARDE 20

// Declarations of functions
void i2c_slave_init();
void init();
void init_usart();
// int leescommand();
void rijden(char x);
//int leessnelheid();
uint8_t timert(int x);
void init_leds();
void toggle_links();
void toggle_rechts();
int stuurt(int update); // 1 = links, 2 = rechts, 3 = 0 = rechtdoor of achteruit
void leds();
int BumperStatusLinks();
int BumperStatusRechts();
void setDistance(uint16_t links, uint16_t rechts);
void init_ENC_interrupt();
void DriveSpecific();

char x,y, k;

int main (void){
    // Initialisers
    cli();
    init();
	init_leds();
    //init_usart();
    sei();
	i2c_slave_init();
	init_ENC_interrupt();

    while(1)
        {
				   toggle_rechts();
				   toggle_links();
				   BumperStatusRechts();
				   BumperStatusLinks();
        }
    return 0;
}

ISR(TWI_vect) {
	
	switch(TWSR) {
		// Slave Receiver mode
		case 0x60: // Own SLA+W has been received; ACK has been returned
		break;
		case 0x80: // Addressed with own SLA+W; data received; ACK send
		x = TWDR; break; 
		case 0xA0: // A STOP or repeated START condition has been received
		y = x; break;
		// Slave Transmitter mode
		case 0xA8: // Own SLA+R has been received; ACK has been returned
		rijden(y); break;
		case 0xB8: // Data byte in TWDR has been send; ACK has been received
		rijden(y); break;
		case 0xC0: // Data byte in TWDR has been send; NACK has been received
		y = 0; break;
	}
	TWCR |= (1<<TWINT); // Clear TWINT Flag
}

// Functions used:
void i2c_slave_init() {
	TWSR = 0;
	TWAR = (SLA<<1); // move address into Address Register
	TWCR = (1<<TWEA) | (1<<TWIE) | (1<<TWEN);
}

void init_usart(){
    UCSRA = 0; // USART status and control registerA uit
    UCSRB = (1 << RXEN) | (1 << RXCIE); // Enable de USART Reciever
    UCSRC = (1 << UCSZ1) | (1 << UCSZ0); /* 8 data bits, 1 stop bit */
    UBRRH = 00;
    UBRRL = 12; //baudrade 38.4K. voor robotloader
}

void rijden (char x) {

    switch(x) {
    case 'w':
		TWDR = '^';
        PORTC &= ~(1 << PINC2);
        PORTC &= ~(1 << PINC3);
		stuurt(3);
        break;
    case 'a':
        PORTC |= (1 << PINC2);
        PORTC &= ~(1 << PINC3);
		stuurt(1);
		TWDR = '<';
        break;
    case 's':
        PORTC |= (1 << PINC2);
        PORTC |= (1 << PINC3);
		TWDR = '.';
		stuurt(3);
        break;
    case 'd':
        PORTC &= ~(1 << PINC2);
        PORTC |= (1 << PINC3);
		stuurt(2);
		TWDR = '>';
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
	case '8':
		OCR1A = 0;
		OCR1B = 0;
		TWDR = 'X';
    }
}


void init()
{
    TCNT1 = 0;

    TCCR1A |= (1 << COM1A1)|(1 << COM1B1)|(1 << WGM11); // set output to high on compare + pwm 9-bit
    TCCR1B |= (1 << WGM13)|(1 << CS10); // no prescaler + pwm, phase & frequency correct

    ICR1 = 0x00FF; // set interrupt
    DDRC |= (1 << PINC2) | (1 << PINC3);
}

/*ISR(USART_RXC_vect) {
    rijden(UDR);
}*/

void init_leds(){
	DDRB |= 0b10000000; // Stel pb7 in als output
	DDRC |= 0b00010000; // Stel pc4 in als output
	TIMSK = (1<<OCIE0); // Timer overflow interrupt bitje
	TCCR0 |= (1<<COM00) | (1<<WGM01); // Timer control register COM00 - toggle oc0a on compare match | WGM on ctc mode
	TCCR0 |= (1<<CS02) | (1<<CS00); // Stelt de prescaler in op 1024. Dan moet de OCA op 77.
	OCR0 = 77; // OUtput compare ingesteld op 77 (80000/1024)
}

void toggle_links(){
	if (stuurt(0) == 1 && timert(0)){
		PORTB ^= (1<<PINB7);
		PORTC &= ~(1<<PINC4);
	} else if(stuurt(0) == 0) {
		PORTB &= ~(1<<PINB7);
	}
}

void toggle_rechts(){
	if (stuurt(0) == 2 && timert(0)){
		PORTC ^= (1<<PINC4);
		PORTB &= ~(1<<PINB7);
	} else if (stuurt(0) == 0){
		PORTC &= ~(1<<PINC4);
	}
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

int stuurt (int update) {
	static int richting = 0;
	if (update) {
		if (update == 3)
			richting = 0;
		else
			richting = update;

		return 0;
	}
	
	return richting;
}

ISR(TIMER0_COMP_vect) // Interrupt Service Routine
{
	timert(1);
}


int BumperStatusLinks()
{
	int DDRBcurrent = DDRB;
	int PORTBcurrent = PORTB;
	DDRB &= ~(1 << PINB0);

	PORTB &= ~(1 << PINB0);
	asm("nop");


	int BumperTriggerLinks = PINB & (1 << PINB0);

	DDRB = DDRBcurrent;
	PORTB = PORTBcurrent;
	
	if (BumperTriggerLinks)
	{
		PORTC |= (1 << PINC2);
		PORTC |= (1 << PINC3);
		return 1;
	}
	return 0;
}

int BumperStatusRechts()
{
	int DDRCcurrent = DDRC;
	int PORTCcurrent = PORTC;
	DDRC &= ~(1 << PINC6);
	PORTC &= ~(1 << PINC6);
	asm("nop");

	int BumperTriggerRechts = PINC & (1 << PINC6);

	DDRC = DDRCcurrent;
	PORTC = PORTCcurrent;
	if (BumperTriggerRechts)
	{
		PORTC |= (1 << PINC2);
		PORTC |= (1 << PINC3);
		return 1;
	}
	return 0;
}

void init_ENC_interrupt(){
	PORTB |= (1<<PINB4);
	DDRD &= ~((1<<PD2) | (1<<PD3)); // Instellen van PD2 en PD3
	PORTD |= (1<<PD2) |(1<<PD3);
	MCUCR |= (1<<ISC10) | (1<<ISC00);
	GICR |= (1<<INT0) | (1<<INT1);  // Interrupt
}


void setDistance(uint16_t links, uint16_t rechts){
	static long long int edges_links = 0;
	static long long int edges_rechts = 0;

	if (links != 0)
	{
		edges_links = links;
	}
	if (rechts != 0)
	{
		edges_rechts = rechts;
	}
	long long int avg_edges = (edges_links+edges_rechts)/2;
	int distance = avg_edges * 0.025;
	static long long int last_distance = 0;
	if (last_distance != distance){
		writeInt(distance);
		last_distance = distance;
	}
	
}

ISR(INT0_vect)
{
	static long long int edges_links = 0;
	edges_links++;
	setDistance(edges_links, 0);
}

ISR(INT1_vect)
{
	static long long int edges_rechts = 0;
	edges_rechts++;
	setDistance(0 , edges_rechts);
}

void DriveSpecific( char k){
	
}