ALIGN 16
_PutSurf16:
	ARG	SSN16, 4, XPSN16, 4, YPSN16, 4, PSType16, 4
		PUSH		EBX
		PUSH		EDI
		PUSH		ESI

		MOV			ESI,[EBP+SSN16]
		MOV			EDI,_SrcSurf

		CopySurf	; copy surf

		MOV			EAX,[EBP+XPSN16]
		MOV			EBX,[EBP+YPSN16]
		MOV			ECX,EAX
		MOV			EDX,EBX

		MOV         ESI,[EBP+PSType16]
		TEST        ESI,1
		JZ          .NormHzPut
		SUB         EAX,[SMinX]
		SUB         ECX,[SMaxX]
		JMP         SHORT .InvHzPut
.NormHzPut:
		ADD			EAX,[SMaxX] ; EAX = PutMaxX
		ADD			ECX,[SMinX] ; ECX = PutMinX
.InvHzPut:
		TEST        ESI,2
		JZ          .NormVtPut
		SUB         EBX,[SMinY]
		SUB         EDX,[SMaxY]
		JMP         SHORT .InvVtPut
.NormVtPut:
		ADD			EBX,[SMaxY] ; EBX = PutMaxY
		ADD			EDX,[SMinY] ; EDX = PutMinY
.InvVtPut:

		CMP			EAX,[_MinX]
		JL			.PasPutSurf
		CMP			EBX,[_MinY]
		JL			.PasPutSurf
		CMP			ECX,[_MaxX]
		JG			.PasPutSurf
		CMP			EDX,[_MaxY]
		JG			.PasPutSurf

		MOV			[PType],ESI ; save put Type

		; compute the clipped/unclipped put rectangle coordinaates in (PutSurfMinX, PutSurfMinY, PutSurfMaxX, PutSurfMaxY)
		;==========================================
		ClipStorePutSurfCoords

		;=========================
		; --- compute Put coordinates of the entire SrcSurf (as if source surf is full view)
		MOV         EAX,[EBP+XPSN16]  ; EAX = PutMaxX / without clipping
		MOV         EBX,[EBP+YPSN16]  ; EBX = PutMaxY / without clipping
		MOV			ESI,[PType] ; restore PType
		ComputeFullViewSrcSurfPutCoords

		CMP			EAX,[PutSurfMaxX]
		JG			.PutSurfClip
		CMP			EBX,[PutSurfMaxY]
		JG			.PutSurfClip
		CMP			ECX,[PutSurfMinX]
		JL			.PutSurfClip
		CMP			EDX,[PutSurfMinY]
		JL			.PutSurfClip

; PutSurf non Clipper *****************************
		MOV			EBP,[SResV]
		TEST		ESI,2 ; vertically reversed ?
		JZ			.NormAdSPut
		MOV			ESI,[Srlfb]
		MOV			EAX,[SScanLine]
		ADD			ESI,[SSizeSurf] ; ESI start of the last line in the surf
		SUB			ESI,EAX
		ADD			EAX,EAX
		NEG			EAX
		JMP			SHORT .InvAdSPut
.NormAdSPut:
		XOR			EAX,EAX
		MOV			ESI,[Srlfb] ; ESI : start copy adress
.InvAdSPut:
		MOV			EDI,EBX ; PutMaxY or the top left corner
		IMUL		EDI,[_NegScanLine]
		MOV			EDX,[_ScanLine]
		LEA			EDI,[EDI+ECX*2] ; += PutMinX*2 top left croner
		SUB			EDX,[SScanLine] ; EDX : dest adress plus
		ADD			EDI,[_vlfb]
		MOV			[Plus2],EDX

		TEST		BYTE [PType],1
		MOV			EDX,[SResH]
		JNZ         .InvHzPSurf
		MOV			[Plus],EAX

.BcPutSurf:
		MOV			EBX,EDX ; = [SResH]
		TEST		EDI,2  		; dword aligned ?
		JZ			.FPasStBAv
		DEC			EBX
		MOVSW
		JZ			.FinSHLine
.FPasStBAv:
		TEST		EDI,4 		; qword aligned ?
		JZ			.PasStDAv
		CMP			EBX,1
		JLE			.StBAp
		MOVSD
		SUB			EBX,BYTE 2
.PasStDAv:
		MOV			ECX,EBX
		SHR			ECX,2
		JZ			.StDAp
;ALIGN 4
.StoMMX:
		MOVQ		mm0,[ESI]
		DEC			ECX
		MOVQ		[EDI],mm0
		LEA			ESI,[ESI+8]
		LEA			EDI,[EDI+8]
		JNZ			.StoMMX
		AND			EBX,BYTE 3
		JZ			.FinSHLine
.StDAp:
		CMP			EBX,2
		JL			.StBAp
		SUB			EBX,BYTE 2
		MOVSD
.StBAp:
		OR			EBX,EBX
		JZ			.PasStBAp
		MOVSW
.PasStBAp:
.FinSHLine:
		ADD			EDI,[Plus2]
		ADD			ESI,[Plus]
		DEC			EBP
		JNZ			.BcPutSurf

		JMP			.PasPutSurf

; Put surf unclipped reversed horizontally *************
.InvHzPSurf:
		LEA			EAX,[EAX+EDX*4] ; +=SScanLine*2
		LEA			ESI,[ESI+EDX*2] ; +=SScanLine
		MOV			[Plus],EAX

.IBcPutSurf:
		MOV			EBX,EDX
.IBcStBAv:
		TEST		EDI,2
		JZ			.IFPasStBAv
		SUB			ESI, BYTE 2
		MOV			AX,[ESI]
		DEC			EBX
		STOSW
		JZ			.IFinSHLine
.IFPasStBAv:
		TEST		EDI,4
		JZ			.IPasStDAv
		CMP			EBX,1
		JLE			.IStBAp
		SUB			ESI,BYTE 4
		MOV			EAX,[ESI]
		SUB			EBX,BYTE 2
		ROR			EAX,16 ; swap word order
		STOSD
.IPasStDAv:
		;MOV			ECX,EBX
		;SHR			ECX,2
		;JZ			.IStDAp
;ALIGN 4
.IStoMMX:
		CMP			EBX,BYTE 3
		JLE			SHORT .IStDAp

		SUB			ESI,BYTE 8
		MOV			EAX,[ESI]
		MOV			ECX,[ESI+4]
		ROR			EAX,16
		ROR			ECX,16
		MOVD		mm1,EAX
		MOVD		mm0,ECX
		SUB			EBX, BYTE 4
		PUNPCKLDQ	mm0,mm1
		MOVQ		[EDI],mm0
		LEA			EDI,[EDI+8]

		JMP			.IStoMMX

		AND			EBX,BYTE 3
		JZ			.IFinSHLine
.IStDAp:
		CMP			EBX,2
		JL			.IStBAp
		SUB			ESI,4
		MOV			EAX,[ESI]
		ROR			EAX,16 ; SWap word order
		STOSD
		SUB			EBX,BYTE 2

.IStBAp:
		OR			EBX,EBX
		JZ			.IPasStBAp
.IBcStBAp:
		SUB			ESI, BYTE 2
		MOV			AX,[ESI]
		DEC			EBX
		STOSW
.IPasStBAp:
.IFinSHLine:
		ADD			EDI,[Plus2]
		ADD			ESI,[Plus]
		DEC			EBP
		JNZ			.IBcPutSurf

.PasPutSurf:
		POP		ESI
		POP		EDI
		POP		EBX

	MMX_RETURN

.PutSurfClip:
; PutSurf Clipper **********************************************
		XOR			EDI,EDI   ; Y Fin Source
		XOR			ESI,ESI   ; X deb Source

		MOV			EBP,[PutSurfMinX]
		CMP			ECX,EBP ; CMP minx, _MinX
		JGE			.PsInfMinX   ; XP1<_MinX
		TEST		BYTE [PType],1
		JNZ			.InvHzCalcDX
		MOV         ESI,EBP
		SUB         ESI,ECX ; ESI = MinX - XP2
		MOV         ECX,EBP
.InvHzCalcDX:
		MOV			ECX,EBP
.PsInfMinX:
		MOV			EBP,[PutSurfMaxY]
		CMP			EBX,EBP ; cmp maxy, _MaxY
		JLE			.PsSupMaxY   ; YP2>_MaxY
		MOV			EDI,EBP
		NEG			EDI
		;MOV		[YP2],EBP
		ADD			EDI,EBX
		MOV			EBX,EBP
.PsSupMaxY:
		MOV			EBP,[PutSurfMinY]
		CMP			EDX,EBP      ; YP1<_MinY
		JGE			.PsInfMinY
		MOV			EDX,EBP
.PsInfMinY:
		MOV			EBP,[PutSurfMaxX]
		CMP			EAX,EBP      ; XP2>_MaxX
		JLE			.PsSupMaxX
		TEST		BYTE [PType],1
		JZ			.PsInvHzCalcDX
		MOV			ESI,EAX
		SUB			ESI,EBP	; ESI = XP2 - _MaxX
.PsInvHzCalcDX:
		MOV			EAX,EBP
.PsSupMaxX:
		SUB			EAX,ECX      ; XP2 - XP1
		MOV			EBP,[SScanLine]
		LEA			EAX,[EAX*2+2]
		SUB			EBP,EAX  ; EBP = SResH-DeltaX, PlusSSurf
		MOV			[Plus],EBP
		MOV			EBP,EBX
		SUB			EBP,EDX      ; YP2 - YP1
		INC			EBP   ; EBP = DeltaY
		MOV			EDX,[_ScanLine]
		MOVD		mm0,EAX ; = DeltaX
		SUB			EDX,EAX ; EDX = _ResH-DeltaX, PlusDSurfS
		TEST		BYTE [PType],2
		MOV			[Plus2],EDX
		JZ			.CNormAdSPut
		MOV			EAX,[Srlfb] ; Si inverse vertical
		ADD			EAX,[SSizeSurf] ; go to the last buffer
		SUB			EAX,[SScanLine] ; jump to the first of the last line
		LEA			EAX,[EAX+ESI*2] ; +X1InSSurf*2 clipping
		IMUL		EDI,[SScanLine] ; Y1InSSurf*ScanLine
		SUB			EAX,EDI
		MOV			ESI,EAX

		MOV			EAX,[SScanLine]
		ADD			EAX,EAX
		NEG			EAX
		JMP			SHORT .CInvAdSPut
.CNormAdSPut:
		IMUL		EDI,[SScanLine]
		XOR			EAX,EAX
		LEA			EDI,[EDI+ESI*2]
		ADD			EDI,[Srlfb]
		MOV			ESI,EDI
.CInvAdSPut:
		MOV			EDI,EBX
		IMUL		EDI,[_NegScanLine]
		LEA			EDI,[EDI+ECX*2] ; + XP1*2 as 16bpp
		PSRLD		mm0,1 ; (deltaX*2) / 2
		ADD			EDI,[_vlfb]

		MOVD		EDX,mm0  ; DeltaX
		TEST		BYTE [PType],1
		JNZ        .CInvHzPSurf
		ADD			[Plus],EAX
		JMP			.BcPutSurf

.CInvHzPSurf:   ; clipper et inverser horizontalement

		ADD			EAX,[SScanLine]
		LEA			EAX,[EAX+EDX*2] ; add to jump to the end
		LEA			ESI,[ESI+EDX*2] ; jump to the end
		MOV			[Plus],EAX
		JMP			.IBcPutSurf



; PUT masked Surf
;*****************

