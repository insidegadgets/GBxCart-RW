/*
 GBxCart RW - Nintendo Power Cart, Restore Multi-Game Cart save games
 Version: 1.1c
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 20/03/2019
 Last Modified: 27/01/2021
 
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
	
	printf("GBxCart RW - Nintendo Power Cart:\n");
	printf("Restore Multi-Game Saves v1.1c by insideGadgets\n");
	printf("###############################################\n");
	
	// Check arguments
	if (argc >= 2) {
		read_config();
		
		// Open COM port
		if (com_test_port() == 0) {
			printf("Device not connected and couldn't be auto detected. Please make sure the COM port isn't open elsewhere. If you are using Linux/Mac, make sure you use \"sudo\".\n");
			read_one_letter();
			return 1;
		}
		printf("Connected on COM port: %i at ", cport_nr+1);
		
		// Break out of any existing functions on ATmega
		set_mode('0');
		RS232_flushRX(cport_nr);
		
		// Get PCB version
		gbxcartPcbVersion = request_value(READ_PCB_VERSION);
		xmas_wake_up();
		
		// Increase to 1.7Mbit baud for v1.4 PCB
		if (gbxcartPcbVersion == PCB_1_4) {
			set_mode(USART_1_7M_SPEED);
			RS232_CloseComport(cport_nr);
			delay_ms(200);
			
			if (RS232_OpenComport(cport_nr, 1700000, "8N1") == 0) {
				printf("1.7M Baud\n");
			}
		}
		else {
			printf("1M Baud\n");
		}
		
		char pcbVersionString[20];
		if (gbxcartPcbVersion == PCB_1_0) {
			strncpy(pcbVersionString, "v1.0", 5);
		}
		else if (gbxcartPcbVersion == PCB_1_1) {
			strncpy(pcbVersionString, "v1.1/v1.2", 10);
		}
		else if (gbxcartPcbVersion == PCB_1_3) {
			strncpy(pcbVersionString, "v1.3", 5);
		}
		else if (gbxcartPcbVersion == PCB_1_4) {
			strncpy(pcbVersionString, "v1.4", 5);
		}
		else if (gbxcartPcbVersion == MINI) {
			strncpy(pcbVersionString, "Mini", 5);
		}
		else if (gbxcartPcbVersion == GBXMAS) {
			strncpy(pcbVersionString, "XMAS", 5);
		}
		
		// Get firmware version
		gbxcartFirmwareVersion = request_value(READ_FIRMWARE_VERSION);
		printf("PCB version: %s, Firmware version: %i\n", pcbVersionString, gbxcartFirmwareVersion);
		
		set_mode(GB_CART_MODE);
		
		// PCB v1.3+ - Set 5V
		if (gbxcartPcbVersion == PCB_1_3 || gbxcartPcbVersion == PCB_1_4 || gbxcartPcbVersion == GBXMAS) {
			if (gbxcartPcbVersion == GBXMAS) {
				xmas_set_leds(0x6555955);
				delay_ms(50);
				set_mode('!');
				delay_ms(50); // Wait for ATmega169 to WDT reset if more than 2 mins idle
			}
			set_mode(VOLTAGE_5V);
			delay_ms(100);
		}
		
		// GBx v1.4 - Power up the cart if not already powered up and flush buffer
		gbx_cart_power_up();
		RS232_flushRX(cport_nr);
		
		// Get cartridge mode - Gameboy or Gameboy Advance
		cartridgeMode = request_value(CART_MODE);
		
		
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
				
				if (ramEndAddress == 0xA1FF) {
					xmas_setup(ramEndAddress / 28);
				}
				else if (ramEndAddress == 0xA7FF) {
					xmas_setup(ramEndAddress / 4 / 28);
				}
				else {
					xmas_setup((ramBanks * (ramEndAddress - 0xA000 + 1)) / 28);
				}
				
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
							led_progress_percent(readBytes, 28);
						}
						else if (ramEndAddress == 0xA7FF) {
							print_progress_percent(readBytes / 4, 64);
							led_progress_percent(readBytes / 4, 28);
						}
						else {
							print_progress_percent(readBytes, (ramBanks * (ramEndAddress - 0xA000 + 1)) / 64);
							led_progress_percent(readBytes, (ramBanks * (ramEndAddress - 0xA000 + 1)) / 28);
						}
						
						com_wait_for_ack();
					}
				}
				printf("]");
				set_bank(0x0000, 0x00); // Disable RAM
				
				fclose(ramFile);
				printf("\nFinished\n");
				
				gbx_set_done_led();
			}
			else {
				printf("\nAborted\n");
			}
		}
		else {
			printf("\nCartridge has no RAM\n");
		}
	}
	
	gbx_cart_power_down();
	read_one_letter();
	
	return 0;
}