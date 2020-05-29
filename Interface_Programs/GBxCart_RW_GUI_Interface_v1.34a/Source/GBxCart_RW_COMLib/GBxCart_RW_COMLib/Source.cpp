/*
GBxCart RW - GUI Interface
Version : 1.34
Author : Alex from insideGadgets(www.insidegadgets.com)
Created : 7 / 11 / 2016
Last Modified : 27 / 05 / 2020

GBxCart RW allows you to dump your Gameboy / Gameboy Colour / Gameboy Advance games ROM, save the RAM and write to the RAM.

*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <windows.h>


extern "C" {

#ifndef uint8_t
	typedef unsigned char uint8_t;
#endif

#ifndef uint16_t
	typedef unsigned int uint16_t;
#endif

#ifndef uint32_t
	typedef unsigned long uint32_t;
#endif

extern void RS232_cputs(int comport_number, const char *text);
extern int RS232_SendByte(int comport_number, unsigned char byte);
extern int RS232_PollComport(int comport_number, unsigned char *buf, int size);
extern int RS232_SendBuf(int comport_number, unsigned char *buf, int size);
extern void xmas_set_leds(uint32_t value);
extern void xmas_wake_up(void);
extern __declspec(dllexport) uint8_t request_value(uint8_t command);

#define LOW 0
#define HIGH 1
#define false 0
#define true 1

#define CART_MODE 'C'
#define GB_MODE 1
#define GBA_MODE 2
#define VOLTAGE_3_3V '3'
#define VOLTAGE_5V '5'

#define READ_FIRMWARE_VERSION 'V'

// GB/GBC defines/commands
#define SET_START_ADDRESS 'A'
#define READ_ROM_RAM 'R'
#define WRITE_RAM 'W'
#define SET_BANK 'B'
#define GB_CART_MODE 'G'

// GBA defines/commands
#define EEPROM_NONE 0
#define EEPROM_4KBIT 1
#define EEPROM_64KBIT 2

#define SRAM_FLASH_NONE 0
#define SRAM_FLASH_256KBIT 1
#define SRAM_FLASH_512KBIT 2
#define SRAM_FLASH_1MBIT 3

#define NOT_CHECKED 0
#define NO_FLASH 1
#define FLASH_FOUND 2
#define FLASH_FOUND_ATMEL 3
#define FLASH_FOUND_INTEL 4
	
#define GBA_READ_ROM 'r'
#define GBA_READ_ROM_256BYTE 'j'
#define GBA_READ_SRAM 'm'
#define GBA_WRITE_SRAM 'w'
#define GBA_WRITE_ONE_BYTE_SRAM 'o'
#define GBA_CART_MODE 'g'

#define GBA_SET_EEPROM_SIZE 'S'
#define GBA_READ_EEPROM 'e'
#define GBA_WRITE_EEPROM 'p'

#define GBA_FLASH_READ_ID 'i'
#define GBA_FLASH_SET_BANK 'k'
#define GBA_FLASH_4K_SECTOR_ERASE 's'
#define GBA_FLASH_WRITE_BYTE 'b'
#define GBA_FLASH_WRITE_ATMEL 'a'

#define RESET_COMMON_LINES 'M'
#define READ_FIRMWARE_VERSION 'V'
#define READ_PCB_VERSION 'h'

// Flash Cart commands
#define GB_FLASH_WE_PIN 'P'
#define WE_AS_AUDIO_PIN 'A'
#define WE_AS_WR_PIN 'W'

#define GB_FLASH_PROGRAM_METHOD 'E'
#define GB_FLASH_PROGRAM_555 0
#define GB_FLASH_PROGRAM_AAA 1
#define GB_FLASH_PROGRAM_555_BIT01_SWAPPED 2
#define GB_FLASH_PROGRAM_AAA_BIT01_SWAPPED 3
#define GB_FLASH_PROGRAM_5555 4

#define GB_FLASH_WRITE_BYTE 'F'
#define GB_FLASH_WRITE_64BYTE 'T'
#define GB_FLASH_WRITE_256BYTE 'X'
#define GB_FLASH_WRITE_BUFFERED_32BYTE 'Y'

#define GB_FLASH_BANK_1_COMMAND_WRITES 'N'

#define GBA_FLASH_CART_WRITE_BYTE 'n'
#define GBA_FLASH_WRITE_64BYTE_SWAPPED_D0D1 'q'
#define GBA_FLASH_WRITE_256BYTE_SWAPPED_D0D1 't'
#define GBA_FLASH_WRITE_256BYTE 'f'
#define GBA_FLASH_WRITE_INTEL_64BYTE 'l'

#define XMAS_LEDS '#'
#define XMAS_VALUE 0x7690FCD

// PCB versions
#define PCB_1_0 1
#define PCB_1_1 2
#define PCB_1_3 4
#define GBXMAS 90

// Common vars
#define READ_BUFFER 0

int cport_nr = 7; // /dev/ttyS7 (COM8 on windows)
int bdrate = 1000000; // 1,000,000 baud

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
char currentFolder[220];
uint32_t bytesReadPrevious = 0;
uint32_t ledStatus = 0;
uint32_t ledCountLeft = 0;
uint32_t ledCountRight = 0;
uint8_t ledSegment = 0;
uint8_t ledProgress = 0;
uint8_t ledBlinking = 0;
uint8_t writingSelected = 0;

uint8_t nintendoLogoGBA[] = { 0x24, 0xFF, 0xAE, 0x51, 0x69, 0x9A, 0xA2, 0x21, 0x3D, 0x84, 0x82, 0x0A, 0x84, 0xE4, 0x09, 0xAD,
	0x11, 0x24, 0x8B, 0x98, 0xC0, 0x81, 0x7F, 0x21, 0xA3, 0x52, 0xBE, 0x19, 0x93, 0x09, 0xCE, 0x20,
	0x10, 0x46, 0x4A, 0x4A, 0xF8, 0x27, 0x31, 0xEC, 0x58, 0xC7, 0xE8, 0x33, 0x82, 0xE3, 0xCE, 0xBF,
	0x85, 0xF4, 0xDF, 0x94, 0xCE, 0x4B, 0x09, 0xC1, 0x94, 0x56, 0x8A, 0xC0, 0x13, 0x72, 0xA7, 0xFC,
	0x9F, 0x84, 0x4D, 0x73, 0xA3, 0xCA, 0x9A, 0x61, 0x58, 0x97, 0xA3, 0x27, 0xFC, 0x03, 0x98, 0x76,
	0x23, 0x1D, 0xC7, 0x61, 0x03, 0x04, 0xAE, 0x56, 0xBF, 0x38, 0x84, 0x00, 0x40, 0xA7, 0x0E, 0xFD,
	0xFF, 0x52, 0xFE, 0x03, 0x6F, 0x95, 0x30, 0xF1, 0x97, 0xFB, 0xC0, 0x85, 0x60, 0xD6, 0x80, 0x25,
	0xA9, 0x63, 0xBE, 0x03, 0x01, 0x4E, 0x38, 0xE2, 0xF9, 0xA2, 0x34, 0xFF, 0xBB, 0x3E, 0x03, 0x44,
	0x78, 0x00, 0x90, 0xCB, 0x88, 0x11, 0x3A, 0x94, 0x65, 0xC0, 0x7C, 0x63, 0x87, 0xF0, 0x3C, 0xAF,
	0xD6, 0x25, 0xE4, 0x8B, 0x38, 0x0A, 0xAC, 0x72, 0x21, 0xD4, 0xF8, 0x07 };

// Read config file for com port and baud rate
__declspec(dllexport) int read_config(int type) {
	int alwaysAddDateTimeToSave = 0;
	int promptForRestoreSaveFile = 0;
	int reReadCartHeader = 0;

	FILE *configfile = fopen("config.ini", "rt");
	if (configfile != NULL) {
		if (fscanf(configfile, "%d\n%d\n%d\n%d\n%d", &cport_nr, &bdrate, &alwaysAddDateTimeToSave, &promptForRestoreSaveFile, &reReadCartHeader) > 5) {
			fprintf(stderr, "Config file is corrupt\n");
		}
		fclose(configfile);
	}
	else {
		fprintf(stderr, "Config file not found\n");
	}

	if (type == 1) {
		return cport_nr;
	}
	else if (type == 2) {
		return bdrate;
	}
	else if (type == 3) {
		return alwaysAddDateTimeToSave;
	}
	else if (type == 4) {
		return promptForRestoreSaveFile;
	}
	else if (type == 5) {
		return reReadCartHeader;
	}

	return 0;
}

// Update config file if com port or baudrate is changed
__declspec(dllexport) void update_config(int comport, INT32 baudrate, int alwaysAddDateTimeToSave, int promptForRestoreSaveFile, int reReadCartHeader) {
	cport_nr = comport;
	bdrate = baudrate;

	FILE *configfile = fopen("config.ini", "wt");
	if (configfile != NULL) {
		fprintf(configfile, "%d\n%d\n%d\n%d\n%d\n", cport_nr+1, bdrate, alwaysAddDateTimeToSave, promptForRestoreSaveFile, reReadCartHeader);
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


// Selected folder
__declspec(dllexport) void update_current_folder(char* folderName) {
	strncpy(currentFolder, folderName, 219);
}

__declspec(dllexport) void gb_specify_mbc_type(int mbcType) {
	cartridgeType = mbcType;
}

__declspec(dllexport) void gb_specify_rom_size(int size) {
	romSize = size;
	romBanks = 2; // Default 32K
	if (romSize >= 1) { // Calculate rom size
		romBanks = 2 << romSize;
	}
}

__declspec(dllexport) void gba_specify_rom_size(int size) {
	romSize = 4 << (size - 1);
	romEndAddr = ((1024 * 1024) * romSize);
}

__declspec(dllexport) void gb_specify_ram_size(int size) {
	ramSize = size;

	// RAM banks
	ramBanks = 0; // Default 0K RAM
	if (ramSize == 1) { ramBanks = 1; }
	if (ramSize == 2) { ramBanks = 1; }
	if (ramSize == 3) { ramBanks = 4; }
	if (ramSize == 4) { ramBanks = 16; }
	if (ramSize == 5) { ramBanks = 8; }

	// RAM end address
	if (ramSize == 1) { ramEndAddress = 0xA7FF; } // 2K RAM
	if (ramSize > 1) { ramEndAddress = 0xBFFF; } // 8K RAM
	if (ramSize == 6) { ramEndAddress = 0xA1FF; ramBanks = 1; ramSize = 1; } // MBC2 512bytes (nibbles)
}

__declspec(dllexport) void gba_specify_ram_size(int memoryType, int flashType, int size) {
	if (memoryType == 1 || memoryType == 2) {
		eepromSize = EEPROM_NONE;
		eepromEndAddress = 0;

		if (memoryType == 1) {
			hasFlashSave = NO_FLASH;
		}
		else {
			if (flashType == 1) {
				hasFlashSave = FLASH_FOUND_ATMEL;
			}
			else if (flashType == 2) {
				hasFlashSave = FLASH_FOUND;
			}
		}

		ramSize = size;
		if (size == 0) {
			ramEndAddress = 0;
		}
		else if (size == 1) {
			ramEndAddress = 0x8000;
			ramBanks = 1;
		}
		else if (size == 2) {
			ramEndAddress = 0x10000;
			ramBanks = 1;
		}
		else if (size == 3) {
			ramEndAddress = 0x10000;
			ramBanks = 2;
		}
	}
	else if (memoryType == 3) {
		ramSize = size; 
		eepromEndAddress = 0x00;
		eepromSize = EEPROM_NONE;
		hasFlashSave = NO_FLASH;
		ramEndAddress = 0;

		if (size == 1) {
			eepromEndAddress = 0x200;
			eepromSize = EEPROM_4KBIT;
			hasFlashSave = NO_FLASH;
			ramEndAddress = 0;
		}
		else if (size == 2) {
			eepromEndAddress = 0x2000;
			eepromSize = EEPROM_64KBIT;
			hasFlashSave = NO_FLASH;
			ramEndAddress = 0;
		}
	}
}

// Send a command with a hex number and a null terminator byte
void set_number(uint32_t number, uint8_t command) {
	char numberString[20];
	sprintf(numberString, "%c%x", command, number);

	RS232_cputs(cport_nr, numberString);
	RS232_SendByte(cport_nr, 0);
}

// Send a single command byte
__declspec(dllexport) void set_mode(char command) {
	if (command == VOLTAGE_5V) {
		gbxcartPcbVersion = request_value(READ_PCB_VERSION);
		
		if (gbxcartPcbVersion == GBXMAS) {
			RS232_cputs(cport_nr, "!\0");
			Sleep(50);

			RS232_cputs(cport_nr, "!\0");
			Sleep(5);
			set_number(XMAS_VALUE, XMAS_LEDS);
			Sleep(5);
			set_number(0x6555955, 'L');
			Sleep(5);
		}
	}
	else if(command == VOLTAGE_3_3V) {
		gbxcartPcbVersion = request_value(READ_PCB_VERSION); 
		
		if (gbxcartPcbVersion == GBXMAS) {
			RS232_cputs(cport_nr, "!\0");
			Sleep(50);

			RS232_cputs(cport_nr, "!\0");
			Sleep(5);
			set_number(XMAS_VALUE, XMAS_LEDS);
			Sleep(5);
			set_number(0x9AAA6AA, 'L');
			Sleep(5);
		}
	}

	char modeString[5];
	sprintf(modeString, "%c", command);
	RS232_cputs(cport_nr, modeString);
}


void xmas_set_leds(uint32_t value) {
	if (writingSelected == 1) {
		set_mode('0');
		Sleep(5);
	}
	set_number(XMAS_VALUE, XMAS_LEDS);
	Sleep(5);
	set_number(value, 'L');
	Sleep(5);
}

void xmas_blink_led(uint8_t value) {
	if (writingSelected == 1) {
		set_mode('0');
		Sleep(5);
	}
	set_number(XMAS_VALUE, XMAS_LEDS);
	Sleep(5);
	set_mode('B');
	set_mode(value);
	Sleep(5);
}

// Print progress
void led_progress_percent(uint32_t bytesRead, uint32_t divideNumber) {
	if (gbxcartPcbVersion == GBXMAS) {
		if (bytesRead >= bytesReadPrevious) {
			bytesReadPrevious += divideNumber;

			if (ledSegment == 0) {
				ledStatus |= (1 << ledCountLeft);
				ledSegment = 1;
				ledCountLeft++;
			}
			else {
				ledSegment = 0;
				ledStatus |= (1 << (ledCountRight + 14));
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


void xmas_reset_values(void) {
	ledStatus = 0;
	ledCountLeft = 0;
	ledCountRight = 0;
	ledSegment = 0;
	ledProgress = 0;
	ledBlinking = 0;
	bytesReadPrevious = 0;
}

// Turn on idle timer
void xmas_idle_on(void) {
	set_mode('0');
	Sleep(5);
	set_number(XMAS_VALUE, XMAS_LEDS);
	Sleep(5);
	set_mode('I');
	Sleep(5);
}

// Turn off idle timer
void xmas_idle_off(void) {
	set_mode('0');
	Sleep(5);
	set_number(XMAS_VALUE, XMAS_LEDS);
	Sleep(5);
	set_mode('O');
	Sleep(5);
}

void xmas_chip_erase_animation(void) {
	set_mode('0');
	Sleep(5);
	set_number(XMAS_VALUE, XMAS_LEDS);
	Sleep(5);
	set_mode('E');
	Sleep(5);
}

void xmas_wake_up(void) {
	if (gbxcartPcbVersion == GBXMAS) {
		set_mode('!');
		Sleep(50);  // Wait for ATmega169 to WDT reset if in idle mode
	}
}

void xmas_setup(uint32_t progressNumber) {
	if (gbxcartPcbVersion == GBXMAS) {
		xmas_wake_up();
		xmas_reset_values();
		xmas_set_leds(0);
		ledBlinking = 1;
		xmas_blink_led(ledBlinking);
		bytesReadPrevious = progressNumber;
	}
}

// Read one letter from stdin
char read_one_letter(void) {
	char c = getchar();
	while (getchar() != '\n' && getchar() != EOF);
	return c;
}

// Print progress
void print_progress_percent(uint32_t &progress, uint32_t bytesRead, uint32_t hashNumber) {
	progress = (double) ((double) bytesRead / (double) hashNumber) * (double) 100;
}

// Wait for a "1" acknowledgement from the ATmega
uint8_t com_wait_for_ack(void) {
	uint8_t buffer[2];
	uint8_t rxBytes = 0;
	uint8_t timeoutCounter = 0;

	while (rxBytes < 1) {
		rxBytes = RS232_PollComport(cport_nr, buffer, 1);

		if (rxBytes > 0) {
			if (buffer[0] == '1') {
				return 1;
			}
			rxBytes = 0;
		}

		Sleep(1);
		timeoutCounter++;
		//if (timeoutCounter >= 250) { // After 250ms, timeout
		//	return 0;
		//}
	}
}

// Stop reading blocks of data
void com_read_stop() {
	RS232_cputs(cport_nr, "0"); // Stop read
	if (gbxcartPcbVersion == GBXMAS) { // Small delay as GBXMAS intercepts these commands
		Sleep(1);
	}
}

// Continue reading the next block of data
void com_read_cont() {
	RS232_cputs(cport_nr, "1"); // Continue read
	if (gbxcartPcbVersion == GBXMAS) { // Small delay as GBXMAS intercepts these commands
		Sleep(1);
	}
}


// Read 1 to 256 bytes from the COM port and write it to the global read buffer or to a file if specified. 
// When polling the com port it return less than the bytes we want, keep polling and wait until we have all bytes requested. 
// We expect no more than 256 bytes.
uint16_t com_read_bytes(FILE *file, int count) {
	uint8_t buffer[257];
	uint8_t rxBytes = 0;
	int readBytes = 0;
	int timeout = 0;

	while (readBytes < count) {
		//rxBytes = RS232_PollComport(cport_nr, buffer, 65);
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
		else {
			timeout++;
			if (timeout >= 20000) {
				return readBytes;
			}
		}
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
}


// Send a single hex byte and wait for ACK back
void send_hex_wait_ack(uint16_t hex) {
	char tempString[15];
	sprintf(tempString, "%x", hex);
	RS232_cputs(cport_nr, tempString);
	RS232_SendByte(cport_nr, 0);
	Sleep(5);

	com_wait_for_ack();
}

// Read the cartridge mode
__declspec(dllexport) uint8_t read_cartridge_mode(void) {
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

// Read the firmware version
__declspec(dllexport) uint8_t read_firmware_version(void) {
	set_mode(READ_FIRMWARE_VERSION);

	uint8_t buffer[2];
	uint8_t rxBytes = 0;
	uint8_t timeoutCounter = 0;
	while (rxBytes < 1) {
		rxBytes = RS232_PollComport(cport_nr, buffer, 1);

		if (rxBytes > 0) {
			return buffer[0];
		}

		Sleep(10);
		timeoutCounter++;
		if (timeoutCounter >= 25) { // After 250ms, timeout
			return 0;
		}
	}

	return 0;
}

// Send 1 byte and read 1 byte
__declspec(dllexport) uint8_t request_value(uint8_t command) {
	set_mode(command);

	uint8_t buffer[2];
	uint8_t rxBytes = 0;
	uint8_t timeoutCounter = 0;
	while (rxBytes < 1) {
		rxBytes = RS232_PollComport(cport_nr, buffer, 1);

		if (rxBytes > 0) {
			return buffer[0];
		}

		Sleep(10);
		timeoutCounter++;
		if (timeoutCounter >= 25) { // After 250ms, timeout
			return 0;
		}
	}

	return 0;
}


// ****** Gameboy / Gameboy Colour functions ******

// Set bank for ROM/RAM switching, send address first and then bank number
void set_bank(uint16_t address, uint8_t bank) {
	char AddrString[15];
	sprintf(AddrString, "%c%x", SET_BANK, address);
	RS232_cputs(cport_nr, AddrString);
	RS232_SendByte(cport_nr, 0);
	Sleep(5);

	char bankString[15];
	sprintf(bankString, "%c%d", SET_BANK, bank);
	RS232_cputs(cport_nr, bankString);
	RS232_SendByte(cport_nr, 0);
	Sleep(5);
}

// MBC2 Fix (unknown why this fixes reading the ram, maybe has to read ROM before RAM?)
// Read 64 bytes of ROM, (really only 1 byte is required)
void mbc2_fix(void) {
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
	RS232_cputs(cport_nr, "0"); // Stop read
}

// Read the first 384 bytes of ROM and process the Gameboy header information
__declspec(dllexport) char* read_gb_header(int &headerlength) {
	set_mode('0'); // Break out of any loops on ATmega
	
	char* headerText;
	headerText = (char*) malloc(500);

	currAddr = 0x0000;
	endAddr = 0x0180;

	set_number(currAddr, SET_START_ADDRESS);
	set_mode(READ_ROM_RAM);

	uint8_t startRomBuffer[385];
	while (currAddr < endAddr) {
		uint8_t comReadBytes = com_read_bytes(READ_BUFFER, 64);

		if (comReadBytes == 64) {
			memcpy(&startRomBuffer[currAddr], readBuffer, 64);
			currAddr += 64;

			// Request 64 bytes more
			if (currAddr < endAddr) {
				com_read_cont();
			}
		}
		else { // Didn't receive 64 bytes, usually this only happens for Apple MACs
			com_read_stop();
			Sleep(500);

			// Flush buffer
			RS232_PollComport(cport_nr, readBuffer, 64);

			// Start off where we left off
			set_number(currAddr, SET_START_ADDRESS);
			set_mode(READ_ROM_RAM);
		}
		
		/*com_read_bytes(READ_BUFFER, 64);
		memcpy(&startRomBuffer[currAddr], readBuffer, 64);
		currAddr += 64;

		if (currAddr < endAddr) {
			RS232_cputs(cport_nr, "1");
		}*/
	}
	com_read_stop();

	// Blank out game title
	for (uint8_t b = 0; b < 16; b++) {
		gameTitle[b] = 0;
	}
	// Read cartridge title and check for non-printable text
	for (uint16_t titleAddress = 0x0134; titleAddress <= 0x143; titleAddress++) {
		char headerChar = startRomBuffer[titleAddress];
		if ((headerChar >= 0x30 && headerChar <= 0x39) || // 0-9
			(headerChar >= 0x41 && headerChar <= 0x5A) || // A-Z
			(headerChar >= 0x61 && headerChar <= 0x7A) || // a-z
			(headerChar >= 0x24 && headerChar <= 0x29) || // #$%&'()
			(headerChar == 0x2D) || // -
			(headerChar == 0x2E) || // .
			(headerChar == 0x5F) || // _
			(headerChar == 0x20)) { // space
			gameTitle[(titleAddress - 0x0134)] = headerChar;
		}
		// Replace with an underscore
		else if (headerChar == 0x3A) { //  : 
			gameTitle[(titleAddress - 0x0134)] = '_';
		}
		else {
			gameTitle[(titleAddress - 0x0134)] = '\0';
			break;
		}
	}
	//printf("Game title: %s\n", gameTitle);
	//sprintf(headerText, "Game title: %s\n", gameTitle);
	
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

	//printf("MBC type: ");
	//sprintf(headerText, "MBC type: ");

	char cartridgeTypeText[50];
	switch (cartridgeType) {
	case 0: strncpy(cartridgeTypeText, "ROM ONLY\0", 30); break;
	case 1: strncpy(cartridgeTypeText, "MBC1\0", 30); break;
	case 2: strncpy(cartridgeTypeText, "MBC1+RAM\0", 30); break;
	case 3: strncpy(cartridgeTypeText, "MBC1+RAM+BATTERY\0", 30); break;
	case 5: strncpy(cartridgeTypeText, "MBC2\0", 30); break;
	case 6: strncpy(cartridgeTypeText, "MBC2+BATTERY\0", 30); break;
	case 8: strncpy(cartridgeTypeText, "ROM+RAM\0", 30); break;
	case 9: strncpy(cartridgeTypeText, "ROM ONLY\0", 30); break;
	case 11: strncpy(cartridgeTypeText, "MMM01\0", 30); break;
	case 12: strncpy(cartridgeTypeText, "MMM01+RAM\0", 30); break;
	case 13: strncpy(cartridgeTypeText, "MMM01+RAM+BATTERY\0", 30); break;
	case 15: strncpy(cartridgeTypeText, "MBC3+TIMER+BATTERY\0", 30); break;
	case 16: strncpy(cartridgeTypeText, "MBC3+TIMER+RAM+BATTERY\0", 30); break;
	case 17: strncpy(cartridgeTypeText, "MBC3\0", 30); break;
	case 18: strncpy(cartridgeTypeText, "MBC3+RAM\0", 30); break;
	case 19: strncpy(cartridgeTypeText, "MBC3+RAM+BATTERY\0", 30); break;
	case 21: strncpy(cartridgeTypeText, "MBC4\0", 30); break;
	case 22: strncpy(cartridgeTypeText, "MBC4+RAM\0", 30); break;
	case 23: strncpy(cartridgeTypeText, "MBC4+RAM+BATTERY\0", 30); break;
	case 25: strncpy(cartridgeTypeText, "MBC5\0", 30); break;
	case 26: strncpy(cartridgeTypeText, "MBC5+RAM\0", 30); break;
	case 27: strncpy(cartridgeTypeText, "MBC5+RAM+BATTERY\0", 30); break;
	case 28: strncpy(cartridgeTypeText, "MBC5+RUMBLE\0", 30); break;
	case 29: strncpy(cartridgeTypeText, "MBC5+RUMBLE+RAM\0", 30); break;
	case 30: strncpy(cartridgeTypeText, "MBC5+RUMBLE+RAM+BATTERY\0", 30); break;
	case 252: strncpy(cartridgeTypeText, "Gameboy Camera\0", 30); break;
	default: strncpy(cartridgeTypeText, "Not found\0", 30);
	}

	//printf("ROM size: ");
	char romSizeText[50];
	switch (romSize) {
	case 0: strncpy(romSizeText,"32KByte (no ROM banking)\0", 30); break;
	case 1: strncpy(romSizeText,"64KByte (4 banks)\0", 30); break;
	case 2: strncpy(romSizeText,"128KByte (8 banks)\0", 30); break;
	case 3: strncpy(romSizeText,"256KByte (16 banks)\0", 30); break;
	case 4: strncpy(romSizeText,"512KByte (32 banks)\0", 30); break;
	case 5:
		if (cartridgeType == 1 || cartridgeType == 2 || cartridgeType == 3) {
			strncpy(romSizeText,"1MByte (63 banks)\0", 30);
		}
		else {
			strncpy(romSizeText,"1MByte (64 banks)\0", 30);
		}
		break;
	case 6:
		if (cartridgeType == 1 || cartridgeType == 2 || cartridgeType == 3) {
			strncpy(romSizeText,"2MByte (125 banks)\0", 30);
		}
		else {
			strncpy(romSizeText,"2MByte (128 banks)\0", 30);
		}
		break;
	case 7: strncpy(romSizeText,"4MByte (256 banks)\0", 30); break;
	case 82: strncpy(romSizeText,"1.1MByte (72 banks)\0", 30); break;
	case 83: strncpy(romSizeText,"1.2MByte (80 banks)\0", 30); break;
	case 84: strncpy(romSizeText,"1.5MByte (96 banks)\0", 30); break;
	default: strncpy(romSizeText,"Not found\0", 30);
	}

	//printf("RAM size: ");
	char ramSizeText[50];
	switch (ramSize) {
	case 0:
		if (cartridgeType == 6) {
			strncpy(ramSizeText,"512 bytes (nibbles)\0", 30);
		}
		else {
			strncpy(ramSizeText,"None\0", 30);
		}
		break;
	case 1: strncpy(ramSizeText,"2 KBytes\0", 30); break;
	case 2: strncpy(ramSizeText,"8 KBytes\0", 30); break;
	case 3: strncpy(ramSizeText,"32 KBytes (4 banks of 8Kbytes)\0", 40); break;
	case 4: strncpy(ramSizeText,"128 KBytes (16 banks of 8Kbytes)\0", 40); break;
	default: strncpy(ramSizeText,"Not found\0", 30);
	}

	// Header checksum check
	uint8_t romCheckSum = 0;
	for (uint16_t x = 0x0134; x <= 0x014C; x++) {
		romCheckSum = romCheckSum - startRomBuffer[x] - 1;
	}
	char checkSumText[50]; 
	if (romCheckSum == startRomBuffer[0x14D]) {
		strncpy(checkSumText, "OK\0", 30);
	}
	else {
		strncpy(checkSumText, "Failed\0", 30);
	}
	
	sprintf(headerText, "Game title: %s\nMBC type: %s\nROM size: %s\nRAM size: %s\nHeader Checksum: %s", gameTitle, cartridgeTypeText, romSizeText, ramSizeText, checkSumText);
	headerlength = strlen(headerText);
	
	return headerText;
}


