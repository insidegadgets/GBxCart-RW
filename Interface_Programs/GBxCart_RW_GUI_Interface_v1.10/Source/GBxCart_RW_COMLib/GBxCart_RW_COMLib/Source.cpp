/*
GBxCart RW - GUI COM Interface
Version : 1.10
Author : Alex from insideGadgets(www.insidegadgets.com)
Created : 7 / 11 / 2016
Last Modified : 28 / 11 / 2017

GBxCart RW allows you to dump your Gameboy / Gameboy Colour / Gameboy Advance games ROM, save the RAM and write to the RAM.

*/

#include <stdio.h>
#include <string.h>
#include <windows.h>

extern "C" {

extern void RS232_cputs(int comport_number, const char *text);
extern int RS232_SendByte(int comport_number, unsigned char byte);
extern int RS232_PollComport(int comport_number, unsigned char *buf, int size);
extern int RS232_SendBuf(int comport_number, unsigned char *buf, int size);

#define LOW 0
#define HIGH 1
#define false 0
#define true 1

#ifndef uint8_t
	typedef unsigned char uint8_t;
#endif

#ifndef uint16_t
	typedef unsigned int uint16_t;
#endif

#ifndef uint32_t
	typedef unsigned long uint32_t;
#endif

#define CART_MODE 'C'
#define GB_MODE 1
#define GBA_MODE 2

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
	
#define GBA_READ_ROM 'r'
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

// Flash ROM commands
#define GB_FLASH_WRITE_BYTE 'F'
#define GB_FLASH_WRITE_64BYTE 'T'

#define GB_AUDIO_FLASH_WRITE 'U'
#define GB_BV5_FLASH_WRITE '5'

// PCB Hardware
#define PCB_1_0 1
#define PCB_1_1 2

// Common vars
#define READ_BUFFER 0

int cport_nr = 7, // /dev/ttyS7 (COM8 on windows)
bdrate = 1000000; // 1,000,000 baud

uint8_t gbxcartFirmwareVersion = 0;
uint8_t gbxcartPcbVersion = 0;
uint8_t readBuffer[65];
uint8_t writeBuffer[128];
char gameTitle[17];
uint16_t cartridgeType = 0;
uint32_t currAddr = 0x0000;
uint32_t endAddr = 0x7FFF;
uint16_t romSize = 0;
uint32_t romEndAddr = 0;
uint16_t romBanks = 0;
uint16_t ramSize = 0;
uint16_t ramBanks = 0;
uint32_t ramEndAddress = 0;
uint8_t eepromSize = 0;
uint16_t eepromEndAddress = 0;
uint8_t hasFlashSave = 0;
uint8_t cartridgeMode = GB_MODE;

uint8_t nintendoLogo[] = { 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
	0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
	0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E };
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
	FILE* configfile = fopen("config.ini", "rb");
	char buffer[100];

	if (configfile != NULL) {
		// Copy the file into the buffer, we only read 2 characters
		fread(buffer, 1, 2, configfile);
		buffer[2] = 0;

		uint8_t numbersFound = 0;
		for (uint8_t x = 0; x < 2; x++) {
			if (buffer[x] >= 48 && buffer[x] <= 57) {
				numbersFound++;
			}
		}
		buffer[numbersFound] = 0;

		if (numbersFound >= 1) {
			cport_nr = atoi(buffer);
		}

		// Remove the \r\n line
		fread(buffer, 1, numbersFound, configfile);

		// Read the baud rate
		fread(buffer, 1, 7, configfile);
		buffer[7] = 0;

		for (uint8_t x = 0; x < 7; x++) {
			if (buffer[x] >= 48 && buffer[x] <= 57) {
				numbersFound++;
			}
		}

		if (numbersFound >= 1) {
			bdrate = atoi(buffer);
		}

		fclose(configfile);
		
		if (type == 1) {
			return cport_nr;
		}
		else {
			return bdrate;
		}
	}

	return 0;
}

// Update config file if com port or baudrate is changed
__declspec(dllexport) void update_config(int comport, INT32 baudrate) {
	cport_nr = comport;
	bdrate = baudrate;

	FILE* configfile = fopen("config.ini", "wb");

	if (configfile != NULL) {
		char comportBuffer[5];
		_itoa((comport+1), comportBuffer, 10);
		
		char baudrateBuffer[10];
		_itoa(baudrate, baudrateBuffer, 10);

		fwrite(comportBuffer, 1, strlen(comportBuffer), configfile);
		fwrite("\r\n", 1, 2, configfile);
		fwrite(baudrateBuffer, 1, strlen(baudrateBuffer), configfile);
		fwrite("\r\n", 1, 2, configfile);

		fclose(configfile);
	}
}

