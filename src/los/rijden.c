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

void drive();

int main() {

    rp6 motor = {.acceleration = 2000,
                 .direction = 1,
                 .speed = 0,
                 .turnDirection = 0,
                 .turnIntensity = 2500 };


    while(1) {

    }
}


void drive() {

}
