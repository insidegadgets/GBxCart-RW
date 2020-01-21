@echo OFF

GBxCart_RW_v1.3_R17\tsb\tsb.exe com16:57600 fw GBxCart_RW_v1.3_R17\main.hex
pause

GBxCart_RW_v1.3_R17\tsb\tsb.exe com16:9600 T 25
pause