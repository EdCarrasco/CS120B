/*
 * ecarr024_lab4_part2.c
 *
 * Created: 4/10/2019 4:21:26 PM
 * Author : Edward-Laptop
 */ 

#include <avr/io.h>


enum Button_States {
	start,
	nonePressed,
	increasePressed,
	increaseWait,
	decreasePressed,
	decreaseWait,
	bothPressed
} Button_State;

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // A input
	DDRC = 0xFF; PORTC = 0x00; // C output
	
	volatile unsigned char tmpC = 7;
	volatile unsigned char buttonIncrease = 0x00;
	volatile unsigned char buttonDecrease = 0x00;
	Button_State = start;
	
	while (1)
	{
		buttonIncrease = PINA & 0x01; // PA0
		buttonDecrease = (PINA & 0x02) >> 1; // PA1
		
		switch(Button_State) { // transitions
			case start:
				Button_State = nonePressed;
				break;
			case nonePressed:
				if (buttonIncrease && buttonDecrease) {
					Button_State = bothPressed;
				} else if (buttonIncrease) {
					Button_State = increasePressed;
				} else if (buttonDecrease) {
					Button_State = decreasePressed;
				} else {
					Button_State = nonePressed;
				}
				break;
			case increasePressed:
				Button_State = increaseWait;
				break;
			case decreasePressed:
				Button_State = decreaseWait;
				break;
			case increaseWait:
				if (buttonIncrease && buttonDecrease) {
					Button_State = bothPressed;
				} else if (buttonIncrease) {
					Button_State = increaseWait;
				} else if (buttonDecrease) {
					Button_State = bothPressed; // maybe go to nonePressed or decreasePressed instead?
				} else {
					Button_State = nonePressed;
				}
				break;
			case decreaseWait:
				if (buttonIncrease && buttonDecrease) {
					Button_State = bothPressed;
				} else if (buttonIncrease) {
					Button_State = bothPressed; // maybe go to nonePressed or increasePressed instead?
				} else if (buttonDecrease) {
					Button_State = decreaseWait;
				} else {
					Button_State = nonePressed;
				}
				break;
			case bothPressed:
				if (buttonIncrease && buttonDecrease) {
					Button_State = bothPressed;
				} else if (buttonIncrease) {
					Button_State = bothPressed;
				} else if (buttonDecrease) {
					Button_State = bothPressed;
				} else {
					Button_State = nonePressed;
				}
				break;
			default:
				Button_State = start;
				break;
		}
		
		switch(Button_State) { // actions
			case start:
				tmpC = 7;
				break;
			case nonePressed:
			case increaseWait:
			case decreaseWait:
				// do nothing
				break;
			case increasePressed:
				if (tmpC < 9) tmpC++;
				break;
			case decreasePressed:
				if (tmpC > 0) tmpC--;
				break;
			case bothPressed:
				tmpC = 0;
				break;
			default:
				tmpC = 0xFF; // should never happen
				break;
		}
		
		// Write to output
		PORTC = tmpC;
	}
}
