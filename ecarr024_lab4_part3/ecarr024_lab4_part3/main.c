/*
 * ecarr024_lab4_part3.c
 *
 * Created: 4/13/2019 8:54:03 PM
 * Author : Edward-Laptop
 */ 

#include <avr/io.h>

enum Door_States {
	locked,
	pressedPound,
	releasedPound,
	pressedY,
	unlocked
} Door_State;

void lockSM(unsigned char *tmpB, unsigned char *tmpC, unsigned char buttonX, unsigned char buttonY, unsigned char buttonPound, unsigned char buttonInside) {
	switch(Door_State) { // transitions
		case locked:
			if (!buttonX && !buttonY && buttonPound) {
				Door_State = pressedPound;
			} else {
				Door_State = locked;
			}
			break;
		case pressedPound:
			if (!buttonX && !buttonY && buttonPound) {
				Door_State = pressedPound;
			} else if (!buttonX && !buttonY && !buttonPound) {
				Door_State = releasedPound;
			} else {
				Door_State = locked;
			}
			break;
		case releasedPound:
			if (!buttonX && !buttonY && !buttonPound) {
				Door_State = releasedPound;
			} else if (!buttonX && buttonY && !buttonPound) {
				Door_State = pressedY;
			} else {
				Door_State = locked;
			}
			break;
		case pressedY:
			Door_State = buttonInside ? locked : unlocked;
			break;
		case unlocked:
			Door_State = buttonInside ? locked : unlocked;
			break;
		default:
			Door_State = locked;
	}
	*tmpC = Door_State;
	
	switch (Door_State) { // actions
		case locked:
		case pressedPound:
		case releasedPound:
		case pressedY:
			*tmpB = 0;
			break;
		case unlocked:
			*tmpB = 1;
			break;
		default:
			*tmpB = 0;
			break;
	}
	return;
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // A input
	DDRB = 0xFF; PORTB = 0x00; // B output
	DDRC = 0xFF; PORTC = 0x00; // C output
	
	unsigned char buttonX = 0x00;
	unsigned char buttonY = 0x00;
	unsigned char buttonPound = 0x00;
	unsigned char buttonInside = 0x00;
	unsigned char tmpB = 0x00;
	unsigned char tmpC = 0x00;
    
    while (1) 
    {
		buttonX = PINA & 0x01;
		buttonY = (PINA & 0x02) >> 1;
		buttonPound = (PINA & 0x04) >> 2;
		buttonInside = (PINA & 0x80) >> 7;
		tmpB = 0x00;
		tmpC = 0xFF;
		
		lockSM(&tmpB, &tmpC, buttonX, buttonY, buttonPound, buttonInside);
		
		PORTB = tmpB;
		PORTC = tmpC;
    }
}

