/*
 GBxCart RW - Console Interface Flasher
 Version: 1.13
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 26/08/2017
 Last Modified: 5/10/2018
 
 This program allows you to write ROM to Flash Carts that are supported.
 
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
	
	printf("GBxCart RW Flasher v1.13 by insideGadgets\n");
	printf("########################################\n");
	
	// Check arguments
	if (argc >= 2) {
		read_config();
		
		// Open COM port
		if (com_test_port() == 0) {
			printf("Device not connected and couldn't be auto detected\n");
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
		
		if (gbxcartFirmwareVersion <= 8) {
			printf("Firmware R9 or higher is required for this functionality.\n");
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
		uint16_t sector = 0;
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
		
		
		// ****** GB Flash Carts ******
		if (flashCartType >= 101) {
			if (flashCartType == 101) {
				printf("32 KByte AM29F010B Gameboy Flash Cart (Audio as WE)\n");
			}
			else if (flashCartType == 102) {
				printf("32 KByte AM29F010B Gameboy Flash Cart (WR as WE)\n");
			}
			else if (flashCartType == 103) {
				printf("32 KByte SST39SF010A / AT49F040 Gameboy Flash Cart (Audio as WE)\n");
			}
			else if (flashCartType == 104) {
				printf("32 KByte SST39SF010A / AT49F040 Gameboy Flash Cart (WR as WE)\n");
			}
			
			currAddr = 0x0000;
			endAddr = 0x7FFF;
			
			// Check file size
			if (fileSize > (endAddr+1)) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 32K\n", argv[1]);
				read_one_letter();
				return 1;
			}
			
			// Flash Setup
			set_mode(GB_CART_MODE); // Gameboy mode
			if (flashCartType == 101 || flashCartType == 103) {
				gb_flash_pin_setup(WE_AS_AUDIO_PIN); // Audio pin
			}
			else {
				gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			}
			if (flashCartType == 101 || flashCartType == 102) {
				gb_flash_program_setup(GB_FLASH_PROGRAM_555);// Flash program byte method
			}
			else {
				gb_flash_program_setup(GB_FLASH_PROGRAM_5555);// Flash program byte method
			}
			
			// Chip erase for this flash chip
			if (flashCartType == 103 || flashCartType == 104) {
				printf("\nErasing Flash");
				gb_flash_write_address_byte(0x5555, 0xAA);
				gb_flash_write_address_byte(0x2AAA, 0x55);
				gb_flash_write_address_byte(0x5555, 0x80);
				gb_flash_write_address_byte(0x5555, 0xAA);
				gb_flash_write_address_byte(0x2AAA, 0x55);
				gb_flash_write_address_byte(0x5555, 0x10);
				
				// Wait for first byte to be 0xFF
				wait_for_flash_chip_erase_ff();
			}
			
			printf("\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			
			// Write ROM
			set_number(currAddr, SET_START_ADDRESS);
			while (currAddr <= endAddr) {
				if (flashCartType == 101 || flashCartType == 102) { // Sector erase for this flash chip
					if (currAddr % 0x4000 == 0) { // Erase sectors
						gb_flash_write_address_byte(0x555, 0xAA);
						gb_flash_write_address_byte(0x2AA, 0x55);
						gb_flash_write_address_byte(0x555, 0x80);
						gb_flash_write_address_byte(0x555, 0xAA);
						gb_flash_write_address_byte(0x2AA, 0x55);
						gb_flash_write_address_byte(sector << 14, 0x30);
						
						wait_for_flash_sector_ff(currAddr);
						sector++;
						
						set_number(currAddr, SET_START_ADDRESS);
					}
				}
				
				com_write_bytes_from_file(GB_FLASH_WRITE_64BYTE, romFile, 64);
				com_wait_for_ack();
				currAddr += 64;
				readBytes += 64;
				
				print_progress_percent(readBytes, (endAddr+1) / 64);
			}
			
			printf("]");
			fclose(romFile);
		}
		
		else if (flashCartType == 2) {
			printf("512 KByte (SST39SF040) Gameboy Flash Cart\n");
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			// Check file size
			if (fileSize > 0x80000) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 512 KByte\n", argv[1]);
				read_one_letter();
				return 1;
			}
			
			currAddr = 0x0000;
			endAddr = 0x7FFF;
			
			// Calculate banks needed from ROM file size
			romBanks = fileSize / 16384;
			
			// Flash Setup
			set_mode(GB_CART_MODE); // Gameboy mode
			set_mode(GB_FLASH_BANK_1_COMMAND_WRITES); // Set bank 1 before issuing flash commands
			gb_flash_pin_setup(WE_AS_AUDIO_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_5555); // Flash program byte method
			
			// Set bank 1 before issuing flash commands
			set_bank(0x2100, 1);
			
			// Chip erase for this flash chip
			printf("\nErasing Flash");
			gb_flash_write_address_byte(0x5555, 0xAA);
			gb_flash_write_address_byte(0x2AAA, 0x55);
			gb_flash_write_address_byte(0x5555, 0x80);
			gb_flash_write_address_byte(0x5555, 0xAA);
			gb_flash_write_address_byte(0x2AAA, 0x55);
			gb_flash_write_address_byte(0x5555, 0x10);
			
			// Wait for first byte to be 0xFF
			wait_for_flash_chip_erase_ff();
			
			
			printf("\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Write ROM
			currAddr = 0x0000;
			for (uint16_t bank = 1; bank < romBanks; bank++) {				
				if (bank > 1) { currAddr = 0x4000; }
				
				// Set start address
				set_number(currAddr, SET_START_ADDRESS);
				
				// Read data
				while (currAddr < endAddr) {
					if (currAddr == 0x4000) { // Switch banks here just before the next bank, not any time sooner
						set_bank(0x2100, bank);
					}
					
					com_write_bytes_from_file(GB_FLASH_WRITE_64BYTE, romFile, 64);
					com_wait_for_ack();
					currAddr += 64;
					readBytes += 64;
					
					// Print progress
					print_progress_percent(readBytes, (romBanks * 16384) / 64);
				}
			}
			
			printf("]");
			fclose(romFile);
		}
		
		else if (flashCartType == 3) {
			printf("2 MByte (BV5) Gameboy Flash Cart\n");
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			// Check file size
			if (fileSize > 0x200000) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 2 MByte\n", argv[1]);
				read_one_letter();
				return 1;
			}
			
			currAddr = 0x0000;
			endAddr = 0x7FFF;
			
			// Calculate banks needed from ROM file size
			romBanks = fileSize / 16384;
			
			// Flash Setup
			set_mode(GB_CART_MODE); // Gameboy mode
			gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_AAA_BIT01_SWAPPED);// Flash program byte method
			
			// Chip erase (data byte's bit 0 & 1 are swapped for chip commands as BV5 D0 & D1 lines are swapped)
			printf("\nErasing Flash");
			gb_flash_write_address_byte(0xAAA, 0xA9);
			gb_flash_write_address_byte(0x555, 0x56);
			gb_flash_write_address_byte(0xAAA, 0x80);
			gb_flash_write_address_byte(0xAAA, 0xA9);
			gb_flash_write_address_byte(0x555, 0x56);
			gb_flash_write_address_byte(0xAAA, 0x10);
			
			// Wait for first byte to be 0xFF
			wait_for_flash_chip_erase_ff();
			
			
			printf("\n\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Write ROM
			currAddr = 0x0000;
			for (uint16_t bank = 1; bank < romBanks; bank++) {				
				if (bank > 1) { currAddr = 0x4000; }
				
				// Set start address
				set_number(currAddr, SET_START_ADDRESS);
				
				// Read data
				while (currAddr < endAddr) {
					if (currAddr == 0x4000) { // Switch banks here just before the next bank, not any time sooner
						set_bank(0x2100, bank);
					}
					
					com_write_bytes_from_file(GB_FLASH_WRITE_64BYTE, romFile, 64);
					com_wait_for_ack();
					currAddr += 64;
					readBytes += 64;
					
					// Print progress
					print_progress_percent(readBytes, (romBanks * 16384) / 64);
				}
			}
			
			printf("]");
			fclose(romFile);
		}
		
		else if (flashCartType == 4) {
			printf("1 MByte (ES29LV160) Gameboy Flash Cart\n");
			
			if (cartridgeMode == GB_MODE) {
				printf("You must switch GBxCart RW to be powered by 3.3V.\n");
				printf("Please unplug it, switch the voltage and re-connect.\n");
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
			gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_555_BIT01_SWAPPED);// Flash program byte method
			
			// Chip erase (data byte's bit 0 & 1 are swapped for chip commands as BV5 D0 & D1 lines are swapped)
			printf("\nErasing Flash");
			gb_flash_write_address_byte(0x555, 0xA9);
			gb_flash_write_address_byte(0x2AA, 0x56);
			gb_flash_write_address_byte(0x555, 0x80);
			gb_flash_write_address_byte(0x555, 0xA9);
			gb_flash_write_address_byte(0x2AA, 0x56);
			gb_flash_write_address_byte(0x555, 0x10);
			
			// Wait for first byte to be 0xFF
			wait_for_flash_chip_erase_ff();
			
			
			printf("\n\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Write ROM
			currAddr = 0x0000;
			for (uint16_t bank = 1; bank < romBanks; bank++) {				
				if (bank > 1) { currAddr = 0x4000; }
				
				// Set start address
				set_number(currAddr, SET_START_ADDRESS);
				
				// Read data
				while (currAddr < endAddr) {
					if (currAddr == 0x4000) { // Switch banks here just before the next bank, not any time sooner
						set_bank(0x2100, bank);
					}
					
					com_write_bytes_from_file(GB_FLASH_WRITE_64BYTE, romFile, 64);
					com_wait_for_ack();
					currAddr += 64;
					readBytes += 64;
					
					// Print progress
					print_progress_percent(readBytes, (romBanks * 16384) / 64);
				}
			}
			
			printf("]");
			fclose(romFile);
		}
		
		else if (flashCartType == 5) {
			printf("2 MByte (AM29LV160DB / 29LV160CTTC / 29LV160TE) Gameboy Flash Cart\n");
			
			if (cartridgeMode == GB_MODE) {
				printf("You must switch GBxCart RW to be powered by 3.3V.\n");
				printf("Please unplug it, switch the voltage and re-connect.\n");
				read_one_letter();
				return 1;
			}
			
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			currAddr = 0x0000;
			endAddr = 0x7FFF;
			
			// Check file size
			if (fileSize > 0x200000) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 2 MByte\n", argv[1]);
				read_one_letter();
				return 1;
			}
			
			// Calculate banks needed from ROM file size
			romBanks = fileSize / 16384;
			
			// Flash Setup
			set_mode(GB_CART_MODE); // Gameboy mode
			gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_AAA);// Flash program byte method
			
			// Chip erase
			printf("\nErasing Flash");
			gb_flash_write_address_byte(0xAAA, 0xAA);
			gb_flash_write_address_byte(0x555, 0x55);
			gb_flash_write_address_byte(0xAAA, 0x80);
			gb_flash_write_address_byte(0xAAA, 0xAA);
			gb_flash_write_address_byte(0x555, 0x55);
			gb_flash_write_address_byte(0xAAA, 0x10);
			
			// Wait for first byte to be 0xFF
			wait_for_flash_chip_erase_ff();
			
			
			printf("\n\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Write ROM
			currAddr = 0x0000;
			for (uint16_t bank = 1; bank < romBanks; bank++) {				
				if (bank > 1) { currAddr = 0x4000; }
				
				// Set start address
				set_number(currAddr, SET_START_ADDRESS);
				
				// Read data
				while (currAddr < endAddr) {
					if (currAddr == 0x4000) { // Switch banks here just before the next bank, not any time sooner
						set_bank(0x2100, bank);
					}
					
					com_write_bytes_from_file(GB_FLASH_WRITE_64BYTE, romFile, 64);
					com_wait_for_ack();
					currAddr += 64;
					readBytes += 64;
					
					// Print progress
					print_progress_percent(readBytes, (romBanks * 16384) / 64);
				}
			}
			
			printf("]");
			fclose(romFile);
		}
		
		else if (flashCartType == 6) {
			printf("2 MByte (AM29F016B) Gameboy Flash Cart\n");
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			// Check file size
			if (fileSize > 0x200000) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 2 MByte\n", argv[1]);
				read_one_letter();
				return 1;
			}
			
			currAddr = 0x0000;
			endAddr = 0x7FFF;
			
			// Calculate banks needed from ROM file size
			romBanks = fileSize / 16384;
			
			// Flash Setup
			set_mode(GB_CART_MODE); // Gameboy mode
			gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_555);// Flash program byte method
			
			// Chip erase
			printf("\nErasing Flash");
			gb_flash_write_address_byte(0x555, 0xAA);
			gb_flash_write_address_byte(0x2AA, 0x55);
			gb_flash_write_address_byte(0x555, 0x80);
			gb_flash_write_address_byte(0x555, 0xAA);
			gb_flash_write_address_byte(0x2AA, 0x55);
			gb_flash_write_address_byte(0x555, 0x10);
			
			// Wait for first byte to be 0xFF
			wait_for_flash_chip_erase_ff();
			
			
			printf("\n\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Write ROM
			currAddr = 0x0000;
			for (uint16_t bank = 1; bank < romBanks; bank++) {				
				if (bank > 1) { currAddr = 0x4000; }
				
				// Set start address
				set_number(currAddr, SET_START_ADDRESS);
				
				// Read data
				while (currAddr < endAddr) {
					if (currAddr == 0x4000) { // Switch banks here just before the next bank, not any time sooner
						set_bank(0x2100, bank);
					}
					
					com_write_bytes_from_file(GB_FLASH_WRITE_64BYTE, romFile, 64);
					com_wait_for_ack();
					currAddr += 64;
					readBytes += 64;
					
					// Print progress
					print_progress_percent(readBytes, (romBanks * 16384) / 64);
				}
			}
			
			printf("]");
			fclose(romFile);
		}
		
		else if (flashCartType == 7) {
			printf("32 MByte (4x 8MB Banks) (256M29) Gameboy Flash Cart\n");
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			printf("\n*** After writing is complete, you will need to power cycle the device in order to write to the other 8MB banks ***\n\n");
			
			if (cartridgeMode == GB_MODE) {
				printf("You must switch GBxCart RW to be powered by 3.3V.\n");
				printf("Please unplug it, switch the voltage and re-connect.\n");
				read_one_letter();
				return 1;
			}
			
			if (gbxcartFirmwareVersion <= 10) {
				printf("Firmware R10 or higher is required for this functionality.\n");
				read_one_letter();
				return 1;
			}
			
			// Check file size
			if (fileSize > 0x800000) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 8 MByte\n", argv[1]);
				read_one_letter();
				return 1;
			}
			
			currAddr = 0x0000;
			endAddr = 0x7FFF;
			
			// Calculate banks needed from ROM file size
			romBanks = fileSize / 16384;
			
			// Flash Setup
			set_mode(GB_CART_MODE); // Gameboy mode
			gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_AAA_BIT01_SWAPPED);// Flash program byte method
			
			
			printf("Please enter which 8MB bank number we should write to (1-4):");
			char bankString[5];
			fgets(bankString, 5, stdin);
			int bankNumber = atoi(bankString);
			
			if (bankNumber == 1) {
				// Chip erase
				printf("\nErasing Flash (may take 1 minute)");
				gb_flash_write_address_byte(0xAAA, 0xA9);
				gb_flash_write_address_byte(0x555, 0x56);
				gb_flash_write_address_byte(0xAAA, 0x80);
				gb_flash_write_address_byte(0xAAA, 0xA9);
				gb_flash_write_address_byte(0x555, 0x56);
				gb_flash_write_address_byte(0xAAA, 0x10);
				
				// Wait for first byte to be 0xFF
				wait_for_flash_chip_erase_ff();
				
				// Set first 8MB bank
				gb_flash_write_address_byte(0x7000, 0x00);
				gb_flash_write_address_byte(0x7001, 0x00);
				gb_flash_write_address_byte(0x7002, 0x90);
				delay_ms(1);
			}
			else if (bankNumber == 2) {
				gb_flash_write_address_byte(0x7000, 0x00);
				gb_flash_write_address_byte(0x7001, 0x00);
				gb_flash_write_address_byte(0x7002, 0x91);
				delay_ms(1);
			}
			else if (bankNumber == 3) {
				gb_flash_write_address_byte(0x7000, 0x00);
				gb_flash_write_address_byte(0x7001, 0x00);
				gb_flash_write_address_byte(0x7002, 0x92);
				delay_ms(1);
			}
			else if (bankNumber == 4) {
				gb_flash_write_address_byte(0x7000, 0x00);
				gb_flash_write_address_byte(0x7001, 0x00);
				gb_flash_write_address_byte(0x7002, 0x93);
				delay_ms(1);
			}
			
			
			printf("\n\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Write ROM
			currAddr = 0x0000;
			for (uint16_t bank = 1; bank < romBanks; bank++) {				
				if (bank > 1) { currAddr = 0x4000; }
				
				// Set start address
				set_number(currAddr, SET_START_ADDRESS);
				
				// Read data
				while (currAddr < endAddr) {
					if (currAddr == 0x4000) { // Switch banks here just before the next bank, not any time sooner
						set_bank(0x2100, bank);
						if (bank >= 256) {
							set_bank(0x3000, 1); // High bit
						}
						else {
							set_bank(0x3000, 0); // High bit
						}
					}
					
					com_write_bytes_from_file(GB_FLASH_WRITE_256BYTE, romFile, 256);
					com_wait_for_ack();
					currAddr += 256;
					readBytes += 256;
					
					// Print progress
					print_progress_percent(readBytes, (romBanks * 16384) / 64);
				}
			}
			
			printf("]");
			fclose(romFile);
		}
		
		else if (flashCartType == 8) {
			printf("4 MByte (M29W640) Gameboy Flash Cart\n");
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			if (cartridgeMode == GB_MODE) {
				printf("You must switch GBxCart RW to be powered by 3.3V.\n");
				printf("Please unplug it, switch the voltage and re-connect.\n");
				read_one_letter();
				return 1;
			}
			
			// Check file size
			if (fileSize > 0x400000) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 4 MByte\n", argv[1]);
				read_one_letter();
				return 1;
			}
			
			currAddr = 0x0000;
			endAddr = 0x7FFF;
			
			// Calculate banks needed from ROM file size
			romBanks = fileSize / 16384;
			
			// Flash Setup
			set_mode(GB_CART_MODE); // Gameboy mode
			gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_AAA_BIT01_SWAPPED);// Flash program byte method
			
			// Chip erase (data byte's bit 0 & 1 are swapped for chip commands as BV5 D0 & D1 lines are swapped)
			printf("\nErasing Flash");
			gb_flash_write_address_byte(0xAAA, 0xA9);
			gb_flash_write_address_byte(0x555, 0x56);
			gb_flash_write_address_byte(0xAAA, 0x80);
			gb_flash_write_address_byte(0xAAA, 0xA9);
			gb_flash_write_address_byte(0x555, 0x56);
			gb_flash_write_address_byte(0xAAA, 0x10);
			
			// Wait for first byte to be 0xFF
			wait_for_flash_chip_erase_ff();
			
			
			printf("\n\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Write ROM
			currAddr = 0x0000;
			for (uint16_t bank = 1; bank < romBanks; bank++) {				
				if (bank > 1) { currAddr = 0x4000; }
				
				// Set start address
				set_number(currAddr, SET_START_ADDRESS);
				
				// Read data
				while (currAddr < endAddr) {
					if (currAddr == 0x4000) { // Switch banks here just before the next bank, not any time sooner
						set_bank(0x2100, bank);
					}
					
					com_write_bytes_from_file(GB_FLASH_WRITE_64BYTE, romFile, 64);
					com_wait_for_ack();
					currAddr += 64;
					readBytes += 64;
					
					// Print progress
					print_progress_percent(readBytes, (romBanks * 16384) / 64);
				}
			}
			
			printf("]");
			fclose(romFile);
		}
		
		
		
		// ****** GBA Flash Carts ******
		else if (flashCartType == 10) {
			printf("16 MByte (MSP55LV128 / 29LV128DTMC) Gameboy Advance Flash Cart\n");
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			// Check file size
			if (fileSize > 0x1000000) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 16 MBytes\n", argv[1]);
				read_one_letter();
				return 1;
			}
			
			printf("\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Read rom a tiny bit before writing
			currAddr = 0x0000;	
			set_number(currAddr, SET_START_ADDRESS);
			set_mode(GBA_READ_ROM);
			com_read_bytes(READ_BUFFER, 64);
			com_read_stop();
			
			// Set end address as file size
			endAddr = fileSize;
			
			// Write ROM
			currAddr = 0x0000;	
			set_number(currAddr, SET_START_ADDRESS);
			while (currAddr < endAddr) {
				if (currAddr % 0x10000 == 0) { // Erase next sector
					gba_flash_write_address_byte(0xAAA, 0xA9);
					gba_flash_write_address_byte(0x555, 0x56);
					gba_flash_write_address_byte(0xAAA, 0x80);
					gba_flash_write_address_byte(0xAAA, 0xA9);
					gba_flash_write_address_byte(0x555, 0x56);
					gba_flash_write_address_byte((uint32_t) sector << 16, 0x30);
					sector++;
					
					// Wait for first 2 bytes to be 0xFF
					readBuffer[0] = 0;
					readBuffer[1] = 0;
					while (readBuffer[0] != 0xFF && readBuffer[1] != 0xFF) {
						set_number(currAddr / 2, SET_START_ADDRESS);
						set_mode(GBA_READ_ROM);
						
						com_read_bytes(READ_BUFFER, 64);
						com_read_stop(); // End read
						delay_ms(50);
					}
					
					set_number(currAddr / 2, SET_START_ADDRESS); // Divide address by 2
				}
				
				com_write_bytes_from_file(GBA_FLASH_WRITE_256BYTE, romFile, 256);
				com_wait_for_ack();
				currAddr += 256;
				readBytes += 256;
				
				print_progress_percent(readBytes, (endAddr) / 64);
			}
		}
		else if (flashCartType == 11) {
			printf("16 MByte (MSP55LV128M / 29GL128EHMC / 256M29EWH) Gameboy Advance Flash Cart\n");
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			// Check file size
			if (fileSize > 0x1000000) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 16 MBytes\n", argv[1]);
				read_one_letter();
				return 1;
			}
			
			printf("\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Read rom a tiny bit before writing
			currAddr = 0x0000;	
			set_number(currAddr, SET_START_ADDRESS);
			set_mode(GBA_READ_ROM);
			com_read_bytes(READ_BUFFER, 64);
			com_read_stop();
			
			// Set end address as file size
			endAddr = fileSize;
			
			// Write ROM
			currAddr = 0x0000;	
			set_number(currAddr, SET_START_ADDRESS);
			while (currAddr < endAddr) {
				if (currAddr % 0x20000 == 0) { // Erase next sector
					gba_flash_write_address_byte(0xAAA, 0xA9);
					gba_flash_write_address_byte(0x555, 0x56);
					gba_flash_write_address_byte(0xAAA, 0x80);
					gba_flash_write_address_byte(0xAAA, 0xA9);
					gba_flash_write_address_byte(0x555, 0x56);
					gba_flash_write_address_byte((uint32_t) sector << 17, 0x30);
					sector++;
					
					// Wait for first 2 bytes to be 0xFF
					readBuffer[0] = 0;
					readBuffer[1] = 0;
					while (readBuffer[0] != 0xFF && readBuffer[1] != 0xFF) {
						set_number(currAddr / 2, SET_START_ADDRESS);
						set_mode(GBA_READ_ROM);
						
						com_read_bytes(READ_BUFFER, 64);
						com_read_stop(); // End read
						delay_ms(50);
					}
					
					set_number(currAddr / 2, SET_START_ADDRESS); // Divide address by 2
				}
				
				com_write_bytes_from_file(GBA_FLASH_WRITE_256BYTE, romFile, 256);
				com_wait_for_ack();
				currAddr += 256;
				readBytes += 256;
				
				print_progress_percent(readBytes, (endAddr) / 64);
			}
		}
		else if (flashCartType == 12 || flashCartType == 13) {
			if (flashCartType == 12) {
				printf("16 MByte M36L0R706 / 32 MByte 256L30B Gameboy Advance Flash Cart\n");
			}
			else {
				printf("16 MByte M36L0R706 / 32 MByte 256L30B (2) Gameboy Advance Flash Cart\n");
			}
			
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			// Check file size
			if (fileSize > 0x2000000) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 32 MBytes\n", argv[1]);
				read_one_letter();
				return 1;
			}
			
			printf("\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			
			// Set to reading mode
			currAddr = 0x0000;
			gba_flash_write_address_byte(currAddr, 0xFF);
			delay_ms(5);
			
			// Read rom a tiny bit before writing
			currAddr = 0x0000;	
			set_number(currAddr, SET_START_ADDRESS);
			set_mode(GBA_READ_ROM);
			com_read_bytes(READ_BUFFER, 64);
			com_read_stop();
			
			// Set end address as file size
			endAddr = fileSize;
			
			
			// Flash ID command
			gba_flash_write_address_byte(0x00, 0x90);
			delay_ms(1);
			
			// Read ID
			set_number(0x00, SET_START_ADDRESS);
			set_mode(GBA_READ_ROM);
			com_read_bytes(READ_BUFFER, 64);
			com_read_stop(); // End read
			
			// For chips with 4x 16K sectors at the start
			// 256L30B (0x8A, 0x0, 0x15, 0x88)
			int sectorEraseAddress = 0x20000;
			if (readBuffer[0] == 0x8A && readBuffer[1] == 0 && readBuffer[2] == 0x15 && readBuffer[3] == 0x88) {
				sectorEraseAddress = 0x8000;
			}
			
			// Back to reading mode
			gba_flash_write_address_byte(currAddr, 0xFF);
			delay_ms(5);
			
			
			// Write ROM
			currAddr = 0x0000;	
			set_number(currAddr, SET_START_ADDRESS);
			uint32_t addressForManualWrite = 0x7FC0;
			
			while (currAddr < endAddr) {
				if (currAddr % sectorEraseAddress == 0) { // Erase next sector
					// For chips with 4x 16K sectors, after 64K, change to 0x20000 sector size
					if (currAddr >= 0x20000 && sectorEraseAddress == 0x8000) {
						sectorEraseAddress = 0x20000;
					}
					
					// Unlock
					gba_flash_write_address_byte(currAddr, 0x60);
					gba_flash_write_address_byte(currAddr, 0xD0);
					
					// Erase
					gba_flash_write_address_byte(currAddr, 0x20);
					gba_flash_write_address_byte(currAddr, 0xD0);
					delay_ms(50);
					
					// Wait for first 2 bytes to be 0x80, 0x00
					readBuffer[0] = 0;
					readBuffer[1] = 0;
					while (readBuffer[0] != 0x80 || readBuffer[1] != 0x00) {
						set_number(currAddr / 2, SET_START_ADDRESS);
						set_mode(GBA_READ_ROM);
						
						com_read_bytes(READ_BUFFER, 64);
						com_read_stop(); // End read
						delay_ms(500);
					}
					
					// Back to reading mode
					gba_flash_write_address_byte(currAddr, 0xFF);
					delay_ms(5);
					
					set_number(currAddr / 2, SET_START_ADDRESS); // Divide address by 2
				}
				
				// Standard buffered writing
				if (flashCartType == 12) {
					com_write_bytes_from_file(GBA_FLASH_WRITE_INTEL_64BYTE, romFile, 64);
					com_wait_for_ack();
					currAddr += 64;
					readBytes += 64;
				}
				else {
					// It seems that some carts don't allow us to use the buffered 32 byte programming mode for 0x7FC0 to 0x7FFF and 
					// it also happens at 0xFFC0 to 0xFFFF and so on. We just write each of the 32 bytes one byte at a time.
					if (currAddr > 0 && currAddr == addressForManualWrite) {
						uint8_t localbuffer[64];
						fread(&localbuffer, 1, 64, romFile);
						
						for (uint8_t x = 0; x < 64; x += 2) {
							uint16_t combinedBytes = (uint16_t) localbuffer[x+1] << 8 | (uint16_t) localbuffer[x];
							gba_flash_write_address_byte(currAddr, 0x40);
							gba_flash_write_address_byte(currAddr, combinedBytes);
							currAddr += 2;
							readBytes += 2;
						}
						addressForManualWrite += 0x8000;
						
						// Set address again (seems to be needed at and after 0x27C0)
						set_number(currAddr / 2, SET_START_ADDRESS); // Divide address by 2
					}
					else {
						com_write_bytes_from_file(GBA_FLASH_WRITE_INTEL_64BYTE, romFile, 64);
						com_wait_for_ack();
						currAddr += 64;
						readBytes += 64;
					}
				}
				print_progress_percent(readBytes, (endAddr) / 64);
			}
			
			// Back to reading mode
			gba_flash_write_address_byte(currAddr, 0xFF);
			delay_ms(5);
		}
		else {
			printf("No Flash Cart selected, please run this program by itself.");
			read_one_letter();
			return 1;
		}
	}	
	else {
		printf("\nPlease select a Flash Cart:\n"\
					 "--- Gameboy ---\n"\
					 "1. 32 KByte\n"\
					 "2. 512 KByte (SST39SF040)\n"\
					 "3. 2 MByte (BV5)\n"\
					 "4. 1 MByte (ES29LV160)\n"\
					 "5. 2 MByte (AM29LV160DB / 29LV160CTTC / 29LV160TE)\n"\
					 "6. 2 MByte (AM29F016B)\n"\
					 "7. 32 MByte (4x 8MB Banks) (256M29)\n"\
					 "8. 4 MByte (M29W640)\n\n"\
					 "--- Gameboy Advance ---\n"\
					 "10. 16 MByte (MSP55LV128 / 29LV128DTMC)\n"\
					 "11. 16 MByte (MSP55LV128M / 29GL128EHMC / 256M29EWH)\n"\
					 "12. 16 MByte M36L0R706 / 32 MByte 256L30B\n"\
					 "13. 16 MByte M36L0R706 / 32 MByte 256L30B (2)\n\n"\
					 "x. Exit\n>");
		
		char optionString[5];
		fgets(optionString, 5, stdin);
		
		int optionSelected = atoi(optionString);
		if (optionSelected == 1) {
			printf("\nPlease select a Flash Chip:\n"\
					 "1. AM29F010B (Audio as WE)\n"\
					 "2. AM29F010B (WR as WE)\n"\
					 "3. SST39SF010A / AT49F040 (Audio as WE)\n"\
					 "4. SST39SF010A / AT49F040 (WR as WE)\n"\
					 "x. Exit\n>");
			
			fgets(optionString, 5, stdin);
			optionSelected = atoi(optionString);
			write_flash_config(optionSelected+100);
			printf("\nYou can now drag and drop your ROM file to this exe file.");
			read_one_letter();
			return 0;
		}
		else {
			write_flash_config(optionSelected);
			printf("\nYou can now drag and drop your ROM file to this exe file.");
			read_one_letter();
			return 0;
		}
	}
	
	printf("\n");
	
	return 0;
}