# Command-line client
ifeq ($(OS),Windows_NT)
	EXE_EXT = .exe
else
	EXE_EXT =
endif
CMDLINE = gbxcart_rw_flasher_v1.34$(EXE_EXT)

# Timestamp fix for Linux
ifeq ($(OS),Windows_NT)
	TIMESTAMP_FIX = 
else
	UNAME_S := $(shell uname -s)
   ifeq ($(UNAME_S),Linux)
		TIMESTAMP_FIX = -D_POSIX_C_SOURCE=199309L
   endif
endif

# By default, build the firmware and command-line client
all: $(CMDLINE)

# One-liner to compile the command-line client
$(CMDLINE): gbxcart_rw_flasher_v1.34.c setup.c rs232/rs232.c
	gcc -O -std=c99 -Wall $(TIMESTAMP_FIX) $^ -o $@
	
# Housekeeping if you want it
clean:
	$(RM) $(CMDLINE)
