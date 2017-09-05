/*
 GBxCart RW - Console Interface Flasher
 Version: 1.0
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 26/08/2017
 Last Modified: 26/08/2017
 
 This program allows you to flash new ROMs to Flash Carts that are supported.
 
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
	
	printf("GBxCart RW Flasher v1.0 by insideGadgets\n");
	printf("########################################\n");
	
	// Check arguments
	if (argc >= 2) {
		read_config();
		
		// Open the port
		if (RS232_OpenComport(cport_nr, bdrate, "8N1")) {
			printf("Device not connected\n");
			read_one_letter();
			return 1;
		}
		
		// Break out of any existing functions on ATmega
		set_mode('0');
		
		// Get cartridge mode - Gameboy or Gameboy Advance
		cartridgeMode = request_value(CART_MODE);
		
		// Get firmware version
		gbxcartFirmwareVersion = request_value(READ_FIRMWARE_VERSION);
		
		// Get PCB version
		gbxcartPcbVersion = request_value(READ_PCB_VERSION);
		
		if (gbxcartPcbVersion == PCB_1_0) {
			printf("\nPCB v1.0 is not supported for this function.");
			read_one_letter();
			return 1;
		}
		
		
		// Separate the file name
		char filenameOnly[100];
		char filename[254];
		strncpy(filename, argv[1], 253);
		
		char *token = strtok(filename, "\\");
		while (token != NULL) {
			strncpy(filenameOnly, token, 99);
			token = strtok(NULL, "\\");
		}
		
		printf("\n--- Write ROM to Flash Cart ---\n");
		printf("Cart selected: ");
		
		uint32_t readBytes = 0;
		uint8_t sector = 0;
		long fileSize = 0;
		
		// Grab file size
		FILE *romFile = fopen(argv[1], "rb");
		if (romFile != NULL) {
			fseek(romFile, 0, SEEK_END);
			fileSize = ftell(romFile);
			fseek(romFile, 0, SEEK_SET);
		}
		else {
			printf("\n%s \nFile not found\n", argv[1]);
			read_one_letter();
			return 1;
		}
		
		
		// Read flash config file and match to a cart type
		read_config_flash();
		if (flashCartType == 1) {
			printf("32K Gameboy Flash Cart\n");
			currAddr = 0x0000;
			endAddr = 0x7FFF;
			
			// Check file size
			if (fileSize > (endAddr+1)) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 32K\n", argv[1]);
				read_one_letter();
				return 1;
			}
			
			printf("\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			if (gbxcartFirmwareVersion == 2) {
				uint8_t writeBuffer[1];
				while (currAddr <= endAddr) {
					if (currAddr % 0x4000 == 0) { // Erase sectors
						fw2_gb_flash_sector_erase(sector);
						wait_for_flash_ff(currAddr);
						sector++;
					}
					
					fread(writeBuffer, 1, 1, romFile);
					fw2_gb_flash_write_address_byte(currAddr, writeBuffer[0]);
					currAddr++;
					readBytes++;
					
					print_progress_percent(readBytes, (endAddr+1) / 64);
				}
			}
			else if (gbxcartFirmwareVersion == 3) {
				fw3_gb_flash_setup();
				
				while (currAddr <= endAddr) {
					if (currAddr % 0x4000 == 0) { // Erase sectors
						fw3_gb_flash_sector_erase(sector);
						wait_for_flash_ff(currAddr);
						sector++;
						
						set_number(currAddr, SET_START_ADDRESS);
					}
					
					com_write_bytes_from_file(GB_FLASH_WRITE_64BYTE_AUDIO, romFile, 64);
					com_wait_for_ack();
					currAddr += 64;
					readBytes += 64;
					
					print_progress_percent(readBytes, (endAddr+1) / 64);
				}
			}
			
			printf("]");
			fclose(romFile);
		}
		else {
			printf("No Flash Cart selected, please run this program by itself.");
			read_one_letter();
			return 1;
		}
	}	
	else {
		printf("\nPlease select a Flash Cart:\n"\
					 "1. 32K Gameboy Flash Cart\n"\
					 "x. Exit\n>");
		
		char optionSelected = read_one_letter();
		if (optionSelected == '1') {
			write_flash_config(1);
		}
		
		printf("\nYou can now drag and drop your ROM file to this exe file.");
	}
	
	return 0;
}