/*
 GBxCart RW Mini
 PCB version: 1.0
 Firmware version: R12
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 7/11/2016
 Last Modified: 4/12/2018
 
 GBxCart RW Mini allows you to dump your Gameboy/Gameboy Colour games ROM, save the RAM, write to the RAM and 
 write to certain Gameboy flash carts. GBA carts are not supported, please check out the non-Mini version of GBxCart RW.
 
 The ATmega8515 talks to the cartridge and interfaces with the CH340G serial to USB converter with the PC.
 
 
 Set fuse bits: External 16MHz crystal, divide clock by 8 is off, boot loader is on (512 bytes), BOD is on (2.7V)
 avrdude -p atmega8515 -c usbasp -U lfuse:w:0xaf:m -U hfuse:w:0xda:m
 
 Program the TinySafeBoot boot loader (modified ASM code, watchdog reset will act as a hardware reset so you can re-program the ATmega)
 avrdude -p atmega8515 -c usbasp -U flash:w:GBxCart_RW_v1.1_Rx\tsb\tsb_m8515_d0d1_resetwdt.hex
 
 Program GBxCart Mini RW through TinySafeBoot
 tsb com16:57600 fw main.hex
 
 Set TinySafeBoot delay time to 25 (T 25), gives ~0.5s in bootloader
 tsb com16:9600 T 25
 
 
 Programming shortcuts (if added in Programmers Notepad):
 Alt + W - Program fuse
 Alt + B - Burn bootloader
 
 Plug in USB then
 Alt + T - Program
 Alt + N - Program delay settings
 
 
 */

// ATmega8515L Pin Map
//
// VCC (5)			GND (6)			XTAL (7 ,8)		Activity LED PD3 (9)				Switch sense PD2 (8)
// SCK PB7 (3)		MOSI PB5 (1)	MISO PB6 (2)	RST (4)		RXD PD0 (5)			TXD PD1 (7)
// 
// Gameboy / Gameboy Colour
// A0-A7 PB0-7 (40-44, 1-3) 		A8-A15 PA0-7 (37-30)			D0-D7 PC0-7 (18-25)		Audio PE1 (27)
// 
// Gameboy Advance
// AD0-AD7 PB0-7 (40-44, 1-3) 	AD8-AD15 PA0-7 (37-30)		A16-23 / D0-D7 PC0-7 (19-26)
// *WR PD6 (13)						*RD PD5 (14)					*MREQ/CS PD4 (15)			CS2/RST PE2 (16)

#define F_CPU 16000000 // 16 MHz
#define PCB_VERSION 100 // Mini v1.0
#define FIRMWARE_VERSION 12

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include "setup.c" // See defines, variables, constants, functions here


