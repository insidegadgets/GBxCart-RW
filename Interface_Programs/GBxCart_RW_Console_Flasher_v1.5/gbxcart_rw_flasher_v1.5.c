/*
 GBxCart RW - Console Interface Flasher
 Version: 1.5
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 26/08/2017
 Last Modified: 17/01/2018
 
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

#include "setup.h" // See defines, variables, constants, functions here

int main(int argc, char **argv) {
	
	printf("GBxCart RW Flasher v1.5 by insideGadgets\n");
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
		if (flashCartType == 1) {
			printf("32 KByte Gameboy Flash Cart\n");
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
					
					com_write_bytes_from_file('U', romFile, 64);
					com_wait_for_ack();
					currAddr += 64;
					readBytes += 64;
					
					print_progress_percent(readBytes, (endAddr+1) / 64);
				}
			}
			else if (gbxcartFirmwareVersion >= 4) {
				fw3_gb_flash_setup();
				
				while (currAddr <= endAddr) {
					if (currAddr % 0x4000 == 0) { // Erase sectors
						fw4_gb_flash_sector_erase(sector);
						wait_for_flash_ff(currAddr);
						sector++;
						
						set_number(currAddr, SET_START_ADDRESS);
					}
					
					set_mode(GB_FLASH_WRITE_64BYTE);
					com_write_bytes_from_file(GB_AUDIO_FLASH_WRITE, romFile, 64);
					com_wait_for_ack();
					currAddr += 64;
					readBytes += 64;
					
					print_progress_percent(readBytes, (endAddr+1) / 64);
				}
			}
			else {
				fclose(romFile);
				printf("Firmware R3 or higher is required for this functionality.\n");
				read_one_letter();
				return 1;
			}
			
			printf("]");
			fclose(romFile);
		}
		else if (flashCartType == 2) {
			printf("2 MByte (BV5) Gameboy Flash Cart\n");
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
			
			if (gbxcartFirmwareVersion <= 3) {
				fclose(romFile);
				printf("Firmware R4 or higher is required for this functionality.\n");
				read_one_letter();
				return 1;
			}
			
			// Calculate banks needed from ROM file size
			romBanks = fileSize / 16384;
			
			// Read rom a tiny bit before writing
			currAddr = 0x0000;	
			set_number(currAddr, SET_START_ADDRESS);
			set_mode(READ_ROM_RAM);
			com_read_bytes(READ_BUFFER, 64);
			com_read_stop();
			
			
			printf("\nErasing Flash");
			
			// Chip erase (data byte's bit 0 & 1 are swapped for chip commands as BV5 D0 & D1 lines are swapped)
			gb_bv5_flash_write_address_byte(0xAAA, 0xA9);
			gb_bv5_flash_write_address_byte(0x555, 0x56);
			gb_bv5_flash_write_address_byte(0xAAA, 0x80);
			gb_bv5_flash_write_address_byte(0xAAA, 0xA9);
			gb_bv5_flash_write_address_byte(0x555, 0x56);
			gb_bv5_flash_write_address_byte(0xAAA, 0x10);
			
			// Wait for first byte to be 0xFF
			readBuffer[0] = 0;
			while (readBuffer[0] != 0xFF) {
				set_number(currAddr, SET_START_ADDRESS);
				set_mode(GBA_READ_SRAM);
				
				com_read_bytes(READ_BUFFER, 64);
				com_read_stop(); // End read
				
				printf(".");
				
				if (readBuffer[0] != 0xFF) {
					delay_ms(500);
				}
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
					}
					
					set_mode(GB_FLASH_WRITE_64BYTE);
					com_write_bytes_from_file(GB_BV5_FLASH_WRITE, romFile, 64);
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
			
			if (gbxcartFirmwareVersion <= 6) {
				fclose(romFile);
				printf("Firmware R7 or higher is required for this functionality.\n");
				read_one_letter();
				return 1;
			}
			
			// Calculate banks needed from ROM file size
			romBanks = fileSize / 16384;
			
			// Read rom a tiny bit before writing
			currAddr = 0x0000;	
			set_number(currAddr, SET_START_ADDRESS);
			set_mode(READ_ROM_RAM);
			com_read_bytes(READ_BUFFER, 64);
			com_read_stop();
			
			
			printf("\nErasing Flash");
			
			// Chip erase (data byte's bit 0 & 1 are swapped for chip commands as BV5 D0 & D1 lines are swapped)
			gb_bv5_flash_write_address_byte(0x555, 0xA9);
			gb_bv5_flash_write_address_byte(0x2AA, 0x56);
			gb_bv5_flash_write_address_byte(0x555, 0x80);
			gb_bv5_flash_write_address_byte(0x555, 0xA9);
			gb_bv5_flash_write_address_byte(0x2AA, 0x56);
			gb_bv5_flash_write_address_byte(0x555, 0x10);
			
			// Wait for first byte to be 0xFF
			readBuffer[0] = 0;
			while (readBuffer[0] != 0xFF) {
				set_number(currAddr, SET_START_ADDRESS);
				set_mode(GBA_READ_SRAM);
				
				com_read_bytes(READ_BUFFER, 64);
				com_read_stop(); // End read
				
				printf(".");
				
				if (readBuffer[0] != 0xFF) {
					delay_ms(500);
				}
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
					}
					
					set_mode(GB_FLASH_WRITE_64BYTE);
					com_write_bytes_from_file(GB_WORD_FLASH_WRITE, romFile, 64);
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
		else if (flashCartType == 4) {
			printf("16 MByte (MSP55LV128) Gameboy Advance Flash Cart\n");
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			// Check file size
			if (fileSize > 0x1000000) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 16 MBytes\n", argv[1]);
				read_one_letter();
				return 1;
			}
			
			if (gbxcartFirmwareVersion <= 4) {
				fclose(romFile);
				printf("Firmware R5 or higher is required for this functionality.\n");
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
				
				com_write_bytes_from_file(GBA_FLASH_WRITE_64BYTE, romFile, 64);
				com_wait_for_ack();
				currAddr += 64;
				readBytes += 64;
				
				print_progress_percent(readBytes, (endAddr) / 64);
			}
		}
		else if (flashCartType == 5) {
			printf("16 MByte (MSP55LV128M / 29GL128EHMC) Gameboy Advance Flash Cart\n");
			printf("\nGoing to write to ROM (Flash cart) from %s\n", filenameOnly);
			
			// Check file size
			if (fileSize > 0x1000000) {
				fclose(romFile);
				printf("\n%s \nFile size is larger than the available Flash cart space of 16 MBytes\n", argv[1]);
				read_one_letter();
				return 1;
			}
			
			if (gbxcartFirmwareVersion <= 5) {
				fclose(romFile);
				printf("Firmware R6 or higher is required for this functionality.\n");
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
				
				com_write_bytes_from_file(GBA_FLASH_WRITE_64BYTE, romFile, 64);
				com_wait_for_ack();
				currAddr += 64;
				readBytes += 64;
				
				print_progress_percent(readBytes, (endAddr) / 64);
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
					 "--- Gameboy ---\n"\
					 "1. 32 KByte Gameboy Flash Cart\n"\
					 "2. 2 MByte (BV5) Gameboy Flash Cart\n"\
					 "3. 1 MByte (ES29LV160) Gameboy Flash Cart\n\n"\
					 "--- Gameboy Advance ---\n"\
					 "4. 16 MByte (MSP55LV128) Gameboy Advance Flash Cart\n"\
					 "5. 16 MByte (MSP55LV128M / 29GL128EHMC) Gameboy Advance Flash Cart\n"\
					 "x. Exit\n>");
		
		char optionSelected = read_one_letter();
		if (optionSelected == '1') {
			write_flash_config(1);
		}
		else if (optionSelected == '2') {
			write_flash_config(2);
		}
		else if (optionSelected == '3') {
			write_flash_config(3);
		}
		else if (optionSelected == '4') {
			write_flash_config(4);
		}
		else if (optionSelected == '5') {
			write_flash_config(5);
		}
		
		printf("\nYou can now drag and drop your ROM file to this exe file.");
	}
	
	printf("\n");
	
	return 0;
}