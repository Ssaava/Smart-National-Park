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


int inputNum = 0; //record the user inputted number
int mode = 0; //represent the current mode, 1 for inputting the 10 yrs and below, 2 for 10yrs and above and 3 for number plate
int fridgeMode = 0;
int currentCapacity = 0;
int attendantPin = 2222;
int bottleCost = 1500;
int fridgeNum = 0;
int inputBottles = 0;
int totalBottles = 10;
int collectedMoney = 0;
int expectedMoney = 0;

void monitorGateKeyPad();
void monitorFridgeKeyPad();

struct TouristCar 
{
	int touristBelow10;
	int touristAbove10;
	int plateNo;
};

struct TouristCar touristCars[CAPACITY];

void latch(int device){
	if (device == 0)
	{
		//latch gate lcd
		PORTG &= ~(1 << PG5);
		_delay_ms(10);
		PORTG |= (1 << PG5);
		_delay_ms(10);
	}
	else{
		//latch fridge
		PORTC &= ~(1 << PC1);
		_delay_ms(10);
		PORTC |= (1 << PC1);
		_delay_ms(10);
	}
	
}



void displayMessage(char message[], int device){
	if (device == 0) //write to gate LCD
	{
		PORTG &= ~(1 << PG3); //command mode
		latch(0);
		
		
		PORTH = 0b00000001; //clear screen
		latch(0);
		
		
		PORTG |= (1 << PG3); //data mode
		PORTG &= ~(1 << PG4); //write mode
		
		latch(0);
		int i;
		
		for (i = 0; message[i] != '\0'; i++)
		{
			PORTH = message[i];
			latch(0);
		}
	} 
	else //write to fridge LCD
	{
		PORTC &= ~(1 << PC3); //command mode
		latch(1);
		
		
		PORTB = 0b00000001; //clear screen
		latch(1);
		
		
		PORTC |= (1 << PC3); //data mode
		PORTC &= ~(1 << PC2); //write mode
		
		latch(1);
		int i;
		
		for (i = 0; message[i] != '\0'; i++)
		{
			PORTB = message[i];
			latch(1);
		}
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


void displayNum(int num, int device){
	int numLen = (int)((ceil(log10(num)) + 1) * sizeof(char));

	char numStr[numLen];
	sprintf(numStr, "%d", num); // Convert only the updated inputNum to a string
	

	displayMessage(numStr, device); // Display the updated inputNum
	_delay_ms(1000);
}

void recordAndDisplay(int num, int device) {
	if (device == 0)
	{
		inputNum = (inputNum * 10) + num; // Update inputNum
		displayNum(inputNum, 0);
	}
	else{
		fridgeNum = (fridgeNum * 10) + num; // Update inputNum
		inputBottles = fridgeNum;
		displayNum(fridgeNum, 1);
	}
	
}

void displayDefaultFridgeMessage(){
		//display fridge message
		int numLen = (int)((ceil(log10(bottleCost)) + 1) * sizeof(char)); //to get the length of an integer

		char bottleCostStr[numLen];
		sprintf(bottleCostStr, "%d", bottleCost);

		
		char* msg = "ENTER IN NO OF BOTTLES @";
		char buf[1000];
		
		snprintf(buf, sizeof(buf), "%s%s", msg, bottleCostStr); //concatenate strings
		
		
		displayMessage(buf, 1);
}


int main(void)
{
	//initiate the tourist cars
	
    /* Replace with your application code */
	EIMSK |= (1 << INT0); //register the int 0 pin
	DDRJ = 0xff;
	DDRD = 0x00;
	DDRH = 0xff;
	DDRG = 0xff;
	DDRF = 0xff;
	DDRC = 0xff;
	DDRB = 0xff;
	DDRK = 0b00000111;
	DDRE = 0b00000111;
	
	sei();
	
	PORTG &= ~(1 << PG3); //command mode
	PORTG &= ~(1 << PG4); //write mode
	latch(0);
	
	
	PORTH = 0x0f; //display
	latch(0);
	
	
	PORTC &= ~(1 << PC3); //command mode fridge
	PORTC &= ~(1 << PC2); //write mode fridge
	latch(1);
	
	inputNum = 0;
	
	PORTB = 0x0f;
	latch(1);
	
	displayDefaultFridgeMessage();

	
    while (1) {
		//PORTC = 0b00100000;
		
		monitorGateKeyPad();
		monitorFridgeKeyPad();
		
    }
}

void monitorFridgeKeyPad(){
	PORTE = 0b11111011;
	if ((PINE & 0b00001000) == 0)
		{ //1
			_delay_ms(150); // to avoid the bouncing contact point error
			recordAndDisplay(1, 1);
		}
		
		if ((PINE & 0b00010000) == 0)
		{ //4
			_delay_ms(150); // to avoid the bouncing contact point error
			recordAndDisplay(4, 1);
		}
		
		if ((PINE & 0b00100000) == 0)
		{ //7
			_delay_ms(150); // to avoid the bouncing contact point error
			recordAndDisplay(7, 1);
		}
		
		if ((PINE & 0b01000000) == 0)
		{ //*
			_delay_ms(150); // to avoid the bouncing contact point error
			if (fridgeMode == 0)
			{
				if (totalBottles <= fridgeNum)
				{
					int numLen = (int)((ceil(log10(totalBottles)) + 1) * sizeof(char));

					char bottlesStr[numLen];
					sprintf(bottlesStr, "%d", totalBottles);

					
					// char* msg = "ENTER BOTTLES less than ";
					char buf[1000];
					
					sprintf(buf, "ENTER BOTTLES less than  %s", bottlesStr);
					displayMessage(buf, 1);
					
				}
				else{
					expectedMoney = fridgeNum * bottleCost;
					
					int numLen = (int)((ceil(log10(expectedMoney)) + 1) * sizeof(char));

					char expectedMoneyStr[numLen];
					sprintf(expectedMoneyStr, "%d", expectedMoney);
					
					// char* msg = "ADD ";
					// char* ms2 = " IN THE MONEY SLOT";
					char buf[1000];
					
					sprintf(buf, "ADD %s IN THE MONEY SLOT", expectedMoneyStr);
					
					displayMessage(buf, 1);
					fridgeMode = 1;
				}
			}
			else if (fridgeMode == 1)
			{
				// if (fridgeNum >= expectedMoney)
				if ((PINE & 0b01000000) == 0 && inputBottles > 0)
				{
					// collectedMoney += fridgeNum;
					collectedMoney += expectedMoney;
					int bottles = expectedMoney / bottleCost;
					// totalBottles -= bottles;
					
					
					displayMessage("MONEY SLOT OPENING", 1);
					
					PORTC |= (1 << PC5);
					_delay_ms(1000);
					
					PORTC &= ~(1 << PC5);
					displayMessage("ADD THE MONEY", 1);
					_delay_ms(2000);
					
					displayMessage("CLOSING SLOT", 1);
					PORTC |= (1 << PC4);
					_delay_ms(1000);
					
					PORTC &= ~(1 << PC4); //stop motor for money slot
					_delay_ms(10);

					//start of the fridge opening to release a bottle
					for(int i = 0; i < bottles; i++){
						displayMessage("OPENING BOTTLE SLOT", 1);
						PORTC |= (1 << PC6);
						_delay_ms(1000);
						
						PORTC &= ~(1 << PC6);
						displayMessage("PICK YOUR BOTTLE", 1);
						_delay_ms(2000);
						
						displayMessage("CLOSING BOTTLE SLOT", 1);
						PORTC |= (1 << PC7);
						_delay_ms(1000);

						PORTC &= ~(1 << PC7); //stop motor for bottle release
						_delay_ms(10);
					}
					
					
					//end of the fridge releasing a bottle
					
					fridgeMode = 0;
					inputBottles = 0; //reset the number of bottles entered by the user
					displayMessage("THANK YOU", 1);
					_delay_ms(1000);
					displayDefaultFridgeMessage();
					
				}
				// else{
					
				// 	displayMessage("FAILED !,INSUFFICIENT MONEY", 1);
				// }
			}
			
			
			fridgeNum = 0; // reset
		}
		
		PORTE = 0b11111101;
		
		if ((PINE & 0b00001000) == 0)
		{ //3
			_delay_ms(150);  // to avoid the bouncing contact point error
			recordAndDisplay(2, 1);
		}
		
		if ((PINE & 0b00010000) == 0)
		{ //5
			_delay_ms(150);  // to avoid the bouncing contact point error
			recordAndDisplay(5, 1);
		}
		
		if ((PINE & 0b00100000) == 0)
		{ //8
			_delay_ms(150);  // to avoid the bouncing contact point error
			recordAndDisplay(8, 1);
		}
		
		if ((PINE & 0b01000000) == 0)
		{ //0
			_delay_ms(150);  // to avoid the bouncing contact point error
			recordAndDisplay(0, 1);
			
		}
		
		PORTE = 0b11111110;
		
		if ((PINE & 0b00001000) == 0)
		{ //3
			_delay_ms(150);  // to avoid the bouncing contact point error
			recordAndDisplay(3, 1);
		}
		
		if ((PINE & 0b00010000) == 0)
		{ //5
			_delay_ms(150); // to avoid the bouncing contact point error
			recordAndDisplay(6, 1);
		}
		
		if ((PINE & 0b00100000) == 0)
		{ //8
			_delay_ms(150);
			recordAndDisplay(9, 1);
		}
		
		if ((PINE & 0b01000000) == 0)
		{ //0
			
			fridgeNum = 0;
			fridgeMode = 0;
			displayDefaultFridgeMessage();
		}
}

void monitorGateKeyPad(){
	PORTK = 0b11111011;
	if ((PINK & 0b00001000) == 0)
		{ //1
			_delay_ms(150); // to avoid the bouncing contact point error
			recordAndDisplay(1, 0);
		}
		
		if ((PINK & 0b00010000) == 0)
		{ //4
			_delay_ms(150); // to avoid the bouncing contact point error
			recordAndDisplay(4, 0);
		}
		
		if ((PINK & 0b00100000) == 0)
		{ //7
			_delay_ms(150); // to avoid the bouncing contact point error
			recordAndDisplay(7, 0);
		}
		
		if ((PINK & 0b01000000) == 0)
		{ //*
			_delay_ms(150); // to avoid the bouncing contact point error
			if(mode == 1){
				// record the number of tourist below 10 yrs
				touristCars[currentCapacity].touristBelow10 = inputNum;
				inputNum = 0; //resett
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
				// plate NO
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
					displayNum(currentCapacity, 0);
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
					
					displayNum(sum, 0);
					displayMessage("1->ALL CARS,2->TOURISTS", 0);
					
				}			}
			inputNum = 0; // reset
		}
		
		PORTK = 0b11111101;
		
		if ((PINK & 0b00001000) == 0)
		{ //3
			_delay_ms(150);  // to avoid the bouncing contact point error
			recordAndDisplay(2, 0);
		}
		
		if ((PINK & 0b00010000) == 0)
		{ //5
			_delay_ms(150);  // to avoid the bouncing contact point error
			recordAndDisplay(5, 0);
		}
		
		if ((PINK & 0b00100000) == 0)
		{ //8
			_delay_ms(150);  // to avoid the bouncing contact point error
			recordAndDisplay(8, 0);
		}
		
		if ((PINK & 0b01000000) == 0)
		{ //0
			_delay_ms(150);  // to avoid the bouncing contact point error
			recordAndDisplay(0, 0);
			
		}
		
		PORTK = 0b11111110;
		
		if ((PINK & 0b00001000) == 0)
		{ //3
			_delay_ms(150);  // to avoid the bouncing contact point error
			recordAndDisplay(3, 0);
		}
		
		if ((PINK & 0b00010000) == 0)
		{ //5
			_delay_ms(150); // to avoid the bouncing contact point error
			recordAndDisplay(6, 0);
		}
		
		if ((PINK & 0b00100000) == 0)
		{ //8
			_delay_ms(150);
			recordAndDisplay(9, 0);
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

ISR(INT0_vect){
	PORTJ = 0xff;
	displayMessage("Incoming Tourist Vehicle", 0);
	
	_delay_ms(2000);
	
	PORTJ = 0x00;
	

	displayMessage("register mode", 0);
	
	//modes 1-for registering tourists < 10, 2-registering tourists >10
	mode = 1;
	
	
	_delay_ms(2000);
	
	displayMessage("TOURISTS <10yrs", 0);
	inputNum = 0;

}
