/*
 GBxCart RW - GB Camera Save to BMP
 Version: 1.8
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 8/07/2017
 Last Modified: 4/05/2019
 
 GBxCart RW allows you to dump your Gameboy/Gameboy Colour/Gameboy Advance games ROM, save the RAM and write to the RAM.
 
 This is a special build which extracts your GB camera images and converts them to BMP files. 
 We create a folder with the date/time and save each image to a BMP file in that folder.
  
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "setup.h" // See defines, variables, constants, functions here

int main(int argc, char **argv) {
	
	printf("GBxCart RW - GB Camera Saver v1.8 by insideGadgets\n");
	printf("##################################################\n");
	printf("Extract all images (including deleted ones)\n");
	
	read_config();
	read_config_folder();
	
	// Open COM port
	if (com_test_port() == 0) {
		printf("Device not connected and couldn't be auto detected\n");
		read_one_letter();
		return 1;
	}
	
	// Break out of any existing functions on ATmega
	set_mode('0');
	
	// Get cartridge mode - Gameboy or Gameboy Advance
	cartridgeMode = request_value(CART_MODE);
	
	// Get firmware version
	gbxcartFirmwareVersion = request_value(READ_BUILD_VERSION);
	
	// Get PCB version
	gbxcartPcbVersion = request_value(READ_PCB_VERSION);
	
	// Check if in GB mode
	if (gbxcartPcbVersion == PCB_1_1 && cartridgeMode == GBA_MODE) {
		printf("\nYou must be in GB Mode, 5V\n");
		read_one_letter();
		return 0;
	}
	
	// Set 5V
	if (gbxcartPcbVersion == PCB_1_3) {
		set_mode(VOLTAGE_5V);
		delay_ms(500);
	}
	
	// Check if the cart inserted is the GB camera
	read_gb_header();
	if (cartridgeType != 252) {
		printf("\nCartridge inserted isn't a GB camera\n");
		read_one_letter();
		return 0;
	}
	
	// Check if the logo check passes
	if (logo_check() == false) {
		printf("\nLogo check failed, can't continue. Please unplug GBxCart RW, re-insert the cartridge and reconnect GBxCart RW.\n");
		read_one_letter();
		return 0;
	}
	
	// Get the date/time to be used for the folder
	time_t rawtime;
	struct tm* timeinfo;
	char timebuffer[25];
	
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	strftime (timebuffer, 80, "%Y-%m-%d_%H-%M-%S", timeinfo);
	
	// Create folder
	char mkfolder[250];
	strncpy(mkfolder, "mkdir ", 7);
	strncat(mkfolder, currentFolder, 219);
	strncat(mkfolder, timebuffer, 25);
	system(mkfolder);
	
	
	// Save memory to file
	char savFilename[250];
	strncpy(savFilename, currentFolder, 219);
	strncat(savFilename, timebuffer, 25);
	strncat(savFilename, "/GBCAMERA.sav", 14);
	
	FILE *ramFile = fopen(savFilename, "wb");
	printf("\nSaving RAM to %s\n", savFilename);
	printf("[             25%%             50%%             75%%            100%%]\n[");
	
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
					RS232_drain(cport_nr);
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
			RS232_drain(cport_nr);
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
				com_read_bytes(ramFile, 64);
				ramAddress += 64;
				readBytes += 64;
				
				// Request 64 bytes more
				if (ramAddress < ramEndAddress) {
					com_read_cont();
					RS232_drain(cport_nr);
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
			RS232_drain(cport_nr);
		}
	}
	printf("]");
	
	set_bank(0x0000, 0x00); // Disable RAM
	
	fclose(ramFile);
	
	
	// Convert .sav file to BMP image files
	printf("\n\nExtracting images from .sav file and converting to BMP images");
	
	// Locate which photos aren't marked as deleted
	//uint16_t activePhotosLocation = 0x11B2;
	//uint8_t activePhotosReadCount = 0x1E;
	//uint8_t activePhotos[30];
	
	//FILE *saveFile = fopen(savFilename, "rb");
	//fseek(saveFile, activePhotosLocation, SEEK_SET);
	//fread(activePhotos, 1, activePhotosReadCount, saveFile);
	//fclose(saveFile);
	
	// Only save photos which are actively showing (i.e not marked as 0xFF)
	uint8_t saveName = 1;
	for (uint8_t x = 0; x < 30; x++) {
		//if (activePhotos[x] != 0xFF) {
			uint32_t startLocation = (x * 0x1000) + 0x2000;
			uint16_t startReadCount = 3584;
			
			// Read save file
			FILE *saveFile = fopen(savFilename, "rb");
			fseek(saveFile, startLocation, SEEK_SET);
			fread(savBuffer, 1, startReadCount, saveFile);
			fclose(saveFile);
			
			// Write to BMP
			char bmpName[60];
			sprintf(bmpName, "%s%s/%i.bmp", currentFolder, timebuffer, saveName);
			save_to_bmp(bmpName);
			saveName++;
		//}
	}
	
	printf("\nFinished\n");
	
	
	return 0;
}