ALIGN 16
_PutMaskSurf16:
	ARG	MSSN16, 4, MXPSN16, 4, MYPSN16, 4, MPSType16, 4
		PUSH		EBX
		PUSH		EDI
		PUSH		ESI

		MOV			EAX,[_MinX]
		MOV			EBX,[_MinY]
		CMP			EAX,[_MaxX]
		MOV			ESI,[EBP+MSSN16]
		JG			.PasPutSurf
		CMP			EBX,[_MaxY]
		MOV			EDI,_SrcSurf
		JG			.PasPutSurf

		CopySurf	; copy surf

		MOVD		mm7,[SMask]
		PUNPCKLWD	mm7,mm7
		PUNPCKLDQ	mm7,mm7 ; = [QSMask16]

		MOV			EAX,[EBP+MXPSN16]
		MOV			EBX,[EBP+MYPSN16]
		MOV			ECX,EAX
		MOV			EDX,EBX

		MOV         ESI,[EBP+MPSType16]
		TEST        ESI,1
		JZ          .NormHzPut
		SUB         EAX,[SMinX]
		SUB         ECX,[SMaxX]
		JMP         SHORT .InvHzPut
.NormHzPut:
		ADD			EAX,[SMaxX] ; EAX = PutMaxX
		ADD			ECX,[SMinX] ; ECX = PutMinX
.InvHzPut:
		TEST        ESI,2
		JZ          .NormVtPut
		SUB         EBX,[SMinY]
		SUB         EDX,[SMaxY]
		JMP         SHORT .InvVtPut
.NormVtPut:
		ADD			EBX,[SMaxY] ; EBX = PutMaxY
		ADD			EDX,[SMinY] ; EDX = PutMinY
.InvVtPut:
		CMP			EAX,[_MinX]
		JL			.PasPutSurf
		CMP			EBX,[_MinY]
		JL			.PasPutSurf
		CMP			ECX,[_MaxX]
		JG			.PasPutSurf
		CMP			EDX,[_MaxY]
		JG			.PasPutSurf

		MOV			[PType],ESI ; save put Type

		; compute the clipped/unclipped put rectangle coordinaates in (PutSurfMinX, PutSurfMinY, PutSurfMaxX, PutSurfMaxY)
		;==========================================
		ClipStorePutSurfCoords

		;=========================
		; --- compute Put coordinates of the entire SrcSurf (as if source surf is full view)
		MOV         EAX,[EBP+MXPSN16]  ; EAX = PutMaxX / without clipping
		MOV         EBX,[EBP+MYPSN16]  ; EBX = PutMaxY / without clipping
		MOV			ESI,[PType] ; restore PType
		ComputeFullViewSrcSurfPutCoords

		CMP			EAX,[PutSurfMaxX]
		JG			.PutSurfClip
		CMP			EBX,[PutSurfMaxY]
		JG			.PutSurfClip
		CMP			ECX,[PutSurfMinX]
		JL			.PutSurfClip
		CMP			EDX,[PutSurfMinY]
		JL			.PutSurfClip
; PutSurf non Clipper *****************************
		MOV			[PType],ESI
		MOV			EBP,[SResV]
		TEST		ESI,2 ; vertically reversed ?
		JZ			.NormAdSPut
		MOV			ESI,[Srlfb]
		MOV			EAX,[SScanLine]
		ADD			ESI,[SSizeSurf] ; ESI start of the last line in the surf
		SUB			ESI,EAX
		ADD			EAX,EAX
		NEG			EAX
		JMP			SHORT .InvAdSPut
.NormAdSPut:
		XOR			EAX,EAX
		MOV			ESI,[Srlfb] ; ESI : start copy adress
.InvAdSPut:
		MOV			EDI,EBX ; PutMaxY or the top left corner
		IMUL		EDI,[_NegScanLine]
		MOV			EDX,[_ScanLine]
		LEA			EDI,[EDI+ECX*2] ; += PutMinX*2 top left croner
		SUB			EDX,[SScanLine] ; EDX : dest adress plus
		ADD			EDI,[_vlfb]
		MOV			[Plus2],EDX

		TEST		BYTE [PType],1
		MOV			EDX,[SResH]
		JNZ         .InvHzPSurf
		MOV			[Plus],EAX

.BcPutSurf:
		MOV			EBX,EDX ; = [SResH]

.BcStBAv:
		TEST		EDI,6  		; qword aligned ?
		JZ			.FPasStBAv
		MOV			AX,[ESI]
		CMP			AX,[SMask]
		JE			.MaskBAv
		MOV			[EDI],AX
.MaskBAv:
		DEC			EBX
		LEA			ESI,[ESI+2]
		LEA			EDI,[EDI+2]
		JZ			.FinSHLine
		JMP			SHORT .BcStBAv
.FPasStBAv:

.StoMMX:
		CMP			EBX,BYTE 3
		JLE			.StBAp

		MOVQ		mm0,[ESI]
        MOVQ        mm6,[EDI]
        MOVQ      	mm2,[ESI]
        MOVQ        mm1,[ESI]

        PCMPEQW     mm2,mm7; [QSMask16]
        PCMPEQW     mm1,mm7; [QSMask16]
        PANDN       mm2,mm0
        PAND        mm6,mm1
        POR         mm2,mm6

		SUB			EBX, BYTE 4
		MOVQ		[EDI],mm2
		LEA			ESI,[ESI+8]
		LEA			EDI,[EDI+8]
		JMP			.StoMMX
.StBAp:
		AND			EBX,BYTE 3
		JZ			.FinSHLine
.BcStBAp:
		MOV			AX,[ESI]
		CMP			AX,[SMask]
		JE			.MaskBAp
		MOV			[EDI],AX
.MaskBAp:
		DEC			EBX
		LEA			ESI,[ESI+2]
		LEA			EDI,[EDI+2]
		JNZ			.BcStBAp
.PasStBAp:

.FinSHLine:
		ADD			EDI,[Plus2]
		ADD			ESI,[Plus]
		DEC			EBP
		JNZ			.BcPutSurf

		JMP			.PasPutSurf

; Put surf unclipped reversed horizontally *************
.InvHzPSurf:
		LEA			EAX,[EAX+EDX*4] ; +=SScanLine*2
		LEA			ESI,[ESI+EDX*2] ; +=SScanLine
		MOV			[Plus],EAX

.IBcPutSurf:
		MOV			EBX,EDX
.IBcStBAv:
		TEST		EDI,6
		JZ			.IFPasStBAv
		SUB			ESI, BYTE 2
		MOV			AX,[ESI]
		CMP			AX,[SMask]
		JE			.IMaskStBAv
		MOV			[EDI],AX
.IMaskStBAv:
		DEC			EBX
		LEA			EDI,[EDI+2]
		JZ			.IFinSHLine
		JMP			SHORT .IBcStBAv
.IFPasStBAv:
.IStoMMX:
		CMP			EBX,BYTE 3
		JLE			.IStBAp

		SUB			ESI,BYTE 8
		MOV			EAX,[ESI]
		MOV			ECX,[ESI+4]
		ROR			EAX,16
		ROR			ECX,16
		MOVD		mm1,EAX
		MOVD		mm0,ECX
		PUNPCKLDQ	mm0,mm1

        MOVQ        mm6,[EDI]
        MOVQ      	mm2,mm0 ; [QHLineOrg]
        MOVQ        mm1,mm0 ; [QHLineOrg]

        PCMPEQW     mm2,mm7; [QSMask16]
        PCMPEQW     mm1,mm7; [QSMask16]
        PANDN       mm2,mm0
        PAND        mm6,mm1
        POR         mm2,mm6

		MOVQ		[EDI],mm2
		SUB			EBX, BYTE 4
		LEA			EDI,[EDI+8]
		JMP			.IStoMMX
.IStBAp:
		AND			EBX,BYTE 3
		JZ			.IFinSHLine
.IBcStBAp:
		SUB			ESI, BYTE 2
		MOV			AX,[ESI]
		CMP			AX,[SMask]
		JE			SHORT .IMaskStBAp
		MOV			[EDI],AX
.IMaskStBAp:
		DEC			EBX
		LEA			EDI,[EDI+2]
		JNZ			.IBcStBAp
.IPasStBAp:
.IFinSHLine:
		ADD			EDI,[Plus2]
		ADD			ESI,[Plus]
		DEC			EBP
		JNZ			.IBcPutSurf

.PasPutSurf:
		POP		ESI
		POP		EDI
		POP		EBX
	MMX_RETURN

.PutSurfClip:
; PutSurf Clipper **********************************************
		MOV			[PType],ESI ; sauvegarde le type
		XOR			EDI,EDI   ; Y Fin Source
		XOR			ESI,ESI   ; X deb Source

		MOV			EBP,[PutSurfMinX]
		CMP			ECX,EBP ; CMP minx, _MinX
		JGE			.PsInfMinX   ; XP1<_MinX
		TEST		BYTE [PType],1
		JNZ			.InvHzCalcDX
		MOV         ESI,EBP
		SUB         ESI,ECX ; ESI = MinX - XP2
		MOV         ECX,EBP
.InvHzCalcDX:
		MOV			ECX,EBP
.PsInfMinX:
		MOV			EBP,[PutSurfMaxY]
		CMP			EBX,EBP ; cmp maxy, _MaxY
		JLE			.PsSupMaxY   ; YP2>_MaxY
		MOV			EDI,EBP
		NEG			EDI
		;MOV		[YP2],EBP
		ADD			EDI,EBX
		MOV			EBX,EBP
.PsSupMaxY:
		MOV			EBP,[PutSurfMinY]
		CMP			EDX,EBP      ; YP1<_MinY
		JGE			.PsInfMinY
		MOV			EDX,EBP
.PsInfMinY:
		MOV			EBP,[PutSurfMaxX]
		CMP			EAX,EBP      ; XP2>_MaxX
		JLE			.PsSupMaxX
        TEST		BYTE [PType],1
        JZ			.PsInvHzCalcDX
		MOV			ESI,EAX
		SUB			ESI,EBP	; ESI = XP2 - _MaxX
.PsInvHzCalcDX:
		MOV			EAX,EBP
.PsSupMaxX:
		SUB			EAX,ECX      ; XP2 - XP1
		MOV			EBP,[SScanLine]
		LEA			EAX,[EAX*2+2]
		SUB			EBP,EAX  ; EBP = SResH-DeltaX, PlusSSurf
		MOV			[Plus],EBP
		MOV			EBP,EBX
		SUB			EBP,EDX      ; YP2 - YP1
		INC			EBP   ; EBP = DeltaY
		MOV			EDX,[_ScanLine]
		MOVD		mm0,EAX ; = DeltaX
		SUB			EDX,EAX ; EDX = _ResH-DeltaX, PlusDSurfS
		TEST		BYTE [PType],2
		MOV			[Plus2],EDX
		JZ			.CNormAdSPut
		MOV			EAX,[Srlfb] ; Si inverse vertical
		ADD			EAX,[SSizeSurf] ; go to the last buffer
		SUB			EAX,[SScanLine] ; jump to the first of the last line
		LEA			EAX,[EAX+ESI*2] ; +X1InSSurf*2 clipping
		IMUL		EDI,[SScanLine] ; Y1InSSurf*ScanLine
		SUB			EAX,EDI
		MOV			ESI,EAX

		MOV			EAX,[SScanLine]
		ADD			EAX,EAX
		NEG			EAX
		JMP			SHORT .CInvAdSPut
.CNormAdSPut:
		IMUL		EDI,[SScanLine]
		XOR			EAX,EAX
		LEA			EDI,[EDI+ESI*2]
		ADD			EDI,[Srlfb]
		MOV			ESI,EDI
.CInvAdSPut:
		MOV			EDI,EBX
		IMUL		EDI,[_NegScanLine]
		LEA			EDI,[EDI+ECX*2] ; + XP1*2 as 16bpp
		PSRLD		mm0,1 ; (deltaX*2) / 2
		ADD			EDI,[_vlfb]

		MOVD		EDX,mm0  ; DeltaX
		TEST		BYTE [PType],1
		JNZ        .CInvHzPSurf
		ADD			[Plus],EAX
		JMP			.BcPutSurf

.CInvHzPSurf:   ; clipper et inverser horizontalement

		ADD			EAX,[SScanLine]
		LEA			EAX,[EAX+EDX*2] ; add to jump to the end
		LEA			ESI,[ESI+EDX*2] ; jump to the end
		MOV			[Plus],EAX
		JMP			.IBcPutSurf

