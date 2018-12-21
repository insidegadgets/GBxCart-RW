@echo OFF

GBxCart_RW_v1.1_R11\tsb\tsb.exe com16:57600 fw GBxCart_RW_v1.1-1.2_R12\main.hex
pause

GBxCart_RW_v1.1_R11\tsb\tsb.exe com16:9600 T 25
pause