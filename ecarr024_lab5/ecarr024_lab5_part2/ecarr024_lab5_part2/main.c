/*
 * ecarr024_lab5_part2.c
 *
 * Created: 4/19/2019 7:37:17 PM
 * Author : Edward-Laptop
 */ 

#include <avr/io.h>

// Bit access function
unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
	return (b ? x | (0x01 << k) : x & ~(0x01 << k));
}
unsigned char GetBit(unsigned char x, unsigned char k) {
	return ((x & (0x01 << k)) != 0);
}

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
	DDRB = 0xFF; PORTB = 0x00; // B output
	
	volatile unsigned char tmpB = 7;
	volatile unsigned char buttonIncrease = 0x00;
	volatile unsigned char buttonDecrease = 0x00;
	Button_State = start;
	
	while (1)
	{
		buttonIncrease = (~PINA) & 0x01; // PA0
		buttonDecrease = ((~PINA) & 0x02) >> 1; // PA1
		
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
				tmpB = 7;
				break;
			case nonePressed:
			case increaseWait:
			case decreaseWait:
				// do nothing
				break;
			case increasePressed:
				if (tmpB < 9) tmpB++;
				break;
			case decreasePressed:
				if (tmpB > 0) tmpB--;
				break;
			case bothPressed:
				tmpB = 0;
				break;
			default:
				tmpB = 0xFF; // should never happen
			break;
		}
		
		// Write to output
		PORTB = tmpB;
	}
}
