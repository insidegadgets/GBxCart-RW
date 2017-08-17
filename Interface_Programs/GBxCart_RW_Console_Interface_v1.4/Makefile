# Command-line client
CMDLINE = gbxcart_rw_console_v1.4.exe

# By default, build the firmware and command-line client
all: $(CMDLINE)

# One-liner to compile the command-line client
$(CMDLINE): gbxcart_rw_console_v1.4.c setup.c rs232/rs232.c
	gcc -O -Wall gbxcart_rw_console_v1.4.c rs232/rs232.c -o gbxcart_rw_console_v1.4.exe
	
# Housekeeping if you want it
clean:
	$(RM) gbxcart_rw_console_v1.4.exe
