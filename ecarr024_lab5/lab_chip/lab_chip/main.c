/*
 * lab_chip.c
 *
 * Created: 4/17/2019 4:16:57 PM
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

/*
int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // A is input
	DDRB = 0xFF; PORTB = 0x00; // B is output
	volatile unsigned char fuelValue = 0x00;
	volatile unsigned char tmpB = 0x00;
    
    while (1) 
    {
		// Read input
		fuelValue = PINA & 0x0F; // rightmost 4 pins
		tmpB = 0x00;
		
		// Compute
		
		//	level 1-2	PC5
		//	level 3-4	PC5 PC4
		//	level 5-6	PC5 PC4 PC3
		//	level 7-9	PC5 PC4 PC3 PC2
		//	level 10-12	PC5 PC4 PC3 PC2 PC1
		//	level 13-15 PC5 PC4 PC3 PC2 PC1 PC0
		
		if (fuelValue >= 13) tmpB = SetBit(tmpB, 0, 1);
		if (fuelValue >= 10) tmpB = SetBit(tmpB, 1, 1);
		if (fuelValue >= 7)  tmpB = SetBit(tmpB, 2, 1);
		if (fuelValue >= 5)  tmpB = SetBit(tmpB, 3, 1);
		if (fuelValue >= 3)  tmpB = SetBit(tmpB, 4, 1);
		if (fuelValue >= 0)  tmpB = SetBit(tmpB, 5, 1);
		if (fuelValue <= 4)  tmpB = SetBit(tmpB, 6, 1); // Low fuel icon
		
		// Write output
		//tmpB = 0x40;
		PORTB = tmpB; // 0110 0000
    }
}
*/



int main(void)
{
	DDRA = 0x00; PORTA = 0x00; // A is input
    DDRB = 0xFF; PORTB = 0xFF; // B is output
	//DDRC = 0xFF; PORTC = 0x00; // C is output
	
	unsigned char led = 0x00;
	unsigned char button1 = 0x00;
	unsigned char button2 = 0x00;
	
    while (1) 
    {
		//button1 = ((~PINA) & 0x01); // A0
		button1 = ((PINA) & 0x01);
		button2 = (~PINA & 0x02) >> 1;
		led = 0x00;
		/*
		if (button1) {
			led = (led & 0xFC) | 0x01; // sets B to bbbbbb01
		} else {
			led = (led & 0xFC) | 0x02; // sets B to bbbbbb10
		}*/
		/*
		if (button1) {
			led = 0x01;
		}
		if (button2) {
			led = 0x02;
		}
		*/
		led = button1;
		//PORTB = led;
		//PORTB = button2 | button1;
		PORTB = ~led;
		//PORTC = 0xFF; // C0 always on
    }
}
