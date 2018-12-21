/*
 GBxCart RW Mini
 PCB version: 1.0
 Firmware version: R12
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 7/11/2016
 Last Modified: 4/12/2018
 
 */
 
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#define LOW 0
#define HIGH 1
#define false 0
#define true 1

#define ACTIVITY_LED PD3

#define WR_PIN PD6
#define RD_PIN PD5
#define CS_MREQ_PIN PD4
#define CS2_PIN PE2
#define AUDIO_PIN PE1

#define wrPin_high	PORTD |= (1<<WR_PIN);
#define wrPin_low		PORTD &= ~(1<<WR_PIN);
#define rdPin_high	PORTD |= (1<<RD_PIN);
#define rdPin_low		PORTD &= ~(1<<RD_PIN);
#define cs_mreqPin_high		PORTD |= (1<<CS_MREQ_PIN);
#define cs_mreqPin_low		PORTD &= ~(1<<CS_MREQ_PIN);
#define cs2Pin_high		PORTE |= (1<<CS2_PIN);
#define cs2Pin_low		PORTE &= ~(1<<CS2_PIN);
#define audioPin_high	PORTE |= (1<<AUDIO_PIN);
#define audioPin_low		PORTE &= ~(1<<AUDIO_PIN);

#define GB_MODE 1

// GB/GBC
#define PORT_ADDR7_0 PORTB
#define PORT_ADDR15_8 PORTA
#define PORT_DATA7_0 PORTC

#define DDR_ADDR7_0 DDRB
#define DDR_ADDR15_8 DDRA
#define DDR_DATA7_0 DDRC

#define PIN_ADDR7_0 PINB
#define PIN_ADDR15_8 PINA
#define PIN_DATA7_0 PINC

#define BANK_WRITE 0
#define MEMORY_WRITE 1


// GB/GBC commands
#define SET_START_ADDRESS 'A'
#define READ_ROM_RAM 'R'
#define WRITE_RAM 'W'
#define SET_BANK 'B'
#define GB_CART_MODE 'G'

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
#define GB_FLASH_WRITE_BUFFERED_32BYTE 'Y'
#define GB_FLASH_WRITE_64BYTE 'T'
#define GB_FLASH_WRITE_256BYTE 'X'

#define GB_FLASH_BANK_1_COMMAND_WRITES 'N'

#define D0D1_NOT_SWAPPED 0
#define D0D1_SWAPPED 1

// General commands
#define SEND_ACK '1'
#define CART_MODE 'C'
#define SET_INPUT 'I'
#define SET_OUTPUT 'O'
#define SET_OUTPUT_LOW 'L'
#define SET_OUTPUT_HIGH 'H'
#define READ_INPUT 'D'
#define RESET_COMMON_LINES 'M'
#define READ_FIRMWARE_VERSION 'V'
#define READ_PCB_VERSION 'h'

#define RESET_AVR '*'
#define RESET_VALUE 0x7E5E1


char receivedBuffer[256];
char receivedChar;

char flashWriteWePin;
uint16_t flashWriteCycle[3][2];
uint8_t flashBank1CommandWrites = 0;
uint8_t lastBankAccessed = 0;

uint8_t cartMode = GB_MODE;

// Receive USART data
uint8_t USART_Receive(void) {
	while ( !(UCSRA & (1<<RXC)) ); // Wait for data to be received
	return UDR; // Get and return received data from buffer
}

// Transmit USART data
void USART_Transmit(unsigned char data) {
	while ( !( UCSRA & (1<<UDRE)) ); // Wait for empty transmit buffer
	UDR = data;
}

// Read 1-256 bytes from the USART 
void usart_read_bytes(int count) {
	for (int x = 0; x < count; x++) {
		receivedBuffer[x] = USART_Receive();
	}
}

// Read the USART until a 0 (string terminator byte) is received
void usart_read_chars(void) {
	int x = 0;
	while (1) {
		receivedBuffer[x] = USART_Receive();
		if (receivedBuffer[x] == 0) {
			break;
		}
		x++;
	}
}

// Turn RD, WR, CS/MREQ and CS2 to high so they are deselected (reset state)
void rd_wr_csmreq_cs2_reset(void) {
	cs2Pin_high; // CS2 off
	cs_mreqPin_high; // CS/MREQ off
	rdPin_high; // RD off
	wrPin_high; // WR off
}



// ****** Gameboy / Gameboy Colour functions ******

// Set Gameboy mode
void gb_mode(void) {
	// Set inputs
	PORT_DATA7_0 = 0;
	DDR_DATA7_0 = 0;
	
	// Set outputs
	PORT_ADDR7_0 = 0;
	PORT_ADDR15_8 = 0;
	DDR_ADDR7_0 = 0xFF;
	DDR_ADDR15_8 = 0xFF;
}

// Set the 16 bit address on A15-0
void set_16bit_address(uint16_t address) {
	PORT_ADDR15_8 = (address >> 8);
	PORT_ADDR7_0 = (address & 0xFF);
}

// Set the address and read a byte from the 8 bit data line
uint8_t read_8bit_data(uint16_t address) {
	set_16bit_address(address);
	
	cs_mreqPin_low;
	rdPin_low;
	
	asm volatile("nop"); // Delay a little (At 8MHz - minimum needed is 1 nops, 2 nops for GB camera)
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	uint8_t data = PIN_DATA7_0; // Read data
	
	rdPin_high;
	cs_mreqPin_high;
	
	return data;
}

