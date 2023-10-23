/*
 * course work.cpp
 *
 * Created: 23/10/2023 02:23:06
 * Author : RASHID
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

void latch(){
	PORTF &= ~(1 << PF6);
	_delay_ms(10);
	PORTF |= (1 << PF6);
	_delay_ms(10);
}

void displayMessage(char *message){
	
	PORTF &= ~(1 << PF4); //command mode
	latch();
	
	PORTH = 0b00000001; //clear screen
	latch();
	
	
	PORTF |= (1 << PF4); //data mode
	PORTF &= ~(1 << PF5); //write mode
	
	latch();
	int i;
	
	for (i = 0; message[i] != '\0'; i++)
	{
		PORTH = message[i];
		latch();
	}
}

int main(void)
{
    /* Replace with your application code */
	EIMSK |= (1 << INT0); //register the int 0 pin
	DDRJ = 0xff;
	DDRD = 0x00;
	DDRH = 0xff;
	DDRF = 0xff;
	sei();
	
	PORTF &= ~(1 << PF4); //command mode
	PORTF &= ~(1 << PF5); //write mode
	latch();
	
	PORTH = 0x0f; //display
	latch();
	
	
    while (1) {
    }
}


ISR(INT0_vect){
	PORTJ = 0xff;
	displayMessage("Car at Gate");
	
	_delay_ms(2000);
	
	PORTJ = 0x00;
	
	displayMessage("register");
}
