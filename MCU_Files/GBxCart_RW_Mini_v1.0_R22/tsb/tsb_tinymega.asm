;***********************************************************************
;***********************************************************************
;***********************************************************************
; TinySafeBoot - The Universal Bootloader for AVR ATtinys and ATmegas
;***********************************************************************
;***********************************************************************
;***********************************************************************
;
;-----------------------------------------------------------------------
; Written in 2011-2015 by Julien Thomas
;
; This program is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 3
; of the License, or (at your option) any later version.
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty
; of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
; See the GNU General Public License for more details.
; You should have received a copy of the GNU General Public License
; along with this program; if not, see:
; http://www.gnu.org/licenses/
;-----------------------------------------------------------------------
;
;
;
;***********************************************************************
; OVERVIEW
;***********************************************************************
;
; TSB assembly source is organized in 4 segments (approx. line numbers)
;
; ~   50 ... Global definitions
; ~  240 ... TSB Installer for ATtinys
; ~  470 ... TSB for ATtinys
; ~ 1180 ... TSB for ATmegas
;
;***********************************************************************
; ADJUSTMENTS FOR INDIVIDUAL ASSEMBLY
;***********************************************************************
;
; This Sourcecode is directly compatible to: AVRASM2, GAVRASM
;
.nolist
;
;-----------------------------------------------------------------------
; SPECIFY TARGET AVR
;-----------------------------------------------------------------------
;
; Comment in and provide def.inc file for target device
;
; [Examples]
;
;.include "tn2313def.inc"
;.include "tn85def.inc"
;.include "m8515def.inc"
;.include "m168def.inc"
;.include "m161def.inc"
;.include "m324Adef.inc"
;.include "m328Pdef.inc"
;.include "tn441def.inc"
;.include "tn167def.inc"
;.include "tn861def.inc"
;.include "tn841def.inc"
;.include "tn84def.inc"
;.include "m8def.inc"
;.include "m644PAdef.inc"
;.include "m644def.inc"
;.include "tn167def.inc"
;.include "tn25def.inc"
;
; [...]
;
;
.list
;
;-----------------------------------------------------------------------
; BUILD INFO
;-----------------------------------------------------------------------
; YY = Year - MM = Month - DD = Day
.set    YY      =       16
.set    MM      =       10
.set    DD      =       27
;
.set BUILDSTATE = $F0   ; version management option
;
;-----------------------------------------------------------------------
; TSB / TSB-INSTALLER SWITCH
;-----------------------------------------------------------------------
; 0 = Regular assembly to target address
; 1 = TSB-Installer (ATtinys only - parameter ignored with ATmegas)
;
.set    TSBINSTALLER = 1
;
;-----------------------------------------------------------------------
; PORTS
;-----------------------------------------------------------------------
;
; Important Note: B0/B1 are defaults for database templates
;
.equ    RXPORT  = PORTB
.equ    RXPIN   = PINB
.equ    RXDDR   = DDRB
.equ    RXBIT   = 0
.equ    TXPORT  = PORTB
.equ    TXDDR   = DDRB
.equ    TXBIT   = 1
;
; Testing
;~ .equ    RXPORT  = PORTA
;~ .equ    RXPIN   = PINA
;~ .equ    RXDDR   = DDRA
;~ .equ    RXBIT   = 0
;~ .equ    TXPORT  = PORTA
;~ .equ    TXDDR   = DDRA
;~ .equ    TXBIT   = 1

;~ .equ    RXPORT  = PORTD
;~ .equ    RXPIN   = PIND
;~ .equ    RXDDR   = DDRD
;~ .equ    RXBIT   = 0
;~ .equ    TXPORT  = PORTD
;~ .equ    TXDDR   = DDRD
;~ .equ    TXBIT   = 1

;-----------------------------------------------------------------------
; *** Changes below this line are on your own risk! ***
;-----------------------------------------------------------------------
;
;
;
;***********************************************************************
; AUTO-ADJUST FOR DIFFERENT ASSEMBLY OPTIONS
;***********************************************************************
;
; Autodetect ATtiny / ATmega and set TINYMEGA switch accordingly

.ifdef RWW_START_ADDR
        .if RWW_START_ADDR == RWW_STOP_ADDR
        .equ TINYMEGA = 0
        .message "DETECTED ATTINY DEFINITIONS"
        .else
        .equ TINYMEGA = 1
        .message "DETECTED ATMEGA DEFINITIONS"
        .endif
.else
        .equ TINYMEGA = 0
        .message "DETECTED ATTINY DEFINITIONS"
.endif

.if FLASHEND > ($7fff)
        .error "SORRY! DEVICES OVER 64 KB NOT SUPPORTED YET."
        .exit
.endif

;-----------------------------------------------------------------------
; Workarounds for devices with renamed or missing definitions
;-----------------------------------------------------------------------
;
.ifndef SPMCSR                  ; SPMEN / PGERS / ...
        .equ SPMCSR = SPMCR
.endif

.ifndef MCUSR                   ; PORF / EXTRF / BORF / WDRF
        .equ MCUSR = MCUCSR
.endif

; Detect Attiny441/841 to amend missing pagesize and apply 4-page mode

.set FOURPAGES = 0

.if ((SIGNATURE_000 == $1E) && (SIGNATURE_002 == $15) && (SIGNATURE_001 == $92))
                .equ PAGESIZE = 32
                .set FOURPAGES = 1
                .message "ATTINY441: 4-PAGE-ERASE MODE"
.endif

.if ((SIGNATURE_000 == $1E) && (SIGNATURE_002 == $15) && (SIGNATURE_001 == $93))
                .equ PAGESIZE = 32
                .set FOURPAGES = 1
                .message "ATTINY841: 4-PAGE-ERASE MODE"
.endif

;-----------------------------------------------------------------------
; Universal Constants and Registers
;-----------------------------------------------------------------------

.equ    REQUEST         = '?'           ; request / answer / go on
.equ    CONFIRM         = '!'           ; confirm / attention

; Current bootloader date coded into 16-bit number
.equ    BUILDDATE   = YY * 512 + MM * 32 + DD

; Other
.equ    INFOLEN         = 8              ; *Words* of Device Info
.equ    BUFFER          = SRAM_START

; Registers (in use by TSB-Firmware and TSB-Installer for ATtinys)
.def    avecl   = r4                    ; application vector temp low
.def    avech   = r5                    ; application vector temp high
.def    bclkl   = r6                    ; baudclock low byte
.def    bclkh   = r7                    ; baudclock high byte
.def    tmp1    = r16                   ; these are
.def    tmp2    = r17                   ; universal
.def    tmp3    = r18                   ; temporary
.def    tmp4    = r19                   ; registers
.def    bcnt    = r20                   ; page bytecounter

;-----------------------------------------------------------------------
; TSB for ATtinys
;-----------------------------------------------------------------------
; TSB for ATtinys may be build directly to its target address   or
; as an 'Installer' that can install new TSB from existing bootloader.
;-----------------------------------------------------------------------

.if TINYMEGA == 0       ; .endif  by the end of TSB for ATtinys

; TSB for ATtinys at target location
.equ    TSBLEN          = ((280 / PAGESIZE)+1) * PAGESIZE
.equ    BOOTSTART       = (FLASHEND+1) - PAGESIZE - TSBLEN
.equ    LASTPAGE        = BOOTSTART + TSBLEN
.equ    DEVICEINFO      = BOOTSTART + TSBLEN - INFOLEN

; TSB-Installer (ATtinys only)
.equ    TBOOTSTART      = $00C0
.equ    TLASTPAGE       = TBOOTSTART + TSBLEN
.equ    TDEVICEINFO     = TLASTPAGE - INFOLEN

;-----------------------------------------------------------------------

.if TSBINSTALLER == 1   ; .endif  by the end of TSB-Installer section

.message "ASSEMBLY OF TSB-INSTALLER FOR ATTINYS"

.set    CLASTPAGE = TLASTPAGE           ; for alignment check later on

.if FOURPAGES == 0
        .set RPAGESIZE = PAGESIZE
.else
        .set RPAGESIZE = PAGESIZE / 4   ; exemption for ATtiny 441/841
.endif

