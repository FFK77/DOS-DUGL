SingleMask			DB	0x0A,0x0A,0x0A,0x0A,0xFF,0xD4,0xD4,0xD4
TransferMode		DB	0x0B,0x0B,0x0B,0x0B,0xFF,0xD6,0xD6,0xD6
ClearFlipFlop		DB	0x0C,0x0C,0x0C,0x0C,0xFF,0xD8,0xD8,0xD8
PageAddress			DB 	0x87,0x83,0x81,0x82,0xFF,0x8B,0x89,0x8A
Address				DB	0x00,0x02,0x04,0x06,0xFF,0xC4,0xC8,0xCC
Length 				DB	0x01,0x03,0x05,0x07,0xFF,0xC6,0xCA,0xCE

%macro @AllocDOSMemDMA8	1
		MOV			AX,0x100
		MOV			BX,(%1>>4)
		INT			0x31
		JC			%%MemDMA8Error
		AND			EAX,0xffff
		SHL			EAX,4    ; adresse physique bloc dans EAX
		CALL		GetBaseAddress
		MOV			ECX,EAX
		MOV			[EBX+PhysAddDMA8],EAX
		MOV			[EBX+SelectorDMA8],EDX
		AND			EAX,0xffff
		ADD			EAX,(%1>>4)
		TEST		EAX,0xffff0000 ; aligne 64 Kb
		JZ			%%MemDMA8Ok
		MOV			AX,0x101   ; DX contient deja selecteur
		INT			0x31
		JC			%%MemDMA8Error
		MOV			AX,0x100   ; alloue nouveau Bloc 2*plus grand
		MOV			BX,(%1>>4)*2
		INT			0x31
		JC			%%MemDMA8Error
		AND			EAX,0xffff
		SHL			EAX,4      ; adresse physique bloc dans EAX
		ADD			EAX,(%1>>4)
		AND			EAX,0xffff0000
		CALL		GetBaseAddress
		MOV			[EBX+PhysAddDMA8],EAX
		MOV			[EBX+SelectorDMA8],EDX
		JMP			SHORT %%MemDMA8Ok

%%MemDMA8Error:
		XOR			EAX,EAX
		JMP			SHORT %%FinAllocMem
%%MemDMA8Ok:
		CALL		GetBaseAddress
		MOV			EAX,[EBX+PhysAddDMA8]
%%FinAllocMem:
%endmacro

%macro @AllocDOSMemDMA16 1
		MOV			AX,0x100
		MOV			BX,(%1>>4)
		INT			0x31
		JC			%%MemDMA16Error
		AND			EAX,0xffff
		SHL			EAX,4    ; adresse physique bloc dans EAX
		CALL		GetBaseAddress
		MOV			ECX,EAX
		MOV			[EBX+PhysAddDMA16],EAX
		MOV			[EBX+SelectorDMA16],EDX
		AND			EAX,0x1ffff
		ADD			EAX,(%1>>4)
		TEST		EAX,0xfffe0000 ; aligne 128 Kb
		JZ			%%MemDMA16Ok
		MOV			AX,0x101   ; DX contient deja selecteur
		INT			0x31
		JC			%%MemDMA16Error
		MOV			AX,0x100   ; alloue nouveau Bloc 2*plus grand
		MOV			BX,(%1>>4)*2
		INT			0x31
		JC			%%MemDMA16Error
		AND			EAX,0xffff
		SHL			EAX,4      ; adresse physique bloc dans EAX
		ADD			EAX,(%1>>4)
		AND			EAX,0xfffe0000
		CALL		GetBaseAddress
		MOV			[EBX+PhysAddDMA16],EAX
		MOV			[EBX+SelectorDMA16],EDX
		JMP			SHORT %%MemDMA16Ok

%%MemDMA16Error:
		XOR			EAX,EAX
		JMP			SHORT %%FinAllocMem
%%MemDMA16Ok:
		CALL		GetBaseAddress
		MOV			EAX,[EBX+PhysAddDMA16]
%%FinAllocMem:
%endmacro

%macro @StartDMA8 1
		XOR			EAX,EAX
		MOV			AL,%1
		CALL		StartDMA8
%endmacro


