/*
 GBxCart RW - 32KBx15 Flasher
 Version: 1.1
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 3/03/2020
 Last Modified: 21/04/2021
 License: CC-BY-NC-SA
 
 This program allows you to write ROMs to the insideGadgets 32KBx15 Flash Cart.
 
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
	
	printf("GBxCart RW 32KBx15 Flasher v1.1 by insideGadgets\n");
	printf("################################################\n");
	
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
		
		currAddr = 0x0000;
		endAddr = 0x7FFF;
		
		// PCB v1.3+ - Set 5V
		if (gbxcartPcbVersion == PCB_1_3 || gbxcartPcbVersion == PCB_1_4 || gbxcartPcbVersion == GBXMAS) {
			set_mode(VOLTAGE_5V);
			delay_ms(500);
		}
		
		// GBx v1.4 - Power up the cart if not already powered up and flush buffer
		gbx_cart_power_up();		
		RS232_flushRX(cport_nr);
		
		// Check file size
		if (fileSize > (endAddr+1)) {
			fclose(romFile);
			printf("\n%s \nFile size is larger than the available Flash cart space of 32KB\n", argv[1]);
			read_one_letter();
			return 1;
		}
		
		// Flash Setup
		set_mode(GB_CART_MODE); // Gameboy mode
		gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
		gb_flash_program_setup(GB_FLASH_PROGRAM_5555);// Flash program byte method
		
		// Print current games
		printf("Cart listing:\n");
		for (uint8_t bank = 1; bank < 16; bank++) {
			set_bank(0x2000, 0x16);
			delay_ms(5);
			set_bank(0x1000, 0x2D);
			delay_ms(5);
			set_bank(0x0000, 0x32);
			delay_ms(5);
			
			set_bank(0x2000, bank);
			delay_ms(5);
			
			printf("%i. ", bank);
			read_gb_title();
		}
		
		printf("\n\nROM file selected: %s\n", filenameOnly);
		
		printf("\nEnter slot number to overwrite (1-15): ");
		char optionString[5];
		fgets(optionString, 5, stdin);
		uint8_t slotNumber = atoi(optionString);
		
		printf("\nSector erasing");
		sector = 0;
		
		
		set_bank(0x2000, 0x16);
		delay_ms(5);
		set_bank(0x1000, 0x2D);
		delay_ms(5);
		set_bank(0x0000, 0x32);
		delay_ms(5);
		
		set_bank(0x2000, slotNumber);
		delay_ms(5);
		
		for (uint8_t x = 0; x < 8; x++) {
			gb_flash_write_address_byte(0x5555, 0xAA);
			gb_flash_write_address_byte(0x2AAA, 0x55);
			gb_flash_write_address_byte(0x5555, 0x80);
			gb_flash_write_address_byte(0x5555, 0xAA);
			gb_flash_write_address_byte(0x2AAA, 0x55);
			gb_flash_write_address_byte(0x1000 * sector, 0x30);
			
			// Wait for first byte to be 0xFF
			wait_for_flash_chip_erase_ff(1);
			sector++;
		}
		
		printf("\nWriting to ROM (Flash cart) from %s\n", filenameOnly);
		printf("[             25%%             50%%             75%%            100%%]\n[");
		
		// Write ROM
		currAddr = 0x0000;
		endAddr = 0x7FFF;
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
		gbx_set_done_led();
		gbx_cart_power_down();
	}
	else {
		printf("Please drag and drop your ROM file to this program in Windows explorer or with a command line: gbxcart_rw_32kbx15_flasher_vxxx <ROM file>\n");
		//read_one_letter();
	}
	printf("\n");
	
	return 0;
}