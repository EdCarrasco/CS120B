/*
 * Edward Carrasco	ecarr024@ucr.edu
 * Nathan Pham		npham014@ucr.edu
 * Lab Section: 022
 * 
 * Assignment: Lab 10 Exercise 4 (challenge)
 * 
 * Description: 
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include "io.h"

/************************************************************************/
/* TASK                                                                 */
/************************************************************************/

typedef struct _Task{
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFunction)(int);
} Task;

const unsigned char tasksSize = 6;
Task tasks[6];
unsigned long tasksPeriod = 2; // ms

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
/* BUZZER                                                                     */
/************************************************************************/

// 0.954 hz is lowest frequency possible with this function,
// based on settings in PWM_on()
// Passing in 0 as the frequency will stop the speaker from generating sound
void set_PWM(double frequency) {
	static double current_frequency; // Keeps track of the currently set frequency
	// Will only update the registers when the frequency changes, otherwise allows
	// music to play uninterrupted.
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; } //stops timer/counter
		else { TCCR3B |= 0x03; } // resumes/continues timer/counter
		
		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		
		// prevents OCR3A from underflowing, using prescaler 64                    // 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) { OCR3A = 0x0000; }
		
		// set OCR3A based on desired frequency
		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }

		TCNT3 = 0; // resets counter
		current_frequency = frequency; // Updates the current frequency
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	// COM3A0: Toggle PB6 on compare match between counter and OCR3A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	// WGM32: When counter (TCNT3) matches OCR3A, reset counter
	// CS31 & CS30: Set a prescaler of 64
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

/************************************************************************/
/* STATE MACHINES                                                       */
/************************************************************************/

unsigned char blinkOut = 0x00;
unsigned char threeOut = 0x00;

enum BL_States {BL_Start, BL_LEDOff, BL_LEDOn} bl_state;
enum TL_States {TL_Start, TL_Seq0, TL_Seq1, TL_Seq2} tl_state;
enum CL_States {CL_Start} cl_state;
	
enum BTN_States {BTN_Released, BTN_Pressed} button_state;
enum BTNF_States {BTNF_None, BTNF_Up, BTNF_Down, BTNF_Both};
enum SPK_States {SPK_Start, SPK_Low, SPK_High} speaker_state;

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
//			PORTB = 0x00 | blinkOut | threeOut;
			break;
		default:
//			PORTB = 0x00;
			break;
	}
	return state;
}

unsigned char isSpeakerOn = 0;
unsigned char buttonSpeaker;

int ButtonSpeaker_Tick(int state) {
	buttonSpeaker = (~PINA & 0x04) >> 2;
	switch(state) {
		case BTN_Released:
			state = (buttonSpeaker) ? BTN_Pressed : BTN_Released;
			break;
		case BTN_Pressed:
			state = (buttonSpeaker) ? BTN_Pressed : BTN_Released;
			break;
		default:
			state = BTN_Released;
			break;
	}
	switch(state) {
		case BTN_Released:
			isSpeakerOn = 0;
			break;
		case BTN_Pressed:
			isSpeakerOn = 1;
			PORTB |= 0x10;
			break;
		default:
			isSpeakerOn = 0;
			break;
	}
	return state;
}

unsigned char buttonUp;
unsigned char buttonDown;
unsigned char frequencyIndex = 0;
struct Frequency {
	double C4, D4, E4, F4, G4, A4, B4, C5;
};


int ButtonSpeakerFrequency_Tick(int state) {
	buttonUp = (~PINA & 0x01);
	buttonDown = (~PINA & 0x02) >> 1;
	
	PORTB = frequencyIndex;

	switch(state) { // transitions
		case BTNF_None:
			if (buttonUp && !buttonDown) {
				state = BTNF_Up;
				if (frequencyIndex < 7) {
					frequencyIndex++;
				}
			} else if (!buttonUp && buttonDown) {
				state = BTNF_Down;
				if (frequencyIndex > 0) frequencyIndex--;
			} else if (buttonUp && buttonDown) {
				state = BTNF_Both;
			} else {
				state = BTNF_None;
			}
			break;
		case BTNF_Up:
		case BTNF_Down:
		case BTNF_Both:
			if (buttonUp && !buttonDown) {
				state = BTNF_Up;
			} else if (!buttonUp && buttonDown) {
				state = BTNF_Down;
			} else if (buttonUp && buttonDown) {
				state = BTNF_Both;
			} else {
				state = BTNF_None;
			}
			break;
		default:
			state = BTNF_None;
	}
	switch(state) { // actions
		case BTNF_None:
		case BTNF_Up:
		case BTNF_Down:
		case BTNF_Both:
			// do nothing
			break;
		default:
			break;
	}
	return state;
}

int Speaker_Tick(int state) {
	struct Frequency frequency;
	frequency.C4 = 261.63;
	frequency.D4 = 293.66;
	frequency.E4 = 329.63;
	frequency.F4 = 349.23;
	frequency.G4 = 392.00;
	frequency.A4 = 440.00;
	frequency.B4 = 493.88;
	frequency.C5 = 523.25;

	double frequencies[8] = {
		500,//frequency.C4,
		600,//frequency.D4,
		700,//frequency.E4,
		800,//frequency.F4,
		900,//frequency.G4,
		1000,//frequency.A4,
		1100,//frequency.B4,
		1200//frequency.C5
	};
	
	switch(state) { // transitions
		case SPK_Low:
			state = (buttonSpeaker) ? SPK_High : SPK_Low;
			break;
		case SPK_High:
			state = SPK_Low;
			break;
		default:
			state = SPK_Low;
			break;
	}
	switch(state) { // actions
		case SPK_Low:
			set_PWM(0);
			break;
		case SPK_High:
			set_PWM(frequencies[frequencyIndex]);
			break;
		default:
			set_PWM(0);
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
	i++;
	tasks[i].state = BTN_Released;
	tasks[i].period = 2;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFunction = &ButtonSpeaker_Tick;
	i++;
	tasks[i].state = SPK_Low;
	tasks[i].period = 2;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFunction = &Speaker_Tick;
	i++;
	tasks[i].state = BTNF_None;
	tasks[i].period = 2;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFunction = &ButtonSpeakerFrequency_Tick;
	
	
	TimerSet(tasksPeriod);
	PWM_on();
	TimerOn();

    while (1) {
		
		//Sleep();
    }
}