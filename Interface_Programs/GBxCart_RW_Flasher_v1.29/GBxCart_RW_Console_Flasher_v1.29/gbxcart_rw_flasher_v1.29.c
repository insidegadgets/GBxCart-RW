/*
 GBxCart RW - Console Interface Flasher
 Version: 1.29
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 26/08/2017
 Last Modified: 22/05/2020
 License: GPL
 
 This program allows you to write ROMs to Flash Carts that are supported.
 
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
	
	printf("GBxCart RW Flasher v1.29 by insideGadgets\n");
	printf("#########################################\n");
	
	// Check arguments
	if (argc >= 2) {
		read_config();
		
		// Open COM port
		if (com_test_port() == 0) {
			printf("Device not connected and couldn't be auto detected. Please make sure the COM port isn't open elsewhere.\n");
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
		xmas_wake_up();
		
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
		if (argc == 2) {
			read_config_flash();
		}
		else if (argc == 3) {
			flashCartType = atoi(argv[2]);
		}
		
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
			
			// PCB v1.3 - Set 5V
			if (gbxcartPcbVersion == PCB_1_3 || gbxcartPcbVersion == GBXMAS) {
				set_mode(VOLTAGE_5V);
				delay_ms(500);
			}
			
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
				gb_check_change_flash_id(GB_FLASH_PROGRAM_555);
			}
			else {
				gb_flash_program_setup(GB_FLASH_PROGRAM_5555);// Flash program byte method
				gb_check_change_flash_id(GB_FLASH_PROGRAM_5555);
			}
			
			// Chip erase for this flash chip
			if (flashCartType == 103 || flashCartType == 104) {
				printf("\nErasing Flash");
				xmas_chip_erase_animation();
				gb_flash_write_address_byte(0x5555, 0xAA);
				gb_flash_write_address_byte(0x2AAA, 0x55);
				gb_flash_write_address_byte(0x5555, 0x80);
				gb_flash_write_address_byte(0x5555, 0xAA);
				gb_flash_write_address_byte(0x2AAA, 0x55);
				gb_flash_write_address_byte(0x5555, 0x10);
				
				// Wait for first byte to be 0xFF
				wait_for_flash_chip_erase_ff(1);
			}
			
			xmas_setup((endAddr+1) / 28);
			
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
						delay_ms(5);
					}
				}
				
				com_write_bytes_from_file(GB_FLASH_WRITE_64BYTE, romFile, 64);
				com_wait_for_ack();
				currAddr += 64;
				readBytes += 64;
				
				print_progress_percent(readBytes, (endAddr+1) / 64);
				led_progress_percent(readBytes, (endAddr+1) / 28);
			}
			
			printf("]");
			fclose(romFile);
		}
		
		else if (flashCartType == 1) {
			printf(" 32 KByte Gameboy Flash Cart\n");
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			currAddr = 0x0000;
			endAddr = 0x7FFF;
			
			// PCB v1.3 - Set 5V
			if (gbxcartPcbVersion == PCB_1_3 || gbxcartPcbVersion == GBXMAS) {
				set_mode(VOLTAGE_5V);
				delay_ms(500);
			}
			
			// Check file size
			if (fileSize > (endAddr+1)) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 32K\n", argv[1]);
				read_one_letter();
				return 1;
			}
			
			// Flash Setup
			set_mode(GB_CART_MODE); // Gameboy mode
			gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_5555);// Flash program byte method
			gb_check_change_flash_id(GB_FLASH_PROGRAM_5555);
			
			// Check if Flash ID matches
			if (flashID[0] != 0xBF || flashID[1] != 0xB5 || flashID[2] != 0x01 || flashID[3] != 0xFF) {
				printf("*** Flash ID doesn't match expected value of 0xBF, 0xB5, 0x01, 0xFF ***\n");
				read_one_letter();
			}
			
			// Chip erase for this flash chip
			printf("\nErasing Flash");
			xmas_chip_erase_animation();
			gb_flash_write_address_byte(0x5555, 0xAA);
			gb_flash_write_address_byte(0x2AAA, 0x55);
			gb_flash_write_address_byte(0x5555, 0x80);
			gb_flash_write_address_byte(0x5555, 0xAA);
			gb_flash_write_address_byte(0x2AAA, 0x55);
			gb_flash_write_address_byte(0x5555, 0x10);
			
			// Wait for first byte to be 0xFF
			wait_for_flash_chip_erase_ff(1);
			xmas_setup((endAddr+1) / 28);
			
			printf("\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Write ROM
			set_number(currAddr, SET_START_ADDRESS);
			while (currAddr <= endAddr) {
				com_write_bytes_from_file(GB_FLASH_WRITE_64BYTE, romFile, 64);
				com_wait_for_ack();
				currAddr += 64;
				readBytes += 64;
				
				print_progress_percent(readBytes, (endAddr+1) / 64);
				led_progress_percent(readBytes, (endAddr+1) / 28);
			}
			
			printf("]");
			fclose(romFile);
		}
		
		else if (flashCartType == 8 || flashCartType == 29) {
			if (flashCartType == 8) {
				printf("512 KByte (SST39SF040) Gameboy Flash Cart\n");
			}
			else {
				printf("insideGadgets 512 KByte Gameboy Flash Cart\n");
			}
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			// PCB v1.3 - Set 5V
			if (gbxcartPcbVersion == PCB_1_3 || gbxcartPcbVersion == GBXMAS) {
				set_mode(VOLTAGE_5V);
				delay_ms(500);
			}
			
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
			xmas_chip_erase_animation();
			gb_flash_write_address_byte(0x5555, 0xAA);
			gb_flash_write_address_byte(0x2AAA, 0x55);
			gb_flash_write_address_byte(0x5555, 0x80);
			gb_flash_write_address_byte(0x5555, 0xAA);
			gb_flash_write_address_byte(0x2AAA, 0x55);
			gb_flash_write_address_byte(0x5555, 0x10);
			
			// Wait for first byte to be 0xFF
			wait_for_flash_chip_erase_ff(1);
			xmas_setup((romBanks * 16384) / 28);
			
			printf("\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Write ROM
			currAddr = 0x0000;
			for (uint16_t bank = 1; bank < romBanks; bank++) {				
				if (bank > 1) { currAddr = 0x4000; }
				
				// Set start address
				set_number(currAddr, SET_START_ADDRESS);
				delay_ms(5);
				
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
					led_progress_percent(readBytes, (romBanks * 16384) / 28);
				}
			}
			
			printf("]");
			fclose(romFile);
		}
		
		else if (flashCartType == 32 || flashCartType == 33) {
			if (flashCartType == 32) {
				printf("512 KByte (AM29LV160 with CPLD) Gameboy Flash Cart\n");
			}
			else {
				printf("1 MByte (29LV320 with CPLD) Gameboy Flash Cart\n");
			}
			
			// PCB v1.1/1.2
			if (gbxcartPcbVersion == PCB_1_1 && cartridgeMode == GB_MODE) {
				printf("You must switch GBxCart RW to be powered by 3.3V.\n");
				printf("Please unplug it, switch the voltage and re-connect.\n");
				read_one_letter();
				return 1;
			}
			else if (gbxcartPcbVersion == PCB_1_3 || gbxcartPcbVersion == GBXMAS) { // PCB v1.3, Set 3.3V
				set_mode(VOLTAGE_3_3V);
				delay_ms(500);
			}
			
			// Check file size
			if (flashCartType == 32 && fileSize > 0x80000) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 512 KByte\n", argv[1]);
				read_one_letter();
				return 1;
			}
			else if (flashCartType == 33 && fileSize > 0x100000) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 1 MByte\n", argv[1]);
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
			gb_flash_program_setup(GB_FLASH_PROGRAM_7AAA_BIT01_SWAPPED);// Flash program byte method
			gb_check_change_flash_id(GB_FLASH_PROGRAM_7AAA_BIT01_SWAPPED);
			
			// Chip erase (data byte's bit 0 & 1 are swapped for chip commands as D0 & D1 lines are swapped)
			printf("\nErasing Flash");
			xmas_chip_erase_animation();
			gb_flash_write_address_byte(0x7AAA, 0xA9);
			gb_flash_write_address_byte(0x7555, 0x56);
			gb_flash_write_address_byte(0x7AAA, 0x80);
			gb_flash_write_address_byte(0x7AAA, 0xA9);
			gb_flash_write_address_byte(0x7555, 0x56);
			gb_flash_write_address_byte(0x7AAA, 0x10);
			
			// Wait for first byte to be 0xFF
			wait_for_flash_chip_erase_ff(1);
			xmas_setup((romBanks * 16384) / 28);			
			
			printf("\n\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Write ROM
			for (uint16_t bank = 0; bank < romBanks; bank++) {				
				currAddr = 0x4000;
				
				// Set start address
				set_number(currAddr, SET_START_ADDRESS);
				delay_ms(5);
				
				if (bank >= 1) {
					set_mode(GB_FLASH_BANK_1_COMMAND_WRITES); // Set bank again as we reset the CPLD after every write
					delay_ms(5);
					set_bank(0x2100, bank);
				}
				
				// Read data
				while (currAddr < endAddr) {
					com_write_bytes_from_file(GB_FLASH_WRITE_64BYTE_PULSE_RESET, romFile, 64);
					com_wait_for_ack();
					currAddr += 64;
					readBytes += 64;
					
					// Print progress
					print_progress_percent(readBytes, (romBanks * 16384) / 64);
					led_progress_percent(readBytes, (romBanks * 16384) / 28);
				}
			}
			
			printf("]");
			fclose(romFile);
		}
		
		else if (flashCartType == 9) {
			printf("1 MByte (ES29LV160) Gameboy Flash Cart\n");
			
			// PCB v1.1/1.2
			if (gbxcartPcbVersion == PCB_1_1 && cartridgeMode == GB_MODE) {
				printf("You must switch GBxCart RW to be powered by 3.3V.\n");
				printf("Please unplug it, switch the voltage and re-connect.\n");
				read_one_letter();
				return 1;
			}
			else if (gbxcartPcbVersion == PCB_1_3 || gbxcartPcbVersion == GBXMAS) { // PCB v1.3, set 3.3V
				set_mode(VOLTAGE_3_3V);
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
			gb_check_change_flash_id(GB_FLASH_PROGRAM_555_BIT01_SWAPPED);
			
			// Chip erase (data byte's bit 0 & 1 are swapped for chip commands as BV5 D0 & D1 lines are swapped)
			printf("\nErasing Flash");
			xmas_chip_erase_animation();
			gb_flash_write_address_byte(0x555, 0xA9);
			gb_flash_write_address_byte(0x2AA, 0x56);
			gb_flash_write_address_byte(0x555, 0x80);
			gb_flash_write_address_byte(0x555, 0xA9);
			gb_flash_write_address_byte(0x2AA, 0x56);
			gb_flash_write_address_byte(0x555, 0x10);
			
			// Wait for first byte to be 0xFF
			wait_for_flash_chip_erase_ff(1);
			xmas_setup((romBanks * 16384) / 28);
			
			printf("\n\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Write ROM
			currAddr = 0x0000;
			for (uint16_t bank = 1; bank < romBanks; bank++) {				
				if (bank > 1) { currAddr = 0x4000; }
				
				// Set start address
				set_number(currAddr, SET_START_ADDRESS);
				delay_ms(5);
				
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
					led_progress_percent(readBytes, (romBanks * 16384) / 28);
				}
			}
			
			printf("]");
			fclose(romFile);
		}
		
		else if (flashCartType == 10) {
			printf("2 MByte (BV5) Gameboy Flash Cart\n");
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			// PCB v1.3 - Set 5V
			if (gbxcartPcbVersion == PCB_1_3 || gbxcartPcbVersion == GBXMAS) {
				set_mode(VOLTAGE_5V);
				delay_ms(500);
			}
			
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
			gb_check_change_flash_id(GB_FLASH_PROGRAM_AAA_BIT01_SWAPPED);
			
			// Chip erase (data byte's bit 0 & 1 are swapped for chip commands as BV5 D0 & D1 lines are swapped)
			printf("\nErasing Flash");
			xmas_chip_erase_animation();
			gb_flash_write_address_byte(0xAAA, 0xA9);
			gb_flash_write_address_byte(0x555, 0x56);
			gb_flash_write_address_byte(0xAAA, 0x80);
			gb_flash_write_address_byte(0xAAA, 0xA9);
			gb_flash_write_address_byte(0x555, 0x56);
			gb_flash_write_address_byte(0xAAA, 0x10);
			
			// Wait for first byte to be 0xFF
			wait_for_flash_chip_erase_ff(1);
			xmas_setup((romBanks * 16384) / 28);
			
			printf("\n\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Write ROM
			currAddr = 0x0000;
			for (uint16_t bank = 1; bank < romBanks; bank++) {				
				if (bank > 1) { currAddr = 0x4000; }
				
				// Set start address
				set_number(currAddr, SET_START_ADDRESS);
				delay_ms(5);
				
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
					led_progress_percent(readBytes, (romBanks * 16384) / 28);
				}
			}
			
			printf("]");
			fclose(romFile);
		}
		
		else if (flashCartType == 11) {
			printf("2 MByte (AM29LV160DB / 29LV160CTTC / 29LV160TE / S29AL016) Gameboy Flash Cart\n");
			
			// PCB v1.1/1.2
			if (gbxcartPcbVersion == PCB_1_1 && cartridgeMode == GB_MODE) {
				printf("You must switch GBxCart RW to be powered by 3.3V.\n");
				printf("Please unplug it, switch the voltage and re-connect.\n");
				read_one_letter();
				return 1;
			}
			else if (gbxcartPcbVersion == PCB_1_3 || gbxcartPcbVersion == GBXMAS) { // PCB v1.3, Set 3.3V
				set_mode(VOLTAGE_3_3V);
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
			gb_check_change_flash_id(GB_FLASH_PROGRAM_AAA);
			
			// Chip erase
			printf("\nErasing Flash");
			xmas_chip_erase_animation();
			gb_flash_write_address_byte(0xAAA, 0xAA);
			gb_flash_write_address_byte(0x555, 0x55);
			gb_flash_write_address_byte(0xAAA, 0x80);
			gb_flash_write_address_byte(0xAAA, 0xAA);
			gb_flash_write_address_byte(0x555, 0x55);
			gb_flash_write_address_byte(0xAAA, 0x10);
			
			// Wait for first byte to be 0xFF
			wait_for_flash_chip_erase_ff(1);
			xmas_setup((romBanks * 16384) / 28);
			
			printf("\n\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Write ROM
			currAddr = 0x0000;
			for (uint16_t bank = 1; bank < romBanks; bank++) {				
				if (bank > 1) { currAddr = 0x4000; }
				
				// Set start address
				set_number(currAddr, SET_START_ADDRESS);
				delay_ms(5);
				
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
					led_progress_percent(readBytes, (romBanks * 16384) / 28);
				}
			}
			
			printf("]");
			fclose(romFile);
		}
		
		else if (flashCartType == 12 || flashCartType == 2 || flashCartType == 3 || flashCartType == 34) {
			if (flashCartType == 12) {
				printf("2 MByte (AM29F016B) (WR as WE) Gameboy Flash Cart\n");
			}
			else if (flashCartType == 34) {
				printf("2 MByte (AM29F016B) (Audio as WE) Gameboy Flash Cart\n");
			}
			else if (flashCartType == 2) {
				printf("insideGadgets 2 MByte 128KB SRAM Flash Cart\n");
			}
			else if (flashCartType == 3) {
				printf("insideGadgets 2 MByte 32KB FRAM Flash Cart\n");
			}
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			// PCB v1.3 - Set 5V
			if (gbxcartPcbVersion == PCB_1_3 || gbxcartPcbVersion == GBXMAS) {
				set_mode(VOLTAGE_5V);
				delay_ms(500);
			}
			
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
			
			if (flashCartType == 34) {
				gb_flash_pin_setup(WE_AS_AUDIO_PIN); // Audio pin
			}
			else {
				gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			}
			
			gb_flash_program_setup(GB_FLASH_PROGRAM_555);// Flash program byte method
			gb_check_change_flash_id(GB_FLASH_PROGRAM_555);
			
			// Check if Flash ID matches
			if (flashCartType == 2 || flashCartType == 3) {
				if (flashID[0] != 0x01 || flashID[1] != 0xAD || flashID[2] != 0x00 || flashID[3] != 0x00) {
					printf("*** Flash ID doesn't match expected value of 0x01, 0xAD, 0x00, 0x00 ***\n");
					read_one_letter();
				}
			}
			
			// Chip erase
			printf("\nErasing Flash");
			xmas_chip_erase_animation();
			gb_flash_write_address_byte(0x555, 0xAA);
			gb_flash_write_address_byte(0x2AA, 0x55);
			gb_flash_write_address_byte(0x555, 0x80);
			gb_flash_write_address_byte(0x555, 0xAA);
			gb_flash_write_address_byte(0x2AA, 0x55);
			gb_flash_write_address_byte(0x555, 0x10);
			
			// Wait for first byte to be 0xFF
			wait_for_flash_chip_erase_ff(1);
			xmas_setup((romBanks * 16384) / 28);
			
			printf("\n\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Write ROM
			currAddr = 0x0000;
			for (uint16_t bank = 1; bank < romBanks; bank++) {				
				if (bank > 1) { currAddr = 0x4000; }
				
				// Set start address
				set_number(currAddr, SET_START_ADDRESS);
				delay_ms(5);
				
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
					led_progress_percent(readBytes, (romBanks * 16384) / 28);
				}
			}
			
			printf("]");
			fclose(romFile);
		}
		
		else if (flashCartType == 13) {
			printf("2 MByte (GB Smart 16M) Gameboy Flash Cart\n");
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			// PCB v1.3 - Set 5V
			if (gbxcartPcbVersion == PCB_1_3 || gbxcartPcbVersion == GBXMAS) {
				set_mode(VOLTAGE_5V);
				delay_ms(500);
			}
			
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
			set_mode(GB_FLASH_BANK_1_COMMAND_WRITES); // Set bank 1 before issuing flash commands
			gb_flash_pin_setup(WE_AS_AUDIO_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_5555); // Flash program byte method
			gb_check_change_flash_id(GB_FLASH_PROGRAM_5555);
			xmas_setup((romBanks * 16384) / 28);
			
			printf("\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			
			uint8_t currentBankSize = romBanks;
			uint8_t romBanksRemaining = romBanks;
			uint8_t chipNo = 1;
			
			// Calculate ROM banks remaining if more than 0x20 (one flash chip)
			if (romBanks > 0x20) {
				currentBankSize = 0x20;
			}
			
			while (romBanksRemaining >= 1) {
				// Set bank 1 before issuing flash commands
				set_bank(0x2100, 1);
				delay_ms(5);
				
				// Set address 0
				currAddr = 0x00;
				set_number(currAddr, SET_START_ADDRESS);
				delay_ms(5);
				
				// Switch to the next flash chip
				if (chipNo >= 2) {
					set_bank(0x2000, 0x20 * (chipNo-1));
					delay_ms(5);
					set_bank(0x1000, 0xA5);
					delay_ms(5);
					set_bank(0x7000, 0x00);
					delay_ms(5);
					set_bank(0x1000, 0x98);
					delay_ms(5);
					
					set_bank(0x2000, 0x20 * (chipNo-1));
					delay_ms(5);
					set_bank(0x1000, 0xA5);
					delay_ms(5);
					set_bank(0x7000, 0x23);
					delay_ms(5);
					set_bank(0x1000, 0x98);
					delay_ms(5);
				}
				
				// Erase flash chip
				gb_flash_write_address_byte(0x5555, 0xAA);
				gb_flash_write_address_byte(0x2AAA, 0x55);
				gb_flash_write_address_byte(0x5555, 0x80);
				gb_flash_write_address_byte(0x5555, 0xAA);
				gb_flash_write_address_byte(0x2AAA, 0x55);
				gb_flash_write_address_byte(0x5555, 0x10);
				delay_ms(5);
				
				// Wait for first byte to be 0xFF
				wait_for_flash_chip_erase_ff(0);
				
				
				// Write ROM
				currAddr = 0x0000;
				for (uint16_t bank = 1; bank < currentBankSize; bank++) {				
					if (bank > 1) { currAddr = 0x4000; }
					
					// Set start address
					set_number(currAddr, SET_START_ADDRESS);
					delay_ms(5);
					
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
						led_progress_percent(readBytes, (romBanks * 16384) / 28);
					}
				}
				
				// Increment the flash chip and calculate remaining banks
				chipNo++;
				romBanksRemaining -= currentBankSize;
				if (romBanksRemaining > 0x20) {
					currentBankSize = 0x20;
				}
				else {
					currentBankSize = romBanksRemaining; // Last banks to write
				}
			}
			printf("]");
			fclose(romFile);
		}
		
		else if (flashCartType == 14) {
			printf("4 MByte (M29W640 / 29DL32BF / GL032A10BAIR4 / S29AL016M9) Gameboy Flash Cart\n");
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			// PCB v1.1/1.2
			if (gbxcartPcbVersion == PCB_1_1 && cartridgeMode == GB_MODE) {
				printf("You must switch GBxCart RW to be powered by 3.3V.\n");
				printf("Please unplug it, switch the voltage and re-connect.\n");
				read_one_letter();
				return 1;
			}
			else if (gbxcartPcbVersion == PCB_1_3 || gbxcartPcbVersion == GBXMAS) { // PCB v1.3, Set 3.3V
				set_mode(VOLTAGE_3_3V);
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
			gb_check_change_flash_id(GB_FLASH_PROGRAM_AAA_BIT01_SWAPPED);
			
			// Chip erase (data byte's bit 0 & 1 are swapped for chip commands as BV5 D0 & D1 lines are swapped)
			printf("\nErasing Flash");
			xmas_chip_erase_animation();
			gb_flash_write_address_byte(0xAAA, 0xA9);
			gb_flash_write_address_byte(0x555, 0x56);
			gb_flash_write_address_byte(0xAAA, 0x80);
			gb_flash_write_address_byte(0xAAA, 0xA9);
			gb_flash_write_address_byte(0x555, 0x56);
			gb_flash_write_address_byte(0xAAA, 0x10);
			
			// Wait for first byte to be 0xFF
			wait_for_flash_chip_erase_ff(1);
			xmas_setup((romBanks * 16384) / 28);
			
			printf("\n\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Write ROM
			currAddr = 0x0000;
			for (uint16_t bank = 1; bank < romBanks; bank++) {				
				if (bank > 1) { currAddr = 0x4000; }
				
				// Set start address
				set_number(currAddr, SET_START_ADDRESS);
				delay_ms(5);
				
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
					led_progress_percent(readBytes, (romBanks * 16384) / 28);
				}
			}
			
			printf("]");
			fclose(romFile);
		}
		
		else if (flashCartType == 15 || flashCartType == 35) {
			if (flashCartType == 15) {
				printf("4 MByte MBC30 (AM29F032B / MBM29F033C) Gameboy Flash Cart\n");
			}
			else if (flashCartType == 35) {
				printf("insideGadgets 4 MByte 32KB FRAM MBC3 RTC Flash Cart\n");
			}
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			// PCB v1.3 - Set 5V
			if (gbxcartPcbVersion == PCB_1_3 || gbxcartPcbVersion == GBXMAS) {
				set_mode(VOLTAGE_5V);
				delay_ms(500);
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
			gb_flash_pin_setup(WE_AS_AUDIO_PIN); // Audio pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_555);// Flash program byte method
			gb_check_change_flash_id(GB_FLASH_PROGRAM_555);
			
			// Chip erase
			printf("\nErasing Flash");
			xmas_chip_erase_animation();
			gb_flash_write_address_byte(0x555, 0xAA);
			gb_flash_write_address_byte(0x2AA, 0x55);
			gb_flash_write_address_byte(0x555, 0x80);
			gb_flash_write_address_byte(0x555, 0xAA);
			gb_flash_write_address_byte(0x2AA, 0x55);
			gb_flash_write_address_byte(0xAAA, 0x10);
			
			// Wait for first byte to be 0xFF
			wait_for_flash_chip_erase_ff(1);
			xmas_setup((romBanks * 16384) / 28);
			
			printf("\n\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Write ROM
			currAddr = 0x0000;
			for (uint16_t bank = 1; bank < romBanks; bank++) {				
				if (bank > 1) { currAddr = 0x4000; }
				
				// Set start address
				set_number(currAddr, SET_START_ADDRESS);
				delay_ms(5);
				
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
					led_progress_percent(readBytes, (romBanks * 16384) / 28);
				}
			}
			
			printf("]");
			fclose(romFile);
		}
		
		else if (flashCartType == 16 || flashCartType == 17) {
			if (flashCartType == 16) {
				printf("32 MByte (4x 8MB Banks) (256M29) Gameboy Flash Cart\n");
			}
			else {
				printf("32 MByte (4x 8MB Banks) (M29W256 / MX29GL256) Gameboy Flash Cart\n");
			}
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			printf("\n*** After writing is complete, you will need to power cycle the device in order to write to the other 8MB banks ***\n\n");
			
			// PCB v1.1/1.2
			if (gbxcartPcbVersion == PCB_1_1 && cartridgeMode == GB_MODE) {
				printf("You must switch GBxCart RW to be powered by 3.3V.\n");
				printf("Please unplug it, switch the voltage and re-connect.\n");
				read_one_letter();
				return 1;
			}
			else if (gbxcartPcbVersion == PCB_1_3 || gbxcartPcbVersion == GBXMAS) { // PCB v1.3, Set 3.3V
				set_mode(VOLTAGE_3_3V);
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
			gb_check_change_flash_id(GB_FLASH_PROGRAM_AAA_BIT01_SWAPPED);
			
			printf("Please enter which 8MB bank number we should write to (1-4):");
			char bankString[5];
			fgets(bankString, 5, stdin);
			int bankNumber = atoi(bankString);
			
			if (bankNumber == 1) {
				// Chip erase
				printf("\nErasing Flash (may take 1 minute)");
				xmas_chip_erase_animation();
				gb_flash_write_address_byte(0xAAA, 0xA9);
				gb_flash_write_address_byte(0x555, 0x56);
				gb_flash_write_address_byte(0xAAA, 0x80);
				gb_flash_write_address_byte(0xAAA, 0xA9);
				gb_flash_write_address_byte(0x555, 0x56);
				gb_flash_write_address_byte(0xAAA, 0x10);
				
				// Wait for first byte to be 0xFF
				wait_for_flash_chip_erase_ff(1);
				
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
			xmas_setup((romBanks * 16384) / 28);
			
			printf("\n\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Write ROM
			currAddr = 0x0000;
			for (uint16_t bank = 1; bank < romBanks; bank++) {				
				if (bank > 1) { currAddr = 0x4000; }
				
				// Set start address
				set_number(currAddr, SET_START_ADDRESS);
				delay_ms(5);
				
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
					
					if (flashCartType == 15) { // Use buffered programming
						com_write_bytes_from_file(GB_FLASH_WRITE_256BYTE, romFile, 256);
						com_wait_for_ack();
						currAddr += 256;
						readBytes += 256;
					}
					else { // M29W256 doesn't seem to work with buffered programming, write one byte at a time
						com_write_bytes_from_file(GB_FLASH_WRITE_64BYTE, romFile, 64);
						com_wait_for_ack();
						currAddr += 64;
						readBytes += 64;
					}
					
					// Print progress
					print_progress_percent(readBytes, (romBanks * 16384) / 64);
					led_progress_percent(readBytes, (romBanks * 16384) / 28);
				}
			}
			
			printf("]");
			fclose(romFile);
		}
		
		else if (flashCartType == 5 || flashCartType == 6) {
			if (flashCartType == 5) {
				printf("64 MByte Mighty Flash Cart\n");
			}
			else {
				printf("64 MByte Mighty Flash Cart - Buffered (Experimental)\n");
			}
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			// PCB v1.3 - Set 5V
			if (gbxcartPcbVersion == PCB_1_3 || gbxcartPcbVersion == GBXMAS) {
				set_mode(VOLTAGE_5V);
				delay_ms(500);
			}
			
			// Check file size
			if (fileSize > 0x4000000) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 64 MByte\n", argv[1]);
				read_one_letter();
				return 1;
			}
			
			currAddr = 0x0000;
			endAddr = 0x7FFF;
			
			// Calculate banks/8MB blocks needed from ROM file size
			romBanks = fileSize / 16384;
			uint8_t romBlocks = (romBanks / 512) + 1;
			uint16_t romBanksCalc = fileSize / 16384;
			uint16_t romBanksTotal = romBanksCalc;
			if (romBlocks > 1) {
				romBanks = 512;
			}
			if (romBlocks == 0) {
				romBlocks = 1;
			}
			
			// Detect if save slots are being used by 
			uint8_t saveSlotsDetected = 0;
			uint32_t zeroCounter = 0;
			
			if (fileSize > 0x120000) { // File needs to be more than this for save slots to be active
				fseek(romFile, 0x20000, SEEK_SET); // First save slot
				
				for (uint16_t counter = 0; counter < 512; counter++) {
					uint8_t buffer[256];
					fread(&buffer, 1, 256, romFile);
					
					for (uint16_t z = 0; z < 256; z++) {
						if (buffer[z] == 0) {
							zeroCounter++;
						}
					}
				}
				
				if (zeroCounter == 0x20000) { // 131072 bytes of all 0's
					saveSlotsDetected = 1;
					printf("Save slots detected in ROM file\n");
				}
				fseek(romFile, 0x00, SEEK_SET);
			}
			
			// Flash Setup
			set_mode(GB_CART_MODE); // Gameboy mode
			gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_AAA);// Flash program byte method
			gb_check_change_flash_id(GB_FLASH_PROGRAM_AAA);
			
			printf("\n\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			
			// Set bank as 0
			set_bank(0x2100, 0);
			set_bank(0x3000, 0);
			
			// Change to first 8MB block
			set_bank(0x1000, 0xD9); // Unlock multi-game mode
			set_bank(0x7000, 0xF0); // Set MBC5 mode and high 3 bits of bank value
			set_bank(0x6000, 0x00); // Bank
			set_bank(0x0000, 0xAA); // Turn off multi-game mode
			
			xmas_setup((romBanks * 16384) / 28);
			
			// Write ROM
			currAddr = 0x0000;
			for (uint8_t block = 0; block < romBlocks; block++) {			
				currAddr = 0x0000;
				sector = 0;
				
				for (uint16_t bank = 1; bank < romBanks; bank++) {				
					if (bank > 1) { currAddr = 0x4000; }
					
					// Set start address
					set_number(currAddr, SET_START_ADDRESS);
					delay_ms(5);
					
					// Read data
					while (currAddr < endAddr) {
						if (currAddr == 0x4000) { // Switch banks here just before the next bank
							set_bank(0x2100, bank);
							if (bank >= 256) {
								set_bank(0x3000, 1); // High bit
							}
						}
						
						// If save slots are present - Write the loader, skip writing to save slots and then write data after save slots
						if (saveSlotsDetected == 0 || (saveSlotsDetected == 1 && (readBytes < 0x20000 || readBytes >= 0x120000))) {
							if (readBytes % 0x20000 == 0) { // Erase sectors
								gb_flash_write_address_byte(0xAAA, 0xAA);
								gb_flash_write_address_byte(0x555, 0x55);
								gb_flash_write_address_byte(0xAAA, 0x80);
								gb_flash_write_address_byte(0xAAA, 0xAA);
								gb_flash_write_address_byte(0x555, 0x55);
								gb_flash_write_address_byte(0x4000, 0x30);
								
								wait_for_flash_sector_ff(currAddr);
								sector++;
								
								set_number(currAddr, SET_START_ADDRESS);
								delay_ms(5);
							}
							
							// Regular writing
							if (flashCartType == 9) {
								com_write_bytes_from_file(GB_FLASH_WRITE_64BYTE, romFile, 64);
								com_wait_for_ack();
								currAddr += 64;
								readBytes += 64;
							}
							else {
								//printf("addr 0x%X\n", currAddr);
								com_write_bytes_from_file(GB_FLASH_WRITE_BUFFERED_32BYTE, romFile, 32);
								com_wait_for_ack();
								currAddr += 32;
								readBytes += 32;
							}
						}
						else { // Skip ROM
							fread(writeBuffer, 1, 64, romFile);
							currAddr += 64;
							readBytes += 64;
						}
						
						// Print progress
						print_progress_percent(readBytes, (romBanksTotal * 16384) / 64);
						led_progress_percent(readBytes, (romBanksTotal * 16384) / 28);
					}
				}
				
				// Calculate remaining rom banks
				if (romBanksCalc > 512) {
					romBanksCalc = romBanksCalc - 512;
					if (romBanksCalc <= 512) {
						romBanks = romBanksCalc;
					}
				}
				
				
				// Set bank as 0
				set_bank(0x2100, 0);
				set_bank(0x3000, 0);
				
				// Change to next 8MB block
				uint8_t nextBlock = 0xF2 + (block * 2);
				set_bank(0x1000, 0xD9); // Unlock multi-game mode
				set_bank(0x7000, nextBlock); // Set MBC5 mode and high 3 bits of bank value
				set_bank(0x6000, 0x00); // Bank
				set_bank(0x0000, 0xAA); // Turn off multi-game mode
			}
			
			
			// Set bank as 0
			set_bank(0x2100, 0);
			set_bank(0x3000, 0);
			
			// Change to first 8MB block
			set_bank(0x1000, 0xD9); // Unlock multi-game mode
			set_bank(0x7000, 0xF0); // Set MBC5 mode and high 3 bits of bank value
			set_bank(0x6000, 0x00); // Bank
			set_bank(0x0000, 0xAA); // Turn off multi-game mode
			
			printf("]");
			fclose(romFile);
		}
		
		else if (flashCartType == 4) {
			printf("insideGadgets 4 MByte 128KB SRAM/FRAM Gameboy Flash Cart\n");
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			// PCB v1.3 - Set 5V
			if (gbxcartPcbVersion == PCB_1_3 || gbxcartPcbVersion == GBXMAS) {
				set_mode(VOLTAGE_5V);
				delay_ms(500);
			}
			currAddr = 0x0000;
			endAddr = 0x7FFF;
			
			// Check file size
			if (fileSize > 0x400000) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 4 MByte\n", argv[1]);
				read_one_letter();
				return 1;
			}
			
			// Calculate banks needed from ROM file size
			romBanks = fileSize / 16384;
			
			// Flash Setup
			set_mode(GB_CART_MODE); // Gameboy mode
			delay_ms(5);
			gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_AAA);// Flash program byte method
			gb_check_change_flash_id(GB_FLASH_PROGRAM_AAA);
			
			// Check if Flash ID matches
			if (flashID[0] == 0x01 && flashID[1] == 0x01 && flashID[2] == 0x7E && flashID[3] == 0x7E) {}
			else if (flashID[0] == 0xC2) {}
			else {
				printf("*** Flash ID doesn't match expected value of 0x01, 0x01, 0x7E, 0x7E or 0xC2 ***\n");
				read_one_letter();
			}
			
			set_number(0, SET_START_ADDRESS);
			delay_ms(5);
			
			// Chip erase
			printf("\nErasing Flash ");
			xmas_chip_erase_animation();
			gb_flash_write_address_byte(0xAAA, 0xAA);
			gb_flash_write_address_byte(0x555, 0x55);
			gb_flash_write_address_byte(0xAAA, 0x80);
			gb_flash_write_address_byte(0xAAA, 0xAA);
			gb_flash_write_address_byte(0x555, 0x55);
			gb_flash_write_address_byte(0xAAA, 0x10);
			
			// Wait for first byte to be 0xFF
			wait_for_flash_chip_erase_ff(1);
			xmas_setup((romBanks * 16384) / 28);
			
			printf("\n\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Write ROM
			currAddr = 0x0000;
			for (uint16_t bank = 1; bank < romBanks; bank++) {				
				if (bank > 1) { currAddr = 0x4000; }
				
				// Set start address
				set_number(currAddr, SET_START_ADDRESS);
				delay_ms(5);
				
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
					led_progress_percent(readBytes, (romBanks * 16384) / 28);
				}
			}
			
			printf("]");
			fclose(romFile);
		}
		
		else if (flashCartType == 30 || flashCartType == 31) {
			if (flashCartType == 30) {
				printf("insideGadgets 1 MByte 128KB SRAM Gameboy Flash Cart\n");
			}
			else {
				printf("insideGadgets 1 MByte 128KB SRAM Custom Logo Flash Cart\n");
			}
			
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			// PCB v1.3 - Set 5V
			if (gbxcartPcbVersion == PCB_1_3 || gbxcartPcbVersion == GBXMAS) {
				set_mode(VOLTAGE_5V);
				delay_ms(500);
			}
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
			
			// Disable Nintendo logo detection for logo cart
			if (flashCartType == 31) {
				set_bank(0x31, 0x2D); // For original logo cart
				delay_ms(5);
				
				set_bank(0x2100, 1); // For ultra low power logo cart
				delay_ms(5);
			}
			
			gb_flash_program_setup(GB_FLASH_PROGRAM_AAA);// Flash program byte method
			gb_check_change_flash_id(GB_FLASH_PROGRAM_AAA);
			
			// Chip erase
			printf("\nErasing Flash");
			xmas_chip_erase_animation();
			gb_flash_write_address_byte(0xAAA, 0xAA);
			gb_flash_write_address_byte(0x555, 0x55);
			gb_flash_write_address_byte(0xAAA, 0x80);
			gb_flash_write_address_byte(0xAAA, 0xAA);
			gb_flash_write_address_byte(0x555, 0x55);
			gb_flash_write_address_byte(0xAAA, 0x10);
			
			// Wait for first byte to be 0xFF
			wait_for_flash_chip_erase_ff(1);
			xmas_setup((romBanks * 16384) / 28);
			
			printf("\n\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Write ROM
			currAddr = 0x0000;
			for (uint16_t bank = 1; bank < romBanks; bank++) {				
				if (bank > 1) { currAddr = 0x4000; }
				
				// Set start address
				set_number(currAddr, SET_START_ADDRESS);
				delay_ms(5);
				
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
					led_progress_percent(readBytes, (romBanks * 16384) / 28);
				}
			}
			
			printf("]");
			fclose(romFile);
		}
		
		
		
		// ****** GBA Flash Carts ******
		else if (flashCartType == 20) {
			printf("insideGadgets 32MB (512Kbit/1Mbit Flash Save) or (256Kbit FRAM) Gameboy Advance Flash Cart\n");
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			// Check file size
			if (fileSize > 0x2000000) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 32 MBytes\n", argv[1]);
				read_one_letter();
				return 1;
			}
			
			// Read rom a tiny bit before writing
			currAddr = 0x0000;	
			set_number(currAddr, SET_START_ADDRESS);
			delay_ms(5);
			set_mode(GBA_READ_ROM);
			delay_ms(5);
			com_read_bytes(READ_BUFFER, 64);
			com_read_stop();
			
			// Set end address as file size
			endAddr = fileSize;
			uint32_t endAddrAligned = fileSize;
			while ((endAddrAligned / 64)  % 64 != 0) { // Align to 64 for printing progress
				endAddrAligned--;
			}
			
			// Verify chip ID
			gba_flash_write_address_byte(0xAAA, 0xAA);
			gba_flash_write_address_byte(0x555, 0x55);
			gba_flash_write_address_byte(0xAAA, 0x90);
			
			currAddr = 0x0000;	
			set_number(currAddr, SET_START_ADDRESS);
			delay_ms(5);
			set_mode(GBA_READ_ROM);
			delay_ms(5);
			com_read_bytes(READ_BUFFER, 64);
			com_read_stop();
			gba_flash_write_address_byte(0x000, 0xF0);
			
			printf("\nFlash ID: 0x%X,0x%X,0x%X,0x%X\n", readBuffer[0], readBuffer[1], readBuffer[2], readBuffer[3]);
			if (readBuffer[0] != 0x1 || readBuffer[1] != 0x0 || readBuffer[2] != 0x7E || readBuffer[3] != 0x22) { 
				printf("\n\nChip ID doesn't match 0x1 0x0 0x7E 0x22. Please re-seat the cartridge or press enter to continue anyway.\n");
				read_one_letter();
			}
			
			// Check if file is more than 16MB, if so, do a chip erase instead of sector by sector erase
			// (sector by sector erase won't seem to work properly after 16MB because A24 is at GND)
			if (fileSize > 0x1000000) {
				currAddr = 0x0000;	
				set_number(currAddr, SET_START_ADDRESS);
				delay_ms(5);
				
				printf("Chip erase as ROM file is more than 16MB, this can take 3-4 minutes");
				gba_flash_write_address_byte(0xAAA, 0xAA);
				gba_flash_write_address_byte(0x555, 0x55);
				gba_flash_write_address_byte(0xAAA, 0x80);
				gba_flash_write_address_byte(0xAAA, 0xAA);
				gba_flash_write_address_byte(0x555, 0x55);
				gba_flash_write_address_byte(0xAAA, 0x10);
				
				// Wait for first 2 bytes to be 0xFF
				wait_for_gba_flash_erase_ff(currAddr);
				printf("\n");
			}
			
			xmas_setup(endAddr / 28);
			
			printf("\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Write ROM
			currAddr = 0x0000;	
			set_number(currAddr, SET_START_ADDRESS);
			delay_ms(5);
			while (currAddr < endAddr) {
				// Sector erase only performed for under 16MB files
				if (fileSize <= 0x1000000 && currAddr % 0x10000 == 0) { // Erase next sector
					gba_flash_write_address_byte(0xAAA, 0xAA);
					gba_flash_write_address_byte(0x555, 0x55);
					gba_flash_write_address_byte(0xAAA, 0x80);
					gba_flash_write_address_byte(0xAAA, 0xAA);
					gba_flash_write_address_byte(0x555, 0x55);
					gba_flash_write_address_byte((uint32_t) sector << 17, 0x30);
					sector++;
					
					// Wait for first 2 bytes to be 0xFF
					wait_for_gba_flash_sector_ff(currAddr, 0xFF, 0xFF);
					
					set_number(currAddr / 2, SET_START_ADDRESS); // Divide address by 2
					delay_ms(5);
				}
				
				com_write_bytes_from_file(GBA_FLASH_WRITE_256BYTE, romFile, 256);
				com_wait_for_ack();
				currAddr += 256;
				readBytes += 256;
				
				print_progress_percent(readBytes, endAddr / 64);
				led_progress_percent(readBytes, endAddr / 28);
			}
		}
		else if (flashCartType == 21) {
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
			delay_ms(5);
			set_mode(GBA_READ_ROM);
			delay_ms(5);
			com_read_bytes(READ_BUFFER, 64);
			com_read_stop();
			
			// Set end address as file size
			endAddr = fileSize;
			uint32_t endAddrAligned = fileSize;
			while ((endAddrAligned / 64)  % 64 != 0) { // Align to 64 for printing progress
				endAddrAligned--;
			}
			
			xmas_setup(endAddrAligned / 28);
			
			// Write ROM
			currAddr = 0x0000;	
			set_number(currAddr, SET_START_ADDRESS);
			delay_ms(5);
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
					wait_for_gba_flash_sector_ff(currAddr, 0xFF, 0xFF);
					
					set_number(currAddr / 2, SET_START_ADDRESS); // Divide address by 2
					delay_ms(5);
				}
				
				com_write_bytes_from_file(GBA_FLASH_WRITE_256BYTE_SWAPPED_D0D1, romFile, 256);
				com_wait_for_ack();
				currAddr += 256;
				readBytes += 256;
				
				print_progress_percent(readBytes, endAddrAligned / 64);
				led_progress_percent(readBytes, endAddrAligned / 28);
			}
		}
		else if (flashCartType == 22) {
			printf("16 MByte (MSP55LV128M / 29GL128EHMC / MX29GL128ELT / M29W128 / S29GL128) / 32MB (256M29EWH) Gameboy Advance Flash Cart\n");
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
			
			// Read rom a tiny bit before writing
			currAddr = 0x0000;	
			set_number(currAddr, SET_START_ADDRESS);
			delay_ms(5);
			set_mode(GBA_READ_ROM);
			delay_ms(5);
			com_read_bytes(READ_BUFFER, 64);
			com_read_stop();
			
			// Set end address as file size
			endAddr = fileSize;
			uint32_t endAddrAligned = fileSize;
			while ((endAddrAligned / 64)  % 64 != 0) { // Align to 64 for printing progress
				endAddrAligned--;
			}
			
			xmas_setup(endAddrAligned / 28);
			
			// Write ROM
			currAddr = 0x0000;	
			set_number(currAddr, SET_START_ADDRESS);
			delay_ms(5);
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
					wait_for_gba_flash_sector_ff(currAddr, 0xFF, 0xFF);
					
					set_number(currAddr / 2, SET_START_ADDRESS); // Divide address by 2
					delay_ms(5);
				}
				
				com_write_bytes_from_file(GBA_FLASH_WRITE_256BYTE_SWAPPED_D0D1, romFile, 256);
				com_wait_for_ack();
				currAddr += 256;
				readBytes += 256;
				
				print_progress_percent(readBytes, endAddrAligned / 64);
				led_progress_percent(readBytes, endAddrAligned / 28);
			}
		}
		else if (flashCartType == 23 || flashCartType == 24) {
			if (flashCartType == 23) {
				printf("16 MByte M36L0R706 / 32 MByte 256L30B / 4455LLZBQO / 4000L0YBQ0 Gameboy Advance Flash Cart\n");
			}
			else {
				printf("16 MByte M36L0R706 (2) / 32 MByte 256L30B (2) / 4455LLZBQO (2) / 4000L0YBQ0 (2) Gameboy Advance Flash Cart\n");
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
			delay_ms(5);
			set_mode(GBA_READ_ROM);
			delay_ms(5);
			com_read_bytes(READ_BUFFER, 64);
			com_read_stop();
			
			// Set end address as file size
			endAddr = fileSize;
			uint32_t endAddrAligned = fileSize;
			while ((endAddrAligned / 64)  % 64 != 0) { // Align to 64 for printing progress
				endAddrAligned--;
			}
			
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
			// 4000L0YBQ0 (0x8A, 0x0, 0x10, 0x88)
			else if (readBuffer[0] == 0x8A && readBuffer[1] == 0 && readBuffer[2] == 0x10 && readBuffer[3] == 0x88) {
				sectorEraseAddress = 0x8000;
			}
			
			// Back to reading mode
			gba_flash_write_address_byte(currAddr, 0xFF);
			delay_ms(5);
			
			xmas_setup(endAddrAligned / 28);
			
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
					wait_for_gba_flash_sector_ff(currAddr, 0x80, 0x00);
					
					// Back to reading mode
					gba_flash_write_address_byte(currAddr, 0xFF);
					delay_ms(5);
					
					set_number(currAddr / 2, SET_START_ADDRESS); // Divide address by 2
					delay_ms(5);
				}
				
				// Standard buffered writing
				if (flashCartType == 23) {
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
						delay_ms(5);
					}
					else {
						com_write_bytes_from_file(GBA_FLASH_WRITE_INTEL_64BYTE, romFile, 64);
						com_wait_for_ack();
						currAddr += 64;
						readBytes += 64;
					}
				}
				print_progress_percent(readBytes, endAddrAligned / 64);
				led_progress_percent(readBytes, endAddrAligned / 28);
			}
			
			// Back to reading mode
			gba_flash_write_address_byte(currAddr, 0xFF);
			delay_ms(5);
		}
		else if (flashCartType == 25) {
			printf("16 MByte GE28F128W30 Gameboy Advance Flash Cart\n");
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			// Check file size
			if (fileSize > 0x1000000) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 16 MBytes\n", argv[1]);
				read_one_letter();
				return 1;
			}
			
			if (gbxcartFirmwareVersion <= 14) {
				printf("Firmware R15 or higher is required for this functionality.\n");
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
			delay_ms(5);
			set_mode(GBA_READ_ROM);
			delay_ms(5);
			com_read_bytes(READ_BUFFER, 64);
			com_read_stop();
			
			// Set end address as file size
			endAddr = fileSize;
			uint32_t endAddrAligned = fileSize;
			while ((endAddrAligned / 64)  % 64 != 0) { // Align to 64 for printing progress
				endAddrAligned--;
			}
			
			// Flash ID command
			gba_flash_write_address_byte(0x00, 0x90);
			delay_ms(1);
			
			// Read ID
			set_number(0x00, SET_START_ADDRESS);
			delay_ms(5);
			set_mode(GBA_READ_ROM);
			delay_ms(5);
			com_read_bytes(READ_BUFFER, 64);
			com_read_stop(); // End read
			
			// Sector erase length
			int sectorEraseAddress = 0x10000;
			// 28F128W30 (0x8A, 0x0, 0x5x, 0x88)
			if (readBuffer[0] == 0x8A && readBuffer[1] == 0 && (readBuffer[2] & 0xF8) == 0x50 && readBuffer[3] == 0x88) {
				sectorEraseAddress = 0x2000; // 8K sectors at start
			}
			
			// Back to reading mode
			gba_flash_write_address_byte(currAddr, 0xFF);
			delay_ms(5);
			
			
			// Write ROM
			currAddr = 0x0000;	
			set_number(currAddr, SET_START_ADDRESS);
			delay_ms(5);
			
			while (currAddr < endAddr) {
				if (currAddr % sectorEraseAddress == 0) { // Erase next sector
					// For chips with 8x 8K sectors, after 64K, change to 0x10000 sector size
					if (currAddr >= 0x10000 && sectorEraseAddress == 0x2000) {
						sectorEraseAddress = 0x10000;
					}
					
					// Unlock
					gba_flash_write_address_byte(currAddr, 0x60);
					gba_flash_write_address_byte(currAddr, 0xD0);
					
					// Erase
					gba_flash_write_address_byte(currAddr, 0x20);
					gba_flash_write_address_byte(currAddr, 0xD0);
					delay_ms(50);
					
					// Wait for byte to be 0x80 or 0xB0
					uint16_t timeout = 0;
					readBuffer[0] = 0;
					readBuffer[1] = 0;
					while (readBuffer[0] != 0x80 && readBuffer[0] != 0xB0) {
						set_number(currAddr / 2, SET_START_ADDRESS);
						delay_ms(5);
						set_mode(GBA_READ_ROM);
						delay_ms(5);
						
						com_read_bytes(READ_BUFFER, 64);
						com_read_stop(); // End read
						delay_ms(50);
						
						timeout++;
						if (timeout >= 200) {
							printf("\n\nWaiting for sector erase has timed out. Please unplug GBxCart RW, re-seat the cartridge and try again.\n");
							read_one_letter();
							exit(1);
						}
					}
					
					// Back to reading mode
					gba_flash_write_address_byte(currAddr, 0xFF);
					delay_ms(5);
					
					set_number(currAddr / 2, SET_START_ADDRESS); // Divide address by 2
					delay_ms(5);
				}
				
				// Word writing
				com_write_bytes_from_file(GBA_FLASH_WRITE_INTEL_64BYTE_WORD, romFile, 64);
				com_wait_for_ack();
				currAddr += 64;
				readBytes += 64;
				
				print_progress_percent(readBytes, endAddrAligned / 64);
				led_progress_percent(readBytes, endAddrAligned / 28);
			}
			
			// Back to reading mode
			gba_flash_write_address_byte(currAddr, 0xFF);
			delay_ms(5);
		}
		else if (flashCartType == 26) {
			printf("4 MByte (MX29LV320) Gameboy Advance Flash Cart\n");
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			// Check file size
			if (fileSize > 0x400000) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 4 MBytes\n", argv[1]);
				read_one_letter();
				return 1;
			}
			
			printf("\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Read rom a tiny bit before writing
			currAddr = 0x0000;	
			set_number(currAddr, SET_START_ADDRESS);
			delay_ms(5);
			set_mode(GBA_READ_ROM);
			delay_ms(5);
			com_read_bytes(READ_BUFFER, 64);
			com_read_stop();
			
			// Set end address as file size
			endAddr = fileSize;
			uint32_t endAddrAligned = fileSize;
			while ((endAddrAligned / 64)  % 64 != 0) { // Align to 64 for printing progress
				endAddrAligned--;
			}
			
			xmas_setup(endAddrAligned / 28);
			
			// Write ROM
			currAddr = 0x0000;	
			set_number(currAddr, SET_START_ADDRESS);
			int sectorEraseAddress = 0x2000;
			while (currAddr < endAddr) {
				if (currAddr >= 0x10000) {
					sectorEraseAddress = 0x10000;
				}
				if (currAddr % sectorEraseAddress == 0) { // Erase next sector
					gba_flash_write_address_byte(0xAAA, 0xAA);
					gba_flash_write_address_byte(0x555, 0x55);
					gba_flash_write_address_byte(0xAAA, 0x80);
					gba_flash_write_address_byte(0xAAA, 0xAA);
					gba_flash_write_address_byte(0x555, 0x55);
					gba_flash_write_address_byte((uint32_t) sector << 13, 0x30);
					sector++;
					
					// Wait for first 2 bytes to be 0xFF
					wait_for_gba_flash_sector_ff(currAddr, 0xFF, 0xFF);
					
					set_number(currAddr / 2, SET_START_ADDRESS); // Divide address by 2
					delay_ms(5);
				}
				
				com_write_bytes_from_file(GBA_FLASH_WRITE_256BYTE, romFile, 256);
				com_wait_for_ack();
				currAddr += 256;
				readBytes += 256;
				
				print_progress_percent(readBytes, endAddrAligned / 64);
				led_progress_percent(readBytes, endAddrAligned / 28);
			}
		}
		else if (flashCartType == 27) {
			printf("insideGadgets 32MB 4K/64K EEPROM Gameboy Advance Flash Cart\n");
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			// Check file size
			if (fileSize > 0x2000000) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 32 MBytes\n", argv[1]);
				read_one_letter();
				return 1;
			}
			
			// Read rom a tiny bit before writing
			currAddr = 0x0000;	
			set_number(currAddr, SET_START_ADDRESS);
			delay_ms(5);
			set_mode(GBA_READ_ROM);
			delay_ms(5);
			com_read_bytes(READ_BUFFER, 64);
			com_read_stop();
			
			// Set end address as file size
			endAddr = fileSize;
			uint32_t endAddrAligned = fileSize;
			while ((endAddrAligned / 64)  % 64 != 0) { // Align to 64 for printing progress
				endAddrAligned--;
			}
			
			// Verify chip ID
			gba_flash_write_address_byte(0xAAA, 0xAA);
			gba_flash_write_address_byte(0x555, 0x55);
			gba_flash_write_address_byte(0xAAA, 0x90);
			
			currAddr = 0x0000;	
			set_number(currAddr, SET_START_ADDRESS);
			delay_ms(5);
			set_mode(GBA_READ_ROM);
			delay_ms(5);
			com_read_bytes(READ_BUFFER, 64);
			com_read_stop();
			gba_flash_write_address_byte(0x000, 0xF0);
			
			printf("Flash ID: 0x%X,0x%X,0x%X,0x%X\n", readBuffer[0], readBuffer[1], readBuffer[2], readBuffer[3]);
			if (readBuffer[0] != 0x1 || readBuffer[1] != 0x0 || readBuffer[2] != 0x7E || readBuffer[3] != 0x22) { 
				printf("\nChip ID doesn't match 0x1 0x0 0x7E 0x22. Please re-seat the cartridge or press enter to continue anyway.\n");
				read_one_letter();
			}
			
			// Check if file is more than 16MB, if so, do a chip erase instead of sector by sector erase
			// (sector by sector erase won't seem to work properly after 16MB because A24 is at GND)
			if (fileSize > 0x1000000) {
				currAddr = 0x0000;	
				set_number(currAddr, SET_START_ADDRESS);
				delay_ms(5);
				
				printf("Chip erase as ROM file is more than 16MB, this can take 3-4 minutes");
				gba_flash_write_address_byte(0xAAA, 0xAA);
				gba_flash_write_address_byte(0x555, 0x55);
				gba_flash_write_address_byte(0xAAA, 0x80);
				gba_flash_write_address_byte(0xAAA, 0xAA);
				gba_flash_write_address_byte(0x555, 0x55);
				gba_flash_write_address_byte(0xAAA, 0x10);
				
				// Wait for first 2 bytes to be 0xFF
				wait_for_gba_flash_erase_ff(currAddr);
				printf("\n");
			}
			
			xmas_setup(endAddrAligned / 28);
			
			printf("\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
			printf("[             25%%             50%%             75%%            100%%]\n[");
			
			// Write ROM
			currAddr = 0x0000;	
			set_number(currAddr, SET_START_ADDRESS);
			delay_ms(5);
			while (currAddr < endAddr) {
				// Sector erase only performed for under 16MB files
				if (fileSize <= 0x1000000 && currAddr % 0x10000 == 0) { // Erase next sector
					gba_flash_write_address_byte(0xAAA, 0xAA);
					gba_flash_write_address_byte(0x555, 0x55);
					gba_flash_write_address_byte(0xAAA, 0x80);
					gba_flash_write_address_byte(0xAAA, 0xAA);
					gba_flash_write_address_byte(0x555, 0x55);
					gba_flash_write_address_byte((uint32_t) sector << 17, 0x30);
					sector++;
					
					// Wait for first 2 bytes to be 0xFF
					wait_for_gba_flash_sector_ff(currAddr, 0xFF, 0xFF);
					
					set_number(currAddr / 2, SET_START_ADDRESS); // Divide address by 2
					delay_ms(5);
				}
				
				com_write_bytes_from_file(GBA_FLASH_WRITE_256BYTE, romFile, 256);
				com_wait_for_ack();
				currAddr += 256;
				readBytes += 256;
				
				print_progress_percent(readBytes, endAddr / 64);
				led_progress_percent(readBytes, endAddrAligned / 28);
				
				// Break when reaching 0x1FFF00 as this is when the EEPROM is mapped to
				if (currAddr == 0x1FFFF00) {
					break;
				}
			}
		}
		else {
			printf("No Flash Cart selected, please run this program by itself.");
			read_one_letter();
			return 1;
		}
	}	
	else {
		printf("\nPlease select a Flash Cart:\n"\
					 "--- Gameboy: insideGadgets Carts ---\n"\
					 "1. insideGadgets 32 KByte (incl 4KB FRAM) Flash Cart\n"\
					 "2. insideGadgets 512KB Flash Cart\n"\
					 "3. insideGadgets 1 MByte 128KB SRAM Flash Cart\n"\
					 "4. insideGadgets 1 MByte 128KB SRAM Custom Logo Flash Cart\n"\
					 "5. insideGadgets 2 MByte 128KB SRAM Flash Cart\n"\
					 "6. insideGadgets 2 MByte 32KB FRAM Flash Cart\n"\
					 "7. insideGadgets 4 MByte 128KB SRAM/FRAM Flash Cart\n"\
					 "8. insideGadgets 4 MByte 32KB FRAM MBC3 RTC Flash Cart\n"\
					 "9. insideGadgets 64 MByte 128KB SRAM Mighty Flash Cart\n"\
					 "10. insideGadgets 64 MByte 128KB SRAM Mighty Flash Cart Buffered (Experimental)\n\n"\
					 
					 "--- Gameboy ---\n"\
					 "11. 32 KByte\n"\
					 "12. 512 KByte (SST39SF040)\n"\
					 "13. 512 KByte (AM29LV160 CPLD cart)\n"\
					 "14. 1 MByte (ES29LV160)\n"\
					 "15. 1 MByte (29LV320 CPLD cart)\n"\
					 "16. 2 MByte (BV5)\n"\
					 "17. 2 MByte (AM29LV160DB / 29LV160CTTC / 29LV160TE / S29AL016 / M29W160EB)\n");
		
		printf("\nPress any key to see the next page...");
		getchar();
		
		printf("\n18. 2 MByte (AM29F016B)\n"\
					"19. 2 MByte (AM29F016B) (Audio as WE)\n"\
					 "20. 2 MByte (GB Smart 16M)\n"\
					 "21. 4 MByte (M29W640 / 29DL32BF / GL032A10BAIR4 / S29AL016M9)\n"\
					 "22. 4 MByte MBC30 (AM29F032B / MBM29F033C)\n"\
					 "23. 32 MByte (4x 8MB Banks) (256M29)\n"\
					 "24. 32 MByte (4x 8MB Banks) (M29W256 / MX29GL256)\n\n"\
					 
					 "--- Gameboy Advance ---\n"\
					 "25. insideGadgets 32MB (512Kbit/1Mbit Flash Save) or (256Kbit FRAM) Flash Cart\n"\
					 "26. insideGadgets 32MB 4Kbit/64Kbit EEPROM Save Flash Cart\n"\
					 "27. 16 MByte (MSP55LV128 / 29LV128DTMC)\n"\
					 "28. 16 MByte (MSP55LV128M / 29GL128EHMC / MX29GL128ELT / M29W128 / S29GL128) / 32MB (256M29EWH)\n"\
					 "29. 16 MByte M36L0R706 / 32 MByte 256L30B / 4455LLZBQO / 4000L0YBQ0\n"\
					 "30. 16 MByte M36L0R706 (2) / 32 MByte 256L30B (2) / 4455LLZBQO (2) / 4000L0YBQ0 (2)\n"\
					 "31. 16 MByte GE28F128W30\n"\
					 "32. 4 MByte (MX29LV320)\n\n"\
					 "x. Exit\n>");
		
		char optionString[5];
		fgets(optionString, 5, stdin);
		
		int optionSelected = atoi(optionString);
		if (optionSelected == 11) {
			printf("\nPlease select a Flash Chip:\n"\
					 "1. AM29F010B (Audio as WE)\n"\
					 "2. AM29F010B (WR as WE)\n"\
					 "3. SST39SF010A / AT49F040 (Audio as WE)\n"\
					 "4. SST39SF010A / AT49F040 (WR as WE)\n"\
					 "x. Exit\n>");
			
			fgets(optionString, 5, stdin);
			optionSelected = atoi(optionString);
			write_flash_config(optionSelected+100);
			printf("\nPlease close this program. You can now drag and drop your ROM file to this exe file in Windows Explorer.\nYou can also use the following command: gbxcart_rw_flasher_v1.xx.exe <ROMFile>\nPress enter to exit.");
			read_one_letter();
			return 0;
		}
		else {
			write_flash_config(optionSelected); // Custom flash cart mapping occurs here, the selected number changes to a constant number
			printf("\nPlease close this program. You can now drag and drop your ROM file to this exe file in Windows Explorer.\nYou can also use the following command: gbxcart_rw_flasher_v1.xx.exe <ROMFile>\nPress enter to exit.");
			read_one_letter();
			return 0;
		}
	}
	
	printf("\n");
	xmas_idle_on();
	
	return 0;
}