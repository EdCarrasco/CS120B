/*
 * npham014_lab6_part1.c
 *
 * Created: 4/22/2019 4:08:33 PM
 * Author : iatea
 */ 

#include <avr/io.h>
volatile unsigned char TimerFlag = 0; //TimerISR() sets this to 1. C programmer should clear to 0.

//Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of lms ticks

void TimerOn() {
	//AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B; //Bit 3 = 0: CTC mode (clear timer on compare)
		//b2b1b0 = 011: pre-scaler/64
		//0001011: 0x0B
		//So, 8 MHz clock or 8million /64 = 125k ticks per sec
		//Thus TCNT1 register will count at 124k ticks per sec
	
	//AVR output compare reigster OCR1A
	OCR1A = 125; //Timer interrupt will be generated when TCNT1 == OCR1A
		//We want 1ms tick. .001 s * 125k tick/s = 125
		//So when TCNT1 is 125 then 1 ms has passed. Thus ew compare to 125.
		
	//AVR timer interrupt mask register
	TIMSK1=0x02; //bit1: OCIE1A -- enables compare match interrupt
	
	//Initialize avr counter
	TCNT1=0;
	
	_avr_timer_cntcurr = _avr_timer_M;
	//TimerISR will be called every _avr_timer_cntcurr milliseconds
	
	//Enable global interrupts
	SREG |= 0x80; //0x80: 10000000
	
	
}

void TimerOff() {
	TCCR1B = 0x00; //b2b1b0 = 000 means timer off
}

void TimerISR() {
	TimerFlag = 1;
}

//in our approach, the c programmer does not touch this ISR but rather TimerISR()

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSEt(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

int main(void)
{
    DDRB = 0xFF; PORTB = 0x00;
	TimerSet(1000);
	TimerOn();
	unsigned char tmpB = 0x00;
    while (1) 
    {
		//User code (ie synchSM calls)
		tmpB = ~tmpB;//Toggle PORTB; temporary, bad coding sytle
		PORTB = tmpB;
		while(!TimerFlag); //Wait for timer to finish
		TimerFlag = 0;
    }
}

