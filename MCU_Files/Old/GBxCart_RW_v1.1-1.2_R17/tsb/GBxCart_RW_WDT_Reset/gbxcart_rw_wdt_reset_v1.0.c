/*
 GBxCart RW - Watchdog Reset to Boot loader
 Version: 1.0
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 25/08/2017
 Last Modified: 25/08/2017
 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "setup.c" // See defines, variables, constants, functions here


int main(int argc, char **argv) {
	
	printf("GBxCart RW - WDT Reset by insideGadgets\n");
	printf("################################\n");
	
	//read_config();
	
	// Check arguments
	if (argc >= 2) {
		
		cport_nr = atoi(argv[1]);
		cport_nr--;
		
		// Open the port
		if (RS232_OpenComport(cport_nr, bdrate, "8N1")) {
			return 0;
		}
		
		// Break out of any existing functions on ATmega
		set_mode('0');
		
		// Activate Watchdog reset to occur in ~250ms to return us to the boot loader
		set_number(RESET_VALUE, RESET_AVR);
		
		// Close the port
		RS232_CloseComport(cport_nr);
		
		delay_ms(300);
		
		printf("Done\n");
	}
	
	return 0;
}