// Load a file which contains the cartridge RAM settings (only needed if Erase RAM option was used, only applies to GBA games)
void load_cart_ram_info(void) {
	char titleFilename[30];
	strncpy(titleFilename, gameTitle, 20);
	strncat(titleFilename, ".si", 4);

	// Create a new file
	FILE *infoFile = fopen(titleFilename, "rb");
	if (infoFile != NULL) {
		fseek(infoFile, 0, SEEK_END);
		long fileSize = ftell(infoFile);
		fseek(infoFile, 0, SEEK_SET);

		char buffer[100];
		fread(buffer, 1, fileSize, infoFile);

		int tokenNo = 0;
		char *token = strtok(buffer, ",");
		while (token != NULL) {
			if (tokenNo == 0) {
				ramSize = atoi(token);
			}
			else if (tokenNo == 1) {
				eepromSize = atoi(token);
			}
			else if (tokenNo == 2) {
				hasFlashSave = atoi(token);
			}
			
			//printf("tok = %i\n", atoi(token));
			tokenNo++;
			token = strtok(NULL, ",");
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
	FILE *infoFileRead = fopen(titleFilename, "rb");
	if (infoFileRead == NULL) {
		
		// Create a new file
		FILE *infoFile = fopen(titleFilename, "wb");
		if (infoFile != NULL) {
			char ramSizeBuffer[10];
			_itoa(ramSize, ramSizeBuffer, 10);

			char eepromSizeBuffer[10];
			_itoa(eepromSize, eepromSizeBuffer, 10);

			char hasFlashSaveBuffer[10];
			_itoa(hasFlashSave, hasFlashSaveBuffer, 10);

			fwrite(ramSizeBuffer, 1, strlen(ramSizeBuffer), infoFile);
			fwrite(",", 1, 1, infoFile);
			fwrite(eepromSizeBuffer, 1, strlen(eepromSizeBuffer), infoFile);
			fwrite(",", 1, 1, infoFile);
			fwrite(hasFlashSaveBuffer, 1, strlen(hasFlashSaveBuffer), infoFile);
			fwrite(",", 1, 1, infoFile);

			fclose(infoFile);
		}
	}
	else {
		fclose(infoFileRead);
	}
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

// Read one letter from stdin
char read_one_letter(void) {
	char c = getchar();
	while (getchar() != '\n' && getchar() != EOF);
	return c;
}

// Print progress
void print_progress_percent(uint32_t &progress, uint32_t bytesRead, uint32_t hashNumber) {
	/*if ((bytesRead % hashNumber == 0) && bytesRead != 0) {
		if (hashNumber == 64) {
			printf("########");
			progress += 8;
		}
		else {
			printf("#");
			progress += 1;
		}
	}*/

	progress = (double) ((double) bytesRead / (double) hashNumber) * (double) 100;
}

// Wait for a "1" acknowledgement from the ATmega
void com_wait_for_ack(void) {
	uint8_t buffer[2];
	uint8_t rxBytes = 0;

	while (rxBytes < 1) {
		rxBytes = RS232_PollComport(cport_nr, buffer, 1);

		if (rxBytes > 0) {
			if (buffer[0] == '1') {
				break;
			}
			rxBytes = 0;
		}
	}
}

// Read 1 to 64 bytes from the COM port and write it to the global read buffer or to a file if specified. 
// When polling the com port it return less than the bytes we want, keep polling and wait until we have all bytes requested. 
// We expect no more than 64 bytes.
void com_read_bytes(FILE *file, uint8_t count) {
	uint8_t buffer[65];
	uint8_t rxBytes = 0;
	uint8_t readBytes = 0;

	while (readBytes < count) {
		rxBytes = RS232_PollComport(cport_nr, buffer, 65);

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
	}
}

// Read 1-128 bytes from the file (or buffer) and write it the COM port with the command given
void com_write_bytes_from_file(uint8_t command, FILE *file, uint8_t count) {
	uint8_t buffer[129];
	buffer[0] = command;

	if (file == NULL) {
		memcpy(&buffer[1], writeBuffer, count);
	}
	else {
		fread(&buffer[1], 1, count, file);
	}

	RS232_SendBuf(cport_nr, buffer, (count + 1)); // command + 1-128 bytes
}

// Send a single command byte
__declspec(dllexport) void set_mode(char command) {
	char modeString[5];
	sprintf(modeString, "%c", command);

	RS232_cputs(cport_nr, modeString);
}

// Send a command with a hex number and a null terminator byte
void set_number(uint32_t number, uint8_t command) {
	char numberString[20];
	sprintf(numberString, "%c%x", command, number);

	RS232_cputs(cport_nr, numberString);
	RS232_SendByte(cport_nr, 0);
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

	char bankString[15];
	sprintf(bankString, "%c%d", SET_BANK, bank);
	RS232_cputs(cport_nr, bankString);
	RS232_SendByte(cport_nr, 0);
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
		com_read_bytes(READ_BUFFER, 64);
		memcpy(&startRomBuffer[currAddr], readBuffer, 64);
		currAddr += 64;

		if (currAddr < endAddr) {
			RS232_cputs(cport_nr, "1");
		}
	}
	RS232_cputs(cport_nr, "0"); // Stop read

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
			(headerChar == 0x20)) { // space
			gameTitle[(titleAddress - 0x0134)] = headerChar;
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

	// Nintendo Logo Check
	uint8_t logoCheck = 1;
	for (uint16_t logoAddress = 0x0104; logoAddress <= 0x133; logoAddress++) {
		if (nintendoLogo[(logoAddress - 0x0104)] != startRomBuffer[logoAddress]) {
			logoCheck = 0;
			break;
		}
	}
	//printf("Logo check: ");
	char logoCheckText[50];
	if (logoCheck == 1) {
		strncpy(logoCheckText, "OK\0", 30);
	}
	else {
		strncpy(logoCheckText, "Failed\0", 30);
	}
	
	sprintf(headerText, "Game title: %s\nMBC type: %s\nROM size: %s\nRAM size: %s\nLogo check: %s", gameTitle, cartridgeTypeText, romSizeText, ramSizeText, logoCheckText);
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
			if (gbxcartPcbVersion == PCB_1_1) {
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
	if (zeroTotal >= 2048) { // No SRAM or Flash
		return 0;
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

// Read the first 192 bytes of ROM, read the title, check and test for ROM, SRAM, EEPROM and Flash
__declspec(dllexport) char* read_gba_header(int &headerlength) {
	set_mode('0'); // Break out of any loops on ATmega

	gbxcartPcbVersion = request_value(READ_PCB_VERSION);

	char* headerText;
	headerText = (char*)malloc(500);

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
			RS232_cputs(cport_nr, "1");
		}
	}
	RS232_cputs(cport_nr, "0"); // Stop read

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
			gameTitle[(titleAddress - 0xA0)] = headerChar;
		}
		else {
			gameTitle[(titleAddress - 0xA0)] = '\0';
			break;
		}
	}
	printf("Game title: %s\n", gameTitle);
	sprintf(headerText, "Game title: %s\n", gameTitle);

	// Nintendo Logo Check
	uint8_t logoCheck = 1;
	for (uint16_t logoAddress = 0x04; logoAddress <= 0x9F; logoAddress++) {
		if (nintendoLogoGBA[(logoAddress - 0x04)] != startRomBuffer[logoAddress]) {
			logoCheck = 0;
			break;
		}
	}

	// ROM size
	printf("Calculating ROM size");
	romSize = gba_check_rom_size();

	// EEPROM check
	printf("\nChecking for EEPROM");
	eepromSize = gba_check_eeprom();

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
__declspec(dllexport) void read_rom(uint32_t &progress, uint8_t &cancelOperation) {
	set_mode('0'); // Break out of any loops on ATmega
	
	cartridgeMode = read_cartridge_mode();
	printf("\n--- Dump ROM ---\n");

	char titleFilename[30];
	strncpy(titleFilename, gameTitle, 20);
	if (cartridgeMode == GB_MODE) {
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
	if (cartridgeMode == GB_MODE) {
		// Set start and end address
		currAddr = 0x0000;
		endAddr = 0x7FFF;

		// Read ROM
		for (uint16_t bank = 1; bank < romBanks; bank++) {
			if (cartridgeType >= 5) { // MBC2 and above
				set_bank(0x2100, bank);
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
				com_read_bytes(romFile, 64);
				currAddr += 64;
				readBytes += 64;

				// Request 64 bytes more
				if (currAddr < endAddr) {
					RS232_cputs(cport_nr, "1");
				}

				// Print progress
				print_progress_percent(progress, readBytes, (romBanks * 16384));

				if (cancelOperation == 1) {
					break;
				}
			}
			RS232_cputs(cport_nr, "0"); // Stop reading ROM (as we will bank switch)

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
		set_mode(GBA_READ_ROM);

		// Read data
		while (currAddr < endAddr) {
			com_read_bytes(romFile, 64);
			currAddr += 64;

			// Request 64 bytes more
			if (currAddr < endAddr) {
				RS232_cputs(cport_nr, "1");
			}

			// Print progress
			print_progress_percent(progress, currAddr, endAddr);

			if (cancelOperation == 1) {
				break;
			}
		}
		printf("]");
		RS232_cputs(cport_nr, "0"); // Stop reading
	}

	fclose(romFile);
	printf("\nFinished\n");
}


__declspec(dllexport) int check_if_file_exists() {
	char titleFilename[30];
	strncpy(titleFilename, gameTitle, 20);
	strncat(titleFilename, ".sav", 4);

	FILE *testFile = fopen(titleFilename, "rb");
	if (testFile != NULL) {
		fclose(testFile);
		return 1;
	}
	return 0;
}

// Read RAM
__declspec(dllexport) void read_ram(uint32_t &progress, uint8_t &cancelOperation) {
	cartridgeMode = read_cartridge_mode();
	gbxcartFirmwareVersion = request_value(READ_FIRMWARE_VERSION);

	printf("\n--- Save RAM to PC---\n");

	if (cartridgeMode == GB_MODE) {
		// Does cartridge have RAM
		if (ramEndAddress > 0) {
			char titleFilename[30];
			strncpy(titleFilename, gameTitle, 20);
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
				// Read RAM
				uint32_t readBytes = 0;
				for (uint8_t bank = 0; bank < ramBanks; bank++) {
					uint16_t ramAddress = 0xA000;
					set_bank(0x4000, bank);
					set_number(ramAddress, SET_START_ADDRESS); // Set start address again
					set_mode(READ_ROM_RAM); // Set rom/ram reading mode

					while (ramAddress < ramEndAddress) {
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
			char titleFilename[30];
			strncpy(titleFilename, gameTitle, 20);
			strncat(titleFilename, ".sav", 4);

			// Create a new file
			FILE *ramFile = fopen(titleFilename, "wb");

			// SRAM/Flash
			if (ramEndAddress > 0) {
				printf("Saving RAM (SRAM/Flash) to %s\n", titleFilename);
				printf("[             25%%             50%%             75%%            100%%]\n[");

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
						com_read_bytes(ramFile, 64);
						currAddr += 64;
						readBytes += 64;

						// Request 64 bytes more
						if (currAddr < endAddr) {
							RS232_cputs(cport_nr, "1");
						}

						print_progress_percent(progress, readBytes, ramBanks * endAddr);

						if (cancelOperation == 1) {
							break;
						}
					}

					RS232_cputs(cport_nr, "0"); // End read (for this bank if Flash)

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
					//printf("%i %i\n", currAddr, endAddr);
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
__declspec(dllexport) void write_ram(uint32_t &progress, uint8_t &cancelOperation) {
	cartridgeMode = read_cartridge_mode();
	printf("\n--- Write RAM to GB Cart ---\n");

	if (cartridgeMode == GB_MODE) {
		// Does cartridge have RAM
		if (ramEndAddress > 0) {
			char titleFilename[30];
			strncpy(titleFilename, gameTitle, 20);
			strncat(titleFilename, ".sav", 4);

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
			char titleFilename[30];
			strncpy(titleFilename, gameTitle, 20);
			strncat(titleFilename, ".sav", 4);

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

						// Wait for ATmega to process write (~320us) and for EEPROM to write data (6ms)
						com_wait_for_ack();

						if (cancelOperation == 1) {
							break;
						}
					}
				}

				// Flash
				else if (hasFlashSave != NO_FLASH) {
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
								}

								com_write_bytes_from_file(GBA_FLASH_WRITE_BYTE, ramFile, 64);
								currAddr += 64;
								readBytes += 64;

								print_progress_percent(progress, readBytes, ramBanks * endAddr);

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

					print_progress_percent(progress, readBytes, ramEndAddress);
					com_wait_for_ack();

					if (cancelOperation == 1) {
						break;
					}
				}
			}

			// EEPROM
			else if (eepromSize != EEPROM_NONE) {
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

					print_progress_percent(progress, readBytes, endAddr);

					// Wait for ATmega to process write (~320us) and for EEPROM to write data (6ms)
					com_wait_for_ack();

					if (cancelOperation == 1) {
						break;
					}
				}
			}

			// Flash
			else if (hasFlashSave != NO_FLASH) {
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

							print_progress_percent(progress, readBytes, ramBanks * endAddr);
							com_wait_for_ack(); // Wait for write complete

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
							}

							currAddr += 64;
							readBytes += 64;

							print_progress_percent(progress, readBytes, ramBanks * endAddr);

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

// Wait for first byte to be 0xFF, that's when we know the sector has been erased
void wait_for_flash_ff(uint16_t address) {
	readBuffer[0] = 0;
	while (readBuffer[0] != 0xFF) {
		set_number(address, SET_START_ADDRESS);
		set_mode(READ_ROM_RAM);

		com_read_bytes(READ_BUFFER, 64);
		RS232_cputs(cport_nr, "0"); // End read

		if (readBuffer[0] != 0xFF) {
			Sleep(20);
		}
	}
}

// GB Flash Cart FW3 - Write address and byte to flash
void fw3_gb_flash_write_address_byte(uint16_t address, uint8_t byte) {
	char AddrString[15];
	sprintf(AddrString, "%c%x", 'F', address);
	RS232_cputs(cport_nr, AddrString);
	RS232_SendByte(cport_nr, 0);

	char byteString[15];
	sprintf(byteString, "%c%x", 'F', byte);
	RS232_cputs(cport_nr, byteString);
	RS232_SendByte(cport_nr, 0);

	com_wait_for_ack();
}

// GB Flash Cart FW4 - Write address and byte to flash
void fw4_gb_flash_write_address_byte(uint16_t address, uint8_t byte) {
	char AddrString[15];
	sprintf(AddrString, "%c%x", 'F', address);
	RS232_cputs(cport_nr, AddrString);
	RS232_SendByte(cport_nr, 0);

	char byteString[15];
	sprintf(byteString, "%c%x", 'U', byte);
	RS232_cputs(cport_nr, byteString);
	RS232_SendByte(cport_nr, 0);

	com_wait_for_ack();
}

// GB Flash Cart FW3 - Setup audio pin as output
void fw3_gb_flash_setup(void) {
	RS232_cputs(cport_nr, "G"); // Gameboy mode
	RS232_cputs(cport_nr, "OE0x02"); // WE (Audio in) as output
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "HE0x02"); // WE (Audio in) high
	RS232_SendByte(cport_nr, 0);
}

// GB Flash Cart FW3 - Sector erase
void fw3_gb_flash_sector_erase(uint8_t sector) {
	fw3_gb_flash_write_address_byte(0x555, 0xAA);
	fw3_gb_flash_write_address_byte(0x2AA, 0x55);
	fw3_gb_flash_write_address_byte(0x555, 0x80);
	fw3_gb_flash_write_address_byte(0x555, 0xAA);
	fw3_gb_flash_write_address_byte(0x2AA, 0x55);
	fw3_gb_flash_write_address_byte(sector << 14, 0x30);
}

// GB Flash Cart FW4 - Sector erase
void fw4_gb_flash_sector_erase(uint8_t sector) {
	fw4_gb_flash_write_address_byte(0x555, 0xAA);
	fw4_gb_flash_write_address_byte(0x2AA, 0x55);
	fw4_gb_flash_write_address_byte(0x555, 0x80);
	fw4_gb_flash_write_address_byte(0x555, 0xAA);
	fw4_gb_flash_write_address_byte(0x2AA, 0x55);
	fw4_gb_flash_write_address_byte(sector << 14, 0x30);
}

// GB Flash Cart FW2 - Reset lines
void fw2_gb_flash_reset_lines(void) {
	RS232_cputs(cport_nr, "HE0x02"); // WE (Audio in) high
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "LC0xFF");
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "LB0xFF");
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "LA0xFF");
	RS232_SendByte(cport_nr, 0);
}

// GB Flash Cart FW2 - Sector erase
void fw2_gb_flash_sector_erase(uint8_t sector) {
	RS232_cputs(cport_nr, "M0"); // Disable CS/RD/WR/CS2-RST from going high after each command 
	RS232_cputs(cport_nr, "G"); // Gameboy mode

	// Set outputs
	RS232_cputs(cport_nr, "OC0xFF");
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "OE0x02"); // WE (Audio in)
	RS232_SendByte(cport_nr, 0);

	// Reset lines
	fw2_gb_flash_reset_lines();


	// 0x555, 0xAA
	RS232_cputs(cport_nr, "HB0x55"); // A0-A7
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "HA0x05"); // A8-15
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "HC0xAA"); // Set byte
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "LE0x02"); // WE (Audio in) low
	RS232_SendByte(cport_nr, 0);
	fw2_gb_flash_reset_lines();


	// 0x2AA, 0x55
	RS232_cputs(cport_nr, "HB0xAA"); // A0-A7
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "HA0x02"); // A8-15
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "HC0x55"); // Set byte
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "LE0x02"); // WE (Audio in) low
	RS232_SendByte(cport_nr, 0);
	fw2_gb_flash_reset_lines();


	// 0x555, 0x80
	RS232_cputs(cport_nr, "HB0x55"); // A0-A7
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "HA0x05"); // A8-15
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "HC0x80"); // Set byte
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "LE0x02"); // WE (Audio in) low
	RS232_SendByte(cport_nr, 0);
	fw2_gb_flash_reset_lines();


	// 0x555, 0xAA
	RS232_cputs(cport_nr, "HB0x55"); // A0-A7
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "HA0x05"); // A8-15
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "HC0xAA"); // Set byte
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "LE0x02"); // WE (Audio in) low
	RS232_SendByte(cport_nr, 0);
	fw2_gb_flash_reset_lines();


	// 0x2AA, 0x55
	RS232_cputs(cport_nr, "HB0xAA"); // A0-A7
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "HA0x02"); // A8-15
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "HC0x55"); // Set byte
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "LE0x02"); // WE (Audio in) low
	RS232_SendByte(cport_nr, 0);
	fw2_gb_flash_reset_lines();


	// Sector << 14, 0x30
	if (sector == 0) {
		RS232_cputs(cport_nr, "HA0x00"); // A8-15
	}
	else {
		RS232_cputs(cport_nr, "HA0x40"); // A8-15
	}
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "HC0x30"); // Set byte
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "LE0x02"); // WE (Audio in) low
	RS232_SendByte(cport_nr, 0);
	fw2_gb_flash_reset_lines();

	RS232_cputs(cport_nr, "M1");
}

