@echo off

avrdude -p atmega8515 -c usbasp -U lfuse:w:0xef:m -U hfuse:w:0xd9:m

avrdude -p atmega8515 -c usbasp -U flash:w:GBxCart_RW_v1.1_R3\main.hex

pause