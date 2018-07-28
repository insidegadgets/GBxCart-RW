/*
 GBxCart RW - Console Interface
 Version: 1.5
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 7/11/2016
 Last Modified: 17/08/2017
 
 */

#define LOW 0
#define HIGH 1
#define false 0
#define true 1

// COM Port settings (default)
#include "rs232/rs232.h"
int cport_nr = 7, // /dev/ttyS7 (COM8 on windows)
bdrate = 1000000; // 1,000,000 baud

#define RESET_AVR '*'
#define RESET_VALUE 0x7E5E1

// Read the config.ini file for the COM port to use and baud rate
void read_config(void) {
	FILE* configfile = fopen ( "config.ini" , "rb" );
	char buffer[100];
	
	if (configfile != NULL) {
		// Copy the file into the buffer, we only read 2 characters
		fread (buffer, 1, 2, configfile);
		buffer[2] = 0;
		
		uint8_t numbersFound = 0;
		for (uint8_t x = 0; x < 2; x++) {
			if (buffer[x] >= 48 && buffer[x] <= 57) {
				numbersFound++;
			}
		}
		
		if (numbersFound >= 1) {
			cport_nr = atoi(buffer);
			cport_nr--;
		}
		
		// Remove the \r\n line
		fread (buffer, 1, numbersFound, configfile);
		
		// Read the baud rate
		fread (buffer, 1, 7, configfile);
		buffer[7] = 0;
		
		for (uint8_t x = 0; x < 7; x++) {
			if (buffer[x] >= 48 && buffer[x] <= 57) {
				numbersFound++;
			}
		}
		
		if (numbersFound >= 1) {
			bdrate = atoi(buffer);
		}
		
		fclose(configfile);
	}
}

void delay_ms(uint16_t ms) {
	#if defined (_WIN32)
		Sleep(ms);
	#else
		usleep(1000 * ms);
	#endif
}

// Read one letter from stdin
char read_one_letter (void) {
	char c = getchar();
	while (getchar() != '\n' && getchar() != EOF);
	return c;
}

// Send a single command byte
void set_mode (char command) {
	char modeString[5];
	sprintf(modeString, "%c", command);
	
	RS232_cputs(cport_nr, modeString);
}

// Send a command with a hex number and a null terminator byte
void set_number (uint32_t number, uint8_t command) {
	char numberString[20];
	sprintf(numberString, "%c%x", command, number);
	
	RS232_cputs(cport_nr, numberString);
	RS232_SendByte(cport_nr, 0);
}
