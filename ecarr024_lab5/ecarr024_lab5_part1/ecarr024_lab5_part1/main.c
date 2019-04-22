/*
 * ecarr024_lab5_part1.c
 *
 * Created: 4/19/2019 7:33:51 PM
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

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // A is input
	DDRB = 0xFF; PORTB = 0x00; // B is output
	
	volatile unsigned char fuelValue = 0x00;
	volatile unsigned char tmpB = 0x00;
    
    while (1) 
    {
		
		// Read input
		fuelValue = (~PINA) & 0x0F; // PA3..PA0
		tmpB = 0x00;
		
		// Compute
		
		//	level 1-2	PC5										 0000 0011 or 0000 0001
		//	level 3-4	PC5 PC4									 0000 0100 or 0000 0011
		//	level 5-6	PC5 PC4 PC3								 0000 0110 or 0000 0101
		//	level 7-9	PC5 PC4 PC3 PC2				0000 1001 or 0000 1000 or 0000 0111
		//	level 10-12	PC5 PC4 PC3 PC2 PC1			0000 1100 or 0000 1011 or 0000 1010
		//	level 13-15 PC5 PC4 PC3 PC2 PC1 PC0		0000 1111 or 0000 1110 or 0000 1101
		
		if (fuelValue >= 13) tmpB = SetBit(tmpB, 0, 1);
		if (fuelValue >= 10) tmpB = SetBit(tmpB, 1, 1);
		if (fuelValue >= 7)  tmpB = SetBit(tmpB, 2, 1);
		if (fuelValue >= 5)  tmpB = SetBit(tmpB, 3, 1);
		if (fuelValue >= 3)  tmpB = SetBit(tmpB, 4, 1);
		if (fuelValue >= 1)  tmpB = SetBit(tmpB, 5, 1);
		if (fuelValue <= 4)  tmpB = SetBit(tmpB, 6, 1); // Low fuel icon
		
		// Write output
		
		PORTB = tmpB; // 0000 0101
    }
}
