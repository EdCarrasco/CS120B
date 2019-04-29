/*
 * ecarr024_lab7_part1.c
 *
 * Created: 4/26/2019 9:58:03 PM
 * Author : Edward-Laptop
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "io.h"

#define SET_BIT(p,i) ((p) |= (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) & (1 << (i)))

/*-------------------------------------------------------------------------*/

#define DATA_BUS PORTC		// port connected to pins 7-14 of LCD display
#define CONTROL_BUS PORTD	// port connected to pins 4 and 6 of LCD disp.
#define RS 6			// pin number of uC connected to pin 4 of LCD disp.
#define E 7			// pin number of uC connected to pin 6 of LCD disp.

/*-------------------------------------------------------------------------*/

void LCD_ClearScreen(void) {
	LCD_WriteCommand(0x01);
}

void LCD_init(void) {

	//wait for 100 ms.
	delay_ms(100);
	LCD_WriteCommand(0x38);
	LCD_WriteCommand(0x06);
	LCD_WriteCommand(0x0f);
	LCD_WriteCommand(0x01);
	delay_ms(10);
}

void LCD_WriteCommand (unsigned char Command) {
	CLR_BIT(CONTROL_BUS,RS);
	DATA_BUS = Command;
	SET_BIT(CONTROL_BUS,E);
	asm("nop");
	CLR_BIT(CONTROL_BUS,E);
	delay_ms(2); // ClearScreen requires 1.52ms to execute
}

void LCD_WriteData(unsigned char Data) {
	SET_BIT(CONTROL_BUS,RS);
	DATA_BUS = Data;
	SET_BIT(CONTROL_BUS,E);
	asm("nop");
	CLR_BIT(CONTROL_BUS,E);
	delay_ms(1);
}

void LCD_DisplayString( unsigned char column, const unsigned char* string) {
	LCD_ClearScreen();
	unsigned char c = column;
	while(*string) {
		LCD_Cursor(c++);
		LCD_WriteData(*string++);
	}
}

void LCD_Cursor(unsigned char column) {
	if ( column < 17 ) { // 16x1 LCD: column < 9
		// 16x2 LCD: column < 17
		LCD_WriteCommand(0x80 + column - 1);
		} else {
		LCD_WriteCommand(0xB8 + column - 9);	// 16x1 LCD: column - 1
		// 16x2 LCD: column - 9
	}
}

void delay_ms(int miliSec) //for 8 Mhz crystal

{
	int i,j;
	for(i=0;i<miliSec;i++)
	for(j=0;j<775;j++)
	{
		asm("nop");
	}
}

/************************************************************************/

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A = 125;    // Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}


/************************************************************************/

enum States {
	INIT,
	NONE,
	INCREASE,
	DECREASE,
	BOTH
} state;

unsigned char LED_SM(unsigned char buttonIncrease, unsigned char buttonDecrease) {
	static unsigned char count;
	switch(state) {
		case INIT:
			state = NONE;
			count = 0;
			break;
		case NONE:
			if (!buttonDecrease && !buttonIncrease) state = NONE;
			else if (!buttonDecrease && buttonIncrease) state = INCREASE;
			else if (buttonDecrease && !buttonIncrease) state = DECREASE;
			else if (buttonDecrease && buttonIncrease) state = BOTH;
			break;
		case INCREASE:
			if (!buttonDecrease && !buttonIncrease) state = NONE;
			else if (!buttonDecrease && buttonIncrease) state = INCREASE;
			else if (buttonDecrease && !buttonIncrease) state = DECREASE;
			else if (buttonDecrease && buttonIncrease) state = BOTH;
			break;
		case DECREASE:
			if (!buttonDecrease && !buttonIncrease) state = NONE;
			else if (!buttonDecrease && buttonIncrease) state = INCREASE;
			else if (buttonDecrease && !buttonIncrease) state = DECREASE;
			else if (buttonDecrease && buttonIncrease) state = BOTH;
			break;
		case BOTH:
			if (!buttonDecrease && !buttonIncrease) state = NONE;
			else if (!buttonDecrease && buttonIncrease) state = INCREASE;
			else if (buttonDecrease && !buttonIncrease) state = DECREASE;
			else if (buttonDecrease && buttonIncrease) state = BOTH;
			break;
		default:
			state = INIT;
	}
	
	switch (state) {
		case INIT:
		case NONE:
			// do nothing
			break;
		case INCREASE:
			count = (count < 9) ? count + 1 : 9;
			break;
		case DECREASE:
			count = (count > 0) ? count - 1 : 0;
			break;
		case BOTH:
			count = 0;
			break;
	}
	return count;
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // A is input
	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines
	
	LCD_init(); // Initializes the LCD display
	//LCD_DisplayString(1, "Hello World"); // Starting at position 1 on the LCD screen, writes Hello World
	
	state = NONE;
	unsigned char buttonIncrease = 0x00;
	unsigned char buttonDecrease = 0x00;
	unsigned char count = 0;
	unsigned long PERIOD = 1000; // ms
	TimerSet(PERIOD);
	TimerOn();
	
	while(1) {
		buttonIncrease = (~PINA) & 0x01;
		buttonDecrease = ((~PINA) & 0x02) >> 1;
		count = LED_SM(buttonIncrease, buttonDecrease);
		
		LCD_ClearScreen();
		LCD_Cursor(1);
		LCD_WriteData(count + '0');
		//unsigned char str = '0' + count;
		//LCD_DisplayString(1, str);
		
		PORTB = ~PINA;
		
		while (!TimerFlag); // wait for 1 PERIOD
		TimerFlag = 0;
	}
}
