'-----------------------------------------------------------------------
'-----------------------------------------------------------------------
'-----------------------------------------------------------------------
' CONSOLE TOOL FOR TINYSAFEBOOT
' ... the tiny and safe bootloader for AVR-ATtinys and ATmegas
'-----------------------------------------------------------------------
'-----------------------------------------------------------------------
'-----------------------------------------------------------------------
' Written in 2011-2016 by Julien Thomas <joytec@gmx.de>
'
' This program is free software; you can redistribute it and/or
' modify it under the terms of the GNU General Public License
' as published by the Free Software Foundation; either version 3
' of the License, or (at your option) any later version.
' This program is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty
' of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
' See the GNU General Public License for more details.
' You should have received a copy of the GNU General Public License
' along with this program; if not, see <http://www.gnu.org/licenses/>.
'
' ----------------------------------------------------------------------
'
' To be compiled with FreeBasic 1.20 up
'
'-----------------------------------------------------------------------
' VARIABLE AND CONSTANTS DEFINITIONS
'-----------------------------------------------------------------------
'
#lang "fb"

' Working variables

dim astr as string
dim bstr as string
dim cstr as string
dim a as ubyte
dim b as ubyte
dim i as uinteger
dim j as uinteger
dim k as uinteger
dim z as uinteger

' Shared constants and variables
const Comset = ",N,8,1,CS,DS,RB0,TB0,ME,BIN"
dim shared Comport      as string
dim shared ONEWIRE      as ubyte = 0
dim shared JMPMODE      as ubyte = 0
dim shared TINYMEGA     as ubyte = 0
dim shared SLOWMODE     as ubyte = 0
dim shared SILENT       as ubyte = 0

dim shared BIGMEGA      as ubyte = 0    ' address more than 16 bit

dim shared Filename     as string

dim shared TSBIDENT     as string
dim shared TSBBUILD     as uinteger
dim shared TSBSTATUS    as ubyte
dim shared SIG000       as ubyte
dim shared SIG001       as ubyte
dim shared SIG002       as ubyte
dim shared PAGESIZE     as ushort
dim shared FLASHSIZE    as uinteger
dim shared APPFLASH     as uinteger
dim shared EEPROMSIZE   as ushort
dim shared APPJUMP      as uinteger
dim shared TIMEOUT      as ubyte
dim shared PASSWORD     as string

dim shared AAAA         as uinteger = 0
dim shared ADDR         as uinteger = 0

dim RXTXPB (3,1)        as ubyte        ' opcode conversion array
dim shared DEVPORTS(6)  as ubyte        ' device i/o ports data-base

const maxarray          as uinteger = 1048576
dim shared BINARRAY(maxarray)   as ubyte : clear BINARRAY(0), 255, maxarray
dim shared LASTPAGE(256)        as ubyte : clear LASTPAGE(0), 255, 256

const REQUEST as string = "?"
const CONFIRM as string = "!"

'-----------------------------------------------------------------------
' SUBS AND FUNCTIONS
'-----------------------------------------------------------------------

declare sub SendCommand (as string)
declare sub DeactivateTSB()
declare sub ShowDeviceInfo()
declare sub TSBChecksum()

declare function StripSwitch(byref argument as string) as string
declare function CheckChecksum (as string) as ubyte
declare function RXBuffer () as string
declare function ReadLASTPAGE () as ubyte
declare function VerifyLASTPAGE () as ubyte
declare function Word2Date (byref InWord as ushort) as uinteger
declare function ActivateTSB (byref Comport as string) as ubyte
declare function LoadToArray () as ubyte
declare function LoadBinToArray () as ubyte
declare function LoadHexToArray () as ubyte
declare function SaveFromArray () as ubyte
declare function Check4SPM() as ubyte
declare function CurrentDateBlock () as string
declare function FWnumber () as string
declare function WriteLASTPAGE () as ubyte
declare function DatasToArray (byref avrname as string) as ubyte
declare function DatasToPortMatrix (byref avrname as string) as ubyte
declare function SignatureToDevicename (byref S0 as ubyte, S1 as ubyte, S2 as ubyte) as string
declare function ISODATE () as string
declare function PasswordInput(byref pwprompt as string) as string

'-----------------------------------------------------------------------

function ISODATE () as string
return (left(__DATE_ISO__,4) + mid(__DATE_ISO__,6,2) + right(__DATE_ISO__,2))
end function

function StripSwitch (byref argument as string) as string
return (UCase(trim (argument, any "-/")))
end function

function CheckChecksum (byref ihline as string) as ubyte
dim I as ubyte
dim C as ubyte
    if ihline = "" then return (255)
        for I = 2 To (len (ihline)) Step 2
        C = C + Val ("&h"+ (mid(ihline,i,2)))
    next I
C = 0 - C   ' equal to +1 and 2's complement (type is ubyte)
'with checksum also added, this should be 0 if record line was o.k.
return (C)
end function