;***********************************************************************
; TSB-INSTALLER FOR ATTINYS
;***********************************************************************
; The 'TSB-Installer' may be uploaded like ordinary Firmware via ISP,
; but also by any bootloader mechanism that is residing on the ATtiny.
; First run, the Installer will check its own integrity, then remove
; the modified reset vector and erase old bootloader from upper memory.
; Then it will transfer the brought-along new TSB-Firmware to its target
; address space and finally remove itself from memory.
; NOTE: This update mechanism is applicable only to ATtinys.
;
; The process evolves in 5 levels:
; 1. check if the program was uploaded true and completely.
; 2. remove modified reset-jump and reinstate itself as reset target.
; 3. erase previous bootloader code.
; 4. write+verify new tsb to upper flash, retry on errors.
; 5. clean-up (self-erase) and handover to new bootloader.
;
;-----------------------------------------------------------------------
; Extra registers in use by TSB-Installer
;-----------------------------------------------------------------------

.def    pcnt    = r21                   ; page counter
.def    checkl  = r22                   ; checksum counters
.def    checkh  = r23
.def    tslo    = r8                    ; temp current source address
.def    tshi    = r9
.def    ttlo    = r10                   ; temp current target address
.def    tthi    = r11
.def    bflo    = r12                   ; start address of buffer
.def    bfhi    = r13

;-----------------------------------------------------------------------
; 1st PAGE: RESET-JUMP / JUMP TO LEVEL 1 (INTEGRITY CHECK)
;-----------------------------------------------------------------------

.org $0000

.if FLASHEND >= ($1fff)
        .message "Using JMP for Reset Vector"
        jmp  TL1CHECK
.else
        .message "Using RJMP for Reset Vector"
        rjmp TL1CHECK
.endif

.org RPAGESIZE-2

TL1CHECK:

        rjmp L1CHECK                    ; LEVEL 1 start with selftest
        ;rjmp TTRESET                   ; debugging: skip selftest

.db $00, $00                            ; placeholder for a checksum

;-----------------------------------------------------------------------
; 2nd PAGE: START OF TSB-INSTALLER CRITICAL CODE
;-----------------------------------------------------------------------

.org RPAGESIZE

TTRESET:
        cli
        ldi tmp1, low (RAMEND)          ; write ramend low to SPL
        out SPL, tmp1
        .ifdef SPH
        ldi tmp1, high(RAMEND)          ; write ramend high to SPH
        out SPH, tmp1                   ; when SRAM > 256 bytes
        .endif
        rjmp L2EraseFirstPage

;-----------------------------------------------------------------------
; LEVEL 1 - CHECK INTEGRITY OF TSB-INSTALLER
;-----------------------------------------------------------------------
; First of all we have to check integrity of the TSB-Installer at whole.
; Therefore a Checksum is calculated over all pages carrying relevant
; machine code of TSB-Installer and attached TSB-Firmware.
; If this value (16 bit) is equal to the checksum that is found in the
; first page (precalculated by TSB-SW in the course of making customized
; TSB-Installer), we're assured, that the Installer has been uploaded
; correctly and in whole and it is safe to start next level.
; Yet with checksum error, the Installer will never leave this level
; and the old Bootloader will stay in force.

L1CHECK:
        cli
        ldi zh, high((RPAGESIZE-1)*2)   ; checksum stored by TSB-SW
        ldi zl, low ((RPAGESIZE-1)*2)   ; in last two bytes of 1st page
        lpm checkh, z+                  ; load checksum high byte
        lpm checkl, z+                  ; load checksum low byte
        ldi pcnt,(TLASTPAGE/RPAGESIZE)-1 ; # of pages to check
L1C0:   ldi bcnt, low(RPAGESIZE*2)       ; # of bytes to check per page
L1C1:   lpm tmp1, z+                    ; load byte of TSB-FW
        sub checkl, tmp1                ; subtract byte value
        sbci checkh, 0                  ; from checksum word
        dec bcnt                        ; until full page checked
        brne L1C1                       ; loop on
        dec pcnt
        brne L1C0                       ; until all pages checked
L1C2:
        cpi checkl, 0                   ; if checksum was correct,
        brne L1C2                       ; both bytes will be zero
        cpi checkh, 0                   ; else we cannot continue
        brne L1C2                       ; and loop in here
        rjmp TTRESET

;-----------------------------------------------------------------------
; LEVEL 2 - REMOVE RESET-VECTOR TO OLD BOOTLOADER
;-----------------------------------------------------------------------
; After integrity of the TSB-Installer has been proven in Level 1,
; we can simply erase the whole 1st page, since it only contained the
; reset-jump to the old bootloader, the rjmp to the checksum routine
; and the checksum, which are no longer needed.
; Relevant machine code for the remaining tasks of TSB-Installer
; begins in 2nd page and will be started at the latest with some
; further reset or coldstart.

L2EraseFirstPage:
        clr zl                          ; set Z to $0000
        clr zh                          ; start of 1st page
        ldi tmp1, 0b00000011            ; enable PGERS + SPMEN
        out SPMCSR, tmp1                ; in SPMCSR and go -
        spm                             ; erase that 1st page!

;-----------------------------------------------------------------------
; LEVEL 3 - ERASE PREVIOUS BOOTLOADER
;-----------------------------------------------------------------------
; Erase flash memory that may contain old bootloader's code
; down from LASTPAGE/FLASHEND to TLASTPAGE.

        ldi zl, low (LASTPAGE*2)        ; reset Z to LASTPAGE's start
        ldi zh, high(LASTPAGE*2)
        ldi pcnt, low ((LASTPAGE - TLASTPAGE) / RPAGESIZE) + 1
L3EPB0:
        ldi tmp1, 0b00000011            ; enable PGERS + SPMEN
        out SPMCSR, tmp1                ; in SPMCSR and erase current
        spm                             ; page by SPM (MCU halted)
        subi zl, low (RPAGESIZE*2)      ; Z = Z - PAGESIZE * 2
        sbci zh, high(RPAGESIZE*2)
        dec pcnt
        brne L3EPB0

;-----------------------------------------------------------------------
; LEVEL 4 - WRITE/VERIFY NEW BOOTLOADER
;-----------------------------------------------------------------------
; Copy new bootloader to its final location in upper flash memory.
; If verifying should fail, it will try again with every next reset.
; Pages copied top-to-bottom to avoid overlap in small attinys < 1k.

L4WriteVerify:

        ldi tmp1, low  (TLASTPAGE*2)    ; starting address source
        ldi tmp2, high (TLASTPAGE*2)
        movw tslo, tmp1
        ldi tmp1, low  (LASTPAGE*2)     ; starting address target
        ldi tmp2, high (LASTPAGE*2)
        movw ttlo, tmp1
        ldi pcnt, (((LASTPAGE-BOOTSTART)/RPAGESIZE)+1)   ; # of pages

; load source page contents into buffer
L4WV0:
        ldi yl, low (BUFFER)            ; set y to start address
        ldi yh, high(BUFFER)            ; of buffer (in SRAM)
        movw bflo, yl                   ; buffer start to bfhi/bflo
        ldi bcnt, low (RPAGESIZE*2)
        movw zl, tslo                   ; load Z with source address
L4WV1:
        lpm tmp1, z+                    ; load flash byte
        st y+, tmp1                     ; and store in buffer
        dec bcnt                        ; until full page buffered
        brne L4WV1                      ; loop on
        subi zl, low (RPAGESIZE*4)      ; decrease by two pages
        sbci zh, high(RPAGESIZE*4)      ; to continue one page below
        movw tslo, zl                   ; save current Z

; transfer buffer to flash write buffer and write to target page
        movw yl, bflo                   ; restore y to buffer start
        ldi bcnt, low(RPAGESIZE*2)
        movw zl, ttlo                   ; load Z with target address
L4WV2:
        ld r0, y+                       ; fill R0/R1 with word
        ld r1, y+                       ; from buffer position Y
        ldi tmp1, 0b00000001            ; set only SPMEN
        out SPMCSR, tmp1                ; to activate page buffering
        spm                             ; store word in page buffer
        adiw zl, 2                      ; Z = Z + 2
        subi bcnt, 2                    ; bcnt=bcnt-2
        brne L4WV2
        subi zl, low (RPAGESIZE*2)      ; restore Z to start of current
        sbci zh, high(RPAGESIZE*2)      ; page for write and verify
        ldi tmp1, 0b00000101            ; enable PRWRT + SPMEN
        out SPMCSR, tmp1                ; in SPMCSR and
        spm                             ; write flash page from buffer
L4WV3:
        in tmp1, SPMCSR                 ; wait for flash write finished
        sbrc tmp1, 0                    ; go on if SPMEN cleared
        rjmp L4WV3                      ; otherwise wait along

; verify page just written, but stop everything in case of an error

L4WV4:  movw yl, bflo                   ; restore y to buffer start
        ldi bcnt, low (RPAGESIZE*2)
