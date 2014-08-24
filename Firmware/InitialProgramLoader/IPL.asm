
	; Simplified One Chip MSX IPL-ROM
	; for reading BIOS from ZPU Control module
	; Simplified by AMR

	ORG	0F000H

XF000:	DI	; Disable interrrupts

	; Move code to RAM

	LD	BC,0200H	; 512 bytes - increase this if ROM expands.
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

XF043:
	LD	SP,0FFFFH	; Set Stack pointer to 64K-1

	; Copy ROM from EPCS device
XF07B:
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

COPYROMS:
	LD	B,18H 		; Number of 16KB ROMs
	LD	A,80H		; ROM base address will be 0x100000
XF0A4:	LD	(7000H),A	; ERM Bank 2 (mapping writes in range 8000H to 9FFFH)
	INC	A
	LD	(7800H),A	; ERM Bank 3 (mapping writes in range A000H to BFFFH)
	INC	A
	PUSH	AF
	PUSH	BC
	LD	B,20H		; 32 sectors, 16kb
	LD	HL,8000H	; Base address 8000H (writes up to BFFFH)
	CALL	ROMLOAD	
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
;	LD	(HL),03H	; Read bytes command
;	LD	(HL),D		; Byte offset (high)
;	LD	(HL),E		; Byte offset
;	LD	(HL),B		; Byte offset (low)
;	LD	A,(HL)		; (don't bother to) Discard the first byte...
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

	END
