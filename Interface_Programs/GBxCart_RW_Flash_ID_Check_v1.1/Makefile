# Command-line client
ifeq ($(OS),Windows_NT)
	EXE_EXT = .exe
else
	EXE_EXT =
endif
CMDLINE = gbxcart_rw_gb_cart_flash_id_check$(EXE_EXT)
CMDLINE2 = gbxcart_rw_gba_cart_flash_id_check$(EXE_EXT)

# By default, build the firmware and command-line client
all: $(CMDLINE) $(CMDLINE2)

# One-liner to compile the command-line client
$(CMDLINE): gbxcart_rw_gb_cart_flash_id_check.c setup.c rs232/rs232.c
	gcc -O -std=c99 -Wall $^ -o $@
	
$(CMDLINE2): gbxcart_rw_gba_cart_flash_id_check.c setup.c rs232/rs232.c
	gcc -O -std=c99 -Wall $^ -o $@
	
# Housekeeping if you want it
clean:
	$(RM) $(CMDLINE) $(CMDLINE2)
