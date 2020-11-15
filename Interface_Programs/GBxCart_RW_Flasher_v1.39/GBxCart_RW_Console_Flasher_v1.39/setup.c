/*
 GBxCart RW - Console Interface Flasher
 Version: 1.39
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 26/08/2017
 Last Modified: 5/11/2020
 License: CC-BY-NC-SA
  
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#define _XOPEN_SOURCE 600
#include <time.h>
#endif

#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)
#define RS232_PORTNR  57
#else
#define RS232_PORTNR  30
#endif

#include <stdio.h>
#include "setup.h"

// COM Port settings (default)
#include "rs232/rs232.h"
int cport_nr = 7; // /dev/ttyS7 (COM8 on windows)
int bdrate = 1000000; // 1,000,000 baud

// Common vars
uint8_t gbxcartFirmwareVersion = 0;
uint8_t gbxcartPcbVersion = 0;
uint8_t readBuffer[257];
uint8_t writeBuffer[257];

char gameTitle[17];
uint16_t cartridgeType = 0;
uint32_t currAddr = 0x0000;
uint32_t endAddr = 0x7FFF;
uint16_t romSize = 0;
uint32_t romEndAddr = 0;
uint16_t romBanks = 0;
int ramSize = 0;
uint16_t ramBanks = 0;
uint32_t ramEndAddress = 0;
int eepromSize = 0;
uint16_t eepromEndAddress = 0;
int hasFlashSave = 0;
uint8_t cartridgeMode = GB_MODE;
int flashCartType = 0;
uint8_t flashID[10];
uint32_t bytesReadPrevious = 0;
uint32_t ledStatus = 0;
uint32_t ledCountLeft = 0;
uint32_t ledCountRight = 0;
uint8_t ledSegment = 0;
uint8_t ledProgress = 0;
uint8_t ledBlinking = 0;
uint8_t mode5vOverride = 0;
int8_t detectedFlashWritingMethod = 0;
uint32_t lastAddrHash = 0;

uint8_t nintendoLogo[] = {0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
									0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
									0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E};
uint8_t nintendoLogoGBA[] = {0x24, 0xFF, 0xAE, 0x51, 0x69, 0x9A, 0xA2, 0x21, 0x3D, 0x84, 0x82, 0x0A, 0x84, 0xE4, 0x09, 0xAD, 
										0x11, 0x24, 0x8B, 0x98, 0xC0, 0x81, 0x7F, 0x21, 0xA3, 0x52, 0xBE, 0x19, 0x93, 0x09, 0xCE, 0x20,
										0x10, 0x46, 0x4A, 0x4A, 0xF8, 0x27, 0x31, 0xEC, 0x58, 0xC7, 0xE8, 0x33, 0x82, 0xE3, 0xCE, 0xBF, 
										0x85, 0xF4, 0xDF, 0x94, 0xCE, 0x4B, 0x09, 0xC1, 0x94, 0x56, 0x8A, 0xC0, 0x13, 0x72, 0xA7, 0xFC, 
										0x9F, 0x84, 0x4D, 0x73, 0xA3, 0xCA, 0x9A, 0x61, 0x58, 0x97, 0xA3, 0x27, 0xFC, 0x03, 0x98, 0x76, 
										0x23, 0x1D, 0xC7, 0x61, 0x03, 0x04, 0xAE, 0x56, 0xBF, 0x38, 0x84, 0x00, 0x40, 0xA7, 0x0E, 0xFD, 
										0xFF, 0x52, 0xFE, 0x03, 0x6F, 0x95, 0x30, 0xF1, 0x97, 0xFB, 0xC0, 0x85, 0x60, 0xD6, 0x80, 0x25, 
										0xA9, 0x63, 0xBE, 0x03, 0x01, 0x4E, 0x38, 0xE2, 0xF9, 0xA2, 0x34, 0xFF, 0xBB, 0x3E, 0x03, 0x44, 
										0x78, 0x00, 0x90, 0xCB, 0x88, 0x11, 0x3A, 0x94, 0x65, 0xC0, 0x7C, 0x63, 0x87, 0xF0, 0x3C, 0xAF, 
										0xD6, 0x25, 0xE4, 0x8B, 0x38, 0x0A, 0xAC, 0x72, 0x21, 0xD4, 0xF8, 0x07};
uint8_t flashCartList[] = { 1, 29, 30, 31, 2, 42, 3, 4, 35, 5, 6, // GB iG carts
									  0, 52, 53, 8, 32, 9, 33, 10, 11, 12, 34, 13, 14, 15, 39, 40, 38, 16, 17,  // GB carts
									  20, 27, 41, 43, 21, 22, 23, 24, 25, 26, 36, 37, 54, // GBA carts
									  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
									  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
									  44, 45, 46, 47, 48, 49, 50, 51}; 

// Read the config.ini file for the COM port to use and baud rate
void read_config(void) {
	char configFilePath[253];
	
#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)
	strncpy(configFilePath, "config.ini", 11);
#else
	strncpy(configFilePath, getenv("USERPROFILE"), 200);
	strncat(configFilePath, "\\gbxcart-config.ini", 20);
#endif

	FILE* configfile = fopen (configFilePath , "rt");
	if (configfile != NULL) {
		if (fscanf(configfile, "%d\n%d", &cport_nr, &bdrate) != 2) {
			fprintf(stderr, "Config file is corrupt\n");
		}
		else {
			cport_nr--;
		}
		fclose(configfile);
	}
	else {
		fprintf(stderr, "Config file not found\n");
	}
}

// Write the config.ini file for the COM port to use and baud rate
void write_config(void) {
	char configFilePath[253];
	
#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)
	strncpy(configFilePath, "config.ini", 11);
#else
	strncpy(configFilePath, getenv("USERPROFILE"), 200);
	strncat(configFilePath, "\\gbxcart-config.ini", 20);
#endif
	
	FILE *configfile = fopen(configFilePath, "wt");
	if (configfile != NULL) {
		fprintf(configfile, "%d\n%d\n", cport_nr+1, bdrate);
		fclose(configfile);
	}
}

// Load a file which contains the cartridge RAM settings (only needed if Erase RAM option was used, only applies to GBA games)
void load_cart_ram_info(void) {
	char titleFilename[30];
	strncpy(titleFilename, gameTitle, 20);
	strncat(titleFilename, ".si", 4);

	// Create a new file
	FILE *infoFile = fopen(titleFilename, "rt");
	if (infoFile != NULL) {
		if (fscanf(infoFile, "%d,%d,%d,", &ramSize, &eepromSize, &hasFlashSave) != 3) {
			fprintf(stderr, "Cart RAM info %s is corrupt\n", titleFilename);
		}
		fclose(infoFile);
	}
}

// Write a file which contains the cartridge RAM settings before it's wiped using Erase RAM (Only applies to GBA games)
void write_cart_ram_info(void) {
	char titleFilename[30];
	strncpy(titleFilename, gameTitle, 20);
	strncat(titleFilename, ".si", 4);
	
	// Check if file exists, if not, write the ram info
	FILE *infoFileRead = fopen(titleFilename, "rt");
	if (infoFileRead == NULL) {
		
		// Create a new file
		FILE *infoFile = fopen(titleFilename, "wt");
		if (infoFile != NULL) {
			fprintf(infoFile, "%d,%d,%d,", ramSize, eepromSize, hasFlashSave);
			fclose(infoFile);
		}
	}
	else {
		fclose(infoFileRead);
	}
}

void delay_ms(uint16_t ms) {
	#if defined (_WIN32)
		Sleep(ms);
	#else
		struct timespec ts;
		ts.tv_sec = ms / 1000;
		ts.tv_nsec = (ms * 1000000) % 1000000000;
		nanosleep(&ts, NULL);
	#endif
}

// Read one letter from stdin
char read_one_letter (void) {
	char c = getchar();
	while (getchar() != '\n' && getchar() != EOF);
	return c;
}

// Print progress
void print_progress_percent (uint32_t bytesRead, uint32_t hashNumber) {
	//printf("%i, %i\n", bytesRead, hashNumber);
	
	if ((bytesRead % hashNumber == 0) && bytesRead != 0) {
		if (hashNumber == 64) {
			printf("########");
			fflush(stdout);
		}
		else {
			printf("#");
			fflush(stdout);
		}
	}
}

// Print progress
void print_progress_percent_addr (uint32_t bytesRead, uint32_t hashNumber) {
	if (currAddr >= lastAddrHash) {
		printf("#");
		fflush(stdout);
		lastAddrHash = currAddr + (endAddr / 64); 
	}
}

// LED progress
void led_progress_percent (uint32_t bytesRead, uint32_t divideNumber) {
	if (gbxcartPcbVersion == GBXMAS) {
		if (bytesRead >= bytesReadPrevious) {
			bytesReadPrevious += divideNumber;
			
			if (ledSegment == 0) {
				ledStatus |= (1<<ledCountLeft);
				ledSegment = 1;
				ledCountLeft++;
			}
			else {
				ledSegment = 0;
				ledStatus |= (1<<(ledCountRight+14));
				ledCountRight++;
			}
			xmas_set_leds(ledStatus);
			
			if (ledBlinking <= 14) {
				ledBlinking = ledBlinking + 14;
			}
			else {
				ledBlinking = ledBlinking - 13;
			}
			
			if (ledProgress < 27) {
				xmas_blink_led(ledBlinking);
			}
			ledProgress++;
		}
	}
}

void xmas_set_leds (uint32_t value) {
	set_mode('0');
	delay_ms(5);
	set_number(XMAS_VALUE, XMAS_LEDS);
	delay_ms(5);
	set_number(value, 'L');
	delay_ms(5);
}

void xmas_blink_led (uint8_t value) {
	set_mode('0');
	delay_ms(5);
	set_number(XMAS_VALUE, XMAS_LEDS);
	delay_ms(5);
	set_mode('B');
	set_mode(value);
	delay_ms(5);
}

void xmas_reset_values (void) {
	ledStatus = 0;
	ledCountLeft = 0;
	ledCountRight = 0;
	ledSegment = 0;
	ledProgress = 0;
	ledBlinking = 0;
	bytesReadPrevious = 0;
}

// Turn on idle timer
void xmas_idle_on (void) {
	if (gbxcartPcbVersion == GBXMAS) {
		set_mode('0'); // Only send when writing
		delay_ms(5);
		set_number(XMAS_VALUE, XMAS_LEDS);
		delay_ms(5);
		set_mode('I');
		delay_ms(5);
	}
}

// Turn off idle timer
void xmas_idle_off (void) {
	set_mode('0'); // Only send when writing
	delay_ms(5);
	set_number(XMAS_VALUE, XMAS_LEDS);
	delay_ms(5);
	set_mode('O');
	delay_ms(5);
}

void xmas_chip_erase_animation (void) {
	if (gbxcartPcbVersion == GBXMAS) {
		set_mode('0');
		delay_ms(5);
		set_number(XMAS_VALUE, XMAS_LEDS);
		delay_ms(5);
		set_mode('E');
		delay_ms(5);
	}
}

void xmas_wake_up (void) {
	if (gbxcartPcbVersion == GBXMAS) {
		set_mode('!');
		delay_ms(50);  // Wait for ATmega169 to WDT reset if in idle mode
	}
}

void xmas_setup (uint32_t progressNumber) {
	if (gbxcartPcbVersion == GBXMAS) {
		xmas_wake_up();
		xmas_reset_values();
		xmas_idle_off();
		xmas_set_leds(0);
		ledBlinking = 1;
		xmas_blink_led(ledBlinking);
		bytesReadPrevious = progressNumber;
	}
}

// Wait for a "1" acknowledgement from the ATmega
void com_wait_for_ack (void) {
	uint8_t buffer[2];
	uint8_t rxBytes = 0;
	uint32_t timeout = 0;
	
	while (rxBytes < 1) {
		rxBytes = RS232_PollComport(cport_nr, buffer, 1);
		
		if (rxBytes > 0) {
			if (buffer[0] == '1') {
				break;
			}
			rxBytes = 0;
		}
		#if defined(__APPLE__)
		else {
			delay_ms(5);
			timeout++;
			if (timeout >= 250) {
				printf("\n\nWriting has timed out. Please unplug GBxCart RW, re-seat the cartridge and try again.\n");
				read_one_letter();
				exit(1);
			}
		}
		#else
		else {
			timeout++;
			if (timeout >= 200000) {
				printf("\n\nWriting has timed out. Please unplug GBxCart RW, re-seat the cartridge and try again.\n");
				read_one_letter();
				exit(1);
			}
		}
		#endif
	}
}

// Stop reading blocks of data
void com_read_stop(void) {
	RS232_cputs(cport_nr, "0"); // Stop read
	RS232_drain(cport_nr);
	if (gbxcartPcbVersion == GBXMAS) { // Small delay as GBXMAS intercepts these commands
		delay_ms(1);
	}
}

// Continue reading the next block of data
void com_read_cont(void) {
	RS232_cputs(cport_nr, "1"); // Continue read
	RS232_drain(cport_nr);
	if (gbxcartPcbVersion == GBXMAS) { // Small delay as GBXMAS intercepts these commands
		delay_ms(1);
	}
}

// Test opening the COM port,if can't be open, try autodetecting device on other COM ports
uint8_t com_test_port(void) {
	// Check if COM port responds correctly
	if (RS232_OpenComport(cport_nr, bdrate, "8N1") == 0) { // Port opened
		set_mode('0');
		uint8_t cartridgeMode = request_value(CART_MODE);
		
		// Responded ok
		if (cartridgeMode == GB_MODE || cartridgeMode == GBA_MODE) {
			return 1;
		}
	}
	
	// If port didn't get opened or responded wrong
	for (uint8_t x = 0; x <= RS232_PORTNR; x++) {
		if (RS232_OpenComport(x, bdrate, "8N1") == 0) { // Port opened
			cport_nr = x;
			
			// See if device responds correctly
			set_mode('0');
			uint8_t cartridgeMode = request_value(CART_MODE);
			
			// Responded ok, save the new port number
			if (cartridgeMode == GB_MODE || cartridgeMode == GBA_MODE) {
				write_config();
				return 1;
			}
			else {
				RS232_CloseComport(x);
			}
		}
	}
	
	return 0;
}

// Read 1 to 256 bytes from the COM port and write it to the global read buffer or to a file if specified. 
// When polling the com port it return less than the bytes we want, keep polling and wait until we have all bytes requested. 
// We expect no more than 256 bytes.
uint8_t com_read_bytes (FILE *file, int count) {
	uint8_t buffer[257];
	uint16_t rxBytes = 0;
	uint16_t readBytes = 0;
	
	#if defined(__APPLE__)
	uint8_t timeout = 0;
	#else
	uint16_t timeout = 0;
	#endif
	
	while (readBytes < count) {
		rxBytes = RS232_PollComport(cport_nr, buffer, 64);
		
		if (rxBytes > 0) {
			buffer[rxBytes] = 0;
			
			if (file == NULL) {
				memcpy(&readBuffer[readBytes], buffer, rxBytes);
			}
			else {
				fwrite(buffer, 1, rxBytes, file);
			}
			
			readBytes += rxBytes;
		}
		#if defined(__APPLE__)
		else {
			delay_ms(5);
			timeout++;
			if (timeout >= 50) {
				return readBytes;
			}
		}
		#else
		else {
			timeout++;
			if (timeout >= 20000) {
				return readBytes;
			}
		}
		#endif
	}
	
	return readBytes;
}

// Read 1-256 bytes from the file (or buffer) and write it the COM port with the command given
void com_write_bytes_from_file(uint8_t command, FILE *file, int count) {
	uint8_t buffer[257];
	buffer[0] = command;

	if (file == NULL) {
		memcpy(&buffer[1], writeBuffer, count);
	}
	else {
		fread(&buffer[1], 1, count, file);
	}
	
	RS232_SendBuf(cport_nr, buffer, (count + 1)); // command + 1-256 bytes
	RS232_drain(cport_nr);
}

// Read 1-256 bytes from the file (or buffer) and write it the COM port with the command given if the data isn't all 0xFF
uint8_t com_write_bytes_from_file_skip_FFs(uint8_t command, FILE *file, int count) {
	uint8_t buffer[257];
	buffer[0] = command;

	if (file == NULL) {
		memcpy(&buffer[1], writeBuffer, count);
	}
	else {
		fread(&buffer[1], 1, count, file);
	}
	
	// 0xFF check
	uint8_t allFF = 1;
	for (int16_t x = 1; x <= count; x++) {
		if (buffer[x] != 0xFF) {
			allFF = 0;
			break;
		}
	}
	
	if (allFF == 0) {
		RS232_SendBuf(cport_nr, buffer, (count + 1)); // command + 1-256 bytes
		RS232_drain(cport_nr);
		
		return 1;
	}
	
	return 0;
}

// Send a single command byte
void set_mode (char command) {
	char modeString[5];
	sprintf(modeString, "%c", command);
	
	RS232_cputs(cport_nr, modeString);
	RS232_drain(cport_nr);
	
	#if defined(__APPLE__)
	delay_ms(5);
	#endif
}

// Send a command with a hex number and a null terminator byte
void set_number (uint32_t number, uint8_t command) {
	char numberString[20];
	sprintf(numberString, "%c%x", command, number);
	
	RS232_cputs(cport_nr, numberString);
	RS232_SendByte(cport_nr, 0);
	RS232_drain(cport_nr);
	
	#if defined(__APPLE__)
	delay_ms(5);
	#endif
}

// Send a single hex byte and wait for ACK back
void send_hex_wait_ack (uint16_t hex) {
	char tempString[15];
	sprintf(tempString, "%x", hex);
	RS232_cputs(cport_nr, tempString);
	RS232_SendByte(cport_nr, 0);
	RS232_drain(cport_nr);
	delay_ms(5);
	com_wait_for_ack();
}


// Read the cartridge mode
uint8_t read_cartridge_mode (void) {
	set_mode(CART_MODE);
	
	uint8_t buffer[2];
	uint8_t rxBytes = 0;
	while (rxBytes < 1) {
		rxBytes = RS232_PollComport(cport_nr, buffer, 1);
		
		if (rxBytes > 0) {
			return buffer[0];
		}
	}
	
	return 0;
}

// Send 1 byte and read 1 byte
uint8_t request_value (uint8_t command) {
	set_mode(command);
	
	uint8_t buffer[2];
	uint8_t rxBytes = 0;
	uint8_t timeoutCounter = 0;
	
	while (rxBytes < 1) {
		rxBytes = RS232_PollComport(cport_nr, buffer, 1);
		
		if (rxBytes > 0) {
			return buffer[0];
		}
		
		delay_ms(10);
		timeoutCounter++;
		if (timeoutCounter >= 25) { // After 250ms, timeout
			return 0;
		}
	}
	
	return 0;
}



// ****** Gameboy / Gameboy Colour functions ******

// Set bank for ROM/RAM switching, send address first and then bank number
void set_bank (uint16_t address, uint8_t bank) {
	char AddrString[15];
	sprintf(AddrString, "%c%x", SET_BANK, address);
	RS232_cputs(cport_nr, AddrString);
	RS232_SendByte(cport_nr, 0);
	RS232_drain(cport_nr);
	delay_ms(5);
	
	char bankString[15];
	sprintf(bankString, "%c%d", SET_BANK, bank);
	RS232_cputs(cport_nr, bankString);
	RS232_SendByte(cport_nr, 0);
	RS232_drain(cport_nr);
	delay_ms(5);
}

// MBC2 Fix (unknown why this fixes reading the ram, maybe has to read ROM before RAM?)
// Read 64 bytes of ROM, (really only 1 byte is required)
void mbc2_fix (void) {
	set_number(0x0000, SET_START_ADDRESS);
	set_mode(READ_ROM_RAM);
	
	uint16_t rxBytes = 0;
	uint8_t byteCount = 0;
	uint8_t tempBuffer[64];
	while (byteCount < 64) {
		rxBytes = RS232_PollComport(cport_nr, tempBuffer, 64);
		
		if (rxBytes > 0) {
			byteCount += rxBytes;
		}
	}
	com_read_stop();
}

// Read the first 384 bytes of ROM and process the Gameboy header information
void read_gb_header (void) {
	currAddr = 0x0000;
	endAddr = 0x0180;
	
	set_number(currAddr, SET_START_ADDRESS);
	set_mode(READ_ROM_RAM);
	
	uint8_t startRomBuffer[385];
	while (currAddr < endAddr) {
		com_read_bytes(READ_BUFFER, 64);
		memcpy(&startRomBuffer[currAddr], readBuffer, 64);
		currAddr += 64;
		
		if (currAddr < endAddr) {
			com_read_cont();
		}
	}
	com_read_stop();
	
	// Blank out game title
	for (uint8_t b = 0; b < 16; b++) {
		gameTitle[b] = 0;
	}
	// Read cartridge title and check for non-printable text
	for (uint16_t titleAddress = 0x0134; titleAddress <= 0x143; titleAddress++) {
		char headerChar = startRomBuffer[titleAddress];
		if ((headerChar >= 0x30 && headerChar <= 0x57) || // 0-9
			 (headerChar >= 0x41 && headerChar <= 0x5A) || // A-Z
			 (headerChar >= 0x61 && headerChar <= 0x7A) || // a-z
			 (headerChar == 0x2E) || // .
			 (headerChar == 0x5F) || // _
			 (headerChar == 0x20)) { // Space
			gameTitle[(titleAddress-0x0134)] = headerChar;
		}
		else {
			gameTitle[(titleAddress-0x0134)] = '\0';
			break;
		}
	}
	printf ("Game title: %s\n", gameTitle);
	
	cartridgeType = startRomBuffer[0x0147];
	romSize = startRomBuffer[0x0148];
	ramSize = startRomBuffer[0x0149];
	
	// ROM banks
	romBanks = 2; // Default 32K
	if (romSize >= 1) { // Calculate rom size
		romBanks = 2 << romSize;
	}
	
	// RAM banks
	ramBanks = 0; // Default 0K RAM
	if (cartridgeType == 6) { ramBanks = 1; }
	if (ramSize == 2) { ramBanks = 1; }
	if (ramSize == 3) { ramBanks = 4; }
	if (ramSize == 4) { ramBanks = 16; }
	if (ramSize == 5) { ramBanks = 8; }
	
	// RAM end address
	if (cartridgeType == 6) { ramEndAddress = 0xA1FF; } // MBC2 512bytes (nibbles)
	if (ramSize == 1) { ramEndAddress = 0xA7FF; } // 2K RAM
	if (ramSize > 1) { ramEndAddress = 0xBFFF; } // 8K RAM
	
	printf ("MBC type: ");
	switch (cartridgeType) {
		case 0: printf ("ROM ONLY\n"); break;
		case 1: printf ("MBC1\n"); break;
		case 2: printf ("MBC1+RAM\n"); break;
		case 3: printf ("MBC1+RAM+BATTERY\n"); break;
		case 5: printf ("MBC2\n"); break;
		case 6: printf ("MBC2+BATTERY\n"); break;
		case 8: printf ("ROM+RAM\n"); break;
		case 9: printf ("ROM ONLY\n"); break;
		case 11: printf ("MMM01\n"); break;
		case 12: printf ("MMM01+RAM\n"); break;
		case 13: printf ("MMM01+RAM+BATTERY\n"); break;
		case 15: printf ("MBC3+TIMER+BATTERY\n"); break;
		case 16: printf ("MBC3+TIMER+RAM+BATTERY\n"); break;
		case 17: printf ("MBC3\n"); break;
		case 18: printf ("MBC3+RAM\n"); break;
		case 19: printf ("MBC3+RAM+BATTERY\n"); break;
		case 21: printf ("MBC4\n"); break;
		case 22: printf ("MBC4+RAM\n"); break;
		case 23: printf ("MBC4+RAM+BATTERY\n"); break;
		case 25: printf ("MBC5\n"); break;
		case 26: printf ("MBC5+RAM\n"); break;
		case 27: printf ("MBC5+RAM+BATTERY\n"); break;
		case 28: printf ("MBC5+RUMBLE\n"); break;
		case 29: printf ("MBC5+RUMBLE+RAM\n"); break;
		case 30: printf ("MBC5+RUMBLE+RAM+BATTERY\n"); break;
		case 252: printf("Gameboy Camera\n"); break;
		default: printf ("Not found\n");
	}
	
	printf ("ROM size: ");
	switch (romSize) {
		case 0: printf ("32KByte (no ROM banking)\n"); break;
		case 1: printf ("64KByte (4 banks)\n"); break;
		case 2: printf ("128KByte (8 banks)\n"); break;
		case 3: printf ("256KByte (16 banks)\n"); break;
		case 4: printf ("512KByte (32 banks)\n"); break;
		case 5: 
			if (cartridgeType == 1 || cartridgeType == 2 || cartridgeType == 3) {
				printf ("1MByte (63 banks)\n");
			}
			else {
				printf ("1MByte (64 banks)\n");
			}
			break;
		case 6: 
			if (cartridgeType == 1 || cartridgeType == 2 || cartridgeType == 3) {
				printf ("2MByte (125 banks)\n");
			}
			else {
				printf ("2MByte (128 banks)\n");
			}
			break;
		case 7: printf ("4MByte (256 banks)\n"); break;
		case 82: printf ("1.1MByte (72 banks)\n"); break;
		case 83: printf ("1.2MByte (80 banks)\n"); break;
		case 84: printf ("1.5MByte (96 banks)\n"); break;
		default: printf ("Not found\n");
	}
	
	printf ("RAM size: ");
	switch (ramSize) {
		case 0: 
			if (cartridgeType == 6) {
				printf ("512 bytes (nibbles)\n");
			}
			else {
				printf ("None\n");
			}
			break;
		case 1: printf ("2 KBytes\n"); break;
		case 2: printf ("8 KBytes\n"); break;
		case 3: printf ("32 KBytes (4 banks of 8Kbytes)\n"); break;
		case 4: printf ("128 KBytes (16 banks of 8Kbytes)\n"); break;
		default: printf ("Not found\n");
	}
	
	// Nintendo Logo Check
	uint8_t logoCheck = 1;
	for (uint16_t logoAddress = 0x0104; logoAddress <= 0x133; logoAddress++) {
		if (nintendoLogo[(logoAddress-0x0104)] != startRomBuffer[logoAddress]) {
			logoCheck = 0;
			break;
		}
	}
	printf ("Logo check: ");
	if (logoCheck == 1) {
		printf ("OK\n");
	}
	else {
		printf ("Failed\n");
	}
}


// ****** Gameboy Advance functions ****** 

// Check the rom size by reading 64 bytes from different addresses and checking if they are all 0x00. There can be some ROMs 
// that do have valid 0x00 data, so we check 32 different addresses in a 4MB chunk, if 30 or more are all 0x00 then we've reached the end.
uint8_t gba_check_rom_size (void) {
	uint32_t fourMbBoundary = 0x3FFFC0;
	uint32_t currAddr = 0x1FFC0;
	uint8_t romZeroTotal = 0;
	uint8_t romSize = 0;
	
	// Loop until 32MB
	for (uint16_t x = 0; x < 512; x++) {
		set_number(currAddr / 2, SET_START_ADDRESS); // Divide current address by 2 as we only increment it by 1 after 2 bytes have been read on the ATmega side
		set_mode(GBA_READ_ROM);
		
		com_read_bytes(READ_BUFFER, 64);
		com_read_stop();
		
		// Check how many 0x00 are found in the 64 bytes
		uint8_t zeroCheck = 0;
		for (uint16_t c = 0; c < 64; c++) {
			if (readBuffer[c] == 0) {
				zeroCheck++;
			}
		}
		if (zeroCheck >= 64) { // All 0x00's found, set 1 more to the ROM's zero total count
			romZeroTotal++;
		}
		
		// After a 4MB chunk, we check the zeroTotal, if more than 30 then we have reached the end, otherwise reset romZeroTotal
		if (currAddr % fourMbBoundary == 0 || currAddr % fourMbBoundary < 512) {
			if (romZeroTotal >= 30) {
				break;
			}
			
			romZeroTotal = 0;
			romSize += 4;
		}
		
		currAddr += 0x20000; // Increment address by 131K
		
		if (x % 10 == 0) {
			printf(".");
		}
	}
	
	return romSize;
}

// Used before we write to RAM as we need to check if we have an SRAM or Flash. 
// Write 1 byte to 0x00 on the SRAM/Flash save, if we read it back successfully then we know SRAM is present, then we write
// the original byte back to how it was. This can be a destructive process to the first byte, if anything goes wrong the user
// could lose the first byte, so we only do this check when writing a save back to the SRAM/Flash.
uint8_t gba_test_sram_flash_write (void) {
	printf("Testing for SRAM or Flash presence... ");
	
	// Save the 1 byte first to buffer
	uint8_t saveBuffer[65];
	set_number(0x0000, SET_START_ADDRESS);
	set_mode(GBA_READ_SRAM);
	com_read_bytes(READ_BUFFER, 64);
	memcpy(&saveBuffer, readBuffer, 64);
	com_read_stop();
	
	// Check to see if the first byte matches our test byte (1 in 255 chance), if so, use the another test byte
	uint8_t testNumber = 0x91;
	if (saveBuffer[0] == testNumber) {
		testNumber = 0xA6;
	}
	
	// Write 1 byte
	set_number(0x0000, SET_START_ADDRESS);
	uint8_t tempBuffer[3];
	tempBuffer[0] = GBA_WRITE_ONE_BYTE_SRAM; // Set write sram 1 byte mode
	tempBuffer[1] = testNumber;
	RS232_SendBuf(cport_nr, tempBuffer, 2);
	RS232_drain(cport_nr);
	com_wait_for_ack();
	
	// Read back the 1 byte
	uint8_t readBackBuffer[65];
	set_number(0x0000, SET_START_ADDRESS);
	set_mode(GBA_READ_SRAM);
	com_read_bytes(READ_BUFFER, 64);
	memcpy(&readBackBuffer, readBuffer, 64);
	com_read_stop();
	
	// Verify
	if (readBackBuffer[0] == testNumber) {
		printf("SRAM found\n");
		
		// Write the byte back to how it was
		set_number(0x0000, SET_START_ADDRESS);
		tempBuffer[0] = GBA_WRITE_ONE_BYTE_SRAM; // Set write sram 1 byte mode
		tempBuffer[1] = saveBuffer[0];
		RS232_SendBuf(cport_nr, tempBuffer, 2);
		RS232_drain(cport_nr);
		com_wait_for_ack();
		
		return NO_FLASH;
	}
	else { // Flash likely present, test by reading the flash ID
		printf("Flash found\n");
		
		set_mode(GBA_FLASH_READ_ID); // Read Flash ID and exit Flash ID mode
		
		uint8_t idBuffer[2];
		com_read_bytes(READ_BUFFER, 2);
		memcpy(&idBuffer, readBuffer, 2);
		
		// Some particular flash memories don't seem to exit the ID mode properly, check if that's the case by reading the first byte 
		// from 0x00h to see if it matches any Flash IDs. If so, exit the ID mode a different way and slowly.
		
		// Read from 0x00
		set_number(0x00, SET_START_ADDRESS);
		set_mode(GBA_READ_SRAM);
		com_read_bytes(READ_BUFFER, 64);
		memcpy(&readBackBuffer, readBuffer, 64);
		com_read_stop();
		
		// Exit the ID mode a different way and slowly
		if (readBackBuffer[0] == 0x1F || readBackBuffer[0] == 0xBF || readBackBuffer[0] == 0xC2 ||
			 readBackBuffer[0] == 0x32 || readBackBuffer[0] == 0x62) {
			
			RS232_cputs(cport_nr, "G"); // Set Gameboy mode
			RS232_drain(cport_nr);
			delay_ms(5);
			
			RS232_cputs(cport_nr, "M0"); // Disable CS/RD/WR/CS2-RST from going high after each command
			RS232_drain(cport_nr);
			delay_ms(5);
			
			RS232_cputs(cport_nr, "OC0xFF"); // Set output lines
			RS232_SendByte(cport_nr, 0);
			RS232_drain(cport_nr);
			delay_ms(5);
			
			RS232_cputs(cport_nr, "HC0xF0"); // Set byte
			RS232_SendByte(cport_nr, 0);
			RS232_drain(cport_nr);
			delay_ms(5);
			
			// V1.1 PCB
			if (gbxcartPcbVersion == PCB_1_1) {
				RS232_cputs(cport_nr, "LD0x40"); // WE low
				RS232_SendByte(cport_nr, 0);
				RS232_drain(cport_nr);
				delay_ms(5);
				
				RS232_cputs(cport_nr, "LE0x04"); // CS2 low
				RS232_SendByte(cport_nr, 0);
				RS232_drain(cport_nr);
				delay_ms(5);
				
				RS232_cputs(cport_nr, "HD0x40"); // WE high
				RS232_SendByte(cport_nr, 0);
				RS232_drain(cport_nr);
				delay_ms(5);
				
				RS232_cputs(cport_nr, "HE0x04"); // CS2 high
				RS232_SendByte(cport_nr, 0);
				RS232_drain(cport_nr);
				delay_ms(5);
			}
			else { // V1.0 PCB
				RS232_cputs(cport_nr, "LD0x90"); // WR, CS2 low
				RS232_SendByte(cport_nr, 0);
				RS232_drain(cport_nr);
				delay_ms(5);
				
				RS232_cputs(cport_nr, "HD0x90"); // WR, CS2 high
				RS232_SendByte(cport_nr, 0);
				RS232_drain(cport_nr);
				delay_ms(5);
			}
			
			delay_ms(50);
			RS232_cputs(cport_nr, "M1"); // Enable CS/RD/WR/CS2-RST goes high after each command
			RS232_drain(cport_nr);
		}
		
		// Check if it's Atmel Flash
		if (idBuffer[0] == 0x1F) {
			return FLASH_FOUND_ATMEL;
		}
		// Check other manufacturers 
		else if (idBuffer[0] == 0xBF || idBuffer[0] == 0xC2 || 
					idBuffer[0] == 0x32 || idBuffer[0] == 0x62) {
			return FLASH_FOUND;
		}
		
		return NO_FLASH;
	}
}

// Check if SRAM/Flash is present and test the size. 
// When a 256Kbit SRAM is read past 256Kbit, the address is loops around, there are some times where the bytes don't all 
// match up 100%, it's like 90% so be a bit lenient. A cartridge that doesn't have an SRAM/Flash reads all 0x00's.
uint8_t gba_check_sram_flash (void) {
	uint16_t currAddr = 0x0000;
	uint16_t zeroTotal = 0;
	hasFlashSave = NOT_CHECKED;
	
	// Special check for certain games
	if (strncmp(gameTitle, "CHUCHU ROCKE", 12) == 0 || strncmp(gameTitle, "CHUCHUROCKET", 12) == 0) { // Chu-Chu Rocket!
		return SRAM_FLASH_512KBIT;
	}
	
	// Pre-read SRAM/Flash (if the cart has an EEPROM, sometimes D0-D7 come back with random data in the first 64 bytes read)
	set_number(currAddr, SET_START_ADDRESS);
	set_mode(GBA_READ_SRAM);
	com_read_bytes(READ_BUFFER, 64);
	com_read_stop();
	
	// Test if SRAM is present, read 32 sections of RAM (64 bytes each)
	for (uint8_t x = 0; x < 32; x++) {
		set_number(currAddr, SET_START_ADDRESS);
		set_mode(GBA_READ_SRAM);
		
		com_read_bytes(READ_BUFFER, 64);
		com_read_stop();
		
		// Check for 0x00 byte
		for (uint8_t c = 0; c < 64; c++) {
			if (readBuffer[c] == 0) {
				zeroTotal++;
			}
		}
		
		currAddr += 0x400;
		
		// Progress
		if (x % 10 == 0) {
			printf(".");
		}
	}
	
	if (zeroTotal >= 2000) { // Looks like no SRAM or Flash present, lets do a more thorough check
		// Set start and end address
		currAddr = 0x0000;
		endAddr = 32768;
		set_number(currAddr, SET_START_ADDRESS);
		set_mode(GBA_READ_SRAM);
		zeroTotal = 0;
		
		// Read data
		while (currAddr < endAddr) {
			com_read_bytes(READ_BUFFER, 64);
			currAddr += 64;
			
			// Check for 0x00 byte
			for (uint8_t c = 0; c < 64; c++) {
				if (readBuffer[c] == 0) {
					zeroTotal++;
				}
			}
			
			// Request 64 bytes more
			if (currAddr < endAddr) {
				com_read_cont();
			}
		}
		com_read_stop();
		
		if (zeroTotal == 32768) {
			return 0;
		}
	}
	
	// Calculate size by checking different addresses (Test 256Kbit or 512Kbit)
	uint16_t duplicateCount = 0;
	char firstBuffer[65];
	char secondBuffer[65];
	for (uint8_t x = 0; x < 32; x++) {
		set_number((uint32_t) (x * 0x400), SET_START_ADDRESS);
		set_mode(GBA_READ_SRAM);
		com_read_bytes(READ_BUFFER, 64);
		memcpy(&firstBuffer, readBuffer, 64);
		com_read_stop();
		
		set_number((uint32_t) (x * 0x400) + 0x8000, SET_START_ADDRESS);
		set_mode(GBA_READ_SRAM);
		com_read_bytes(READ_BUFFER, 64);
		memcpy(&secondBuffer, readBuffer, 64);
		com_read_stop();
		
		// Compare
		for (uint8_t x = 0; x < 64; x++) {
			if (firstBuffer[x] == secondBuffer[x]) {
				duplicateCount++;
			}
		}
		
		// Progress
		if (x % 10 == 0) {
			printf(".");
		}
	}
	if (duplicateCount >= 2000) {
		return SRAM_FLASH_256KBIT;
	}
	
	
	// Check if it's SRAM or Flash at this stage, maximum for SRAM is 512Kbit
	printf("\n");
	hasFlashSave = gba_test_sram_flash_write();
	if (hasFlashSave == NO_FLASH) {
		return SRAM_FLASH_512KBIT;
	}
	
	// Test 512Kbit or 1Mbit Flash, read first 64 bytes on bank 0 then bank 1 and compare
	else {
		duplicateCount = 0;
		
		printf("Testing for 512Kbit or 1Mbit Flash... ");
		for (uint8_t x = 0; x < 32; x++) {
			// Read bank 0
			set_number((uint32_t) (x * 0x400), SET_START_ADDRESS);
			set_mode(GBA_READ_SRAM);
			com_read_bytes(READ_BUFFER, 64);
			memcpy(&firstBuffer, readBuffer, 64);
			com_read_stop();
			
			// Read bank 1
			set_number(1, GBA_FLASH_SET_BANK); // Set bank 1
			
			set_number((uint32_t) (x * 0x400), SET_START_ADDRESS);
			set_mode(GBA_READ_SRAM);
			com_read_bytes(READ_BUFFER, 64);
			memcpy(&secondBuffer, readBuffer, 64);
			com_read_stop();
			
			set_number(0, GBA_FLASH_SET_BANK); // Set back to bank 0
			
			// Compare
			for (uint8_t x = 0; x < 64; x++) {
				if (firstBuffer[x] == secondBuffer[x]) {
					duplicateCount++;
				}
			}
		}
		
		// If bank 0 and 1 are duplicated, then it's 512Kbit Flash
		if (duplicateCount >= 2000) {
			printf("512Kbit\n");
			return SRAM_FLASH_512KBIT;
		}
		else {
			printf("1Mbit\n");
			return SRAM_FLASH_1MBIT;
		}
	}
}

// Erase 4K sector on flash on sector address
void flash_4k_sector_erase (uint8_t sector) {
	set_number(sector, GBA_FLASH_4K_SECTOR_ERASE);
}

// Check if an EEPROM is present and test the size. A 4Kbit EEPROM when accessed like a 64Kbit EEPROM sends the first 8 bytes over
// and over again. A cartridge that doesn't have an EEPROM reads all 0x00 or 0xFF.
uint8_t gba_check_eeprom (void) {
	set_number(EEPROM_64KBIT, GBA_SET_EEPROM_SIZE); // Set 64Kbit size
	
	// Set start and end address
	uint16_t currAddr = 0x000;
	uint16_t endAddr = 0x200;
	set_number(currAddr, SET_START_ADDRESS);
	set_mode(GBA_READ_EEPROM);
	
	// Read EEPROM
	uint16_t repeatedCount = 0;
	uint16_t zeroTotal = 0;
	uint8_t firstEightCheck[8];
	while (currAddr < endAddr) {
		com_read_bytes(READ_BUFFER, 8);
		
		if (currAddr == 0) { // Copy the first 8 bytes to check other readings against them
			memcpy(&firstEightCheck, readBuffer, 8);
		}
		else { // Check the 8 bytes for repeats
			for (uint8_t x = 0; x < 8; x++) {
				if (firstEightCheck[x] == readBuffer[x]) {
					repeatedCount++;
				}
			}
		}
		
		// Check for 0x00 or 0xFF bytes
		for (uint8_t x = 0; x < 8; x++) { 
			if (readBuffer[x] == 0 || readBuffer[x] == 0xFF) {
				zeroTotal++;
			}
		}
		
		currAddr += 8;
		
		// Request 8 bytes more
		if (currAddr < endAddr) {
			com_read_cont();
		}
	}
	com_read_stop();
	
	if (zeroTotal >= 512) { // Blank, likely no EEPROM
		return EEPROM_NONE;
	}
	if (repeatedCount >= 300) { // Likely a 4K EEPROM is present
		return EEPROM_4KBIT;
	}
	else {
		// Additional check for EEPROMs which seem to allow 4Kbit or 64Kbit reads without any issues
		// Check to see if 4Kbit data is repeated in 64Kbit EEPROM mode, if so, it's a 4Kbit EEPROM
		
		// Read first 512 bytes
		currAddr = 0x000;
		endAddr = 0x200;
		set_number(currAddr, SET_START_ADDRESS);
		set_mode(GBA_READ_EEPROM);
		
		uint8_t eepromFirstBuffer[0x200];
		while (currAddr < endAddr) {
			com_read_bytes(READ_BUFFER, 8);
			memcpy(&eepromFirstBuffer[currAddr], readBuffer, 8);
			
			currAddr += 8;
			
			// Request 8 bytes more
			if (currAddr < endAddr) {
				com_read_cont();
			}
		}
		
		// Read second 512 bytes
		endAddr = 0x400;
		com_read_cont();
		
		uint8_t eepromSecondBuffer[0x200];
		while (currAddr < endAddr) {
			com_read_bytes(READ_BUFFER, 8);
			memcpy(&eepromSecondBuffer[currAddr-0x200], readBuffer, 8);
			
			currAddr += 8;
			
			// Request 8 bytes more
			if (currAddr < endAddr) {
				com_read_cont();
			}
		}
		
		// Compare 512 bytes
		repeatedCount = 0;
		for (uint16_t c = 0; c < 0x200; c++) {
			if (eepromFirstBuffer[c] == eepromSecondBuffer[c]) {
				repeatedCount++;
			}
		}
		com_read_stop();
		
		if (repeatedCount >= 512) {
			return EEPROM_4KBIT; 
		}
		
		return EEPROM_64KBIT; 
	}	
}

// Read GBA game title (used for reading title when ROM mapping)
void gba_read_gametitle(void) {
	currAddr = 0x0000;
	endAddr = 0x00BF;
	set_number(currAddr, SET_START_ADDRESS);
	set_mode(GBA_READ_ROM);
	
	uint8_t startRomBuffer[385];
	while (currAddr < endAddr) {
		com_read_bytes(READ_BUFFER, 64);
		memcpy(&startRomBuffer[currAddr], readBuffer, 64);
		currAddr += 64;
		
		if (currAddr < endAddr) {
			com_read_cont();
		}
	}
	com_read_stop();
	
	// Blank out game title
	for (uint8_t b = 0; b < 16; b++) {
		gameTitle[b] = 0;
	}
	// Read cartridge title and check for non-printable text
	for (uint16_t titleAddress = 0xA0; titleAddress <= 0xAB; titleAddress++) {
		char headerChar = startRomBuffer[titleAddress];
		if ((headerChar >= 0x30 && headerChar <= 0x57) || // 0-9
			 (headerChar >= 0x41 && headerChar <= 0x5A) || // A-Z
			 (headerChar >= 0x61 && headerChar <= 0x7A) || // a-z
			 (headerChar == 0x2E) || // .
			 (headerChar == 0x5F) || // _
			 (headerChar == 0x20)) { // Space
			gameTitle[(titleAddress-0xA0)] = headerChar;
		}
		else {
			gameTitle[(titleAddress-0xA0)] = '\0';
			break;
		}
	}
}

// Read the first 192 bytes of ROM, read the title, check and test for ROM, SRAM, EEPROM and Flash
void read_gba_header (void) {
	currAddr = 0x0000;
	endAddr = 0x00BF;
	set_number(currAddr, SET_START_ADDRESS);
	set_mode(GBA_READ_ROM);
	
	uint8_t startRomBuffer[385];
	while (currAddr < endAddr) {
		com_read_bytes(READ_BUFFER, 64);
		memcpy(&startRomBuffer[currAddr], readBuffer, 64);
		currAddr += 64;
		
		if (currAddr < endAddr) {
			com_read_cont();
		}
	}
	com_read_stop();
	
	// Blank out game title
	for (uint8_t b = 0; b < 16; b++) {
		gameTitle[b] = 0;
	}
	// Read cartridge title and check for non-printable text
	for (uint16_t titleAddress = 0xA0; titleAddress <= 0xAB; titleAddress++) {
		char headerChar = startRomBuffer[titleAddress];
		if ((headerChar >= 0x30 && headerChar <= 0x57) || // 0-9
			 (headerChar >= 0x41 && headerChar <= 0x5A) || // A-Z
			 (headerChar >= 0x61 && headerChar <= 0x7A) || // a-z
			 (headerChar == 0x2E) || // .
			 (headerChar == 0x5F) || // _
			 (headerChar == 0x20)) { // Space
			gameTitle[(titleAddress-0xA0)] = headerChar;
		}
		else {
			gameTitle[(titleAddress-0xA0)] = '\0';
			break;
		}
	}
	printf ("Game title: %s\n", gameTitle);
	
	
	// Nintendo Logo Check
	uint8_t logoCheck = 1;
	for (uint16_t logoAddress = 0x04; logoAddress <= 0x9F; logoAddress++) {
		if (nintendoLogoGBA[(logoAddress-0x04)] != startRomBuffer[logoAddress]) {
			logoCheck = 0;
			break;
		}
	}
	printf ("Logo check: ");
	if (logoCheck == 1) {
		printf ("OK\n");
	}
	else {
		printf ("Failed\n");
	}
	
	
	// ROM size
	printf ("Calculating ROM size");
	romSize = gba_check_rom_size();
	
	// EEPROM check
	printf ("\nChecking for EEPROM");
	eepromSize = gba_check_eeprom();
	
	// SRAM/Flash check/size, if no EEPROM present
	if (eepromSize == 0) {
		printf ("\nCalculating SRAM/Flash size");
		ramSize = gba_check_sram_flash();
	}
	else {
		ramSize = 0;
	}
	
	// If file exists, we know the ram has been erased before, so read memory info from this file
	load_cart_ram_info();
	
	// Print out
	printf ("\nROM size: %iMByte\n", romSize);
	romEndAddr = ((1024 * 1024) * romSize);
	
	if (hasFlashSave >= 2) {
		printf("Flash size: ");
	}
	else if (hasFlashSave == NO_FLASH) {
		printf("SRAM size: ");
	}
	else {
		printf("SRAM/Flash size: ");
	}
	
	if (ramSize == 0) {
		ramEndAddress = 0;
		printf ("None\n");
	}
	else if (ramSize == 1) {
		ramEndAddress = 0x8000;
		ramBanks = 1;
		printf ("256Kbit\n");
	}
	else if (ramSize == 2) {
		ramEndAddress = 0x10000;
		ramBanks = 1;
		printf ("512Kbit\n");
	}
	else if (ramSize == 3) {
		ramEndAddress = 0x10000;
		ramBanks = 2;
		printf ("1Mbit\n");
	}
	
	printf ("EEPROM: ");
	if (eepromSize == EEPROM_NONE) {
		eepromEndAddress = 0;
		printf ("None\n");
	}
	else if (eepromSize == EEPROM_4KBIT) {
		eepromEndAddress = 0x200;
		printf ("4Kbit\n");
	}
	else if (eepromSize == EEPROM_64KBIT) {
		eepromEndAddress = 0x2000;
		printf ("64Kbit\n");
	}
}



// ****** GB Cart Flasher functions ******

// Write flash config file
void write_flash_config(int number) {
	// Keep the original numbers so older flash config files can still be used without having to re-configure them
	if (number < 100) {
		number = flashCartList[number-1];
	}
	
	char configFilePath[253];

#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)
	strncpy(configFilePath, "config-flash.ini", 17);
#else
	strncpy(configFilePath, getenv("USERPROFILE"), 200);
	strncat(configFilePath, "\\gbxcart-config-flash.ini", 26);
#endif
	
	FILE* configfile = fopen(configFilePath, "wt");
	if (configfile != NULL) {
		fprintf(configfile, "%d,", number);
		fclose(configfile);
	}
}

// Read the config-flash.ini file for the flash cart type
void read_config_flash(void) {
	char configFilePath[253];
	
#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)
	strncpy(configFilePath, "config-flash.ini", 17);
#else
	strncpy(configFilePath, getenv("USERPROFILE"), 200);
	strncat(configFilePath, "\\gbxcart-config-flash.ini", 26);
#endif
	
	FILE* configfile = fopen(configFilePath, "rt");
	if (configfile != NULL) {
		if (fscanf(configfile, "%d", &flashCartType) != 1) {
			fprintf(stderr, "Flash Config file is corrupt\n");
		}
		fclose(configfile);
	}
	else {
		fprintf(stderr, "Flash Config file not found\n");
	}
}

// Wait for first byte of chosen address to be 0xFF, that's when we know the sector has been erased
void wait_for_flash_sector_ff(uint16_t address) {
	uint16_t timeout = 0;
	readBuffer[0] = 0;
	
	while (readBuffer[0] != 0xFF) {
		set_number(address, SET_START_ADDRESS);
		set_mode(READ_ROM_RAM);
		
		uint8_t comReadBytes = com_read_bytes(READ_BUFFER, 64);
		com_read_stop(); // End read
		
		if (comReadBytes != 64) {
			fflush(stdin);
			delay_ms(500);
			
			// Flush buffer
			RS232_PollComport(cport_nr, readBuffer, 64);			
		}
		
		if (readBuffer[0] != 0xFF) {
			delay_ms(20);
			
			timeout++;
			if (timeout >= 200) {
				printf("\n\nWaiting for sector erase has timed out. Please unplug GBxCart RW, re-seat the cartridge and try again.\n");
				read_one_letter();
				exit(1);
			}
		}
	}
}

// Wait for 2 bytes of chosen address to be 0xFF, that's when we know the sector has been erased
void wait_for_gba_flash_sector_ff(uint32_t address, uint8_t byteOne,  uint8_t byteTwo) {
	uint16_t timeout = 0;
	
	// Wait for first 2 bytes to be 0xFF
	readBuffer[0] = 0;
	readBuffer[1] = 0;
	while (readBuffer[0] != byteOne && readBuffer[0] != byteTwo) {
		set_number(address / 2, SET_START_ADDRESS);
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
}


// Wait for first byte of Flash to be 0xFF, that's when we know the sector has been erased
void wait_for_gba_flash_erase_ff(uint32_t currAddr) {
	uint16_t timeout = 0;
	readBuffer[0] = 0;
	readBuffer[1] = 0;
	
	while (readBuffer[0] != 0xFF && readBuffer[1] != 0xFF) {
		set_number(currAddr / 2, SET_START_ADDRESS);
		delay_ms(5);
		set_mode(GBA_READ_ROM);
		delay_ms(5);
		
		com_read_bytes(READ_BUFFER, 64);
		com_read_stop(); // End read
		
		printf(".");
		delay_ms(2000);
		
		timeout++;
		if (timeout >= 200) {
			printf("\n\nWaiting for sector erase has timed out. Please unplug GBxCart RW, re-seat the cartridge and try again.\n");
			read_one_letter();
			exit(1);
		}
	}
}

// Wait for first byte of Flash to be 0xFF, that's when we know the sector has been erased
void wait_for_flash_chip_erase_ff(uint8_t printProgress) {
	uint16_t timeout = 0;
	readBuffer[0] = 0;
	
	while (readBuffer[0] != 0xFF) {
		set_number(currAddr, SET_START_ADDRESS);
		set_mode(READ_ROM_RAM);
		
		uint8_t comReadBytes = com_read_bytes(READ_BUFFER, 64);
		com_read_stop(); // End read
		
		if (comReadBytes != 64) {
			fflush(stdin);
			delay_ms(500);
			
			// Flush buffer
			RS232_PollComport(cport_nr, readBuffer, 64);			
		}
		
		if (printProgress == 1) {
			printf(".");
			//printf("0x%X ", readBuffer[0]);
			fflush(stdout);
		}
		
		if (readBuffer[0] != 0xFF) {
			delay_ms(500);
			
			timeout++;
			if (flashCartType == 16 || flashCartType == 17) {
				if (timeout >= 600) {
					printf("\n\n Waiting for chip erase has timed out. Please unplug GBxCart RW, re-seat the cartridge and try again.\n");
					read_one_letter();
					exit(1);
				}
			}
			else {
				if (timeout >= 240) {
					printf("\n\n Waiting for chip erase has timed out. Please unplug GBxCart RW, re-seat the cartridge and try again.\n");
					read_one_letter();
					exit(1);
				}
			}
		}
	}
}

// Select which pin need to pulse as WE (Audio or WR)
void gb_flash_pin_setup(char pin) {
	set_mode(GB_FLASH_WE_PIN);
	set_mode(pin);
}

// Select which flash program method to use
void gb_flash_program_setup(uint8_t method) {
	set_mode(GB_FLASH_PROGRAM_METHOD);
	
	if (method == GB_FLASH_PROGRAM_555) {
		send_hex_wait_ack(0x555);	send_hex_wait_ack(0xAA);
		send_hex_wait_ack(0x2AA);	send_hex_wait_ack(0x55);
		send_hex_wait_ack(0x555);	send_hex_wait_ack(0xA0);
	}
	else if (method == GB_FLASH_PROGRAM_AAA) {
		send_hex_wait_ack(0xAAA);	send_hex_wait_ack(0xAA);
		send_hex_wait_ack(0x555);	send_hex_wait_ack(0x55);
		send_hex_wait_ack(0xAAA);	send_hex_wait_ack(0xA0);
	}
	else if (method == GB_FLASH_PROGRAM_555_BIT01_SWAPPED) {
		send_hex_wait_ack(0x555);	send_hex_wait_ack(0xA9);
		send_hex_wait_ack(0x2AA);	send_hex_wait_ack(0x56);
		send_hex_wait_ack(0x555);	send_hex_wait_ack(0xA0);
	}
	else if (method == GB_FLASH_PROGRAM_AAA_BIT01_SWAPPED) {
		send_hex_wait_ack(0xAAA);	send_hex_wait_ack(0xA9);
		send_hex_wait_ack(0x555);	send_hex_wait_ack(0x56);
		send_hex_wait_ack(0xAAA);	send_hex_wait_ack(0xA0);
	}
	else if (method == GB_FLASH_PROGRAM_7AAA_BIT01_SWAPPED) {
		send_hex_wait_ack(0x7AAA);	send_hex_wait_ack(0xA9);
		send_hex_wait_ack(0x7555);	send_hex_wait_ack(0x56);
		send_hex_wait_ack(0x7AAA);	send_hex_wait_ack(0xA0);
	}
	else if (method == GB_FLASH_PROGRAM_5555) {
		send_hex_wait_ack(0x5555);	send_hex_wait_ack(0xAA);
		send_hex_wait_ack(0x2AAA);	send_hex_wait_ack(0x55);
		send_hex_wait_ack(0x5555);	send_hex_wait_ack(0xA0);
	}
}

// Write address and byte to flash
void gb_flash_write_address_byte (uint16_t address, uint8_t byte) {
	char AddrString[15];
	sprintf(AddrString, "%c%x", 'F', address);
	RS232_cputs(cport_nr, AddrString);
	RS232_SendByte(cport_nr, 0);
	RS232_drain(cport_nr);
	delay_ms(5);
	
	char byteString[15];
	sprintf(byteString, "%x", byte);
	RS232_cputs(cport_nr, byteString);
	RS232_SendByte(cport_nr, 0);
	RS232_drain(cport_nr);
	delay_ms(5);
	
	com_wait_for_ack(); 
}


// Read a bit of the ROM a few times to see if anything changes
void gb_check_stable_cart_data (void) {
	uint8_t readRomResult[10];
	
	// Read ROM a few times to see if anything changes
	for (uint8_t x = 0; x < 10; x++) {
		set_number(0, SET_START_ADDRESS);
		delay_ms(5);
		set_mode(READ_ROM_RAM);
		delay_ms(5);
		com_read_bytes(READ_BUFFER, 64);
		com_read_stop(); // End read
		delay_ms(50);
		
		//printf("Read rom: 0x%X,0x%X,0x%X,0x%X\n", readBuffer[0], readBuffer[1], readBuffer[2], readBuffer[3]);
		
		// Check if ROM read is different than last time
		if (x >= 1) {
			for (uint8_t r = 0; r < 8; r++) {
				if (readBuffer[r] != readRomResult[r]) {
					printf("\n*** The cartridge is changing it's data when being read back.\nPlease re-seat the cart and power cycle GBxCart. ***\n");
					read_one_letter();
					break;
				}
			}
		}
	}
}

// Check if the ROM reads differently when issuing the Flash ID command
void gb_check_change_flash_id (uint8_t flashMethod) {
	uint8_t readRomResult[10];
	
	// Read ROM a few times to see if anything changes
	for (uint8_t x = 0; x < 10; x++) {
		set_number(0, SET_START_ADDRESS);
		delay_ms(5);
		set_mode(READ_ROM_RAM);
		delay_ms(5);
		com_read_bytes(READ_BUFFER, 64);
		com_read_stop(); // End read
		delay_ms(50);
		
		//printf("Read rom: 0x%X,0x%X,0x%X,0x%X\n", readBuffer[0], readBuffer[1], readBuffer[2], readBuffer[3]);
		
		// Check if ROM read is different than last time
		if (x >= 1) {
			for (uint8_t r = 0; r < 8; r++) {
				if (readBuffer[r] != readRomResult[r]) {
					printf("\n*** The cartridge is changing it's data when being read back.\nPlease re-seat the cart and power cycle GBxCart. ***\n");
					read_one_letter();
					break;
				}
			}
		}
		
		// Store result
		for (uint8_t r = 0; r < 8; r++) {
			readRomResult[r] = readBuffer[r];
		}
	}
	
	// Request Flash ID
	if (flashMethod == GB_FLASH_PROGRAM_555) {
		gb_flash_write_address_byte(0x555, 0xAA);
		gb_flash_write_address_byte(0x2AA, 0x55);
		gb_flash_write_address_byte(0x555, 0x90);
	}
	else if (flashMethod == GB_FLASH_PROGRAM_AAA) {
		gb_flash_write_address_byte(0xAAA, 0xAA);
		gb_flash_write_address_byte(0x555, 0x55);
		gb_flash_write_address_byte(0xAAA, 0x90);
	}
	else if (flashMethod == GB_FLASH_PROGRAM_555_BIT01_SWAPPED) {
		gb_flash_write_address_byte(0x555, 0xA9);
		gb_flash_write_address_byte(0x2AA, 0x56);
		gb_flash_write_address_byte(0x555, 0x90);
	}
	else if (flashMethod == GB_FLASH_PROGRAM_AAA_BIT01_SWAPPED) {
		gb_flash_write_address_byte(0xAAA, 0xA9);
		gb_flash_write_address_byte(0x555, 0x56);
		gb_flash_write_address_byte(0xAAA, 0x90);
	}
	else if (flashMethod == GB_FLASH_PROGRAM_5555) {
		gb_flash_write_address_byte(0x5555, 0xAA);
		gb_flash_write_address_byte(0x2AAA, 0x55);
		gb_flash_write_address_byte(0x5555, 0x90);
	}
	else if (flashMethod == GB_FLASH_PROGRAM_7AAA_BIT01_SWAPPED) {
		gb_flash_write_address_byte(0x7AAA, 0xA9);
		gb_flash_write_address_byte(0x7555, 0x56);
		gb_flash_write_address_byte(0x7AAA, 0x90);
	}
	delay_ms(50);
	
	// Read ID
	set_number(0, SET_START_ADDRESS);
	delay_ms(5);
	set_mode(READ_ROM_RAM);
	delay_ms(5);
	com_read_bytes(READ_BUFFER, 64);
	com_read_stop(); // End read
	
	printf("Flash ID: 0x%X,0x%X,0x%X,0x%X\n", readBuffer[0], readBuffer[1], readBuffer[2], readBuffer[3]);
	
	// Check if ROM read is different to Flash ID
	uint8_t resultChanged = 0;
	for (uint8_t r = 0; r < 8; r++) {
		if (readBuffer[r] != readRomResult[r]) {
			resultChanged = 1;
		}
		flashID[r] = readBuffer[r];
	}
	
	// Exit
	gb_flash_write_address_byte(0x000, 0xF0);
	delay_ms(5);
	set_number(0, SET_START_ADDRESS);
	delay_ms(5);
	
	if (resultChanged == 0) {
		printf("\n*** Flash chip doesn't appear to be responding. Please re-seat the cart and power cycle GBxCart ***\n");
		read_one_letter();
	}
}

int8_t gb_check_flash_id (void) {
	uint8_t readROMResult[4];
	
	printf("          Read ROM: ");
	set_number(0, SET_START_ADDRESS);
	set_mode(READ_ROM_RAM);
	com_read_bytes(READ_BUFFER, 64);
	com_read_stop(); // End read
	
	for (uint8_t r = 0; r < 4; r++) {
		readROMResult[r] = readBuffer[r];
	}
	for (uint8_t r = 0; r < 8; r++) {
		printf("0x%X, ", readBuffer[r]);
	}
	printf("\n");
	
	set_mode(GB_CART_MODE); // Gameboy mode
	gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
	
	
	printf("Flash ID (555, AA): ");
	gb_flash_write_address_byte(0x555, 0xAA);
	gb_flash_write_address_byte(0x2AA, 0x55);
	gb_flash_write_address_byte(0x555, 0x90);
	delay_ms(50);
	
	set_number(0, SET_START_ADDRESS);
	set_mode(READ_ROM_RAM);
	com_read_bytes(READ_BUFFER, 64);
	com_read_stop(); // End read
	
	for (uint8_t r = 0; r < 8; r++) {
		printf("0x%X, ", readBuffer[r]);
	}
	printf("\n");
	gb_flash_write_address_byte(0x000, 0xF0);
	
	uint8_t sameData = 1;
	for (uint8_t r = 0; r < 4; r++) {
		if (readROMResult[r] != readBuffer[r]) {
			sameData = 0;
		}
	}
	if (sameData == 0) {
		return GB_FLASH_PROGRAM_555;
	}
	
	
	printf("Flash ID (555, A9): ");
	gb_flash_write_address_byte(0x555, 0xA9);
	gb_flash_write_address_byte(0x2AA, 0x56);
	gb_flash_write_address_byte(0x555, 0x90);
	delay_ms(50);
	
	set_number(0, SET_START_ADDRESS);
	set_mode(READ_ROM_RAM);
	com_read_bytes(READ_BUFFER, 64);
	com_read_stop(); // End read
	
	for (uint8_t r = 0; r < 8; r++) {
		printf("0x%X, ", readBuffer[r]);
	}
	printf("\n");
	gb_flash_write_address_byte(0x000, 0xF0);
	
	sameData = 1;
	for (uint8_t r = 0; r < 4; r++) {
		if (readROMResult[r] != readBuffer[r]) {
			sameData = 0;
		}
	}
	if (sameData == 0) {
		return GB_FLASH_PROGRAM_555_BIT01_SWAPPED;
	}
	
	
	printf("Flash ID (AAA, AA): ");
	gb_flash_write_address_byte(0xAAA, 0xAA);
	gb_flash_write_address_byte(0x555, 0x55);
	gb_flash_write_address_byte(0xAAA, 0x90);
	delay_ms(50);
	
	set_number(0, SET_START_ADDRESS);
	set_mode(READ_ROM_RAM);
	com_read_bytes(READ_BUFFER, 64);
	com_read_stop(); // End read
	
	for (uint8_t r = 0; r < 8; r++) {
		printf("0x%X, ", readBuffer[r]);
	}
	printf("\n");
	gb_flash_write_address_byte(0x000, 0xF0);
	
	sameData = 1;
	for (uint8_t r = 0; r < 4; r++) {
		if (readROMResult[r] != readBuffer[r]) {
			sameData = 0;
		}
	}
	if (sameData == 0) {
		return GB_FLASH_PROGRAM_AAA;
	}
	
	printf("Flash ID (AAA, A9): ");
	gb_flash_write_address_byte(0xAAA, 0xA9);
	gb_flash_write_address_byte(0x555, 0x56);
	gb_flash_write_address_byte(0xAAA, 0x90);
	delay_ms(50);
	
	set_number(0, SET_START_ADDRESS);
	set_mode(READ_ROM_RAM);
	com_read_bytes(READ_BUFFER, 64);
	com_read_stop(); // End read
	
	for (uint8_t r = 0; r < 8; r++) {
		printf("0x%X, ", readBuffer[r]);
	}
	printf("\n");
	gb_flash_write_address_byte(0x000, 0xF0);
	
	sameData = 1;
	for (uint8_t r = 0; r < 4; r++) {
		if (readROMResult[r] != readBuffer[r]) {
			sameData = 0;
		}
	}
	if (sameData == 0) {
		return GB_FLASH_PROGRAM_AAA_BIT01_SWAPPED;
	}
	
	printf("\n*** Flash chip doesn't appear to be responding. Please re-seat the cart and power cycle GBxCart ***\n");
	read_one_letter();
	
	return -1;
}



// ****** GBA Cart Flasher functions ******

// GBA Flash Cart, write address and byte
void gba_flash_write_address_byte (uint32_t address, uint16_t byte) {
	// Divide address by 2 as one address has 16 bytes of data
	address /= 2;
	
	char AddrString[20];
	sprintf(AddrString, "%c%x", 'n', address);
	RS232_cputs(cport_nr, AddrString);
	RS232_SendByte(cport_nr, 0);
	RS232_drain(cport_nr);
	delay_ms(5);
	
	char byteString[15];
	sprintf(byteString, "%c%x", 'n', byte);
	RS232_cputs(cport_nr, byteString);
	RS232_SendByte(cport_nr, 0);
	RS232_drain(cport_nr);
	delay_ms(5);
	
	com_wait_for_ack();
}


int8_t gba_check_flash_id (void) {
	uint8_t readROMResult[4];
	
	printf("          Read ROM: ");
	currAddr = 0x0000;	
	set_number(currAddr, SET_START_ADDRESS);
	set_mode(GBA_READ_ROM);
	com_read_bytes(READ_BUFFER, 64);
	com_read_stop();
	
	for (uint8_t r = 0; r < 4; r++) {
		readROMResult[r] = readBuffer[r];
	}
	for (uint8_t r = 0; r < 8; r++) {
		printf("0x%X, ", readBuffer[r]);
	}
	printf("\n");
	gba_flash_write_address_byte(0x000, 0xF0);
	
	
	printf("Flash ID (AAA, A9): ");
	gba_flash_write_address_byte(0xAAA, 0xA9);
	gba_flash_write_address_byte(0x555, 0x56);
	gba_flash_write_address_byte(0xAAA, 0x90);
	
	currAddr = 0x0000;	
	set_number(currAddr, SET_START_ADDRESS);
	set_mode(GBA_READ_ROM);
	com_read_bytes(READ_BUFFER, 64);
	com_read_stop();
	
	for (uint8_t r = 0; r < 8; r++) {
		printf("0x%X, ", readBuffer[r]);
	}
	printf("\n");
	gba_flash_write_address_byte(0x000, 0xF0);
	
	uint8_t sameData = 1;
	for (uint8_t r = 0; r < 4; r++) {
		if (readROMResult[r] != readBuffer[r]) {
			sameData = 0;
		}
	}
	if (sameData == 0) {
		return GBA_FLASH_PROGRAM_AAA_BIT01_SWAPPED;
	}
	
	
	printf("Flash ID (AAA, AA): ");
	gba_flash_write_address_byte(0xAAA, 0xAA);
	gba_flash_write_address_byte(0x555, 0x55);
	gba_flash_write_address_byte(0xAAA, 0x90);
	
	currAddr = 0x0000;	
	set_number(currAddr, SET_START_ADDRESS);
	set_mode(GBA_READ_ROM);
	com_read_bytes(READ_BUFFER, 64);
	com_read_stop();
	
	for (uint8_t r = 0; r < 8; r++) {
		printf("0x%X, ", readBuffer[r]);
	}
	printf("\n");
	gba_flash_write_address_byte(0x000, 0xF0);
	
	sameData = 1;
	for (uint8_t r = 0; r < 4; r++) {
		if (readROMResult[r] != readBuffer[r]) {
			sameData = 0;
		}
	}
	if (sameData == 0) {
		return GBA_FLASH_PROGRAM_AAA;
	}
	
	printf("\n*** Flash chip doesn't appear to be responding. Please re-seat the cart and power cycle GBxCart ***\n");
	read_one_letter();
	
	return -1;
}
