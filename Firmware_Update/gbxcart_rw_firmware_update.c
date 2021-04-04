/*
 GBxCart RW - Firmware update
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 2/11/2019
 Last Modified: 17/10/2020
 
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

#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)
#define RS232_PORTNR  57
#else
#define RS232_PORTNR  30
#endif
extern char *comports[RS232_PORTNR];

uint8_t miniFirmware = 26;
uint8_t standardFirmware = 29;

#include "setup.h" // See defines, variables, constants, functions here


void watchdog_reset(int delayTime) {
	// Break out of any existing functions on ATmega
	set_mode('0');
	
	// Activate Watchdog reset to occur in ~250ms to return us to the boot loader
	set_number(RESET_VALUE, RESET_AVR);
	
	// Close the port
	RS232_CloseComport(cport_nr);
	
	delay_ms(delayTime);
	
	printf("WDT Reset Done\n");
}

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
	sprintf(portname, "%i", cport_nr);
#endif
	printf("Connected on COM port: %s\n", portname);
	
	// Break out of any existing functions on ATmega
	set_mode('0');
	
	// Get firmware version
	gbxcartFirmwareVersion = request_value(READ_FIRMWARE_VERSION);
	
	// Get PCB version
	gbxcartPcbVersion = request_value(READ_PCB_VERSION);
	
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
	
	#if defined(__APPLE__)
		printf("Please make sure you install Mono. You may need to close and re-open the terminal once installation is complete.\n\n");
	#endif
	
	// Mini update
	if (gbxcartPcbVersion == 100) {
		if (gbxcartFirmwareVersion < miniFirmware) {
			printf("Specify delay time (in ms) or press enter for default (250ms): ");
			char optionString[5];
			fgets(optionString, 5, stdin);
			int delayTime = atoi(optionString);
			if (delayTime == 0) {
				delayTime = 250;
			}
			
			printf("Firmware R%i is available. Would you like to update? (Y/[N])\n>", miniFirmware);
			
			char modeSelected = read_one_letter();
			if (modeSelected == 'y' || modeSelected == 'Y') {
				printf("\n\n");
				
				watchdog_reset(delayTime);
				
				char tsbFirmware[200];
				#ifdef _WIN32
					sprintf(tsbFirmware, "tsb" DIR_SEPARATOR "tsbloader_adv" EXE_SUFFIX " -port=%s -fop=w -ffile=gbxcart_rw_mini_v1.0_pcb_r%i.hex", portname, miniFirmware);
				#elif defined(__APPLE__)
					sprintf(tsbFirmware, "mono tsb" DIR_SEPARATOR "tsbloader_adv.exe -port=%s -fop=w -ffile=gbxcart_rw_mini_v1.0_pcb_r%i.hex", comports[cport_nr], miniFirmware);
				#else
					sprintf(tsbFirmware, "tsb" DIR_SEPARATOR "tsb" EXE_SUFFIX " %s:57600 fw gbxcart_rw_mini_v1.0_pcb_r%i.hex", comports[cport_nr], miniFirmware);
				#endif
				system(tsbFirmware);
				printf("Finished\n");
			}
			else {
				printf("Update cancelled\n");
			}
		}
		else {
			printf("You are on the latest firmware\n");
		}
	}
	else if (gbxcartPcbVersion == 2 || gbxcartPcbVersion == 4) {
		if (gbxcartFirmwareVersion < standardFirmware) {
			printf("Specify delay time (in ms) or press enter for default (250ms): ");
			char optionString[5];
			fgets(optionString, 5, stdin);
			int delayTime = atoi(optionString);
			if (delayTime == 0) {
				delayTime = 250;
			}
			
			printf("Firmware R%i is available. Would you like to update? (Y/[N])\n>", standardFirmware);
			
			char modeSelected = read_one_letter();
			if (modeSelected == 'y' || modeSelected == 'Y') {
				if (gbxcartPcbVersion == 2) { // v1.1-v1.2 PCB
					printf("\n\n");
					
					watchdog_reset(delayTime);
					
					char tsbFirmware[200];
					#ifdef _WIN32
						sprintf(tsbFirmware, "tsb" DIR_SEPARATOR "tsbloader_adv" EXE_SUFFIX " -port=%s -fop=w -ffile=gbxcart_rw_v1.1-1.2_pcb_r%i.hex", portname, standardFirmware);
					#elif defined(__APPLE__)
						sprintf(tsbFirmware, "mono tsb" DIR_SEPARATOR "tsbloader_adv.exe -port=%s -fop=w -ffile=gbxcart_rw_v1.1-1.2_pcb_r%i.hex", comports[cport_nr], standardFirmware);
					#else
						sprintf(tsbFirmware, "tsb" DIR_SEPARATOR "tsb" EXE_SUFFIX " %s:57600 fw gbxcart_rw_v1.1-1.2_pcb_r%i.hex", comports[cport_nr], standardFirmware);
					#endif
					system(tsbFirmware);
					printf("Finished\n");
				}
				else if (gbxcartPcbVersion == 4) { // v1.3 PCB
					printf("\n\n");
					
					watchdog_reset(delayTime);
					
					char tsbFirmware[200];
					#ifdef _WIN32
						sprintf(tsbFirmware, "tsb" DIR_SEPARATOR "tsbloader_adv" EXE_SUFFIX " -port=%s -fop=w -ffile=gbxcart_rw_v1.3_pcb_r%i.hex", portname, standardFirmware);
					#elif defined(__APPLE__)
						sprintf(tsbFirmware, "mono tsb" DIR_SEPARATOR "tsbloader_adv.exe -port=%s -fop=w -ffile=gbxcart_rw_v1.3_pcb_r%i.hex", comports[cport_nr], standardFirmware);
					#else
						sprintf(tsbFirmware, "tsb" DIR_SEPARATOR "tsb" EXE_SUFFIX " %s:57600 fw gbxcart_rw_v1.3_pcb_r%i.hex", comports[cport_nr], standardFirmware);
					#endif
					system(tsbFirmware);
					printf("Finished\n");
					
					// To do for Mac users:
					// sprintf(tsbFirmware, "mono tsb" DIR_SEPARATOR "tsbloader_adv.exe -port=/dev/tty.usbserial-1410 -fop=w -ffile=gbxcart_rw_v1.3_pcb_r%i.hex", standardFirmware);
				}
			}
			else {
				printf("Update cancelled\n");
			}
		}
		else {
			printf("You are on the latest firmware\n");
		}
	}
	else if (gbxcartPcbVersion == 90) {
		if (gbxcartFirmwareVersion < standardFirmware) {
			printf("Please note that it may take several attempts for the update to work.\n");
			printf("Specify delay time (in ms) or press enter for default (250ms): ");
			char optionString[5];
			fgets(optionString, 5, stdin);
			int delayTime = atoi(optionString);
			if (delayTime == 0) {
				delayTime = 250;
			}
			
			printf("Firmware R%i is available. Would you like to update? (Y/[N])\n>", standardFirmware);
			
			char modeSelected = read_one_letter();
			if (modeSelected == 'y' || modeSelected == 'Y') {
				printf("\n\n");
				
				watchdog_reset(delayTime);
				
				char tsbFirmware[200];
				#ifdef _WIN32
					sprintf(tsbFirmware, "tsb" DIR_SEPARATOR "tsbloader_adv" EXE_SUFFIX " -port=%s -fop=w -ffile=gbxmas_rw_v1.0_pcb_r%i.hex", portname, standardFirmware);
				#elif defined(__APPLE__)
					sprintf(tsbFirmware, "mono tsb" DIR_SEPARATOR "tsbloader_adv.exe -port=%s -fop=w -ffile=gbxmas_rw_v1.0_pcb_r%i.hex", comports[cport_nr], standardFirmware);
				#else
					sprintf(tsbFirmware, "tsb" DIR_SEPARATOR "tsb" EXE_SUFFIX " %s:57600 fw gbxmas_rw_v1.0_pcb_r%i.hex", comports[cport_nr], standardFirmware);
				#endif
				system(tsbFirmware);
				printf("Finished\n");
			}
			else {
				printf("Update cancelled\n");
			}
		}
		else {
			printf("You are on the latest firmware\n");
		}
	}
	
	read_one_letter();
	
	return 0;
}