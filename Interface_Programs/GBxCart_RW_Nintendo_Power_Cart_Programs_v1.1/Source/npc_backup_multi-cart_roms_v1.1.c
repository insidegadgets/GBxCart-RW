/*
 GBxCart RW - Nintendo Power Cart, Backup Multi-Game Cart ROMs
 Version: 1.1
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 20/03/2019
 Last Modified: 4/05/2019
 
 Backup your Gameboy 1MB Nintendo Power Flash Cart's Multi-game ROMs. 
 
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
	
	printf("GBxCart RW - Nintendo Power Cart Backup Multi-Game ROMs by insideGadgets\n");
	printf("########################################################################\n");
	
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
	
	// Get firmware version
	gbxcartFirmwareVersion = request_value(READ_FIRMWARE_VERSION);
	
	// Get PCB version
	gbxcartPcbVersion = request_value(READ_PCB_VERSION);
	
	// PCB v1.3 - Set 5V
	if (gbxcartPcbVersion == PCB_1_3) {
		set_mode(VOLTAGE_5V);
		delay_ms(100);
	}
	
	// Get the date/time to be used for the filename
	time_t rawtime;
	struct tm* timeinfo;
	char timebuffer[130];
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	strftime (timebuffer,120,"_%Y-%m-%d_%H-%M-%S.gb",timeinfo);
	
	
	// Loop loader + 7 games
	char loaderGameTitle[18];
	for (uint8_t g = 0; g < 8; g++) {
		// For multi-games
		if (g >= 1) {
			// Enable flash chip access
			set_bank(0x120, 0x09);
			set_bank(0x121, 0xaa);
			set_bank(0x122, 0x55);
			set_bank(0x13f, 0xa5);
			
			// Load game
			set_bank(0x120, 0x80 + g);
			set_bank(0x13f, 0xa5);
			delay_ms(100);
			
			printf("\nMulti-Game Cart - Game %d\n", g);
		}
		else {
			printf("\nLoader\n");
		}
		
		// Check if game has the Nintendo logo
		if (check_for_valid_nintendo_logo()) {
			read_gb_header();
			
			// Only backup ROM if game title doesn't match the loader's game title
			if (strncmp(gameTitle, loaderGameTitle, 17) != 0) {
				
				// Store loader game title
				if (g == 0) {
					strncpy(loaderGameTitle, gameTitle, 18);
				}
				
				char titleFilename[250];
				strncpy(titleFilename, "NP_", 20);
				strncat(titleFilename, gameTitle, 20);
				strncat(titleFilename, timebuffer, 121);
				strncat(titleFilename, "\0", 2);
				
				printf("Reading ROM to %s\n", titleFilename);
				printf("[             25%%             50%%             75%%            100%%]\n[");
				
				// Create a new file
				FILE *romFile = fopen(titleFilename, "wb");
				
				uint32_t readBytes = 0;
				if (cartridgeMode == GB_MODE) {
					// Set start and end address
					currAddr = 0x0000;
					endAddr = 0x7FFF;
					
					// Read ROM
					for (uint16_t bank = 1; bank < romBanks; bank++) {				
						if (cartridgeType >= 5) { // MBC2 and above
							set_bank(0x2100, bank & 0xFF);
							if (bank >= 256) {
								set_bank(0x3000, 1); // High bit
							}
						}
						else { // MBC1
							set_bank(0x6000, 0); // Set ROM Mode 
							set_bank(0x4000, bank >> 5); // Set bits 5 & 6 (01100000) of ROM bank
							set_bank(0x2000, bank & 0x1F); // Set bits 0 & 4 (00011111) of ROM bank
						}
						
						if (bank > 1) { currAddr = 0x4000; }
						
						// Set start address and rom reading mode
						set_number(currAddr, SET_START_ADDRESS);
						set_mode(READ_ROM_RAM);
						
						// Read data
						while (currAddr < endAddr) {
							uint8_t comReadBytes = com_read_bytes(romFile, 64);
							if (comReadBytes == 64) {
								currAddr += 64;
								readBytes += 64;
								
								// Request 64 bytes more
								if (currAddr < endAddr) {
									com_read_cont();
								}
							}
							else { // Didn't receive 64 bytes, usually this only happens for Apple MACs
								fflush(romFile);
								com_read_stop();
								delay_ms(500);
								printf("Retrying\n");
								
								// Flush buffer
								RS232_PollComport(cport_nr, readBuffer, 64);											
								
								// Start off where we left off
								fseek(romFile, readBytes, SEEK_SET);
								set_number(currAddr, SET_START_ADDRESS);
								set_mode(READ_ROM_RAM);				
							}
							
							// Print progress
							print_progress_percent(readBytes, (romBanks * 16384) / 64);
						}
						com_read_stop(); // Stop reading ROM (as we will bank switch)
					}
					printf("]");
				}
				
				fclose(romFile);
				printf("\nFinished\n");
			}
		}
	}
	
	return 0;
}