; -------------------------------
; Put a Surf blended with a color
; -------------------------------
ALIGN 16
_PutSurfBlnd16:
	ARG	SSBN16, 4, XPSBN16, 4, YPSBN16, 4, PSBType16, 4, PSBCol16, 4
		PUSH		EBX
		PUSH		EDI
		PUSH		ESI

		MOV			ESI,[EBP+SSBN16]
		MOV			EDI,_SrcSurf

		CopySurf	; copy surf

; prepare col blending
		MOV			EAX,[EBP+PSBCol16] ;
		MOV			EBX,EAX ;
		MOV			ECX,EAX ;
		MOV			EDX,EAX ;
		AND			EBX,[QBlue16Mask] ; EBX = Bclr16 | Bclr16
		SHR			EAX,24
		AND			ECX,[QGreen16Mask] ; ECX = Gclr16 | Gclr16
		AND			AL,BlendMask ; remove any ineeded bits
		AND			EDX,[QRed16Mask] ; EDX = Rclr16 | Rclr16
		XOR			AL,BlendMask ; 31-blendsrc
		MOV			DI,AX
		SHL			EDI,16
		OR			DI,AX
		XOR			AL,BlendMask ; 31-blendsrc
		INC			AL
		SHR			DX,5 ; right shift red 5bits
		IMUL		BX,AX
		IMUL		CX,AX
		IMUL		DX,AX
		MOVD		mm3,EBX
		MOVD		mm4,ECX
		MOVD		mm5,EDX
		PUNPCKLWD	mm3,mm3
		PUNPCKLWD	mm4,mm4
		PUNPCKLWD	mm5,mm5
		PUNPCKLDQ	mm3,mm3
		PUNPCKLDQ	mm4,mm4
		MOVD		mm7,EDI
		PUNPCKLDQ	mm5,mm5
		PUNPCKLDQ	mm7,mm7

		MOV			EAX,[EBP+XPSBN16]
		MOV			EBX,[EBP+YPSBN16]
		MOV			ECX,EAX
		MOV			EDX,EBX

		MOV         ESI,[EBP+PSBType16]
		TEST        ESI,1
		JZ          SHORT .NormHzPut
		SUB         EAX,[SMinX]
		SUB         ECX,[SMaxX]
		JMP         SHORT .InvHzPut
.NormHzPut:
		ADD			EAX,[SMaxX] ; EAX = PutMaxX
		ADD			ECX,[SMinX] ; ECX = PutMinX
.InvHzPut:
		TEST        ESI,2
		JZ          SHORT .NormVtPut
		SUB         EBX,[SMinY]
		SUB         EDX,[SMaxY]
		JMP         SHORT .InvVtPut
.NormVtPut:
		ADD			EBX,[SMaxY] ; EBX = PutMaxY
		ADD			EDX,[SMinY] ; EDX = PutMinY
.InvVtPut:

		CMP			EAX,[_MinX]
		JL			.PasPutSurf
		CMP			EBX,[_MinY]
		JL			.PasPutSurf
		CMP			ECX,[_MaxX]
		JG			.PasPutSurf
		CMP			EDX,[_MaxY]
		JG			.PasPutSurf

		MOV			[PType],ESI ; save put Type

		; compute the clipped/unclipped put rectangle coordinaates in (PutSurfMinX, PutSurfMinY, PutSurfMaxX, PutSurfMaxY)
		;==========================================
		ClipStorePutSurfCoords

		;=========================
		; --- compute Put coordinates of the entire SrcSurf (as if source surf is full view)
		MOV         EAX,[EBP+XPSBN16]  ; EAX = PutMaxX / without clipping
		MOV         EBX,[EBP+YPSBN16]  ; EBX = PutMaxY / without clipping
		MOV			ESI,[PType] ; restore PType
		ComputeFullViewSrcSurfPutCoords

		CMP			EAX,[PutSurfMaxX]
		JG			.PutSurfClip
		CMP			EBX,[PutSurfMaxY]
		JG			.PutSurfClip
		CMP			ECX,[PutSurfMinX]
		JL			.PutSurfClip
		CMP			EDX,[PutSurfMinY]
		JL			.PutSurfClip

; PutSurf non Clipper *****************************
		MOV			EBP,[SResV]
		TEST		ESI,2 ; vertically reversed ?
		JZ			SHORT .NormAdSPut
		MOV			ESI,[Srlfb]
		MOV			EAX,[SScanLine]
		ADD			ESI,[SSizeSurf] ; ESI start of the last line in the surf
		SUB			ESI,EAX
		ADD			EAX,EAX
		NEG			EAX
		JMP			SHORT .InvAdSPut
.NormAdSPut:
		XOR			EAX,EAX
		MOV			ESI,[Srlfb] ; ESI : start copy adress
.InvAdSPut:
		MOV			EDI,EBX ; PutMaxY or the top left corner
		IMUL		EDI,[_NegScanLine]
		MOV			EDX,[_ScanLine]
		LEA			EDI,[EDI+ECX*2] ; += PutMinX*2 top left croner
		SUB			EDX,[SScanLine] ; EDX : dest adress plus
		ADD			EDI,[_vlfb]
		MOV			[Plus2],EDX

		TEST		BYTE [PType],1
		MOV			EDX,[SResH]
		JNZ         .InvHzPSurf
		MOV			[Plus],EAX

.BcPutSurf:
		MOV			EBX,EDX ; = [SResH]
.BcStBAv:
		TEST		EDI,6  		; dword aligned ?
		JZ			.FPasStBAv
		MOV			AX,[ESI]
		DEC			EBX
		MOVD		mm0,EAX
		MOVD		mm1,EAX
		MOVD		mm2,EAX
		@SolidBlndQ
		MOVD		EAX,mm0
		LEA			ESI,[ESI+2]
		STOSW
		JZ			.FinSHLine
		JMP			SHORT .BcStBAv
.FPasStBAv:
		MOV			ECX,EBX
		SHR			ECX,2
		JZ			.StBAp
;ALIGN 4
.StoMMX:
		MOVQ		mm0,[ESI]
		MOVQ		mm1,[ESI]
		MOVQ		mm2,[ESI]
		@SolidBlndQ
		DEC			ECX
		MOVQ		[EDI],mm0
		LEA			ESI,[ESI+8]
		LEA			EDI,[EDI+8]
		JNZ			.StoMMX
.StBAp:
		AND			EBX,BYTE 3
		JZ			.FinSHLine
.BcStBAp:
		MOV			AX,[ESI]
		DEC			EBX
		MOVD		mm0,EAX
		MOVD		mm1,EAX
		MOVD		mm2,EAX
		@SolidBlndQ
		MOVD		EAX,mm0
		LEA			ESI,[ESI+2]
		STOSW
		JNZ			.BcStBAp
.PasStBAp:
.FinSHLine:
		ADD			EDI,[Plus2]
		ADD			ESI,[Plus]
		DEC			EBP
		JNZ			.BcPutSurf

		JMP			.PasPutSurf

; Put surf unclipped reversed horizontally *************
.InvHzPSurf:
		LEA			EAX,[EAX+EDX*4] ; +=SScanLine*2
		LEA			ESI,[ESI+EDX*2] ; +=SScanLine
		MOV			[Plus],EAX

.IBcPutSurf:
		MOV			EBX,EDX
.IBcStBAv:
		TEST		EDI,6
		JZ			.IFPasStBAv
		SUB			ESI, BYTE 2
		MOV			AX,[ESI]
		MOVD		mm0,EAX
		MOVD		mm1,EAX
		MOVD		mm2,EAX
		@SolidBlndQ
		DEC			EBX
		MOVD		EAX,mm0
		STOSW
		JZ			.IFinSHLine
		JMP			SHORT .IBcStBAv
.IFPasStBAv:
		MOV			ECX,EBX
		SHR			ECX,2
		JZ			.IStBAp
;ALIGN 4
.IStoMMX:
		CMP			EBX,BYTE 3
		JLE			SHORT .IStBAp

		SUB			ESI,BYTE 8
		MOV			EAX,[ESI]
		MOV			ECX,[ESI+4]
		ROR			EAX,16
		ROR			ECX,16
		MOVD		mm1,EAX
		MOVD		mm0,ECX
		SUB			EBX, BYTE 4
		PUNPCKLDQ	mm0,mm1
		MOVQ		mm1,mm0
		MOVQ		mm2,mm0
		@SolidBlndQ
		MOVQ		[EDI],mm0
		LEA			EDI,[EDI+8]
		JNZ			.IStoMMX
.IStBAp:
		AND			EBX,BYTE 3
		JZ			.IFinSHLine
.IBcStBAp:
		SUB			ESI, BYTE 2
		MOV			AX,[ESI]
		MOVD		mm0,EAX
		MOVD		mm1,EAX
		MOVD		mm2,EAX
		@SolidBlndQ
		MOVD		EAX,mm0
		DEC			EBX
		STOSW
		JNZ			.IBcStBAp
.IPasStBAp:
.IFinSHLine:
		ADD			EDI,[Plus2]
		ADD			ESI,[Plus]
		DEC			EBP
		JNZ			.IBcPutSurf

.PasPutSurf:
		POP			ESI
		POP			EDI
		POP			EBX
	MMX_RETURN

.PutSurfClip:
; PutSurf Clipper **********************************************
		XOR			EDI,EDI   ; Y Fin Source
		XOR			ESI,ESI   ; X deb Source

		MOV			EBP,[PutSurfMinX]
		CMP			ECX,EBP ; CMP minx, _MinX
		JGE			.PsInfMinX   ; XP1<_MinX
		TEST		BYTE [PType],1
		JNZ			.InvHzCalcDX
		MOV         ESI,EBP
		SUB         ESI,ECX ; ESI = MinX - XP2
		MOV         ECX,EBP
.InvHzCalcDX:
		MOV			ECX,EBP
.PsInfMinX:
		MOV			EBP,[PutSurfMaxY]
		CMP			EBX,EBP ; cmp maxy, _MaxY
		JLE			.PsSupMaxY   ; YP2>_MaxY
		MOV			EDI,EBP
		NEG			EDI
		;MOV		[YP2],EBP
		ADD			EDI,EBX
		MOV			EBX,EBP
.PsSupMaxY:
		MOV			EBP,[PutSurfMinY]
		CMP			EDX,EBP      ; YP1<_MinY
		JGE			.PsInfMinY
		MOV			EDX,EBP
.PsInfMinY:
		MOV			EBP,[PutSurfMaxX]
		CMP			EAX,EBP      ; XP2>_MaxX
		JLE			.PsSupMaxX
		TEST		BYTE [PType],1
		JZ			.PsInvHzCalcDX
		MOV			ESI,EAX
		SUB			ESI,EBP	; ESI = XP2 - _MaxX
.PsInvHzCalcDX:
		MOV			EAX,EBP
.PsSupMaxX:
		SUB			EAX,ECX      ; XP2 - XP1
		MOV			EBP,[SScanLine]
		LEA			EAX,[EAX*2+2]
		SUB			EBP,EAX  ; EBP = SResH-DeltaX, PlusSSurf
		MOV			[Plus],EBP
		MOV			EBP,EBX
		SUB			EBP,EDX      ; YP2 - YP1
		INC			EBP   ; EBP = DeltaY
		MOV			EDX,[_ScanLine]
		MOVD		mm0,EAX ; = DeltaX
		SUB			EDX,EAX ; EDX = _ResH-DeltaX, PlusDSurfS
		TEST		BYTE [PType],2
		MOV			[Plus2],EDX
		JZ			.CNormAdSPut
		MOV			EAX,[Srlfb] ; Si inverse vertical
		ADD			EAX,[SSizeSurf] ; go to the last buffer
		SUB			EAX,[SScanLine] ; jump to the first of the last line
		LEA			EAX,[EAX+ESI*2] ; +X1InSSurf*2 clipping
		IMUL		EDI,[SScanLine] ; Y1InSSurf*ScanLine
		SUB			EAX,EDI
		MOV			ESI,EAX

		MOV			EAX,[SScanLine]
		ADD			EAX,EAX
		NEG			EAX
		JMP			SHORT .CInvAdSPut
