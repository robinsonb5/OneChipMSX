
	; One Chip MSX IPL-ROM
	; Disassembly & Comments By : NYYRIKKI
	; Further comments by AMR

	ORG	0F000H

XF000:	DI	; Disable interrrupts
	JR	START


	; Load data
	; HL = Load address
	; B  = Number of sectors
	; CDE= Sector number (auto increase)

LOAD:	JP	SECLOAD
; LOADADR:	EQU	$-2

	; Move code to RAM

START:	LD	BC,0200H	; 512 bytes - increase this if ROM expands.
	LD	DE,XF000
	LD	HL,0000H
	LDIR

	; Init Palette
	LD	HL,VDPDATA
	LD	BC,0299H	; Copy 2 bytes to IO_99
	OTIR
	LD	BC,209AH	; Copy 32 bytes to IO_9A (why 9a?)
	OTIR
	JP	XF043
VDPDATA:
	DEFB 00,90
	DEFB 00,00,00,00,11,06,33,07,17,01,27,03,51,01,27,06
	DEFB 71,01,73,03,61,06,64,06,11,04,65,02,55,05,77,07

XF043:	LD	SP,0FFFFH	; Set Stack pointer to 64K-1
	LD	A,40H
	LD	(6000H),A	; Write 0x40 -> ErmBank0
	LD	BC,0100H	; B: number of blocks.  C: top byte of LBA address
	LD	DE,0000H	; Lower two bytes of LBA address
	LD	HL,0C000H	; Target address
	CALL LOAD
	JR	C,XF07B		; Carry set, load from SD failed - copy from EPCS instead
	CALL CHKFAT
	JR	C,XF071		; Carry set, found FAT, so load ROM file
	CALL CHKMBR		; otherwise check for an MBR
	JR	C,XF07B		; Carry set, no MBR found, so copy from EPCS.
	PUSH DE
	PUSH BC			; Save registers
	LD	B,01H		; 0x0100 -> BC?
	LD	HL,0C000H	; Target address
	CALL LOAD
	POP	BC
	POP	DE
	JR	C,XF07B	; Bug? Should be XF07B?  Yes, almost certainly
XF071:	CALL	FINDROMFILE
	JR	C,XF07B
	CALL	COPYROMS
	JR	XF08D

	; Copy ROM from EPCS device
XF07B:	LD	HL,ROMLOAD
;	LD	(LOADADR),HL
	LD	(LOAD+1),HL	; Modify load routine to EPCS version
	LD	A,60H
	LD	(6000H),A	; Select EPCS rather than SD card
	LD	DE,0200H
	LD	C,E
	CALL	COPYROMS

XF08D:	XOR	A
	LD	(6000H),A	; 0 -> 6000 (Deselect SD and EPCS)
	INC	A
	LD	(6800H),A	; 1 -> 6800
	LD	(7000H),A	; 1 -> 7000
	LD	(7800H),A	; 1 -> 7800
	LD	A,0C0H
	OUT	(0A8H),A	; c0 -> IO_A8
	RST	00H			; 

	; LBA address in E:D:C
COPYROMS:
	LD	B,10H 		; Number of 16KB ROMs
	LD	A,80H		; ROM base address will be 0x100000
XF0A4:	LD	(7000H),A	; ERM Bank 2 (mapping writes in range 8000H to 9FFFH)
	INC	A
	LD	(7800H),A	; ERM Bank 3 (mapping writes in range A000H to BFFFH)
	INC	A
	PUSH	AF
	PUSH	BC
	LD	B,20H		; 32 sectors, 16kb
	LD	HL,8000H	; Base address 8000H (writes up to BFFFH)
	CALL	LOAD	
	POP	BC
	POP	HL
	RET	C			; Bail out on failure
	LD	A,H			; Base address for next ROM
	DJNZ	XF0A4	; Keep reading ROMs
	RET

	; Byte offset in E:D:C
	; Target address in HL
ROMLOAD:
	PUSH	DE
	PUSH	BC
	SLA	E
	RL	D			; Double the incoming address
	LD	A,B
	ADD	A,A			; Double the number of sectors (loop copies 256 bytes at a time)
	LD	C,A
	LD	B,00H
	PUSH	HL
	LD	HL,4000H	; Send commands to EPCS
	LD	(HL),03H	; Read bytes command
	LD	(HL),D		; Byte offset (high)
	LD	(HL),E		; Byte offset
	LD	(HL),B		; Byte offset (low)
	LD	A,(HL)
	POP	DE			; Target address
XF0D3:	LD	A,(HL)
	LD	(DE),A
	INC	DE
	DJNZ	XF0D3	; 256 bytes
	DEC	C
	JR	NZ,XF0D3	; More 256 byte chunks?
	LD	A,(5000H)
	POP	BC
	POP	HL
	XOR	A
	LD	D,A
	LD	E,B			; Increment byte offset for next call.
	ADD	HL,DE
	EX	DE,HL
	ADC	A,C
	LD	C,A
	RET

	; Sector load & subroutines

	; SNDCMD
	; SPI address in HL
	; LBA address in E:D:C
	; Command in B
