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

#define CAPACITY 10


unsigned int inputNum = 0; //record the user inputted number
int mode = 0; //represent the current mode, 1 for inputting the 10 yrs and below, 2 for 10yrs and above and 3 for number plate
int currentCapacity = 0;
int attendantPin = 2222;

struct TouristCar 
{
	int touristBelow10;
	int touristAbove10;
	int plateNo;
};

void latch(){
	PORTG &= ~(1 << PG5);
	_delay_ms(10);
	PORTG |= (1 << PG5);
	_delay_ms(10);
}



void displayMessage(char message[], int row){
	char rows[] = {0x80, 0xC0, 0x90, 0xD0};
	
	PORTG &= ~(1 << PG3); //command mode
	latch();
	
	
	PORTH = 0b00000001; //clear screen
	latch();
	
	PORTG = rows[row];
	latch();
	
	
	PORTG |= (1 << PG3); //data mode
	PORTG &= ~(1 << PG4); //write mode
	
	latch();
	int i;
	
	for (i = 0; message[i] != '\0'; i++)
	{
		PORTH = message[i];
		latch();
	}
}

void openAndCloseGate(){
	//open gate
	displayMessage("OPENING GATE", 0);
	PORTF = 0b00000010;
	_delay_ms(2000);
	
	//open gate
	displayMessage("CLOSING GATE", 0);
	PORTF = 0b00000001;
	
	_delay_ms(2000);
	PORTF = 0x00;
}

void displayNum(int num){
	int numLen = (int)((ceil(log10(num)) + 1) * sizeof(char));

	char numStr[numLen];
	sprintf(numStr, "%d", num); // Convert only the updated inputNum to a string
	
	char display[100];

	displayMessage(numStr, 0); // Display the updated inputNum
	_delay_ms(1000);
}

void recordAndDisplay(int num) {
	inputNum = (inputNum * 10) + num; // Update inputNum
	
	displayNum(inputNum);
}


int main(void)
{
	//initiate the tourist cars
	struct TouristCar touristCars[CAPACITY];
	
    /* Replace with your application code */
	EIMSK |= (1 << INT0); //register the int 0 pin
	DDRJ = 0xff;
	DDRD = 0x00;
	DDRH = 0xff;
	DDRG = 0xff;
	DDRF = 0xff;
	DDRK = 0b00000111;
	
	sei();
	
	PORTG &= ~(1 << PG3); //command mode
	PORTG &= ~(1 << PG4); //write mode
	latch();
	
	
	PORTH = 0x0f; //display
	latch();
	
	inputNum = 0;
	
    while (1) {
		//
		
		PORTK = 0b11111011;
		
		if ((PINK & 0b00001000) == 0)
		{ //1
			_delay_ms(150); // to avoid the bouncing contact point error
			recordAndDisplay(1);
		}
		
		if ((PINK & 0b00010000) == 0)
		{ //4
			_delay_ms(150); // to avoid the bouncing contact point error
			recordAndDisplay(4);
		}
		
		if ((PINK & 0b00100000) == 0)
		{ //7
			_delay_ms(150); // to avoid the bouncing contact point error
			recordAndDisplay(7);
		}
		
		if ((PINK & 0b01000000) == 0)
		{ //*
			_delay_ms(150); // to avoid the bouncing contact point error
			if(mode == 1){
				// record the number of tourist below 10 yrs
				touristCars[currentCapacity].touristBelow10 = inputNum;
				inputNum = 0;
				displayMessage("TOURISTS > 10yrs", 0);
				mode = 2;
			}
			else if(mode == 2){
				// record the number of tourist below 10 yrs
				touristCars[currentCapacity].touristAbove10 = inputNum;
				inputNum = 0;
				displayMessage("PLATE NO", 0);
				mode = 3;
			}
			else if(mode == 3){
				// record the number of tourist below 10 yrs
				touristCars[currentCapacity].plateNo = inputNum;
				inputNum = 0;
				displayMessage("TOURISTS RECORDED", 0);
				_delay_ms(150);
				
				openAndCloseGate();
				
				mode = 0;
				currentCapacity += 1;
				inputNum = 0; // reset
			}
			else if (mode == 4)
			{
				if (inputNum == attendantPin)
				{
					displayMessage("LOGIN SUCCESS", 0);
					_delay_ms(300);
					
					displayMessage("1->ALL CARS,2->TOURISTS", 0);
					mode = 5;
				}
				else{
					displayMessage("LOGIN FAILED", 0);
					
					inputNum = 0;
			}
			inputNum = 0; // reset
			}
			else if (mode == 5)
			{
				if (inputNum == 1)
				{
					//all cars
					displayNum(currentCapacity);
					displayMessage("1->ALL CARS,2->TOURISTS", 0);
					
				}
				else if (inputNum == 2)
				{
					// all tourists
					int sum = 0;
					for (int i = 0; i < currentCapacity; i ++)
					{
						sum += (touristCars[i].touristAbove10 + touristCars[1].touristBelow10);
					}
					
					displayNum(sum);
					displayMessage("1->ALL CARS,2->TOURISTS", 0);
					
				}
			}
			inputNum = 0; // reset
		}
		
		PORTK = 0b11111101;
		
		if ((PINK & 0b00001000) == 0)
		{ //3
			_delay_ms(150);  // to avoid the bouncing contact point error
			recordAndDisplay(2);
		}
		
		if ((PINK & 0b00010000) == 0)
		{ //5
			_delay_ms(150);  // to avoid the bouncing contact point error
			recordAndDisplay(5);
		}
		
		if ((PINK & 0b00100000) == 0)
		{ //8
			_delay_ms(150);  // to avoid the bouncing contact point error
			recordAndDisplay(8);
		}
		
		if ((PINK & 0b01000000) == 0)
		{ //0
			_delay_ms(150);  // to avoid the bouncing contact point error
			recordAndDisplay(0);
			
		}
		
		PORTK = 0b11111110;
		
		if ((PINK & 0b00001000) == 0)
		{ //3
			_delay_ms(150);  // to avoid the bouncing contact point error
			recordAndDisplay(3);
		}
		
		if ((PINK & 0b00010000) == 0)
		{ //5
			_delay_ms(150); // to avoid the bouncing contact point error
			recordAndDisplay(6);
		}
		
		if ((PINK & 0b00100000) == 0)
		{ //8
			_delay_ms(150);
			recordAndDisplay(9);
		}
		
		if ((PINK & 0b01000000) == 0)
		{ //0
			_delay_ms(150);  // to avoid the bouncing contact point error
			displayMessage("ATTENDANT MODE", 0);
			_delay_ms(300);
			
			displayMessage("ENTER PIN", 0);
			mode = 4;
			
		}
		
    }
}


ISR(INT0_vect){
	PORTJ = 0xff;
	displayMessage("Car at Gate", 0);
	
	_delay_ms(2000);
	
	PORTJ = 0x00;
	
	displayMessage("register mode", 2);
	
	
	mode = 1;
	
	
	_delay_ms(2000);
	
	displayMessage("TOURISTS <10yrs", 0);
	inputNum = 0;
}
