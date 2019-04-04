#ifndef RP6_H
#define RP6_H
#include <avr/io.h>

/* If a setting needs updating in while loop *********************************/
typedef struct {
	int speed;
	int dir;
} RP6_Update;

/* Speed settings RP6 ********************************************************/
typedef struct {
	int cur;
	int prev;
	int next;
} RP6_Speed;

/* Full settings RP6 *********************************************************/
typedef struct {
	RP6_Speed speed;
	char dir;
	int blinkerCount;
	RP6_Update update;
} RP6_Full;

void RP6_SetBlinker(RP6_Full * RP6); /* Update RP6 knipperlicht */
void RP6_SetSpeed(RP6_Full * RP6, char speed); /* Update RP6 speed geleidelijk */
void RP6_SetDirection(RP6_Full * RP6, char direction); /* Update RP6 richting */
char RP6_GetDirection(RP6_Full * RP6);		       /* Get current direction of RP6 */
void RP6_SetCurrentSpeed(RP6_Full * RP6);	 /* Set acceleration for next item */

/*****************************************************************************/
/*      De "Execute" functies zijn in princepe alleen voor RP6_Execute.      */
/*****************************************************************************/
void RP6_Execute(RP6_Full * RP6);		 /* Execute RP6 settings */
void RP6_Execute_Direction(RP6_Full * RP6);	 /* Execute drive direction */
void RP6_Execute_Blinker(RP6_Full * RP6);	 /* Execute blinker toggles */
void RP6_Execute_Speed(RP6_Full * RP6); /* Execute RP6 speed */

#endif /* RP6_H */