SNDCMD:	LD	A,(HL)	; Spin SPI
	SLA	E			; E << 1   -  Turn LBA address into byte offset (Only works for SD, not SDHC)
	RL	D			; D << 1 | carry
	RL	C			; C << 1 | carry
	LD	(HL),B		; Command
	LD	(HL),C		; LBA >> 24
	LD	(HL),D		; LBA >> 16
	LD	(HL),E		; LBA >> 8
	LD	(HL),00H	; LBA (Assuming SD semantics, not SHDC)
	LD	(HL),95H	; CRC must be valid for reset, can be ignored thereafter.
	LD	A,(HL)		;	Spin SPI
	LD	B,10H
cmdloop:	LD	A,(HL)	; Read response from card
	CP	0FFH		; Did we get a response yet?
	CCF	; Complement carry flag
	RET	NC			; Return if carry zero
	DJNZ	cmdloop
	SCF
	RET				; Return with carry set for failure.


	; Reset SD Card

SDRESET:
	LD	B,0AH
spin_reset:	LD	A,(5000H)
	DJNZ	spin_reset
	LD	BC,4000H	; Reset command, 0x40, LBA 0
	LD	E,C
	LD	D,C
	CALL	SNDCMD
	RET	C			; Failed, return with carry set
	AND	0F7H
	CP	01H			; Did we get the response 0x01
	SCF
	RET	NZ			; No? Return with carry set

l_sdinit:
	LD	B,77H		; V2 init - "CMD55"
	CALL	SNDCMD
	AND	04H			; Understood?
	JR	Z,initv1
	LD	B,41H		; No?  Send V1 Init command
	CALL	SNDCMD
	JR	initcheck
initv1:
	LD	B,69H	; "CMD41"
	CALL	SNDCMD
initcheck:
	RET	C			; Return with carry set on failure
	CP	01H
	JR	Z,l_sdinit	; If we received 0x01 repeat the process
	OR	A
	RET	Z			; If we received 0x00 then the card is initialised and ready to use.
	SCF
	RET

XF137:	CALL	SDRESET
	POP	BC
	POP	DE
	POP	HL
	RET	C			; BUG?  Should this fall through?

	; B - number of blocks to read
	; CDE - LBA address
	; HL - Target address

SECLOAD:	
	PUSH	HL
	PUSH	DE
	PUSH	BC		; Save registers
	LD	B,51H		; Attempt a read
	LD	HL,4000H
	CALL	SNDCMD
	JR	C,XF137		; Read failed - reset card.
	POP	BC
	POP	DE
	POP	HL
	OR	A
	SCF
	RET	NZ			; Fail if read returned anything other than zero
	PUSH	DE
	PUSH	BC
	EX	DE,HL
	LD	BC,0200H
	LD	HL,4000H
XF15A:	LD	A,(HL)
	CP	0FEH
	JR	NZ,XF15A
	LDIR
	EX	DE,HL
	LD	A,(DE)
	POP	BC
	LD	A,(DE)
	POP	DE
	INC	DE
	LD	A,D
	OR	E
	JR	NZ,XF16C
	INC	C
XF16C:	DJNZ	SECLOAD
	RET

	; Search for "FAT"
	; C = Found

CHKFAT:	LD	HL,0C000H
	LD	BC,0080H
XF175:	LD	A,46H  ; F
	CPIR
	JR	Z,XF17D
	OR	A
	RET
XF17D:	PUSH	HL
	LD	D,(HL)
	INC	HL
	LD	E,(HL)
	LD	HL,4154H ; AT
	OR	A
	SBC	HL,DE
	POP	HL
	JR	NZ,XF175
	LD	C,00H
	LD	E,C
	LD	D,C
	SCF
	RET

	; Search Partition
	; C = Not Found

CHKMBR:	LD	B,04H
	LD	HL,0C1C6H
XF195:	PUSH	HL
	LD	E,(HL)
	INC	HL
	LD	D,(HL)
	INC	HL
	LD	C,(HL)
	LD	A,C
	OR	D
	OR	E
	POP	HL
	RET	NZ
	LD	DE,0010H
	ADD	HL,DE
	DJNZ	XF195
	SCF
	RET

FINDROMFILE:
	LD	IX,0C000H
	LD	L,(IX+0EH)	; Reserved sectors
	LD	H,(IX+0FH)
	LD	A,C
	ADD	HL,DE
	ADC	A,00H
	LD	C,A
	LD	E,(IX+11H) ; Root entries
	LD	D,(IX+12H)
	LD	A,E
	AND	0FH
	LD	B,04H
XF1C2:	SRL	D
	RR	E
	DJNZ	XF1C2
	OR	A
	JR	Z,XF1CC
	INC	DE
XF1CC:	PUSH	DE
	LD	B,(IX+10H) ; Number of FATs
	LD	E,(IX+16H) ; Sectors / FAT
	LD	D,(IX+17H)
	LD	A,C
XF1D7:	ADD	HL,DE
	ADC	A,00H
	DJNZ	XF1D7
	POP	DE
	ADD	HL,DE
	EX	DE,HL
	LD	C,A
	PUSH	DE
	PUSH	BC
	LD	B,01H
	LD	HL,0C000H
	CALL	LOAD
	RET	C
	LD	HL,(0C000H)
	LD	DE,4241H	; ROM header
	OR	A
	SBC	HL,DE
	POP	BC
	POP	DE
	RET	Z
	SCF
	RET

	DEFB 0,0,0,0,0,0,0

	END
