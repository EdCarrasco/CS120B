/*
 * npham014_Lab4_Part2.c
 *
 * Created: 4/12/2019 12:55:36 PM
 * Author : Nathan Pham
 */ 

#include <avr/io.h>
enum state {NONE,INCREASING,DECREASING,INCPUSHED,DECPUSHED,BOTHPUSHED} currState = NONE;

unsigned char counterSM(unsigned char incr, unsigned char decr, unsigned char output) {
	switch(currState) {
		case NONE:
			if(incr == 1) {
				currState = INCREASING;
			}
			else if(decr == 1) {
				currState = DECREASING;
			}
			else {
				currState = NONE;
			}
			break;
		case INCREASING:
			currState = INCPUSHED;
			break;
		case DECREASING:
			currState = DECPUSHED;
			break;
		case INCPUSHED:
			if(incr != 1) {
				currState = NONE;
			}
			else if(decr == 1) {
				currState = BOTHPUSHED;
			}
			else {
				currState = INCPUSHED;
			}
			break;
		case DECPUSHED:
			if(decr != 1) {
				currState = NONE;
			}
			else if(incr == 1) {
				currState = BOTHPUSHED;
			}
			else {
				currState = DECPUSHED;
			}
			break;
		case BOTHPUSHED:
			if(incr != 1) {
				currState = DECPUSHED;
			}
			else if(decr != 1) {
				currState = INCPUSHED;
			}
			else {
				currState = BOTHPUSHED;
			}
			break;
		default:
			currState = NONE;
			break;	
	}
	
	switch (currState) {
		case INCREASING:
			return output + 1;
			break;
		case DECREASING:
			return output - 1;
			break;
		case BOTHPUSHED:
			return 0;
			break;
		default:
			return output;
			break;
	}
	
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	unsigned char incButton = 0x00;
	unsigned char decButton = 0x00;
	unsigned char currVal = 0x07;
    while (1) 
    {
		incButton = PINA % 2;
		decButton = (PINA>>1) %2;
		currVal = counterSM(incButton,decButton, currVal);
		PORTC = currVal;
    }
}

