/*
 GBxCart RW - Nintendo Power Cart, Restore Multi-Game Cart save games
 Version: 1.0
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 20/03/2019
 Last Modified: 26/03/2019
 
 Restore Multi-game cart saves to your Gameboy 1MB Nintendo Power Flash Cart. 
 
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

#include "setup.h" // See defines, variables, constants, functions here

int main(int argc, char **argv) {
	
	printf("GBxCart RW - Nintendo Power Cart Restore Multi-Game Saves by insideGadgets\n");
	printf("##########################################################################\n");
	
	// Check arguments
	if (argc >= 2) {
		read_config();
		
		// Open COM port
		if (com_test_port() == 0) {
			printf("Device not connected and couldn't be auto detected\n");
			read_one_letter();
			return 1;
		}
		printf("Connected on COM port: %i\n", cport_nr+1);
		
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
		
		if (gbxcartFirmwareVersion <= 12) {
			printf("Firmware R13 or higher is required for this functionality.\n");
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
		
		// Grab file size
		long fileSize = 0;
		FILE *ramFile = fopen(argv[1], "rb");
		if (ramFile != NULL) {
			fseek(ramFile, 0, SEEK_END);
			fileSize = ftell(ramFile);
			fseek(ramFile, 0, SEEK_SET);
		}
		else {
			printf("\n%s \nFile not found\n", argv[1]);
			read_one_letter();
			return 1;
		}
		
		// Check file size
		if (fileSize > (endAddr+1)) {
			fclose(ramFile);
			printf("\n%s \nFile size is larger than the available save space of 32KB\n", argv[1]);
			read_one_letter();
			return 1;
		}
		
		// PCB v1.3 - Set 5V
		if (gbxcartPcbVersion == PCB_1_3) {
			set_mode(VOLTAGE_5V);
			delay_ms(100);
		}
		
		
		// Ask for game slot
		printf("\nPlease select a game slot to restore to:\n");
		
		// Query list of game titles
		for (uint8_t g = 1; g < 8; g++) {
			// Enable flash chip access
			set_bank(0x120, 0x09);
			set_bank(0x121, 0xaa);
			set_bank(0x122, 0x55);
			set_bank(0x13f, 0xa5);
			
			// Load game
			set_bank(0x120, 0x80 + g);
			set_bank(0x13f, 0xa5);
			delay_ms(100);
			
			printf("%d: ", g);
			
			// Check if game has the Nintendo logo
			if (check_for_valid_nintendo_logo()) {
				printf("%s", gameTitle);
			}
			printf("\n");
		}
		printf(">");
		
		char optionString[5];
		fgets(optionString, 5, stdin);
		uint8_t optionSelected = atoi(optionString);
		
		// Enable flash chip access
		set_bank(0x120, 0x09);
		set_bank(0x121, 0xaa);
		set_bank(0x122, 0x55);
		set_bank(0x13f, 0xa5);
		
		// Load game
		set_bank(0x120, 0x80 + optionSelected);
		set_bank(0x13f, 0xa5);
		delay_ms(100);
		
		printf("\nSelecting Game -\n");
		read_gb_header();
		
		// Does cartridge have RAM
		if (ramEndAddress > 0) {
			
			printf("\nGoing to restore save from %s...", filenameOnly);
			printf("\n\n*** This will erase the save game from your Gameboy Cartridge ***");
			printf("\nPress y to continue or any other key to abort.\n");
			
			char confirmWrite = read_one_letter();
			if (confirmWrite == 'y') {
				printf("\nRestoring save from %s\n", filenameOnly);
				printf("[             25%%             50%%             75%%            100%%]\n[");
				
				mbc2_fix();
				if (cartridgeType <= 4) { // MBC1
					set_bank(0x6000, 1); // Set RAM Mode
				}
				set_bank(0x0000, 0x0A); // Initialise MBC
				
				// Write RAM
				uint32_t readBytes = 0;
				for (uint8_t bank = 0; bank < ramBanks; bank++) {
					uint16_t ramAddress = 0xA000;
					set_bank(0x4000, bank);
					set_number(0xA000, SET_START_ADDRESS); // Set start address again
					
					while (ramAddress < ramEndAddress) {
						com_write_bytes_from_file(WRITE_RAM, ramFile, 64);
						ramAddress += 64;
						readBytes += 64;
						
						// Print progress
						if (ramEndAddress == 0xA1FF) {
							print_progress_percent(readBytes, 64);
						}
						else if (ramEndAddress == 0xA7FF) {
							print_progress_percent(readBytes / 4, 64);
						}
						else {
							print_progress_percent(readBytes, (ramBanks * (ramEndAddress - 0xA000 + 1)) / 64);
						}
						
						com_wait_for_ack();
					}
				}
				printf("]");
				set_bank(0x0000, 0x00); // Disable RAM
				
				fclose(ramFile);
				printf("\nFinished\n");
			}
			else {
				printf("\nAborted\n");
			}
		}
		else {
			printf("\nCartridge has no RAM\n");
		}
	}
	
	read_one_letter();
	
	return 0;
}