L4WV5:
        lpm tmp1, z+                    ; load byte from flash
        ld tmp2, y+                     ; load buffer byte
        cp tmp1, tmp2                   ; compare them
        breq L4WV7                      ; and loop on if equal

L4WV6:  rjmp L4WV6                      ; else - hangup!

L4WV7:  dec bcnt                        ; count down page bytecounter
        brne L4WV5                      ; loop until all bytes checked
        subi zl, low (RPAGESIZE*4)      ; decrease by two pages
        sbci zh, high(RPAGESIZE*4)      ; to continue one page below
        movw ttlo, zl                   ; save z to target pointer
L4WV8:
        dec pcnt                        ; loop on with write and verify
        brne L4WV0                      ; until all pages transferred
L4WVx:

;-----------------------------------------------------------------------
; LEVEL 5 - HANDOVER TO NEW TSB AND CLEAN-UP FLASH
;-----------------------------------------------------------------------
; Now jump to the absolute address of 'EraseAppFlash' in new TSB.
; This will remove all installer code from memory and complete
; the installation by leaving new TSB with cleaned up AppFlash.

L5EraseInstaller:
        ldi tmp1, low (EraseAppFlash+(BOOTSTART-TBOOTSTART))
        ldi tmp2, high(EraseAppFlash+(BOOTSTART-TBOOTSTART))
        push tmp1                       ; push lowbyte on stack
        push tmp2                       ; push highbyte on stack
        ret                             ; = improvised absolute jump!

;***********************************************************************
; END OF TSB-INSTALLER
;***********************************************************************

.endif          ; from the if-part launching TSB-INSTALLER


;***********************************************************************
;***********************************************************************
;***********************************************************************
; START OF REGULAR TSB CODE FOR ATTINYS
;***********************************************************************
;***********************************************************************
;***********************************************************************
; NOTE: The programming of TSB is largely relocatable which allows it
; being assembled inline with the Installer-routines.
; Only the absolute addresses of LASTPAGE, BOOTSTART and DEVICEINFO
; must be hard coded with respect to the bootloader's target location.

; Make the following code a TSB-Installer-piggyback
.if TSBINSTALLER == 1
.org TBOOTSTART
.endif

; Make the following code a TSB for target address
.if TSBINSTALLER == 0
.org BOOTSTART
.message "ASSEMBLY OF TSB FOR ATTINYS TO REGULAR ADDRESS RANGE"
.set CLASTPAGE = LASTPAGE
.endif

RESET:
        cli

        in tmp4, MCUSR                  ; check reset condition
        sbrc tmp4, WDRF                 ; in case of a Watchdog reset
        rjmp APPJUMP                    ; immediately leave TSB

        ldi tmp1, low (RAMEND)          ; write ramend low
        out SPL, tmp1                   ; into SPL (stackpointer low)
        .ifdef SPH
        ldi tmp1, high(RAMEND)          ; write ramend high for ATtinys
        out SPH, tmp1                   ; with SRAM > 256 bytes
        .message "PROVIDING FOR STACK BIGGER THAN 256 BYTES"
        .endif

;-----------------------------------------------------------------------
; ACTIVATION CHECK
;-----------------------------------------------------------------------
; Configure RX/TX Ports, wait for stable conditions when coldstart
; and startbit of incoming auto-calibration character, else Timeout

WaitRX:
        sbi TXDDR, TXBIT                ; if RX=TX (One-Wire), port is
        cbi RXDDR, RXBIT                ; driven open collector style,
        sbi RXPORT, RXBIT               ; else RX is input with pullup
        sbi TXPORT, TXBIT               ; and TX preset logical High

; power-up or brown-out resets - wait for RX to stabilize
WRX0To:
        sbic RXPIN, RXBIT               ; loop while RX = 0
        rjmp WRXSTo                     ; if RX = 1, start COM Timeout
        sbrc tmp4, EXTRF                ; in warmstart (external reset)
        rjmp APPJUMP                    ; we may skip this
        sbiw xl, 1                      ; use X for Timeout counter
        brcc WRX0To                     ; if timed out with 0 level
        rjmp APPJUMP                    ; goto APPJUMP in LASTPAGE
;~ ;~
WRXSTo:                                 ; else
        rcall ZtoLASTPAGE               ; set Z to start'o'LASTPAGE
        adiw zl, 2                      ; skip first 2 bytes (APPJUMP)
        lpm xh, z+                      ; load TIMEOUT byte
        ldi xl, 128                     ; least delay
WRX1To:
        dec tmp1                        ; inner delay counter
        brne WRX1To                     ; also debouncing/denoising
        sbis RXPIN, RXBIT               ; if RX=0 (startbit) occurs
        rjmp Activate                   ; go Activate
        sbiw xl, 1                      ; use X for down counter
        brcc WRX1To                     ; to Timeout
        rjmp APPJUMP                    ; goto APPJUMP in LASTPAGE



;-----------------------------------------------------------------------
; BAUDRATE CALIBRATION CYCLE
;-----------------------------------------------------------------------
; Calibrate RS232-timing by leading autobaud characters ('@@@')
; starting with definite HIGH level, all LOW levels may be count up
;                databits
;    startbit > s12345678S < stopbit
;    1st byte : 0000000101   ALL 0-bits being measured
; followed by : 0000000101   in the three characters
; followed by : 0000000101   for good calibration
; ... summing up to *24* bitcells providing good timing reference
;
Activate:
        clr xl                          ; clear temporary
        clr xh                          ; baudrate counter
        ldi tmp1, 5                     ; number of expected bit-changes
actw0:
        sbis RXPIN, RXBIT               ; discard first 0-state
        rjmp actw0
actw1:
        sbic RXPIN, RXBIT               ; idle 1-states (stopbits, ones)
        rjmp actw1                      ; with NO Timeout
actw2:
        adiw xl, 1                      ; precision measuring loop
        sbis RXPIN, RXBIT               ; count clock cycles in 0-state
        rjmp actw2

        sbiw xl, 10
        dec tmp1
        brne actw1
actwx:
        movw bclkl, xl                  ; save result to bclk

;-----------------------------------------------------------------------
; CHECK PASSWORD
;-----------------------------------------------------------------------
; Check Password, if existing. No feedback whatsoever while characters
; are being received. Silent hangup with wrong characters submit.
; After full and correct password has been entered, send device info.
; Detect request for and execute an Emergency Erase.
; uses: tmp1, tmp3, tmp4, Z (must point to LASTPAGE+3 at the beginning)

CheckPassword:

chpw0:  ser tmp4                        ; tmp4 = 255 enables comparison
chpw1:  lpm tmp3, z+                    ; load pw character from Z
        and tmp3, tmp4                  ; tmp4 = 0 disables comparison
        cpi tmp3, 255                   ; byte value 255 indicates
        breq chpwx                      ; end of password -> success
chpw2:  rcall Receivebyte               ; else receive next character
        cpi tmp1, 0                     ; rxbyte = 0 will branch
        breq chpwee                     ; to confirm emergency erase
        cp  tmp1, tmp3                  ; compare password with rxbyte
        breq chpw0                      ; if equal check next character
        clr  tmp4                       ; tmp4 = 0 to loop forever
        rjmp chpw1                      ; and smoothen power profile
chpwee:
        rcall RequestConfirm            ; request confirm
        brts chpa                       ; not confirmed, leave
        ;~ rcall RequestConfirm            ; request 2nd confirm
        ;~ brts chpa                       ; can't be mistake now
        rcall EmergencyErase            ; go, emergency erase!
        rjmp  Mainloop
chpa:
        rjmp APPJUMP                    ; start application
chpwx:

;-----------------------------------------------------------------------
; SEND DEVICEINFO
;-----------------------------------------------------------------------

SendDeviceInfo:
        ldi zl, low (DEVICEINFO*2)      ; load address of deviceinfo
        ldi zh, high(DEVICEINFO*2)      ; low and highbyte
        ldi bcnt, INFOLEN*2
        rcall SendFromFlash

;-----------------------------------------------------------------------
; MAIN LOOP TO RECEIVE AND EXECUTE COMMANDS
;-----------------------------------------------------------------------

Mainloop:
        clr zl                          ; clear Z pointer
        clr zh                          ; which is frequently used
        rcall SendConfirm               ; send CONFIRM via RS232
        rcall Receivebyte               ; receive command via RS232
        rcall CheckCommands             ; check command letter
        rjmp Mainloop                   ; and loop on

;-----------------------------------------------------------------------
; CHANGE USER DATA IN LASTPAGE
;-----------------------------------------------------------------------
; Receive page data and write into LASTPAGE
; uses: Y, Z, r0, r1, tmp1, bcnt