StartDMA8:
		MOV			EDI,EAX
		CALL		GetBaseAddress

		XOR			EDX,EDX
		MOV			ECX,[EBX+SbDMA8]; Mask Canal DMA ------------
		MOV			DL,[EBX+ECX+SingleMask]
		LEA			EAX,[ECX+4]
		OUT			DX,AL

		MOV			DL,[EBX+ECX+TransferMode]; Mode Transfert ---
		LEA			EAX,[EDI+ECX] ; Mode + num Canal DMA
		OUT			DX,AL

		MOV			DL,[EBX+ECX+ClearFlipFlop]; Clear flipflop --
		OUT			DX,AL

		MOV			DL,[EBX+ECX+Address]; Offset ----------------
		MOV			AL,[EBX+OffDMA8]
		OUT			DX,AL
		MOV			AL,[EBX+OffDMA8+1]
		OUT			DX,AL

		MOV			DL,[EBX+ECX+PageAddress]; Page --------------
		MOV			EAX,[EBX+PageDMA8]
		OUT			DX,AX

		MOV			DL,[EBX+ECX+ClearFlipFlop]; Clear flipflop --
		OUT			DX,AL

		MOV			DL,[EBX+ECX+Length]; Taille -----------------
		MOV			AL,((SizeDMABuff-1) & 0xff)
		OUT			DX,AL
		MOV			AL,(((SizeDMABuff-1) >> 8) & 0xff)
		OUT			DX,AL

		MOV			ECX,[EBX+SbDMA8]; Demask Canal DMA ----------
		MOV			DL,[EBX+ECX+SingleMask]
		LEA			EAX,[ECX]
		OUT			DX,AL

		RET

%macro @StartDMA16 1
		XOR			EAX,EAX
		MOV			AL,%1
		CALL		StartDMA16
%endmacro

StartDMA16:
		MOV			EDI,EAX
		CALL		GetBaseAddress

		XOR			EDX,EDX
		MOV			ECX,[EBX+SbDMA16]; Mask Canal DMA -----------
		MOV			DL,[EBX+ECX+SingleMask]
		LEA			EAX,[ECX] ; +4-4 = 0
		OUT			DX,AL

		MOV			DL,[EBX+ECX+TransferMode]; Mode Transfert ---
		LEA			EAX,[EDI+ECX-4] ; Mode + (num Canal DMA - 4)
		OUT			DX,AL

		MOV			DL,[EBX+ECX+ClearFlipFlop]; Clear flipflop --
		OUT			DX,AL

		MOV			DL,[EBX+ECX+Address]; Offset ----------------
		MOV			AL,[EBX+OffDMA16]
		OUT			DX,AL
		MOV			AL,[EBX+OffDMA16+1]
		OUT			DX,AL

		MOV			DL,[EBX+ECX+PageAddress]; Page --------------
		MOV			EAX,[EBX+PageDMA16]
		OUT			DX,AX

		MOV			DL,[EBX+ECX+ClearFlipFlop]; Clear flipflop --
		OUT			DX,AL

		MOV			DL,[EBX+ECX+Length]; Taille -----------------
		MOV			AL,((SizeDMABuff-1) & 0xff)
		OUT			DX,AL
		MOV			AL,(((SizeDMABuff-1) >> 8) & 0xff)
		OUT			DX,AL

		MOV			ECX,[EBX+SbDMA16]; Demask Canal DMA ---------
		MOV			DL,[EBX+ECX+SingleMask]
		LEA			EAX,[ECX-4]
		OUT			DX,AL

		RET

%macro @StopDMA8 0
		CALL		StopDMA8
%endmacro

StopDMA8:
		CALL		GetBaseAddress
		XOR			EDX,EDX
		MOV			ECX,[EBX+SbDMA8]; Mask Canal DMA ------------
		MOV			DL,[EBX+ECX+SingleMask]
		LEA			EAX,[ECX+4]
		OUT			DX,AL
		RET

%macro @StopDMA16 0
		CALL		StopDMA16
%endmacro


StopDMA16:
		CALL		GetBaseAddress
		XOR			EDX,EDX
		MOV			ECX,[EBX+SbDMA16]; Mask Canal DMA -----------
		MOV			DL,[EBX+ECX+SingleMask]
		LEA			EAX,[ECX] ; +4-4 = 0
		OUT			DX,AL
		RET

