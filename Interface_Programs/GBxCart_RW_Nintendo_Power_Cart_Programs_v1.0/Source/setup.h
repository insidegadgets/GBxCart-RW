/*
 GBxCart RW - Nintendo Power Cart
 Version: 1.0
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 20/03/2019
 Last Modified: 22/03/2019
 
 */

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <stdint.h>
#include <stdio.h>

#define LOW 0
#define HIGH 1
#define false 0
#define true 1

// COM Port settings (default)
#include "rs232/rs232.h"
extern int cport_nr;
extern int bdrate;

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
#define GB_FLASH_WRITE_NP_128BYTE 'Z'


#define GB_FLASH_BANK_1_COMMAND_WRITES 'N'

#define GBA_FLASH_CART_WRITE_BYTE 'n'
#define GBA_FLASH_WRITE_64BYTE_SWAPPED_D0D1 'q'
#define GBA_FLASH_WRITE_256BYTE_SWAPPED_D0D1 't'
#define GBA_FLASH_WRITE_256BYTE 'f'
#define GBA_FLASH_WRITE_INTEL_64BYTE 'l'


// General commands
#define CART_MODE 'C'
#define SET_INPUT 'I'
#define SET_OUTPUT 'O'
#define SET_OUTPUT_LOW 'L'
#define SET_OUTPUT_HIGH 'H'
#define READ_INPUT 'D'
#define RESET_COMMON_LINES 'M'
#define READ_FIRMWARE_VERSION 'V'
#define READ_PCB_VERSION 'h'

#define VOLTAGE_3_3V '3'
#define VOLTAGE_5V '5'

#define RESET_AVR '*'
#define RESET_VALUE 0x7E5E1

// PCB versions
#define PCB_1_0 1
#define PCB_1_1 2
#define PCB_1_3 4

// Common vars
#define READ_BUFFER 0

extern uint8_t gbxcartFirmwareVersion;
extern uint8_t gbxcartPcbVersion;
extern uint8_t readBuffer[257];
extern uint8_t writeBuffer[257];

extern char gameTitle[17];
extern uint16_t cartridgeType;
extern uint32_t currAddr;
extern uint32_t endAddr;
extern uint16_t romSize;
extern uint32_t romEndAddr;
extern uint16_t romBanks;
extern int ramSize;
extern uint16_t ramBanks;
extern uint32_t ramEndAddress;
extern int eepromSize;
extern uint16_t eepromEndAddress;
extern int hasFlashSave;
extern uint8_t cartridgeMode;
uint8_t npHiddenRegister[256];

// Read the config.ini file for the COM port to use and baud rate
void read_config(void);

// Write the config.ini file for the COM port to use and baud rate
void write_config(void);

// Load a file which contains the cartridge RAM settings (only needed if Erase RAM option was used, only applies to GBA games)
void load_cart_ram_info(void);

// Write a file which contains the cartridge RAM settings before it's wiped using Erase RAM (Only applies to GBA games)
void write_cart_ram_info(void);

void delay_ms(uint16_t ms);

// Read one letter from stdin
char read_one_letter(void);

// Print progress
void print_progress_percent(uint32_t bytesRead, uint32_t hashNumber);

// Wait for a "1" acknowledgement from the ATmega
void com_wait_for_ack (void);

// Stop reading blocks of data
void com_read_stop(void);

// Continue reading the next block of data
void com_read_cont(void);

// Test opening the COM port,if can't be open, try autodetecting device on other COM ports
uint8_t com_test_port(void);


// Read 1 to 64 bytes from the COM port and write it to the global read buffer or to a file if specified. 
// When polling the com port it return less than the bytes we want, keep polling and wait until we have all bytes requested. 
// We expect no more than 64 bytes.
uint16_t com_read_bytes(FILE *file, int count);

// Read 1-128 bytes from the file (or buffer) and write it the COM port with the command given
void com_write_bytes_from_file(uint8_t command, FILE *file, int count);