.CNormAdSPut:
		IMUL		EDI,[SScanLine]
		XOR			EAX,EAX
		LEA			EDI,[EDI+ESI*2]
		ADD			EDI,[Srlfb]
		MOV			ESI,EDI
.CInvAdSPut:
		MOV			EDI,EBX
		IMUL		EDI,[_NegScanLine]
		LEA			EDI,[EDI+ECX*2] ; + XP1*2 as 16bpp
		PSRLD		mm0,1 ; (deltaX*2) / 2
		ADD			EDI,[_vlfb]

		MOVD		EDX,mm0  ; DeltaX
		TEST		BYTE [PType],1
		JNZ         .CInvHzPSurf
		ADD			[Plus],EAX
		JMP			.BcPutSurf

.CInvHzPSurf:   ; clipper et inverser horizontalement

		ADD			EAX,[SScanLine]
		LEA			EAX,[EAX+EDX*2] ; add to jump to the end
		LEA			ESI,[ESI+EDX*2] ; jump to the end
		MOV			[Plus],EAX
		JMP			.IBcPutSurf


ALIGN 16
_SurfCopyBlnd16:
	ARG	PDstSrfB, 4, PSrcSrfB, 4, SCBCol, 4
		PUSH		EDI
		PUSH	    ESI
		PUSH		EBX

; prepare col blending
		MOV			EAX,[EBP+SCBCol] ;
		MOV			EBX,EAX ;
		MOV			ECX,EAX ;
		MOV			EDX,EAX ;
		AND			EBX,[QBlue16Mask] ; EBX = Bclr16 | Bclr16
		SHR			EAX,24
		AND			ECX,[QGreen16Mask] ; ECX = Gclr16 | Gclr16
		AND			AL,BlendMask ; remove any ineeded bits
		AND			EDX,[QRed16Mask] ; EDX = Rclr16 | Rclr16
		XOR			AL,BlendMask ; 31-blendsrc
		MOV			DI,AX
		SHL			EDI,16
		OR			DI,AX
		XOR			AL,BlendMask ; 31-blendsrc
		INC			AL
		SHR			DX,5 ; right shift red 5bits
		IMUL		BX,AX
		IMUL		CX,AX
		IMUL		DX,AX
		MOVD		mm3,EBX
		MOVD		mm4,ECX
		MOVD		mm5,EDX
		PUNPCKLWD	mm3,mm3
		PUNPCKLWD	mm4,mm4
		PUNPCKLWD	mm5,mm5
		PUNPCKLDQ	mm3,mm3
		PUNPCKLDQ	mm4,mm4
		MOVD		mm7,EDI
		PUNPCKLDQ	mm5,mm5
		PUNPCKLDQ	mm7,mm7

		MOV			ESI,[EBP+PSrcSrfB]
		MOV			EDI,[EBP+PDstSrfB]
		MOV			EBX,[ESI+_SizeSurf-_CurSurf]

		MOV			EDI,[EDI+_rlfb-_CurSurf]
		SHR			EBX,1
		MOV			ESI,[ESI+_rlfb-_CurSurf]

.BcStBAv:
		TEST		EDI,6  		; dword aligned ?
		JZ			.FPasStBAv
		MOV			AX,[ESI]
		DEC			EBX
		MOVD		mm0,EAX
		MOVD		mm1,EAX
		MOVD		mm2,EAX
		@SolidBlndQ
		MOVD		EAX,mm0
		LEA			ESI,[ESI+2]
		STOSW
		JZ			.FinSurfCopy
		JMP			SHORT .BcStBAv
.FPasStBAv:
		MOV			ECX,EBX
		SHR			ECX,2
		JZ			.StBAp
;ALIGN 4
.StoMMX:
		MOVQ		mm0,[ESI]
		MOVQ		mm1,[ESI]
		MOVQ		mm2,[ESI]
		@SolidBlndQ
		DEC			ECX
		MOVQ		[EDI],mm0
		LEA			ESI,[ESI+8]
		LEA			EDI,[EDI+8]
		JNZ			.StoMMX
.StBAp:
		AND			EBX,BYTE 3
		JZ			.FinSurfCopy
.BcStBAp:
		MOV			AX,[ESI]
		DEC			EBX
		MOVD		mm0,EAX
		MOVD		mm1,EAX
		MOVD		mm2,EAX
		@SolidBlndQ
		MOVD		EAX,mm0
		LEA			ESI,[ESI+2]
		STOSW
		JNZ			.BcStBAp
.FinSurfCopy:

		POP		EBX
		POP		ESI
		POP		EDI
	MMX_RETURN

; =======================================
; Put a MASKED Surf blended with a color
; =======================================
ALIGN 16
_PutMaskSurfBlnd16:
	ARG	MSSBN16, 4, MXPSBN16, 4, MYPSBN16, 4, MPSBType16, 4, MPSBCol16, 4
		PUSH		EBX
		PUSH		EDI
		PUSH		ESI

		MOV			ESI,[EBP+MSSBN16]
		MOV			EDI,_SrcSurf

		CopySurf	; copy surf

; prepare col blending
		MOV			EAX,[EBP+MPSBCol16] ;
		MOV			EBX,EAX ;
		MOV			ECX,EAX ;
		MOV			EDX,EAX ;
		AND			EBX,[QBlue16Mask] ; EBX = Bclr16 | Bclr16
		SHR			EAX,24
		AND			ECX,[QGreen16Mask] ; ECX = Gclr16 | Gclr16
		AND			AL,BlendMask ; remove any ineeded bits
		AND			EDX,[QRed16Mask] ; EDX = Rclr16 | Rclr16
		XOR			AL,BlendMask ; 31-blendsrc
		MOV			DI,AX
		SHL			EDI,16
		OR			DI,AX
		XOR			AL,BlendMask ; 31-blendsrc
		INC			AL
		SHR			DX,5 ; right shift red 5bits
		IMUL		BX,AX
		IMUL		CX,AX
		IMUL		DX,AX
		MOVD		mm6,[SMask]
		MOVD		mm3,EBX
		MOVD		mm4,ECX
		MOVD		mm5,EDX
		PUNPCKLWD	mm6,mm6
		PUNPCKLWD	mm3,mm3
		PUNPCKLWD	mm4,mm4
		PUNPCKLWD	mm5,mm5
		PUNPCKLDQ	mm6,mm6 ; = [QSMask16]
		PUNPCKLDQ	mm3,mm3
		PUNPCKLDQ	mm4,mm4
		MOVD		mm7,EDI
		PUNPCKLDQ	mm5,mm5
		PUNPCKLDQ	mm7,mm7
		MOVQ		[QSMask16],mm6

		MOV			EAX,[EBP+MXPSBN16]
		MOV			EBX,[EBP+MYPSBN16]
		MOV			ECX,EAX
		MOV			EDX,EBX

		MOV         ESI,[EBP+MPSBType16]
		TEST        ESI,1
		JZ          SHORT .NormHzPut
		SUB         EAX,[SMinX]
		SUB         ECX,[SMaxX]
		JMP         SHORT .InvHzPut
.NormHzPut:
		ADD			EAX,[SMaxX] ; EAX = PutMaxX
		ADD			ECX,[SMinX] ; ECX = PutMinX
.InvHzPut:
		TEST        ESI,2
		JZ          SHORT .NormVtPut
		SUB         EBX,[SMinY]
		SUB         EDX,[SMaxY]
		JMP         SHORT .InvVtPut
.NormVtPut:
		ADD			EBX,[SMaxY] ; EBX = PutMaxY
		ADD			EDX,[SMinY] ; EDX = PutMinY
.InvVtPut:

		CMP			EAX,[_MinX]
		JL			.PasPutSurf
		CMP			EBX,[_MinY]
		JL			.PasPutSurf
		CMP			ECX,[_MaxX]
		JG			.PasPutSurf
		CMP			EDX,[_MaxY]
		JG			.PasPutSurf

		MOV			[PType],ESI ; save put Type

		; compute the clipped/unclipped put rectangle coordinaates in (PutSurfMinX, PutSurfMinY, PutSurfMaxX, PutSurfMaxY)
		;==========================================
		ClipStorePutSurfCoords

		;=========================
		; --- compute Put coordinates of the entire SrcSurf (as if source surf is full view)
		MOV         EAX,[EBP+MXPSBN16]  ; EAX = PutMaxX / without clipping
		MOV         EBX,[EBP+MYPSBN16]  ; EBX = PutMaxY / without clipping
		MOV			ESI,[PType] ; restore PType
		ComputeFullViewSrcSurfPutCoords

		CMP			EAX,[PutSurfMaxX]
		JG			.PutSurfClip
		CMP			EBX,[PutSurfMaxY]
		JG			.PutSurfClip
		CMP			ECX,[PutSurfMinX]
		JL			.PutSurfClip
		CMP			EDX,[PutSurfMinY]
		JL			.PutSurfClip

; PutSurf non Clipper *****************************
		MOV			EBP,[SResV]
		TEST		ESI,2 ; vertically reversed ?
		JZ			.NormAdSPut
		MOV			ESI,[Srlfb]
		MOV			EAX,[SScanLine]
		ADD			ESI,[SSizeSurf] ; ESI start of the last line in the surf
		SUB			ESI,EAX
		ADD			EAX,EAX
		NEG			EAX
		JMP			SHORT .InvAdSPut
.NormAdSPut:
		XOR			EAX,EAX
		MOV			ESI,[Srlfb] ; ESI : start copy adress
.InvAdSPut:
		MOV			EDI,EBX ; PutMaxY or the top left corner
		IMUL		EDI,[_NegScanLine]
		MOV			EDX,[_ScanLine]
		LEA			EDI,[EDI+ECX*2] ; += PutMinX*2 top left croner
		SUB			EDX,[SScanLine] ; EDX : dest adress plus
		ADD			EDI,[_vlfb]
		MOV			[Plus2],EDX

		TEST		BYTE [PType],1
		MOV			EDX,[SResH]
		JNZ         .InvHzPSurf
		MOV			[Plus],EAX

.BcPutSurf:
		MOV			EBX,EDX
.BcStBAv:
		TEST		EDI,6  		; qword aligned ?
		JZ			.FPasStBAv
		MOV			AX,[ESI]
		CMP			AX,[SMask]
		JE			.MaskBAv
		MOVD		mm0,EAX
		MOVD		mm1,EAX
		MOVD		mm2,EAX
		@SolidBlndQ
		MOVD		EAX,mm0
		MOV			[EDI],AX
.MaskBAv:
		DEC			EBX
		LEA			ESI,[ESI+2]
		LEA			EDI,[EDI+2]
		JZ			.FinSHLine
		JMP			SHORT .BcStBAv
.FPasStBAv:
		;MOV			ECX,EBX
		;SHR			ECX,2
		;JZ			.StBAp
;ALIGN 4
.StoMMX:
		CMP			EBX,BYTE 3
		JLE			.StBAp

		MOVQ		mm0,[ESI]
		MOVQ		mm1,[ESI]
		MOVQ		mm2,[ESI]
		@SolidBlndQ

        MOVQ      	mm2,[ESI]
        MOVQ        mm6,[EDI]
        MOVQ        mm1,[ESI]

        PCMPEQW     mm2,[QSMask16]
        PCMPEQW     mm1,[QSMask16]
        PANDN       mm2,mm0
        PAND        mm6,mm1
        POR         mm2,mm6

		SUB			EBX, BYTE 4
		MOVQ		[EDI],mm2
		LEA			ESI,[ESI+8]
		LEA			EDI,[EDI+8]
		JMP			.StoMMX
.StBAp:
		AND			EBX,BYTE 3
		JZ			.FinSHLine
.BcStBAp:
		MOV			AX,[ESI]
		CMP			AX,[SMask]
		JE			.MaskBAp
		MOVD		mm0,EAX
		MOVD		mm1,EAX
		MOVD		mm2,EAX
		@SolidBlndQ
		MOVD		EAX,mm0
		MOV			[EDI],AX
.MaskBAp:
		DEC			EBX
		LEA			ESI,[ESI+2]
		LEA			EDI,[EDI+2]
		JNZ			.BcStBAp