ChangeSettings:
        rcall GetNewPage                ; get new LASTPAGE contents
        brtc ChangeS0                   ; from Host (if confirmed)
        ret
ChangeS0:
        rcall ZtoLASTPAGE               ; re-write LASTPAGE
        set                             ; T=1 to disable internal verify
        rcall WritePageAfterErase       ; erase and write LASTPAGE

;-----------------------------------------------------------------------
; SEND USER DATA FROM LASTPAGE
;-----------------------------------------------------------------------
; uses: bcnt

CheckSettings:
        rcall ZtoLASTPAGE               ; point to LASTPAGE
;       rjmp SendPageFromFlash          ; send LASTPAGE over RS232

;-----------------------------------------------------------------------
; SEND DATA FROM FLASH MEMORY
;-----------------------------------------------------------------------
; Send bcnt number of flash memory bytes, count up Z accordingly
; uses: tmp1, bcnt, Z

SendPageFromFlash:
        ldi bcnt, low (PAGESIZE*2)
SendFromFlash:
        lpm tmp1, z+                    ; read directly from flash
        rcall Transmitbyte              ; and send out to RS232
        dec bcnt                        ; bcnt is number of bytes
        brne SendFromFlash
        ret

;-----------------------------------------------------------------------
; READ APPLICATION FLASH
;-----------------------------------------------------------------------
; read and transmit application flash area (pagewise)
; uses: Z (should be $0000 at the beginning), tmp1, bcnt

ReadAppFlash:
RAF0:   rcall RwaitConfirm
        brts RAFx
        rcall SendPageFromFlash
RAF1:   cpi zl, low (BOOTSTART*2)
        brne RAF0
        cpi zh, high(BOOTSTART*2)       ; count Z to end of flash
        brne RAF0
RAFx:
        ret

;-----------------------------------------------------------------------
; WRITE APPLICATION FLASH
;-----------------------------------------------------------------------
; Write Appflash pagewise, modify reset vector and RJMP APP

WriteAppFlash:
        rcall EraseAppFlash             ; Erase whole app flash
        rcall GetNewPage                ; get first page's data
        brtc Flash1                     ; continue on CONFIRM (T=0)
        ret                             ; abort with Flash erased (T=1)

Flash1:
        rcall YtoBUFFER

.if FLASHEND >= ($1fff)                 ; biggg ATtiny

.message "ATTINY OVER 16 KB USES JMP FOR MODIFIED RESET VECTOR"
        ldd avecl, y+2                  ; read address portion of JMP
        ldd avech, y+3                  ; directly into avecl/avech
        ldi tmp1, low (BOOTSTART)       ; generate new JMP target
        ldi tmp2, high(BOOTSTART)       ; (16 bits = byte 3+4)
        std y+2, tmp1                   ; write low/highbyte of changed
        std y+3, tmp2                   ; jump address into buffer

.else                                   ; small ATtiny

.message "ATTINY BELOW 16 KB USES RJMP FOR MODIFIED RESET VECTOR"
        ldd tmp1, y+0                   ; read bytes from original rjmp
        ldd tmp2, y+1                   ; into tmp1/tmp2
        subi tmp1, low (-(4096-LASTPAGE))      ; calculate
        sbci tmp2, high(-(4096-LASTPAGE))      ; rjmp APPSTART
        movw avecl, tmp1                ; save for later reference
        ldi tmp1, low (BOOTSTART-1 + $C000)     ; generate new opcode
        ldi tmp2, high(BOOTSTART-1 + $C000)     ; for rjmp BOOTSTART
        std y+0, tmp1                   ; replace rjmp in buffer
        std y+1, tmp2                   ; with rjmp BOOTSTART
.endif

        rcall WritePage                 ; write modified first page
Flash2:
        rcall GetNewPage                ; get next page to write
        brts FlashX                     ; stop on user's behalf
Flash3:
        rcall WritePage                 ; write page data into flash
Flash4:
        cpi zh, high(BOOTSTART*2-1)     ; check for end of Appflash
        brne Flash2                     ; if Z reached last location
        cpi zl, low (BOOTSTART*2-1)     ; then we are finished
        brne Flash2                     ; else go on
FlashX:
        rcall ReadLastPage              ; finally update LASTPAGE
        std y+0, avecl                  ; change first opcode
        std y+1, avech                  ; to jump target, then go
        rjmp WritePageafterErase

;-----------------------------------------------------------------------
; CHECK COMMANDS
;-----------------------------------------------------------------------
; Placed 'in between' to reach all subs by direct branches :-)

CheckCommands:
        cpi tmp1, 'c'                   ; read LASTPAGE
        breq CheckSettings
        cpi tmp1, 'C'                   ; write LASTPAGE
        breq ChangeSettings
        cpi tmp1, 'f'                   ; read Appflash
        breq ReadAppFlash
        cpi tmp1, 'F'                   ; write Appflash
        breq WriteAppFlash
        cpi tmp1, 'e'                   ; read EEPROM
        breq EEpromRead
        cpi tmp1, 'E'                   ; write EEPROM
        breq EEpromWrite
        rjmp APPJUMP                    ; else start application

;-----------------------------------------------------------------------
; EEPROM READ/WRITE ACCESS
;-----------------------------------------------------------------------
; Read or write full EEPROM address space
; With blocksize = PAGESIZE * 2 (since PAGESIZE is defined in WORDS)
; uses: Y, Z, tmp1, tmp3

EEpromRead:
EeRe1:
        rcall RwaitConfirm              ; wait to confirm
        brts EERWFx                     ; else we are finished
        ldi bcnt, low(PAGESIZE*2)       ; again PAGESIZE*2 is blocksize
EERe2:
        out EEARL, zl                   ; current EEPROM address low
.ifdef  EEARH
        out EEARH, zh                   ; current EEPROM address high
.endif
        sbi EECR, 0                     ; set EERE - EEPROM read enable
        in tmp1, EEDR                   ; read byte from current address
        rcall Transmitbyte              ; send out to RS232
        adiw zl,1                       ; count up EEPROM address
        dec bcnt                        ; count down block byte counter
        brne EERe2                      ; loop on if block not finished
        rjmp EERe1

;-----------------------------------------------------------------------

EEpromWrite:
EEWr0:
        rcall GetNewPage                ; get EEPROM datablock
        brts EERWFx                     ; or abort on host's demand
EEWr1:
        rcall YtoBUFFER                 ; Y = Buffer and Bcnt = blocksize
EEWr2:
        ld tmp1, y+                     ; read EEPROM byte from buffer
        rcall EEWriteByte
        dec bcnt                        ; count down block byte counter
        brne EEWr2                      ; loop on if block not finished
        rjmp EeWr0
EERWFx:
        ret

;-----------------------------------------------------------------------

EEWriteByte:
        out EEDR, tmp1                  ; write to EEPROM data register
        out EEARL, zl                   ; current EEPROM address low
.ifdef  EEARH
        out EEARH, zh                   ; high EEARH for some attinys
.endif
        sbi EECR, 2                     ; EEPROM master prog enable
        sbi EECR, 1                     ; EEPE initiate prog cycle
EeWB:
        sbic EECR, 1                    ; wait write cycle to complete
        rjmp EeWB                       ; before we can go on
        adiw zl,1                       ; count up EEPROM address
        ret

;-----------------------------------------------------------------------
; READ LASTPAGE INTO BUFFER
;-----------------------------------------------------------------------
; Page Address Z, SRAM buffer pointer Y, repoint Z to LASTPAGE

ReadLastPage:
        rcall ZtoLASTPAGE               ; reset Z to LASTPAGE start
        rcall YtoBUFFER                 ; reset Y to BUFFER start
ReLaPa:
        lpm tmp1, z+                    ; read byte from flash memory
        st y+, tmp1                     ; store in SRAM via Y pointer
        dec bcnt
        brne ReLaPa
ReLax:
        rcall ZtoLASTPAGE               ; reset Z to LASTPAGE start
        rjmp  YtoBUFFER                 ; reset Y to BUFFER start

;-----------------------------------------------------------------------
; GET NEW PAGE
;-----------------------------------------------------------------------
; Read page data into SRAM buffer
; uses: X, tmp1, bcnt, SRAM

GetNewPage:
        rcall RequestConfirm            ; check for Confirm
        brts GNPx                       ; abort if not confirmed
GNP0:
        rcall YtoBUFFER                 ; Y = BUFFER, bcnt = PAGESIZE*2
