/*
 GBxCart RW - insideGadgets Power Cart Flasher
 Version: 1.2
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 1/05/2020
 Last Modified: 1/05/2021
 
 Write a ROM to the insideGadgets Gameboy 1MB Power Flash Cart.
 
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
	
	printf("GBxCart RW - insideGadgets Power Cart:\n");
	printf("Flasher v1.2 by insideGadgets\n");
	printf("#############################\n");
	
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
		
		// Get cartridge mode - Gameboy or Gameboy Advance
		cartridgeMode = request_value(CART_MODE);
		
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
		
		printf("\n--- Write ROM to Flash Cart ---\n");
		printf("Cart selected: insideGadgets Gameboy 1MB Power Flash Cart");
		
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
		
		// Power cart setup
		set_mode(GB_CART_MODE); // Gameboy mode
		set_bank(0x6000, 0x00); // ROM bank 0, allow for all ROM access
		set_bank(0x2000, 0);
		set_bank(0x4000, 0x00); // RAM bank 0
		set_bank(0x0000, 0x00); // RAM not accessible
		
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
		
		gbx_set_done_led();
		gbx_cart_power_down();
	}
	else {
		printf("\nYou must drag and drop your ROM file to this exe file in Windows Explorer.\nYou can also use the following command: npc_flasher_vxxx.exe <ROMFile>\nPress enter to exit.");
		read_one_letter();
		return 0;
	}
	printf("\n");
	
	return 0;
}