// GB Flash Cart FW2 - Write address and byte
void fw2_gb_flash_write_address_byte(uint16_t address, uint8_t byte) {
	RS232_cputs(cport_nr, "M0"); // Disable CS/RD/WR/CS2-RST from going high after each command 
	RS232_cputs(cport_nr, "G"); // Gameboy mode

								// Set outputs
	RS232_cputs(cport_nr, "OC0xFF");
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "OE0x02"); // WE (Audio in)
	RS232_SendByte(cport_nr, 0);

	// Reset lines
	fw2_gb_flash_reset_lines();


	// 0x555, 0xAA
	RS232_cputs(cport_nr, "HB0x55"); // A0-A7
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "HA0x05"); // A8-15
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "HC0xAA"); // Set byte
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "LE0x02"); // WE (Audio in) low
	RS232_SendByte(cport_nr, 0);
	fw2_gb_flash_reset_lines();


	// 0x2AA, 0x55
	RS232_cputs(cport_nr, "HB0xAA"); // A0-A7
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "HA0x02"); // A8-15
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "HC0x55"); // Set byte
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "LE0x02"); // WE (Audio in) low
	RS232_SendByte(cport_nr, 0);
	fw2_gb_flash_reset_lines();


	// 0x555, 0xA0
	RS232_cputs(cport_nr, "HB0x55"); // A0-A7
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "HA0x05"); // A8-15
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "HC0xA0"); // Set byte
	RS232_SendByte(cport_nr, 0);
	RS232_cputs(cport_nr, "LE0x02"); // WE (Audio in) low
	RS232_SendByte(cport_nr, 0);
	fw2_gb_flash_reset_lines();


	// Write byte
	char addrHexNumLow[10];
	snprintf(addrHexNumLow, 7, "HB0x%x", address & 0xFF);
	RS232_cputs(cport_nr, addrHexNumLow); // A0-A7
	RS232_SendByte(cport_nr, 0);

	char addrHexNumHigh[10];
	snprintf(addrHexNumHigh, 7, "HA0x%x", address >> 8);
	RS232_cputs(cport_nr, addrHexNumHigh); // A8-15
	RS232_SendByte(cport_nr, 0);

	char byteHexNum[10];
	snprintf(addrHexNumHigh, 7, "HC0x%x", byte);
	RS232_cputs(cport_nr, byteHexNum); // Set byte
	RS232_SendByte(cport_nr, 0);

	RS232_cputs(cport_nr, "LE0x02"); // WE (Audio in) low
	RS232_SendByte(cport_nr, 0);
	fw2_gb_flash_reset_lines();


	RS232_cputs(cport_nr, "M1");
}