GNP1:
        rcall ReceiveByte               ; receive serial byte
        st y+, tmp1                     ; and store in buffer
        dec bcnt                        ; until full page loaded
        brne GNP1                       ; loop on
GNPx:
        ret                             ; finished

;-----------------------------------------------------------------------
; REQUEST TO CONFIRM / AWAIT CONFIRM
;-----------------------------------------------------------------------
; Send REQUEST and wait answer from Host (received character in tmp1)
; T=0: CONFIRM / T=1: NOT CONFIRM (any other char)

RequestConfirm:
        ldi tmp1, REQUEST               ; send request character
        rcall Transmitbyte              ; prompt to confirm (or not)
RwaitConfirm:
        rcall ReceiveByte               ; get host's reply
        clt                             ; set T=0 for confirm
        cpi tmp1, CONFIRM               ; if host HAS sent CONFIRM
        breq RCx                        ; return with the T=0
        set                             ; else set T=1 (NOT CONFIRMED)
RCx:    ret                             ; whether confirmed or not

;-----------------------------------------------------------------------
; WRITE FLASH PAGES FROM BUFFER, VERIFYING AND VERIFY-ERROR-HANDLING
;-----------------------------------------------------------------------
; uses: r0, r1, Y, Z, tmp1, tmp2, bcnt
; Z must point to start address of page to be written.
; Z is left at the start address of next page.
; T=0 for internal verifying and EraseAppFlash on any verifying error
; T=1 to skip verifying and ignore errors

.if FOURPAGES == 0      ; assembly option for normal page mode

WritePageafterErase:
        rcall EraseFlashPage
WritePage:
        rcall YtoBUFFER                 ; Y=BUFFER, bcnt=PAGESIZE*2
WrPa1:
        ld r0, y+                       ; fill R0/R1 with word
        ld r1, y+                       ; from buffer position Y / Y+1
        ldi tmp1, 0b00000001            ; set only SPMEN in SPMCSR
        out SPMCSR, tmp1                ; to activate page buffering
        spm                             ; store word in page buffer
        adiw zl, 2                      ; and forward to next word
        subi bcnt, 2
        brne WrPa1
        ; Z = start of next page now
        subi zl, low (PAGESIZE*2)       ; point back Z to
        sbci zh, high(PAGESIZE*2)       ; start of current page
        ; Z = back on current page's start
WrPa2:
        ldi tmp1, 0b00000101            ; enable PRWRT + SPMEN
        out SPMCSR, tmp1                ; in SPMCSR
        spm                             ; write whole page to flash
WrPa3:
        in tmp1, SPMCSR                 ; wait for flash write finished
        sbrc tmp1, 0                    ; skip if SPMEN (bit0) cleared
        rjmp WrPa3                      ; ITS BEEN WRITTEN

;-----------------------------------------------------------------------
; INTERNAL VERIFY AGAINST BUFFER CONTENTS
;-----------------------------------------------------------------------
; If verify of any page in AppFlash has FAILED, the program code and/or
; reset-vector may be corrupted - danger of bootloader lock-out!
; It is better then to IMMEDIATELY ERASE ALL APPLICATION FLASH
; to make sure the bootloader remains accessible by the next session
; (with option for a new write attempt).

VerifyPage:
        rcall YtoBUFFER                 ; Y=BUFFER, bcnt=PAGESIZE*2
WrPV1:
        lpm tmp1, z+                    ; load flash byte
        ld tmp2,  y+                    ; load buffer byte
        cp tmp1, tmp2                   ; else compare
        breq WrPV2                      ; if EQUAL loop on
        brts WrPV2                      ; T=1 skips verify consequences
        rcall SendConfirm               ; in case of verify error
        rcall EraseAppFlash             ; erase appflash and freeze
WrPVf:  rjmp WrPVf                      ; for bootloader's sake
WrPV2:
        dec bcnt                        ; count down page bytecounter
        brne WrPV1                      ; loop until all bytes checked
WrPx:
        ret

.endif                  ; end of assembly for normal page devices

;-----------------------------------------------------------------------

.if FOURPAGES == 1                      ; 4-Page Erase+Write (tn441/841)
                                        ; for real PAGESIZE of 8 Words
WritePageafterErase:
        rcall EraseFlashPage
WritePage:
        rcall YtoBUFFER
        ldi tmp3, 4
WrPa4P:
        ldi bcnt, 16
WrPa1:
        ld r0, y+                       ; fill R0/R1 with word
        ld r1, y+                       ; from buffer position Y / Y+1
        ldi tmp1, 0b00000001            ; set only SPMEN in SPMCSR
        out SPMCSR, tmp1                ; to activate page buffering
        spm                             ; store word in page buffer
        adiw zl, 2                      ; and forward to next word
        subi bcnt, 2
        brne WrPa1
        sbiw zl, 16                     ; Z back to start of real-page
WrPa2:
        ldi tmp1, 0b00000101            ; enable PGWRT + SPMEN
        out SPMCSR, tmp1                ; in SPMCSR
        spm                             ; write sub-page to flash
WrPa3:
        in tmp1, SPMCSR                 ; wait for flash write finished
        sbrc tmp1, 0                    ; skip if SPMEN (bit0) cleared
        rjmp WrPa3                      ; ITS BEEN WRITTEN

        ldi bcnt, 16
        sbiw yl, 16                     ; back to start of real-page
WrPV1:
        lpm tmp1, z+                    ; load flash byte
        ld tmp2,  y+                    ; load buffer byte
        cp tmp1, tmp2                   ; else compare
        breq WrPV2                      ; if EQUAL loop on
        brts WrPV2                      ; T=1 skips verify consequences
        rcall SendConfirm               ; in case of verify error
        rcall EraseAppFlash             ; erase appflash and freeze
WrPVf:
        rjmp WrPVf                      ; for bootloader's sake
WrPV2:
        dec bcnt                        ; count down page bytecounter
        brne WrPV1                      ; loop until all bytes checked
WrPx:
        dec tmp3                        ; count all 4 sub-pages
        brne WrPa4P                     ; to be written from buffer
        ret
.endif          ; end of exclusive assembly for 4-page devices

;-----------------------------------------------------------------------
; FLASH ERASE TOP-TO-BOTTOM ( (BOOTSTART-1) ... $0000)
;-----------------------------------------------------------------------
; uses: Z, tmp1

EraseAppFlash:
        ldi zl, low (BOOTSTART*2)       ; point Z to BOOTSTART
        ldi zh, high(BOOTSTART*2)       ; 1st page's 1st address
EAF0:
        subi zl, low (PAGESIZE*2)       ; start erasing
        sbci zh, high(PAGESIZE*2)       ; one page below
        rcall EraseFlashPage            ; then erase pagewise down
        brne EAF0                       ; until first page reached
EAFx:   ret                             ; and leave with Z = $0000

;-----------------------------------------------------------------------
; ERASE FLASH / EEPROM / USERDATA
;-----------------------------------------------------------------------
; uses: Z, tmp1

EmergencyErase:
        rcall EraseAppFlash             ; first kill AppFlash contents
        ser tmp1
EEE0:
        rcall EEWriteByte               ; write EEPROM byte, Z = Z + 1
        cpi zh, high(EEPROMEND+1)+2     ; EEPROMEND
        brne EEE0                       ; and loop on until finished

        rcall ZtoLASTPAGE               ; to finally
;       rjmp EraseFlashPage             ; erase LASTPAGE (below)

;-----------------------------------------------------------------------
; ERASE FLASH PAGE
;-----------------------------------------------------------------------
; NOTE: with tn441/841 this implies 4-page erased at once

EraseFlashPage:
        ldi tmp1, 0b00000011            ; enable PGERS + SPMEN
        out SPMCSR, tmp1                ; in SPMCSR and erase current
        spm                             ; page by SPM (MCU halted)
        ret                             ; then return

;-----------------------------------------------------------------------
; OTHER SUBROUTINES
;-----------------------------------------------------------------------

YtoBUFFER:
        ldi yl, low (BUFFER)            ; reset pointer
        ldi yh, high(BUFFER)            ; to programming buffer
        ldi bcnt, low(PAGESIZE*2)       ; and often needed
        ret

;-----------------------------------------------------------------------

ZtoLASTPAGE:
        ldi zl, low (LASTPAGE*2)        ; reset Z to LASTPAGE start
        ldi zh, high(LASTPAGE*2)
        ret

;-----------------------------------------------------------------------
; RS232 RECEIVE BYTE
;-----------------------------------------------------------------------
; uses: tmp1 (received data byte), tmp2 (bitcounter)

ReceiveByte:
        sbi RXPORT, RXBIT               ; set pullup for RX