int main(void) {
	setup();
	
	uint32_t address = 0;
	uint8_t resetCommonLines = 1;
	
	while(1) {
		if (resetCommonLines == 1) {
			rd_wr_csmreq_cs2_reset();
		}
		receivedChar = USART_Receive(); // Wait for 1 byte of data
		
		
		// Return the cart mode in use
		if (receivedChar == CART_MODE) {
			USART_Transmit(cartMode);
		}
		
		// Change to GB mode or GBA mode if requested
		else if (receivedChar == GB_CART_MODE) {
			gb_mode();
		}
		
		// Set address
		else if (receivedChar == SET_START_ADDRESS) {
			usart_read_chars(); // Read start address
			address = strtol(receivedBuffer, NULL, 16); // Convert address string in hex to dec
		}
		
		
		// ****** Gameboy / Gameboy Colour ******
		
		// Read 64 bytes of ROM/RAM from address (and increment) until anything but 1 is received
		else if (receivedChar == READ_ROM_RAM) {
			gb_mode();
			receivedChar = '1';
			while (receivedChar == '1') {
				PORTD |= (1<<ACTIVITY_LED);
				for (uint8_t x = 0; x < 64; x++) {
					USART_Transmit(read_8bit_data(address));
					address++;
				}
				
				PORTD &= ~(1<<ACTIVITY_LED);
				receivedChar = USART_Receive();
			}
		}
		
		// Write 64 bytes to RAM on address (and increment)
		else if (receivedChar == WRITE_RAM) {
			gb_mode();
			
			// Read 64 bytes first as CH340G sends them all at once
			usart_read_bytes(64);
			
			PORTD |= (1<<ACTIVITY_LED);
			for (uint8_t x = 0; x < 64; x++) {
				write_8bit_data(address, receivedBuffer[x], MEMORY_WRITE);
				address++;
			}
			
			PORTD &= ~(1<<ACTIVITY_LED);
			USART_Transmit(SEND_ACK); // Send back acknowledgement
		}
		
		// Set bank address and write a byte
		else if (receivedChar == SET_BANK) {
			gb_mode();
			
			usart_read_chars(); // Read start address
			uint16_t bankaddress = strtol(receivedBuffer, NULL, 16); // Convert address string in hex to dec
			
			receivedChar = USART_Receive(); // Wait for bank number
			if (receivedChar == 'B') {
				usart_read_chars(); // Read data
				uint8_t data = atoi(receivedBuffer); // Convert data string to dec
				lastBankAccessed = data; // Store the last bank accessed (used for flash carts that need it)
				
				write_8bit_data(bankaddress, data, BANK_WRITE);
			}
		}
		
		
		// ---------- GB FLASH CARTS ----------
		// Select which pin need to pulse as WE (Audio or WR)
		else if (receivedChar == GB_FLASH_WE_PIN) {
			flashWriteWePin = USART_Receive();
			
			if (flashWriteWePin == WE_AS_AUDIO_PIN) {
				DDRE |= (1<<AUDIO_PIN);
				audioPin_high;
			}
		}
		
		// Some flash carts may require changing the bank back to 1 in order to accept flash chip commands
		else if (receivedChar == GB_FLASH_BANK_1_COMMAND_WRITES) {
			flashBank1CommandWrites = 1;
		}
		
		// Load the program method to use
		else if (receivedChar == GB_FLASH_PROGRAM_METHOD) {
			for (uint8_t x = 0; x < 3; x++) {
				usart_read_chars(); // Address
				flashWriteCycle[x][0] = strtol(receivedBuffer, NULL, 16);
				USART_Transmit(SEND_ACK);
				
				usart_read_chars(); // Data
				flashWriteCycle[x][1] = strtol(receivedBuffer, NULL, 16);
				USART_Transmit(SEND_ACK);
			}
		}
		
		// Write address and one byte to Flash, pulse a pin
		else if (receivedChar == GB_FLASH_WRITE_BYTE) {
			usart_read_chars(); // Read address
			uint16_t flashAddress = strtol(receivedBuffer, NULL, 16);
			
			usart_read_chars(); // Read data byte
			uint8_t flashByte = strtol(receivedBuffer, NULL, 16);
			
			PORTD |= (1<<ACTIVITY_LED);
			gb_flash_write_bus_cycle(flashAddress, flashByte);
			PORTD &= ~(1<<ACTIVITY_LED);
			
			USART_Transmit(SEND_ACK); // Send back acknowledgement
		}
		
		// Write 64 bytes to Flash address one byte write at a time (and increment), pulse a pin
		else if (receivedChar == GB_FLASH_WRITE_64BYTE) {
			usart_read_bytes(64);
			
			PORTD |= (1<<ACTIVITY_LED);
			for (uint8_t x = 0; x < 64; x++) {
				if (flashBank1CommandWrites == 0) {
					gb_flash_write_byte(address, receivedBuffer[x]);
				}
				else { // Some flash carts need to change to bank 1 to issue flash commands
					gb_flash_write_byte_bank1_commands(address, receivedBuffer[x]);
				}
				address++;
			}
			USART_Transmit(SEND_ACK); // Send back acknowledgement
			
			PORTD &= ~(1<<ACTIVITY_LED);
		}
		
		// Buffered programming, write 32 bytes to Flash address
		else if (receivedChar == GB_FLASH_WRITE_BUFFERED_32BYTE) {
			usart_read_bytes(32);
			PORTD |= (1<<ACTIVITY_LED);
			
			// Setup buffered write
			gb_flash_write_bus_cycle(0xAAA, 0xAA);
			gb_flash_write_bus_cycle(0x555, 0x55);
			gb_flash_write_bus_cycle(address, 0x25);
			gb_flash_write_bus_cycle(address, 0x1F); // Length
			_delay_us(1);
			
			// Write data
			for (uint8_t x = 0; x < 32; x++) {
				gb_flash_write_bus_cycle(address, receivedBuffer[x]);
				address++;
			}
			
			// Write buffer to flash
			gb_flash_write_bus_cycle(address-32, 0x29);
			_delay_us(200);
			
			// Verify last byte written
			uint8_t dataVerify = gb_flash_read_byte(address-1);
			uint8_t verifyCount = 0;
			while (dataVerify != receivedBuffer[31]) {
				dataVerify = gb_flash_read_byte(address-1);
				_delay_us(5);
				verifyCount++;
				if (verifyCount >= 200) {
					/*gb_flash_write_bus_cycle(0xAAA, 0xAA);
					gb_flash_write_bus_cycle(address, 0x55);
					gb_flash_write_bus_cycle(0x555, 0xF0);
					
					cs2Pin_low;
					_delay_ms(200);
					cs2Pin_high;
					_delay_ms(200);
					
					gb_flash_write_bus_cycle(0x555, 0xF0);*/
					
					_delay_ms(500);
					break;
				}
			}
			
			USART_Transmit(SEND_ACK); // Send back acknowledgement
			PORTD &= ~(1<<ACTIVITY_LED);
		}
		
		// Buffered programming, write 256 bytes to Flash address
		else if (receivedChar == GB_FLASH_WRITE_256BYTE) {
			usart_read_bytes(256);
			PORTD |= (1<<ACTIVITY_LED);
			
			// Setup buffered write
			gb_flash_write_bus_cycle(0xAAA, 0xA9);
			gb_flash_write_bus_cycle(0x555, 0x56);
			gb_flash_write_bus_cycle(address, 0x26);
			gb_flash_write_bus_cycle(address, 0xFF); // Length
			_delay_us(50);
			
			// Write data
			for (int x = 0; x < 256; x++) {
				gb_flash_write_bus_cycle(address, receivedBuffer[x]);
				address++;
			}
			
			// Write buffer to flash
			gb_flash_write_bus_cycle(address-256, 0x2A);
			
			// Verify last byte written
			uint8_t dataVerify = gb_flash_read_byte(address-1);
			while (dataVerify != receivedBuffer[255]) {
				dataVerify = gb_flash_read_byte(address-1);
				_delay_us(5);
			}
			
			USART_Transmit(SEND_ACK); // Send back acknowledgement
			PORTD &= ~(1<<ACTIVITY_LED);
		}
		
		
		// ---------- General commands ----------
		// Set any pin as input/output
		// Reads the DDR/PORT (e.g. DDRB/PORTB is 'B') and the hex value that represents pins to set to an input (e.g. PB7 is 0x80)
		else if (receivedChar == SET_INPUT || receivedChar == SET_OUTPUT) {
			char portChar = USART_Receive();
			usart_read_chars();
			uint8_t setValue = strtol(receivedBuffer, NULL, 16);
			
			PORTD |= (1<<ACTIVITY_LED);
			if (receivedChar == SET_INPUT) {
				if (portChar == 'A') {
					DDRA &= ~(setValue);
				}
				else if (portChar == 'B') {
					DDRB &= ~(setValue);
				}
				else if (portChar == 'C') {
					DDRC &= ~(setValue);
				}
				else if (portChar == 'D') {
					DDRD &= ~(setValue);
				}
				else if (portChar == 'E') {
					DDRE &= ~(setValue);
				}
			}
			else if (receivedChar == SET_OUTPUT) {
				if (portChar == 'A') {
					DDRA |= (setValue);
				}
				else if (portChar == 'B') {
					DDRB |= (setValue);
				}
				else if (portChar == 'C') {
					DDRC |= (setValue);
				}
				else if (portChar == 'D') {
					DDRD |= (setValue);
				}
				else if (portChar == 'E') {
					DDRE |= (setValue);
				}
			}
			PORTD &= ~(1<<ACTIVITY_LED);
		}
		
		// Set pin output as low
		else if (receivedChar == SET_OUTPUT_LOW) {
			char portChar = USART_Receive();			
			usart_read_chars();
			uint8_t setValue = strtol(receivedBuffer, NULL, 16);
			
			PORTD |= (1<<ACTIVITY_LED);
			if (portChar == 'A') {
				PORTA &= ~(setValue);
			}
			else if (portChar == 'B') {
				PORTB &= ~(setValue);
			}
			else if (portChar == 'C') {
				PORTC &= ~(setValue);
			}
			else if (portChar == 'D') {
				PORTD &= ~(setValue);
			}
			else if (portChar == 'E') {
				PORTE &= ~(setValue);
			}
			PORTD &= ~(1<<ACTIVITY_LED);
		}
		
		// Set pin output as high
		else if (receivedChar == SET_OUTPUT_HIGH) {
			char portChar = USART_Receive();			
			usart_read_chars();
			uint8_t setValue = strtol(receivedBuffer, NULL, 16);
			
			PORTD |= (1<<ACTIVITY_LED);
			if (portChar == 'A') {
				PORTA |= (setValue);
			}
			else if (portChar == 'B') {
				PORTB |= (setValue);
			}
			else if (portChar == 'C') {
				PORTC |= (setValue);
			}
			else if (portChar == 'D') {
				PORTD |= (setValue);
			}
			else if (portChar == 'E') {
				PORTE |= (setValue);
			}
			PORTD &= ~(1<<ACTIVITY_LED);
		}
		
		// Read all pins of a PORT and return the value
		else if (receivedChar == READ_INPUT) {
			char portChar = USART_Receive();			
			
			PORTD |= (1<<ACTIVITY_LED);
			if (portChar == 'A') {
				USART_Transmit(PINA);
			}
			else if (portChar == 'B') {
				USART_Transmit(PINB);
			}
			else if (portChar == 'C') {
				USART_Transmit(PINC);
			}
			else if (portChar == 'D') {
				USART_Transmit(PIND);
			}
			else if (portChar == 'E') {
				USART_Transmit(PINE);
			}
			PORTD &= ~(1<<ACTIVITY_LED);
		}
		
		// Set the reset common lines variable on or off, useful if you are controlling all the pins directly
		else if (receivedChar == RESET_COMMON_LINES) {
			char commonChar = USART_Receive();
			if (commonChar == '1') {
				resetCommonLines = 1;
			}
			else if (commonChar == '0') {
				resetCommonLines = 0;
			}
		}
		
		// Send back the PCB version number
		else if (receivedChar == READ_PCB_VERSION) {
			USART_Transmit(PCB_VERSION);
		}
		
		// Send back the firmware version number
		else if (receivedChar == READ_FIRMWARE_VERSION) {
			USART_Transmit(FIRMWARE_VERSION);
		}
		
		// Reset the AVR if it matches the number
		else if (receivedChar == RESET_AVR) {
			usart_read_chars();
			uint32_t resetValue = strtol(receivedBuffer, NULL, 16);
			if (resetValue == RESET_VALUE) {
				// Clear watchdog flag
				MCUCSR &= ~(1<<WDRF);
				
				// Start timed sequence
				WDTCR = (1<<WDCE) | (1<<WDE);
				
				// Reset in 250 ms
				WDTCR = (1<<WDP2) | (1<<WDE);
				
				// Wait for reset
				_delay_loop_2(65535);
			}
		}
	}
}
