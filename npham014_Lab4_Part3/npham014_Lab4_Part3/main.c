/*
 * npham014_Lab4_Part3.c
 *
 * Created: 4/12/2019 1:42:39 PM
 * Author : Nathan Pham
 */ 

#include <avr/io.h>
enum state {LOCKED,PND,WAITY,UNLOCK,WAITLOCK,LOCKING} currState = LOCKED;

void lockSM(unsigned char x, unsigned char y, unsigned char pnd, unsigned char lock) {
	unsigned char output 0x00;
	switch(currState) {
		case LOCKED:
			if(pnd) {
				currState = PND;
			}
			else {
				currState = LOCKED;
			}
			break;
		case PND: 
			if(!pnd) {
				currState = WAITY;
			}
			else {
				currState = PND;
			}
			break;
		case WAITY:
			if(y) {
				currState = UNLOCK;
			}	
			else if(!x && !pnd){
				currState = WAITY;
			}
			else {
				currState = LOCKED;
			}
			break;
		case UNLOCK:
			if(y) {
				currState = UNLOCK;
			}
			else {
				currState = WAITLOCK;
			}
			break;
		case WAITLOCK:
			if(lock) {
				currState = LOCKING;
			}
			else {
				currState = WAITLOCK;
			}
			break;
		case LOCKING:
			if(lock) {
				currState = LOCKING;
			}
			else {
				currState = LOCKED;
			}
	}
	
	switch(currState) {
		case UNLOCK:
			PORTB = 0x01;
			return;
			break;
		case LOCKING:
			PORTB = 0x00;
			return;
			break;
		default:
			return;
	}
}


int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	unsigned char xButton = 0x00;
	unsigned char yButton = 0x00;
	unsigned char pndButton = 0x00;
	unsigned char lckButton = 0x00;
	unsigned char pushedCount = 0x00;
	unsigned char tempA = PINA;
    /* Replace with your application code */
    while (1) 
    {
		tempA = PINA;
		pushedCount = 0x00;
		xButton = PINA % 2;
		yButton = (PINA >> 1) % 2;
		pndButton = (PINA >> 2) % 2;
		lckButton = (PINA >> 7) % 2;
		
		while(tempA) {
			pushedCount += tempA & 0x01;
			tempA >>= 1;
		}
		if(pushedCount <= 1) {
			lockSM(xButton,yButton,pndButton,lckButton);
		}
		
    }
}