Recb1:
        sbic RXPIN, RXBIT               ; wait for startbit (0)
        rjmp Recb1                      ; loop whilst in stop state (1)
Recb2:
        ldi tmp2, 8                     ; bitcounter
        rcall Waithalfbitcell           ; go center of startbit
Recb3:
        rcall Waitbitcell               ; tune to center of bitcell
        lsr tmp1                        ; right shift 0 into bit 7
        sbic RXPIN, RXBIT               ; if RXD bit is 1 now
        sbr tmp1, 0b10000000            ; set bit 7 in RX byte
Recb4:
        dec tmp2                        ; count down bitcounter
        brne Recb3                      ; loop until 8 bits collected
        rjmp Waitbitcell                ; wait for center of stopbit

;-----------------------------------------------------------------------
; RS232 SEND CONFIRM CHARACTER
;-----------------------------------------------------------------------

SendConfirm:
        ldi tmp1, CONFIRM
        ;rjmp Transmitbyte              ; transmit confirm character

;-----------------------------------------------------------------------
; RS232 TRANSMIT BYTE
;-----------------------------------------------------------------------
; uses: tmp1 (transmit byte will be shifted out), tmp2 (bitcounter)
;
; with different portlines defined for RX and TX ("Two-Wire")
; => TX-line is actively driving high/low levels (LSTTL/HCMOS)
;
; with the same portline defined for RX and TX ("One-Wire")
; => TX-line is acting like an open collector/drain with weak pullup

TransmitByte:
        rcall Waitbitcell               ; ensure safe RX-TX transition
        rcall Trx0                      ; transmit 0 = startbit
        ldi tmp2, 8                     ; set bitcounter
Trxbit:                                 ; transmit byte loop
        sbrc tmp1, 0
        rcall Trx1                      ; sent logical 1 bitcell
        sbrs tmp1, 0                    ; or
        rcall Trx0                      ; sent logical 0 bitcell
        lsr tmp1                        ; shift out that bit
        dec tmp2                        ; count down
        brne Trxbit                     ; loop until all bits sent
Trx1:
        sbi TXDDR, TXBIT                ; if RX=TX (One-Wire), result is
        cbi RXDDR, RXBIT                ; pullup to Vcc for "1" (high-Z)
        sbi TXPORT, TXBIT               ; else portbit actively driven
        rjmp Waitbitcell
Trx0:
        sbi TXDDR, TXBIT                ; set TX driver for output
        cbi TXPORT, TXBIT               ; set portbit to active "0"
;       rjmp Waitbitcell                ; continue with Waitbitcell

;-----------------------------------------------------------------------
; RS232 PRECISION TIMING
;-----------------------------------------------------------------------
; input: bclkh, bclkl   (division-factor for bitcells at given baudrate)

Waitbitcell:
        movw xl, bclkl                  ; load bitcell clock timer
wbc1:
        sbiw xl, 17                     ; same number of clocks
        nop                             ; as in calibration loop
        brcc wbc1
wbcx:   ret

Waithalfbitcell:
        movw xl, bclkl                  ; load bitcell clock timer
        lsr xh                          ; shift out bit 0 of xh to carry
        ror xl                          ; carry shifted in bit 7 of xl
        rjmp wbc1                       ; run timer with 1/2 divider

;-----------------------------------------------------------------------
; DEVICE INFO BLOCK = PERMANENT DATA
;-----------------------------------------------------------------------
; 16 bytes of permanent data identifying bootloader and device

.if TSBINSTALLER == 0
        .org DEVICEINFO         ; TSB for ATtiny on target address
.else
        .org TDEVICEINFO        ; TSB-Installer temporary address
.endif

.message "DEVICE INFO BLOCK FOR ATTINY"

.db "tsb", low (BUILDDATE), high (BUILDDATE), BUILDSTATE
.db SIGNATURE_000, SIGNATURE_001, SIGNATURE_002, low (PAGESIZE)
.dw BOOTSTART
.dw EEPROMEND

.if FLASHEND >= ($1fff)         ; check for device >= 16 KB
        APPJUMP:
        .db $0C, $94            ; first two bytes (opcode) for a JMP!
.else
        .db $00, $00
        APPJUMP:
.endif

;-----------------------------------------------------------------------
; LASTPAGE WITH APPLICATION-JUMP, TIMEOUT, PASSWORD (USER DATA)
;-----------------------------------------------------------------------
; Leaving this page blank ($FF) will be safe defaults

RLASTPAGE:           ; reference label for end of fixed TSB code
                     ; in TSB/TSB-Installer (LASTPAGE)
; Check for correct alignment of TSB code when assembled to target

.if (RLASTPAGE == CLASTPAGE)
        .message "TSB CODE IS PROPERLY ALIGNED"
.endif

.if (RLASTPAGE >  CLASTPAGE)
        .message "WARNING: TSB CODE IS TOO LARGE!"
.endif

.message "ASSEMBLY OF TSB FOR ATTINY SUCCESSFULLY FINISHED!"

.endif          ; this endif closes assembly of TSB for ATtinys

;***********************************************************************
; END OF TSB FOR ATTINYS
;***********************************************************************
;
;
;
;
;
;***********************************************************************
;***********************************************************************
;***********************************************************************
; START OF TSB FOR ATMEGAS
;***********************************************************************
;***********************************************************************
;***********************************************************************
;
; TSB for ATmegas is always coded directly to target address.

.if TINYMEGA == 1

.message "ASSEMBLY OF TSB FOR ATMEGA"

.equ    BOOTSTART       = (FLASHEND+1)-256      ; = 512 Bytes
.equ    LASTPAGE        = BOOTSTART - PAGESIZE  ; = 1 page below TSB!

.org    BOOTSTART

RESET:
        cli

        in tmp4, MCUSR                  ; check reset condition
        sbrc tmp4, WDRF                 ; in case of a Watchdog reset
        rjmp APPJUMP                    ; immediately leave TSB

        ldi tmp1, low (RAMEND)          ; write ramend low
        out SPL, tmp1                   ; into SPL (stackpointer low)
.ifdef SPH
        ldi tmp1, high(RAMEND)          ; write ramend high for ATtinys
        out SPH, tmp1                   ; with SRAM > 256 bytes
.endif

;-----------------------------------------------------------------------
; ACTIVATION CHECK
;-----------------------------------------------------------------------

WaitRX:
        sbi TXDDR, TXBIT                ; if RX=TX (One-Wire), port is
        cbi RXDDR, RXBIT                ; driven open collector style,
        sbi RXPORT, RXBIT               ; else RX is input with pullup
        sbi TXPORT, TXBIT               ; and TX preset logical High

; coldstart (power-up) - wait for RX to stabilize
WRX0To:
        sbic RXPIN, RXBIT               ; 1st stage - loop while RX = 0
        rjmp WRXSTo                     ; if RX = 1, start COM Timeout
        sbiw xl, 1                      ; use X for fast countdown
        brcc WRX0To                     ; if timed out with 0 level
        rjmp APPJUMP                    ; goto APPJUMP in LASTPAGE
WRXSTo:                                 ; else
        rcall ZtoLASTPAGE               ; set Z to start'o'LASTPAGE
        adiw zl, 2                      ; skip first 2 bytes
        lpm xh, z+                      ; load TIMEOUT byte
        ldi xl, 128

WRX1To:
        dec tmp1                        ; inner counter to delay
        brne WRX1To                     ; for debouncing/denoising
        sbis RXPIN, RXBIT               ; if serial startbit occurs
        rjmp Activate                   ; go Activate
        sbiw xl, 1                      ; use X for down counter
        brcc WRX1To                     ; to Timeout
        ;rjmp APPJUMP                   ; Timeout! Goto APPJUMP

;-----------------------------------------------------------------------
; ATMEGA APPJUMP = SIMPLE JUMP TO $0000 (ORIGINAL RESET VECTOR)
;-----------------------------------------------------------------------
; Boot Reset Vector (BOOTRST) must be activated for TSB on ATmegas.
; After timeout or executing commands, TSB for ATmegas will simply
; handover to the App by a (relative or absolute) jump to $0000.

APPJUMP:
        rcall SPMwait                   ; make sure everything's done

.if FLASHEND >= ($1fff)
        jmp  $0000                      ; absolute jump
.else
        rjmp $0000                      ; relative jump
.endif

;-----------------------------------------------------------------------
; BAUDRATE CALIBRATION CYCLE
;-----------------------------------------------------------------------

Activate:
        clr xl                          ; clear temporary
        clr xh                          ; baudrate counter
        ldi tmp1, 5                     ; number of expected bit-changes
