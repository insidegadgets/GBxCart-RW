# Command-line client
ifeq ($(OS),Windows_NT)
	EXE_EXT = .exe
else
	EXE_EXT =
endif
CMDLINE = igpc_backup_full_rom_v1.2$(EXE_EXT)
CMDLINE2 = igpc_backup_multi-cart_save_games_v1.2$(EXE_EXT)
CMDLINE3 = igpc_backup_multi-cart_roms_v1.2$(EXE_EXT)
CMDLINE4 = igpc_flasher_v1.2$(EXE_EXT)
CMDLINE5 = igpc_restore_multi-cart_save_games_v1.2$(EXE_EXT)
CMDLINE6 = igpc_erase_multi-cart_save_games_v1.2$(EXE_EXT)

# By default, build the firmware and command-line client
all: $(CMDLINE) $(CMDLINE2) $(CMDLINE3) $(CMDLINE4) $(CMDLINE5) $(CMDLINE6)

# One-liner to compile the command-line client
$(CMDLINE): igpc_backup_full_rom_v1.2.c setup.c rs232/rs232.c
	gcc -O -std=c99 -Wall $^ -o $@

$(CMDLINE2): igpc_backup_multi-cart_save_games_v1.2.c setup.c rs232/rs232.c
	gcc -O -std=c99 -Wall $^ -o $@
	
$(CMDLINE3): igpc_backup_multi-cart_roms_v1.2.c setup.c rs232/rs232.c
	gcc -O -std=c99 -Wall $^ -o $@
	
$(CMDLINE4): igpc_flasher_v1.2.c setup.c rs232/rs232.c
	gcc -O -std=c99 -Wall $^ -o $@
	
$(CMDLINE5): igpc_restore_multi-cart_save_games_v1.2.c setup.c rs232/rs232.c
	gcc -O -std=c99 -Wall $^ -o $@
	
$(CMDLINE6): igpc_erase_multi-cart_save_games_v1.2.c setup.c rs232/rs232.c
	gcc -O -std=c99 -Wall $^ -o $@
	
# Housekeeping if you want it
clean:
	$(RM) $(CMDLINE) $(CMDLINE2) $(CMDLINE3) $(CMDLINE4) $(CMDLINE5) $(CMDLINE6)
