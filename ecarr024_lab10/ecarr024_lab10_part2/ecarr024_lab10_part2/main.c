/*
 * Edward Carrasco	ecarr024@ucr.edu
 * Nathan Pham		npham014@ucr.edu
 * Lab Section: 022
 * 
 * Assignment: Lab 10 Exercise 2
 * 
 * Description: 
 */

#include <avr/io.h>
#include <avr/interrupt.h>

/************************************************************************/
/* TASK                                                                 */
/************************************************************************/

typedef struct _Task{
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFunction)(int);
} Task;

const unsigned char tasksSize = 3;
Task tasks[3];
unsigned long tasksPeriod = 100; // ms

/************************************************************************/
/* TIMER                                                                */
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
	unsigned char i;
	for (i = 0; i < tasksSize; i++) {
		if (tasks[i].elapsedTime >= tasks[i].period) {
			tasks[i].state = tasks[i].TickFunction(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += tasksPeriod;
	}
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
/* STATE MACHINES                                                       */
/************************************************************************/

unsigned char blinkOut = 0x00;
unsigned char threeOut = 0x00;

enum BL_States {BL_Start, BL_LEDOff, BL_LEDOn} bl_state;
enum TL_States {TL_Start, TL_Seq0, TL_Seq1, TL_Seq2} tl_state;
enum CL_States {CL_Start} cl_state;

int BlinkLED_Tick(int state) {
	switch(state) { // transitions
		case BL_Start:
			state = BL_LEDOff;
		case BL_LEDOff:
			state = BL_LEDOn;
			break;
		case BL_LEDOn:
			state = BL_LEDOff;
			break;
		default:
			state = BL_Start;
			break;
	}
	switch(state) { // actions
		case BL_Start:
			blinkOut = 0;
			break;
		case BL_LEDOff:
			blinkOut = 0;
			break;
		case BL_LEDOn:
			blinkOut = 1;
			break;
		default:
			break;
	}
	return state;
}

int ThreeLED_Tick(int state) {
	switch(state) { // transitions
		case TL_Start:
			state = TL_Seq0;
			break;
		case TL_Seq0:
			state = TL_Seq1;
			break;
		case TL_Seq1:
			state = TL_Seq2;
			break;
		case TL_Seq2:
			state = TL_Seq0;
			break;
		default:
			state = TL_Start;
			break;
	}
	switch(state) { // actions
		case TL_Start:
			threeOut = 0x00;
			break;
		case TL_Seq0:
			threeOut = 0x02;
			break;
		case TL_Seq1:
			threeOut = 0x04;
			break;
		case TL_Seq2:
			threeOut = 0x08;
			break;
		default:
			threeOut = 0x00;
			break;
	}
	return state;
}

int CombineLED_Tick(int state) {
	switch(state) {
		case CL_Start:
			state = CL_Start;
			break;
		default:
			state = CL_Start;
			break;
	}
	switch(state) {
		case CL_Start:
			PORTB = 0x00 | blinkOut | threeOut;
			break;
		default:
			PORTB = 0x00;
			break;
	}
	return state;
}

/************************************************************************/
/* MAIN                                                                 */
/************************************************************************/

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // A is input
	DDRB = 0xFF; PORTB = 0x00; // B is output
	
    unsigned char i = 0;
    tasks[i].state = BL_LEDOff;
    tasks[i].period = 1000;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFunction = &BlinkLED_Tick;
    i++;
    tasks[i].state = TL_Seq0;
    tasks[i].period = 300;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFunction = &ThreeLED_Tick;
	i++;
	tasks[i].state = CL_Start;
	tasks[i].period = 100;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFunction = &CombineLED_Tick;
	
	TimerSet(tasksPeriod);
	TimerOn();
	
    while (1) {
		//Sleep();
    }
}