.PasStBAp:
.FinSHLine:
		ADD			EDI,[Plus2]
		ADD			ESI,[Plus]
		DEC			EBP
		JNZ			.BcPutSurf

		JMP			.PasPutSurf

; Put surf unclipped reversed horizontally *************
.InvHzPSurf:
		LEA			EAX,[EAX+EDX*4] ; +=SScanLine*2
		LEA			ESI,[ESI+EDX*2] ; +=SScanLine
		MOV			[Plus],EAX

.IBcPutSurf:
		MOV			EBX,EDX
.IBcStBAv:
		TEST		EDI,6
		JZ			.IFPasStBAv
		SUB			ESI, BYTE 2
		MOV			AX,[ESI]
		CMP			AX,[SMask]
		JE			.IMaskStBAv
		MOVD		mm0,EAX
		MOVD		mm1,EAX
		MOVD		mm2,EAX
		@SolidBlndQ
		MOVD		EAX,mm0
		MOV			[EDI],AX
.IMaskStBAv:
		DEC			EBX
		LEA			EDI,[EDI+2]
		JZ			.IFinSHLine
		JMP			SHORT .IBcStBAv
.IFPasStBAv:
		;MOV			ECX,EBX
		;SHR			ECX,2
		;JZ			.IStBAp
;ALIGN 4
.IStoMMX:
		CMP			EBX,BYTE 3
		JLE			.IStBAp

		SUB			ESI,BYTE 8
		MOV			EAX,[ESI]
		MOV			ECX,[ESI+4]
		ROR			EAX,16
		ROR			ECX,16
		MOVD		mm1,EAX
		MOVD		mm0,ECX
		MOV			EAX,QHLineOrg
		PUNPCKLDQ	mm0,mm1
		MOVQ		[EAX],mm0 ; [QHLineOrg]
		MOVQ		mm1,mm0
		MOVQ		mm2,mm0
;		MOVD		ECX,mm6
		@SolidBlndQ

        MOVQ      	mm2,[EAX] ; [QHLineOrg]
        MOVQ        mm6,[EDI]
        MOVQ        mm1,[EAX] ; [QHLineOrg]

        PCMPEQW     mm2,[QSMask16]
        PCMPEQW     mm1,[QSMask16]
        PANDN       mm2,mm0
        PAND        mm6,mm1
        POR         mm2,mm6

		MOVQ		[EDI],mm2
		SUB			EBX, BYTE 4
		LEA			EDI,[EDI+8]
		JMP			.IStoMMX
.IStBAp:
		AND			EBX,BYTE 3
		JZ			.IFinSHLine
.IBcStBAp:
		SUB			ESI, BYTE 2
		MOV			AX,[ESI]
		CMP			AX,[SMask]
		JE			SHORT .IMaskStBAp
		MOVD		mm0,EAX
		MOVD		mm1,EAX
		MOVD		mm2,EAX
		@SolidBlndQ
		MOVD		EAX,mm0
		MOV			[EDI],AX
.IMaskStBAp:
		DEC			EBX
		LEA			EDI,[EDI+2]
		JNZ			.IBcStBAp
.IPasStBAp:
.IFinSHLine:
		ADD			EDI,[Plus2]
		ADD			ESI,[Plus]
		DEC			EBP
		JNZ			.IBcPutSurf

.PasPutSurf:
		POP			ESI
		POP			EDI
		POP			EBX
	MMX_RETURN

.PutSurfClip:
; PutSurf Clipper **********************************************
		XOR			EDI,EDI   ; Y Fin Source
		XOR			ESI,ESI   ; X deb Source

		MOV			EBP,[PutSurfMinX]
		CMP			ECX,EBP ; CMP minx, _MinX
		JGE			.PsInfMinX   ; XP1<_MinX
		TEST		BYTE [PType],1
		JNZ			.InvHzCalcDX
		MOV         ESI,EBP
		SUB         ESI,ECX ; ESI = MinX - XP2
		MOV         ECX,EBP
.InvHzCalcDX:
		MOV			ECX,EBP
.PsInfMinX:
		MOV			EBP,[PutSurfMaxY]
		CMP			EBX,EBP ; cmp maxy, _MaxY
		JLE			.PsSupMaxY   ; YP2>_MaxY
		MOV			EDI,EBP
		NEG			EDI
		;MOV		[YP2],EBP
		ADD			EDI,EBX
		MOV			EBX,EBP
.PsSupMaxY:
		MOV			EBP,[PutSurfMinY]
		CMP			EDX,EBP      ; YP1<_MinY
		JGE			.PsInfMinY
		MOV			EDX,EBP
.PsInfMinY:
		MOV			EBP,[PutSurfMaxX]
		CMP			EAX,EBP      ; XP2>_MaxX
		JLE			.PsSupMaxX
		TEST		BYTE [PType],1
        JZ			.PsInvHzCalcDX
		MOV			ESI,EAX
		SUB			ESI,EBP	; ESI = XP2 - _MaxX
.PsInvHzCalcDX:
		MOV			EAX,EBP
.PsSupMaxX:
		SUB			EAX,ECX      ; XP2 - XP1
		MOV			EBP,[SScanLine]
		LEA			EAX,[EAX*2+2]
		SUB			EBP,EAX  ; EBP = SResH-DeltaX, PlusSSurf
		MOV			[Plus],EBP
		MOV			EBP,EBX
		SUB			EBP,EDX      ; YP2 - YP1
		INC			EBP   ; EBP = DeltaY
		MOV			EDX,[_ScanLine]
		MOVD		mm0,EAX ; = DeltaX
		SUB			EDX,EAX ; EDX = _ResH-DeltaX, PlusDSurfS
		TEST		BYTE [PType],2
		MOV			[Plus2],EDX
		JZ			.CNormAdSPut
		MOV			EAX,[Srlfb] ; Si inverse vertical
		ADD			EAX,[SSizeSurf] ; go to the last buffer
		SUB			EAX,[SScanLine] ; jump to the first of the last line
		LEA			EAX,[EAX+ESI*2] ; +X1InSSurf*2 clipping
		IMUL		EDI,[SScanLine] ; Y1InSSurf*ScanLine
		SUB			EAX,EDI
		MOV			ESI,EAX

		MOV			EAX,[SScanLine]
		ADD			EAX,EAX
		NEG			EAX
		JMP			SHORT .CInvAdSPut
.CNormAdSPut:
		IMUL		EDI,[SScanLine]
		XOR			EAX,EAX
		LEA			EDI,[EDI+ESI*2]
		ADD			EDI,[Srlfb]
		MOV			ESI,EDI
.CInvAdSPut:
		MOV			EDI,EBX
		IMUL		EDI,[_NegScanLine]
		LEA			EDI,[EDI+ECX*2] ; + XP1*2 as 16bpp
		PSRLD		mm0,1 ; (deltaX*2) / 2
		ADD			EDI,[_vlfb]
		MOVD		EDX,mm0  ; DeltaX
		TEST		BYTE [PType],1
		JNZ         .CInvHzPSurf
		ADD			[Plus],EAX

		JMP			.BcPutSurf

.CInvHzPSurf:   ; clipper et inverser horizontalement

		ADD			EAX,[SScanLine]
		LEA			EAX,[EAX+EDX*2] ; add to jump to the end
		LEA			ESI,[ESI+EDX*2] ; jump to the end
		MOV			[Plus],EAX
		JMP			.IBcPutSurf


ALIGN 16
_SurfMaskCopyBlnd16:
	ARG	PDstSrfMB, 4, PSrcSrfMB, 4, SCMBCol, 4
		PUSH		EDI
		PUSH	    ESI
		PUSH		EBX

; prepare col blending
		MOV			EAX,[EBP+SCMBCol] ;
		MOV			EBX,EAX ;
		MOV			ECX,EAX ;
		MOV			EDX,EAX ;
		AND			EBX,[QBlue16Mask] ; EBX = Bclr16 | Bclr16
		SHR			EAX,24
		AND			ECX,[QGreen16Mask] ; ECX = Gclr16 | Gclr16
		AND			AL,BlendMask ; remove any ineeded bits
		AND			EDX,[QRed16Mask] ; EDX = Rclr16 | Rclr16
		XOR			AL,BlendMask ; 31-blendsrc
		MOV			DI,AX
		SHL			EDI,16
		OR			DI,AX
		XOR			AL,BlendMask ; 31-blendsrc
		INC			AL
		SHR			DX,5 ; right shift red 5bits
		IMUL		BX,AX
		IMUL		CX,AX
		IMUL		DX,AX
		MOVD		mm6,[SMask]
		MOVD		mm3,EBX
		MOVD		mm4,ECX
		MOVD		mm5,EDX
		PUNPCKLWD	mm6,mm6
		PUNPCKLWD	mm3,mm3
		PUNPCKLWD	mm4,mm4
		PUNPCKLWD	mm5,mm5
		PUNPCKLDQ	mm6,mm6 ; = [QSMask16]
		PUNPCKLDQ	mm3,mm3
		PUNPCKLDQ	mm4,mm4
		MOVD		mm7,EDI
		PUNPCKLDQ	mm5,mm5
		PUNPCKLDQ	mm7,mm7
		MOVQ		[QSMask16],mm6

		MOV			ESI,[EBP+PSrcSrfMB]
		MOV			EDI,[EBP+PDstSrfMB]
		MOV			EBX,[ESI+_SizeSurf-_CurSurf]
		MOV			EBP,[ESI+_Mask-_CurSurf] ; EBP = src Surf Mask

		MOV			EDI,[EDI+_rlfb-_CurSurf]
		SHR			EBX,1
		MOV			ESI,[ESI+_rlfb-_CurSurf]

.BcStBAv:
		TEST		EDI,6  		; dword aligned ?
		JZ			.FPasStBAv
		MOV			AX,[ESI]
		CMP			AX,BP ; source Surf Mask
		JE			.MaskBAv
		MOVD		mm0,EAX
		MOVD		mm1,EAX
		MOVD		mm2,EAX
		@SolidBlndQ
		MOVD		EAX,mm0
		MOV			[EDI],AX
.MaskBAv:
		DEC			EBX
		LEA			ESI,[ESI+2]
		LEA			EDI,[EDI+2]
		JZ			.FinSurfCopy
		JMP			SHORT .BcStBAv
.FPasStBAv:
		MOV			ECX,EBX
		SHR			ECX,2
		JZ			.StBAp
;ALIGN 4
.StoMMX:
		MOVQ		mm0,[ESI]
		MOVQ		mm1,[ESI]
		MOVQ		mm2,[ESI]
		@SolidBlndQ

        MOVQ      	mm2,[ESI]
        MOVQ        mm6,[EDI]
        MOVQ        mm1,[ESI]

        PCMPEQW     mm2,[QSMask16]
        PCMPEQW     mm1,[QSMask16]
        PANDN       mm2,mm0
        PAND        mm6,mm1
        POR         mm2,mm6

		DEC			ECX
		MOVQ		[EDI],mm2
		LEA			ESI,[ESI+8]
		LEA			EDI,[EDI+8]
		JNZ			.StoMMX
.StBAp:
		AND			EBX,BYTE 3
		JZ			.FinSurfCopy
.BcStBAp:
		MOV			AX,[ESI]
		CMP			AX,BP
		JE			.MaskBAp
		MOVD		mm0,EAX
		MOVD		mm1,EAX
		MOVD		mm2,EAX
		@SolidBlndQ
		MOVD		EAX,mm0
		MOV			[EDI],AX
.MaskBAp:
		DEC		EBX
		LEA		ESI,[ESI+2]
		LEA		EDI,[EDI+2]
		JNZ		.BcStBAp
.PasStBAp:
.FinSurfCopy:
		POP		EBX
		POP		ESI
		POP		EDI
	MMX_RETURN

