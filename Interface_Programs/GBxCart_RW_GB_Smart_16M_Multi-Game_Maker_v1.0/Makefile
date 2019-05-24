# Command-line client
ifeq ($(OS),Windows_NT)
	EXE_EXT = .exe
else
	EXE_EXT =
endif
CMDLINE = multi-game_maker_v1.0$(EXE_EXT)

# By default, build the firmware and command-line client
all: $(CMDLINE)

# One-liner to compile the command-line client
$(CMDLINE): multi-game_maker_v1.0.c
	gcc -O -std=c99 -Wall $^ -o $@
	
# Housekeeping if you want it
clean:
	$(RM) $(CMDLINE)
