#ifndef __AVR_ATmega32__
#define __AVR_ATmega32__
#endif
#include "rp6.h"
#include <avr/io.h>

void RP6_SetBlinker(RP6_Full * RP6) {
        RP6->blinkerCount = 20;
}

void RP6_SetSpeed(RP6_Full * RP6, char speed) {
        int newSpeed = 50 * (int)speed;
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
        }

        RP6->speed.cur += inc;

        if (RP6->speed.cur == RP6->speed.next) {
                RP6->update.speed = 0;
        }
}

void RP6_Execute(RP6_Full * RP6) {
        if (RP6->update.dir) {
                RP6_Execute_Direction(RP6);
        }
        if (RP6->blinkerCount > 20) {
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
        switch(RP6->dir) {
        case 'a':
                PORTC ^= (1 << PINC4); /* Toggle left blinker */
                PORTB &= ~(1 << PINB7); /* Disable right blinker */
                break;
        case 'd':
                PORTC &= ~(1 << PINC4); /* Disable left blinker */
                PORTB ^= (1 << PINB7);  /* Toggle right blinker */
                break;
        default:
                PORTC &= ~(1 << PINC4); /* Disable left blinker */
                PORTB &= ~(1 << PINB7); /* Disable right blinker */
                break;
        }
}

void RP6_Execute_Speed(RP6_Full * RP6) {
        OCR1A = RP6->speed.cur;
        OCR1B = RP6->speed.cur;
}