; mix 16bpp colors: source in (mm0, mm1, mm2) / dest in (mm3, mm4, mm5)
; source miltipilier mm7 (4 words from 0 to 31)
; dest   miltipilier mm6 (4 words from 0 to 31)
%macro	@TransBlndQ 0
		PAND		mm0,[QBlue16Mask]
		PAND		mm3,[QBlue16Mask]
		PAND		mm1,[QGreen16Mask]
		PAND		mm4,[QGreen16Mask]
		PAND		mm2,[QRed16Mask]
		PMULLW		mm0,mm7 ; [blend_src]
		PAND		mm5,[QRed16Mask]
		PMULLW		mm3,mm6 ; [blend_dst]
		PSRLW		mm2,5
		PMULLW		mm4,mm6 ; [blend_dst]
		PSRLW		mm5,5
		PMULLW		mm1,mm7 ; [blend_src]
		PMULLW		mm2,mm7 ; [blend_src]
		PADDW		mm0,mm3
		PMULLW		mm5,mm6 ; [blend_dst]

		PADDW		mm1,mm4
		PADDW		mm2,mm5
		PSRLW		mm0,5
		PSRLW		mm1,5
		PAND		mm2,[QRed16Mask]
		;PAND		mm0,[QBlue16Mask]
		PAND		mm1,[QGreen16Mask]
		POR			mm0,mm2
		POR			mm0,mm1
%endmacro

; -------------------------------
; Put a Transparent Surf
; -------------------------------

ALIGN 16
_PutSurfTrans16:
	ARG	SSTN16, 4, XPSTN16, 4, YPSTN16, 4, PSTType16, 4, PSTrans16, 4
		PUSH		EBX
		PUSH		EDI
		PUSH		ESI

		MOV			ESI,[EBP+SSTN16]
		MOV			EDI,_SrcSurf

		CopySurf	; copy surf

; prepare col blending
		MOV			EAX,[EBP+PSTrans16] ;
		AND			EAX,BYTE BlendMask
		JZ			.PasPutSurf
		MOV			EDX,EAX ;
		INC			EAX

		XOR			DL,BlendMask ; 31-blendsrc
		MOVD		mm7,EAX
		MOVD		mm6,EDX
		PUNPCKLWD	mm7,mm7
		PUNPCKLWD	mm6,mm6
		PUNPCKLDQ	mm7,mm7
		PUNPCKLDQ	mm6,mm6

		MOV			EAX,[EBP+XPSTN16]
		MOV			EBX,[EBP+YPSTN16]
		MOV			ECX,EAX
		MOV			EDX,EBX

		MOV         ESI,[EBP+PSTType16]
		TEST        ESI,1
		JZ          .NormHzPut
		SUB         EAX,[SMinX]
		SUB         ECX,[SMaxX]
		JMP         SHORT .InvHzPut
.NormHzPut:
		ADD			EAX,[SMaxX] ; EAX = PutMaxX
		ADD			ECX,[SMinX] ; ECX = PutMinX
.InvHzPut:
		TEST        ESI,2
		JZ          .NormVtPut
		SUB         EBX,[SMinY]
		SUB         EDX,[SMaxY]
		JMP         SHORT .InvVtPut
.NormVtPut:
		ADD			EBX,[SMaxY] ; EBX = PutMaxY
		ADD			EDX,[SMinY] ; EDX = PutMinY
.InvVtPut:

		CMP			EAX,[_MinX]
		JL			.PasPutSurf
		CMP			EBX,[_MinY]
		JL			.PasPutSurf
		CMP			ECX,[_MaxX]
		JG			.PasPutSurf
		CMP			EDX,[_MaxY]
		JG			.PasPutSurf

		MOV			[PType],ESI ; save put Type

		; compute the clipped/unclipped put rectangle coordinaates in (PutSurfMinX, PutSurfMinY, PutSurfMaxX, PutSurfMaxY)
		;==========================================
		ClipStorePutSurfCoords

		;=========================
		; --- compute Put coordinates of the entire SrcSurf (as if source surf is full view)
		MOV         EAX,[EBP+XPSTN16]  ; EAX = PutMaxX / without clipping
		MOV         EBX,[EBP+YPSTN16]  ; EBX = PutMaxY / without clipping
		MOV			ESI,[PType] ; restore PType
		ComputeFullViewSrcSurfPutCoords

		CMP			EAX,[PutSurfMaxX]
		JG			.PutSurfClip
		CMP			EBX,[PutSurfMaxY]
		JG			.PutSurfClip
		CMP			ECX,[PutSurfMinX]
		JL			.PutSurfClip
		CMP			EDX,[PutSurfMinY]
		JL			.PutSurfClip

; PutSurf non Clipper *****************************
		MOV			EBP,[SResV]
		TEST		ESI,2 ; vertically reversed ?
		JZ			.NormAdSPut
		MOV			ESI,[Srlfb]
		MOV			EAX,[SScanLine]
		ADD			ESI,[SSizeSurf] ; ESI start of the last line in the surf
		SUB			ESI,EAX
		ADD			EAX,EAX
		NEG			EAX
		JMP			SHORT .InvAdSPut
.NormAdSPut:
		XOR			EAX,EAX
		MOV			ESI,[Srlfb] ; ESI : start copy adress
.InvAdSPut:
		MOV			EDI,EBX ; PutMaxY or the top left corner
		IMUL		EDI,[_NegScanLine]
		LEA			EDI,[EDI+ECX*2] ; += PutMinX*2 top left croner
		MOV			EDX,[_ScanLine]
		ADD			EDI,[_vlfb]
		SUB			EDX,[SScanLine] ; EDX : dest adress plus
		MOV			[Plus2],EDX

		TEST		BYTE [PType],1
		MOV			EDX,[SResH]
		JNZ        .InvHzPSurf
		MOV			[Plus],EAX

.BcPutSurf:
		MOV			EBX,EDX
.BcStBAv:
		TEST		EDI,6  		; dword aligned ?
		JZ			.FPasStBAv
		MOV			AX,[ESI]
		MOV			CX,[EDI]
		DEC			EBX
		MOVD		mm0,EAX
		MOVD		mm3,ECX
		MOVQ		mm1,mm0
		MOVQ		mm4,mm3
		MOVQ		mm2,mm0
		MOVQ		mm5,mm3
		@TransBlndQ
		MOVD		EAX,mm0
		LEA			ESI,[ESI+2]
		STOSW
		JZ			.FinSHLine
		JMP			.BcStBAv
.FPasStBAv:
		;MOV			ECX,EBX
		;SHR			ECX,2
		;JZ			.StBAp
;ALIGN 4
.StoMMX:
		CMP			EBX,BYTE 3
		JLE			.StBAp

		MOVQ		mm0,[ESI]
		MOVQ		mm3,[EDI]
		MOVQ		mm1,mm0
		MOVQ		mm4,mm3
		MOVQ		mm2,mm0
		MOVQ		mm5,mm3
		@TransBlndQ
		SUB			EBX,BYTE 4
		MOVQ		[EDI],mm0
		LEA			ESI,[ESI+8]
		LEA			EDI,[EDI+8]
		JNZ			.StoMMX
.StBAp:
		AND			EBX,BYTE 3
		JZ			.FinSHLine
.BcStBAp:
		MOV			AX,[ESI]
		MOV			CX,[EDI]
		DEC			EBX
		MOVD		mm0,EAX
		MOVD		mm3,ECX
		MOVQ		mm1,mm0
		MOVQ		mm4,mm3
		MOVQ		mm2,mm0
		MOVQ		mm5,mm3
		@TransBlndQ
		MOVD		EAX,mm0
		LEA			ESI,[ESI+2]
		STOSW
		JNZ			.BcStBAp
.PasStBAp:
.FinSHLine:
		ADD			EDI,[Plus2]
		ADD			ESI,[Plus]
		DEC			EBP
		JNZ			.BcPutSurf

		JMP			.PasPutSurf

; Put surf unclipped reversed horizontally *************
.InvHzPSurf:
		LEA			EAX,[EAX+EDX*4] ; +=SScanLine*2
		LEA			ESI,[ESI+EDX*2] ; +=SScanLine
		MOV			[Plus],EAX

.IBcPutSurf:
		MOV			EBX,EDX
.IBcStBAv:
		TEST		EDI,6
		JZ			.IFPasStBAv
		SUB			ESI, BYTE 2
		MOV			AX,[ESI]
		MOV			CX,[EDI]
		MOVD		mm0,EAX
		MOVD		mm3,ECX
		MOVQ		mm1,mm0
		MOVQ		mm4,mm3
		MOVQ		mm2,mm0
		MOVQ		mm5,mm3
		@TransBlndQ
		DEC			EBX
		MOVD		EAX,mm0
		STOSW
		JZ			.IFinSHLine
		JMP			.IBcStBAv
.IFPasStBAv:
		;MOV			ECX,EBX
		;SHR			ECX,2
		;JZ			.IStBAp
;ALIGN 4
.IStoMMX:
		CMP			EBX,BYTE 3
		JLE			.IStBAp

		SUB			ESI,BYTE 8
		MOVQ		mm3,[EDI]
		MOV			EAX,[ESI]
		MOV			ECX,[ESI+4]
		ROR			EAX,16
		ROR			ECX,16
		MOVD		mm1,EAX
		MOVD		mm0,ECX
		SUB			EBX, BYTE 4
		PUNPCKLDQ	mm0,mm1
		MOVQ		mm4,mm3
		MOVQ		mm1,mm0
		MOVQ		mm5,mm3
		MOVQ		mm2,mm0
		@TransBlndQ
		MOVQ		[EDI],mm0
		LEA			EDI,[EDI+8]
		JMP			.IStoMMX
.IStBAp:
		AND			EBX,BYTE 3
		JZ			.IFinSHLine
.IBcStBAp:
		SUB			ESI, BYTE 2
		MOV			AX,[ESI]
		MOV			CX,[EDI]
		MOVD		mm0,EAX
		MOVD		mm3,ECX
		MOVQ		mm1,mm0
		MOVQ		mm4,mm3
		MOVQ		mm2,mm0
		MOVQ		mm5,mm3
		@TransBlndQ
		DEC			EBX
		MOVD		EAX,mm0
		STOSW
		JNZ			.IBcStBAp
.IPasStBAp:
.IFinSHLine:
		ADD			EDI,[Plus2]
		ADD			ESI,[Plus]
		DEC			EBP
		JNZ			.IBcPutSurf

.PasPutSurf:

		POP		ESI
		POP		EDI
		POP		EBX
	MMX_RETURN

; PutSurf Clipper **********************************************
.PutSurfClip:
		XOR			EDI,EDI   ; Y Fin Source
		XOR			ESI,ESI   ; X deb Source

		MOV			EBP,[PutSurfMinX]
		CMP			ECX,EBP ; CMP minx, _MinX
		JGE			.PsInfMinX   ; XP1<_MinX
		TEST		BYTE [PType],1
		JNZ			.InvHzCalcDX
		MOV         ESI,EBP
		SUB         ESI,ECX ; ESI = MinX - XP2
		MOV         ECX,EBP
.InvHzCalcDX:
		MOV			ECX,EBP
.PsInfMinX:
		MOV			EBP,[PutSurfMaxY]
		CMP			EBX,EBP ; cmp maxy, _MaxY
		JLE			.PsSupMaxY   ; YP2>_MaxY
		MOV			EDI,EBP
		NEG			EDI
		;MOV		[YP2],EBP
		ADD			EDI,EBX
		MOV			EBX,EBP
.PsSupMaxY:
		MOV			EBP,[PutSurfMinY]
		CMP			EDX,EBP      ; YP1<_MinY
		JGE			.PsInfMinY
		MOV			EDX,EBP
.PsInfMinY:
		MOV			EBP,[PutSurfMaxX]
		CMP			EAX,EBP      ; XP2>_MaxX
		JLE			.PsSupMaxX
		TEST		BYTE [PType],1
		JZ			.PsInvHzCalcDX
		MOV			ESI,EAX
		SUB			ESI,EBP	; ESI = XP2 - _MaxX
.PsInvHzCalcDX:
		MOV			EAX,EBP