// BV5 GB Flash Cart
void gb_bv5_flash_write_address_byte(uint16_t address, uint8_t byte) {
	char AddrString[15];
	sprintf(AddrString, "%c%x", 'F', address);
	RS232_cputs(cport_nr, AddrString);
	RS232_SendByte(cport_nr, 0);

	char byteString[15];
	sprintf(byteString, "%c%x", '5', byte);
	RS232_cputs(cport_nr, byteString);
	RS232_SendByte(cport_nr, 0);

	com_wait_for_ack();
}


__declspec(dllexport) int erase_rom(uint32_t &progress, uint8_t &cancelOperation) {
	// Read rom a tiny bit before writing
	currAddr = 0x0000;
	set_number(currAddr, SET_START_ADDRESS);
	set_mode(READ_ROM_RAM);
	com_read_bytes(READ_BUFFER, 64);
	RS232_cputs(cport_nr, "0"); // Stop read

	// Chip erase (data byte's bit 0 & 1 are swapped for chip commands as BV5 D0 & D1 lines are swapped)
	gb_bv5_flash_write_address_byte(0xAAA, 0xA9);
	gb_bv5_flash_write_address_byte(0x555, 0x56);
	gb_bv5_flash_write_address_byte(0xAAA, 0x80);
	gb_bv5_flash_write_address_byte(0xAAA, 0xA9);
	gb_bv5_flash_write_address_byte(0x555, 0x56);
	gb_bv5_flash_write_address_byte(0xAAA, 0x10);

	// Wait for first byte to be 0xFF
	uint8_t waitCounter = 0;
	readBuffer[0] = 0;
	while (readBuffer[0] != 0xFF) {
		set_number(currAddr, SET_START_ADDRESS);
		set_mode(GBA_READ_SRAM);

		com_read_bytes(READ_BUFFER, 64);
		RS232_cputs(cport_nr, "0"); // End read

		if (readBuffer[0] != 0xFF) {
			Sleep(500);
			
			if (waitCounter < 50) {
				waitCounter++;
			}

			print_progress_percent(progress, waitCounter, 50);
		}

		if (cancelOperation == 1) {
			break;
		}
	}

	waitCounter = 50;
}

