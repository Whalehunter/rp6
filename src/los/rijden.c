#include "rijden.h"

int
main() {
    rp6 motor = {.acceleration  = 2000,
                 .direction     = 1,
                 .speed         = 0,
                 .turnDirection = 0,
                 .turnIntensity = 2500 };
    init();

    while(1) {
        drive(&motor);
    }
}


int
drive() {
    uint64_t updateTimer = 0;

    if (updateTimer > rp6_timer()) {
        return 0;
    }

    /* updateTimer = rp6_timer() + (*m).speed; */

    OCR1A = 1000;
    OCR1B = 1000;

    return 0;
}

uint64_t
timer_counter(int inc) {
    static uint64_t i = 1;
    if (inc)
        return i++;
    return i;
}

uint64_t
rp6_timer() {
    return ((2048 * timer_counter(0)) + (TCNT0 * 2048 / 256));
}

void
init() {
    /* Enable global interrupts */
    sei();

    /* Enable overflow interrupts */
    TIMSK |= (1 << TOIE0);

    /* Set timer prescaler = FCPU/64 */
    TCCR0 |= (1 << CS00)|(1 << CS01);

    /* Initialize AVR timer */
    TCNT0 = 0;

    /* D4 & D5 on output (motors) */
    DDRD |= 0x30;
    /* C2 & C3 on output (motor directions) */
    DDRC |= 0x0C;

    /* Motor timer */
    TCCR1A |= (1 << COM1A1)|(1 << COM1B1)|(1 << WGM11);
    TCCR1B |= (1 << WGM13)|(1 << CS10);
    /* Top at 63999 (125hz) */
    ICR1 = 63999;

    /* Start compare registers at 0, no signal */
    OCR1A = 0;
    OCR1B = 0;
}

/* Set handler for TIMER0 overflow */
ISR(TIMER0_OVF_vect) {
    timer_counter(1);
}
