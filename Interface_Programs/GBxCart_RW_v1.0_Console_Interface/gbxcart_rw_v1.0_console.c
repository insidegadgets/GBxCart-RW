/*
 GBxCart RW - Console Interface
 Version: 1.0
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 7/11/2016
 Last Modified: 11/03/2017
 
 GBxCartRead allows you to dump your Gameboy/Gameboy Colour/Gameboy Advance games ROM, save the RAM and write to the RAM.
 
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

#include "setup.c" // See defines, variables, constants, functions here

int main(int argc, char **argv) {
	
	printf("GBxCart RW v1.0 by insideGadgets\n");
	printf("################################\n");
	
	read_config();
	
	// Open the port
	if (RS232_OpenComport(cport_nr, bdrate, "8N1")) {
		return 0;
	}
	
	// Break out of any existing functions on ATmega
	set_mode('0');
	
	uint8_t inLoop = true;
	while (inLoop == true) {
		printf("\nPlease select an option:\n"\
				 "0. Read Header\n"\
				 "1. Dump ROM\n"\
				 "2. Save RAM to PC\n"\
				 "3. Write RAM to GB Cart\n"\
				 "4. Specify Cart ROM\n"\
				 "5. Specify Cart RAM\n"\
				 "x. Exit\n");
		char optionSelected = read_one_letter();
		
		// Get cartridge mode - Gameboy or Gameboy Advance
		cartridgeMode = read_cartridge_mode();
		
		if (optionSelected == '0') {
			printf("\n--- Read Header ---\n");
			
			if (cartridgeMode == GB_MODE) {
				read_gb_header();
			}
			else {
				read_gba_header();
			}
		}
		
		
		// Dump ROM		
		else if (optionSelected == '1') {
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
						print_progress_percent(readBytes, (romBanks * 16384) / 64);
					}
					RS232_cputs(cport_nr, "0"); // Stop reading ROM (as we will bank switch)
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
					print_progress_percent(currAddr, endAddr / 64);
				}
				printf("]");
				RS232_cputs(cport_nr, "0"); // Stop reading
			}
			
			fclose(romFile);
			printf("\nFinished\n");
		}
		
		
		// Save RAM
		else if (optionSelected == '2') {
			printf("\n--- Save RAM to PC---\n");
			
			if (cartridgeMode == GB_MODE) {
				// Does cartridge have RAM
				if (ramEndAddress > 0) {
					char titleFilename[30];
					strncpy(titleFilename, gameTitle, 20);
					strncat(titleFilename, ".sav", 4);
					
					// Check if file exists
					FILE *ramFile = fopen(titleFilename, "rb");
					char confirmWrite = 'y';
					if (ramFile != NULL) {
						printf("File %s exists on your PC.", titleFilename);
						printf("\n\n*** This will erase the save game from your PC ***");
						printf("\nPress y to continue or any other key to abort.\n");
						
						confirmWrite = read_one_letter();
						printf("\n");
						fclose(ramFile);
					}
					
					if (confirmWrite == 'y') {
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
								if (ramEndAddress == 0xA1FF) {
									print_progress_percent(readBytes, 64);
								}
								else if (ramEndAddress == 0xA7FF) {
									print_progress_percent(readBytes / 4, 64);
								}
								else {
									print_progress_percent(readBytes, (ramBanks * (ramEndAddress - 0xA000 + 1)) / 64);
								}
							}
							RS232_cputs(cport_nr, "0"); // Stop reading RAM (as we will bank switch)
						}
						printf("]");
						
						set_bank(0x0000, 0x00); // Disable RAM
						
						fclose(ramFile);
						printf("\nFinished\n");
					}
					else {
						printf("Aborted\n");
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
					
					// Check if file exists
					FILE *ramFile = fopen(titleFilename, "rb");
					char confirmWrite = 'y';
					if (ramFile != NULL) {
						printf("File %s exists on your PC.", titleFilename);
						printf("\n\n*** This will erase the save game from your PC ***");
						printf("\nPress y to continue or any other key to abort.\n");
						
						confirmWrite = read_one_letter();
						printf("\n");
						fclose(ramFile);
					}
					
					if (confirmWrite == 'y') {
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
									
									print_progress_percent(readBytes, (ramBanks * ramEndAddress) / 64);
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
								
								print_progress_percent(readBytes, endAddr / 64);
							}
						}
						printf("]");
						RS232_cputs(cport_nr, "0"); // Stop reading
						fclose(ramFile);
						printf("\nFinished\n");
					}
					else {
						printf("Aborted\n");
					}
				}
				else {
					printf("Cartridge has no RAM\n");
				}
			}
		}
		
		
		// Write RAM
		else if (optionSelected == '3') {
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
						printf("Going to write to RAM from %s...", titleFilename);
						printf("\n\n*** This will erase the save game from your Gameboy Cartridge ***");
						printf("\nPress y to continue or any other key to abort.\n");
						
						char confirmWrite = read_one_letter();
						if (confirmWrite == 'y') {
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
									if (ramEndAddress == 0xA1FF) {
										print_progress_percent(readBytes, 64);
									}
									else if (ramEndAddress == 0xA7FF) {
										print_progress_percent(readBytes / 4, 64);
									}
									else {
										print_progress_percent(readBytes, (ramBanks * (ramEndAddress - 0xA000 + 1)) / 64);
									}
									
									com_wait_for_ack();
								}
							}
							printf("]");
							set_bank(0x0000, 0x00); // Disable RAM
							
							fclose(ramFile);
							printf("\nFinished\n");
						}
						else {
							printf("Aborted\n");
						}
					}
					else {
						printf("%s File not found\n" ,titleFilename);
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
						
						printf("\n\n*** This will erase the save game from your Gameboy Advance Cartridge ***");
						printf("\nPress y to continue or any other key to abort.\n");
						
						char confirmWrite = read_one_letter();
						if (confirmWrite == 'y') {
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
									
									print_progress_percent(readBytes, ramEndAddress / 64);
									com_wait_for_ack();
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
									
									print_progress_percent(readBytes, endAddr / 64);
									
									// Wait for ATmega to process write (~320us) and for EEPROM to write data (6ms)
									com_wait_for_ack();
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
											
											print_progress_percent(readBytes, (ramBanks * endAddr) / 64);
											com_wait_for_ack(); // Wait for write complete
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
											
											print_progress_percent(readBytes, endAddr / 64);
											com_wait_for_ack(); // Wait for write complete
										}
									}
									
									set_number(0, GBA_FLASH_SET_BANK); // Set bank 0 again
								}
							}
							printf("]");
							
							fclose(ramFile);
							printf("\nFinished\n");
						}
						else {
							printf("Aborted\n");
						}
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
		
		
		// Specify cart info
		else if (optionSelected == '4') {
			printf("\n--- Specify ROM size ---\n");
			
			if (cartridgeMode == GB_MODE) {
				printf("1. 32KByte (no ROM banking)\n");
				printf("2. 64KByte (4 banks)\n");
				printf("3. 128KByte (8 banks)\n");
				printf("4. 256KByte (16 banks)\n");
				printf("5. 512KByte (32 banks)\n");
				printf("6. 1MByte (64 banks)  - only 63 banks used by MBC1\n");
				printf("7. 2MByte (128 banks) - only 125 banks used by MBC1\n");
				printf("8. 4MByte (256 banks)\n");
				printf("9. 8MByte (512 banks)\n");
				printf("x. Return\n");
				
				char selection[5];
				int selectionNumber;
				fgets(selection, sizeof selection, stdin);
				sscanf(selection, "%d", &selectionNumber);
				
				romSize = selectionNumber-1;
				romBanks = 2; // Default 32K
				if (romSize >= 1) { // Calculate rom size
					romBanks = 2 << romSize;
				}
			}
			else {
				printf("1. 4 Mbyte\n");
				printf("2. 8 Mbyte\n");
				printf("3. 16 Mbyte\n");
				printf("4. 32 Mbyte\n");
				printf("x. Return\n");
				
				char selection[5];
				int selectionNumber;
				fgets(selection, sizeof selection, stdin);
				sscanf(selection, "%d", &selectionNumber);
				
				romSize = 4 * selectionNumber;
				romEndAddr = ((1024 * 1024) * romSize);
			}
		}
		else if (optionSelected == '5') {
			printf("\n--- Specify RAM size ---\n");
			
			if (cartridgeMode == GB_MODE) {
				printf("1. None\n");
				printf("2. 2 KBytes\n");
				printf("3. 8 Kbytes\n");
				printf("4. 32 KBytes (4 banks of 8KBytes each)\n");
				printf("5. 128 KBytes (16 banks of 8KBytes each)\n");
				printf("6. 64 KBytes (8 banks of 8KBytes each)\n");
				printf("7. 512bytes (nibbles)\n");
				printf("x. Return\n");
				
				char selection[5];
				int selectionNumber;
				fgets(selection, sizeof selection, stdin);
				sscanf(selection, "%d", &selectionNumber);
				ramSize = selectionNumber - 1;
				printf("%d\n", ramSize);
				
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
			else {
				printf("Type: \n");
				printf("1. SRAM\n");
				printf("2. Flash\n");
				printf("3. EEPROM\n");
				printf("x. Return\n");
				
				char typeSelected = read_one_letter();
				
				if (typeSelected == '1' || typeSelected == '2') {
					eepromSize = EEPROM_NONE;
					eepromEndAddress = 0;
					
					if (typeSelected == '1') {
						hasFlashSave = NO_FLASH;
					}
					else {
						printf("\nFlash type: \n");
						printf("1. Atmel\n");
						printf("2. Other\n");
						
						char flashTypeSelected = read_one_letter();
						if (flashTypeSelected == 1) {
							hasFlashSave = FLASH_FOUND_ATMEL;
						}
						else if (flashTypeSelected == 2) {
							hasFlashSave = FLASH_FOUND;
						}
					}
					
					printf("\nSRAM/Flash Size: \n");
					printf("1. None\n");
					printf("2. 256Kbit\n");
					printf("3. 512Kbit\n");
					printf("4. 1Mbit\n");
					printf("x. Return\n");
					
					char selection[5];
					int selectionNumber;
					fgets(selection, sizeof selection, stdin);
					sscanf(selection, "%d", &selectionNumber);
					ramSize = selectionNumber - 1;
					
					if (ramSize == 0) {
						ramEndAddress = 0;
					}
					else if (ramSize == 1) {
						ramEndAddress = 0x8000;
						ramBanks = 1;
					}
					else if (ramSize == 2) {
						ramEndAddress = 0x10000;
						ramBanks = 1;
					}
					else if (ramSize == 3) {
						ramEndAddress = 0x10000;
						ramBanks = 2;
					}
				}
				else if (typeSelected == '3') {
					printf("\nEEPROM Size: \n");
					printf("1. None\n");
					printf("2. 4Kbit\n");
					printf("3. 64Kbit\n");
					printf("x. Return\n");
					
					char eepromTypeSelected = read_one_letter();
					
					if (eepromTypeSelected == '1') {
						eepromEndAddress = 0;
						eepromSize = EEPROM_NONE; 
						hasFlashSave = NO_FLASH;
						ramEndAddress = 0;
					}
					else if (eepromTypeSelected == '2') {
						eepromEndAddress = 0x200;
						eepromSize = EEPROM_4KBIT;
						hasFlashSave = NO_FLASH;
						ramEndAddress = 0;
					}
					else if (eepromTypeSelected == '3') {
						eepromEndAddress = 0x2000;
						eepromSize = EEPROM_64KBIT;
						hasFlashSave = NO_FLASH;
						ramEndAddress = 0;
					}
				}
			}
		}
		
		
		else if (optionSelected == 'x') { // Exit
			inLoop = false;
		}
		else {
			printf("\nUnknown command\n\n");
		}
	}
	
	return 0;
}
