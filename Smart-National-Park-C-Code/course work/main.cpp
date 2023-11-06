/*
 * course work.cpp
 *
 * Created: 23/10/2023 02:23:06
 * Author : RASHID
 Group Member Names 
 Name                      Student No.             RegNo.
SSENTEZA EMMANUEL          2100713955              21/U/13955/PS 
KISEJJERE RASHID           2100711543              21/U/11543/EVE
SSEMAGANDA TREVOUR         2100718348              21/U/18348/EVE
GUM PRISCILLA PENNINAH     2100717674              21/U/17674/EVE
BINDYA PHILIP              2100714629              21/U/14629/EVE


 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#define F_CPU 16000000UL
#include <avr/sleep.h>
#define BAUDRATE 9600
#define UBRR ((F_CPU/(BAUDRATE*16UL))-1) //UBRR=CPUclock/16/baud - 1 (from datasheet)
#define DELAY 10000
#define BUTTON_DELAY 1000

#define CAPACITY 10


int inputNum = 0; //record the user inputted number
int mode = 0; //represent the current mode, 1 for inputting the 10 yrs and below, 2 for 10yrs and above and 3 for number plate
int fridgeMode = 0;
int currentCapacity = 0;
int bottleCost = 1500;
int fridgeNum = 0;
int totalBottles = 10;
int collectedFridgeMoney = 0;
int expectedMoney = 0;

char termialInput[1000];
char attendantPin[] = "2222";
int chargeTouristsBelow10 = 2000;
int chargeTouristsAbove10 = 5000;
int terminalIndex = 0;
bool isAttendantLoggedIn = false;
int terminalMode = 0;
bool isTerminalOn = false;
int inputBottles = 0;


char terminalMenu[] = "SELECT AN OPTION \r\n\
0. TURN OFF CONSOLE(THIS IS NECESSARY INODER TO BE ABLE TO INTERACT WITH THE REST OF SYSTEM)\r\n\
1. TOTAL NUMBER OF TOURISTS CATEGORIZED BY AGE GROUP IN THE PARK\r\n\
2. ALL VEHICLES STILL IN THE PARK\r\n\
3. AMOUNT COLLECTED BY THE PARK AGGREGATED BY FRIDGE NUMBER AND ENTRACE FUND.\r\n\
4. TOTAL NUMBER OF DRIVERS IN THE PARK\r\n\
5. NUMBER OF BOTTLES IN THE FRIDGE\r\n\
6. REPLENISH FRIDGE\r\n\
7. LOGIN\r\n\
8. LOGOUT\r\n\
9. CHANGE CHARGE TOURISTS BELOW 10\r\n\
10. CHANGE CHARGE TOURISTS ABOVE 10\r\n\
11. CHANGE BOTTLE COST\r\n\
12. VIEW ALL CHARGES\r\n\r\n";


void monitorGateKeyPad();
void monitorFridgeKeyPad();

uint8_t address = 0x0a;

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
		_delay_ms(100);
		PORTG |= (1 << PG5);
		_delay_ms(100);
	}
	else{
		//latch fridge
		PORTC &= ~(1 << PC1);
		_delay_ms(100);
		PORTC |= (1 << PC1);
		_delay_ms(100);
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
	_delay_ms(DELAY);
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

		
		// char* msg = "ENTER IN NO OF BOTTLES @";
		char buf[1000];
		
		// snprintf(buf, sizeof(buf), "%s%s", msg, bottleCostStr); //concatenate strings
		if(totalBottles > 0) {
			sprintf(buf, "WATER AVAILABLE @%s", bottleCostStr);
		}else {
			sprintf(buf, "SORRY NO WATER AVAILABLE AT THE MOMENT", bottleCostStr);
		}
		
		
		displayMessage(buf, 1);
}

void usart_init(){
	//set baud rate
	UBRR1L = (unsigned char) UBRR;
	UBRR1H = (unsigned char)(UBRR >> 8);
	//enable rx/tx for USART1 and enable receive interrupt
	UCSR1B |= (1<<TXEN1) | (1 << RXEN1);
	//set frame format: 1 stop bit, 8bit data

}



void usart_send(unsigned char i){
	
	// Wait for the UDR1 register to be empty.
	while(!(UCSR1A & (1 << UDRE1)));
	// Write the data to the UDR1 register.
	UDR1 = i;
}


void displayTerminalMsg(char *msg){
	int i =0;
	while(msg[i] != '\0'){
		usart_send(msg[i]);
		i++;
	}
}

bool isStringEqual(char *str1, char *str2){
	int i;
	if (terminalIndex == strlen(str2))
	{
		
		for (i =0; i < strlen(str2); i++)
		{
			if (str1[i] != str2[i])
			{
				return false;
			}
		}
		
	return true;
	}

	return false;
}

void displayTerminalInfo(char str[], int info){
	int numLen = (int)((ceil(log10(info)) + 1) * sizeof(char));

	char infoStr[numLen];
	sprintf(infoStr, "%d", info);

	int buff_len = strlen(str) + numLen;
	
	char buf[buff_len + 1];
	
	snprintf(buf, sizeof(buf), "%s%s", str, infoStr);
	displayTerminalMsg(buf);
	displayTerminalMsg("\r\n\r\n");
	memset(buf, 0, strlen(buf));
}

int strToInt(char *str, int strLen){
	int converted = 0;
	int i = 0;
	while (i < strLen)
	{
		int j = str[i] - '0'; //convert character to int
		converted = (converted * 10) + j;
		i++;
	}
	
	return converted;
}


void attendantOperate(){
	if (terminalMode == 1)
	{
		//login mode
		if (isStringEqual(termialInput, attendantPin))
		{
			displayTerminalMsg("LOGIN SUCCESSFUL!! \r\n ");
			displayTerminalMsg(terminalMenu);
			isAttendantLoggedIn = true;
			terminalMode = 0;
			terminalIndex = 0;
			termialInput[0] = '\0'; //reset
			
		}
		else{
			displayTerminalMsg("LOGIN FAILED, TRY AGAIN !! \r\n");
		}
	}
	else if (terminalMode == 2)
	{
		//add new number
		int newBottles = strToInt(termialInput, terminalIndex);
		totalBottles = newBottles;
		
		displayTerminalInfo("BOTTLES REPLENISHED SUCCESSFULLY TO = ", totalBottles);
		terminalMode = 0; //reset
		displayTerminalMsg(terminalMenu);
	}
	else if (terminalMode == 3)
	{
		//add new number
		int newCharge = strToInt(termialInput, terminalIndex);
		chargeTouristsBelow10 = newCharge;
		
		displayTerminalInfo("CHARGE UPDATED SUCCESSFULLY TO = ", chargeTouristsBelow10);
		terminalMode = 0; //reset
		displayTerminalMsg(terminalMenu);
	}
	else if (terminalMode == 4)
	{
		//add new number
		int newCharge = strToInt(termialInput, terminalIndex);
		chargeTouristsAbove10 = newCharge;
		
		displayTerminalInfo("CHARGE UPDATED SUCCESSFULLY TO = ", chargeTouristsAbove10);
		terminalMode = 0; //reset
		displayTerminalMsg(terminalMenu);
	}
	else if (terminalMode == 5)
	{
		//add new number
		int newCharge = strToInt(termialInput, terminalIndex);
		bottleCost = newCharge;
		
		displayTerminalInfo("COST UPDATED SUCCESSFULLY TO = ", bottleCost);
		terminalMode = 0; //reset
		displayTerminalMsg(terminalMenu);
	}
	else{
		if (isStringEqual(termialInput, "0"))
		{
			//turn off the console
			displayTerminalMsg("\r\n TURNING OF TERMINAL.. PRESS # ON THE KEYPAD TO TURN IT BACK ON !!! \r\n");
			terminalMode = 0;
			terminalIndex = 0;
			isTerminalOn = false;
			termialInput[0] = '\0'; //reset
		}
		
		else if (isStringEqual(termialInput, "7"))
		{
			displayTerminalMsg("\r\n ENTER IN PIN !!! \r\n");
			terminalMode = 1;
		}
		
		else{
			if (isAttendantLoggedIn == false)
			{
				displayTerminalMsg("\r\n PLEASE LOGIN IN FIRST TO CONTINUE !!! \r\n");
				displayTerminalMsg(terminalMenu);
			}
			else
			{
				if (isStringEqual(termialInput, "12"))
				{
					//all charges display
					displayTerminalInfo("CURRENT CHARGE OF TOURISTS BELOW 10 = ", chargeTouristsBelow10);
					displayTerminalInfo("CURRENT CHARGE OF TOURISTS ABOVE 10 = ", chargeTouristsAbove10);
					displayTerminalInfo("CURRENT CHARGE OF FRIDGE BOTTLE = ", bottleCost);
					displayTerminalMsg(terminalMenu);
				}
				else if (isStringEqual(termialInput, "11"))
				{
					//CHANGE BOTTLE COST
					displayTerminalInfo("CURRENT BOTTLE COST = ", bottleCost);
					displayTerminalMsg("ENTER IN THE NEW COST : \r\n");
					terminalMode = 5;
				}
				else if (isStringEqual(termialInput, "10"))
				{
					//9. CHANGE CHARGE TOURISTS BELOW 10\r\n
					displayTerminalInfo("CURRENT CHARGE OF TOURISTS ABOVE 10 = ", chargeTouristsAbove10);
					displayTerminalMsg("ENTER IN THE NEW CHARGE : \r\n");
					terminalMode = 4;
				}
				else if (isStringEqual(termialInput, "1"))
				{
					//1. TOTAL NUMBER OF TOURISTS CATEGORIZED BY AGE GROUP IN THE PARK
					int totalBelow10= 0;
					int totalAbove10 = 0;
					int i = 0;
					while ( i < currentCapacity)
					{
						totalAbove10 = totalAbove10 + touristCars[i].touristAbove10;
						totalBelow10 = totalBelow10 + touristCars[i].touristBelow10;
						i++;
					}
					
					displayTerminalInfo("TOTAL TOURISTS BELOW 10 = ", totalBelow10);
					displayTerminalInfo("TOTAL TOURISTS ABOVE 10 = ", totalAbove10);
					displayTerminalMsg(terminalMenu);
				}
				else if (isStringEqual(termialInput, "2"))
				{
					//2. ALL VEHICLES STILL IN THE PARK
					displayTerminalMsg("BELOW ARE THE VEHICLE NUMBER PLATES STILL IN THE PARK \r\n");
					for (int i = 0; i < currentCapacity; i++)
					{
						
						displayTerminalInfo("VEHICLE - ", touristCars[i].plateNo);
					}
					
					displayTerminalMsg(terminalMenu);
				}
				else if (isStringEqual(termialInput, "3"))
				{
					// 3. AMOUNT COLLECTED BY THE PARK AGGREGATED BY FRIDGE NUMBER AND ENTRACE FUND.\r\n
					int totalTouristsCollectedMoney= 0;
					int i = 0;
					while ( i < currentCapacity)
					{
						totalTouristsCollectedMoney += (touristCars[i].touristAbove10 * chargeTouristsAbove10) + (touristCars[i].touristBelow10 * chargeTouristsBelow10);
						i++;
					}
					int totalAmount = totalTouristsCollectedMoney + collectedFridgeMoney;
					
					displayTerminalInfo("TOTAL COLLECTED MONEY FROM TOURISTS = ", totalTouristsCollectedMoney);
					displayTerminalInfo("TOTAL COLLECTED MONEY FROM FRIDGE = ", collectedFridgeMoney);
					displayTerminalInfo("TOTAL COLLECTED MONEY = ", totalAmount);
					displayTerminalMsg(terminalMenu);
					
				}
				else if (isStringEqual(termialInput, "4"))
				{
					//4. TOTAL DRIVERS
					displayTerminalInfo("TOTAL NUMBER OF DRIVERS STILL IN THE PARK = ", currentCapacity);
					displayTerminalMsg(terminalMenu);
				}
				else if (isStringEqual(termialInput, "5"))
				{
					//5. NUMBER OF BOTTLES IN THE FRIDGE\r\n
					displayTerminalInfo("TOTAL NUMBER OF BOTTLES IN THE FRIDGE = ", totalBottles);
					displayTerminalMsg(terminalMenu);
				}
				else if (isStringEqual(termialInput, "6"))
				{
					//6. REPLENISH FRIDGE\r\n
					displayTerminalInfo("CURRENT NUMBER OF BOTTLES IN THE FRIDGE = ", totalBottles);
					displayTerminalMsg("ENTER IN THE NEW NUMBER OF BOTTLES : \r\n");
					terminalMode = 2;
				}
				else if (isStringEqual(termialInput, "8"))
				{
					//LOGOUT
					displayTerminalMsg("LOGOUT SUCCESSFULL \r\n");
					isAttendantLoggedIn = false;
				}
				else if (isStringEqual(termialInput, "9"))
				{
					//9. CHANGE CHARGE TOURISTS BELOW 10\r\n
					displayTerminalInfo("CURRENT CHARGE OF TOURISTS BELOW 10 = ", chargeTouristsBelow10);
					displayTerminalMsg("ENTER IN THE NEW CHARGE : \r\n");
					terminalMode = 3;
				}
				
			}
		}
		
		terminalIndex = 0;
		termialInput[0] = '\0'; //reset
	}
}

void usart_receive(){
	while(!(UCSR1A & (1 << RXC1)));
	unsigned char i = UDR1;
	if (i == 0x0D)
	{
		//Enter clicked
		attendantOperate();
	}
	else if (i == 0x08)
	{
		//backspace
		terminalIndex --;
	}
	else{
		termialInput[terminalIndex] = i;
		terminalIndex++;
	}
}




int main(void)
{
	//initiate the tourist cars
	
    /* Replace with your application code */
	EIMSK |= (1 << INT0); //register the int 0 pin
	EIMSK |= (1 << INT1); //register the int 1 pin
	DDRJ = 0xff; //buzzer code
	
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
	
	//serial console
	unsigned char str[] = "Testing";
	int i=0;
	usart_init();
	
	displayTerminalMsg("SERAIL CONSOLE IS CURRENTLY OFF, PRESS # ON THE KEYPAD TO ACTIVATE IT \r\n\r\n");
	
	
    while (1) {
		//PORTC = 0b00100000;
		if(isTerminalOn){
			usart_receive();
		}
		
		
		monitorGateKeyPad();
		monitorFridgeKeyPad();
		
    }
}

