/*
 GBxCart RW - Nintendo Power Cart Backup Full ROM/Hidden Sector
 Version: 1.2
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 20/03/2019
 Last Modified: 1/05/2021
 
 Backup your Gameboy 1MB Nintendo Power Flash Cart's ROM and the hidden sector.
 
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
	printf("Backup Full ROM/Hidden Sector v1.2 by insideGadgets\n");
	printf("###################################################\n");
	
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
	
	
	// Enable flash chip access
	set_bank(0x120, 0x09);
	set_bank(0x121, 0xaa);
	set_bank(0x122, 0x55);
	set_bank(0x13f, 0xa5);
	
	// Re-Enable writes to MBC registers
	set_bank(0x120, 0x11);
	set_bank(0x13f, 0xa5);
	
	// Set bank 1
	set_bank(0x2100, 0x01);
	delay_ms(50);
	
	// Reset flash to read
	set_bank(0x120, 0x02);
	set_bank(0x120, 0x0F);
	set_bank(0x125, 0x40);
	set_bank(0x126, 0x80);
	set_bank(0x127, 0xF0);
	set_bank(0x13f, 0xA5);
	
	// Map all flash chip
	set_bank(0x120, 0x04);
	set_bank(0x13f, 0xa5);
	
	// Undo Wakeup
	set_bank(0x120, 0x08);
	set_bank(0x13f, 0xa5);
	delay_ms(50);
	
	
	// Get the date/time to be used for the filename
	time_t rawtime;
	struct tm* timeinfo;
	char timebuffer[130];
	
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	strftime (timebuffer,120,"NP_1MB_ROM_Backup_%Y-%m-%d_%H-%M-%S.gb",timeinfo);
	
	
	printf("\n--- Read ROM ---\n");
	currAddr = 0x0000;	
	set_number(currAddr, SET_START_ADDRESS);
	romBanks = 64; // 1MB size
	
	printf("Reading ROM to %s\n", timebuffer);
	printf("[             25%%             50%%             75%%            100%%]\n[");
	
	// Create a new file
	FILE *romFile = fopen(timebuffer, "wb");
	
	uint32_t readBytes = 0;
	if (cartridgeMode == GB_MODE) {
		xmas_setup((romBanks * 16384) / 28);
		// Set start and end address
		currAddr = 0x0000;
		endAddr = 0x7FFF;
		
		// Read ROM
		for (uint16_t bank = 1; bank < romBanks; bank++) {				
			set_bank(0x2100, bank & 0xFF);
			if (bank >= 256) {
				set_bank(0x3000, 1); // High bit
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
				led_progress_percent(readBytes, (romBanks * 16384) / 28);
			}
			com_read_stop(); // Stop reading ROM (as we will bank switch)
		}
		printf("]");
	}
	
	
	// Enable flash chip access
	set_bank(0x120, 0x09);
	set_bank(0x121, 0xaa);
	set_bank(0x122, 0x55);
	set_bank(0x13f, 0xa5);
	
	// Re-Enable writes to MBC registers
	set_bank(0x120, 0x11);
	set_bank(0x13f, 0xa5);
	
	// Set bank 1
	set_bank(0x2100, 0x01);
	delay_ms(5);
	
	// Backup hidden sector
	set_bank(0x120, 0x0F);
	set_bank(0x125, 0x55);
	set_bank(0x126, 0x55);
	set_bank(0x127, 0xAA);
	set_bank(0x13f, 0xA5);
	delay_ms(5);
	
	set_bank(0x120, 0x0F);
	set_bank(0x125, 0x2A);
	set_bank(0x126, 0xAA);
	set_bank(0x127, 0x55);
	set_bank(0x13f, 0xA5);
	delay_ms(5);
	
	set_bank(0x120, 0x0F);
	set_bank(0x125, 0x55);
	set_bank(0x126, 0x55);
	set_bank(0x127, 0x77);
	set_bank(0x13f, 0xA5);
	delay_ms(5);
	
	set_bank(0x120, 0x0F);
	set_bank(0x125, 0x55);
	set_bank(0x126, 0x55);
	set_bank(0x127, 0xAA);
	set_bank(0x13f, 0xA5);
	delay_ms(5);
	
	set_bank(0x120, 0x0F);
	set_bank(0x125, 0x2A);
	set_bank(0x126, 0xAA);
	set_bank(0x127, 0x55);
	set_bank(0x13f, 0xA5);
	delay_ms(5);
	
	set_bank(0x120, 0x0F);
	set_bank(0x125, 0x55);
	set_bank(0x126, 0x55);
	set_bank(0x127, 0x77);
	set_bank(0x13f, 0xA5);
	delay_ms(5);
	
	
	char npHiddenFilename[130];
	strftime (npHiddenFilename,120,"NP_HiddenSector_%Y-%m-%d_%H-%M-%S.gb",timeinfo);
	printf("\n\nBacking up NP Hidden Sector to %s\n", npHiddenFilename);
	
	FILE *npFile = fopen(npHiddenFilename, "wb");
	currAddr = 0x0000;
	endAddr = 0x100;
	set_number(currAddr, SET_START_ADDRESS);
	set_mode(READ_ROM_RAM);
	
	// Read data
	while (currAddr < endAddr) {
		com_read_bytes(npFile, 64);
		currAddr += 64;
		
		// Request 64 bytes more
		if (currAddr < endAddr) {
			com_read_cont();
		}
	}
	
	com_read_stop(); // Stop reading ROM (as we will bank switch)
	fclose(npFile);
	
	gbx_set_done_led();
	gbx_cart_power_down();
	
	return 0;
}