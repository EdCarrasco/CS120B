/*
 * npham014_lab9_part3.c
 *
 * Created: 5/8/2019 1:43:08 PM
 * Author : iatea
 */ 

#include <avr/io.h>

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
		
		// prevents OCR3A from underflowing, using prescaler 64					// 31250 is largest frequency that will not result in underflow
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
	TimerFlag++;
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

struct Frequency {
	double C4, D4, E4, F4, G4, A4, B4, C5,E5,F5,D5;
};

void playSong() {
		struct Frequency frequency;
		frequency.C4 = 261.63;
		frequency.D4 = 293.66;
		frequency.E4 = 329.63;
		frequency.F4 = 349.23;
		frequency.G4 = 392.00;
		frequency.A4 = 440.00;
		frequency.B4 = 493.88;
		frequency.C5 = 523.25;
		frequency.D5 = 587.33;
		frequency.E5 = 659.26;
		frequency.F5 = 698.46;
		
	double song[23] = {
		/*1*/frequency.F4,frequency.A4,frequency.F4,frequency.F4,frequency.B4,frequency.F4,frequency.E4,
		/*1*/frequency.F4,frequency.C4,frequency.F4,frequency.F4,frequency.D5,frequency.C4,frequency.A4,
		frequency.F4,frequency.C4,frequency.F5, frequency.F4, frequency.E4,frequency.E4,frequency.C4,frequency.G4,/*3*/frequency.F4,
	};
	double noteLengths [23] = {
		500,250,250,250,250,250,250,
		500,250,250,250,250,250,250,
		250,250,250,250,250,250,250,250,750
	}; //ms
	
	PWM_on();
	set_PWM(0);
	for(unsigned char i = 0; i < 23; i++) {//Loop through each note
		set_PWM(song[i]);//Set pwm to the frequency of the current note
		while(TimerFlag < noteLengths[i]);//Wait for that note's length
		TimerFlag = 0;//Reset timer
		set_PWM(0);//turn off pwm
		while(TimerFlag < 150);//Wait 150 ms between notes
		TimerFlag = 0;//reset timer
	}
}



int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	unsigned char button = PINA & 0x01;
	TimerSet(1);
	TimerOn();
    /* Replace with your application code */
    while (1) 
    {
		if(button) {
			playSong();
		}
    }
}

