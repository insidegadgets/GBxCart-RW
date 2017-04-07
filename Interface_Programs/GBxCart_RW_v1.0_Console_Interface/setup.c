/*
 GBxCart RW - Console Interface
 Version: 1.0
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 7/11/2016
 Last Modified: 11/03/2017
 
 */

#define LOW 0
#define HIGH 1
#define false 0
#define true 1

// COM Port settings (default)
#include "rs232/rs232.h"
int cport_nr = 7, // /dev/ttyS7 (COM8 on windows)
bdrate = 1000000; // 1,000,000 baud

#define CART_MODE 'C'
#define GB_MODE 1
#define GBA_MODE 2

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

// Common vars
#define READ_BUFFER 0

uint8_t readBuffer[65];
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


// Read the config.ini file for the COM port to use and baud rate
void read_config(void) {
	FILE* configfile = fopen ( "config.ini" , "rb" );
	char* buffer;
	if (configfile != NULL) {
		// Allocate memory 
		buffer = (char*) malloc (sizeof(char) * 2);
		
		// Copy the file into the buffer, we only read 2 characters
		fread (buffer, 1, 2, configfile);
		
		uint8_t numbersFound = 0;
		for (uint8_t x = 0; x < 2; x++) {
			if (buffer[x] >= 48 && buffer[x] <= 57) {
				numbersFound++;
			}
		}
		
		if (numbersFound >= 1) {
			cport_nr = atoi(buffer);
			cport_nr--;
		}
		
		// Remove the \r\n line
		fread (buffer, 1, numbersFound, configfile);
		
		// Read the baud rate
		fread (buffer, 1, 7, configfile);
		
		for (uint8_t x = 0; x < 7; x++) {
			if (buffer[x] >= 48 && buffer[x] <= 57) {
				numbersFound++;
			}
		}
		
		if (numbersFound >= 1) {
			bdrate = atoi(buffer);
		}
		
		fclose (configfile);
		free (buffer);
	}
}

// Read one letter from stdin
char read_one_letter (void) {
	char c = getchar();
	while (getchar() != '\n' && getchar() != EOF);
	return c;
}

// Print progress
void print_progress_percent (uint32_t bytesRead, uint32_t hashNumber) {
	if ((bytesRead % hashNumber == 0) && bytesRead != 0) {
		if (hashNumber == 64) {
			printf("########");
		}
		else {
			printf("#");
		}
	}
}

// Wait for a "1" acknowledgement from the ATmega
void com_wait_for_ack (void) {
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
void com_read_bytes (FILE *file, uint8_t count) {
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

// Read 1-128 bytes from the file and write it the COM port with the command given
void com_write_bytes_from_file (uint8_t command, FILE *file, uint8_t count) {
	uint8_t buffer[129];
	
	buffer[0] = command;
	fread(&buffer[1], 1, count, file);
	RS232_SendBuf(cport_nr, buffer, (count+1)); // command + 1-128 bytes
}

// Send a single command byte
void set_mode (char command) {
	char modeString[5];
	sprintf(modeString, "%c", command);
	
	RS232_cputs(cport_nr, modeString);
}

// Send a command with a hex number and a null terminator byte
void set_number (uint32_t number, uint8_t command) {
	char numberString[20];
	sprintf(numberString, "%c%x", command, number);
	
	RS232_cputs(cport_nr, numberString);
	RS232_SendByte(cport_nr, 0);
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



// ****** Gameboy / Gameboy Colour functions ******

// Set bank for ROM/RAM switching, send address first and then bank number
void set_bank (uint16_t address, uint8_t bank) {
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
	RS232_cputs(cport_nr, "0"); // Stop read
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
	
	// Loop until 16MB
	for (uint16_t x = 0; x < 256; x++) {
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
		if (currAddr % fourMbBoundary == 0 || currAddr % fourMbBoundary < 256) {
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
		
		set_mode(GBA_FLASH_READ_ID);
		com_read_bytes(READ_BUFFER, 2);
		
		// Check if it's Atmel Flash
		if (readBuffer[0] == 0x1F) {
			return FLASH_FOUND_ATMEL;
		}
		// Check other manufacturers 
		else if (readBuffer[0] == 0xBF || readBuffer[0] == 0xC2 || 
					readBuffer[0] == 0x32 || readBuffer[0] == 0x62) {
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
		set_number((uint32_t) (x * 0x400), SET_START_ADDRESS);
		set_mode(GBA_READ_SRAM);
		com_read_bytes(READ_BUFFER, 64);
		memcpy(&firstBuffer, readBuffer, 64);
		RS232_cputs(cport_nr, "0"); // Stop read
		
		set_number((uint32_t) (x * 0x400) + 0x8000, SET_START_ADDRESS);
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
	if (duplicateCount >= 1500) {
		return SRAM_FLASH_256KBIT;
	}
	return SRAM_FLASH_512KBIT;
	
	
	// Check if it's SRAM or Flash at this stage, maximum for SRAM is 512Kbit
	hasFlashSave = gba_test_sram_flash_write();
	if (hasFlashSave == NO_FLASH) {
		return SRAM_FLASH_512KBIT;
	}
	
	// Test 512Kbit or 1Mbit Flash, read first 64 bytes on bank 1 then bank 2 and compare
	else {
		duplicateCount = 0;
		
		// Read bank 0
		set_number(0, SET_START_ADDRESS);
		set_mode(GBA_READ_SRAM);
		com_read_bytes(READ_BUFFER, 64);
		memcpy(&firstBuffer, readBuffer, 64);
		RS232_cputs(cport_nr, "0"); // Stop read
		
		// Read bank 1
		set_number(1, GBA_FLASH_SET_BANK); // Set bank 1
		
		set_number(0, SET_START_ADDRESS);
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
		
		// If duplicated bank 0 and 1, then 512Kbit
		if (duplicateCount == 64) {
			return SRAM_FLASH_512KBIT;
		}
		else {
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
	char firstEightCheck[8];
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
		return EEPROM_64KBIT; 
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
			 (headerChar == 0x20)) {
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
	
	// SRAM/Flash check/size
	printf ("\nCalculating SRAM/Flash size");
	ramSize = gba_check_sram_flash();
	hasFlashSave = NOT_CHECKED; // Reset
	
	// EEPROM check
	eepromSize = gba_check_eeprom();
	
	
	// Print out
	printf ("\nROM size: %iMByte\n", romSize);
	romEndAddr = ((1024 * 1024) * romSize);
	
	printf ("SRAM/Flash size: ");
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
