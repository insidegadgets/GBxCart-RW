# Command-line client
ifeq ($(OS),Windows_NT)
	EXE_EXT = .exe
else
	EXE_EXT =
endif
CMDLINE = npc_backup_full_rom_and_hidden_sector_v1.0$(EXE_EXT)
CMDLINE2 = npc_backup_multi-cart_save_games_v1.0$(EXE_EXT)
CMDLINE3 = npc_backup_multi-cart_roms_v1.0$(EXE_EXT)
CMDLINE4 = npc_flasher_v1.0$(EXE_EXT)
CMDLINE5 = npc_restore_multi-cart_save_games_v1.0$(EXE_EXT)

# By default, build the firmware and command-line client
all: $(CMDLINE) $(CMDLINE2) $(CMDLINE3) $(CMDLINE4) $(CMDLINE5)

# One-liner to compile the command-line client
$(CMDLINE): npc_backup_full_rom_and_hidden_sector_v1.0.c setup.c rs232/rs232.c
	gcc -O -std=c99 -Wall $^ -o $@

$(CMDLINE2): npc_backup_multi-cart_save_games_v1.0.c setup.c rs232/rs232.c
	gcc -O -std=c99 -Wall $^ -o $@
	
$(CMDLINE3): npc_backup_multi-cart_roms_v1.0.c setup.c rs232/rs232.c
	gcc -O -std=c99 -Wall $^ -o $@
	
$(CMDLINE4): npc_flasher_v1.0.c setup.c rs232/rs232.c
	gcc -O -std=c99 -Wall $^ -o $@
	
$(CMDLINE5): npc_restore_multi-cart_save_games_v1.0.c setup.c rs232/rs232.c
	gcc -O -std=c99 -Wall $^ -o $@
	
# Housekeeping if you want it
clean:
	$(RM) $(CMDLINE) $(CMDLINE2) $(CMDLINE3) $(CMDLINE4) $(CMDLINE5)
