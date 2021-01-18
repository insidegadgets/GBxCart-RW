/*
 GBxCart RW - Flash ID Check
 Version: 1.0
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 26/08/2017
 Last Modified: 15/03/2020
 
 This program allows you to write ROM to Flash Carts that are supported.
 
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
	
	printf("GBxCart RW - GBA Cart Flash ID Check by insideGadgets\n");
	printf("#####################################################\n");
	
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
	
	// Get cartridge mode - Gameboy or Gameboy Advance
	cartridgeMode = request_value(CART_MODE);
	
	// Get firmware version
	gbxcartFirmwareVersion = request_value(READ_FIRMWARE_VERSION);
	
	// Get PCB version
	gbxcartPcbVersion = request_value(READ_PCB_VERSION);
	
	if (gbxcartPcbVersion == PCB_1_0) {
		printf("\nPCB v1.0 is not supported for this function.");
		read_one_letter();
		return 1;
	}
	
	if (gbxcartFirmwareVersion <= 8) {
		printf("Firmware R9 or higher is required for this functionality.\n");
		read_one_letter();
		return 1;
	}
	printf("\n");
	
	// PCB v1.3 - Set 3.3V
	if (gbxcartPcbVersion == PCB_1_3 || gbxcartPcbVersion == GBXMAS) {
		set_mode(VOLTAGE_3_3V);
		delay_ms(500);
	}
	
	
	
	printf("          Read ROM: ");
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
	
	
	
	printf("Flash ID (555, A9): ");
	gba_flash_write_address_byte(0x555, 0xA9);
	gba_flash_write_address_byte(0x2AA, 0x56);
	gba_flash_write_address_byte(0x555, 0x90);
	
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
	
	
	
	printf("Flash ID (555, AA): ");
	gba_flash_write_address_byte(0x555, 0xAA);
	gba_flash_write_address_byte(0x2AA, 0x55);
	gba_flash_write_address_byte(0x555, 0x90);
	
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
	
	
	
	
	printf("Finished");
	read_one_letter();
	return 0;
}