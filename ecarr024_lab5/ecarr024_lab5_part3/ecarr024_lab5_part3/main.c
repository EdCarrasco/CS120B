/*
 * ecarr024_lab5_part3.c
 *
 * Created: 4/19/2019 7:55:47 PM
 * Author : Edward-Laptop
 */ 

#include <avr/io.h>

enum Festive_States{
	released_config1,
	pressed_config2,
	released_config2,
	pressed_config3,
	released_config3,
	pressed_config4,
	released_config4,
	pressed_config5,
	released_config5,
	pressed_config6,
	released_config6,
	pressed_config1
} state;

unsigned char festive_SM(unsigned char button) {
	unsigned char led = 0x00;
	
	switch(state) {
		case released_config1:
			state = button ? pressed_config2 : released_config1;
			break;
		case pressed_config2:
			state = button ? pressed_config2 : released_config2;
			break;
		case released_config2:
			state = button ? pressed_config3 : released_config2;
			break;
		case pressed_config3:
			state = button ? pressed_config3 : released_config3;
			break;
		case released_config3:
			state = button ? pressed_config4 : released_config3;
			break;
		case pressed_config4:
			state = button ? pressed_config4 : released_config4;
			break;
		case released_config4:
			state = button ? pressed_config5 : released_config4;
			break;
		case pressed_config5:
			state = button ? pressed_config5 : released_config5;
			break;
		case released_config5:
			state = button ? pressed_config6 : released_config5;
			break;
		case pressed_config6:
			state = button ? pressed_config6 : released_config1;
			break;
		default:
			state = released_config1;
	}
	
	switch(state) {
		case pressed_config1:
		case released_config1:
			led = 0x55; // .*.*.*.*
			break;
		case pressed_config2:
		case released_config2:
			led = 0x2A; // ..*.*.*.
			break;
		case pressed_config3:
		case released_config3:
			led = 0x14; // ...*.*..
			break;
		case pressed_config4:
		case released_config4:
			led = 0x08; // ....*...
			break;
		case pressed_config5:
		case released_config5:
			led = 0x14; // ...*.*..
			break;
		case pressed_config6:
		case released_config6:
			led = 0x2A; // ..*.*.*.
			break;
		default:
			led = 0xFF; // should never happen
	}
	
	return led;
}

int main(void)
{
    DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	unsigned char button = 0x00;
	unsigned char led = 0x00;
	
	state = released_config1;
	
    while (1) 
    {
		button = (~PINA) & 0x01;
		led = festive_SM(button);
		PORTB = led;
    }
}

