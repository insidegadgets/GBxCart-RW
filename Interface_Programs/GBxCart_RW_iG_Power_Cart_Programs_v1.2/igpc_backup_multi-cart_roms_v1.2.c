/*
 GBxCart RW - insideGadgets Power Cart, Backup Multi-Game Cart ROMs
 Version: 1.2
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 1/05/2020
 Last Modified: 1/05/2021
 
 Backup your insideGadgets Gameboy 1MB Power Flash Cart's Multi-game ROMs. 
 
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
	
	printf("GBxCart RW - insideGadgets Power Cart:\n");
	printf("Backup Multi-Game ROMs v1.2 by insideGadgets\n");
	printf("############################################\n");
	
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
	
	// Get the date/time to be used for the filename
	time_t rawtime;
	struct tm* timeinfo;
	char timebuffer[130];
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	strftime (timebuffer,120,"_%Y-%m-%d_%H-%M-%S.gb",timeinfo);
	
	
	// Power cart setup
	set_bank(0x6000, 0x00); // ROM bank 0, allow for all ROM access
	set_bank(0x2000, 0);
	set_bank(0x4000, 0x00); // RAM bank 0
	set_bank(0x0000, 0x00); // RAM not accessible
	
	
	// Read game ROM/RAM info
	uint16_t startAddr = 0;
	currAddr = 0x2000;
	endAddr = 0x2300;
	set_number(currAddr, SET_START_ADDRESS);
	set_mode(READ_ROM_RAM);
	
	uint8_t infoBuffer[0x400];
	while (currAddr < endAddr) {
		uint8_t comReadBytes = com_read_bytes(READ_BUFFER, 64);
		
		if (comReadBytes == 64) {
			memcpy(&infoBuffer[startAddr], readBuffer, 64);
			startAddr += 64;
			currAddr += 64;
			
			// Request 64 bytes more
			if (currAddr < endAddr) {
				com_read_cont();
			}
		}
		else { // Didn't receive 64 bytes, usually this only happens for Apple MACs
			com_read_stop();
			delay_ms(500);
			
			// Flush buffer
			RS232_PollComport(cport_nr, readBuffer, 64);											
			
			// Start off where we left off
			set_number(currAddr, SET_START_ADDRESS);
			set_mode(READ_ROM_RAM);				
		}
	}
	com_read_stop();
	
	// Set Audio pin as output
	if (gbxcartFirmwareVersion >= 22) {
		set_mode(AUDIO_HIGH);
		delay_ms(300);
	}
	else {
		RS232_cputs(cport_nr, "OE0x02");
		RS232_SendByte(cport_nr, 0);
		RS232_drain(cport_nr);
	}
	
	// Loop 7 games
	char loaderGameTitle[18];
	for (uint8_t g = 1; g < 8; g++) {
		
		// Game present
		if (infoBuffer[g] != 0xFF) {
			// Audio - set low to reset multi-game
			if (gbxcartFirmwareVersion >= 22) {
				set_mode(AUDIO_LOW);
				delay_ms(300);
			}
			else {
				RS232_cputs(cport_nr, "LE0x02");
				RS232_SendByte(cport_nr, 0);
				RS232_drain(cport_nr);
				delay_ms(300);
			}
			
			// Audio - set high
			if (gbxcartFirmwareVersion >= 22) {
				set_mode(AUDIO_HIGH);
				delay_ms(300);
			}
			else {
				RS232_cputs(cport_nr, "HE0x02");
				RS232_SendByte(cport_nr, 0);
				RS232_drain(cport_nr);
				delay_ms(300);
			}
			
			// Power cart setup
			set_bank(0x6000, infoBuffer[g]); // ROM bank
			set_bank(0x2000, 0);
			set_bank(0x4000, infoBuffer[g+0x200]); // RAM bank + 8KB or 32KB locked
			set_bank(0x0000, 0x01); // RAM accessible
			
			printf("\nMulti-Game Cart - Game %d\n", g);
			
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
					strncpy(titleFilename, "iGPC_", 20);
					strncat(titleFilename, gameTitle, 20);
					strncat(titleFilename, timebuffer, 121);
					strncat(titleFilename, "\0", 2);
					
					printf("Reading ROM to %s\n", titleFilename);
					printf("[             25%%             50%%             75%%            100%%]\n[");
					
					// Create a new file
					FILE *romFile = fopen(titleFilename, "wb");
					
					uint32_t readBytes = 0;
					if (cartridgeMode == GB_MODE) {
						xmas_setup((romBanks * 16384) / 28);
						
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
								led_progress_percent(readBytes, (romBanks * 16384) / 28);
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
	}
	
	// Audio - set low to reset multi-game
	if (gbxcartFirmwareVersion >= 22) {
		set_mode(AUDIO_LOW);
		delay_ms(300);
	}
	else {
		RS232_cputs(cport_nr, "LE0x02");
		RS232_SendByte(cport_nr, 0);
		RS232_drain(cport_nr);
		delay_ms(300);
	}
	
	// Audio - set high
	if (gbxcartFirmwareVersion >= 22) {
		set_mode(AUDIO_HIGH);
		delay_ms(300);
	}
	else {
		RS232_cputs(cport_nr, "HE0x02");
		RS232_SendByte(cport_nr, 0);
		RS232_drain(cport_nr);
		delay_ms(300);
	}
	
	// Set back to menu
	set_bank(0x6000, 0x00); // ROM bank 0, allow for all ROM access
	set_bank(0x2000, 0);
	set_bank(0x4000, 0x00); // RAM bank 0
	set_bank(0x0000, 0x00); // RAM not accessible
	
	gbx_set_done_led();
	gbx_cart_power_down();
	
	return 0;
}