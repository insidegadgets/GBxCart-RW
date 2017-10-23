@echo OFF

avrdude -p atmega8515 -c usbasp -U lfuse:w:0xaf:m -U hfuse:w:0xda:m

avrdude -p atmega8515 -c usbasp -U flash:w:GBxCart_RW_v1.1_R4\tsb\tsb_m8515_d0d1_20161027.hex

pause