.PsSupMaxX:
		SUB			EAX,ECX      ; XP2 - XP1
		MOV			EBP,[SScanLine]
		LEA			EAX,[EAX*2+2]
		SUB			EBP,EAX  ; EBP = SResH-DeltaX, PlusSSurf
		MOV			[Plus],EBP
		MOV			EBP,EBX
		SUB			EBP,EDX      ; YP2 - YP1
		INC			EBP   ; EBP = DeltaY
		MOV			EDX,[_ScanLine]
		MOVD		mm0,EAX ; = DeltaX
		SUB			EDX,EAX ; EDX = _ResH-DeltaX, PlusDSurfS
		TEST		BYTE [PType],2
		MOV			[Plus2],EDX
		JZ			.CNormAdSPut
		MOV			EAX,[Srlfb] ; Si inverse vertical
		ADD			EAX,[SSizeSurf] ; go to the last buffer
		SUB			EAX,[SScanLine] ; jump to the first of the last line
		LEA			EAX,[EAX+ESI*2] ; +X1InSSurf*2 clipping
		IMUL		EDI,[SScanLine] ; Y1InSSurf*ScanLine
		SUB			EAX,EDI
		MOV			ESI,EAX

		MOV			EAX,[SScanLine]
		ADD			EAX,EAX
		NEG			EAX
		JMP			SHORT .CInvAdSPut
.CNormAdSPut:
		IMUL		EDI,[SScanLine]
		XOR			EAX,EAX
		LEA			EDI,[EDI+ESI*2]
		ADD			EDI,[Srlfb]
		MOV			ESI,EDI
.CInvAdSPut:
		MOV			EDI,EBX
		IMUL		EDI,[_NegScanLine]
		LEA			EDI,[EDI+ECX*2] ; + XP1*2 as 16bpp
		PSRLD		mm0,1 ; (deltaX*2) / 2
		ADD			EDI,[_vlfb]

		MOVD		EDX,mm0  ; DeltaX
		TEST		BYTE [PType],1
		JNZ         .CInvHzPSurf
		ADD			[Plus],EAX
		JMP			.BcPutSurf

.CInvHzPSurf:   ; clipper et inverser horizontalement

		ADD			EAX,[SScanLine]
		LEA			EAX,[EAX+EDX*2] ; add to jump to the end
		LEA			ESI,[ESI+EDX*2] ; jump to the end
		MOV			[Plus],EAX
		JMP			.IBcPutSurf



ALIGN 16
_SurfCopyTrans16:
	ARG	PDstSrfT, 4, PSrcSrfT, 4, SCTrans, 4
		PUSH		EDI
		PUSH		ESI
		PUSH		EBX

; prepare col blending
		MOV			EAX,[EBP+SCTrans] ;
		AND			EAX,BYTE BlendMask
		JZ			.FinSurfCopy
		MOV			EDX,EAX ;
		INC			EAX

		XOR			DL,BlendMask ; 31-blendsrc
		MOVD		mm7,EAX
		MOVD		mm6,EDX
		PUNPCKLWD	mm7,mm7
		PUNPCKLWD	mm6,mm6
		PUNPCKLDQ	mm7,mm7
		PUNPCKLDQ	mm6,mm6

		MOV			ESI,[EBP+PSrcSrfT]
		MOV			EDI,[EBP+PDstSrfT]
		MOV			EBX,[ESI+_SizeSurf-_CurSurf]

		MOV			EDI,[EDI+_rlfb-_CurSurf]
		SHR			EBX,1
		MOV			ESI,[ESI+_rlfb-_CurSurf]

.BcStBAv:
		TEST		EDI,6  		; qword aligned ?
		JZ			.FPasStBAv
		MOV			AX,[ESI]
		MOV			DX,[EDI]
		DEC			EBX
		MOVD		mm0,EAX
		MOVD		mm3,EDX
		MOVQ		mm1,mm0
		MOVQ		mm4,mm3
		MOVQ		mm2,mm0
		MOVQ		mm5,mm3
		@TransBlndQ
		MOVD		EAX,mm0
		LEA			ESI,[ESI+2]
		STOSW
		JZ			.FinSurfCopy
		JMP			.BcStBAv
.FPasStBAv:
		MOV			ECX,EBX
		SHR			ECX,2
		JZ			.StBAp
;ALIGN 4
.StoMMX:
		MOVQ		mm0,[ESI]
		MOVQ		mm3,[EDI]
		MOVQ		mm1,mm0
		MOVQ		mm4,mm3
		MOVQ		mm2,mm0
		MOVQ		mm5,mm3
		@TransBlndQ
		DEC			ECX
		MOVQ		[EDI],mm0
		LEA			ESI,[ESI+8]
		LEA			EDI,[EDI+8]
		JNZ			.StoMMX
.StBAp:
		AND			EBX,BYTE 3
		JZ			.FinSurfCopy
.BcStBAp:
		MOV			AX,[ESI]
		MOV			DX,[EDI]
		DEC			EBX
		MOVD		mm0,EAX
		MOVD		mm3,EDX
		MOVQ		mm1,mm0
		MOVQ		mm4,mm3
		MOVQ		mm2,mm0
		MOVQ		mm5,mm3
		@TransBlndQ
		MOVD		EAX,mm0
		LEA			ESI,[ESI+2]
		STOSW
		JNZ			.BcStBAp
.PasStBAp:
.FinSurfCopy:

		POP			EBX
		POP			ESI
		POP			EDI
	MMX_RETURN

; -------------------------------
; Put a Masked Transparent Surf
; -------------------------------

ALIGN 16
_PutMaskSurfTrans16:
	ARG	SMSTN16, 4, XPMSTN16, 4, YPMSTN16, 4, PMSTType16, 4, PMSTrans16, 4
		PUSH		EBX
		PUSH		EDI
		PUSH		ESI

		MOV			ESI,[EBP+SMSTN16]
		MOV			EDI,_SrcSurf

		CopySurf	; copy surf

; prepare col blending
		MOV			EAX,[EBP+PMSTrans16] ;
		AND			EAX,BYTE BlendMask
		JZ			.PasPutSurf
		MOV			EDX,EAX ;
		INC			EAX

		XOR			DL,BlendMask ; 31-blendsrc
		MOVD		mm0,[SMask]
		MOVD		mm7,EAX
		MOVD		mm6,EDX
		PUNPCKLWD	mm0,mm0
		PUNPCKLWD	mm7,mm7
		PUNPCKLWD	mm6,mm6
		PUNPCKLDQ	mm0,mm0
		PUNPCKLDQ	mm7,mm7
		PUNPCKLDQ	mm6,mm6
		MOVQ		[QSMask16],mm0

		MOV			EAX,[EBP+XPMSTN16]
		MOV			EBX,[EBP+YPMSTN16]
		MOV			ECX,EAX
		MOV			EDX,EBX

		MOV         ESI,[EBP+PMSTType16]
		TEST        ESI,1
		JZ          .NormHzPut
		SUB         EAX,[SMinX]
		SUB         ECX,[SMaxX]
		JMP         SHORT .InvHzPut
.NormHzPut:
		ADD			EAX,[SMaxX] ; EAX = PutMaxX
		ADD			ECX,[SMinX] ; ECX = PutMinX
.InvHzPut:
		TEST        ESI,2
		JZ          .NormVtPut
		SUB         EBX,[SMinY]
		SUB         EDX,[SMaxY]
		JMP         .InvVtPut
.NormVtPut:
		ADD			EBX,[SMaxY] ; EBX = PutMaxY
		ADD			EDX,[SMinY] ; EDX = PutMinY
.InvVtPut:
		CMP			EAX,[_MinX]
		JL			.PasPutSurf
		CMP			EBX,[_MinY]
		JL			.PasPutSurf
		CMP			ECX,[_MaxX]
		JG			.PasPutSurf
		CMP			EDX,[_MaxY]
		JG			.PasPutSurf

		MOV			[PType],ESI ; save put Type

		; compute the clipped/unclipped put rectangle coordinaates in (PutSurfMinX, PutSurfMinY, PutSurfMaxX, PutSurfMaxY)
		;==========================================
		ClipStorePutSurfCoords

		;=========================
		; --- compute Put coordinates of the entire SrcSurf (as if source surf is full view)
		MOV         EAX,[EBP+XPSN16]  ; EAX = PutMaxX / without clipping
		MOV         EBX,[EBP+YPSN16]  ; EBX = PutMaxY / without clipping
		MOV			ESI,[PType] ; restore PType
		ComputeFullViewSrcSurfPutCoords

		CMP			EAX,[PutSurfMaxX]
		JG			.PutSurfClip
		CMP			EBX,[PutSurfMaxY]
		JG			.PutSurfClip
		CMP			ECX,[PutSurfMinX]
		JL			.PutSurfClip
		CMP			EDX,[PutSurfMinY]
		JL			.PutSurfClip

; PutSurf non Clipper *****************************
		MOV			EBP,[SResV]
		TEST		ESI,2 ; vertically reversed ?
		JZ			.NormAdSPut
		MOV			ESI,[Srlfb]
		MOV			EAX,[SScanLine]
		ADD			ESI,[SSizeSurf] ; ESI start of the last line in the surf
		SUB			ESI,EAX
		ADD			EAX,EAX
		NEG			EAX
		JMP			SHORT .InvAdSPut
.NormAdSPut:
		XOR			EAX,EAX
		MOV			ESI,[Srlfb] ; ESI : start copy adress
.InvAdSPut:
		MOV			EDI,EBX ; PutMaxY or the top left corner
		IMUL		EDI,[_ScanLine]
		NEG			EDI
		LEA			EDI,[EDI+ECX*2] ; += PutMinX*2 top left croner
		MOV			EDX,[_ScanLine]
		ADD			EDI,[_vlfb]
		SUB			EDX,[SScanLine] ; EDX : dest adress plus
		MOV			[Plus2],EDX

		TEST		BYTE [PType],1
		MOV			EDX,[SResH]
		JNZ         .InvHzPSurf
		MOV			[Plus],EAX

.BcPutSurf:
		MOV			EBX,EDX
.BcStBAv:
		TEST		EDI,6  		; dword aligned ?
		JZ			.FPasStBAv
		MOV			AX,[ESI]
		MOV			CX,[EDI]
		CMP			AX,[SMask]
		JE			.MaskStBAv
		MOVD		mm0,EAX
		MOVD		mm3,ECX
		MOVQ		mm1,mm0
		MOVQ		mm4,mm3
		MOVQ		mm2,mm0
		MOVQ		mm5,mm3
		@TransBlndQ
		MOVD		EAX,mm0
		MOV			[EDI],AX
.MaskStBAv:
		DEC			EBX
		LEA			ESI,[ESI+2]
		LEA			EDI,[EDI+2]
		JZ			.FinSHLine
		JMP			.BcStBAv
.FPasStBAv:
		;MOV			ECX,EBX
		;SHR			ECX,2
		;JZ			.StBAp
;ALIGN 4
.StoMMX:
		CMP			EBX,BYTE 3
		JLE			.StBAp

		MOVQ		mm0,[ESI]
		MOVQ		mm3,[EDI]
		MOVQ		mm1,mm0
		MOVQ		mm4,mm3
		MOVQ		mm2,mm0
		MOVQ		mm5,mm3
		MOVQ		[QHLineOrg],mm0 ; save original 4 pixels before transparency
		@TransBlndQ

        MOVQ      	mm3,[QHLineOrg]
        MOVQ        mm4,[EDI]
        MOVQ        mm1,mm3

        PCMPEQW     mm3,[QSMask16]
        PCMPEQW     mm1,[QSMask16]
        PANDN       mm3,mm0
        PAND        mm4,mm1
        POR         mm3,mm4

		SUB			EBX,BYTE 4
		MOVQ		[EDI],mm3 ; write the 8 bytes
		LEA			ESI,[ESI+8]
		LEA			EDI,[EDI+8]
		JMP			.StoMMX
.StBAp:
		AND			EBX,BYTE 3
		JZ			.FinSHLine
