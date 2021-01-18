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
	
	printf("GBxCart RW - GB Cart Flash ID Check by insideGadgets\n");
	printf("####################################################\n");
	
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
	
	// PCB v1.3 - Set 5V
	if (gbxcartPcbVersion == PCB_1_3 || gbxcartPcbVersion == GBXMAS) {
		set_mode(VOLTAGE_5V);
		delay_ms(500);
	}
	
	
	printf("          Read ROM: ");
	set_number(0, SET_START_ADDRESS);
	set_mode(READ_ROM_RAM);
	com_read_bytes(READ_BUFFER, 64);
	com_read_stop(); // End read
	
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
	
	
	printf("Flash ID (AA, 98):  ");
	gb_flash_write_address_byte(0xAA, 0x98);
	delay_ms(50);
	
	set_number(0, SET_START_ADDRESS);
	delay_ms(5);
	set_mode(READ_ROM_RAM);
	delay_ms(5);
	com_read_bytes(READ_BUFFER, 64);
	com_read_stop(); // End read
	
	for (uint8_t r = 0; r < 8; r++) {
		printf("0x%X, ", readBuffer[r]);
	}
	printf("\n");
	gb_flash_write_address_byte(0x000, 0xF0);
	
	
	printf("Flash ID (5555, AA): ");
	set_bank(0x2100, 1); // Set bank 1 before issuing flash commands
	delay_ms(5);
	gb_flash_write_address_byte(0x5555, 0xAA);
	gb_flash_write_address_byte(0x2AAA, 0x55);
	gb_flash_write_address_byte(0x5555, 0x90);
	delay_ms(50);
	set_bank(0x2100, 0); 
	
	set_number(0, SET_START_ADDRESS);
	set_mode(READ_ROM_RAM);
	com_read_bytes(READ_BUFFER, 64);
	com_read_stop(); // End read
	
	for (uint8_t r = 0; r < 8; r++) {
		printf("0x%X, ", readBuffer[r]);
	}
	printf("\n");
	gb_flash_write_address_byte(0x000, 0xF0);
	
	
	printf("Flash ID (5555, A9): ");
	set_bank(0x2100, 1); // Set bank 1 before issuing flash commands
	delay_ms(5);
	gb_flash_write_address_byte(0x5555, 0xA9);
	gb_flash_write_address_byte(0x2AAA, 0x56);
	gb_flash_write_address_byte(0x5555, 0x90);
	delay_ms(50);
	set_bank(0x2100, 0); 
	
	set_number(0, SET_START_ADDRESS);
	set_mode(READ_ROM_RAM);
	com_read_bytes(READ_BUFFER, 64);
	com_read_stop(); // End read
	
	for (uint8_t r = 0; r < 8; r++) {
		printf("0x%X, ", readBuffer[r]);
	}
	printf("\n");
	gb_flash_write_address_byte(0x000, 0xF0);
	
	
	printf("Flash ID (7AAA, A9): ");
	gb_flash_write_address_byte(0x7AAA, 0xA9);
	gb_flash_write_address_byte(0x7555, 0x56);
	gb_flash_write_address_byte(0x7AAA, 0x90);
	delay_ms(50);
	
	set_number(0x0000, SET_START_ADDRESS);
	delay_ms(5);
	set_mode(READ_ROM_RAM);
	delay_ms(5);
	com_read_bytes(READ_BUFFER, 64);
	com_read_stop(); // End read
	
	for (uint8_t r = 0; r < 8; r++) {
		printf("0x%X, ", readBuffer[r]);
	}
	printf("\n");
	gb_flash_write_address_byte(0x7000, 0xF0);
	
	
	
	
	gb_flash_pin_setup(WE_AS_AUDIO_PIN);
	
	printf("Flash ID Audio Pin (555, AA): ");
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
	
	
	printf("Flash ID Audio Pin (555, A9): ");
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
	
	
	printf("Flash ID Audio Pin (AAA, AA): ");
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
	
	
	
	printf("Flash ID Audio Pin (AAA, A9): ");
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
	
	
	printf("Flash ID Audio Pin (5555, AA): ");
	set_bank(0x2100, 1); // Set bank 1 before issuing flash commands
	gb_flash_write_address_byte(0x5555, 0xAA);
	gb_flash_write_address_byte(0x2AAA, 0x55);
	gb_flash_write_address_byte(0x5555, 0x90);
	delay_ms(50);
	set_bank(0x2100, 0);
	
	set_number(0, SET_START_ADDRESS);
	set_mode(READ_ROM_RAM);
	com_read_bytes(READ_BUFFER, 64);
	com_read_stop(); // End read
	
	for (uint8_t r = 0; r < 8; r++) {
		printf("0x%X, ", readBuffer[r]);
	}
	printf("\n");
	gb_flash_write_address_byte(0x000, 0xF0);
	
	
	printf("Flash ID Audio Pin (5555, A9): ");
	set_bank(0x2100, 1); // Set bank 1 before issuing flash commands
	gb_flash_write_address_byte(0x5555, 0xA9);
	gb_flash_write_address_byte(0x2AAA, 0x56);
	gb_flash_write_address_byte(0x5555, 0x90);
	delay_ms(50);
	set_bank(0x2100, 0);
	
	set_number(0, SET_START_ADDRESS);
	set_mode(READ_ROM_RAM);
	com_read_bytes(READ_BUFFER, 64);
	com_read_stop(); // End read
	
	for (uint8_t r = 0; r < 8; r++) {
		printf("0x%X, ", readBuffer[r]);
	}
	printf("\n");
	gb_flash_write_address_byte(0x000, 0xF0);
	
	
	
	printf("Finished");
	read_one_letter();
	return 0;
}