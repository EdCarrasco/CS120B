/*
 * npham014_lab11_part1.c
 *
 * Created: 5/13/2019 3:08:16 PM
 * Author : iatea
 */ 

#include <avr/io.h>
#include "bit.h"
#include "io.h"
#include "io.c"
//#include <avr/interrupt.h>
#include "timer.h"
#include <stdio.h>

// Returns '\0' if no key pressed, else returns char '1', '2', ... '9', 'A', ...
// If multiple keys pressed, returns leftmost-topmost one
// Keypad must be connected to port C
/* Keypad arrangement
        PC4 PC5 PC6 PC7
   col  1   2   3   4
row
PC0 1   1 | 2 | 3 | A
PC1 2   4 | 5 | 6 | B
PC2 3   7 | 8 | 9 | C
PC3 4   * | 0 | # | D
*/
unsigned char GetKeypadKey() {

	PORTC = 0xEF; // Enable col 4 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINC,0)==0) { return('1'); }
	if (GetBit(PINC,1)==0) { return('4'); }
	if (GetBit(PINC,2)==0) { return('7'); }
	if (GetBit(PINC,3)==0) { return('*'); }

	// Check keys in col 2
	PORTC = 0xDF; // Enable col 5 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINC,0)==0) { return('2'); }
	if (GetBit(PINC,1)==0) { return('5'); }
	if (GetBit(PINC,2)==0) { return('8'); }
	if (GetBit(PINC,3)==0) { return('0'); }

	// Check keys in col 3
	PORTC = 0xBF; // Enable col 6 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINC,0)==0) { return('3'); }
	if (GetBit(PINC,1)==0) { return('6'); }
	if (GetBit(PINC,2)==0) { return('9'); }
	if (GetBit(PINC,3)==0) { return('#'); }

	// Check keys in col 4	
	PORTC = 0x7F;
	asm("nop");
	if (GetBit(PINC,0)==0) { return('A'); }
	if (GetBit(PINC,1)==0) { return('B'); }
	if (GetBit(PINC,2)==0) { return('C'); }
	if (GetBit(PINC,3)==0) { return('D'); }

	return('\0'); // default value

}

typedef struct task {
	unsigned char state;
	unsigned long period;
	unsigned long elapsedTime;
	void (*TickFct)(unsigned char);
	} task;
	
void keypadSM(unsigned char curState) {
	
	switch (curState) {
		case '\0': PORTB = 0x1F; break; // All 5 LEDs on
		case '1': PORTB = 0x01; LCD_WriteData('1');break; // hex equivalent
		case '2': PORTB = 0x02; LCD_WriteData('2');break;
		case '3': PORTB = 0x03; LCD_WriteData('3');break;
		case '4': PORTB = 0x04; LCD_WriteData('4');break;
		case '5': PORTB = 0x05; LCD_WriteData('5');break;
		case '6': PORTB = 0x06; LCD_WriteData('6');break;
		case '7': PORTB = 0x07; LCD_WriteData('7');break;
		case '8': PORTB = 0x08; LCD_WriteData('8');break;
		case '9': PORTB = 0x09; LCD_WriteData('9');break;
		case 'A': PORTB = 0x0A; LCD_WriteData('A');break;
		case 'B': PORTB = 0x0B; LCD_WriteData('B');break;
		case 'C': PORTB = 0x0C; LCD_WriteData('C');break;
		case 'D': PORTB = 0x0D; LCD_WriteData('D');break;
		case '*': PORTB = 0x0E; LCD_WriteData('*');break;
		case '0': PORTB = 0x00; LCD_WriteData('0');break;
		case '#': PORTB = 0x0F; LCD_WriteData('#');break;
		default: PORTB = 0x1B; break; // Should never occur. Middle LED off.
	}
	
	
}

int main(void)
{
	task keypadTask;
	DDRA = 0x03; PORTA = 0xFC; //First 2 bits of porta used for LCD control
	DDRB = 0xFF; PORTB = 0x00; // PORTB set to output, outputs init 0s
	DDRC = 0xF0; PORTC = 0x0F; // PC7..4 outputs init 0s, PC3..0 inputs init 1s
	DDRD = 0xFF; PORTD = 0x00; //Set port d to output for the led display
	
	unsigned char x;
	keypadTask.TickFct = &keypadSM;
	LCD_init();
	TimerSet(100);
	TimerOn();
	while(1) {
		LCD_ClearScreen();
		LCD_Cursor(1);
		x = GetKeypadKey();
		if(x != '\0') {
			keypadTask.state = x;
		}
		keypadTask.TickFct(keypadTask.state);
		while(!TimerFlag);
		TimerFlag = 0;
	}
}