// Send a single command byte
void set_mode (char command);

// Send a command with a hex number and a null terminator byte
void set_number (uint32_t number, uint8_t command);

// Read the cartridge mode
uint8_t read_cartridge_mode (void);

// Send 1 byte and read 1 byte
uint8_t request_value (uint8_t command);



// ****** Gameboy / Gameboy Colour functions ******

// Set bank for ROM/RAM switching, send address first and then bank number
void set_bank (uint16_t address, uint8_t bank);

// MBC2 Fix (unknown why this fixes reading the ram, maybe has to read ROM before RAM?)
// Read 64 bytes of ROM, (really only 1 byte is required)
void mbc2_fix (void);

uint8_t check_for_valid_nintendo_logo (void);

// Read the first 384 bytes of ROM and process the Gameboy header information
void read_gb_header (void);


// ****** Gameboy Advance functions ****** 

// Check the rom size by reading 64 bytes from different addresses and checking if they are all 0x00. There can be some ROMs 
// that do have valid 0x00 data, so we check 32 different addresses in a 4MB chunk, if 30 or more are all 0x00 then we've reached the end.
uint8_t gba_check_rom_size (void);

// Used before we write to RAM as we need to check if we have an SRAM or Flash. 
// Write 1 byte to 0x00 on the SRAM/Flash save, if we read it back successfully then we know SRAM is present, then we write
// the original byte back to how it was. This can be a destructive process to the first byte, if anything goes wrong the user
// could lose the first byte, so we only do this check when writing a save back to the SRAM/Flash.
uint8_t gba_test_sram_flash_write (void);

// Check if SRAM/Flash is present and test the size. 
// When a 256Kbit SRAM is read past 256Kbit, the address is loops around, there are some times where the bytes don't all 
// match up 100%, it's like 90% so be a bit lenient. A cartridge that doesn't have an SRAM/Flash reads all 0x00's.
uint8_t gba_check_sram_flash (void);

// Erase 4K sector on flash on sector address
void flash_4k_sector_erase (uint8_t sector);

// Check if an EEPROM is present and test the size. A 4Kbit EEPROM when accessed like a 64Kbit EEPROM sends the first 8 bytes over
// and over again. A cartridge that doesn't have an EEPROM reads all 0x00 or 0xFF.
uint8_t gba_check_eeprom (void);

// Read GBA game title (used for reading title when ROM mapping)
void gba_read_gametitle(void);

// Read the first 192 bytes of ROM, read the title, check and test for ROM, SRAM, EEPROM and Flash
void read_gba_header (void);



// ****** GBA Cart Flasher functions ******

uint8_t gba_detect_intel_flash_cart(void);

// GBA Flash Cart, write address and byte
void gba_flash_write_address_byte (uint32_t address, uint16_t byte);



// ****** GB Cart Flasher functions ******

// Write flash config file
void write_flash_config(int number);

// Read the config-flash.ini file for the flash cart type
void read_config_flash(void);

// Wait for first byte of chosen address to be 0xFF, that's when we know the sector has been erased
void wait_for_flash_sector_ff(uint16_t address);

// Wait for first byte of Flash to be 0xFF, that's when we know the sector has been erased
void wait_for_flash_chip_erase_ff(void);

// Select which pin need to pulse as WE (Audio or WR)
void gb_flash_pin_setup(char pin);

// Select which flash program method to use
void gb_flash_program_setup(uint8_t method);

// Write address and byte to flash
void gb_flash_write_address_byte (uint16_t address, uint8_t byte);

// Nintendo Power - Enable flash chip access, disable flash chip protection
void gb_np_enable_flash_chip_access(void);

// Nintendo Power - Unlock sector 0
void gb_np_unlock_sector_0(void);

void gb_np_backup_hidden_sector(void);
void gb_np_chip_erase(void);
void gb_np_erase_hidden_sector(void);
void gb_np_write_hidden_sector(void);
void gb_np_map_all_flash_memory(void);
