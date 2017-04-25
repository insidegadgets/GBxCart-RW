/*

GBxCart RW - GUI COM Interface
Version : 1.1
Author : Alex from insideGadgets(www.insidegadgets.com)
Created : 7 / 11 / 2016
Last Modified : 4 / 04 / 2017

GBxCart RW allows you to dump your Gameboy / Gameboy Colour / Gameboy Advance games ROM, save the RAM and write to the RAM.

*/

#include <stdio.h>
#include <string.h>
#include <windows.h>

extern "C" {

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

int cport_nr = 7, // /dev/ttyS7 (COM8 on windows)
bdrate = 1000000; // 1,000,000 baud

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


// RS232 library license

/*
***************************************************************************
*
* Author: Teunis van Beelen
*
* Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Teunis van Beelen
*
* Email: teuniz@gmail.com
*
***************************************************************************
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
***************************************************************************
*/

/* Last revision: July 10, 2016 */

/* For more info and how to use this library, visit: http://www.teuniz.net/RS-232/ */

	#define RS232_PORTNR  16

	HANDLE Cport[RS232_PORTNR];

	char *comports[RS232_PORTNR] = { "\\\\.\\COM1",  "\\\\.\\COM2",  "\\\\.\\COM3",  "\\\\.\\COM4",
		"\\\\.\\COM5",  "\\\\.\\COM6",  "\\\\.\\COM7",  "\\\\.\\COM8",
		"\\\\.\\COM9",  "\\\\.\\COM10", "\\\\.\\COM11", "\\\\.\\COM12",
		"\\\\.\\COM13", "\\\\.\\COM14", "\\\\.\\COM15", "\\\\.\\COM16" };

	char mode_str[128];
	
	__declspec(dllexport) int RS232_OpenComport(int comport_number, int baudrate, const char *mode)
	{
		if ((comport_number >= RS232_PORTNR) || (comport_number<0))
		{
			printf("illegal comport number\n");
			return(1);
		}

		switch (baudrate)
		{
		case     110: strcpy(mode_str, "baud=110");
			break;
		case     300: strcpy(mode_str, "baud=300");
			break;
		case     600: strcpy(mode_str, "baud=600");
			break;
		case    1200: strcpy(mode_str, "baud=1200");
			break;
		case    2400: strcpy(mode_str, "baud=2400");
			break;
		case    4800: strcpy(mode_str, "baud=4800");
			break;
		case    9600: strcpy(mode_str, "baud=9600");
			break;
		case   19200: strcpy(mode_str, "baud=19200");
			break;
		case   38400: strcpy(mode_str, "baud=38400");
			break;
		case   57600: strcpy(mode_str, "baud=57600");
			break;
		case  115200: strcpy(mode_str, "baud=115200");
			break;
		case  128000: strcpy(mode_str, "baud=128000");
			break;
		case  256000: strcpy(mode_str, "baud=256000");
			break;
		case  500000: strcpy(mode_str, "baud=500000");
			break;
		case 1000000: strcpy(mode_str, "baud=1000000");
			break;
		default: printf("invalid baudrate\n");
			return(1);
			break;
		}

		if (strlen(mode) != 3)
		{
			printf("invalid mode \"%s\"\n", mode);
			return(1);
		}

		switch (mode[0])
		{
		case '8': strcat(mode_str, " data=8");
			break;
		case '7': strcat(mode_str, " data=7");
			break;
		case '6': strcat(mode_str, " data=6");
			break;
		case '5': strcat(mode_str, " data=5");
			break;
		default: printf("invalid number of data-bits '%c'\n", mode[0]);
			return(1);
			break;
		}

		switch (mode[1])
		{
		case 'N':
		case 'n': strcat(mode_str, " parity=n");
			break;
		case 'E':
		case 'e': strcat(mode_str, " parity=e");
			break;
		case 'O':
		case 'o': strcat(mode_str, " parity=o");
			break;
		default: printf("invalid parity '%c'\n", mode[1]);
			return(1);
			break;
		}

		switch (mode[2])
		{
		case '1': strcat(mode_str, " stop=1");
			break;
		case '2': strcat(mode_str, " stop=2");
			break;
		default: printf("invalid number of stop bits '%c'\n", mode[2]);
			return(1);
			break;
		}

		strcat(mode_str, " dtr=on rts=on");

		/*
		http://msdn.microsoft.com/en-us/library/windows/desktop/aa363145%28v=vs.85%29.aspx

		http://technet.microsoft.com/en-us/library/cc732236.aspx
		*/

		Cport[comport_number] = CreateFileA(comports[comport_number],
			GENERIC_READ | GENERIC_WRITE,
			0,                          /* no share  */
			NULL,                       /* no security */
			OPEN_EXISTING,
			0,                          /* no threads */
			NULL);                      /* no templates */

		if (Cport[comport_number] == INVALID_HANDLE_VALUE)
		{
			printf("unable to open comport\n");
			return(1);
		}

		DCB port_settings;
		memset(&port_settings, 0, sizeof(port_settings));  /* clear the new struct  */
		port_settings.DCBlength = sizeof(port_settings);

		if (!BuildCommDCBA(mode_str, &port_settings))
		{
			printf("unable to set comport dcb settings\n");
			CloseHandle(Cport[comport_number]);
			return(1);
		}

		if (!SetCommState(Cport[comport_number], &port_settings))
		{
			printf("unable to set comport cfg settings\n");
			CloseHandle(Cport[comport_number]);
			return(1);
		}

		COMMTIMEOUTS Cptimeouts;

		Cptimeouts.ReadIntervalTimeout = MAXDWORD;
		Cptimeouts.ReadTotalTimeoutMultiplier = 0;
		Cptimeouts.ReadTotalTimeoutConstant = 0;
		Cptimeouts.WriteTotalTimeoutMultiplier = 0;
		Cptimeouts.WriteTotalTimeoutConstant = 0;

		if (!SetCommTimeouts(Cport[comport_number], &Cptimeouts))
		{
			printf("unable to set comport time-out settings\n");
			CloseHandle(Cport[comport_number]);
			return(1);
		}

		return(0);
	}
	
	int RS232_PollComport(int comport_number, unsigned char *buf, int size)
	{
		int n;

		/* added the void pointer cast, otherwise gcc will complain about */
		/* "warning: dereferencing type-punned pointer will break strict aliasing rules" */

		ReadFile(Cport[comport_number], buf, size, (LPDWORD)((void *)&n), NULL);

		return(n);
	}


	int RS232_SendByte(int comport_number, unsigned char byte)
	{
		int n;

		WriteFile(Cport[comport_number], &byte, 1, (LPDWORD)((void *)&n), NULL);

		if (n<0)  return(1);

		return(0);
	}


	int RS232_SendBuf(int comport_number, unsigned char *buf, int size)
	{
		int n;

		if (WriteFile(Cport[comport_number], buf, size, (LPDWORD)((void *)&n), NULL))
		{
			return(n);
		}

		return(-1);
	}


	__declspec(dllexport) void RS232_CloseComport(int comport_number)
	{
		CloseHandle(Cport[comport_number]);
	}

	/*
	http://msdn.microsoft.com/en-us/library/windows/desktop/aa363258%28v=vs.85%29.aspx
	*/

	int RS232_IsDCDEnabled(int comport_number)
	{
		int status;

		GetCommModemStatus(Cport[comport_number], (LPDWORD)((void *)&status));

		if (status&MS_RLSD_ON) return(1);
		else return(0);
	}


	int RS232_IsCTSEnabled(int comport_number)
	{
		int status;

		GetCommModemStatus(Cport[comport_number], (LPDWORD)((void *)&status));

		if (status&MS_CTS_ON) return(1);
		else return(0);
	}


	int RS232_IsDSREnabled(int comport_number)
	{
		int status;

		GetCommModemStatus(Cport[comport_number], (LPDWORD)((void *)&status));

		if (status&MS_DSR_ON) return(1);
		else return(0);
	}


	void RS232_enableDTR(int comport_number)
	{
		EscapeCommFunction(Cport[comport_number], SETDTR);
	}


	void RS232_disableDTR(int comport_number)
	{
		EscapeCommFunction(Cport[comport_number], CLRDTR);
	}


	void RS232_enableRTS(int comport_number)
	{
		EscapeCommFunction(Cport[comport_number], SETRTS);
	}


	void RS232_disableRTS(int comport_number)
	{
		EscapeCommFunction(Cport[comport_number], CLRRTS);
	}

	/*
	https://msdn.microsoft.com/en-us/library/windows/desktop/aa363428%28v=vs.85%29.aspx
	*/

	void RS232_flushRX(int comport_number)
	{
		PurgeComm(Cport[comport_number], PURGE_RXCLEAR | PURGE_RXABORT);
	}


	void RS232_flushTX(int comport_number)
	{
		PurgeComm(Cport[comport_number], PURGE_TXCLEAR | PURGE_TXABORT);
	}


	void RS232_flushRXTX(int comport_number)
	{
		PurgeComm(Cport[comport_number], PURGE_RXCLEAR | PURGE_RXABORT);
		PurgeComm(Cport[comport_number], PURGE_TXCLEAR | PURGE_TXABORT);
	}


	void RS232_cputs(int comport_number, const char *text)  /* sends a string to serial port */
	{
		while (*text != 0)   RS232_SendByte(comport_number, *(text++));
	}


	/* return index in comports matching to device name or -1 if not found */
	int RS232_GetPortnr(const char *devname)
	{
		int i;

		char str[32];

		strcpy(str, "\\\\.\\");

		strncat(str, devname, 16);
		str[31] = 0;

		for (i = 0; i<RS232_PORTNR; i++)
		{
			if (!strcmp(comports[i], str))
			{
				return i;
			}
		}

		return -1;  /* device not found */
	}


__declspec(dllexport) int read_config(int type) {
	FILE* configfile = fopen("config.ini", "rb");
	char* buffer;
	if (configfile != NULL) {
		// Allocate memory 
		buffer = (char*)malloc(sizeof(char) * 2);

		// Copy the file into the buffer, we only read 2 characters
		fread(buffer, 1, 2, configfile);

		uint8_t numbersFound = 0;
		for (uint8_t x = 0; x < 2; x++) {
			if (buffer[x] >= 48 && buffer[x] <= 57) {
				numbersFound++;
			}
		}

		if (numbersFound >= 1) {
			cport_nr = atoi(buffer);
		}

		// Remove the \r\n line
		fread(buffer, 1, numbersFound, configfile);

		// Read the baud rate
		fread(buffer, 1, 7, configfile);

		for (uint8_t x = 0; x < 7; x++) {
			if (buffer[x] >= 48 && buffer[x] <= 57) {
				numbersFound++;
			}
		}

		if (numbersFound >= 1) {
			bdrate = atoi(buffer);
		}

		fclose(configfile);
		//free(buffer);

		if (type == 1) {
			return cport_nr;
		}
		else {
			return bdrate;
		}
	}

	return 0;
}

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

__declspec(dllexport) void gb_specify_rom_size(int size) {
	romSize = size;
	romBanks = 2; // Default 32K
	if (romSize >= 1) { // Calculate rom size
		romBanks = 2 << romSize;
	}
}

__declspec(dllexport) void gba_specify_rom_size(int size) {
	romSize = 4 * size;
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
		if (size == 1) {
			eepromEndAddress = 0;
			eepromSize = EEPROM_NONE;
			hasFlashSave = NO_FLASH;
			ramEndAddress = 0;
		}
		else if (size == 2) {
			eepromEndAddress = 0x200;
			eepromSize = EEPROM_4KBIT;
			hasFlashSave = NO_FLASH;
			ramEndAddress = 0;
		}
		else if (size == 3) {
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

// Read 1-128 bytes from the file and write it the COM port with the command given
void com_write_bytes_from_file(uint8_t command, FILE *file, uint8_t count) {
	uint8_t buffer[129];

	buffer[0] = command;
	fread(&buffer[1], 1, count, file);
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
	//set_mode(GBA_MODE);

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
uint8_t gba_check_sram_flash(void) {
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
__declspec(dllexport) char* read_gba_header(int &headerlength) {
	set_mode('0'); // Break out of any loops on ATmega

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
			(headerChar == 0x20)) { // space
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

	// SRAM/Flash check/size
	printf("\nCalculating SRAM/Flash size");
	ramSize = gba_check_sram_flash();
	hasFlashSave = NOT_CHECKED; // Reset

	// EEPROM check
	eepromSize = gba_check_eeprom();


	// Print out
	printf("\nROM size: %iMByte\n", romSize);
	sprintf(headerText + strlen(headerText), "ROM size: %iMByte\n", romSize);
	romEndAddr = ((1024 * 1024) * romSize);

	printf("SRAM/Flash size: ");
	sprintf(headerText + strlen(headerText), "SRAM / Flash size: ");
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
		for (uint8_t bank = 1; bank < romBanks; bank++) {
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

						print_progress_percent(progress, readBytes, ramBanks * ramEndAddress);

						if (cancelOperation == 1) {
							break;
						}
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
			}
			printf("]");
			RS232_cputs(cport_nr, "0"); // Stop reading
			fclose(ramFile);
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

					if (hasFlashSave > 1) {
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
					if (hasFlashSave > 1) {
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
								}

								com_write_bytes_from_file(GBA_FLASH_WRITE_BYTE, ramFile, 64);
								currAddr += 64;
								readBytes += 64;

								print_progress_percent(progress, readBytes, endAddr);
								com_wait_for_ack(); // Wait for write complete

								if (cancelOperation == 1) {
									break;
								}
							}
						}

						set_number(0, GBA_FLASH_SET_BANK); // Set bank 0 again

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

}