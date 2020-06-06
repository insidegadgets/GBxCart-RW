# Command-line client
ifeq ($(OS),Windows_NT)
	EXE_EXT = .exe
else
	EXE_EXT =
endif
CMDLINE = gbxcart_rw_flasher_v1.30$(EXE_EXT)

# By default, build the firmware and command-line client
all: $(CMDLINE)

# One-liner to compile the command-line client
$(CMDLINE): gbxcart_rw_flasher_v1.30.c setup.c rs232/rs232.c
	gcc -O -std=c99 -Wall -D_POSIX_C_SOURCE=199309L $^ -o $@
	
# Housekeeping if you want it
clean:
	$(RM) $(CMDLINE)