// ****** Gameboy Advance functions ****** 

// Check the rom size by reading 64 bytes from different addresses and checking if they are all 0x00. There can be some ROMs 
// that do have valid 0x00 data, so we check 32 different addresses in a 4MB chunk, if 30 or more are all 0x00 then we've reached the end.
uint8_t gba_check_rom_size(void) {
	uint32_t fourMbBoundary = 0x3FFFC0;
	uint32_t currAddr = 0x1FFC0;
	uint8_t romZeroTotal = 0;
	uint8_t romSize = 0;

	// Loop until 32MB
	for (uint16_t x = 0; x < 512; x++) {
		set_number(currAddr / 2, SET_START_ADDRESS); // Divide current address by 2 as we only increment it by 1 after 2 bytes have been read on the ATmega side
		set_mode(GBA_READ_ROM);

		com_read_bytes(READ_BUFFER, 64);
		RS232_cputs(cport_nr, "0"); // Stop read

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
uint8_t gba_test_sram_flash_write(void) {
	printf("Testing for SRAM or Flash presence... ");

	// Save the 1 byte first to buffer
	uint8_t saveBuffer[65];
	set_number(0x0000, SET_START_ADDRESS);
	set_mode(GBA_READ_SRAM);
	com_read_bytes(READ_BUFFER, 64);
	memcpy(&saveBuffer, readBuffer, 64);
	RS232_cputs(cport_nr, "0"); // Stop read

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
	com_wait_for_ack();

	// Read back the 1 byte
	uint8_t readBackBuffer[65];
	set_number(0x0000, SET_START_ADDRESS);
	set_mode(GBA_READ_SRAM);
	com_read_bytes(READ_BUFFER, 64);
	memcpy(&readBackBuffer, readBuffer, 64);
	RS232_cputs(cport_nr, "0"); // Stop read

	// Verify
	if (readBackBuffer[0] == testNumber) {
		printf("SRAM found\n");

		// Write the byte back to how it was
		set_number(0x0000, SET_START_ADDRESS);
		tempBuffer[0] = GBA_WRITE_ONE_BYTE_SRAM; // Set write sram 1 byte mode
		tempBuffer[1] = saveBuffer[0];
		RS232_SendBuf(cport_nr, tempBuffer, 2);
		com_wait_for_ack();

		return NO_FLASH;
	}
	else { // Flash likely present, test by reading the flash ID
		printf("Flash found\n");

		set_mode(GBA_FLASH_READ_ID); // Read Flash ID and exit Flash ID mode
		Sleep(100);

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
		RS232_cputs(cport_nr, "0"); // Stop read

		// Exit the ID mode a different way and slowly
		if (readBackBuffer[0] == 0x1F || readBackBuffer[0] == 0xBF || readBackBuffer[0] == 0xC2 ||
			readBackBuffer[0] == 0x32 || readBackBuffer[0] == 0x62) {

			RS232_cputs(cport_nr, "G"); // Set Gameboy mode
			Sleep(5);

			RS232_cputs(cport_nr, "M0"); // Disable CS/RD/WR/CS2-RST from going high after each command
			Sleep(5);

			RS232_cputs(cport_nr, "OC0xFF"); // Set output lines
			RS232_SendByte(cport_nr, 0);
			Sleep(5);

			RS232_cputs(cport_nr, "HC0xF0"); // Set byte
			RS232_SendByte(cport_nr, 0);
			Sleep(5);

			// V1.1 PCB
			if (gbxcartPcbVersion == PCB_1_1 || gbxcartPcbVersion == PCB_1_3) {
				RS232_cputs(cport_nr, "LD0x40"); // WE low
				RS232_SendByte(cport_nr, 0);
				Sleep(5);

				RS232_cputs(cport_nr, "LE0x04"); // CS2 low
				RS232_SendByte(cport_nr, 0);
				Sleep(5);

				RS232_cputs(cport_nr, "HD0x40"); // WE high
				RS232_SendByte(cport_nr, 0);
				Sleep(5);

				RS232_cputs(cport_nr, "HE0x04"); // CS2 high
				RS232_SendByte(cport_nr, 0);
				Sleep(5);
			}
			else { // V1.0 PCB
				RS232_cputs(cport_nr, "LD0x90"); // WR, CS2 low
				RS232_SendByte(cport_nr, 0);
				Sleep(5);

				RS232_cputs(cport_nr, "HD0x90"); // WR, CS2 high
				RS232_SendByte(cport_nr, 0);
				Sleep(5);
			}

			Sleep(50);
			RS232_cputs(cport_nr, "M1"); // Enable CS/RD/WR/CS2-RST goes high after each command
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
uint8_t gba_check_sram_flash(void) {
	uint16_t currAddr = 0x0000;
	uint16_t zeroTotal = 0;
	hasFlashSave = NOT_CHECKED;

	// Special check for certain games
	if (strncmp(gameTitle, "CHUCHU ROCKE", 12) == 0) { // Chu-Chu Rocket!
		return SRAM_FLASH_512KBIT;
	}

	// Pre-read SRAM/Flash (if the cart has an EEPROM, sometimes D0-D7 come back with random data in the first 64 bytes read)
	set_number(currAddr, SET_START_ADDRESS);
	set_mode(GBA_READ_SRAM);
	com_read_bytes(READ_BUFFER, 64);
	RS232_cputs(cport_nr, "0"); // Stop read

	// Test if SRAM is present, read 32 sections of RAM (64 bytes each)
	for (uint8_t x = 0; x < 32; x++) {
		set_number(currAddr, SET_START_ADDRESS);
		set_mode(GBA_READ_SRAM);

		com_read_bytes(READ_BUFFER, 64);
		RS232_cputs(cport_nr, "0"); // Stop read

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
		currAddr = 0x0000; // Set start and end address
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
				RS232_cputs(cport_nr, "1");
			}
		}
		RS232_cputs(cport_nr, "0");

		if (zeroTotal == 32768) {
			return 0;
		}
	}


	// Calculate size by checking different addresses (Test 256Kbit or 512Kbit)
	uint16_t duplicateCount = 0;
	char firstBuffer[65];
	char secondBuffer[65];
	for (uint8_t x = 0; x < 32; x++) {
		set_number((uint32_t)(x * 0x400), SET_START_ADDRESS);
		set_mode(GBA_READ_SRAM);
		com_read_bytes(READ_BUFFER, 64);
		memcpy(&firstBuffer, readBuffer, 64);
		RS232_cputs(cport_nr, "0"); // Stop read

		set_number((uint32_t)(x * 0x400) + 0x8000, SET_START_ADDRESS);
		set_mode(GBA_READ_SRAM);
		com_read_bytes(READ_BUFFER, 64);
		memcpy(&secondBuffer, readBuffer, 64);
		RS232_cputs(cport_nr, "0"); // Stop read

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
	hasFlashSave = gba_test_sram_flash_write();
	if (hasFlashSave == NO_FLASH) {
		return SRAM_FLASH_512KBIT;
	}

	// Test 512Kbit or 1Mbit Flash, read first 64 bytes on bank 1 then bank 2 and compare
	else {
		duplicateCount = 0;

		for (uint8_t x = 0; x < 32; x++) {
			// Read bank 0
			set_number((uint32_t)(x * 0x400), SET_START_ADDRESS);
			set_mode(GBA_READ_SRAM);
			com_read_bytes(READ_BUFFER, 64);
			memcpy(&firstBuffer, readBuffer, 64);
			RS232_cputs(cport_nr, "0"); // Stop read

			// Read bank 1
			set_number(1, GBA_FLASH_SET_BANK); // Set bank 1

			set_number((uint32_t)(x * 0x400), SET_START_ADDRESS);
			set_mode(GBA_READ_SRAM);
			com_read_bytes(READ_BUFFER, 64);
			memcpy(&secondBuffer, readBuffer, 64);
			RS232_cputs(cport_nr, "0"); // Stop read

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
void flash_4k_sector_erase(uint8_t sector) {
	set_number(sector, GBA_FLASH_4K_SECTOR_ERASE);
}

// Check if an EEPROM is present and test the size. A 4Kbit EEPROM when accessed like a 64Kbit EEPROM sends the first 8 bytes over
// and over again. A cartridge that doesn't have an EEPROM reads all 0x00 or 0xFF.
uint8_t gba_check_eeprom(void) {
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
			RS232_cputs(cport_nr, "1");
		}
	}
	RS232_cputs(cport_nr, "0"); // Stop read

	if (zeroTotal >= 512) { // Blank, likely no EEPROM
		return EEPROM_NONE;
	}
	if (repeatedCount >= 400) { // Likely a 4K EEPROM is present
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
				RS232_cputs(cport_nr, "1");
			}
		}

		// Read second 512 bytes
		endAddr = 0x400;
		RS232_cputs(cport_nr, "1"); // Request 8 bytes more

		uint8_t eepromSecondBuffer[0x200];
		while (currAddr < endAddr) {
			com_read_bytes(READ_BUFFER, 8);
			memcpy(&eepromSecondBuffer[currAddr - 0x200], readBuffer, 8);

			currAddr += 8;

			// Request 8 bytes more
			if (currAddr < endAddr) {
				RS232_cputs(cport_nr, "1");
			}
		}

		// Compare 512 bytes
		repeatedCount = 0;
		for (uint16_t c = 0; c < 0x200; c++) {
			if (eepromFirstBuffer[c] == eepromSecondBuffer[c]) {
				repeatedCount++;
			}
		}
		RS232_cputs(cport_nr, "0"); // Stop reading

		if (repeatedCount >= 512) {
			return EEPROM_4KBIT;
		}

		return EEPROM_64KBIT;
	}
}


// ****** GBA Cart Flasher Functions Start ******

// GBA Flash Cart, write address and byte
void gba_flash_write_address_byte(uint32_t address, uint16_t byte) {
	// Divide address by 2 as one address has 16 bytes of data
	address /= 2;

	char AddrString[20];
	sprintf(AddrString, "%c%x", 'n', address);
	RS232_cputs(cport_nr, AddrString);
	RS232_SendByte(cport_nr, 0);
	Sleep(5);

	char byteString[15];
	sprintf(byteString, "%c%x", 'n', byte);
	RS232_cputs(cport_nr, byteString);
	RS232_SendByte(cport_nr, 0);
	Sleep(5);

	com_wait_for_ack();
}

// ****** GBA Cart Flasher Functions End ******



// Check for Intel based flash carts
uint8_t gba_detect_intel_flash_cart(void) {
	// Set to reading mode
	gba_flash_write_address_byte(0x00, 0xFF);
	Sleep(5);

	// Read rom a tiny bit before writing
	set_number(0x00, SET_START_ADDRESS);
	set_mode(GBA_READ_ROM);
	com_read_bytes(READ_BUFFER, 64);
	com_read_stop();

	// Flash ID command
	gba_flash_write_address_byte(0x00, 0x90);
	Sleep(1);

	// Read ID
	set_number(0x00, SET_START_ADDRESS);
	set_mode(GBA_READ_ROM);
	com_read_bytes(READ_BUFFER, 64);
	com_read_stop(); // End read

	// Check Manufacturer/Chip ID
	if ((readBuffer[0] == 0x8A && readBuffer[1] == 0 && readBuffer[2] == 0x15 && readBuffer[3] == 0x88) ||
		(readBuffer[0] == 0x20 && readBuffer[1] == 0 && readBuffer[2] == 0xC4 && readBuffer[3] == 0x88)) {

		// Back to reading mode
		gba_flash_write_address_byte(currAddr, 0xFF);
		Sleep(5);

		return FLASH_FOUND_INTEL;
	}
	else {
		// Back to reading mode
		gba_flash_write_address_byte(currAddr, 0xFF);
		Sleep(5);

		return 0;
	}
}

// Read the first 192 bytes of ROM, read the title, check and test for ROM, SRAM, EEPROM and Flash
__declspec(dllexport) char* read_gba_header(int &headerlength) {
	set_mode('0'); // Break out of any loops on ATmega

	gbxcartPcbVersion = request_value(READ_PCB_VERSION);
	gbxcartFirmwareVersion = request_value(READ_FIRMWARE_VERSION);

	char* headerText;
	headerText = (char*)malloc(500);

	uint8_t logoCheck = 0;
	uint8_t logoCheckCounter = 0;
	uint8_t startRomBuffer[385];
	while (logoCheck == 0) {
		currAddr = 0x0000;
		endAddr = 0x00BF;
		set_number(currAddr, SET_START_ADDRESS);
		set_mode(GBA_READ_ROM);

		while (currAddr < endAddr) {
			uint8_t comReadBytes = com_read_bytes(READ_BUFFER, 64);

			if (comReadBytes == 64) {
				memcpy(&startRomBuffer[currAddr], readBuffer, 64);
				currAddr += 64;

				// Request 64 bytes more
				if (currAddr < endAddr) {
					com_read_cont();
				}
			}
			else { // Didn't receive 64 bytes, usually this only happens for Apple MACs
				com_read_stop();
				Sleep(500);

				// Flush buffer
				RS232_PollComport(cport_nr, readBuffer, 64);

				// Start off where we left off
				set_number(currAddr / 2, SET_START_ADDRESS);
				set_mode(GBA_READ_ROM);
			}
		}
		com_read_stop();

		// Nintendo Logo Check
		logoCheck = 1;
		for (uint16_t logoAddress = 0x04; logoAddress <= 0x9F; logoAddress++) {
			if (nintendoLogoGBA[(logoAddress - 0x04)] != startRomBuffer[logoAddress]) {
				logoCheck = 0;
				printf(".");
				break;
			}
		}

		Sleep(250);
		logoCheckCounter++;

		if (logoCheckCounter >= 20) {
			break;
		}
	}

	// Blank out game title
	for (uint8_t b = 0; b < 16; b++) {
		gameTitle[b] = 0;
	}
	// Read cartridge title and check for non-printable text
	for (uint16_t titleAddress = 0xA0; titleAddress <= 0xAB; titleAddress++) {
		char headerChar = startRomBuffer[titleAddress];
		if ((headerChar >= 0x30 && headerChar <= 0x39) || // 0-9
			(headerChar >= 0x41 && headerChar <= 0x5A) || // A-Z
			(headerChar >= 0x61 && headerChar <= 0x7A) || // a-z
			(headerChar >= 0x24 && headerChar <= 0x29) || // #$%&'()
			(headerChar == 0x2D) || // -
			(headerChar == 0x2E) || // .
			(headerChar == 0x5F) || // _
			(headerChar == 0x20)) { // Space
			gameTitle[(titleAddress - 0xA0)] = headerChar;
		}
		// Replace with an underscore
		else if (headerChar == 0x3A) { //  : 
			gameTitle[(titleAddress - 0xA0)] = '_';
		}
		else {
			gameTitle[(titleAddress - 0xA0)] = '\0';
			break;
		}
	}
	printf("Game title: %s\n", gameTitle);
	sprintf(headerText, "Game title: %s\n", gameTitle);

	// ROM size
	printf("Calculating ROM size");
	romSize = gba_check_rom_size();

	// EEPROM check
	printf("\nChecking for EEPROM");

	// Check if we have a Intel flash cart, if so, skip the EEPROM check as it can interfer with reading the last 2MB of the ROM
	if (gbxcartFirmwareVersion >= 10) {
		if (gba_detect_intel_flash_cart() == FLASH_FOUND_INTEL) {
			printf("... Skipping, Intel Flash cart detected");
			eepromSize = 0;
		}
		else {
			eepromSize = gba_check_eeprom();
		}
	}
	else {
		eepromSize = gba_check_eeprom();
	}

	// SRAM/Flash check/size, if no EEPROM present
	if (eepromSize == 0) {
		printf("\nCalculating SRAM/Flash size");
		ramSize = gba_check_sram_flash();
	}
	else {
		ramSize = 0;
	}

	// If file exists, we know the ram has been erased before, so read memory info from this file
	load_cart_ram_info();

	// Print out
	printf("\nROM size: %iMByte\n", romSize);
	sprintf(headerText + strlen(headerText), "ROM size: %iMByte\n", romSize);
	romEndAddr = ((1024 * 1024) * romSize);

	if (hasFlashSave >= FLASH_FOUND) {
		printf("Flash size: ");
		sprintf(headerText + strlen(headerText), "Flash size: ");
	}
	else {
		printf("SRAM/Flash size: ");
		sprintf(headerText + strlen(headerText), "SRAM/Flash size: ");
	}

	if (ramSize == 0) {
		ramEndAddress = 0;
		printf("None\n");
		sprintf(headerText + strlen(headerText), "None\n");
	}
	else if (ramSize == 1) {
		ramEndAddress = 0x8000;
		ramBanks = 1;
		printf("256Kbit\n");
		sprintf(headerText + strlen(headerText), "256Kbit\n");
	}
	else if (ramSize == 2) {
		ramEndAddress = 0x10000;
		ramBanks = 1;
		printf("512Kbit\n");
		sprintf(headerText + strlen(headerText), "512Kbit\n");
	}
	else if (ramSize == 3) {
		ramEndAddress = 0x10000;
		ramBanks = 2;
		printf("1Mbit\n");
		sprintf(headerText + strlen(headerText), "1Mbit\n");
	}

	printf("EEPROM: ");
	sprintf(headerText + strlen(headerText), "EEPROM: ");

	if (eepromSize == EEPROM_NONE) {
		eepromEndAddress = 0;
		printf("None\n");
		sprintf(headerText + strlen(headerText), "None\n");
	}
	else if (eepromSize == EEPROM_4KBIT) {
		eepromEndAddress = 0x200;
		printf("4Kbit\n");
		sprintf(headerText + strlen(headerText), "4Kbit\n");
	}
	else if (eepromSize == EEPROM_64KBIT) {
		eepromEndAddress = 0x2000;
		printf("64Kbit\n");
		sprintf(headerText + strlen(headerText), "64Kbit\n");
	}

	printf("Logo check: ");
	sprintf(headerText + strlen(headerText), "Logo check: ");
	if (logoCheck == 1) {
		printf("OK\n");
		sprintf(headerText + strlen(headerText), "OK");
	}
	else {
		printf("Failed\n");
		sprintf(headerText + strlen(headerText), "Failed");
	}

	headerlength = strlen(headerText);
	return headerText;
}


// Read ROM
__declspec(dllexport) void read_rom(uint8_t cartMode, uint32_t &progress, uint8_t &cancelOperation) {
	set_mode('0'); // Break out of any loops on ATmega
	writingSelected = 0;

	//cartridgeMode = read_cartridge_mode();
	printf("\n--- Dump ROM ---\n");

	char titleFilename[250];
	strncpy(titleFilename, currentFolder, 219);
	strncat(titleFilename, "\\", 3);
	strncat(titleFilename, gameTitle, 20);
	if (cartMode == GB_MODE) {
		strncat(titleFilename, ".gb", 3);
	}
	else {
		strncat(titleFilename, ".gba", 4);
	}

	printf("Dumping ROM to %s\n", titleFilename);
	printf("[             25%%             50%%             75%%            100%%]\n[");

	// Create a new file
	FILE *romFile = fopen(titleFilename, "wb");

	uint32_t readBytes = 0;
	if (cartMode == GB_MODE) {
		// Set start and end address
		currAddr = 0x0000;
		endAddr = 0x7FFF;

		xmas_setup((romBanks * 16384) / 28);

		// Read ROM
		for (uint16_t bank = 1; bank < romBanks; bank++) {
			if (romBanks > 2) {
				if (cartridgeType >= 5) { // MBC2 and above
					set_bank(0x2100, bank);
					if (bank >= 256) {
						set_bank(0x3000, 1); // High bit
					}
				}
				else { // MBC1
					if ((strncmp(gameTitle, "MOMOCOL", 7) == 0) || (strncmp(gameTitle, "BOMCOL", 6) == 0)) { // MBC1 Hudson
						set_bank(0x4000, bank >> 4);
						if (bank < 10) {
							set_bank(0x2000, bank & 0x1F);
						}
						else {
							set_bank(0x2000, 0x10 | (bank & 0x1F));
						}
					}
					else { // Regular MBC1
						set_bank(0x6000, 0); // Set ROM Mode 
						set_bank(0x4000, bank >> 5); // Set bits 5 & 6 (01100000) of ROM bank
						set_bank(0x2000, bank & 0x1F); // Set bits 0 & 4 (00011111) of ROM bank
					}
				}
			}

			if (bank > 1) { currAddr = 0x4000; }

			// Set start address and rom reading mode
			set_number(currAddr, SET_START_ADDRESS);
			set_mode(READ_ROM_RAM);

			// Read data
			while (currAddr < endAddr) {
				//if (!com_read_bytes(romFile, 64)) {
				//	cancelOperation = 1;
				//}
				/*com_read_bytes(romFile, 64);
				currAddr += 64;
				readBytes += 64;

				// Request 256 bytes more
				if (currAddr < endAddr) {
					RS232_cputs(cport_nr, "1");
				}*/

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
					Sleep(500);
					printf("Retrying\n");

					// Flush buffer
					RS232_PollComport(cport_nr, readBuffer, 64);

					// Start off where we left off
					fseek(romFile, readBytes, SEEK_SET);
					set_number(currAddr, SET_START_ADDRESS);
					set_mode(READ_ROM_RAM);
				}

				// Print progress
				print_progress_percent(progress, readBytes, (romBanks * 16384));
				led_progress_percent(readBytes, (romBanks * 16384) / 28);

				if (cancelOperation == 1) {
					break;
				}
			}
			com_read_stop(); // Stop reading ROM (as we will bank switch)

			if (cancelOperation == 1) {
				break;
			}
		}
		printf("]");
	}
	else { // GBA mode
		   // Set start and end address
		currAddr = 0x00000;
		endAddr = romEndAddr;
		set_number(currAddr, SET_START_ADDRESS);
		xmas_setup(endAddr / 28);

		uint16_t readLength = 64; 
		set_mode(GBA_READ_ROM);
		/*if (gbxcartPcbVersion != PCB_1_0) {
			set_mode(GBA_READ_ROM_256BYTE);
			readLength = 256;
		}
		else {
			set_mode(GBA_READ_ROM);
		}*/

		// Read data
		while (currAddr < endAddr) {
			int comReadBytes = com_read_bytes(romFile, readLength);
			if (comReadBytes == readLength) {
				currAddr += readLength;

				// Request 64 bytes more
				if (currAddr < endAddr) {
					com_read_cont();
				}
			}
			else { // Didn't receive 64 bytes
				fflush(romFile);
				com_read_stop();
				Sleep(500);
				printf("Retrying\n");

				// Flush buffer
				RS232_PollComport(cport_nr, readBuffer, readLength);

				// Start off where we left off
				fseek(romFile, currAddr, SEEK_SET);
				set_number(currAddr / 2, SET_START_ADDRESS);
				//if (gbxcartPcbVersion != PCB_1_0) {
				//	set_mode(GBA_READ_ROM_256BYTE);
				//}
				//else {
					set_mode(GBA_READ_ROM);
				//}
			}

			// Print progress
			print_progress_percent(progress, currAddr, endAddr);
			led_progress_percent(currAddr, endAddr / 28);

			if (cancelOperation == 1) {
				break;
			}
		}
		printf("]");
		com_read_stop(); // Stop reading
	}

	fclose(romFile);
	printf("\nFinished\n");
}


__declspec(dllexport) int check_if_file_exists() {
	char titleFilename[250];
	strncpy(titleFilename, currentFolder, 219);
	strncat(titleFilename, "\\", 3);
	strncat(titleFilename, gameTitle, 20);
	strncat(titleFilename, ".sav", 4);

	FILE *testFile = fopen(titleFilename, "rb");
	if (testFile != NULL) {
		fclose(testFile);
		return 1;
	}
	return 0;
}

// Read RAM
__declspec(dllexport) void read_ram(int saveAsNewFile, uint32_t &progress, uint8_t &cancelOperation) {
	cartridgeMode = read_cartridge_mode();
	gbxcartFirmwareVersion = request_value(READ_FIRMWARE_VERSION);
	writingSelected = 0;

	printf("\n--- Save RAM to PC---\n");

	if (cartridgeMode == GB_MODE) {
		// Does cartridge have RAM
		if (ramEndAddress > 0) {
			char titleFilename[250];
			strncpy(titleFilename, currentFolder, 219);
			strncat(titleFilename, "\\", 3);
			strncat(titleFilename, gameTitle, 20);

			// Add date/time to save file
			if (saveAsNewFile == true) {
				time_t rawtime;
				struct tm* timeinfo;
				char timebuffer[25];

				time(&rawtime);
				timeinfo = localtime(&rawtime);
				strftime(timebuffer, 80, "_%Y.%m.%d-%H.%M.%S", timeinfo);

				strncat(titleFilename, timebuffer, 25);
			}
			strncat(titleFilename, ".sav", 4);
		
			printf("Saving RAM to %s\n", titleFilename);
			printf("[             25%%             50%%             75%%            100%%]\n[");

			// Create a new file
			FILE *ramFile = fopen(titleFilename, "wb");

			mbc2_fix();
			if (cartridgeType <= 4) { // MBC1
				set_bank(0x6000, 1); // Set RAM Mode
			}
			set_bank(0x0000, 0x0A); // Initialise MBC

			// Check if Gameboy Camera cart with v1.0/1.1 PCB with R1 firmware, read data slower
			if (cartridgeType == 252 && gbxcartFirmwareVersion == 1) {
				// Read RAM
				uint32_t readBytes = 0;
				for (uint8_t bank = 0; bank < ramBanks; bank++) {
					uint16_t ramAddress = 0xA000;
					set_bank(0x4000, bank);
					set_number(ramAddress, SET_START_ADDRESS); // Set start address again

					RS232_cputs(cport_nr, "M0"); // Disable CS/RD/WR/CS2-RST from going high after each command
					Sleep(5);

					set_mode(GB_CART_MODE);

					while (ramAddress < ramEndAddress) {
						for (uint8_t x = 0; x < 64; x++) {

							char hexNum[7];
							sprintf(hexNum, "HA0x%x", ((ramAddress + x) >> 8));
							RS232_cputs(cport_nr, hexNum);
							RS232_SendByte(cport_nr, 0);

							sprintf(hexNum, "HB0x%x", ((ramAddress + x) & 0xFF));
							RS232_cputs(cport_nr, hexNum);
							RS232_SendByte(cport_nr, 0);

							RS232_cputs(cport_nr, "LD0x60"); // cs_mreqPin_low + rdPin_low
							RS232_SendByte(cport_nr, 0);

							RS232_cputs(cport_nr, "DC");

							RS232_cputs(cport_nr, "HD0x60"); // cs_mreqPin_high + rdPin_high
							RS232_SendByte(cport_nr, 0);

							RS232_cputs(cport_nr, "LA0xFF");
							RS232_SendByte(cport_nr, 0);

							RS232_cputs(cport_nr, "LB0xFF");
							RS232_SendByte(cport_nr, 0);
						}

						com_read_bytes(ramFile, 64);

						ramAddress += 64;
						readBytes += 64;

						// Request 64 bytes more
						if (ramAddress < ramEndAddress) {
							RS232_cputs(cport_nr, "1");
						}

						// Print progress
						print_progress_percent(progress, readBytes, (ramBanks * (ramEndAddress - 0xA000 + 1)));

						if (cancelOperation == 1) {
							break;
						}
					}
					RS232_cputs(cport_nr, "0"); // Stop reading RAM (as we will bank switch)
					if (cancelOperation == 1) {
						break;
					}
				}

				RS232_cputs(cport_nr, "M1");
			}

			else {
				if (ramEndAddress == 0xA1FF) {
					xmas_setup(ramEndAddress / 28);
				}
				else if (ramEndAddress == 0xA7FF) {
					xmas_setup(ramEndAddress / 4 / 28);
				}
				else {
					xmas_setup((ramBanks * (ramEndAddress - 0xA000 + 1)) / 28);
				}
				
				// Read RAM
				uint32_t readBytes = 0;
				for (uint8_t bank = 0; bank < ramBanks; bank++) {
					uint16_t ramAddress = 0xA000;
					set_bank(0x4000, bank);
					set_number(ramAddress, SET_START_ADDRESS); // Set start address again
					set_mode(READ_ROM_RAM); // Set rom/ram reading mode

					while (ramAddress < ramEndAddress) {
						/*com_read_bytes(ramFile, 64);
						ramAddress += 64;
						readBytes += 64;

						// Request 64 bytes more
						if (ramAddress < ramEndAddress) {
							RS232_cputs(cport_nr, "1");
						}*/

						uint8_t comReadBytes = com_read_bytes(ramFile, 64);
						if (comReadBytes == 64) {
							ramAddress += 64;
							readBytes += 64;

							// Request 64 bytes more
							if (ramAddress < ramEndAddress) {
								com_read_cont();
							}
						}
						else { // Didn't receive 64 bytes, usually this only happens for Apple MACs
							fflush(ramFile);
							com_read_stop();
							Sleep(500);
							printf("Retrying\n");

							// Flush buffer
							RS232_PollComport(cport_nr, readBuffer, 64);

							// Start off where we left off
							fseek(ramFile, readBytes, SEEK_SET);
							set_number(ramAddress, SET_START_ADDRESS);
							set_mode(READ_ROM_RAM);
						}

						// Print progress
						print_progress_percent(progress, readBytes, (ramBanks * (ramEndAddress - 0xA000 + 1)));
						led_progress_percent(readBytes, (ramBanks * (ramEndAddress - 0xA000 + 1)) / 28);

						if (cancelOperation == 1) {
							break;
						}
					}
					com_read_stop(); // Stop reading RAM (as we will bank switch)
					if (cancelOperation == 1) {
						break;
					}
				}
			}
			printf("]");

			set_bank(0x0000, 0x00); // Disable RAM

			fclose(ramFile);
			printf("\nFinished\n");
		}
		else {
			printf("Cartridge has no RAM\n");
		}
	}

	else { // GBA mode
		// Does cartridge have RAM
		if (ramEndAddress > 0 || eepromEndAddress > 0) {
			char titleFilename[250];
			strncpy(titleFilename, currentFolder, 219);
			strncat(titleFilename, "\\", 3);
			strncat(titleFilename, gameTitle, 20);

			// Add date/time to save file
			if (saveAsNewFile == true) {
				time_t rawtime;
				struct tm* timeinfo;
				char timebuffer[25];

				time(&rawtime);
				timeinfo = localtime(&rawtime);
				strftime(timebuffer, 80, "_%Y.%m.%d-%H.%M.%S", timeinfo);

				strncat(titleFilename, timebuffer, 25);
			}
			strncat(titleFilename, ".sav", 4);

			// Create a new file
			FILE *ramFile = fopen(titleFilename, "wb");

			// SRAM/Flash
			if (ramEndAddress > 0) {
				printf("Saving RAM (SRAM/Flash) to %s\n", titleFilename);
				printf("[             25%%             50%%             75%%            100%%]\n[");

				xmas_setup((ramBanks * ramEndAddress) / 28);

				// Read RAM
				uint32_t readBytes = 0;
				for (uint8_t bank = 0; bank < ramBanks; bank++) {
					// Flash, switch bank 1
					if (bank == 1) {
						set_number(1, GBA_FLASH_SET_BANK);
					}
					
					// Set start and end address
					currAddr = 0x00000;
					endAddr = ramEndAddress;
					set_number(currAddr, SET_START_ADDRESS);
					set_mode(GBA_READ_SRAM);

					while (currAddr < endAddr) {
						/*com_read_bytes(ramFile, 64);
						currAddr += 64;
						readBytes += 64;

						// Request 64 bytes more
						if (currAddr < endAddr) {
							RS232_cputs(cport_nr, "1");
						}*/

						uint8_t comReadBytes = com_read_bytes(ramFile, 64);
						if (comReadBytes == 64) {
							currAddr += 64;
							readBytes += 64;

							// Request 64 bytes more
							if (currAddr < endAddr) {
								com_read_cont();
							}
						}
						else { // Didn't receive 64 bytes, usually this only happens for Apple MACs
							fflush(ramFile);
							com_read_stop();
							Sleep(500);
							printf("Retrying\n");

							// Flush buffer
							RS232_PollComport(cport_nr, readBuffer, 64);

							// Start off where we left off
							fseek(ramFile, currAddr, SEEK_SET);
							set_number(currAddr, SET_START_ADDRESS);
							set_mode(GBA_READ_SRAM);
						}

						print_progress_percent(progress, readBytes, ramBanks * endAddr);
						led_progress_percent(readBytes, (ramBanks * ramEndAddress) / 28);

						if (cancelOperation == 1) {
							break;
						}
					}
					com_read_stop(); // End read (for this bank if Flash)

					// Flash, switch back to bank 0
					if (bank == 1) {
						set_number(0, GBA_FLASH_SET_BANK);
					}
					if (cancelOperation == 1) {
						break;
					}
				}
			}

			// EEPROM
			else {
				printf("Saving RAM (EEPROM) to %s\n", titleFilename);
				printf("[             25%%             50%%             75%%            100%%]\n[");

				xmas_setup(eepromEndAddress / 28);
				set_number(eepromSize, GBA_SET_EEPROM_SIZE);

				// Set start and end address
				currAddr = 0x000;
				endAddr = eepromEndAddress;
				set_number(currAddr, SET_START_ADDRESS);
				set_mode(GBA_READ_EEPROM);

				// Read EEPROM
				uint32_t readBytes = 0;
				while (currAddr < endAddr) {
					com_read_bytes(ramFile, 8);
					currAddr += 8;
					readBytes += 8;

					// Request 64 bytes more
					if (currAddr < endAddr) {
						RS232_cputs(cport_nr, "1");
					}

					print_progress_percent(progress, readBytes, endAddr);
					led_progress_percent(readBytes, endAddr / 28);
					if (cancelOperation == 1) {
						break;
					}
				}
				RS232_cputs(cport_nr, "0"); // Stop reading
			}
			fclose(ramFile); 
			printf("]");			
			printf("\nFinished\n");
		
		}
		else {
			printf("Cartridge has no RAM\n");
		}
	}
}

// Write RAM
__declspec(dllexport) void write_ram(char* writeSaveFileName, uint32_t &progress, uint8_t &cancelOperation) {
	cartridgeMode = read_cartridge_mode();
	writingSelected = 1;
	printf("\n--- Write RAM to GB Cart ---\n");

	if (cartridgeMode == GB_MODE) {
		// Does cartridge have RAM
		if (ramEndAddress > 0) {
			char titleFilename[250];
			
			if (strlen(writeSaveFileName) >= 5) {
				strncpy(titleFilename, writeSaveFileName, 249);
			}
			else {
				strncpy(titleFilename, currentFolder, 219);
				strncat(titleFilename, "\\", 3);
				strncat(titleFilename, gameTitle, 20);
				strncat(titleFilename, ".sav", 4);
			}

			// Open file
			FILE *ramFile = fopen(titleFilename, "rb");
			if (ramFile != NULL) {
				printf("\nWriting to RAM from %s\n", titleFilename);
				printf("[             25%%             50%%             75%%            100%%]\n[");

				mbc2_fix();
				if (cartridgeType <= 4) { // MBC1
					set_bank(0x6000, 1); // Set RAM Mode
				}
				set_bank(0x0000, 0x0A); // Initialise MBC

				xmas_setup((ramBanks * (ramEndAddress - 0xA000 + 1)) / 28);

				// Write RAM
				uint32_t readBytes = 0;
				for (uint8_t bank = 0; bank < ramBanks; bank++) {
					uint16_t ramAddress = 0xA000;
					set_bank(0x4000, bank);
					set_number(0xA000, SET_START_ADDRESS); // Set start address again

					while (ramAddress < ramEndAddress) {
						com_write_bytes_from_file(WRITE_RAM, ramFile, 64);
						ramAddress += 64;
						readBytes += 64;

						// Print progress
						print_progress_percent(progress, readBytes, (ramBanks * (ramEndAddress - 0xA000 + 1)));
						led_progress_percent(readBytes, (ramBanks * (ramEndAddress - 0xA000 + 1)) / 28);
						com_wait_for_ack();

						if (cancelOperation == 1) {
							break;
						}
					}
					if (cancelOperation == 1) {
						break;
					}
				}
				printf("]");
				set_bank(0x0000, 0x00); // Disable RAM

				fclose(ramFile);
				printf("\nFinished\n");
			}
			else {
				printf("%s File not found\n", titleFilename);
			}
		}
		else {
			printf("Cartridge has no RAM\n");
		}
	}
	else { // GBA mode
		   // Does cartridge have RAM
		if (ramEndAddress > 0 || eepromEndAddress > 0) {
			char titleFilename[250];

			if (strlen(writeSaveFileName) >= 5) {
				strncpy(titleFilename, writeSaveFileName, 249);
			}
			else {
				strncpy(titleFilename, currentFolder, 219);
				strncat(titleFilename, "\\", 3);
				strncat(titleFilename, gameTitle, 20);
				strncat(titleFilename, ".sav", 4);
			}

			// Open file
			FILE *ramFile = fopen(titleFilename, "rb");
			if (ramFile != NULL) {

				// SRAM/Flash or EEPROM
				if (eepromSize == EEPROM_NONE) {
					// Check if it's SRAM or Flash (if we haven't checked before)
					if (hasFlashSave == NOT_CHECKED) {
						hasFlashSave = gba_test_sram_flash_write();
					}

					if (hasFlashSave >= FLASH_FOUND) {
						printf("Going to write to RAM (Flash) from %s", titleFilename);
					}
					else {
						printf("Going to write to RAM (SRAM) from %s", titleFilename);
					}
				}
				else {
					printf("Going to write to RAM (EEPROM) from %s", titleFilename);
				}

				if (eepromSize == EEPROM_NONE) {
					if (hasFlashSave >= FLASH_FOUND) {
						printf("\nWriting to RAM (Flash) from %s", titleFilename);
					}
					else {
						printf("\nWriting to RAM (SRAM) from %s", titleFilename);
					}
				}
				else {
					printf("\nWriting to RAM (EEPROM) from %s", titleFilename);
				}
				printf("\n[             25%%             50%%             75%%            100%%]\n[");

				// SRAM
				if (hasFlashSave == NO_FLASH && eepromSize == EEPROM_NONE) {
					xmas_setup(ramEndAddress / 28);
					
					// Set start and end address
					currAddr = 0x0000;
					endAddr = ramEndAddress;
					set_number(currAddr, SET_START_ADDRESS);

					// Write
					uint32_t readBytes = 0;
					while (currAddr < endAddr) {
						com_write_bytes_from_file(GBA_WRITE_SRAM, ramFile, 64);
						currAddr += 64;
						readBytes += 64;

						print_progress_percent(progress, readBytes, ramEndAddress);
						com_wait_for_ack();

						if (cancelOperation == 1) {
							break;
						}
					}
				}

				// EEPROM
				else if (eepromSize != EEPROM_NONE) {
					xmas_setup(eepromEndAddress / 28);
					set_number(eepromSize, GBA_SET_EEPROM_SIZE);

					// Set start and end address
					currAddr = 0x000;
					endAddr = eepromEndAddress;
					set_number(currAddr, SET_START_ADDRESS);

					// Write
					uint32_t readBytes = 0;
					while (currAddr < endAddr) {
						com_write_bytes_from_file(GBA_WRITE_EEPROM, ramFile, 8);
						currAddr += 8;
						readBytes += 8;

						print_progress_percent(progress, readBytes, endAddr);
						led_progress_percent(readBytes, endAddr / 28);

						// Wait for ATmega to process write (~320us) and for EEPROM to write data (6ms)
						com_wait_for_ack();

						if (cancelOperation == 1) {
							break;
						}
					}
				}

				// Flash
				else if (hasFlashSave != NO_FLASH) {
					xmas_setup((ramBanks * endAddr) / 28);

					uint32_t readBytes = 0;
					for (uint8_t bank = 0; bank < ramBanks; bank++) {
						// Set start and end address
						currAddr = 0x0000;
						endAddr = ramEndAddress;
						set_number(currAddr, SET_START_ADDRESS);

						// Program flash in 128 bytes at a time
						if (hasFlashSave == FLASH_FOUND_ATMEL) {
							while (currAddr < endAddr) {
								com_write_bytes_from_file(GBA_FLASH_WRITE_ATMEL, ramFile, 128);
								currAddr += 128;
								readBytes += 128;

								print_progress_percent(progress, readBytes, ramBanks * endAddr);
								led_progress_percent(readBytes, (ramBanks * endAddr) / 28);
								com_wait_for_ack(); // Wait for write complete

								if (cancelOperation == 1) {
									break;
								}
							}
						}
						else { // Program flash in 1 byte at a time
							if (bank == 1) {
								set_number(1, GBA_FLASH_SET_BANK); // Set bank 1
							}

							uint8_t sector = 0;
							while (currAddr < endAddr) {
								if (currAddr % 4096 == 0) {
									flash_4k_sector_erase(sector);
									sector++;
									com_wait_for_ack(); // Wait 25ms for sector erase

									// Wait for first byte to be 0xFF, that's when we know the sector has been erased
									readBuffer[0] = 0;
									while (readBuffer[0] != 0xFF) {
										set_number(currAddr, SET_START_ADDRESS);
										set_mode(GBA_READ_SRAM);

										com_read_bytes(READ_BUFFER, 64);
										RS232_cputs(cport_nr, "0"); // End read

										if (readBuffer[0] != 0xFF) {
											Sleep(5);
										}
									}

									// Set start address again
									set_number(currAddr, SET_START_ADDRESS);

									Sleep(5); // Wait a little bit more as there is some rare cases that the hardware isn't ready for the write command
								}

								com_write_bytes_from_file(GBA_FLASH_WRITE_BYTE, ramFile, 64);
								currAddr += 64;
								readBytes += 64;

								print_progress_percent(progress, readBytes, ramBanks * endAddr);
								led_progress_percent(readBytes, (ramBanks * endAddr) / 28);

								com_wait_for_ack(); // Wait for write complete

								if (cancelOperation == 1) {
									break;
								}
							}
						}

						if (bank == 1) {
							set_number(0, GBA_FLASH_SET_BANK); // Set bank 0 again
						}
						if (cancelOperation == 1) {
							break;
						}
					}
				}
				printf("]");

				fclose(ramFile);
				printf("\nFinished\n");
				
			}
			else {
				printf("%s File not found\n", titleFilename);
			}
		}
		else {
			printf("Cartridge has no RAM\n");
		}
	}
}


// Erase memory
__declspec(dllexport) void erase_ram(uint32_t &progress, uint8_t &cancelOperation) {	
	// Default for SRAM
	writingSelected = 1;
	for (uint8_t x = 0; x < 128; x++) {
		writeBuffer[x] = 0x00;
	}

	cartridgeMode = read_cartridge_mode();
	if (cartridgeMode == GB_MODE) {
		// Does cartridge have RAM
		if (ramEndAddress > 0) {
			mbc2_fix();
			if (cartridgeType <= 4) { // MBC1
				set_bank(0x6000, 1); // Set RAM Mode
			}
			set_bank(0x0000, 0x0A); // Initialise MBC
			xmas_setup((ramBanks * (ramEndAddress - 0xA000 + 1)) / 28);

			// Erase RAM
			uint32_t readBytes = 0;
			for (uint8_t bank = 0; bank < ramBanks; bank++) {
				uint16_t ramAddress = 0xA000;
				set_bank(0x4000, bank);
				set_number(0xA000, SET_START_ADDRESS); // Set start address again

				while (ramAddress < ramEndAddress) {
					com_write_bytes_from_file(WRITE_RAM, NULL, 64);
					ramAddress += 64;
					readBytes += 64;

					// Print progress
					print_progress_percent(progress, readBytes, (ramBanks * (ramEndAddress - 0xA000 + 1)));
					led_progress_percent(readBytes, (ramBanks * (ramEndAddress - 0xA000 + 1)) / 28);

					com_wait_for_ack();

					if (cancelOperation == 1) {
						break;
					}
				}
				if (cancelOperation == 1) {
					break;
				}
			}

			set_bank(0x0000, 0x00); // Disable RAM
		}
	}
	else { // GBA mode
		// Does cartridge have RAM
		if (ramEndAddress > 0 || eepromEndAddress > 0) {
			// Check if it's SRAM or Flash (if we haven't checked before)
			if (eepromSize == 0 && hasFlashSave == NOT_CHECKED) {
				hasFlashSave = gba_test_sram_flash_write();
			}
			
			// Before erasing, make a .info file with the memory details as we won't be able to automatically detect it anymore
			// Check if file already exists
			write_cart_ram_info();

			// SRAM
			if (hasFlashSave == NO_FLASH && eepromSize == EEPROM_NONE) {
				xmas_setup(ramEndAddress / 28);

				// Set start and end address
				currAddr = 0x0000;
				endAddr = ramEndAddress;
				set_number(currAddr, SET_START_ADDRESS);

				// Write
				uint32_t readBytes = 0;
				while (currAddr < endAddr) {
					com_write_bytes_from_file(GBA_WRITE_SRAM, NULL, 64);
					currAddr += 64;
					readBytes += 64;
					com_wait_for_ack();

					print_progress_percent(progress, readBytes, ramEndAddress);
					led_progress_percent(readBytes, ramEndAddress / 28);
					
					if (cancelOperation == 1) {
						break;
					}
				}
			}

			// EEPROM
			else if (eepromSize != EEPROM_NONE) {
				xmas_setup(eepromEndAddress / 28);
				set_number(eepromSize, GBA_SET_EEPROM_SIZE);

				// Set start and end address
				currAddr = 0x000;
				endAddr = eepromEndAddress;
				set_number(currAddr, SET_START_ADDRESS);

				// Write
				uint32_t readBytes = 0;
				while (currAddr < endAddr) {
					com_write_bytes_from_file(GBA_WRITE_EEPROM, NULL, 8);
					currAddr += 8;
					readBytes += 8;
					
					// Wait for ATmega to process write (~320us) and for EEPROM to write data (6ms)
					com_wait_for_ack();

					print_progress_percent(progress, readBytes, endAddr);
					led_progress_percent(readBytes, endAddr / 28);
					
					if (cancelOperation == 1) {
						break;
					}
				}
			}

			// Flash
			else if (hasFlashSave != NO_FLASH) {
				xmas_setup((ramBanks * endAddr) / 28);
				
				uint32_t readBytes = 0;
				for (uint8_t bank = 0; bank < ramBanks; bank++) {
					// Set start and end address
					currAddr = 0x0000;
					endAddr = ramEndAddress;
					set_number(currAddr, SET_START_ADDRESS);

					// Program flash in 128 bytes at a time
					if (hasFlashSave == FLASH_FOUND_ATMEL) {
						while (currAddr < endAddr) {
							com_write_bytes_from_file(GBA_FLASH_WRITE_ATMEL, NULL, 128);
							currAddr += 128;
							readBytes += 128;
							com_wait_for_ack(); // Wait for write complete

							print_progress_percent(progress, readBytes, ramBanks * endAddr);
							led_progress_percent(readBytes, (ramBanks * endAddr) / 28);

							if (cancelOperation == 1) {
								break;
							}
						}
					}
					else {
						if (bank == 1) {
							set_number(1, GBA_FLASH_SET_BANK); // Set bank 1
						}
						
						uint8_t sector = 0;
						while (currAddr < endAddr) {
							if (currAddr % 4096 == 0) {
								flash_4k_sector_erase(sector);
								sector++;
								com_wait_for_ack(); // Wait 25ms for sector erase

								// Wait for first byte to be 0xFF, that's when we know the sector has been erased
								readBuffer[0] = 0;
								while (readBuffer[0] != 0xFF) {
									set_number(currAddr, SET_START_ADDRESS);
									set_mode(GBA_READ_SRAM);

									com_read_bytes(READ_BUFFER, 64);
									RS232_cputs(cport_nr, "0"); // End read

									if (readBuffer[0] != 0xFF) {
										Sleep(5);
									}
								}
								Sleep(5); // Wait a little bit more as there is some rare cases that the hardware isn't ready for the next command
							}

							currAddr += 64;
							readBytes += 64;

							print_progress_percent(progress, readBytes, ramBanks * endAddr);
							led_progress_percent(readBytes, (ramBanks * endAddr) / 28);

							if (cancelOperation == 1) {
								break;
							}
						}
					}

					if (bank == 1) {
						set_number(0, GBA_FLASH_SET_BANK); // Set bank 0 again
					}

					if (cancelOperation == 1) {
						break;
					}
				}
			}
		}
	}
}


// ---------- GB Cart Flasher functions ----------

// Wait for first byte of chosen address to be 0xFF, that's when we know the sector has been erased
void wait_for_flash_sector_ff(uint16_t address) {
	readBuffer[0] = 0;
	while (readBuffer[0] != 0xFF) {
		set_number(address, SET_START_ADDRESS);
		set_mode(READ_ROM_RAM);

		com_read_bytes(READ_BUFFER, 64);
		com_read_stop(); // End read

		if (readBuffer[0] != 0xFF) {
			Sleep(20);
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
	else if (method == GB_FLASH_PROGRAM_5555) {
		send_hex_wait_ack(0x5555);	send_hex_wait_ack(0xAA);
		send_hex_wait_ack(0x2AAA);	send_hex_wait_ack(0x55);
		send_hex_wait_ack(0x5555);	send_hex_wait_ack(0xA0);
	}
}

// Write address and byte to flash
void gb_flash_write_address_byte(uint16_t address, uint8_t byte) {
	char AddrString[15];
	sprintf(AddrString, "%c%x", 'F', address);
	RS232_cputs(cport_nr, AddrString);
	RS232_SendByte(cport_nr, 0);
	Sleep(5);

	char byteString[15];
	sprintf(byteString, "%x", byte);
	RS232_cputs(cport_nr, byteString);
	RS232_SendByte(cport_nr, 0);
	Sleep(5);

	com_wait_for_ack();
}


__declspec(dllexport) int erase_rom(uint8_t cartType, uint32_t &progress, uint8_t &cancelOperation) {
	// GB Carts
	if (cartType <= 99) {
		// Read rom a tiny bit before writing
		currAddr = 0x0000;
		set_number(currAddr, SET_START_ADDRESS);
		set_mode(READ_ROM_RAM);
		com_read_bytes(READ_BUFFER, 64);
		RS232_cputs(cport_nr, "0"); // Stop read

		// 32K SST39SF010A/AT49F040 
		if (cartType == 6 || cartType == 7) {
			// Flash Setup
			set_mode(GB_CART_MODE); // Gameboy mode
			if (cartType == 7) {
				gb_flash_pin_setup(WE_AS_AUDIO_PIN); // Audio pin
			}
			else {
				gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			}

			// Chip erase for this flash chip
			gb_flash_write_address_byte(0x5555, 0xAA);
			gb_flash_write_address_byte(0x2AAA, 0x55);
			gb_flash_write_address_byte(0x5555, 0x80);
			gb_flash_write_address_byte(0x5555, 0xAA);
			gb_flash_write_address_byte(0x2AAA, 0x55);
			gb_flash_write_address_byte(0x5555, 0x10);
		}

		if (cartType == 2) { // 2 MByte (BV5) GB Flash Cart 
			// Flash Setup
			set_mode(GB_CART_MODE); // Gameboy mode
			gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_AAA_BIT01_SWAPPED); // Flash program byte method													   

			// Chip erase (data byte's bit 0 & 1 are swapped for chip commands as  D0 & D1 lines are swapped)
			gb_flash_write_address_byte(0xAAA, 0xA9);
			gb_flash_write_address_byte(0x555, 0x56);
			gb_flash_write_address_byte(0xAAA, 0x80);
			gb_flash_write_address_byte(0xAAA, 0xA9);
			gb_flash_write_address_byte(0x555, 0x56);
			gb_flash_write_address_byte(0xAAA, 0x10);
		}
		else if (cartType == 4) { // 1 MByte (ES29LV160) GB Flash Cart
			// Flash configured as 1M x 16 bits (Word mode)
			// Chip erase (data byte's bit 0 & 1 are swapped for chip commands as D0 & D1 lines are swapped)
			gb_flash_write_address_byte(0x555, 0xA9);
			gb_flash_write_address_byte(0x2AA, 0x56);
			gb_flash_write_address_byte(0x555, 0x80);
			gb_flash_write_address_byte(0x555, 0xA9);
			gb_flash_write_address_byte(0x2AA, 0x56);
			gb_flash_write_address_byte(0x555, 0x10);
		}
		else if (cartType == 5) { // 2 MByte (AM29LV160DB / 29LV160CTTC) GB Flash Cart
			// Flash configured as 1M x 8 bits (Byte mode)
			// Chip erase
			gb_flash_write_address_byte(0xAAA, 0xAA);
			gb_flash_write_address_byte(0x555, 0x55);
			gb_flash_write_address_byte(0xAAA, 0x80);
			gb_flash_write_address_byte(0xAAA, 0xAA);
			gb_flash_write_address_byte(0x555, 0x55);
			gb_flash_write_address_byte(0xAAA, 0x10);
		}
		else if (cartType == 8) { // 2 MByte (AM29F016B) GB Flash Cart
			gb_flash_write_address_byte(0x555, 0xAA);
			gb_flash_write_address_byte(0x2AA, 0x55);
			gb_flash_write_address_byte(0x555, 0x80);
			gb_flash_write_address_byte(0x555, 0xAA);
			gb_flash_write_address_byte(0x2AA, 0x55);
			gb_flash_write_address_byte(0x555, 0x10);
		}
		else if (cartType == 9) { // 512 KByte (SST39SF040) GB Flash Cart
			set_mode(GB_CART_MODE); // Gameboy mode
			gb_flash_pin_setup(WE_AS_AUDIO_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_5555); // Flash program byte method

			// Set bank 1 before issuing flash commands
			set_bank(0x2100, 1);

			gb_flash_write_address_byte(0x5555, 0xAA);
			gb_flash_write_address_byte(0x2AAA, 0x55);
			gb_flash_write_address_byte(0x5555, 0x80);
			gb_flash_write_address_byte(0x5555, 0xAA);
			gb_flash_write_address_byte(0x2AAA, 0x55);
			gb_flash_write_address_byte(0x5555, 0x10);
		}
		else if ((cartType >= 10 && cartType <= 13) || (cartType >= 19 && cartType <= 22)) { // 32 MByte (4x 8MB Banks) (256M29)
			// Flash Setup
			set_mode(GB_CART_MODE); // Gameboy mode
			gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_AAA_BIT01_SWAPPED); // Flash program byte method

			gb_flash_write_address_byte(0xAAA, 0xA9);
			gb_flash_write_address_byte(0x555, 0x56);
			gb_flash_write_address_byte(0xAAA, 0x80);
			gb_flash_write_address_byte(0xAAA, 0xA9);
			gb_flash_write_address_byte(0x555, 0x56);
			gb_flash_write_address_byte(0xAAA, 0x10);
		}
		else if (cartType == 14) { // 512 KByte (SST39SF040) GB Flash Cart
			set_mode(GB_CART_MODE); // Gameboy mode
			gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_AAA_BIT01_SWAPPED); // Flash program byte method

			gb_flash_write_address_byte(0xAAA, 0xA9);
			gb_flash_write_address_byte(0x555, 0x56);
			gb_flash_write_address_byte(0xAAA, 0x80);
			gb_flash_write_address_byte(0xAAA, 0xA9);
			gb_flash_write_address_byte(0x555, 0x56);
			gb_flash_write_address_byte(0xAAA, 0x10);
		}
		else if (cartType == 17) { // 4 MByte (MX29LV320) GB Flash Cart
			set_mode(GB_CART_MODE); // Gameboy mode
			gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_AAA); // Flash program byte method

			gb_flash_write_address_byte(0xAAA, 0xAA);
			gb_flash_write_address_byte(0x555, 0x55);
			gb_flash_write_address_byte(0xAAA, 0x80);
			gb_flash_write_address_byte(0xAAA, 0xAA);
			gb_flash_write_address_byte(0x555, 0x55);
			gb_flash_write_address_byte(0xAAA, 0x10);
		}


		// Wait for first byte to be 0xFF
		uint8_t waitCounter = 0;
		readBuffer[0] = 0;
		while (readBuffer[0] != 0xFF) {
			set_number(currAddr, SET_START_ADDRESS);
			Sleep(5);
			set_mode(READ_ROM_RAM);
			Sleep(5);

			com_read_bytes(READ_BUFFER, 64);
			RS232_cputs(cport_nr, "0"); // End read

			if (readBuffer[0] != 0xFF) {
				Sleep(500);

				if (waitCounter < 50) {
					waitCounter++;
				}

				print_progress_percent(progress, waitCounter, 50);
				if (cancelOperation == 1) { return 0; }
			}
			if (cancelOperation == 1) { return 0; }
		}
	}
	else { // GBA Carts
		// insideGadgets 32MB Cart (S29GL256 / S29GL512)
		if (cartType == 106) {	
			currAddr = 0x0000;
			set_number(currAddr, SET_START_ADDRESS);
			Sleep(5);

			gba_flash_write_address_byte(0xAAA, 0xAA);
			gba_flash_write_address_byte(0x555, 0x55);
			gba_flash_write_address_byte(0xAAA, 0x80);
			gba_flash_write_address_byte(0xAAA, 0xAA);
			gba_flash_write_address_byte(0x555, 0x55);
			gba_flash_write_address_byte(0xAAA, 0x10);

			// Wait for first 2 bytes to be 0xFF
			readBuffer[0] = 0;
			readBuffer[1] = 0;
			uint8_t waitCounter = 0;
			while (readBuffer[0] != 0xFF && readBuffer[1] != 0xFF) {
				set_number(currAddr / 2, SET_START_ADDRESS);
				Sleep(5);
				set_mode(GBA_READ_ROM);
				Sleep(5);

				com_read_bytes(READ_BUFFER, 64);
				com_read_stop(); // End read

				Sleep(5000);
				waitCounter++;

				print_progress_percent(progress, waitCounter, 100);
				if (cancelOperation == 1) { return 0; }
			}
		}
	}
}