void monitorFridgeKeyPad(){
	PORTE = 0b11111011;
	if ((PINE & 0b00001000) == 0)
		{ //1
			_delay_ms(BUTTON_DELAY); // to avoid the bouncing contact point error
			recordAndDisplay(1, 1);
		}
		
		if ((PINE & 0b00010000) == 0)
		{ //4
			_delay_ms(BUTTON_DELAY); // to avoid the bouncing contact point error
			recordAndDisplay(4, 1);
		}
		
		if ((PINE & 0b00100000) == 0)
		{ //7
			_delay_ms(BUTTON_DELAY); // to avoid the bouncing contact point error
			recordAndDisplay(7, 1);
		}
		
		if ((PINE & 0b01000000) == 0)
		{ //*
			_delay_ms(BUTTON_DELAY); // to avoid the bouncing contact point error
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
					// collectedFridgeMoney += fridgeNum;
					collectedFridgeMoney += expectedMoney;
					int bottles = expectedMoney / bottleCost;
					totalBottles -= bottles; //reduce the number of bottles available
					
					
					displayMessage("MONEY SLOT OPENING", 1);
					
					PORTC |= (1 << PC5);
					_delay_ms(10000);
					
					PORTC &= ~(1 << PC5);
					displayMessage("ADD THE MONEY", 1);
					_delay_ms(10000);
					
					displayMessage("CLOSING SLOT", 1);
					PORTC |= (1 << PC4);
					_delay_ms(10000);
					
					PORTC &= ~(1 << PC4); //stop motor for money slot
					_delay_ms(10000);

					//start of the fridge opening to release a bottle
					for(int i = 0; i < bottles; i++){
						displayMessage("OPENING BOTTLE SLOT", 1);
						PORTC |= (1 << PC6);
						_delay_ms(10000);
						
						PORTC &= ~(1 << PC6);
						displayMessage("PICK YOUR BOTTLE", 1);
						_delay_ms(10000);
						
						displayMessage("CLOSING BOTTLE SLOT", 1);
						PORTC |= (1 << PC7);
						_delay_ms(10000);

						PORTC &= ~(1 << PC7); //stop motor for bottle release
						_delay_ms(10000);
					}
					
					
					//end of the fridge releasing a bottle
					
					fridgeMode = 0;
					inputBottles = 0; //reset the number of bottles entered by the user
					displayMessage("THANK YOU", 1);
					_delay_ms(10000);
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
			_delay_ms(BUTTON_DELAY);  // to avoid the bouncing contact point error
			recordAndDisplay(2, 1);
		}
		
		if ((PINE & 0b00010000) == 0)
		{ //5
			_delay_ms(BUTTON_DELAY);  // to avoid the bouncing contact point error
			recordAndDisplay(5, 1);
		}
		
		if ((PINE & 0b00100000) == 0)
		{ //8
			_delay_ms(BUTTON_DELAY);  // to avoid the bouncing contact point error
			recordAndDisplay(8, 1);
		}
		
		if ((PINE & 0b01000000) == 0)
		{ //0
			_delay_ms(BUTTON_DELAY);  // to avoid the bouncing contact point error
			recordAndDisplay(0, 1);
			
		}
		
		PORTE = 0b11111110;
		
		if ((PINE & 0b00001000) == 0)
		{ //3
			_delay_ms(BUTTON_DELAY);  // to avoid the bouncing contact point error
			recordAndDisplay(3, 1);
		}
		
		if ((PINE & 0b00010000) == 0)
		{ //5
			_delay_ms(BUTTON_DELAY); // to avoid the bouncing contact point error
			recordAndDisplay(6, 1);
		}
		
		if ((PINE & 0b00100000) == 0)
		{ //8
			_delay_ms(DELAY);
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
			_delay_ms(BUTTON_DELAY); // to avoid the bouncing contact point error
			recordAndDisplay(1, 0);
		}
		
		if ((PINK & 0b00010000) == 0)
		{ //4
			_delay_ms(BUTTON_DELAY); // to avoid the bouncing contact point error
			recordAndDisplay(4, 0);
		}
		
		if ((PINK & 0b00100000) == 0)
		{ //7
			_delay_ms(BUTTON_DELAY); // to avoid the bouncing contact point error
			recordAndDisplay(7, 0);
		}
		
		if ((PINK & 0b01000000) == 0)
		{ //*
			_delay_ms(BUTTON_DELAY); // to avoid the bouncing contact point error
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
				_delay_ms(DELAY);
				
				openAndCloseGate();
				
				mode = 0;
				currentCapacity += 1;
				inputNum = 0; // reset
			}
			else if (mode == 4)
			{
				//exit model
				int plateNo = inputNum;
				
				for (int i = 0; i < currentCapacity; i++)
				{
					//check if there's a car with the same plate No
					if(touristCars[i].plateNo == plateNo){
						//remove the plate number
						displayMessage("CAR EXITED SUCCESSFULLY", 0);
						
						_delay_ms(DELAY);
						mode = 0;
						currentCapacity --;
						
						//delete the object from the array of tourist cars
						for (int j = i; j < currentCapacity; j++)
						{
							touristCars[i] = touristCars[i+1];
						}
						
						return;
					}
				}
				
				//There's no car with the same plate No
				displayMessage("INVALID CAR NUMBER PLATE", 0);
				
				
			}
			else if (mode == 5)
			{
			}
			inputNum = 0; // reset
		}
		
		PORTK = 0b11111101;
		
		if ((PINK & 0b00001000) == 0)
		{ //3
			_delay_ms(BUTTON_DELAY);  // to avoid the bouncing contact point error
			recordAndDisplay(2, 0);
		}
		
		if ((PINK & 0b00010000) == 0)
		{ //5
			_delay_ms(BUTTON_DELAY);  // to avoid the bouncing contact point error
			recordAndDisplay(5, 0);
		}
		
		if ((PINK & 0b00100000) == 0)
		{ //8
			_delay_ms(BUTTON_DELAY);  // to avoid the bouncing contact point error
			recordAndDisplay(8, 0);
		}
		
		if ((PINK & 0b01000000) == 0)
		{ //0
			_delay_ms(BUTTON_DELAY);  // to avoid the bouncing contact point error
			recordAndDisplay(0, 0);
			
		}
		
		PORTK = 0b11111110;
		
		if ((PINK & 0b00001000) == 0)
		{ //3
			_delay_ms(BUTTON_DELAY);  // to avoid the bouncing contact point error
			recordAndDisplay(3, 0);
		}
		
		if ((PINK & 0b00010000) == 0)
		{ //5
			_delay_ms(BUTTON_DELAY); // to avoid the bouncing contact point error
			recordAndDisplay(6, 0);
		}
		
		if ((PINK & 0b00100000) == 0)
		{ //8
			_delay_ms(BUTTON_DELAY);
			recordAndDisplay(9, 0);
		}
		
		if ((PINK & 0b01000000) == 0)
		{ //0
			isTerminalOn = true;
			displayTerminalMsg("TURNING ON THE CONSOLE... PRESS 0 TO TURN OFF THE CONSOLE \r\n\r\n\r\n");
			displayTerminalMsg(terminalMenu);
			
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


ISR(INT1_vect){
	PORTJ = 0xff;
	displayMessage("EXITING VEHICLE", 0);
	
	_delay_ms(2000);
	
	PORTJ = 0x00;
	

	displayMessage("exit mode", 0);
	
	//modes 1-for registering tourists < 10, 2-registering tourists >10
	mode = 4;
	
	
	_delay_ms(2000);
	
	displayMessage("TOURIST PLATE NO", 0);
	inputNum = 0;

}
// solving merge conflicts


