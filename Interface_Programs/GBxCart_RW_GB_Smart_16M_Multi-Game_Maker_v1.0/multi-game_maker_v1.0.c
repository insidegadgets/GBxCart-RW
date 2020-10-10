/*
 Multi-Game Menu Maker for GB Smart 16M
 Version: 1.0
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 18/05/2019
 Last Modified: 24/05/2019
 License: GPL
  
 Drag and drop your ROM files one at a time to this executable and it will create and append your roms to an output.gb file 
 for you to flash to the GB Smart 16M Flash cart. Delete the output.gb file if you wish to start again.
  
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <libgen.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

// Read one letter from stdin
char read_one_letter (void) {
	char c = getchar();
	while (getchar() != '\n' && getchar() != EOF);
	return c;
}

int main(int argc, char **argv) {
	
	printf("insideGadgets - Multi-Game Maker v1.0 for GB Smart 16M\n");
	printf("######################################################\n");
	printf("This only works with the GB Smart 16M Flash Cart\n\n");
	
	// Check if output.gb file exists, if not, create it
	FILE *outputFileCheck = fopen("output.gb", "r");
	if (outputFileCheck == NULL) {
		// Copy the base rom file to the output file
		system("copy GB16M.gb output.gb > /nul");
	}
	fclose(outputFileCheck);
	
	
	if (argc >= 2) {
		// Read the file size of the existing output file
		FILE *outputFile = fopen("output.gb", "rb");
		fseek (outputFile, 0, SEEK_END);
		long outputFileSize = ftell(outputFile);
		fclose(outputFile);
		
		// Read the file size of the rom file
		FILE *romReadFile = fopen(argv[1], "rb");
		fseek (romReadFile, 0, SEEK_END);
		long fileSize = ftell (romReadFile);
		fclose(romReadFile);
		
		// Check if we need to align the file
		uint32_t alignmentAmount = 0;
		if (outputFileSize % fileSize != 0) {
			// Keep adding 32KB until we are aligned
			while (outputFileSize % fileSize != 0) {
				alignmentAmount += 32768;
				outputFileSize += 32768;
			}
		}
		
		// Align the file
		if (alignmentAmount >= 1) {
			FILE *romWriteFile = fopen("output.gb", "r+b");
			fseek (romWriteFile, 0, SEEK_END);
			
			// Add 32KB of 0xFF to output.gb
			uint8_t buffer[1] = {0xFF};
			
			for (uint32_t x = 0; x < alignmentAmount; x++) {
				fwrite(buffer, 1, 1, romWriteFile);
			}
			fclose(romReadFile);
		}
		
		// Append the rom file
		char appendCommand[255];
		strncpy(appendCommand, "copy /b output.gb+\"", 21);
		strncat(appendCommand, argv[1], 200);
		strncat(appendCommand, "\" output.gb", 12);
		strncat(appendCommand, " > /nul", 8);
		system(appendCommand);
	}
	
	return 0;
}