// Set the address and write a byte to the 8 bit data line and pulse cs/mREQ if writing to RAM
void write_8bit_data(uint16_t address, uint8_t data, uint8_t type) {
	set_16bit_address(address);
	
	DDR_DATA7_0 = 0xFF; // Set data pins as outputs
	PORT_DATA7_0 = data; // Set data
	
	// Pulse WR and mREQ if the type matches
	wrPin_low;
	if (type == MEMORY_WRITE) {
		cs_mreqPin_low;
	}
	
	asm volatile("nop");
	asm volatile("nop");
	
	if (type == MEMORY_WRITE) {
		cs_mreqPin_high;
	}
	wrPin_high;
	
	// Clear data outputs and set data pins as inputs
	PORT_DATA7_0 = 0;
	DDR_DATA7_0 = 0;
}



// ---------- GB FLASH CARTS ----------

// Read a byte from the flash (No CS pin pulse)
uint8_t gb_flash_read_byte(uint16_t address) {
	PORT_DATA7_0 = 0;
	DDR_DATA7_0 = 0;
	
	set_16bit_address(address);
	
	rdPin_low;
	asm volatile("nop"); // Delay a little
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	uint8_t data = PIN_DATA7_0; // Read data
	rdPin_high;
	
	return data;
}

// Set the address and data for the write byte cycle to the flash
void gb_flash_write_bus_cycle(uint16_t address, uint8_t data) {
	DDR_DATA7_0 = 0xFF; // Set data pins as outputs
	set_16bit_address(address);
	PORT_DATA7_0 = data;
	
	if (flashWriteWePin == WE_AS_AUDIO_PIN) { // Audio pin
		audioPin_low; // WE low
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		audioPin_high; // WE high
	}
	else { // WR pin
		wrPin_low; // WE low
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		wrPin_high; // WE high
	}
}

// Write a single byte to the Flash address. Takes 10-50us to program each byte.
void gb_flash_write_byte(uint16_t address, uint8_t data) {
	gb_flash_write_bus_cycle(flashWriteCycle[0][0], flashWriteCycle[0][1]);
	gb_flash_write_bus_cycle(flashWriteCycle[1][0], flashWriteCycle[1][1]);
	gb_flash_write_bus_cycle(flashWriteCycle[2][0], flashWriteCycle[2][1]);
	gb_flash_write_bus_cycle(address, data);
	_delay_us(10); // Wait byte program time
	
	// Set data pins inputs
	PORT_DATA7_0 = 0;
	DDR_DATA7_0 = 0;
	
	// Verify data
	uint8_t dataVerify = gb_flash_read_byte(address);
	while (data != dataVerify) {
		dataVerify = gb_flash_read_byte(address);
		_delay_us(5);
	}
}

// Write a single byte to the Flash address. Takes 10-50us to program each byte. 
// Switch to bank 1 to issue flash commands, then switch back to the bank we were at before
void gb_flash_write_byte_bank1_commands(uint16_t address, uint8_t data) {
	// Set bank 1
	DDR_DATA7_0 = 0xFF;
	set_16bit_address(0x2100);
	PORT_DATA7_0 = 1;
	wrPin_low; // Pulse WR
	asm volatile("nop");
	asm volatile("nop");
	wrPin_high;
	
	gb_flash_write_bus_cycle(flashWriteCycle[0][0], flashWriteCycle[0][1]);
	gb_flash_write_bus_cycle(flashWriteCycle[1][0], flashWriteCycle[1][1]);
	gb_flash_write_bus_cycle(flashWriteCycle[2][0], flashWriteCycle[2][1]);
	
	
	// Set bank back
	DDR_DATA7_0 = 0xFF;
	set_16bit_address(0x2100);
	PORT_DATA7_0 = lastBankAccessed;
	wrPin_low; // Pulse WR
	asm volatile("nop");
	asm volatile("nop");
	wrPin_high;
	
	gb_flash_write_bus_cycle(address, data);
	_delay_us(10); // Wait byte program time
	
	// Set data pins inputs
	PORT_DATA7_0 = 0;
	DDR_DATA7_0 = 0;
	
	// Verify data
	uint8_t dataVerify = gb_flash_read_byte(address);
	while (data != dataVerify) {
		dataVerify = gb_flash_read_byte(address);
		_delay_us(5);
	}
}


// Setup
void setup(void) {
	// Turn off watchdog
	MCUCSR &= ~(1<<WDRF);
	WDTCR = (1<<WDCE) | (1<<WDE);
	WDTCR = 0;
	
	// Reset common lines
	rd_wr_csmreq_cs2_reset();
	
	// Set outputs
	DDRD |= (1<<ACTIVITY_LED) | (1<<WR_PIN) | (1<<RD_PIN) | (1<<CS_MREQ_PIN);
	DDRE |= (1<<CS2_PIN);
	
	// Pull ups
	PORTE |= (1<<PE0);
	PORTD |= (1<<PD7) | (1<<PD2);
	
	// Set all pins as inputs
	PORT_DATA7_0 = 0;
	DDR_DATA7_0 = 0;
	PORT_ADDR7_0 = 0;
	DDR_ADDR7_0 = 0;
	PORT_ADDR15_8 = 0;
	DDR_ADDR15_8 = 0;
	
	// Set Reset low for 50ms to reset the cart (also for CPLD carts to initialise properly)
	cs2Pin_low;
	_delay_ms(50);
	cs2Pin_high;
	
	// Light LED
	PORTD |= (1<<ACTIVITY_LED);
	_delay_ms(500);
	PORTD &= ~(1<<ACTIVITY_LED);
	
	// Setup USART
	UBRRL = 0; // 1Mbps Baud rate
	sbi(UCSRB, TXEN); // Transmitter enable
	sbi(UCSRB, RXEN); // Receiver enable
	
	// Turn on interrupts
	sei();
}