__declspec(dllexport) int write_rom(char* fileName, uint8_t flashCartType, uint32_t fileSize, uint32_t &progress, uint8_t &cancelOperation) {
	uint32_t readBytes = 0;
	uint8_t sector = 0;

	FILE *romFile = fopen(fileName, "rb");
	if (romFile == NULL) {
		return 0;
	}
	
	gbxcartFirmwareVersion = request_value(READ_FIRMWARE_VERSION);

	if (flashCartType == 1) {
		currAddr = 0x0000;
		endAddr = 0x7FFF;

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

				// Print progress
				print_progress_percent(progress, readBytes, (endAddr + 1));

				if (cancelOperation == 1) {
					break;
				}
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

				// Print progress
				print_progress_percent(progress, readBytes, (endAddr + 1));

				if (cancelOperation == 1) {
					break;
				}
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

				// Print progress
				print_progress_percent(progress, readBytes, (endAddr + 1));

				if (cancelOperation == 1) {
					break;
				}
			}
		}

		fclose(romFile);
	}
	else if (flashCartType == 2) {
		currAddr = 0x0000;
		endAddr = 0x7FFF;

		// Calculate banks needed from ROM file size
		romBanks = fileSize / 16384;

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
				print_progress_percent(progress, readBytes, (romBanks * 16384));

				if (cancelOperation == 1) {
					break;
				}
			}

			if (cancelOperation == 1) {
				break;
			}
		}

		fclose(romFile);
	}

	return 1;
}
}