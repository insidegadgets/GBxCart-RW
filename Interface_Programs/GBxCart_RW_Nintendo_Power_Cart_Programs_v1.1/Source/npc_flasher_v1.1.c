/*
 GBxCart RW - Nintendo Power Cart Backup Full ROM/Hidden Sector
 Version: 1.1
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 20/03/2019
 Last Modified: 4/05/2019
 
 Write a ROM to the Gameboy 1MB Nintendo Power Flash Cart.
 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "setup.h" // See defines, variables, constants, functions here

int main(int argc, char **argv) {
	
	printf("GBxCart RW - Nintendo Power Cart Flasher by insideGadgets\n");
	printf("#########################################################\n");
	
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
		
		// PCB v1.3 - Set 5V
		if (gbxcartPcbVersion == PCB_1_3) {
			set_mode(VOLTAGE_5V);
			delay_ms(100);
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
		printf("Cart selected: Nintendo Power 1MB Gameboy Flash Cart");
		
		uint32_t readBytes = 0;
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
		
		
		printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
		
		currAddr = 0x0000;
		endAddr = 0x7FFF;
		
		// Check file size
		if (fileSize > 0x100000) {
			fclose(romFile);
			printf("\n%s \nFile size is larger than the available Flash cart space of 1 MByte\n", argv[1]);
			read_one_letter();
			return 1;
		}
		
		// Calculate banks needed from ROM file size
		romBanks = fileSize / 16384;
		
		// Flash Setup
		set_mode(GB_CART_MODE); // Gameboy mode
		gb_np_enable_flash_chip_access();
		
		// Check if .map hidden sector file is present
		char romMapFileName[250];
		strncpy(romMapFileName, argv[1], strlen(argv[1])-3);
		strncat(romMapFileName, ".map", 5);
		
		FILE *romMapFile = fopen(romMapFileName, "rb");
		if (romMapFile != NULL) { // Map file found, read 64 bytes to array
			fread(npHiddenRegister, 64, 1, romMapFile);
			fclose(romMapFile);
		}
		else { // No .map file found
			// Read out MBC type and Ram size from ROM file
			npHiddenRegister[0] = 0;
			npHiddenRegister[1] = 0;
			uint8_t fileReadBuffer[1];
			fseek(romFile, 0x0147, SEEK_SET); // MBC type
			fread(fileReadBuffer, 1, 1, romFile);
			uint8_t mbcType = fileReadBuffer[0];
			
			fseek(romFile, 0x0149, SEEK_SET); // RAM size
			fread(fileReadBuffer, 1, 1, romFile);
			uint8_t ramSize = fileReadBuffer[0];
			fseek(romFile, 0, SEEK_SET); // Rewind
			
			// MBC
			if (mbcType <= 0x03) { // MBC1
				npHiddenRegister[0] = 0x20;
			}
			else if (mbcType >= 0x05 && mbcType <= 0x06) { // MBC2
				npHiddenRegister[0] = 0x40;
			}
			else if (mbcType >= 0x0F && mbcType <= 0x13) { // MBC3
				npHiddenRegister[0] = 0x60;
			}
			else { // MBC5
				npHiddenRegister[0] = 0xA0;
			}
			
			// Default 1MB ROM size
			npHiddenRegister[0] |= 0x14;
			
			// RAM
			if (ramSize == 0x02) { // 8KB RAM
				npHiddenRegister[0] |= 0x01;
			}
			else if (ramSize >= 0x03) { // 32KB RAM
				npHiddenRegister[0] |= 0x01;
				npHiddenRegister[1] = 0x80;
			}
		}
		
		
		// Erase chip and map all flash memory
		gb_np_enable_flash_chip_access();
		gb_np_unlock_sector_0();
		gb_np_chip_erase();
		gb_np_map_all_flash_memory();
		
		printf("\n\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
		printf("[             25%%             50%%             75%%            100%%]\n[");
		
		// Write ROM
		currAddr = 0x0000;
		endAddr = 0x7FFF;
		for (uint16_t bank = 1; bank < romBanks; bank++) {				
			if (bank > 1) { currAddr = 0x4000; }
			
			// Set start address
			set_number(currAddr, SET_START_ADDRESS);
			
			// Read data
			while (currAddr < endAddr) {
				if (currAddr == 0x4000) { // Switch banks here just before the next bank, not any time sooner
					set_bank(0x2100, bank);
				}
				
				//printf("bank 0x%X, addr 0x%X\n", bank, currAddr);
				com_write_bytes_from_file(GB_FLASH_WRITE_NP_128BYTE, romFile, 128);
				com_wait_for_ack();
				currAddr += 128;
				readBytes += 128;
				
				// Print progress
				print_progress_percent(readBytes, (romBanks * 16384) / 64);
			}
		}
		
		printf("]\n\n");
		fclose(romFile);
		
		// Write hidden sector
		gb_np_enable_flash_chip_access();
		gb_np_unlock_sector_0();
		gb_np_erase_hidden_sector();
		gb_np_write_hidden_sector();
	}
	printf("\n");
	
	return 0;
}