sub SendCommand (byref astr as string)
    dim a as ushort
    dim t as ushort
    dim bstr as string
    if len(astr) = 0 then exit sub
    print #8, astr;             ' send commando/data string

    if ONEWIRE then             ' strip off local echo
        a = 0
        do until a = len(astr)
            t = timer + 1
            if SLOWMODE = 1 then t = timer + 10
            do until (loc(8)) or (timer > t) : loop
            bstr = input(1, #8)
            a = a + 1

        loop
    endif

end sub

function RXBuffer () as string
' adaptive RS232 buffer readout
dim a as ushort
dim t as double
dim w as ushort
dim astr as string = ""
dim bstr as string = ""
'
bstr = ""

if SLOWMODE then
        ' read and empty RX buffer under slow baudrates (< 1200 bps)
        t = timer + 10
        do until loc(8) or (timer > t) : loop
        if loc(8) = 0 then return ("")
        w = (10 - (t - timer)) * 3000
        '? w
        t = timer + 10
        do until eof(8) or (timer > t)
                a = loc(8)
                sleep 500
                sleep w
                a = loc(8) - a
                if a = 0 then
                        astr = input(loc(8), #8)
                else
                        astr = input(1, #8)
                endif
                bstr = bstr + astr
        loop
else
        ' read and empty RX buffer under modest baudrates (> 1200 bps up)
        t = timer + 3
        do until loc(8) or (timer > t) : loop   ' try to read first characters
        if loc(8) = 0 then return ("")          ' no success
        w = (3 - (t - timer)) * 1500
        t = timer + 3
        do until eof(8) or (timer > t)
                a = loc(8)
                sleep 5
                sleep w
                a = loc(8) - a
                if a = 0 then
                        astr = input(loc(8), #8)
                else
                        astr = input(1, #8)
                endif
                bstr = bstr + astr
        loop

endif

return (bstr)

end function

function ReadLASTPAGE () as ubyte

'load userdata from LASTPAGE into array LASTPAGE()

dim i as ushort
dim bstr as string

print #99, "READ USER DATA   ... ";
SendCommand ("c")

bstr = RXBuffer

if len(bstr) < PAGESIZE+1       then return (255) ' page read error
if right(bstr,1) <> CONFIRM    then return (255) ' block not terminated

clear LASTPAGE(0), 255, 256
for i = 0 To PAGESIZE-1 : LASTPAGE (i) = bstr[i] : next i

APPJUMP = (LASTPAGE(0) + LASTPAGE(1)*256) ' low/high byte in memory
if TINYMEGA = 1 then APPJUMP = 0000       ' ATmegas APPJUMP not relevant

TIMEOUT = LASTPAGE(2)   ' load timeout byte from LASTPAGE

i = 3 : PASSWORD = ""
do until (i = PAGESIZE) or (LASTPAGE(i) = 255)
        PASSWORD = PASSWORD + chr(LASTPAGE(i))
        i = i + 1
loop

print #99, "OK" : print #99, ""

return (0)

end function

function VerifyLASTPAGE () as ubyte

dim i as ushort
dim bstr as string

print #99, "VERIFY USER DATA ... ";

SendCommand ("c")       ' reload LASTPAGE

bstr = RXBuffer

if len(bstr) < 16       then return (255)       ' page not correctly read
if right(bstr,1) <> CONFIRM then return (255)  ' block not terminated

for i = 0 to len(bstr)-1
    if LASTPAGE (i) <> bstr[i] then exit for
next i

if i < (len(bstr)-1) then return (255)  ' oh no, errors detected!

print #99, "OK" : print #99, ""

return (0)              ' verification against LASTPAGE successfull

end function

function Word2Date (byref InWord as ushort) as uinteger
Word2Date =     ( InWord and 31 ) + _
        ((InWord and 480) \ 32) * 100 + _
        ((Inword and 65024 ) \ 512) * 10000 _
        + 20000000
end function

function ActivateTSB (byref Comport as string) as ubyte

dim bstr as string

if open com (Comport + Comset for binary as #8) > 0 then return (Err)
sleep 100

if SLOWMODE = 1 then
        sleep 500
        print #99, ""
        print #99, "SLOW MODE. Please be patient ..."
endif

print #8, "@@@";        ' first try without password
                        ' when there is no reaction, password may be needed
bstr = RXBuffer

if left(bstr,3) = "@@@" then
        ONEWIRE = 1                ' one-wire echo detected
        bstr = right(bstr, (len(bstr)-3))  ' detach echoed characters
        print #99, "" : print #99, "ONE-WIRE INTERFACE DETECTED."
endif

if bstr = "" then
        print
        PASSWORD = PasswordInput ("Password : ")
        SendCommand (PASSWORD)              ' submit password
        bstr = RXBuffer
        if bstr <> "" then                  ' now we got a reply
        print : print "Password ... OK"
        else return (255)
        endif
endif

' analyze tsb's reply to gain device data (permanent data string)
if right(bstr,1)        <> CONFIRM     then return (255)      ' no good
if lcase(left(bstr,3)) <> "tsb"       then return (255)      ' no good

dim BUILDWORD as ushort

TSBIDENT        = left(bstr,3)
BUILDWORD       = bstr[3] + bstr[4] * 256
TSBSTATUS       = bstr[5]
SIG000          = bstr[6]
SIG001          = bstr[7]
SIG002          = bstr[8]
PAGESIZE        = bstr[9] * 2                           ' WORDS * 2 = BYTES
APPFLASH        = (bstr[10] + bstr[11] * 256) * 2       ' WORDS * 2 = BYTES
FLASHSIZE       = ((APPFLASH \ 1024) + 1) * 1024        ' round up to multiple of 1024
EEPROMSIZE      = (bstr[12] + bstr[13] * 256) + 1       ' EEPROM size always BYTES

' NOTE: While AVR/Assembler is accounting flash addresses in WORDS
'       (except from EEPROM), on the PC we use BYTES
'
' Check for regular PAGESIZEs (transmission successfull)

if      (PAGESIZE mod 16) > 0 then
        print #99, "PAGESIZE NOT VALID - ABORT."
        return (255)
endif

' consider TSB firmware with new date identifier and status byte
if BUILDWORD < 32768 then
    TSBBUILD = Word2Date (BUILDWORD)
else    'old date encoding in tsb-fw (three bytes)
    TSBBUILD = BUILDWORD + 65536 + 20000000
endif

' detect whether device is ATtiny or ATmega from identifier
' in last byte of device info block
' while decision for jmp/rjmp depends on memory size
select case asc(mid(bstr,15,1))
case &h00 : JMPMODE = 0 : TINYMEGA = 0
case &h0C : JMPMODE = 1 : TINYMEGA = 0
case &hAA : JMPMODE = 0 : TINYMEGA = 1
end select

print #99, ""

return ReadLASTPAGE()        ' 0 = OK / <> 0 = ERROR

end function

sub DeactivateTSB ()
print #8, "q";
close #8
end sub

function LoadBinToArray () as ubyte

clear BINARRAY(0), 255, maxarray

dim ff as ubyte = freefile
dim i as ubyte
dim as uinteger Max_AAAA = 0

' Load binary file

        AAAA = 0
        if open (Filename for binary as ff) <> 0 then
            print "File not found!" : return (1)
        endif

        if lof(ff) = 0 then return (1)
        do while AAAA < lof (ff)
            BINARRAY(AAAA) = asc(input(1,ff))
        AAAA = AAAA + 1
        loop

        if AAAA <> lof(ff) then return (1) ' load error

close ff

' if function leaves here, the binary data from file has been loaded
' into BINARRAY() and AAAA is the number of bytes *sharply*.

return (0)

end function

function LoadHexToArray () as ubyte

clear BINARRAY(0), 255, maxarray

dim ff as ubyte = freefile
dim ihline as string
dim LL as ubyte
dim i as ubyte
dim as uinteger Max_AAAA = 0

' Try to load ihex encoded file by default.
' Return values:
' 0 = success
' 1 = file not found
' 2 = no valid ihex
' 3 = checksum error

' Load ihex file

    if open (Filename for input as #ff) <> 0 then close ff: return (1)

        While not eof(ff)

        line input #ff, ihline       ' fetch whole lines (CR/LF-terminated)

        if left(ihline,1) <> ":" then close ff : return (2) ' no valid ihex

        ihline = UCase(ihline)

        if CheckChecksum (ihline) then close ff : return (3) ' checksum error

            if mid(ihline, 8,2) = "00" then
                LL =   Val("&h"+mid(ihline,2,2))
                AAAA = Val("&h"+mid(ihline,4,4))

                for i = 1 To LL

                    BINARRAY(AAAA) = val("&h" + mid( ihline, 8 + i*2 , 2 ) )

                                        if AAAA > Max_AAAA then Max_AAAA = AAAA: endif

                    AAAA = AAAA+1

                next i

            endif

        Wend

        AAAA = Max_AAAA ' set AAAA to max address of any hexfile content loaded

        AAAA = AAAA+1   ' set return value of AAAA to AAAA+1 ( = number of bytes)

close ff

' if function leaves here, contents of the hex file have been successfully
' loaded into BINARRAY() with AAAA as the exact number of bytes thereof.

return (0)

end function

function LoadToArray () as ubyte

select  case LoadHexToArray()
        case 1 : print #99, "File not found." : return (1)
        case 2 : if LoadBinToArray () <> 0 then return(1)
        case 3 : print #99, "Checksum error in Hex record." : return (1)
end select

return (0)

end function

function SaveFromArray () as ubyte

if AAAA = 0 then return (1)
ADDR = 0
dim ff as ubyte = freefile
dim i as ubyte
dim C as ubyte
dim astr as string
const CRLF as string = chr(13) + chr(10) ' all lines CRLF terminated

if lcase(right(Filename,4)) = ".hex" _
or lcase(right(Filename,4)) = ".eep" then

' Save Intel HEX encoded file

    if AAAA > 2047 then ' skip large areas of empty/undefined flash
        do until (BINARRAY(ADDR) <> 255) or (ADDR > AAAA)
        ADDR = ADDR + 1
        loop
    endif

    if open (Filename for output as #ff) <> 0 then return (Err)

    print #ff, ":020000020000FC"; CRLF;         ' heading < 64 KB only
    'print #ff, ":020000021000EC"; CRLF;     ' heading for 128 KB devs
    'print #ff, ":020000023000CC"; CRLF;     ' heading for 256 KB devs
    'print #ff, ":020000027000CC"; CRLF;     ' heading for 512 KB devs
    'print #ff, ":02000002F000CC"; CRLF;     ' heading for 1024 KB devs

    do until ADDR >= AAAA
    astr = ":10" + whex (ADDR, 4) + "00"
        for i = 1 to 16
            astr = astr + whex(BINARRAY(ADDR),2)
            ADDR = ADDR + 1
        next i
    C = 0
        for i = 2 to (len (astr)) step 2
            C = C + val ("&h"+ (mid(astr,i,2)))
        next I

    C = 0 - C

    astr = astr + whex(C,2)

    print #ff, astr; CRLF;
    loop
    astr = ":00000001FF" :  print #ff, astr; CRLF;  ' end of file record

else

' Save binary file

        ADDR = 0
        if open (Filename for output as #ff) <> 0 then return (Err)
        do until (ADDR = AAAA)
        print #ff, chr(BINARRAY(ADDR));
        ADDR = ADDR + 1
        loop

endif

close ff

return (Err)

end function

function Check4SPM() as ubyte
dim CheckADDR as ushort = 0
do until CheckADDR >= AAAA
    if BINARRAY(CheckADDR)=&hE8 and BINARRAY(CheckADDR+1)=&h95 then
    exit do
    endif
    CheckADDR = CheckADDR + 2
loop
  if CheckADDR\2 < AAAA\2 then return (255) else return (0)
end function

function CurrentDateBlock () as string
' this function is currently not needed
dim bstr as string * 8
    bstr = right(date, 4)
    bstr = bstr + left(date,2)
    bstr = bstr + mid(date,4, 2)
return bstr
end function

function FWnumber () as string

dim fstr as string * 8
dim taddr as ushort
dim BUILDWORD as ushort

taddr = AAAA - 256      ' start search well below last page
fstr = ""

do  ' find TSB's device info block
taddr = taddr + 1
fstr = chr(BINARRAY(taddr)) + chr(BINARRAY(taddr+1)) + chr(BINARRAY(taddr+2))
loop until (fstr = "tsb") or (fstr = "TSB") or (taddr = AAAA)

if taddr = AAAA then return ("")
BUILDWORD = (BINARRAY(taddr+3) + BINARRAY(taddr+4)*256)
if BUILDWORD < 32768 then
    fstr = str( Word2Date(BUILDWORD) )
else
    fstr = str( BINARRAY(taddr+3) + BINARRAY(taddr+4) * 256 + 65536 + 20000000)
endif
return fstr

end function

function WriteLASTPAGE () as ubyte

dim i as ushort
dim astr as string
dim bstr as string

print #99, "WRITE USER DATA  ... ";

SendCommand ("C")               ' command Change for TSB

if RXBuffer <> REQUEST then return (255)    ' would be error

        for i = 0 to PAGESIZE-1
        Astr = Astr + chr(LASTPAGE(i))
        next i

SendCommand (CONFIRM + Astr)

if RXBuffer = CONFIRM then return (255)     ' would be error

print #99, "OK" : print #99, ""

return (0)      ' write of LASTPAGE successfully finished

end function

function DatasToArray (byref avrname as string) as ubyte

Clear BINARRAY(0), 255, maxarray

dim ihline as string
dim LL as ubyte
dim i as ubyte
dim max_AAAA as UInteger : max_AAAA = 0

avrname = lcase(avrname)

restore

' search for beginning of tsb-templates / tt-datas
' (included and compiled into this program)
do
read ihline
loop until lcase(ihline) = "begin templates" or ihline = ""

if ihline ="" then return (1)   ' serious problem: no tt-datas found!

' o.k. datas found, now parse for the desired attiny
do
read ihline
loop until (ihline = avrname) or lcase(ihline) = "end templates"

if ihline = "end templates" then return (1)    ' did not find that stuff

    do
        read ihline     'fetch hex record from datas
        if CheckChecksum (ihline) then return (1)
        if mid(ihline, 8,2) = "00" then
            LL =   Val("&h"+mid(ihline,2,2))
            AAAA = Val("&h"+mid(ihline,4,4))
            for i = 1 to LL
                BINARRAY(AAAA) = Val("&h"+mid(ihline, 8+i*2,2))
                if AAAA > max_AAAA then max_AAAA=AAAA
                AAAA = AAAA+1
            next i
        endif
    loop until ucase(ihline) = ":00000001FF"   'ihex terminator
    AAAA = Max_AAAA+1  'set AAAA to max address of hexfile contents + 1

return (0)

end function

function DatasToPortMatrix (byref avrname as string) as ubyte

dim astr as string
dim a as Ubyte
dim i as Ubyte

avrname = lcase(avrname)

' search datas of port definitions
restore

do
read astr
loop until lcase(astr) = "begin devices" or astr = ""
if astr ="" then return (1)

' o.k. found them, now parse the desired ATtiny
do
read astr
astr = lcase(astr)
loop until (astr = avrname) or lcase(astr) = "end devices"
if astr ="end devices" then return (1)     ' no match found

' device found, now skip the 3 signature bytes
read a, a, a

' load and convert port assignments
for i = 0 to 6
        read DEVPORTS(i)
        if DEVPORTS(i) < &hFF then DEVPORTS(i) = DEVPORTS(i) * 8
next i

return (0)

end function

function SignatureToDevicename (byref S0 as ubyte, S1 as ubyte, S2 as ubyte) as string

dim astr        as string
dim a           as ubyte
dim avrname     as string
dim i           as ubyte
dim as ubyte    Sig0, Sig1, Sig2

' find signature-to-devicename mapping for given signature

restore
do
        read astr
loop    until lcase(astr) = "begin devices" or astr = ""

if astr ="" then return ("DATABASE ERROR")

' o.k. found them, now parse for given signature

do
    read avrname                ' read Device Name
    read Sig0, Sig1, Sig2       ' read Device Signature
    read a,a,a,a,a,a,a          ' dummy read further port data
    a = 0
    if (S0 = Sig0) and (S1 = Sig1) and (S2 = Sig2) then a = 1

loop until (a=1) or lcase(avrname) = "end devices"

if avrname ="end devices" then return ("UNKNOWN DEVICE")     ' no match found

if left(avrname,2) = "tn" then avrname = trim (avrname, "tn")   : avrname = "ATtiny" + avrname
if left(avrname,1) = "m"  then avrname = trim (avrname, "m")    : avrname = "ATmega" + avrname
if right(avrname,1)= "A"  then avrname = left(avrname, len(avrname)-1)

return (avrname)

end function

sub ShowDeviceInfo()
        print #99,
        print #99, "TINY SAFE BOOTLOADER"
        print #99, "VERSION   : "; TSBBUILD
        print #99, "STATUS    : "; whex(TSBStatus)
        print #99, "SIGNATURE : "; whex(SIG000,2);" "; whex(SIG001,2);" "; whex(SIG002,2)
        print #99, "DEVICE    : "; SignatureToDevicename(SIG000, SIG001, SIG002)
        print #99, "FLASH     : "; FLASHSIZE
        print #99, "APPFLASH  : "; APPFLASH
        print #99, "PAGESIZE  : "; PAGESIZE
        print #99, "EEPROM    : "; EEPROMSIZE
        print #99, "APPJUMP   : "; whex(APPJUMP,4)
        print #99, "TIMEOUT   : "; TIMEOUT
        print #99,
end sub

sub TSBChecksum()
' Parse customized TSB-Firmware/Installer loaded to BINARRAY():
' (1) Found TSB-Installer = calculate checksum beginning from 2nd page
'     and store in the last 2 bytes prior to TSB-Firmware-segment
' (2) Found TSB- or other firmware = do not change anything

dim i as ushort
dim j as ushort

dim FWChecksum as ushort = &h0000

i = 3   ' skip possible RJMP/JMP in 1st page (TSB-Installer)

do : i = i + 1 : loop until (BINARRAY(i)) < 255 or (i > 256)

if i < 8 then return    ' apparently no TSB-Installer
if i > 256 then return  ' possibly TSB-Firmware, no checksum to calc

j = i + 2
i = i + 4

BINARRAY(j +0) = 0
BINARRAY(j +1) = 0
BINARRAY(AAAA) = 255

do
    FWChecksum = FWChecksum + BINARRAY(i)
    i = i + 1
loop    until i = AAAA

BINARRAY(j+0) = FWChecksum \ 256
BINARRAY(j+1) = FWChecksum - (FWChecksum \ 256) * 256

end sub

function PasswordInput(byref pwprompt as string = "") as string

dim pstr As string
dim pchr As string
dim a as Ubyte
dim b as Ubyte

print pwprompt;

a = 0
b = 0

do
        pchr = inkey

        if pchr <> "" then

                a = asc (pchr)

                if a = 13 then exit do                  ' enter
                if a = 8 then
                        if b > 0 then
                                print chr(8);          ' backspace
                                print chr(32);
                                print chr(8);
                                b = b - 1
                                pstr = left(pstr, b)
                        endif

                else
                        pstr = pstr + chr(a) : b = b + 1
                        print "*";
                endif

        endif
loop

print ""

return (pstr)

end function
'-----------------------------------------------------------------------
'-----------------------------------------------------------------------
'-----------------------------------------------------------------------


'-----------------------------------------------------------------------
' MAIN PROGRAM - PARSE COMMANDLINE
'-----------------------------------------------------------------------

Mainprogram:

' very last argument may be the 'silent-switch'
if StripSwitch (command(__FB_ARGC__ - 1)) = "S" then
        ' no output to screen
        open "NUL"  for output as #99   : SILENT = 1
else
        ' messages output to screen
        open "SCRN" for output as #99   : SILENT = 0
endif

if __FB_ARGC__ < 2      then goto HelpScreen

astr = StripSwitch(command(1))
if astr = ""     then goto HelpScreen
if astr = "L"    then goto Licensenote
if astr = "H"   or astr = "?" then goto HelpScreen

' 1st argument may be either some COM-Port or AVR-Devicename

astr = lcase(command(1))

If      left(astr,3) <> "com"  and _
        left(astr,4) <> "/dev" and _
        left(astr,4) <> "/tty" then goto MakeMode ' make bootloader

Comport = command(1)         ' user wants to talk to the bootloader

        ' check comport string for baudrate
i = 0 : Comport = trim (Comport, ":")
do : i=i+1 : loop until (i=len(Comport)) or (mid(Comport,i,1) = ":")
if i = (len(Comport)) then
        ' no baudrate specified, use default
        Comport = Comport + ":9600"
else    ' baudrate specified, decide for slow speed mode
        if (val(right(Comport, (len(Comport) - i ))) < 600) then SLOWMODE = 1
endif

astr = StripSwitch(command(2))         ' check mode switch

' these options don't require filename
If astr = ""    then goto ShowInfo
If astr = "I"   then goto ShowInfo
If astr = "?"   then goto HelpScreen
If astr = "P"   then goto PasswordChange
If astr = "T"   then goto TimeoutChange
If astr = "EE"  then goto EEPROMerase
If astr = "FE"  then goto FLASHerase
If astr = "XXX" then goto EmergencyErase

' these options require filename
Filename = (command(3))                ' get filename

If Filename = "" then goto LError
If astr = "FR"  then goto FLASHread
If astr = "FW"  then goto FLASHwrite
If astr = "FV"  then goto FLASHverify
If astr = "ER"  then goto EEPROMread
If astr = "EW"  then goto EEPROMwrite
if astr = "EV"  then goto EEPROMverify

' other unspecified commands
goto LError

'-----------------------------------------------------------------------
' END COMMANDLINE PARSER
'-----------------------------------------------------------------------
'
'
'
'
'-----------------------------------------------------------------------
' MAKE CUSTOMIZED TSB FIRMWARE FLASH FILE FROM FIRMWARE TEMPLATE
'-----------------------------------------------------------------------
' Search internal database for template, make TSB with customized ports.

MakeMode:

AAAA = 0
ADDR = 0
astr = lcase(astr)

Filename = Astr

print #99, ""
'
' STEP 1:
' First we search the Database for pre-assembled code template
' of the respective device that matches the given Filename.
'
if len (StripSwitch(Filename)) < 2 then goto LError
if DatasToArray(Filename) then
    print #99, "Sorry, this Device is not in the database yet." : print #99, ""
    goto LError
endif
'
' STEP 2:
' If Template has been successfully loaded, we get the port definitions
' (ports existing, ports i/o) for that device.
'
if DatasToPortMatrix(Filename) <> 0 then
    print #99, "Matrix error." : print #99, ""
    goto LError
endif

if AAAA < 250             then goto LError  ' binary way too short
if Len (command(2)) <> 4 then goto LError  ' port definitions seem ok

' Filename for saving customized tsb
if command(3) = "" then                ' if no filename specified, save in hex anyway
        Filename = "tsb_" + astr + "_" + _
        lcase(command(2)) + "_" +  FWnumber() + ".hex"
        else Filename = command(3)     ' otherwise try using filename from commandline
endif
'
' STEP 3:
' Prepare working array of RX/TX addresses
' for the ports of chosen device.
'
astr = UCase(command(2))
'valid port name ranges from "A" to "G"
a = asc(mid(astr,1,1)) : if a < 65 or a > 71 then goto LError
a = a - 65                                      ' result can be 0,1,2,3
'valid port bits range from "0" to "7"
b = val (mid(astr,2,1))
if b > 7 then              print #99, "Portbit must range from 0 to 7." : goto LError
if DEVPORTS(a) = &hFF then print #99, "Invalid port assignment."        : goto LError

RXTXPB(0,0) = DEVPORTS (a) + 0  + b          ' PINx
RXTXPB(1,0) = DEVPORTS (a) + 8  + b          ' DDRx
RXTXPB(2,0) = DEVPORTS (a) + 16 + b          ' PORTx

'ensure that port name ranges from "A" to "G"
a = asc(mid(astr,3,1)) : if a < 65 or a > 71 then goto LError
a = a - 65
'ensure that port bit  ranges from "0" to "7"
b = val (mid(astr,4,1))
if b > 7 then              print #99, "Portbit must range from 0 to 7." : goto LError
if DEVPORTS(a) = &hFF then print #99, "Invalid port assignment."        : goto LError

RXTXPB(0,1) = DEVPORTS (a) + 0  + b          ' PINx
RXTXPB(1,1) = DEVPORTS (a) + 8  + b          ' DDRx
RXTXPB(2,1) = DEVPORTS (a) + 16 + b          ' PORTx

'for i = 0 to 2 : ? whex(RXTXPB(i,0),2), whex(RXTXPB(i,1),2): next i

print #99, "Make TSB from code template:   "; Filename
print #99, ""
print #99, "RXD = P"; mid(astr,1,2); " / TXD = P"; mid(astr,3,2)
print #99, ""
'
' STEP 4:
' Search pre-assembled firmware template for dummy port assignments
' (being RX = PINB0/DDRB0/PORTB0 / TX = PINB1/DDRB1/PORTB1)
' and modify these with respect to the desired ports and portbits
'
do until ADDR >= AAAA

    select case BINARRAY(ADDR+1)
    case &h98, &h99, &h9A, &h9B     ' find opcodes of cbi/sbic/sbi/sbis

        select case (BINARRAY (ADDR+0) And &b00000111)

        case 0
            select case (BINARRAY (ADDR+0) And &b11111000)
            case DEVPORTS(1)+0   : BINARRAY(ADDR+0) = RXTXPB(0,0)  ' change PINB
            case DEVPORTS(1)+8   : BINARRAY(ADDR+0) = RXTXPB(1,0)  ' change DDRB
            case DEVPORTS(1)+16  : BINARRAY(ADDR+0) = RXTXPB(2,0)  ' change PORTB
            end select

        case 1
            select case (BINARRAY (ADDR+0) And &b11111000)
            case DEVPORTS(1)+0   : BINARRAY(ADDR+0) = RXTXPB(0,1)  ' change PINB
            case DEVPORTS(1)+8   : BINARRAY(ADDR+0) = RXTXPB(1,1)  ' change DDRB
            case DEVPORTS(1)+16  : BINARRAY(ADDR+0) = RXTXPB(2,1)  ' change PORTB
            end select

        end select

    end select

    ADDR = ADDR + 2

    print #99, chr(13) ; "Modifying reference code " ; whex(ADDR,4); " ... ";

loop

print #99, "OK"
print #99, ""
'
' STEP 5:
' Save modified machine code with filename determined above
'
TSBchecksum()   ' calculate actual checksum for TSB-Installer
SaveFromArray()
print #99, "Saved TSB firmware file:   "; Filename

goto LSuccess


'-----------------------------------------------------------------------
' EEPROM READ
'-----------------------------------------------------------------------
EEPROMread:

if ActivateTSB(Comport) <> 0 then goto LError

ADDR = 0

SendCommand ("e")

do
        SendCommand(CONFIRM)
        astr = RXBuffer
                if len(astr) < PAGESIZE then goto LError
        print #99, ".";
        for i = 1 To PAGESIZE
        BINARRAY (ADDR) = asc(mid(astr,i,1))

        ADDR = ADDR +1
        next
        print #99, chr(13) ; "EEPROM READ  " ; whex(ADDR-1,3); " ... ";

loop    until ADDR = EEPROMSIZE

print #99, "OK" : print #99, ""

AAAA = EEPROMSIZE : SaveFromArray()
print #99,

goto LSuccess

'-----------------------------------------------------------------------
' EEPROM ERASE (OVERWRITE EEPROM WITH FF)
'-----------------------------------------------------------------------
EEPROMerase:

if ActivateTSB(Comport) <> 0 then goto LError
print #99, "EEPROM ERASE ... ";
ADDR = 0
SendCommand ("E")
do      until (RXBuffer <> REQUEST) or ADDR = EEPROMSIZE
        SendCommand (CONFIRM)
        astr = string(PAGESIZE, chr(255))
        SendCommand (astr)
        ADDR = ADDR + PAGESIZE
loop
' ensure we are back on command level
SendCommand (REQUEST)
if RXBuffer <> CONFIRM then goto LError
print #99, "OK"  : print #99, ""

goto LSuccess

'-----------------------------------------------------------------------
' EEPROM WRITE
'-----------------------------------------------------------------------
EEPROMwrite:

if LoadToArray <> 0 then goto LError
if ActivateTSB(Comport) <> 0 then goto LError
if AAAA > EEPROMSIZE then print #99, "File too long!" : goto LSuccess

ADDR = 0
AAAA = (((AAAA-1)\PAGESIZE)+1)*PAGESIZE     'round up to full page

SendCommand ("E")

do      until (RXBuffer <> REQUEST) Or ADDR = AAAA
        SendCommand (CONFIRM)
        astr = ""
        for i = 1 To PAGESIZE
                astr = astr + Chr(BINARRAY(ADDR))
                ADDR = ADDR + 1
        next i
        SendCommand (astr)

        print #99, Chr(13) ; "EEPROM WRITE $" ; whex(ADDR-1,3); " ... ";
loop

' ensure we are back on command level

SendCommand (REQUEST)
if RXBuffer <> CONFIRM then goto LError

print #99, "OK"  : print #99, ""

goto LSuccess

'-----------------------------------------------------------------------
' EEPROM VERIFY
'-----------------------------------------------------------------------
EEPROMverify:

if ActivateTSB(Comport) <> 0 then goto LError

if LoadToArray <> 0 then goto LError

if AAAA > EEPROMSIZE then print #99, "File too long!" : goto LError

ADDR = 0

SendCommand ("e")

do
        SendCommand (CONFIRM)
        astr = RXBuffer
        if Len(astr) < PAGESIZE then goto LError
        for i = 1 To PAGESIZE
                print #99, Chr(13) ; "EEPROM VERIFY $" ; whex(ADDR-1,3); " ... ";
                if BINARRAY (ADDR) <> asc(mid(astr,i,1)) then Exit do
                ADDR = ADDR+1
        next i
loop    until ADDR = EEPROMSIZE

if ADDR = EEPROMSIZE or right(astr,1) = CONFIRM then
        print #99, "OK"
        else
        beep
        print #99, "--- ERROR! ---" : goto LError
endif

goto LSuccess

'-----------------------------------------------------------------------
' FLASH READ
'-----------------------------------------------------------------------

FLASHread:

if ActivateTSB(Comport) <> 0 then goto LError
ADDR = 0
AAAA = APPFLASH

SendCommand ("f")

do
        SendCommand(CONFIRM)
        astr = RXBuffer
        if len(astr) < PAGESIZE then goto LError

        for i = 0 To PAGESIZE-1
                BINARRAY (ADDR) = astr[i]
                ADDR = ADDR + 1
        next i

        print #99, chr(13) ; "FLASH READ $" ; whex(ADDR-1,4); " ... ";

loop    until ADDR = APPFLASH

'print #99,
print #99, "OK"
print #99,

' remove unallocated (erased) pages from end of read-back application
do while ((BINARRAY(AAAA)) = 255) and AAAA > 0
        AAAA = AAAA - 1
loop

' recover original reset vector (from appjump in LASTPAGE)
if AAAA > 0 then
        if TINYMEGA = 0 then
                if JMPMODE = 0 then                     ' relative jump

                        if (APPJUMP - &hC000) >    (4096 - FLASHSIZE\2) then
                                ' newer TSB with backward rjmp
                                i = APPJUMP - PAGESIZE\2 - (4096 - FLASHSIZE\2)
                        else
                                ' older TSB with forward rjmp
                                i = APPJUMP - PAGESIZE\2
                        endif

                        BINARRAY(1) = i\256
                        BINARRAY(0) = i-BINARRAY(1)*256
                else                                    ' absolute jump
                        BINARRAY(2) = LASTPAGE(0)
                        BINARRAY(3) = LASTPAGE(1)
                endif
        endif

        SaveFromArray
else
        print #99, "FLASH WAS EMPTY. NO FILE SAVED."
endif

goto LSuccess


'-----------------------------------------------------------------------
' FLASH ERASE
'-----------------------------------------------------------------------
' Making use of the Flash Write feature in TSB-FW: starting flash write
' session but without sending data to be written will only erase the
' whole Appflash.

FLASHerase:
if StripSwitch (command(3)) = "S" then SILENT = 1
if ActivateTSB(Comport) <> 0 then goto LError
SendCommand ("F")
print #99, "FLASH ERASE ... ";
if RXBuffer <> REQUEST then goto LError
SendCommand (REQUEST)       ' other than CONFIRM will do as well
if RXBuffer <> CONFIRM then goto LError
print #99, "OK"
goto LSuccess

'-----------------------------------------------------------------------
' FLASH WRITE
'-----------------------------------------------------------------------
FLASHwrite:

if ActivateTSB(Comport) <> 0 then goto LError
if LoadToArray <> 0 then goto LError

'round up to full pages (multiple of PAGESIZE)
AAAA = ( ( (AAAA-1) \ PAGESIZE ) +1 ) * PAGESIZE

if AAAA > APPFLASH then print #99, "File too long / Address conflict !" : goto LSuccess


if SILENT = 0 then
if TINYMEGA = 0 then
        if Check4SPM () <> 0 then
                print "WARNING: The firmware you are about to upload"
                print "contains the SPM opcode that performs direct flash writes"
                print "and thus may be dangerous for the bootloader."
                print ""
                input "Continue anyway (Y/n) "; astr
                print
                if astr <> "Y" then goto LSuccess
        endif
endif
endif

ADDR = 0

SendCommand ("F")

if RXBuffer <> REQUEST then goto LError

'do until ADDR >= AAAA
do until ADDR >= AAAA
    SendCommand (CONFIRM)       'CONFIRM to bootloader's REQUEST
    astr = ""
    for i = 1 to PAGESIZE
        astr = astr + chr(BINARRAY(ADDR))
        ADDR = ADDR + 1
    next i
    SendCommand (astr)          'send page data block to bootloader
    print #99, chr(13) ; "FLASH WRITE $" ; whex(ADDR-1,4); " ... ";
    bstr = RXBuffer

    if bstr <> REQUEST then exit do     'Error or end of flash override

loop

if (bstr<>REQUEST) and (right(bstr,1)=CONFIRM) then
    print #99,
    print #99, "INTERNAL VERIFY ERROR - FLASH ERASE INITIATED !" : goto LError
endif

if (bstr<>REQUEST) and (right(bstr,1)<>CONFIRM) then
    print #99,
    print #99, "UNDEFINED ERROR WHILST FLASH WRITE!" : goto LError
endif

if ADDR < APPFLASH then
    SendCommand (REQUEST)                       ' finished
    if RXBuffer <> CONFIRM then goto LError     ' not confirmed?
endif

print #99, "OK"

goto LSuccess

'-----------------------------------------------------------------------
' FLASH VERIFY
'-----------------------------------------------------------------------
FLASHverify:

if ActivateTSB(Comport) <> 0 then goto LError

if LoadToArray <> 0 then goto LError

SendCommand ("f")               ' = read Flash
ADDR = 4

SendCommand (CONFIRM)
' Always skip first 4 bytes with RJMP/JMP
astr = RXBuffer
if len(astr) < PAGESIZE then goto LError
    for i = 5 To PAGESIZE
        print #99, chr(13) ; "FLASH VERIFY $" ; whex(ADDR-1,4); " ... ";
        if BINARRAY (ADDR) <> asc(mid(astr,i,1)) then exit for
        ADDR = ADDR+1
    next
if ADDR < PAGESIZE then

        print #99, "First Verify Error at $"; whex(ADDR,4); " - Abort!"
        close : beep : end
endif

if ADDR < AAAA then             ' Compare remaining flash pages
        do
        SendCommand (CONFIRM)
        astr = RXBuffer
        for i = 1 To PAGESIZE
                print #99, chr(13) ; "FLASH VERIFY $" ; whex(ADDR,4); " ... ";
                if BINARRAY (ADDR) <> asc(mid(astr,i,1)) then exit do
                if ADDR = AAAA then exit do
                ADDR = ADDR+1
        next i
        loop    until ADDR = AAAA
endif

if AAAA <= ADDR then
    print #99, "OK"
    else
    print #99, "First Verify Error at $"; whex(ADDR,4); " - Abort!"
    close : beep : end
endif

end


'-----------------------------------------------------------------------
' PASSWORD CHANGE DIALOG
'-----------------------------------------------------------------------

PasswordChange:

if ActivateTSB(Comport) <> 0 then goto LError
    print
    print using "Enter new Password (max. ### chars) : "; (PAGESIZE-4);

    astr = passwordinput("")

    if Len(astr) > (PAGESIZE-4) then print "Password too long!": goto LError

    print       "Please re-enter your new Password   : ";

    bstr = passwordinput("")

    print
    if astr <> bstr then print "Passwords not equal - Nothing changed.": beep: close : end

if astr <> "" then
    print "WARNING: If your Password got lost, you will no longer be able"
    print "to maintain Userdata (Flash, EEPROM, Timeout and Password)."
    print "To regain full access to TSB, you still have option to perform"
    print "an Emergency Erase."
        print
    input "Change the Password (Y/n) "; cstr
    print
    if cstr <> "Y" then print "Nothing changed!": goto LSuccess

    astr = astr + chr(255)  ' add FF terminator to new password
    ' also erase whole LASTPAGE except from APPJUMP and TIMEOUT
    for i = 3 to PAGESIZE - 1 : LASTPAGE(i) = 255 : next i
    for i = 1 to len(astr) : LASTPAGE(2+i) = astr[i-1] : next i

    if WriteLASTPAGE () <> 0 then goto LError
    if VerifyLASTPAGE() <> 0 then goto LError
    if ReadLASTPAGE()   <> 0 then goto LError
    print "NEW PASSWORD DEFINED."

else
    for i = 3 To PAGESIZE-1 : LASTPAGE(i) = 255 : next i    'clear password
    if WriteLASTPAGE () <> 0 then goto LError
    if VerifyLASTPAGE() <> 0 then goto LError
    if ReadLASTPAGE()   <> 0 then goto LError
    print "TSB USES NO PASSWORD."
endif
Showdeviceinfo()
goto LSuccess

'-----------------------------------------------------------------------
' TIMEOUT CHANGE (DIALOG OR INLINE)
'-----------------------------------------------------------------------

TimeoutChange:

if ActivateTSB(Comport) <> 0 then goto LError
if command(3) = "" then
        print      "OLD TIMEOUT BYTE : "; TIMEOUT
        print
        line input "NEW TIMEOUT BYTE : ", astr
        print #99, ""
else
        astr = command(3)
endif

a = val(astr)

if SLOWMODE = 0 then

        if a < 8 or a > 255 then
        print #99, ""
        print #99, "ERROR: Timeout must range from 4 to 255!"
        print #99, ""
        print #99, "Nothing changed!"
        goto LSuccess
        endif
endif

LASTPAGE(2) = a
if WriteLASTPAGE()  <> 0 then goto LError
if VerifyLASTPAGE() <> 0 then goto LError
'if ReadLASTPAGE()   <> 0 then goto LError
TIMEOUT = LASTPAGE(2)
Showdeviceinfo()

goto LSuccess

'-----------------------------------------------------------------------
' SHOW DEVICE AND BOOTLOADER INFORMATION
'-----------------------------------------------------------------------

ShowInfo:

if ActivateTSB(Comport) <> 0 then goto LError
Showdeviceinfo()
goto LSuccess

'-----------------------------------------------------------------------
' LAUNCH EMERGENCY ERASE
'-----------------------------------------------------------------------

EmergencyErase:

close 8
sleep 100
if open com (Comport + Comset for binary as #8) > 0 then goto LError
sleep 100
print

print #8, "@@@";                        ' probe for normal TSB access

bstr = RXBuffer
if left(bstr,3) = "@@@" then           ' one-wire echo detected
    ONEWIRE = 1
    bstr = right(bstr, (len(bstr) - 3))
endif

print "WARNING: Emergency Erase deletes all APPFLASH and EEPROM contents,"
print "as well as Timeout and Password. This provides for a clean TSB "
print "that can be accessed with default values. (i.e. max. Timeout, no Password)"
print ""

if len (bstr) then                      ' received device data?
        print "Seems that TSB is still accessible without password."
        goto LSuccess

        else

        print "Do you REALLY want to do an Emergency Erase (Y/n) ";
endif

input ""; cstr
print
if cstr <> "Y" then print "Nothing changed!": goto LSuccess
SendCommand (chr(0))
if RXBuffer <> REQUEST then goto LError
SendCommand (CONFIRM)
'~ if RXBuffer <> REQUEST then goto LError
'~ SendCommand (CONFIRM)
print "TSB IS PERFORMING AN EMERGENCY ERASE.  Please stand by ... "
print
z = timer + 10 : do until loc(8) or (timer > z) : loop
if RXBuffer <> CONFIRM then goto LError
SendCommand ("c")
bstr = RXBuffer
if len(bstr) < 32 then goto LError
if right(bstr,1) <> CONFIRM then goto LError
i = 2
do until (asc(mid(bstr, i+1, 1)) < 255) or (i > 255)
    i = i + 1
loop
if (i mod 16) > 0 then goto LError  ' check for regular pagesizes
print "Emergency Erase successfull! TSB restored to defaults."
goto LSuccess

'-----------------------------------------------------------------------
' HELPSCREEN
'-----------------------------------------------------------------------
HelpScreen:
#ifdef __FB_WIN32__
print "-------------------------------------------------------------------------------"
print "Console Tool for TinySafeBoot, the tiny and safe AVR bootloader    SW:"; ISODATE()
print "-------------------------------------------------------------------------------"
print ""
print "Info:         tsb        Show this helpscreen"
print "              tsb -l     Show license note and version info"
print
print "SYNTAX 1:     CONNECT TO BOOTLOADER"
print ""
print "              tsb [Devicename[:Baudrate] {Mode} [Filename] {Specials} ]"
print "Devicename"
print "COMx          Device name of genuine or virtual serial port under Win/DOS."
print "[:Baudrate]   Specify standard baudrate for serial communications."
print "              With no baud specified, default of 9600 is chosen."
print "              Try slower baudrates for devices with low clock frequency."
print ""
print "{Mode}        Switches that specify the general mode of operation."
print "              Switches not case-sensitive. Prefixes '-' or '/' are optional."
print "ER EE EW EV   EEPROM Read / Erase / Write / Verify"
print "FR FE FW FV   FLASH  Read / Erase / Write / Verify"
print "I             Show bootloader and device info."
print "P             Enter Password changing dialog (TAKE CARE!)."
print "T             Change timeout factor (8...255, approx. 0.1 to some seconds)."
print "XXX           Emergency Erase - kill Userdata and restore Bootloader."
print ""
print "Filename      Filename (or full path) of source or target file."
print "              For write and verify, Intel(R) Hex is automatically recognized."
print "              With read operations, specifying extension '.hex' or '.eep' saves"
print "              the data in a Hex record, other extensions will save raw binary."
print ""
print "Examples:     tsb com1:19200 fw firmware.bin"
print "              tsb com2:38400 ew workingdata.eep"
print ""
print "{Specials}    Switches that enable special features."
print "              These may be attached to the command string in any sequence."
print "S             Silent mode. No screen output for most operations."
print ""
print "*** Apply RESET or POWER-UP on target device right before TSB session! ***"
print ""
print ""
print "SYNTAX 2:     MAKE CUSTOM TSB FIRMWARE"
print ""
print "              tsb tnxxxx {RxTx} [Filename]   or   tsb mxxxx {RxTx} [Filename]"
print "              Generate TSB for designated ATtiny/ATmega using specified I/O. "
print "              Firmware file will be Hex (.hex) or Binary (other extension)."
print ""
print "Examples:     tsb tn2313 d0d1 mytiny.hex"
print "              Generate TSB installer 'mytiny.hex' for the ATtiny2313"
print "              using Port D0=RXD and D1=TXD for serial communications."
print ""
print "              tsb m8515 e0e0"
print "              Generate TSB for ATmega8515 with bidirectional (One-Wire)"
print "              interface on port E0. Filename automatically chosen."
print ""
end
#endif
'-----------------------------------------------------------------------
#ifdef __FB_UNIX__
print "-------------------------------------------------------------------------------"
print "Console Tool for TinySafeBoot, the tiny and safe AVR bootloader    SW:"; ISODATE()
print "-------------------------------------------------------------------------------"
print ""
print "Info:         tsb        Show this helpscreen"
print "              tsb -l     Show license note and version info"
print
print "SYNTAX 1:     CONNECT TO BOOTLOADER"
print ""
print "              tsb [Devicename[:Baudrate] {Mode} [Filename] {Specials} ]"
print "Devicename"
print "/dev/ttySx    Device name for genuine serial ports under Linux."
print "/dev/ttyUSBx  Device name for USB ('virtual') serial ports under Linux."
print "[:Baudrate]   Specify standard baudrate for serial communications."
print "              With no baudrate specified, default of 9600 is chosen."
print "              Try lower baudrates for devices with low clock frequency."
print ""
print "{Mode}        Switches that specify operation mode and target."
print "              Switches not case-sensitive. Prefixes '-' or '/' are optional."
print "ER EE EW EV   EEPROM Read / Erase / Write / Verify"
print "FR FE FW FV   FLASH  Read / Erase / Write / Verify"
print "I             Show bootloader and device info."
print "P             Enter Password changing dialog (TAKE CARE!)."
print "T             Change timeout factor (8...255, approx. 0.1 up to many seconds)."
print "XXX           Emergency Erase - kill Userdata and restore Bootloader."
print ""
print "Filename      Filename (or full path) of source or target file."
print "              For write and verify, Intel(R) Hex is automatically recognized."
print "              With read operations, specifying extension '.hex' or '.eep' saves"
print "              the data in a Hex record, other extensions will save raw binary."
print ""
print "Examples:     ./tsb /dev/ttyS0:19200 fw firmware.hex"
print "              ./tsb /dev/ttyUSB0:19200 ew eeprom.eep"
print ""
print "{Specials}    Switches that enable special features."
print "              These may be attached to the command string in any sequence."
print "S             Silent mode. No screen output for most operations."
print ""
print "*** Apply RESET or POWER-UP on target device right before TSB session! ***"
print ""
print ""
print "SYNTAX 2:     MAKE CUSTOM TSB FIRMWARE"
print ""
print "              tsb tnxxxx {RxTx} [Filename]   or   tsb mxxxx {RxTx} [Filename]"
print "              Generate TSB for designated ATtiny/ATmega using specified I/O. "
print "              Firmware file will be Hex (.hex) or Binary (other extension)."
print ""
print "Examples:     tsb tn2313 d0d1 mytiny.hex"
print "              Generate TSB installer 'mytiny.hex' for the ATtiny2313"
print "              using Port D0=RXD and D1=TXD for serial communications."
print ""
print "              tsb m8515 e0e0"
print "              Generate TSB for ATmega8515 with bidirectional (One-Wire)"
print "              interface on port E0. Filename automatically chosen."
print ""
end
#endif

Licensenote:
print
print "TSB - Console Tool for TinySafeBoot, the tiny and safe AVR bootloader"
print "Copyright (C) Julien Thomas <joytec@gmx.de>"
print ""
print "This program is free software; you can redistribute it and/or modify it"
print "under the terms of the GNU General Public License as published"
print "by the Free Software Foundation; either version 3 of the License"
print "or (at your option) any later version."
print "This program is distributed in the hope that it will be useful,"
print "but WITHOUT ANY WARRANTY; without even the implied warranty"
print "of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
print "See the GNU General Public License for more details."
print "You should have received a copy of the GNU General Public License"
print "along with this program; if not, see <http://www.gnu.org/licenses/>."
print ""
print "Compiled on " & __DATE_ISO__ " with " & __FB_SIGNATURE__ " for ";
#ifdef __FB_WIN32__
print "Win32"
#endif
#ifdef __FB_UNIX__
print "Linux"
#endif
#ifdef __FB_DOS__
print "DOS"
#endif
print
end

'-----------------------------------------------------------------------
' LEAVE PROGRAM WITH OR WITHOUT ERROR (RETURN ERROR FLAG)
'-----------------------------------------------------------------------

LError:
print #99, ""
print #99, "ERROR."
print #99, ""
close
end (1)

'-----------------------------------------------------------------------

LSuccess:
DeactivateTSB
print #99, ""
close
end (0)

'-----------------------------------------------------------------------
' DEVICE SIGNATURES AND PORT DEFINITIONS
'-----------------------------------------------------------------------
'
data "begin devices"
'                  SIG0  SIG1  SIG2  PINA  PINB  PINC  PIND  PINE  PINF  PING
data "1200"      , &h1E, &h90, &h01, &hFF, &h16, &hFF, &h10, &hFF, &hFF, &hFF
data "2313"      , &h1E, &h91, &h01, &hFF, &h16, &hFF, &h10, &hFF, &hFF, &hFF
data "2323"      , &h1E, &h91, &h02, &hFF, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "2343"      , &h1E, &h91, &h03, &hFF, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "4414"      , &h1E, &h92, &h01, &h19, &h16, &h13, &h10, &hFF, &hFF, &hFF
data "4433"      , &h1E, &h92, &h03, &hFF, &h16, &h13, &h10, &hFF, &hFF, &hFF
data "4434"      , &h1E, &h93, &h03, &h19, &h16, &h13, &h10, &hFF, &hFF, &hFF
data "8515"      , &h1E, &h93, &h01, &h19, &h16, &h13, &h10, &hFF, &hFF, &hFF
data "8535"      , &h1E, &h93, &h03, &h19, &h16, &h13, &h10, &hFF, &hFF, &hFF
data "m1280"     , &h1E, &h97, &h03, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m1281"     , &h1E, &h97, &h04, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m1284"     , &h1E, &h97, &h06, &h00, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m1284P"    , &h1E, &h97, &h05, &h00, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m1284RFR2" , &h1E, &hA7, &h03, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m128A"     , &h1E, &h97, &h02, &h19, &h16, &h13, &h10, &h01, &h00, &hFF
data "m128"      , &h1E, &h97, &h02, &h19, &h16, &h13, &h10, &h01, &h00, &hFF
data "m128RFA1"  , &h1E, &hA7, &h01, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m128RFR2"  , &h1E, &hA7, &h02, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m162"      , &h1E, &h94, &h04, &h19, &h16, &h13, &h10, &h05, &hFF, &hFF
data "m164A"     , &h1E, &h94, &h0F, &h00, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m164PA"    , &h1E, &h94, &h0A, &h00, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m164P"     , &h1E, &h94, &h0A, &h00, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m165A"     , &h1E, &h94, &h10, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m165PA"    , &h1E, &h94, &h07, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m165P"     , &h1E, &h94, &h07, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m168A"     , &h1E, &h94, &h06, &hFF, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m168"      , &h1E, &h94, &h06, &hFF, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m168PA"    , &h1E, &h94, &h0B, &hFF, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m168P"     , &h1E, &h94, &h0B, &hFF, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m169A"     , &h1E, &h94, &h11, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m169PA"    , &h1E, &h94, &h05, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m169P"     , &h1E, &h94, &h05, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m16A"      , &h1E, &h94, &h03, &h19, &h16, &h13, &h10, &hFF, &hFF, &hFF
data "m16"       , &h1E, &h94, &h03, &h19, &h16, &h13, &h10, &hFF, &hFF, &hFF
data "m16HVA"    , &h1E, &h94, &h0C, &h00, &h03, &h06, &hFF, &hFF, &hFF, &hFF
data "m16HVB"    , &h1E, &h94, &h0D, &h00, &h03, &h06, &hFF, &hFF, &hFF, &hFF
data "m16M1"     , &h1E, &h94, &h84, &hFF, &h03, &h06, &h09, &h0C, &hFF, &hFF
data "m16U2"     , &h1E, &h94, &h89, &hFF, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m16U4"     , &h1E, &h94, &h88, &hFF, &h03, &h06, &h09, &h0C, &h0F, &hFF
data "m2560"     , &h1E, &h98, &h01, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m2561"     , &h1E, &h98, &h02, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m2564RFR2" , &h1E, &hA8, &h03, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m256RFR2"  , &h1E, &hA8, &h02, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m324A"     , &h1E, &h95, &h15, &h00, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m324PA"    , &h1E, &h95, &h11, &h00, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m324P"     , &h1E, &h95, &h08, &h00, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m3250A"    , &h1E, &h95, &h0E, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m3250"     , &h1E, &h95, &h06, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m3250PA"   , &h1E, &h95, &h0E, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m3250P"    , &h1E, &h95, &h0E, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m325A"     , &h1E, &h95, &h05, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m325"      , &h1E, &h95, &h05, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m325PA"    , &h1E, &h95, &h05, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m325P"     , &h1E, &h95, &h0D, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m328"      , &h1E, &h95, &h14, &hFF, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m328P"     , &h1E, &h95, &h0F, &hFF, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m3290A"    , &h1E, &h95, &h0C, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m3290"     , &h1E, &h95, &h04, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m3290PA"   , &h1E, &h95, &h0C, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m3290P"    , &h1E, &h95, &h0C, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m329A"     , &h1E, &h95, &h03, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m329"      , &h1E, &h95, &h03, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m329PA"    , &h1E, &h95, &h0B, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m329P"     , &h1E, &h95, &h0B, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m32A"      , &h1E, &h95, &h02, &h19, &h16, &h13, &h10, &hFF, &hFF, &hFF
data "m32C1"     , &h1E, &h95, &h86, &hFF, &h03, &h06, &h09, &h0C, &hFF, &hFF
data "m32"       , &h1E, &h95, &h02, &h19, &h16, &h13, &h10, &hFF, &hFF, &hFF
data "m32HVB"    , &h1E, &h95, &h10, &h00, &h03, &h06, &hFF, &hFF, &hFF, &hFF
data "m32HVBrevB", &h1E, &h95, &h10, &h00, &h03, &h06, &hFF, &hFF, &hFF, &hFF
data "m32M1"     , &h1E, &h95, &h84, &hFF, &h03, &h06, &h09, &h0C, &hFF, &hFF
data "m32U2"     , &h1E, &h95, &h8A, &hFF, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m32U4"     , &h1E, &h95, &h87, &hFF, &h03, &h06, &h09, &h0C, &h0F, &hFF
data "m406"      , &h1E, &h95, &h07, &h00, &h03, &hFF, &h09, &hFF, &hFF, &hFF
data "m48A"      , &h1E, &h92, &h05, &hFF, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m48"       , &h1E, &h92, &h05, &hFF, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m48PA"     , &h1E, &h92, &h0A, &hFF, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m48P"      , &h1E, &h92, &h0A, &hFF, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m640"      , &h1E, &h96, &h08, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m644A"     , &h1E, &h96, &h09, &h00, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m644"      , &h1E, &h96, &h09, &h00, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m644PA"    , &h1E, &h96, &h0A, &h00, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m644P"     , &h1E, &h96, &h0A, &h00, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m644RFR2"  , &h1E, &hA6, &h03, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m6450A"    , &h1E, &h96, &h06, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m6450"     , &h1E, &h96, &h06, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m6450P"    , &h1E, &h96, &h06, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m645A"     , &h1E, &h96, &h05, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m645"      , &h1E, &h96, &h05, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m645P"     , &h1E, &h96, &h05, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m6490A"    , &h1E, &h96, &h04, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m6490"     , &h1E, &h96, &h04, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m6490P"    , &h1E, &h96, &h04, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m649A"     , &h1E, &h96, &h03, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m649"      , &h1E, &h96, &h03, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m649P"     , &h1E, &h96, &h0B, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m64A"      , &h1E, &h96, &h02, &h19, &h16, &h13, &h10, &h01, &h00, &hFF
data "m64C1"     , &h1E, &h96, &h86, &hFF, &h03, &h06, &h09, &h0C, &hFF, &hFF
data "m64"       , &h1E, &h96, &h02, &h19, &h16, &h13, &h10, &h01, &h00, &hFF
data "m64HVE2"   , &h1E, &h96, &h10, &h00, &h03, &hFF, &hFF, &hFF, &hFF, &hFF
data "m64M1"     , &h1E, &h96, &h84, &hFF, &h03, &h06, &h09, &h0C, &hFF, &hFF
data "m64RFR2"   , &h1E, &hA6, &h02, &h00, &h03, &h06, &h09, &h0C, &h0F, &h12
data "m8515"     , &h1E, &h93, &h06, &h19, &h16, &h13, &h10, &h05, &hFF, &hFF
data "m8535"     , &h1E, &h93, &h08, &h19, &h16, &h13, &h10, &hFF, &hFF, &hFF
data "m88A"      , &h1E, &h93, &h0A, &hFF, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m88"       , &h1E, &h93, &h0A, &hFF, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m88PA"     , &h1E, &h93, &h0F, &hFF, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m88P"      , &h1E, &h93, &h0F, &hFF, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "m8A"       , &h1E, &h93, &h07, &hFF, &h16, &h13, &h10, &hFF, &hFF, &hFF
data "m8"        , &h1E, &h93, &h07, &hFF, &h16, &h13, &h10, &hFF, &hFF, &hFF
data "m8HVA"     , &h1E, &h93, &h10, &h00, &h03, &h06, &hFF, &hFF, &hFF, &hFF
data "m8U2"      , &h1E, &h93, &h89, &hFF, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "tn10"      , &h1E, &h90, &h03, &hFF, &h00, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn13A"     , &h1E, &h90, &h07, &hFF, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn13"      , &h1E, &h90, &h07, &hFF, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn1634"    , &h1E, &h94, &h12, &h0F, &h0B, &h07, &hFF, &hFF, &hFF, &hFF
data "tn167"     , &h1E, &h94, &h87, &h00, &h03, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn20"      , &h1E, &h91, &h0F, &h00, &h04, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn2313A"   , &h1E, &h91, &h0A, &h19, &h16, &hFF, &h10, &hFF, &hFF, &hFF
data "tn2313"    , &h1E, &h91, &h0A, &h19, &h16, &hFF, &h10, &hFF, &hFF, &hFF
data "tn24A"     , &h1E, &h91, &h0B, &h19, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn24"      , &h1E, &h91, &h0B, &h19, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn25"      , &h1E, &h91, &h08, &hFF, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn261A"    , &h1E, &h91, &h0C, &h19, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn261"     , &h1E, &h91, &h0C, &h19, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn26"      , &h1E, &h91, &h09, &h19, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn28"      , &h1E, &h91, &h07, &h19, &h16, &hFF, &h10, &hFF, &hFF, &hFF
data "tn40"      , &h1E, &h92, &h0E, &h00, &h04, &h1B, &hFF, &hFF, &hFF, &hFF
data "tn4313"    , &h1E, &h92, &h0D, &h19, &h16, &hFF, &h10, &hFF, &hFF, &hFF
data "tn43U"     , &h1E, &h92, &h0C, &h19, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn441"     , &h1E, &h92, &h15, &h19, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn44A"     , &h1E, &h92, &h07, &h19, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn44"      , &h1E, &h92, &h07, &h19, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn45"      , &h1E, &h92, &h06, &hFF, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn461A"    , &h1E, &h92, &h08, &h19, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn461"     , &h1E, &h92, &h08, &h19, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn48"      , &h1E, &h92, &h09, &h0C, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "tn4"       , &h1E, &h8F, &h0A, &hFF, &h00, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn5"       , &h1E, &h8F, &h09, &hFF, &h00, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn828"     , &h1E, &h93, &h14, &h00, &h04, &h08, &h0C, &hFF, &hFF, &hFF
data "tn841"     , &h1E, &h93, &h15, &h19, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn84A"     , &h1E, &h93, &h0C, &h19, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn84"      , &h1E, &h93, &h0C, &h19, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn85"      , &h1E, &h93, &h0B, &hFF, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn861A"    , &h1E, &h93, &h0D, &h19, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn861"     , &h1E, &h93, &h0D, &h19, &h16, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn87"      , &h1E, &h93, &h87, &h00, &h03, &hFF, &hFF, &hFF, &hFF, &hFF
data "tn88"      , &h1E, &h93, &h11, &h0C, &h03, &h06, &h09, &hFF, &hFF, &hFF
data "tn9"       , &h1E, &h90, &h08, &hFF, &h00, &hFF, &hFF, &hFF, &hFF, &hFF
data "end devices"
'
'-----------------------------------------------------------------------
' INCLUDE DATA-BASE OF PRE-ASSEMBLED TSB-TEMPLATES FOR SUPPORTED DEVICES
'-----------------------------------------------------------------------
'
data "begin templates"
#include "datas.bas"
data "end templates"
'
'-----------------------------------------------------------------------
'-----------------------------------------------------------------------
'-----------------------------------------------------------------------
' END BASIC CODE FOR TINYSAFEBOOT CONSOLE TOOL
'-----------------------------------------------------------------------
'-----------------------------------------------------------------------
'-----------------------------------------------------------------------