actw0:
        sbis RXPIN, RXBIT               ; discard first 0-state
        rjmp actw0
actw1:
        sbic RXPIN, RXBIT               ; idle 1-states (stopbits, ones)
        rjmp actw1
actw2:
        adiw xl, 1                      ; precision measuring loop
        sbis RXPIN, RXBIT               ; count clock cycles
        rjmp actw2                      ; while RX is active (0)

        sbiw xl, 10
        dec tmp1
        brne actw1
actwx:
        movw bclkl, xl                  ; save result in bclk

;-----------------------------------------------------------------------
; CHECK PASSWORD / EMERGENCY ERASE
;-----------------------------------------------------------------------

CheckPassword:
chpw0:
        ser tmp4                        ; tmp4 = 255 enables comparison
chpw1:
        lpm tmp3, z+                    ; load pw character from Z
        and tmp3, tmp4                  ; tmp3 = 0 disables comparison
        cpi tmp3, 255                   ; byte value 255 indicates
        breq chpwx                      ; end of password -> success
chpw2:
        rcall Receivebyte               ; else receive next character
        cpi tmp1, 0                     ; rxbyte = 0 will branch
        breq chpwee                     ; to confirm emergency erase
        cp  tmp1, tmp3                  ; compare password with rxbyte
        breq chpw0                      ; if equal check next character
        clr  tmp4                       ; tmp4 = 0 to loop forever
        rjmp chpw1                      ; all to smoothen power profile
chpwee:
        rcall RequestConfirm            ; request confirmation
        brts chpa                       ; not confirmed, leave
        ;~ rcall RequestConfirm            ; request 2nd confirmation
        ;~ brts chpa                       ; can't be mistake now
        rcall EmergencyErase            ; go, emergency erase!
        rjmp  Mainloop                  ; return to main loop
chpa:
        rjmp APPJUMP                    ; else start application
chpwx:
;       rjmp SendDeviceInfo             ; go on to SendDeviceInfo

;-----------------------------------------------------------------------
; SEND DEVICEINFO
;-----------------------------------------------------------------------

SendDeviceInfo:
        ldi zl, low (DEVICEINFO*2)      ; load address of deviceinfo
        ldi zh, high(DEVICEINFO*2)      ; low and highbyte
        ldi bcnt, INFOLEN*2
        rcall SendFromFlash             ; send info, then go mainloop

;-----------------------------------------------------------------------
; MAIN LOOP TO RECEIVE AND EXECUTE COMMANDS
;-----------------------------------------------------------------------

Mainloop:
        clr zl                          ; clear Z pointer
        clr zh                          ; which is frequently used
        rcall SendConfirm               ; send CONFIRM via RS232
        rcall Receivebyte               ; receive command via RS232
        rcall CheckCommands             ; check command letter
        rjmp Mainloop                   ; and loop on

;-----------------------------------------------------------------------
; CHANGE USER DATA IN LASTPAGE
;-----------------------------------------------------------------------

ChangeSettings:
        rcall GetNewPage                ; get new LASTPAGE contents
        brtc ChangeS0                   ; from Host (if confirmed)
        ret
ChangeS0:
        rcall ZtoLASTPAGE               ; re-write LASTPAGE
        rcall EraseFlashPage
        rcall WritePage                 ; erase and write LASTPAGE

;-----------------------------------------------------------------------
; SEND USER DATA FROM LASTPAGE
;-----------------------------------------------------------------------

CheckSettings:
        rcall ZtoLASTPAGE               ; point to LASTPAGE
;       rcall SendPageFromFlash

;-----------------------------------------------------------------------
; SEND DATA FROM FLASH MEMORY
;-----------------------------------------------------------------------

SendPageFromFlash:
        ldi bcnt, low (PAGESIZE*2)      ; whole Page to send
SendFromFlash:
        rcall SPMwait                   ; (re)enable RWW read access
        lpm tmp1, z+                    ; read directly from flash
        rcall Transmitbyte              ; and send out to RS232
        dec bcnt                        ; bcnt is number of bytes
        brne SendFromFlash
        ret

;-----------------------------------------------------------------------
; READ APPLICATION FLASH
;-----------------------------------------------------------------------
; read and transmit application flash area (pagewise)

ReadAppFlash:
RAF0:
        rcall RwaitConfirm
        brts RAFx
        rcall SendPageFromFlash
RAF1:
        cpi zl, low (LASTPAGE*2)        ; count up to last byte
        brne RAF0                       ; below LASTPAGE
        cpi zh, high(LASTPAGE*2)
        brne RAF0
RAFx:
        ret

;-----------------------------------------------------------------------
; WRITE APPLICATION FLASH
;-----------------------------------------------------------------------
; Write Appflash pagewise, don't modify first page on ATmegas

WriteAppFlash:
        rcall EraseAppFlash             ; Erase whole app flash
Flash2:
        rcall GetNewPage                ; get next page from host
        brts FlashX                     ; stop on user's behalf
Flash3:
        rcall WritePage                 ; write page data into flash
Flash4:
        cpi zh, high(LASTPAGE*2-1)      ; end of available Appflash?
        brne Flash2                     ; if Z reached last location
        cpi zl, low (LASTPAGE*2-1)      ; then we are finished
        brne Flash2                     ; else go on
FlashX:
        ret                             ; we're already finished!

;-----------------------------------------------------------------------
; WRITE FLASH PAGE FROM BUFFER, VERIFYING AND VERIFY-ERROR-HANDLING
;-----------------------------------------------------------------------

WritePage:
        rcall YtoBUFFER                 ; Y=BUFFER, bcnt=PAGESIZE*2
WrPa1:
        ld r0, y+                       ; fill R0/R1 with word
        ld r1, y+                       ; from buffer position Y / Y+1
        ldi tmp1, 0b00000001            ; set only SPMEN in SPMCSR
        out SPMCSR, tmp1                ; to activate page buffering
        spm                             ; store word in page buffer
        adiw zl, 2                      ; and forward to next word
        subi bcnt, 2
        brne WrPa1
        ; Z = start of next page now
        subi zl, low (PAGESIZE*2)       ; point back Z to
        sbci zh, high(PAGESIZE*2)       ; start of current page
        ; Z = back on current page's start
WrPa2:
        ldi tmp1, 0b00000101            ; enable PRWRT + SPMEN
        out SPMCSR, tmp1                ; in SPMCSR
        spm                             ; write whole page to flash
WrPa3:
        in tmp1, SPMCSR                 ; wait for flash write finished
        sbrc tmp1, 0                    ; skip if SPMEN (bit0) cleared
        rjmp WrPa3                      ; ITS BEEN WRITTEN
        subi zl, low (-PAGESIZE*2)      ; same effect as
        sbci zh, high(-PAGESIZE*2)      ; Z = Z + PAGESIZE*2
        ret

;-----------------------------------------------------------------------
; CHECK COMMANDS
;-----------------------------------------------------------------------

CheckCommands:
        cpi tmp1, 'c'                   ; read LASTPAGE
        breq CheckSettings
        cpi tmp1, 'C'                   ; write LASTPAGE
        breq ChangeSettings
        cpi tmp1, 'f'                   ; read Appflash
        breq ReadAppFlash
        cpi tmp1, 'F'                   ; write Appflash
        breq WriteAppFlash
        cpi tmp1, 'e'                   ; read EEPROM
        breq EepromRead
        cpi tmp1, 'E'                   ; write EEPROM
        breq EEpromWrite
        rjmp APPJUMP                    ; else start application

;-----------------------------------------------------------------------
; EEPROM READ/WRITE ACCESS
;-----------------------------------------------------------------------

EepromWrite:
EEWr0:
        rcall GetNewPage                ; get EEPROM datablock
        brts EERWFx                     ; or abort on host's demand
EEWr1:
        rcall YtoBUFFER                 ; Y = Buffer and Bcnt = blocksize
EEWr2:
        ld tmp1, y+                     ; read EEPROM byte from buffer
        rcall EEWriteByte
        dec bcnt                        ; count down block byte counter
        brne EEWr2                      ; loop on if block not finished
        rjmp EeWr0

;-----------------------------------------------------------------------

EEpromRead:
EeRe1:
        rcall RwaitConfirm              ; wait to confirm
        brts EERWFx                     ; else we are finished
        ldi bcnt, low(PAGESIZE*2)       ; again PAGESIZE*2 is blocksize
