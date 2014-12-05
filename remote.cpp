#include <string>
#include <getopt.h>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <RF24/RF24.h>

using namespace std;
//RF24 radio("/dev/spidev0.0",8000000 , 25);  
//RF24 radio(RPI_V2_GPIO_P1_15, RPI_V2_GPIO_P1_24, BCM2835_SPI_SPEED_8MHZ);

RF24 radio(RPI_V2_GPIO_P1_22, RPI_V2_GPIO_P1_24, BCM2835_SPI_SPEED_8MHZ);
//const int role_pin = 7;
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
//const uint8_t pipes[][6] = {"1Node","2Node"};

// hack to avoid SEG FAULT, issue #46 on RF24 github https://github.com/TMRh20/RF24.git
unsigned long  got_message;

void setup(void){
	//Prepare the radio module
	printf("\nPreparing interface\n");
	radio.begin();
	radio.setRetries( 15, 15);
	//	radio.setChannel(0x4c);
	//	radio.setPALevel(RF24_PA_MAX);
	//	radio.setPALevel(RF24_PA_MAX);

	radio.printDetails();
	radio.openWritingPipe(pipes[0]);
	radio.openReadingPipe(1,pipes[1]);
	//	radio.startListening();

}

bool sendMessage(int action){
	//This function send a message, the 'action', to the arduino and wait for answer
	//Returns true if ACK package is received
	//Stop listening
	radio.stopListening();
	unsigned long message = action;
	printf("Now sending  %lu...", message);

	//Send the message
	bool ok = radio.write( &message, sizeof(unsigned long) );
	if (!ok){
		printf("failed...\n\r");
	}else{
		printf("ok!\n\r");
	}	
	//Listen for ACK
	radio.startListening();
	//Let's take the time while we listen
	unsigned long started_waiting_at = millis();
	bool timeout = false;
	while ( ! radio.available() && ! timeout ) {
		//printf("%d", !radio.available());
		if (millis() - started_waiting_at > 1000 ){
			timeout = true;
		}
	}

	if( timeout ){
		//If we waited too long the transmission failed
		printf("Oh gosh, it's not giving me any response...\n\r");
		return false;
	}else{
		//If we received the message in time, let's read it and print it
		radio.read( &got_message, sizeof(unsigned long) );
		printf("Yay! Got this response %lu.\n\r",got_message);
		return true;
	}

}  

int main( int argc, char ** argv){

	char choice;
	setup();
	bool switched = false;
	int counter = 0;

	//Define the options

	while(( choice = getopt( argc, argv, "m:")) != -1){

		if (choice == 'm'){


			printf("\n Talking with my NRF24l01+ friends out there....\n");

			while(switched == false && counter < 5){

				switched = sendMessage(atoi(optarg));
				
				counter ++;

				sleep(1);
			}


		}else{
			// A little help:
			printf("\n\rIt's time to make some choices...\n");
			printf("\n\rTIP: Use -m idAction for the message to send. ");


			printf("\n\rExample (id number 12, action number 1): ");
			printf("\nsudo ./remote -m 121\n");
		}

		//return 0 if everything went good, 2 otherwise
		if (counter < 5)
			return 0;
		else
			return 2;
	}
}
