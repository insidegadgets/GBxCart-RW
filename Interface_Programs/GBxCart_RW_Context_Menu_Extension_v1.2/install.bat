@echo off
echo GBxCart Context Menu Extension Installer v1.2
echo ---------------------------------------------
echo This will add an extension to .gb, .gbc and .gba files so you can right click them and select "Flash with GBxCart".
pause

mkdir C:\Users\Public\GBxCart
copy GBxCart C:\Users\Public\GBxCart

regedit.exe /S gbxcart_extensions.reg

echo Completed
pause