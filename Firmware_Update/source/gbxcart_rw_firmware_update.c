/*
 GBxCart RW - Firmware update
 Version: 1.2
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 2/11/2019
 Last Modified: 3/07/2020
 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <Windows.h>
#define DIR_SEPARATOR "\\"
#define EXE_SUFFIX ".exe"
#else
#include <unistd.h>
#define DIR_SEPARATOR "/"
#define EXE_SUFFIX ""
#endif

#define MINI_FIRMWARE 17
#define STANDARD_FIRMWARE 18

#include "setup.h" // See defines, variables, constants, functions here

int main(int argc, char **argv) {
	char portname[128];
	printf("GBxCart RW Firmware Update\n");
	printf("##########################\n");
	
	read_config();
	
	// Open COM port
	if (com_test_port() == 0) {
		printf("Device not connected and couldn't be auto detected\n");
		read_one_letter();
		return 1;
	}
#ifdef _WIN32
	sprintf(portname, "com%i", cport_nr+1);
#else
	sprintf(portname, "%s", RS232_GetPortName(cport_nr));
#endif
	printf("Connected on COM port: %s\n", portname);
	
	// Break out of any existing functions on ATmega
	set_mode('0');
	
	// Get firmware version
	gbxcartFirmwareVersion = request_value(READ_FIRMWARE_VERSION);
	
	// Get PCB version
	gbxcartPcbVersion = request_value(READ_PCB_VERSION);
	RS232_CloseComport(cport_nr);
	
	if (gbxcartPcbVersion == 100) {
		printf("Detected GBxCart RW: Mini PCB version\n");
	}
	else if (gbxcartPcbVersion == 2) {
		printf("Detected GBxCart RW: v1.1-v1.2 PCB version\n");
	}
	else if (gbxcartPcbVersion == 4) {
		printf("Detected GBxCart RW: v1.3 PCB version\n");
	}
	else if (gbxcartPcbVersion == 90) {
		printf("Detected GBxMAS RW: v1.0 PCB version\n");
	}
	
	printf("Detected Firmware: %i\n\n", gbxcartFirmwareVersion);
	
	// Mini update
	if (gbxcartPcbVersion == 100) {
		if (gbxcartFirmwareVersion < MINI_FIRMWARE) {
			printf("Firmware R%i is available. Would you like to update? (Y/[N])\n>", MINI_FIRMWARE);
			
			char modeSelected = read_one_letter();
			if (modeSelected == 'y') {
				printf("\n\n");
				char tsbReset[100];
				sprintf(tsbReset, "tsb" DIR_SEPARATOR "gbxcart_rw_wdt_reset_v1.0" EXE_SUFFIX " %i", cport_nr+1);
				system(tsbReset);
				
				char tsbFirmware[200];
				sprintf(tsbFirmware, "tsb" DIR_SEPARATOR "tsb" EXE_SUFFIX " %s:57600 fw gbxcart_rw_mini_v1.0_pcb_r17.hex", portname);
				system(tsbFirmware);
			}
			else {
				printf("Update cancelled\n");
			}
		}
		else {
			printf("You are on the latest firmware\n");
			read_one_letter();
		}
	}
	else if (gbxcartPcbVersion == 2 || gbxcartPcbVersion == 4) {
		if (gbxcartFirmwareVersion < STANDARD_FIRMWARE) {
			printf("Firmware R%i is available. Would you like to update? (Y/[N])\n>", STANDARD_FIRMWARE);
			
			char modeSelected = read_one_letter();
			if (modeSelected == 'y') {
				if (gbxcartPcbVersion == 2) { // v1.1-v1.2 PCB
					printf("\n\n");
					char tsbReset[100];
					sprintf(tsbReset, "tsb" DIR_SEPARATOR "gbxcart_rw_wdt_reset_v1.0" EXE_SUFFIX " %i", cport_nr+1);
					system(tsbReset);
					
					char tsbFirmware[200];
					sprintf(tsbFirmware, "tsb" DIR_SEPARATOR "tsb" EXE_SUFFIX " %s:57600 fw gbxcart_rw_v1.1-1.2_pcb_r18.hex", portname);
					system(tsbFirmware);
					printf(tsbFirmware);
				}
				else if (gbxcartPcbVersion == 4) { // v1.3 PCB
					printf("\n\n");
					char tsbReset[100];
					sprintf(tsbReset, "tsb" DIR_SEPARATOR "gbxcart_rw_wdt_reset_v1.0" EXE_SUFFIX " %i", cport_nr+1);
					system(tsbReset);
					
					char tsbFirmware[200];
					sprintf(tsbFirmware, "tsb" DIR_SEPARATOR "tsb" EXE_SUFFIX " %s:57600 fw gbxcart_rw_v1.3_pcb_r18.hex", portname);
					system(tsbFirmware);
				}
			}
			else {
				printf("Update cancelled\n");
			}
		}
		else {
			printf("You are on the latest firmware\n");
			read_one_letter();
		}
	}
	else if (gbxcartPcbVersion == 90) {
		if (gbxcartFirmwareVersion < STANDARD_FIRMWARE) {
			printf("Please note that it may take several attempts for the update to work.\n");
			printf("Firmware R%i is available. Would you like to update? (Y/[N])\n>", STANDARD_FIRMWARE);
			
			char modeSelected = read_one_letter();
			if (modeSelected == 'y') {
				printf("\n\n");
				char tsbReset[100];
				sprintf(tsbReset, "tsb" DIR_SEPARATOR "gbxcart_rw_wdt_reset_v1.0" EXE_SUFFIX " %i", cport_nr+1);
				system(tsbReset);
				
				char tsbFirmware[200];
				sprintf(tsbFirmware, "tsb" DIR_SEPARATOR "tsb" EXE_SUFFIX " %s:57600 fw gbxmas_rw_v1.0_pcb_r18.hex", portname);
				system(tsbFirmware);
			}
			else {
				printf("Update cancelled\n");
			}
		}
		else {
			printf("You are on the latest firmware\n");
			read_one_letter();
		}
	}
	
	return 0;
}