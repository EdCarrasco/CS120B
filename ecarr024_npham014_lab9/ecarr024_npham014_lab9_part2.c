/*
 * Edward Carrasco	ecarr024@ucr.edu
 * Nathan Pham		npham014@ucr.edu
 * Lab Section: 022
 * 
 * Assignment: Lab 9 Exercise 2
 * 
 * Description: 
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
		if (!frequency) { TCCR0B &= 0x08; } //stops timer/counter
		else { TCCR0B |= 0x03; } // resumes/continues timer/counter
		
		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) { OCR0A = 0xFFFF; }
		
		// prevents OCR0A from underflowing, using prescaler 64                    // 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) { OCR0A = 0x0000; }
		
		// set OCR3A based on desired frequency
		else { OCR0A = (short)(8000000 / (128 * frequency)) - 1; }

		TCNT0 = 0; // resets counter
		current_frequency = frequency; // Updates the current frequency
	}
}

void PWM_on() {
	TCCR0A = (1 << COM0A0);
	// COM3A0: Toggle PB3 on compare match between counter and OCR0A
	TCCR0B = (1 << WGM02) | (1 << CS01) | (1 << CS00);
	// WGM02: When counter (TCNT0) matches OCR0A, reset counter
	// CS01 & CS30: Set a prescaler of 64
	set_PWM(0);
}

void PWM_off() {
	TCCR0A = 0x00;
	TCCR0B = 0x00;
}

enum States {
	START,
	INCREASE_PRESSED,
	DECREASE_PRESSED
} state;

enum ToggleStates {
	RELEASED,
	PRESSED
} toggleState;

unsigned char ToggleSM(unsigned char buttonToggle) {
	static unsigned char toggle = 0;
	switch(toggleState) {
		case RELEASED:
			if (buttonToggle) {
				state = PRESSED;
				toggle = !toggle;
			} else {
				state = RELEASED;
			}
			break;
		case PRESSED:
			state = (buttonToggle) ? PRESSED : RELEASED;
			break;
		default:
			state = RELEASED;
			toggle = 0;
			break;
	}
	
	switch(toggleState) {
		case RELEASED:
		case PRESSED:
			// do nothing
			break;
		default:
			break;
	}
	
	return toggle;
}

double FrequencySM(unsigned char buttonIncrease, unsigned char buttonDecrease, unsigned char toggle) {
	static unsigned char frequencyIndex = 0;
	switch(state) {
		case START:
			if (toggle && buttonIncrease && !buttonDecrease) {
				state = INCREASE_PRESSED;
				frequencyIndex++;
				if (frequencyIndex > 7) frequencyIndex = 7;
			} else if (toggle && !buttonIncrease && buttonDecrease) {
				state = DECREASE_PRESSED;
				frequencyIndex--;
				if (frequencyIndex < 0) frequencyIndex = 0;
			} else {
				state = START;
			}
			break;
		case INCREASE_PRESSED:
			state = (buttonIncrease) ? INCREASE_PRESSED : START;
			break;
		case DECREASE_PRESSED:
			state = (buttonDecrease) ? DECREASE_PRESSED : START;
			break;
		default:
			state = START;
	}
	
	switch(state) {
		case START:
		case INCREASE_PRESSED:
		case DECREASE_PRESSED:
			// do nothing
			break;
		default:
			break;
	}
	
	return frequencyIndex;
}
struct Frequency {
	double C4, D4, E4, F4, G4, A4, B4, C5;
};

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // A is input
	DDRB = 0xFF; PORTB = 0x00; // B3 is speaker, others are regular output
	
	unsigned char buttonToggle = 0x00;
	unsigned char buttonIncrease = 0x00;
	unsigned char buttonDecrease = 0x00;
	
	unsigned char toggle = 0;
	unsigned char index;
	
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
		frequency.C4,
		frequency.D4,
		frequency.E4,
		frequency.F4,
		frequency.G4,
		frequency.A4,
		frequency.B4,
		frequency.C5
	};
	
	PWM_on();
	
	state = START;
	toggleState = RELEASED;
	
	while (1)
	{
		buttonToggle = ~PINA & 0x01;
		buttonIncrease = (~PINA & 0x02) >> 1;
		buttonDecrease = (~PINA & 0x04) >> 2;
		
		toggle = ToggleSM(buttonToggle);
		index = FrequencySM(buttonIncrease,buttonDecrease, toggle);
		
		if (toggle) {
			set_PWM(frequencies[index]);
		} else {
			set_PWM(0);
		}
	}
}

