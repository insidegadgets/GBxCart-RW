@echo off
@echo GBxCart Mini RW v1.0 PCB - Firmware R14 Update
@echo ----------------------------------------
@echo:
@echo This will update your Firmware to R14.
@echo:

setlocal
SET /P COMPORT=Please enter your COM Port Number: 

SET /P AREYOUSURE=Are you sure you wish to update the Firmware (Y/[N])?
IF /I "%AREYOUSURE%" NEQ "Y" GOTO END

tsb\gbxcart_rw_wdt_reset_v1.0.exe %COMPORT%
tsb\tsb.exe com%COMPORT%:57600 fw gbxcart_mini_rw_v1.0_pcb_r14.hex
pause

:END
endlocal
