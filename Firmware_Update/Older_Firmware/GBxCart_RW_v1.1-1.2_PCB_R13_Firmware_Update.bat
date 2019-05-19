@echo off
@echo GBxCart RW v1.1/v1.2 PCB - Firmware R13 Update
@echo ----------------------------------------
@echo:
@echo This will update your Firmware to R13.
@echo:

setlocal
SET /P COMPORT=Please enter your COM Port Number: 

SET /P AREYOUSURE=Are you sure you wish to update the Firmware (Y/[N])?
IF /I "%AREYOUSURE%" NEQ "Y" GOTO END

tsb\gbxcart_rw_wdt_reset_v1.0.exe %COMPORT%
tsb\tsb.exe com%COMPORT%:57600 fw gbxcart_rw_v1.1-1.2_pcb_r13.hex
pause

:END
endlocal
