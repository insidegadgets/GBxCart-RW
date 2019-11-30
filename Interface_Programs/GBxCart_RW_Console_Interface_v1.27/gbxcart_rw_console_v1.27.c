/*
 GBxCart RW - Console Interface
 Version: 1.27
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 7/11/2016
 Last Modified: 30/11/2019
 
 GBxCart RW allows you to dump your Gameboy/Gameboy Colour/Gameboy Advance games ROM, save the RAM and write to the RAM.
 
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
	
	printf("GBxCart RW v1.27 by insideGadgets\n");
	printf("################################\n");
	
	read_config();
	
	// Open COM port
	if (com_test_port() == 0) {
		printf("Device not connected and couldn't be auto detected\n");
		read_one_letter();
		return 1;
	}
	printf("Connected on COM port: %i\n", cport_nr+1);
	
	// Break out of any existing functions on ATmega
	set_mode('0');
	
	// Get firmware version
	gbxcartFirmwareVersion = request_value(READ_FIRMWARE_VERSION);
	
	// Get PCB version
	gbxcartPcbVersion = request_value(READ_PCB_VERSION);
	
	// Prompt for GB or GBA mode
	if (gbxcartPcbVersion == PCB_1_3) {
		printf("\nPlease select a mode:\n"\
				"1. GB/GBC\n"\
				"2. GBA\n"); 
		
		char modeSelected = read_one_letter();
		if (modeSelected == '1') {
			set_mode(VOLTAGE_5V);
		}
		else {
			set_mode(VOLTAGE_3_3V);
		}
	}
	
	uint8_t inLoop = true;
	while (inLoop == true) {
		printf("\nPlease select an option:\n"\
				 "0. Read Header\n"\
				 "1. Read ROM\n"\
				 "2. Backup Save (Cart to PC)\n"\
				 "3. Restore Save (PC to Cart)\n"\
				 "4. Erase Save from Cart\n"\
				 "5. Specify Cart ROM/MBC\n"\
				 "6. Specify Cart RAM\n"\
				 "7. Custom commands\n"\
				 "8. Other options\n"\
				 "x. Exit\n>"); 
		char optionSelected = read_one_letter();
		
		// Get cartridge mode - Gameboy or Gameboy Advance
		cartridgeMode = request_value(CART_MODE);
		
		
		// Read header
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
			printf("\n--- Read ROM ---\n");
			
			char titleFilename[30];
			strncpy(titleFilename, gameTitle, 20);
			if (cartridgeMode == GB_MODE) {
				strncat(titleFilename, ".gb", 3);
			}
			else {
				strncat(titleFilename, ".gba", 4);
			}
			printf("Reading ROM to %s\n", titleFilename);
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
						set_bank(0x2100, bank & 0xFF);
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
					}
					com_read_stop(); // Stop reading ROM (as we will bank switch)
				}
				printf("]");
			}
			else { // GBA mode
				// Set start and end address
				currAddr = 0x00000;
				endAddr = romEndAddr;
				set_number(currAddr, SET_START_ADDRESS);
				
				uint16_t readLength = 64;
				set_mode(GBA_READ_ROM);
				
				// Read data
				while (currAddr < endAddr) {
					uint8_t comReadBytes = com_read_bytes(romFile, readLength);
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
						delay_ms(500);
						
						// Flush buffer
						RS232_PollComport(cport_nr, readBuffer, readLength);											
						
						// Start off where we left off
						fseek(romFile, currAddr, SEEK_SET);
						set_number(currAddr / 2, SET_START_ADDRESS);
						set_mode(GBA_READ_ROM);				
					}
					
					// Print progress
					print_progress_percent(currAddr, endAddr / 64);
				}
				printf("]");
				com_read_stop();
			}
			
			fclose(romFile);
			printf("\nFinished\n");
		}
		
		
		// Save RAM
		else if (optionSelected == '2') {
			printf("\n--- Backup save from Cartridge to PC---\n");
			
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
						printf("Backing up save to %s\n", titleFilename);
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
								RS232_drain(cport_nr);
								delay_ms(5);
								
								set_mode(GB_CART_MODE);
								
								while (ramAddress < ramEndAddress) {
									for (uint8_t x = 0; x < 64; x++) {
										
										char hexNum[7];
										sprintf(hexNum, "HA0x%x", ((ramAddress+x) >> 8));
										RS232_cputs(cport_nr, hexNum);
										RS232_SendByte(cport_nr, 0);
										RS232_drain(cport_nr);
										
										sprintf(hexNum, "HB0x%x", ((ramAddress+x) & 0xFF));
										RS232_cputs(cport_nr, hexNum);
										RS232_SendByte(cport_nr, 0);
										RS232_drain(cport_nr);
										
										RS232_cputs(cport_nr, "LD0x60"); // cs_mreqPin_low + rdPin_low
										RS232_SendByte(cport_nr, 0);
										RS232_drain(cport_nr);
										
										RS232_cputs(cport_nr, "DC");
										RS232_drain(cport_nr);
										
										RS232_cputs(cport_nr, "HD0x60"); // cs_mreqPin_high + rdPin_high
										RS232_SendByte(cport_nr, 0);
										RS232_drain(cport_nr);
										
										RS232_cputs(cport_nr, "LA0xFF");
										RS232_SendByte(cport_nr, 0);
										RS232_drain(cport_nr);
										
										RS232_cputs(cport_nr, "LB0xFF");
										RS232_SendByte(cport_nr, 0);
										RS232_drain(cport_nr);
									}
									
									com_read_bytes(ramFile, 64);
									
									ramAddress += 64;
									readBytes += 64;
									
									// Request 64 bytes more
									if (ramAddress < ramEndAddress) {
										com_read_cont();
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
								com_read_stop(); // Stop reading RAM (as we will bank switch)
							}
							
							RS232_cputs(cport_nr, "M1");
							RS232_drain(cport_nr);
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
										delay_ms(500);
										printf("Retrying\n");
										
										// Flush buffer
										RS232_PollComport(cport_nr, readBuffer, 64);											
										
										// Start off where we left off
										fseek(ramFile, readBytes, SEEK_SET);
										set_number(ramAddress, SET_START_ADDRESS);
										set_mode(READ_ROM_RAM);				
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
								com_read_stop(); // Stop reading RAM (as we will bank switch)
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
							printf("Backing up save (SRAM/Flash) to %s\n", titleFilename);
							printf("[             25%%             50%%             75%%            100%%]\n[");
							
							// Read RAM
							uint32_t readBytes = 0;
							for (uint8_t bank = 0; bank < ramBanks; bank++) {
								// Flash, switch bank 1
								if (hasFlashSave >= FLASH_FOUND && bank == 1) {
									set_number(1, GBA_FLASH_SET_BANK);
								}
								else if (hasFlashSave == NO_FLASH && bank == 1) { // 1Mbit SRAM
									gba_flash_write_address_byte(0x1000000, 0x1);
								}
								
								// Set start and end address
								currAddr = 0x00000;
								endAddr = ramEndAddress;
								set_number(currAddr, SET_START_ADDRESS);
								set_mode(GBA_READ_SRAM);
								
								while (currAddr < endAddr) {
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
										delay_ms(500);
										printf("Retrying\n");
										
										// Flush buffer
										RS232_PollComport(cport_nr, readBuffer, 64);											
										
										// Start off where we left off
										fseek(ramFile, currAddr, SEEK_SET);
										set_number(currAddr, SET_START_ADDRESS);
										set_mode(GBA_READ_SRAM);				
									}
									
									print_progress_percent(readBytes, (ramBanks * ramEndAddress) / 64);
								}
								
								com_read_stop(); // End read (for bank if flash)
								
								// Flash, switch back to bank 0
								if (hasFlashSave >= FLASH_FOUND && bank == 1) {
									set_number(0, GBA_FLASH_SET_BANK);
								}
								// SRAM 1Mbit, switch back to bank 0
								else if (hasFlashSave == NO_FLASH && bank == 1) {
									gba_flash_write_address_byte(0x1000000, 0x0);
								}
							}
						}
						
						// EEPROM
						else {
							printf("Backing up save (EEPROM) to %s\n", titleFilename);
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
								
								// Request 8 bytes more
								if (currAddr < endAddr) {
									com_read_cont();
								}
								
								print_progress_percent(readBytes, endAddr / 64);
							}
							
							com_read_stop(); // End read
						}
						
						fclose(ramFile);
						printf("]");
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
			printf("\n--- Restore save from PC to Cartridge ---\n");
			
			if (cartridgeMode == GB_MODE) {
				// Does cartridge have RAM
				if (ramEndAddress > 0) {
					char titleFilename[30];
					strncpy(titleFilename, gameTitle, 20);
					strncat(titleFilename, ".sav", 4);
					
					// Open file
					FILE *ramFile = fopen(titleFilename, "rb");
					if (ramFile != NULL) {
						printf("Going to write save from %s...", titleFilename);
						printf("\n\n*** This will erase the save game from your Gameboy Cartridge ***");
						printf("\nPress y to continue or any other key to abort.\n");
						
						char confirmWrite = read_one_letter();
						if (confirmWrite == 'y') {
							printf("\nRestoring save from %s\n", titleFilename);
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
							
							if (hasFlashSave >= FLASH_FOUND) {
								printf("Going to write save to Flash from %s", titleFilename);
							}
							else {
								printf("Going to write save to SRAM from %s", titleFilename);
							}
						}
						else {			
							printf("Going to write save to EEPROM from %s", titleFilename);
						}
						
						printf("\n\n*** This will erase the save game from your Gameboy Advance Cartridge ***");
						printf("\nPress y to continue or any other key to abort.\n");
						
						char confirmWrite = read_one_letter();
						if (confirmWrite == 'y') {
							if (eepromSize == EEPROM_NONE) {
								if (hasFlashSave >= FLASH_FOUND) {
									printf("\nWriting Save to Flash from %s", titleFilename);
								}
								else {
									printf("\nWriting Save to SRAM from %s", titleFilename);
								}
							}
							else {
								printf("\nWriting Save to EEPROM from %s", titleFilename);
							}
							printf("\n[             25%%             50%%             75%%            100%%]\n[");
							
							// SRAM
							if (hasFlashSave == NO_FLASH && eepromSize == EEPROM_NONE) {
								
								uint32_t readBytes = 0;
								for (uint8_t bank = 0; bank < ramBanks; bank++) {
									if (bank == 1) { // 1Mbit SRAM
										gba_flash_write_address_byte(0x1000000, 0x1);
									}
									
									// Set start and end address
									currAddr = 0x0000;
									endAddr = ramEndAddress;
									set_number(currAddr, SET_START_ADDRESS);
									
									// Write
									while (currAddr < endAddr) {
										com_write_bytes_from_file(GBA_WRITE_SRAM, ramFile, 64);
										currAddr += 64;
										readBytes += 64;
										
										print_progress_percent(readBytes, ramEndAddress * ramBanks / 64);
										com_wait_for_ack();
									}
									
									// SRAM 1Mbit, switch back to bank 0
									if (bank == 1) {
										gba_flash_write_address_byte(0x1000000, 0x0);
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
												
												// Wait for first byte to be 0xFF, that's when we know the sector has been erased
												readBuffer[0] = 0;
												while (readBuffer[0] != 0xFF) {
													set_number(currAddr, SET_START_ADDRESS);
													set_mode(GBA_READ_SRAM);
													
													com_read_bytes(READ_BUFFER, 64);
													com_read_stop();
													
													if (readBuffer[0] != 0xFF) {
														delay_ms(5);
													}
												}
												
												// Set start address again
												set_number(currAddr, SET_START_ADDRESS);
												
												delay_ms(5); // Wait a little bit as hardware might not be ready
											}
											
											com_write_bytes_from_file(GBA_FLASH_WRITE_BYTE, ramFile, 64);
											currAddr += 64;
											readBytes += 64;
											
											print_progress_percent(readBytes, (ramBanks * endAddr) / 64);
											com_wait_for_ack(); // Wait for write complete
										}
									}
									
									if (bank == 1) {
										set_number(0, GBA_FLASH_SET_BANK); // Set bank 0 again
									}
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
		
		
		// Erase save
		else if (optionSelected == '4') {
			printf("\n--- Erase save from Cart ---\n");
			printf("*** This will erase the save game from your Gameboy/Gameboy Advance Cart ***");
			printf("\nPress y to continue or any other key to abort.\n");
			
			char confirmWrite = read_one_letter();
			if (confirmWrite == 'y') {
				// Default for SRAM
				for (uint8_t x = 0; x < 128; x++) {
					writeBuffer[x] = 0x00;
				}
				
				cartridgeMode = read_cartridge_mode();
				if (cartridgeMode == GB_MODE) {
					printf("\nErasing save from Cart");
					printf("\n[             25%%             50%%             75%%            100%%]\n[");
					
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
						
						printf("\nFinished\n");
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
						
						printf("\nErasing save from Cart");
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
								com_write_bytes_from_file(GBA_WRITE_SRAM, NULL, 64);
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
								com_write_bytes_from_file(GBA_WRITE_EEPROM, NULL, 8);
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
										com_write_bytes_from_file(GBA_FLASH_WRITE_ATMEL, NULL, 128);
										currAddr += 128;
										readBytes += 128;
										
										print_progress_percent(readBytes, (ramBanks * endAddr) / 64);
										com_wait_for_ack(); // Wait for write complete	
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
												com_read_stop();
												
												if (readBuffer[0] != 0xFF) {
													delay_ms(5);
												}
											}
										}
										
										com_write_bytes_from_file(GBA_FLASH_WRITE_BYTE, NULL, 64);
										currAddr += 64;
										readBytes += 64;
										
										print_progress_percent(readBytes, (ramBanks * endAddr) / 64);
										com_wait_for_ack(); // Wait for write complete
									}
								}
								
								if (bank == 1) {
									set_number(0, GBA_FLASH_SET_BANK); // Set bank 0 again
								}
							}
						}
						printf("]");
						printf("\nFinished\n");
					}
				}
			}
		}
		
		
		// Specify cart info
		else if (optionSelected == '5') {
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
				
				printf("\n--- Specify MBC Type ---\n");
				printf("1. MBC1\n");
				printf("2. MBC2\n");
				printf("3. MBC3\n");
				printf("4. MBC5\n");
				
				selection[0] = 0;
				selectionNumber = 0;
				fgets(selection, sizeof selection, stdin);
				sscanf(selection, "%d", &selectionNumber);
				
				if (selectionNumber == 1) {
					cartridgeType = 3;
				}
				else if (selectionNumber == 2) {
					cartridgeType = 6;
				}
				else if (selectionNumber == 3) {
					cartridgeType = 19;
				}
				else if (selectionNumber == 4) {
					cartridgeType = 27;
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
				
				romSize = 4 << (selectionNumber - 1);
				romEndAddr = ((1024 * 1024) * romSize);
				printf("%i", romEndAddr);
			}
		}
		else if (optionSelected == '6') {
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
						if (flashTypeSelected == '1') {
							hasFlashSave = FLASH_FOUND_ATMEL;
						}
						else if (flashTypeSelected == '2') {
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
		
		// Custom commands	
		else if (optionSelected == '7') {
			RS232_cputs(cport_nr, "G");
			RS232_drain(cport_nr);
			delay_ms(5);
			
			printf("\n--- Custom Commands ---\n"\
					 "Enter the custom command from the list:\n"\
					 "Type x to exit\n\n");
			
			while (1) {
				printf(">");
				
				char readInput[10];
				fgets(readInput, sizeof readInput, stdin);
				//printf("%s, %i\n", readInput, strlen(readInput));
				
				if (readInput[0] == 'x') {
					break;
				}
				
				for (uint8_t x = 0; x < strlen(readInput); x++) {
					if (readInput[x] >= 97 && readInput[x] <= 122 && readInput[x] != 120) {
						readInput[x] -= 32;
					}
				}
				//printf("%s, %i\n", readInput, strlen(readInput));
				
				
				RS232_cputs(cport_nr, readInput);
				RS232_drain(cport_nr);
				
				if (readInput[0] == 'I' || readInput[0] == 'O' || readInput[0] == 'L' || readInput[0] == 'H') {
					RS232_SendByte(cport_nr, 0);
					RS232_drain(cport_nr);
				}
				
				if (readInput[0] == 'D') {
					for (uint8_t x = 0; x < 20; x++) {
						readBuffer[x] = 0;
					}
					
					com_read_bytes(READ_BUFFER, 1);
					if (readBuffer[0] <= 0x0F) {
						printf("Read: 0x0%X\n", readBuffer[0]);
					}
					else {
						printf("Read: 0x%X\n", readBuffer[0]);
					}
				}
			}
		}
		
		
		// Other options
		else if (optionSelected == '8') {
			printf("\n--- Other options ---\n"\
					 "1. Sachen ROM mapper\n"\
					 "2. GBA Flash cart ROM mapper\n"\
					 "3. GB \"22 in 1\" Cart Bank Reader\n"\
					 "x. Exit\n>");
			char otherOptionSelected = read_one_letter();
			
			// Sachen ROM mapper
			if (otherOptionSelected == '1') {
				printf("\n--- Sachen ROM mapper ---\n"\
							"Used for mapping ROMs to 0x00 and decoding the Sachen header.\n"\
							"Type x to exit");
				
				while (1) {
					printf("\n\nEnter the ROM start location in Hex: 0x");
					
					// Address
					char readInput[10];
					readInput[0] = '0';
					readInput[1] = 'x';
					fgets(&readInput[2], sizeof(readInput)-3, stdin);
					fflush(stdin);
					
					if (readInput[2] == 'x') {
						break;
					}
					
					int readAddress = (int) strtol(readInput, NULL, 16);
					
					// ROM size
					printf("\nSelect the ROM size\n");
					printf("1. 32KByte (no ROM banking)\n");
					printf("2. 64KByte (4 banks)\n");
					printf("3. 128KByte (8 banks)\n");
					printf("4. 256KByte (16 banks)\n");
					printf("5. 512KByte (32 banks)\n");
					printf(">");
					
					char selection[5];
					int selectionNumber;
					fgets(selection, sizeof selection, stdin);
					fflush(stdin);
					
					if (selection[0] == 'x') {
						break;
					}
					
					sscanf(selection, "%d", &selectionNumber);
					
					romSize = selectionNumber-1;
					romBanks = 2; // Default 32K
					if (romSize >= 1) { // Calculate rom size
						romBanks = 2 << romSize;
					}
					int romBase = readAddress / 0x4000;
					int romMask = 0xFF - (romSize * 2) - 1;
					
					if (romBase <= 0x0F) {
						printf("\nSelecting ROM Base 0x0%X at Address: 0x0%X, Size: %iKByte, ROM Mask 0x%X\n", romBase, readAddress, (1<<romSize) * 32, romMask);
					}
					else {
						printf("\nSelecting ROM Base 0x%X at Address: 0x%X, Size: %iKByte, ROM Mask 0x%X\n", romBase, readAddress, (1<<romSize) * 32, romMask);
					}
					
					RS232_cputs(cport_nr, "G"); // Set Gameboy mode
					RS232_drain(cport_nr);
					delay_ms(5);
					
					RS232_cputs(cport_nr, "M0"); // Disable CS/RD/WR/CS2-RST from going high after each command
					RS232_drain(cport_nr);
					delay_ms(5);
					
					// V1.1 PCB
					if (gbxcartPcbVersion == PCB_1_1) {
						RS232_cputs(cport_nr, "OE0x04"); // Pulse Reset
						RS232_SendByte(cport_nr, 0);
						RS232_drain(cport_nr);
						delay_ms(5);
						
						RS232_cputs(cport_nr, "LE0x04");
						RS232_SendByte(cport_nr, 0);
						RS232_drain(cport_nr);
						delay_ms(5);
						
						RS232_cputs(cport_nr, "HE0x04");
						RS232_SendByte(cport_nr, 0);
						RS232_drain(cport_nr);
						delay_ms(5);
					}
					else { // V1.0 PCB
						RS232_cputs(cport_nr, "OD0x80"); // Pulse Reset
						RS232_SendByte(cport_nr, 0);
						RS232_drain(cport_nr);
						delay_ms(5);
						
						RS232_cputs(cport_nr, "LD0x80");
						RS232_SendByte(cport_nr, 0);
						RS232_drain(cport_nr);
						delay_ms(5);
						
						RS232_cputs(cport_nr, "HD0x80");
						RS232_SendByte(cport_nr, 0);
						RS232_drain(cport_nr);
						delay_ms(5);
					}
					
					// Pulse A15 pin 0x60 times
					RS232_cputs(cport_nr, "OA0x80");
					RS232_SendByte(cport_nr, 0);
					RS232_drain(cport_nr);
					
					for (uint8_t x = 0; x < 0x60; x++) {
						RS232_cputs(cport_nr, "HA0x80");
						RS232_SendByte(cport_nr, 0);
						RS232_drain(cport_nr);
						delay_ms(5);
						
						RS232_cputs(cport_nr, "LA0x80");
						RS232_SendByte(cport_nr, 0);
						RS232_drain(cport_nr);
						delay_ms(5);
					}
					
					RS232_cputs(cport_nr, "M1"); // Enable CS/RD/WR/CS2-RST goes high after each command
					RS232_drain(cport_nr);
					
					
					// Allow ROM bank/mask changes
					set_bank(0x2000, 0x30);
					delay_ms(5);
					
					// Set ROM base bank
					set_bank(0x0000, romBase);
					delay_ms(5);
					
					// Set ROM mask
					set_bank(0x4000, romMask);
					delay_ms(5);
					
					// Apply changes
					set_bank(0x2000, 0x00);
					delay_ms(5);
					
					printf("Done\n");
					
					
					// Set ROM title
					strncpy(gameTitle, "Sachen_", 7);
					
					char addressString[20];
					sprintf(addressString, "0x%X", readAddress);
					
					char titleFilename[30];
					strncpy(titleFilename, gameTitle, 20);
					strncat(titleFilename, addressString, 8);
					strncat(titleFilename, ".gb", 3);
					
					
					// Read ROM
					printf("\n--- Dump ROM ---\n");
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
									com_read_cont();
								}
								
								// Print progress
								print_progress_percent(readBytes, (romBanks * 16384) / 64);
							}
							com_read_stop(); // Stop reading ROM (as we will bank switch)
						}
						printf("]");
					}
					
					fclose(romFile);
					printf("\nFinished\n");
				}
			}
			else if (otherOptionSelected == '2') {
				printf("\n--- GBA Flash cart ROM mapper ---\n"\
							"Used for mapping ROMs to 0x00 on GBA flash carts like \"24 in 1\" ones.\n"\
							"There are 2 address data bytes to set and ROM size. Most of the time data bytes are in multiples of 8 (and can be 0 too).\n"\
							"E.g, Address 2 set to 0x30 and Address 3 set to 0x28 gives \"Ice Age\" game.\n"\
							"Type x to exit");
				
				printf("\n\nWould you like to autoscan for game titles? (y/n) \n>");
				char scanOption = read_one_letter();
				
				if (scanOption == 'y') {
					// Scan for games
					uint8_t a2 = 0;
					while (a2 <= 128) {
						
						uint8_t a3 = 0;
						while (a3 < 128) {
							RS232_cputs(cport_nr, "M0"); // Disable CS/RD/WR/CS2-RST from going high after each command
							RS232_drain(cport_nr);
							
							if (gbxcartPcbVersion == PCB_1_1) {	// V1.1 PCB
								RS232_cputs(cport_nr, "LE0x04"); // CS2 low
							}
							else {
								RS232_cputs(cport_nr, "LD0x80"); // CS2 low
							}
							RS232_SendByte(cport_nr, 0);
							RS232_drain(cport_nr);
							
							set_bank(2, a2);
							set_bank(3, a3);
							set_bank(4, a3);
							
							if (gbxcartPcbVersion == PCB_1_1) {	// V1.1 PCB
								RS232_cputs(cport_nr, "HE0x04"); // CS2 high
							}
							else {
								RS232_cputs(cport_nr, "HD0x80"); // CS2 high
							}
							RS232_SendByte(cport_nr, 0);
							RS232_drain(cport_nr);
							
							RS232_cputs(cport_nr, "M1");
							RS232_drain(cport_nr);
							
							gba_read_gametitle();
							if (strlen(gameTitle) >= 5) {
								printf ("Address 2 = 0x%X, Address 3 = 0x%X, Game title: %s\n", a2, a3, gameTitle);
							}
							a3 += 8;
						}
						a2 += 16;
					}
				}
				
				while (1) {
					// Address 2 byte
					printf("\n\nEnter Address 2 byte in Hex: 0x");
					char readInput[10];
					readInput[0] = '0';
					readInput[1] = 'x';
					fgets(&readInput[2], sizeof(readInput)-3, stdin);
					fflush(stdin);
					
					if (readInput[2] == 'x') {
						break;
					}
					
					int address2Byte = (int) strtol(readInput, NULL, 16);
					
					// Address 3 byte
					printf("Enter Address 3 byte in Hex: 0x");
					readInput[0] = '0';
					readInput[1] = 'x';
					fgets(&readInput[2], sizeof(readInput)-3, stdin);
					fflush(stdin);
					
					if (readInput[2] == 'x') {
						break;
					}
					
					int address3Byte = (int) strtol(readInput, NULL, 16);
					
					// Select banks
					RS232_cputs(cport_nr, "M0"); // Disable CS/RD/WR/CS2-RST from going high after each command
					RS232_drain(cport_nr);
					
					if (gbxcartPcbVersion == PCB_1_1) {	// V1.1 PCB
						RS232_cputs(cport_nr, "LE0x04"); // CS2 low
					}
					else {
						RS232_cputs(cport_nr, "LD0x80"); // CS2 low
					}
					RS232_SendByte(cport_nr, 0);
					RS232_drain(cport_nr);
					
					set_bank(2, address2Byte);
					set_bank(3, address3Byte);
					set_bank(4, address3Byte);
					
					if (gbxcartPcbVersion == PCB_1_1) {	// V1.1 PCB
						RS232_cputs(cport_nr, "HE0x04"); // CS2 high
					}
					else {
						RS232_cputs(cport_nr, "HD0x80"); // CS2 high
					}
					RS232_SendByte(cport_nr, 0);
					RS232_drain(cport_nr);
					
					RS232_cputs(cport_nr, "M1");
					RS232_drain(cport_nr);
					
					gba_read_gametitle();
					printf ("Game title: %s\n", gameTitle);
					
					// ROM size
					printf("\nEnter the ROM size in Mbytes to dump (or any key to cancel)\n");
					printf(">");
					
					char selection[5];
					int selectionNumber;
					fgets(selection, sizeof selection, stdin);
					fflush(stdin);
					
					if (selection[0] == 'x') {
						break;
					}
					
					sscanf(selection, "%d", &selectionNumber);
					
					if (selectionNumber >= 4 && selectionNumber <= 32) {
						char titleFilename[30];
						strncpy(titleFilename, "FC ", 4);
						strncat(titleFilename, gameTitle, 20);
						if (cartridgeMode == GB_MODE) {
							strncat(titleFilename, ".gb", 3);
						}
						else {
							strncat(titleFilename, ".gba", 4);
						}
						
						printf("\nDumping ROM to %s\n", titleFilename);
						printf("[             25%%             50%%             75%%            100%%]\n[");
						
						// Create a new file
						FILE *romFile = fopen(titleFilename, "wb");
						if (romFile != NULL) {
							// Set start and end address
							currAddr = 0x00000;
							endAddr = ((1024 * 1024) * selectionNumber);
							set_number(currAddr, SET_START_ADDRESS);
							set_mode(GBA_READ_ROM);
							
							// Read data
							while (currAddr < endAddr) {
								com_read_bytes(romFile, 64);
								currAddr += 64;
								
								// Request 64 bytes more
								if (currAddr < endAddr) {
									com_read_cont();
								}
								
								// Print progress
								print_progress_percent(currAddr, endAddr / 64);
							}
							printf("]\n");
							com_read_stop();
							fclose(romFile);
						}
					}
					
				}
			}
			
			// GB "22 in 1" Bank Reader
			else if (otherOptionSelected == '3') {
				printf("\n--- GB \"22 in 1\" Cart Bank Reader ---\n"\
							"Used for reading the 4 banks of the GB \"22 in 1\" carts.\n");
				
				printf("\nEnter the Bank number (1-4):\n");
				printf(">");
				
				char selection[5];
				int selectionNumber;
				fgets(selection, sizeof selection, stdin);
				fflush(stdin);
				sscanf(selection, "%d", &selectionNumber);
				printf("\n");
				
				// Switch to 5V
				if (gbxcartPcbVersion == PCB_1_3) {
					set_mode(VOLTAGE_5V);
					delay_ms(300);
				}
				
				gb_flash_pin_setup(WE_AS_WR_PIN); // WR pin
				read_gb_header();
				cartridgeType = 8;
				
				// Switch bank
				gb_flash_write_address_byte(0x7000, 0x00);
				gb_flash_write_address_byte(0x7001, 0x00);
				gb_flash_write_address_byte(0x7002, 0x8F + selectionNumber);
				delay_ms(1);
				
				romBanks = 512;
				strncpy(gameTitle, "BANK1", 6);
				
				
				// Dump ROM		
				printf("\n--- Read ROM ---\n");
				
				char titleFilename[30];
				strncpy(titleFilename, gameTitle, 20);
				if (cartridgeMode == GB_MODE) {
					strncat(titleFilename, ".gb", 3);
				}
				else {
					strncat(titleFilename, ".gba", 4);
				}
				printf("Reading ROM to %s\n", titleFilename);
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
							if (bank >= 256) {
								set_bank(0x3000, 1); // High bit
							}
							else {
								set_bank(0x3000, 0); // High bit
							}
						}
						
						if (bank > 1) { currAddr = 0x4000; }
						
						// Set start address and rom reading mode
						set_number(currAddr, SET_START_ADDRESS);
						delay_ms(1);
						set_mode(READ_ROM_RAM);
						delay_ms(1);
						
						// Read data
						while (currAddr < endAddr) {
							com_read_bytes(romFile, 64);
							currAddr += 64;
							readBytes += 64;
							
							// Request 64 bytes more
							if (currAddr < endAddr) {
								com_read_cont();
							}
							
							// Print progress
							print_progress_percent(readBytes, (romBanks * 16384) / 64);
						}
						com_read_stop(); // Stop reading ROM (as we will bank switch)
					}
					printf("]");
				}
				
				fclose(romFile);
				printf("\nFinished. You must now power cycle GBxCart RW to dump the other banks.\n");
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