.BcStBAp:
		MOV			AX,[ESI]
		MOV			CX,[EDI]
		CMP			AX,[SMask]
		JE			.MaskStBAp
		MOVD		mm0,EAX
		MOVD		mm3,ECX
		MOVQ		mm1,mm0
		MOVQ		mm4,mm3
		MOVQ		mm2,mm0
		MOVQ		mm5,mm3
		@TransBlndQ
		MOVD		EAX,mm0
		MOV			[EDI],AX
.MaskStBAp:
		DEC			EBX
		LEA			ESI,[ESI+2]
		LEA			EDI,[EDI+2]
		JNZ			.BcStBAp
.PasStBAp:
.FinSHLine:
		ADD			EDI,[Plus2]
		ADD			ESI,[Plus]
		DEC			EBP
		JNZ			.BcPutSurf

		JMP			.PasPutSurf

; Put surf unclipped reversed horizontally *************
.InvHzPSurf:
		LEA			EAX,[EAX+EDX*4] ; +=SScanLine*2
		LEA			ESI,[ESI+EDX*2] ; +=SScanLine
		MOV			[Plus],EAX

.IBcPutSurf:
		MOV			EBX,EDX
.IBcStBAv:
		TEST		EDI,6
		JZ			.IFPasStBAv
		SUB			ESI, BYTE 2
		MOV			CX,[EDI]
		MOV			AX,[ESI]
		CMP			AX,[SMask]
		JE			.IMaskStBAv
		MOVD		mm0,EAX
		MOVD		mm3,ECX
		MOVQ		mm1,mm0
		MOVQ		mm4,mm3
		MOVQ		mm2,mm0
		MOVQ		mm5,mm3
		@TransBlndQ
		MOVD		EAX,mm0
		MOV			[EDI],AX
.IMaskStBAv:
		DEC			EBX
		LEA			EDI,[EDI+2]
		JZ			.IFinSHLine
		JMP			.IBcStBAv
.IFPasStBAv:
		;MOV			ECX,EBX
		;SHR			ECX,2
		;JZ			.IStBAp
;ALIGN 4
.IStoMMX:
		CMP			EBX,BYTE 3
		JLE			.IStBAp

		SUB			ESI,BYTE 8
		MOVQ		mm3,[EDI]
		MOV			EAX,[ESI]
		MOV			ECX,[ESI+4]
		ROR			EAX,16
		ROR			ECX,16
		MOVD		mm1,EAX
		MOVD		mm0,ECX
		PUNPCKLDQ	mm0,mm1
		MOVQ		mm4,mm3
		MOVQ		mm1,mm0
		MOVQ		mm5,mm3
		MOVQ		mm2,mm0
		MOVQ		[QHLineOrg],mm0 ; save original 4 pixels before transparency
		@TransBlndQ
        MOVQ      	mm3,[QHLineOrg]
        MOVQ        mm4,[EDI]
        MOVQ        mm1,mm3

        PCMPEQW     mm3,[QSMask16]
        PCMPEQW     mm1,[QSMask16]
        PANDN       mm3,mm0
        PAND        mm4,mm1
        POR         mm3,mm4

		MOVQ		[EDI],mm3

		SUB			EBX,BYTE 4
		LEA			EDI,[EDI+8]
		JMP			.IStoMMX
.IStBAp:
		AND			EBX,BYTE 3
		JZ			.IFinSHLine
.IBcStBAp:
		SUB			ESI, BYTE 2
		MOV			CX,[EDI]
		MOV			AX,[ESI]
		CMP			AX,[SMask]
		JE			.IMaskStBAp
		MOVD		mm0,EAX
		MOVD		mm3,ECX
		MOVQ		mm1,mm0
		MOVQ		mm4,mm3
		MOVQ		mm2,mm0
		MOVQ		mm5,mm3
		@TransBlndQ
		MOVD		EAX,mm0
		MOV			[EDI],AX
.IMaskStBAp:
		DEC			EBX
		LEA			EDI,[EDI+2]
		JNZ			.IBcStBAp
.IPasStBAp:
.IFinSHLine:
		ADD			EDI,[Plus2]
		ADD			ESI,[Plus]
		DEC			EBP
		JNZ			.IBcPutSurf

.PasPutSurf:
		POP			ESI
		POP			EDI
		POP			EBX
	MMX_RETURN

.PutSurfClip:
; PutSurf Clipper **********************************************
		XOR			EDI,EDI   ; Y Fin Source
		XOR			ESI,ESI   ; X deb Source

		MOV			EBP,[PutSurfMinX]
		CMP			ECX,EBP ; CMP minx, _MinX
		JGE			.PsInfMinX   ; XP1<_MinX
		TEST		BYTE [PType],1
		JNZ			.InvHzCalcDX
		MOV         ESI,EBP
		SUB         ESI,ECX ; ESI = MinX - XP2
		MOV         ECX,EBP
.InvHzCalcDX:
		MOV			ECX,EBP
.PsInfMinX:
		MOV			EBP,[PutSurfMaxY]
		CMP			EBX,EBP ; cmp maxy, _MaxY
		JLE			.PsSupMaxY   ; YP2>_MaxY
		MOV			EDI,EBP
		NEG			EDI
		;MOV		[YP2],EBP
		ADD			EDI,EBX
		MOV			EBX,EBP
.PsSupMaxY:
		MOV			EBP,[PutSurfMinY]
		CMP			EDX,EBP      ; YP1<_MinY
		JGE			.PsInfMinY
		MOV			EDX,EBP
.PsInfMinY:
		MOV			EBP,[PutSurfMaxX]
		CMP			EAX,EBP      ; XP2>_MaxX
		JLE			.PsSupMaxX
		TEST		BYTE [PType],1
        JZ			.PsInvHzCalcDX
		MOV			ESI,EAX
		SUB			ESI,EBP	; ESI = XP2 - _MaxX
.PsInvHzCalcDX:
		MOV			EAX,EBP
.PsSupMaxX:
		SUB			EAX,ECX      ; XP2 - XP1
		MOV			EBP,[SScanLine]
		LEA			EAX,[EAX*2+2]
		SUB			EBP,EAX  ; EBP = SResH-DeltaX, PlusSSurf
		MOV			[Plus],EBP
		MOV			EBP,EBX
		SUB			EBP,EDX      ; YP2 - YP1
		INC			EBP   ; EBP = DeltaY
		MOV			EDX,[_ScanLine]
		MOVD		mm0,EAX ; = DeltaX
		SUB			EDX,EAX ; EDX = _ResH-DeltaX, PlusDSurfS
		TEST		BYTE [PType],2
		MOV			[Plus2],EDX
		JZ			.CNormAdSPut
		MOV			EAX,[Srlfb] ; Si inverse vertical
		ADD			EAX,[SSizeSurf] ; go to the last buffer
		SUB			EAX,[SScanLine] ; jump to the first of the last line
		LEA			EAX,[EAX+ESI*2] ; +X1InSSurf*2 clipping
		IMUL		EDI,[SScanLine] ; Y1InSSurf*ScanLine
		SUB			EAX,EDI
		MOV			ESI,EAX

		MOV			EAX,[SScanLine]
		ADD			EAX,EAX
		NEG			EAX
		JMP			SHORT .CInvAdSPut
.CNormAdSPut:
		IMUL		EDI,[SScanLine]
		XOR			EAX,EAX
		LEA			EDI,[EDI+ESI*2]
		ADD			EDI,[Srlfb]
		MOV			ESI,EDI
.CInvAdSPut:
		MOV			EDI,EBX
		IMUL		EDI,[_NegScanLine]
		LEA			EDI,[EDI+ECX*2] ; + XP1*2 as 16bpp
		PSRLD		mm0,1 ; (deltaX*2) / 2
		ADD			EDI,[_vlfb]

		MOVD		EDX,mm0  ; DeltaX
		TEST		BYTE [PType],1
		JNZ         .CInvHzPSurf
		ADD			[Plus],EAX
		JMP			.BcPutSurf

.CInvHzPSurf:   ; clipper et inverser horizontalement

		ADD			EAX,[SScanLine]
		LEA			EAX,[EAX+EDX*2] ; add to jump to the end
		LEA			ESI,[ESI+EDX*2] ; jump to the end
		MOV			[Plus],EAX
		JMP			.IBcPutSurf


ALIGN 16
_SurfMaskCopyTrans16:
	ARG	PDstSrfMT, 4, PSrcSrfMT, 4, SCMTrans, 4
		PUSH		EDI
		PUSH		ESI
		PUSH		EBX

; prepare col blending
		MOV			EAX,[EBP+SCMTrans] ;
		AND			EAX,BYTE BlendMask
		JZ			.FinSurfCopy
		MOV			EDX,EAX ;
		INC			EAX

		XOR			DL,BlendMask ; 31-blendsrc
		MOVD		mm0,[SMask]
		MOVD		mm7,EAX
		MOVD		mm6,EDX
		PUNPCKLWD	mm0,mm0
		PUNPCKLWD	mm7,mm7
		PUNPCKLWD	mm6,mm6
		PUNPCKLDQ	mm0,mm0
		PUNPCKLDQ	mm7,mm7
		PUNPCKLDQ	mm6,mm6
		MOVQ		[QSMask16],mm0

		MOV			ESI,[EBP+PSrcSrfMT]
		MOV			EDI,[EBP+PDstSrfMT]
		MOV			EBX,[ESI+_SizeSurf-_CurSurf]
		MOV			EBP,[ESI+_Mask-_CurSurf]

		MOV			EDI,[EDI+_rlfb-_CurSurf]
		SHR			EBX,1
		MOV			ESI,[ESI+_rlfb-_CurSurf]

.BcStBAv:
		TEST		EDI,6  		; dword aligned ?
		JZ			.FPasStBAv
		MOV			AX,[ESI]
		MOV			DX,[EDI]
		CMP			AX,BP
		JE			.MaskStBAv
		MOVD		mm0,EAX
		MOVD		mm3,EDX
		MOVQ		mm1,mm0
		MOVQ		mm4,mm3
		MOVQ		mm2,mm0
		MOVQ		mm5,mm3
		@TransBlndQ
		MOVD		EAX,mm0
		MOV			[EDI],AX
.MaskStBAv:
		DEC			EBX
		LEA			ESI,[ESI+2]
		LEA			EDI,[EDI+2]
		JZ			.FinSurfCopy
		JMP			.BcStBAv
.FPasStBAv:
		MOV			ECX,EBX
		SHR			ECX,2
		JZ			.StBAp
;ALIGN 4
.StoMMX:
		MOVQ		mm0,[ESI]
		MOVQ		mm3,[EDI]
		MOVQ		mm1,mm0
		MOVQ		mm4,mm3
		MOVQ		mm2,mm0
		MOVQ		mm5,mm3
		MOVQ		[QHLineOrg],mm0 ; save original 4 pixels before transparency
		@TransBlndQ

        MOVQ      	mm3,[QHLineOrg]
        MOVQ        mm4,[EDI]
        MOVQ        mm1,mm3

        PCMPEQW     mm3,[QSMask16]
        PCMPEQW     mm1,[QSMask16]
        PANDN       mm3,mm0
        PAND        mm4,mm1
        POR         mm3,mm4

		DEC			ECX
		MOVQ		[EDI],mm3 ; write the 8 bytes
		LEA			ESI,[ESI+8]
		LEA			EDI,[EDI+8]
		JNZ			.StoMMX
.StBAp:
		AND			EBX,BYTE 3
		JZ			.FinSurfCopy
.BcStBAp:
		MOV			AX,[ESI]
		MOV			DX,[EDI]
		CMP			AX,BP
		JE			.MaskStBAp
		MOVD		mm0,EAX
		MOVD		mm3,EDX
		MOVQ		mm1,mm0
		MOVQ		mm4,mm3
		MOVQ		mm2,mm0
		MOVQ		mm5,mm3
		@TransBlndQ
		MOVD		EAX,mm0
		MOV			[EDI],AX
.MaskStBAp:
		DEC			EBX
		LEA			ESI,[ESI+2]
		LEA			EDI,[EDI+2]
		JNZ			.BcStBAp
.PasStBAp:
.FinSurfCopy:

		POP		EBX
		POP		ESI
		POP		EDI
	MMX_RETURN