__declspec(dllexport) int write_rom(char* fileName, uint8_t flashCartType, uint32_t fileSize, uint32_t &progress, uint8_t &cancelOperation) {
	uint32_t readBytes = 0;
	uint8_t sector = 0;

	FILE *romFile = fopen(fileName, "rb");
	if (romFile == NULL) {
		return 0;
	}
	
	gbxcartFirmwareVersion = request_value(READ_FIRMWARE_VERSION);

	// *** GB Flash Carts ***

	// 32K AM29F010B/SST39SF010A/AT49F040 GB Flash Cart (Audio or WR as WE)
	if (flashCartType == 1 || flashCartType == 3 || flashCartType == 6 || flashCartType == 7) {
		currAddr = 0x0000;
		endAddr = 0x7FFF;

		// Flash Setup
		set_mode(GB_CART_MODE); // Gameboy mode
		if (flashCartType == 1 || flashCartType == 7) {
			gb_flash_pin_setup(WE_AS_AUDIO_PIN); // Audio pin
		}
		else {
			gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
		}
		if (flashCartType == 1 || flashCartType == 3) {
			gb_flash_program_setup(GB_FLASH_PROGRAM_555); // Flash program byte method
		}
		else {
			gb_flash_program_setup(GB_FLASH_PROGRAM_5555); // Flash program byte method
		}

		// Write ROM
		set_number(currAddr, SET_START_ADDRESS);
		while (currAddr <= endAddr) {
			if (flashCartType == 1 || flashCartType == 3) { // Sector erase for this flash chip
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
					Sleep(5);
				}
			}

			com_write_bytes_from_file(GB_FLASH_WRITE_64BYTE, romFile, 64);
			if (!com_wait_for_ack()) { // Wait for ack, checks if stalled after 250ms
				cancelOperation = 2;
				return 0;
			}
			currAddr += 64;
			readBytes += 64;

			print_progress_percent(progress, readBytes, (endAddr + 1));

			if (cancelOperation == 1) { return 0; }
		}

		printf("]");
		fclose(romFile);
	}

	// Other GB Flash Carts
	else if (flashCartType == 2 || flashCartType == 4 || flashCartType == 5 || flashCartType == 8 || flashCartType == 9 || flashCartType == 14 || flashCartType == 17) {
		currAddr = 0x0000;
		endAddr = 0x7FFF;

		// Calculate banks needed from ROM file size
		romBanks = fileSize / 16384;
		
		// 2 MByte(BV5) GB Flash Cart
		if (flashCartType == 2) {
			set_mode(GB_CART_MODE); // Gameboy mode
			gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_AAA_BIT01_SWAPPED); // Flash program byte method
		}
		// 1 MByte (ES29LV160) GB Flash Cart
		else if (flashCartType == 4) {
			set_mode(GB_CART_MODE); // Gameboy mode
			gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_555_BIT01_SWAPPED); // Flash program byte method
		}
		// 2 MByte (AM29LV160DB / 29LV160CTTC) GB Flash Cart
		else if (flashCartType == 5) {
			set_mode(GB_CART_MODE); // Gameboy mode
			gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_AAA); // Flash program byte method
		}
		// 2 MByte (AM29F016B) GB Flash Cart
		else if (flashCartType == 8) {
			set_mode(GB_CART_MODE); // Gameboy mode
			gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_555);// Flash program byte method
		}
		// 512 KByte (SST39SF040) GB Flash Cart
		else if (flashCartType == 9) {
			set_mode(GB_CART_MODE); // Gameboy mode
			set_mode(GB_FLASH_BANK_1_COMMAND_WRITES); // Set bank 1 before issuing flash commands
			gb_flash_pin_setup(WE_AS_AUDIO_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_5555);// Flash program byte method
		}
		// 4 MByte (M29W640) GB Flash Cart
		else if (flashCartType == 14) {
			set_mode(GB_CART_MODE); // Gameboy mode
			gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_AAA_BIT01_SWAPPED);// Flash program byte method
		}
		// 4 MByte (MX29LV320) GB Flash Cart
		else if (flashCartType == 17) {
			set_mode(GB_CART_MODE); // Gameboy mode
			gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
			gb_flash_program_setup(GB_FLASH_PROGRAM_AAA);// Flash program byte method
		}

		// Write ROM
		currAddr = 0x0000;
		for (uint16_t bank = 1; bank < romBanks; bank++) {
			if (bank > 1) { currAddr = 0x4000; }

			// Set start address
			set_number(currAddr, SET_START_ADDRESS);
			Sleep(5);

			// Read data
			while (currAddr < endAddr) {
				if (currAddr == 0x4000) { // Switch banks here just before the next bank, not any time sooner
					set_bank(0x2100, bank);
				}

				com_write_bytes_from_file(GB_FLASH_WRITE_64BYTE, romFile, 64);
				if (!com_wait_for_ack()) { // Wait for ack, checks if stalled after 250ms
					cancelOperation = 2;
					return 0;
				}
				currAddr += 64;
				readBytes += 64;

				// Print progress
				print_progress_percent(progress, readBytes, (romBanks * 16384));

				if (cancelOperation == 1) { return 0; }
			}
			if (cancelOperation == 1) { return 0; }
		}

		fclose(romFile);
	}

	// 32 MByte (4x 8MB Banks) (256M29 / M29W256)
	else if ((flashCartType >= 10 && flashCartType <= 13) || (flashCartType >= 19 && flashCartType <= 22)) {
		currAddr = 0x0000;
		endAddr = 0x7FFF;

		// Calculate banks needed from ROM file size
		romBanks = fileSize / 16384;

		// Flash Setup
		set_mode(GB_CART_MODE); // Gameboy mode
		gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
		gb_flash_program_setup(GB_FLASH_PROGRAM_AAA_BIT01_SWAPPED); // Flash program byte method

		// Set 8MB bank
		if (flashCartType == 10 || flashCartType == 19) {
			gb_flash_write_address_byte(0x7000, 0x00);
			gb_flash_write_address_byte(0x7001, 0x00);
			gb_flash_write_address_byte(0x7002, 0x90);
			Sleep(1);
		}
		else if (flashCartType == 11 || flashCartType == 20) {
			gb_flash_write_address_byte(0x7000, 0x00);
			gb_flash_write_address_byte(0x7001, 0x00);
			gb_flash_write_address_byte(0x7002, 0x91);
			Sleep(1);
		}
		else if (flashCartType == 12 || flashCartType == 21) {
			gb_flash_write_address_byte(0x7000, 0x00);
			gb_flash_write_address_byte(0x7001, 0x00);
			gb_flash_write_address_byte(0x7002, 0x92);
			Sleep(1);
		}
		else if (flashCartType == 13 || flashCartType == 22) {
			gb_flash_write_address_byte(0x7000, 0x00);
			gb_flash_write_address_byte(0x7001, 0x00);
			gb_flash_write_address_byte(0x7002, 0x93);
			Sleep(1);
		}

		// Write ROM
		currAddr = 0x0000;
		for (uint16_t bank = 1; bank < romBanks; bank++) {
			if (bank > 1) { currAddr = 0x4000; }

			// Set start address
			set_number(currAddr, SET_START_ADDRESS);
			Sleep(5);

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

				if (flashCartType >= 10 && flashCartType <= 13) { // Use buffered programming
					com_write_bytes_from_file(GB_FLASH_WRITE_256BYTE, romFile, 256);
					if (!com_wait_for_ack()) { // Wait for ack, checks if stalled after 250ms
						cancelOperation = 2;
						return 0;
					}
					currAddr += 256;
					readBytes += 256;
				}
				else { // M29W256 doesn't seem to work with buffered programming, write one byte at a time
					com_write_bytes_from_file(GB_FLASH_WRITE_64BYTE, romFile, 256);
					if (!com_wait_for_ack()) { // Wait for ack, checks if stalled after 250ms
						cancelOperation = 2;
						return 0;
					}
					currAddr += 64;
					readBytes += 64;
				}

				// Print progress
				print_progress_percent(progress, readBytes, (romBanks * 16384));

				if (cancelOperation == 1) { return 0; }
			}
			if (cancelOperation == 1) { return 0; }
		}

		fclose(romFile);
	}

	// 64 MByte (S29GL512) / 64 MByte (S29GL512) Buffered (Experimental)
	else if (flashCartType >= 15 && flashCartType <= 16) {
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
			}
			fseek(romFile, 0x00, SEEK_SET);
		}

		// Flash Setup
		set_mode(GB_CART_MODE); // Gameboy mode
		gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
		gb_flash_program_setup(GB_FLASH_PROGRAM_AAA);// Flash program byte method

		
		// Set bank as 0
		set_bank(0x2100, 0);
		set_bank(0x3000, 0);

		// Change to first 8MB block
		set_bank(0x1000, 0xD9); // Unlock multi-game mode
		set_bank(0x7000, 0xF0); // Set MBC5 mode and high 3 bits of bank value
		set_bank(0x6000, 0x00); // Bank
		set_bank(0x0000, 0xAA); // Turn off multi-game mode


		// Write ROM
		currAddr = 0x0000;
		for (uint8_t block = 0; block < romBlocks; block++) {
			currAddr = 0x0000;
			sector = 0;

			for (uint16_t bank = 1; bank < romBanks; bank++) {
				if (bank > 1) { currAddr = 0x4000; }

				// Set start address
				set_number(currAddr, SET_START_ADDRESS);
				Sleep(5);

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
							Sleep(5);
						}

						// Regular writing
						if (flashCartType == 14) {
							com_write_bytes_from_file(GB_FLASH_WRITE_64BYTE, romFile, 64);
							if (!com_wait_for_ack()) { // Wait for ack, checks if stalled after 250ms
								cancelOperation = 2;
								return 0;
							}
							currAddr += 64;
							readBytes += 64;
						}
						else {
							com_write_bytes_from_file(GB_FLASH_WRITE_BUFFERED_32BYTE, romFile, 32);
							if (!com_wait_for_ack()) { // Wait for ack, checks if stalled after 250ms
								cancelOperation = 2;
								return 0;
							}
							currAddr += 32;
							readBytes += 32;
						}
					}
					else {
						currAddr += 64;
						readBytes += 64;
					}

					// Print progress
					print_progress_percent(progress, readBytes, (romBanksTotal * 16384));
					if (cancelOperation == 1) { return 0; }
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
	}

	// 2 MByte GB Smart 16M
	else if (flashCartType == 18) {
		currAddr = 0x0000;
		endAddr = 0x7FFF;

		// Calculate banks needed from ROM file size
		romBanks = fileSize / 16384;

		// Flash Setup
		set_mode(GB_CART_MODE); // Gameboy mode
		set_mode(GB_FLASH_BANK_1_COMMAND_WRITES); // Set bank 1 before issuing flash commands
		gb_flash_pin_setup(WE_AS_AUDIO_PIN); // WR pin
		gb_flash_program_setup(GB_FLASH_PROGRAM_5555); // Flash program byte method
													   
										   
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
			Sleep(5);

			// Set address 0
			currAddr = 0x00;
			set_number(currAddr, SET_START_ADDRESS);
			Sleep(5);

			// Switch to the next flash chip
			if (chipNo >= 2) {
				set_bank(0x2000, 0x20 * (chipNo - 1));
				Sleep(5);
				set_bank(0x1000, 0xA5);
				Sleep(5);
				set_bank(0x7000, 0x00);
				Sleep(5);
				set_bank(0x1000, 0x98);
				Sleep(5);

				set_bank(0x2000, 0x20 * (chipNo - 1));
				Sleep(5);
				set_bank(0x1000, 0xA5);
				Sleep(5);
				set_bank(0x7000, 0x23);
				Sleep(5);
				set_bank(0x1000, 0x98);
				Sleep(5);
			}

			// Erase flash chip
			gb_flash_write_address_byte(0x5555, 0xAA);
			gb_flash_write_address_byte(0x2AAA, 0x55);
			gb_flash_write_address_byte(0x5555, 0x80);
			gb_flash_write_address_byte(0x5555, 0xAA);
			gb_flash_write_address_byte(0x2AAA, 0x55);
			gb_flash_write_address_byte(0x5555, 0x10);
			Sleep(5);

			// Wait for first byte to be 0xFF
			uint8_t waitCounter = 0;
			readBuffer[0] = 0;
			while (readBuffer[0] != 0xFF) {
				currAddr = 0x00; 
				set_number(currAddr, SET_START_ADDRESS);
				Sleep(5);
				set_mode(READ_ROM_RAM);
				Sleep(5);

				com_read_bytes(READ_BUFFER, 64);
				RS232_cputs(cport_nr, "0"); // End read

				if (readBuffer[0] != 0xFF) {
					Sleep(500);
					if (cancelOperation == 1) { return 0; }
				}
				if (cancelOperation == 1) { return 0; }
			}


			// Write ROM
			currAddr = 0x0000;
			for (uint16_t bank = 1; bank < currentBankSize; bank++) {
				if (bank > 1) { currAddr = 0x4000; }

				// Set start address
				set_number(currAddr, SET_START_ADDRESS);
				Sleep(5);

				// Read data
				while (currAddr < endAddr) {
					if (currAddr == 0x4000) { // Switch banks here just before the next bank, not any time sooner
						set_bank(0x2100, bank);
					}

					com_write_bytes_from_file(GB_FLASH_WRITE_64BYTE, romFile, 64);
					if (!com_wait_for_ack()) { // Wait for ack, checks if stalled after 250ms
						cancelOperation = 2;
						return 0;
					}
					currAddr += 64;
					readBytes += 64;

					// Print progress
					print_progress_percent(progress, readBytes, (romBanks * 16384));

					if (cancelOperation == 1) { return 0; }
				}
				if (cancelOperation == 1) { return 0; }
			}
			if (cancelOperation == 1) { return 0; }

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
		
		fclose(romFile);
	}
	

	// *** GBA carts ***

	// MSP55LV128 / 29LV128DTMC
	else if (flashCartType == 100) {
		// Read rom a tiny bit before writing
		currAddr = 0x0000;
		set_number(currAddr, SET_START_ADDRESS);
		Sleep(5);
		set_mode(GBA_READ_ROM);
		Sleep(5);
		com_read_bytes(READ_BUFFER, 64);
		RS232_cputs(cport_nr, "0");

		// Set end address as file size
		endAddr = fileSize;
		uint32_t endAddrAligned = fileSize;
		while ((endAddrAligned / 64) % 64 != 0) { // Align to 64 for printing progress
			endAddrAligned--;
		}

		// Write ROM
		currAddr = 0x0000;
		set_number(currAddr, SET_START_ADDRESS);
		Sleep(5);
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
					Sleep(5);
					set_mode(GBA_READ_ROM);
					Sleep(5);

					com_read_bytes(READ_BUFFER, 64);
					RS232_cputs(cport_nr, "0"); // End read
					Sleep(50);

					if (cancelOperation == 1) { return 0; }
				}
				set_number(currAddr / 2, SET_START_ADDRESS); // Set address back
				Sleep(5);
			}

			com_write_bytes_from_file(GBA_FLASH_WRITE_256BYTE_SWAPPED_D0D1, romFile, 256);
			if (!com_wait_for_ack()) { // Wait for ack, checks if stalled after 250ms
				cancelOperation = 2;
				return 0;
			}
			currAddr += 256;
			readBytes += 256;

			// Print progress
			print_progress_percent(progress, readBytes, endAddrAligned);

			if (cancelOperation == 1) { return 0; }
		}
		
		fclose(romFile);
	}

	// MSP55LV128M / 29GL128EHMC / 256M29EWH 
	else if (flashCartType == 101) {
		// Read rom a tiny bit before writing
		currAddr = 0x0000;
		set_number(currAddr, SET_START_ADDRESS);
		Sleep(5);
		set_mode(GBA_READ_ROM);
		Sleep(5);
		com_read_bytes(READ_BUFFER, 64);
		RS232_cputs(cport_nr, "0");

		// Set end address as file size
		endAddr = fileSize;
		uint32_t endAddrAligned = fileSize;
		while ((endAddrAligned / 64) % 64 != 0) { // Align to 64 for printing progress
			endAddrAligned--;
		}

		// Write ROM
		currAddr = 0x0000;
		set_number(currAddr, SET_START_ADDRESS);
		Sleep(5);
		while (currAddr < endAddr) {
			if (currAddr % 0x20000 == 0) { // Erase next 2 sectors
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
					Sleep(5);
					set_mode(GBA_READ_ROM);
					Sleep(5);

					com_read_bytes(READ_BUFFER, 64);
					RS232_cputs(cport_nr, "0"); // End read
					Sleep(50);

					if (cancelOperation == 1) { return 0; }
				}
				set_number(currAddr / 2, SET_START_ADDRESS); // Set address back
				Sleep(5);
			}

			com_write_bytes_from_file(GBA_FLASH_WRITE_256BYTE_SWAPPED_D0D1, romFile, 256);
			if (!com_wait_for_ack()) { // Wait for ack, checks if stalled after 250ms
				cancelOperation = 2;
				return 0;
			}
			currAddr += 256;
			readBytes += 256;

			// Print progress
			print_progress_percent(progress, readBytes, endAddrAligned);

			if (cancelOperation == 1) { return 0; }
		}

		fclose(romFile);
	}

	// M36L0R706 / 256L30B
	else if (flashCartType == 102 || flashCartType == 103) {
		// Set reading mode
		gba_flash_write_address_byte(0x00, 0xFF);
		Sleep(5);
		
		// Read rom a tiny bit before writing
		currAddr = 0x0000;
		set_number(currAddr, SET_START_ADDRESS);
		Sleep(5);
		set_mode(GBA_READ_ROM);
		Sleep(5);
		com_read_bytes(READ_BUFFER, 64);
		RS232_cputs(cport_nr, "0");

		// Set end address as file size
		endAddr = fileSize;
		uint32_t endAddrAligned = fileSize;
		while ((endAddrAligned / 64) % 64 != 0) { // Align to 64 for printing progress
			endAddrAligned--;
		}

		// Flash ID command
		gba_flash_write_address_byte(0x00, 0x90);
		Sleep(1);

		// Read ID
		set_number(0x00, SET_START_ADDRESS);
		Sleep(5);
		set_mode(GBA_READ_ROM);
		Sleep(5);
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
		gba_flash_write_address_byte(0x00, 0xFF);
		Sleep(5);

		currAddr = 0x0000;
		set_number(currAddr, SET_START_ADDRESS);
		Sleep(5);
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
				Sleep(50);

				// Wait for first 2 bytes to be 0x80, 0x00
				readBuffer[0] = 0;
				readBuffer[1] = 0;
				while (readBuffer[0] != 0x80 && readBuffer[1] != 0xB0) {
					set_number(currAddr / 2, SET_START_ADDRESS);
					Sleep(5);
					set_mode(GBA_READ_ROM);
					Sleep(5);

					com_read_bytes(READ_BUFFER, 64);
					com_read_stop(); // End read
					Sleep(500);
				}

				// Back to reading mode
				gba_flash_write_address_byte(currAddr, 0xFF);
				Sleep(5);

				set_number(currAddr / 2, SET_START_ADDRESS); // Divide address by 2
				Sleep(5);
			}

			// Standard buffered writing
			if (flashCartType == 12) {
				com_write_bytes_from_file(GBA_FLASH_WRITE_INTEL_64BYTE, romFile, 64);
				if (!com_wait_for_ack()) { // Wait for ack, checks if stalled after 250ms
					cancelOperation = 2;
					return 0;
				}
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
						uint16_t combinedBytes = (uint16_t)localbuffer[x + 1] << 8 | (uint16_t)localbuffer[x];
						gba_flash_write_address_byte(currAddr, 0x40);
						gba_flash_write_address_byte(currAddr, combinedBytes);
						currAddr += 2;
						readBytes += 2;
					}
					addressForManualWrite += 0x8000;

					// Set address again (seems to be needed at and after 0x27C0)
					set_number(currAddr / 2, SET_START_ADDRESS); // Divide address by 2
					Sleep(5);
				}
				else {
					com_write_bytes_from_file(GBA_FLASH_WRITE_INTEL_64BYTE, romFile, 64);
					if (!com_wait_for_ack()) { // Wait for ack, checks if stalled after 250ms
						cancelOperation = 2;
						return 0;
					}
					currAddr += 64;
					readBytes += 64;
				}
			}

			print_progress_percent(progress, readBytes, endAddrAligned);
		}

		// Back to reading mode
		gba_flash_write_address_byte(currAddr, 0xFF);
		Sleep(5);
	}

	// 4 MByte (MX29LV320)
	else if (flashCartType == 104) {
		// Set end address as file size
		endAddr = fileSize;
		uint32_t endAddrAligned = fileSize;
		while ((endAddrAligned / 64) % 64 != 0) { // Align to 64 for printing progress
			endAddrAligned--;
		}

		// Write ROM
		currAddr = 0x0000;
		set_number(currAddr, SET_START_ADDRESS);
		Sleep(5);
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
				gba_flash_write_address_byte((uint32_t)sector << 13, 0x30);
				sector++;

				// Wait for first 2 bytes to be 0xFF
				readBuffer[0] = 0;
				readBuffer[1] = 0;
				while (readBuffer[0] != 0xFF && readBuffer[1] != 0xFF) {
					set_number(currAddr / 2, SET_START_ADDRESS);
					Sleep(5);
					set_mode(GBA_READ_ROM);
					Sleep(5);

					com_read_bytes(READ_BUFFER, 64);
					com_read_stop(); // End read
					Sleep(50);

					if (cancelOperation == 1) { return 0; }
				}

				set_number(currAddr / 2, SET_START_ADDRESS); // Divide address by 2
				Sleep(5);
			}

			com_write_bytes_from_file(GBA_FLASH_WRITE_256BYTE, romFile, 256);
			if (!com_wait_for_ack()) { // Wait for ack, checks if stalled after 250ms
				cancelOperation = 2;
				return 0;
			}
			currAddr += 256;
			readBytes += 256;

			// Print progress
			print_progress_percent(progress, readBytes, endAddrAligned);

			if (cancelOperation == 1) { return 0; }
		}
	}

	// insideGadgets 32MB Cart (S29GL256 / S29GL512)
	else if (flashCartType == 105 || flashCartType == 106) {
		// Set end address as file size
		endAddr = fileSize;
		uint32_t endAddrAligned = fileSize;
		while ((endAddrAligned / 64) % 64 != 0) { // Align to 64 for printing progress
			endAddrAligned--;
		}

		// Write ROM
		currAddr = 0x0000;
		set_number(currAddr, SET_START_ADDRESS);
		Sleep(5);
		while (currAddr < endAddr) {
			// Sector erase only performed for under 16MB files
			if (flashCartType == 105 && currAddr % 0x10000 == 0) { // Erase next sector
				gba_flash_write_address_byte(0xAAA, 0xAA);
				gba_flash_write_address_byte(0x555, 0x55);
				gba_flash_write_address_byte(0xAAA, 0x80);
				gba_flash_write_address_byte(0xAAA, 0xAA);
				gba_flash_write_address_byte(0x555, 0x55);
				gba_flash_write_address_byte((uint32_t)sector << 17, 0x30);
				sector++;

				// Wait for first 2 bytes to be 0xFF
				readBuffer[0] = 0;
				readBuffer[1] = 0;
				while (readBuffer[0] != 0xFF && readBuffer[1] != 0xFF) {
					set_number(currAddr / 2, SET_START_ADDRESS);
					Sleep(5);
					set_mode(GBA_READ_ROM);
					Sleep(5);

					com_read_bytes(READ_BUFFER, 64);
					com_read_stop(); // End read
					Sleep(500);

					if (cancelOperation == 1) { return 0; }
				}

				set_number(currAddr / 2, SET_START_ADDRESS); // Divide address by 2
				Sleep(5);
			}

			com_write_bytes_from_file(GBA_FLASH_WRITE_256BYTE, romFile, 256);
			if (!com_wait_for_ack()) { // Wait for ack, checks if stalled after 250ms
				cancelOperation = 2;
				return 0;
			}
			currAddr += 256;
			readBytes += 256;

			// Print progress
			print_progress_percent(progress, readBytes, endAddrAligned);

			if (cancelOperation == 1) { return 0; }
		}
	}


	return 1;
}
}