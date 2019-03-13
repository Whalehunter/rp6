#include <avr/io.h>
#include <avr/interrupt.h>

#include "../includes/rp6aansluitingen.h"

typedef struct {
    uint16_t acceleration;
    int8_t direction;
    uint32_t speed;
    int8_t turnDirection;
    uint16_t turnIntensity;
} rp6;

/* Drive motor ***************************************************************/
/* int drive(rp6 *m); */
int drive();

/* Set timer *****************************************************************/
uint64_t rp6_timer();

/* Set initial values for dependencies ***************************************/
void init();

/* Initialise and return timer counter ***************************************/
uint64_t timer_counter(int inc);