EERe2:
        out EEARL, zl                   ; current EEPROM address low
        .ifdef  EEARH
        out EEARH, zh                   ; current EEPROM address high
        .endif
        sbi EECR, 0                     ; set EERE - EEPROM read enable
        in tmp1, EEDR                   ; read byte from current address
        rcall Transmitbyte              ; send out to RS232
        adiw zl,1                       ; count up EEPROM address
        dec bcnt                        ; count down block byte counter
        brne EERe2                      ; loop on if block not finished
        rjmp EERe1
EERWFx:
        ret

;-----------------------------------------------------------------------

EEWriteByte:
        out EEDR, tmp1                  ; write to EEPROM data register
        out EEARL, zl                   ; current EEPROM address low
        .ifdef  EEARH
        out EEARH, zh                   ; high EEARH for some attinys
        .endif
        sbi EECR, 2                     ; EEPROM master prog enable
        sbi EECR, 1                     ; EEPE initiate prog cycle
EeWB:
        sbic EECR, 1                    ; wait write cycle to complete
        rjmp EeWB                       ; before we can go on
        adiw zl,1                       ; count up EEPROM address
        ret

;-----------------------------------------------------------------------
; GET NEW PAGE
;-----------------------------------------------------------------------

GetNewPage:
        rcall RequestConfirm            ; check for Confirm
        brts GNPx                       ; abort if not confirmed
GNP0:
        rcall YtoBUFFER                 ; Y = BUFFER, bcnt = PAGESIZE*2
GNP1:
        rcall ReceiveByte               ; receive serial byte
        st y+, tmp1                     ; and store in buffer
        dec bcnt                        ; until full page loaded
        brne GNP1                       ; loop on
GNPx:
        ret                             ; finished
;-----------------------------------------------------------------------
; REQUEST TO CONFIRM / AWAIT CONFIRM COMMAND
;-----------------------------------------------------------------------

RequestConfirm:
        ldi tmp1, REQUEST               ; send request character
        rcall Transmitbyte              ; prompt to confirm (or not)
RwaitConfirm:
        rcall ReceiveByte               ; get host's reply
        clt                             ; set T=0 for confirmation
        cpi tmp1, CONFIRM               ; if host HAS sent CONFIRM
        breq RCx                        ; return with the T=0
        set                             ; else set T=1 (NOT CONFIRMED)
RCx:
        ret                             ; whether confirmed or not

;-----------------------------------------------------------------------
; FLASH ERASE TOP-TO-BOTTOM ( (BOOTSTART-1) ... $0000)
;-----------------------------------------------------------------------

EraseAppFlash:
        rcall ZtoLASTPAGE               ; point Z to LASTPAGE, directly
EAF0:
        subi zl, low (PAGESIZE*2)
        sbci zh, high(PAGESIZE*2)
        rcall EraseFlashPage
        brne EAF0                       ; until first page reached
EAFx:   ret                             ; and leave with Z = $0000

;-----------------------------------------------------------------------
; EMERGENCY ERASE OF FLASH / EEPROM / USERDATA
;-----------------------------------------------------------------------

EmergencyErase:
        rcall EraseAppFlash             ; erase Application Flash
        ser tmp1                        ; byte value for EEPROM writes
EEE0:
        rcall EEWriteByte               ; write EEPROM byte, Z = Z + 1
        cpi zh, high(EEPROMEND+1)+2     ; EEPROMEND
        brne EEE0                       ; and loop on until finished

        rcall ZtoLASTPAGE               ; LASTPAGE is to be erased
;        rcall EraseFlashPage

;-----------------------------------------------------------------------
; ERASE ONE FLASH PAGE
;-----------------------------------------------------------------------

EraseFlashPage:
        ldi tmp1, 0b00000011            ; enable PGERS + SPMEN
        out SPMCSR, tmp1                ; in SPMCSR and erase current
        spm                             ; page by SPM (MCU halted)

; Waiting for SPM to finish is *obligatory* on ATmegas!
SPMwait:
        in tmp1, SPMCSR
        sbrc tmp1, 0                    ; wait previous SPMEN
        rjmp SPMwait
        ldi tmp1, 0b00010001            ; set RWWSRE and SPMEN
        out SPMCSR, tmp1
        spm
        ret

;-----------------------------------------------------------------------
; OTHER SUBROUTINES
;-----------------------------------------------------------------------

YtoBUFFER:
        ldi yl, low (BUFFER)            ; reset pointer
        ldi yh, high(BUFFER)            ; to programming buffer
        ldi bcnt, low(PAGESIZE*2)       ; and often needed
        ret

;-----------------------------------------------------------------------

ZtoLASTPAGE:
        ldi zl, low (LASTPAGE*2)        ; reset Z to LASTPAGE start
        ldi zh, high(LASTPAGE*2)
        ret

;-----------------------------------------------------------------------
; RS232 RECEIVE BYTE
;-----------------------------------------------------------------------

ReceiveByte:
        sbi RXPORT, RXBIT               ; again set pullup for RX
Recb1:
        sbic RXPIN, RXBIT               ; wait for startbit (0)
        rjmp Recb1                      ; loop while stop state (1)
Recb2:
        ldi tmp2, 8                     ; bitcounter
        rcall Waithalfbitcell           ; tune to center of startbit
Recb3:
        rcall Waitbitcell               ; tune to center of bitcell
        lsr tmp1                        ; right shift 0 into bit 7
        sbic RXPIN, RXBIT               ; if RXD bit is 1
        sbr tmp1, 0b10000000            ; set bit 7
Recb4:
        dec tmp2                        ; count down bitcounter
        brne Recb3                      ; loop until 8 bits collected
        rjmp Waitbitcell                ; wait into center of stopbit

;-----------------------------------------------------------------------
; RS232 SEND CONFIRM CHARACTER
;-----------------------------------------------------------------------

SendConfirm:
        ldi tmp1, CONFIRM
        ;rjmp Transmitbyte

;-----------------------------------------------------------------------
; RS232 TRANSMIT BYTE
;-----------------------------------------------------------------------

TransmitByte:
        rcall Waitbitcell               ; ensure safe RX-TX transition
        rcall Trx0                      ; transmit 0 = startbit
        ldi tmp2, 8                     ; set bitcounter
Trxbit:                                 ; transmit byte loop
        sbrc tmp1, 0
        rcall Trx1                      ; sent logical 1 bitcell
        sbrs tmp1, 0                    ; or
        rcall Trx0                      ; sent logical 0 bitcell
        lsr tmp1                        ; shift out that bit
        dec tmp2                        ; count down
        brne Trxbit                     ; loop until all bits sent
Trx1:
        sbi TXDDR, TXBIT                ; if RX=TX (One-Wire), result is
        cbi RXDDR, RXBIT                ; pullup to Vcc for "1" (high-Z)
        sbi TXPORT, TXBIT               ; else portbit actively driven
        rjmp Waitbitcell
Trx0:
        sbi TXDDR, TXBIT                ; set TX driver for output
        cbi TXPORT, TXBIT               ; set portbit to active "0"
;       rjmp Waitbitcell                ; continue with Waitbitcell

;-----------------------------------------------------------------------
; RS232 PRECISION TIMING
;-----------------------------------------------------------------------

Waitbitcell:
        movw xl, bclkl                  ; load bitcell clock timer
wbc1:
        sbiw xl, 17                     ; same number of clocks
        nop                             ; as in calibration loop
        brcc wbc1
wbcx:   ret

Waithalfbitcell:
        movw xl, bclkl                  ; load bitcell clock timer
        lsr xh                          ; shiftout bit 0 of xh to carry
        ror xl                          ; carry shifted in bit 7 of xl
        rjmp wbc1                       ; run timer with 1/2 divider

;-----------------------------------------------------------------------
; DEVICE INFO BLOCK = PERMANENT DATA
;-----------------------------------------------------------------------

DEVICEINFO:
.message "DEVICE INFO BLOCK FOR ATMEGA"
.db "TSB", low (BUILDDATE), high (BUILDDATE), BUILDSTATE
.db SIGNATURE_000, SIGNATURE_001, SIGNATURE_002, low (PAGESIZE)
.dw BOOTSTART-PAGESIZE
.dw EEPROMEND
.db $AA, $AA

.message "ASSEMBLY OF TSB FOR ATMEGA SUCCESSFULLY FINISHED!"

.endif               ; closing TSB for ATmega sourcecode;

;***********************************************************************
; END OF TSB FOR ATMEGAS
;***********************************************************************

.exit

;***********************************************************************
;***********************************************************************
;***********************************************************************
; END OF CONDITIONAL ASSEMBLY SOURCE OF TSB FOR ATTINYS AND ATMEGAS
;***********************************************************************
;***********************************************************************
;***********************************************************************


