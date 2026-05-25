%include "PARAM.MAC"

; GLOBAL Function****
; 8bpp
GLOBAL	_SetDFONT, _GetDFONT,_OutText,_WidthText,_WidthPosText,_PosWidthText
; 16bpp
GLOBAL	_OutText16

; GLOBAL DATA
GLOBAL	_CurFONT, _FntPtr, _FntHaut, _FntDistLgn, _FntLowPos, _FntHighPos
GLOBAL	_FntSens, _FntTab, _FntX, _FntY, _FntCol

; EXTERN DATA
; _CurSurf
EXTERN	_vlfb,_ResH,_ResV,_MaxX,_MaxY,_MinX, _MinY, _OrgY, _OrgX, _SizeSurf
EXTERN	_OffVMem, _rlfb, _BitsPixel, _ScanLine, _Mask, _NegScanLine

SECTION .text
[BITS 32]
ALIGN 32

;***************** FONT
_SetDFONT:
	ARG	SF, 4
		MOVD		mm0,ESI
		MOVD		mm1,EDI

		MOV		ESI,[EBP+SF]
		MOV		EDI,_CurFONT
		MOV		ECX,8
		REP		MOVSD

		MOVD		ESI,mm0
		MOVD		EDI,mm1

	MMX_RETURN

_GetDFONT:
	ARG	CF, 4
		MOVD		mm0,ESI
		MOVD		mm1,EDI

		MOV		ESI,_CurFONT
		MOV		EDI,[EBP+CF]
		MOV		ECX,8
		REP		MOVSD

		MOVD		ESI,mm0
		MOVD		EDI,mm1

	MMX_RETURN

ALIGN 32
_OutText:
	ARG	Str, 4
		MOVD		mm0,ESI
		MOVD		mm1,EDI
		MOVD		mm2,EBX

		MOV		EBX,[_FntPtr]
		OR		EBX,EBX
		JZ		.FinOutText
		MOV		ESI,[EBP+Str]
		XOR		EAX,EAX
.BcOutChar:
		LODSB
		MOVD		mm3,ESI
		MOVD		mm4,EAX
		MOVD		mm5,EBX
		OR		AL,AL
		JZ		.FinOutText
;**************** Affichage et traitement **********************************
;***************************************************************************
		CMP		AL,13		 ;** Debut Cas Special
		MOVSX		ESI,BYTE [EBX+EAX*8+4] ; PlusX
		JE		.DebLigne
		XOR		EDX,EDX
		CMP		AL,10
		MOV		DL,[EBX+EAX*8+7] ;Largeur
		JE		.DebNextLigne
		XOR		ECX,ECX
		CMP		AL,9
		MOV		CL,[EBX+EAX*8+6] ; Haut
		JE		.TabCar	 ;** Fin   Cas Special
		MOVSX		EDI,BYTE [EBX+EAX*8+5] ; PlusLgn
		MOV		[ChLarg],EDX
		MOV		[ChPlusX],ESI
		MOVD		mm6,[EBX+EAX*8]        ; PtrDat
		OR		ESI,ESI
		MOV		[ChPlusLgn],EDI
		MOV		[ChHaut],ECX
		JNS		.GauchDroit
		JZ		.RienZero1
		LEA		EAX,[ESI+1]
		ADD		[_FntX],EAX
.GauchDroit:
.RienZero1:
		MOV		EBP,[_FntX]  ; MinX
		ADD		EDI,[_FntY]  ; MinY
		LEA		EBX,[EBP+EDX-1] ; MaxX: EBX=MinX+Larg-1
		LEA		ESI,[EDI+ECX-1] ; MaxY: ESI=MinY+Haut-1

		CMP		EBX,[_MaxX]
		JG		.CharClip
		CMP		ESI,[_MaxY]
		JG		.CharClip
		CMP		EBP,[_MinX]
		JL		.CharClip
		CMP		EDI,[_MinY]
		JL		.CharClip
;****** trace caractere IN *****************************
.CharIn:
		MOV		ECX,[_ResH]
		MOV		EBP,EDX ;Largeur
		IMUL		EDI,ECX
		ADD		ECX,EDX
		NEG		EDI
		MOV		[ChPlus],ECX
		ADD		EDI,[_FntX]
		MOV		EDX,[ChHaut]
		ADD		EDI,[_vlfb]
		XOR		EAX,EAX
		MOVD		ESI,mm6
		MOV		AL,[_FntCol]
.LdNext:	MOV		EBX,[ESI]
		MOV		CL,32
		ADD		ESI,BYTE 4
;ALIGN 4
.BcDrCarHline:	TEST		BL,1
		JZ		.PasDrPixel
		MOV		[EDI],AL
.PasDrPixel:
		SHR		EBX,1
		INC		EDI
		DEC		EBP
		JZ		.FinDrCarHline
		DEC		CL
		JNZ		.BcDrCarHline
		JZ		.LdNext
;ALIGN 4
.FinDrCarHline:	MOV		EBX,[ESI]
		SUB		EDI,[ChPlus]
		MOV		CL,32
		LEA		ESI,[ESI+4]
		DEC		DL
		MOV 		EBP,[ChLarg]
		JNZ		.BcDrCarHline
		JMP		.FinDrChar
;****** Trace Caractere Clip ***************************
.CharClip:
		CMP		EBX,[_MinX]
		JL		.FinDrChar
		CMP		ESI,[_MinY]
		JL		.FinDrChar
		CMP		EBP,[_MaxX]
		JG		.FinDrChar
		CMP		EDI,[_MaxY]
		JG		.FinDrChar
		; traitement MaxX********************************************
		CMP		EBX,[_MaxX]	; MaxX>_MaxX
		MOV		EAX,EBX
		JLE		.PasApPlus
		SUB		EAX,[_MaxX]	; DXAp = EAX = MaxX-_MaxX
		SUB		EDX,EAX 	; EDX = Larg-DXAp
		CMP		EAX,BYTE 32
		JL		.PasApPlus
		MOV		DWORD [ChApPlus],4
		JMP		SHORT .ApPlus
.PasApPlus:	XOR		EAX,EAX
		MOV		[ChApPlus],EAX
.ApPlus:
		; traitement MinX********************************************
		MOV		EAX,[_MinX]
		SUB		EAX,EBP
		JLE		.PasAvPlus
		SUB		EDX,EAX

		CMP		EAX,BYTE 32
		JL		.PasAvPlus2
		MOV		DWORD [ChAvPlus],4
		SUB		AL,32
		MOV		AH,32
		MOV		[ChAvDecal],AL
		SUB		AH,AL
		MOV		[ChNbBitDat],AH
		JMP		SHORT .AvPlus
.PasAvPlus2:
		MOV		AH,32
		MOV		[ChAvDecal],AL
		SUB		AH,AL
		MOV		[ChNbBitDat],AH
		XOR		EAX,EAX
		MOV		[ChAvPlus],EAX
		JMP		SHORT .AvPlus
.PasAvPlus:	XOR		EAX,EAX
		MOV		BYTE [ChNbBitDat],32
		MOV		[ChAvPlus],EAX
		MOV		[ChAvDecal],AL
.AvPlus:
		; traitement MaxY********************************************
		CMP		ESI,[_MaxY]	; MaxY>_MaxY
		MOV		EAX,ESI
		JLE		.PasSupMaxY
		SUB		EAX,[_MaxY]	; DY = EAX = MaxY-_MaxY
		SUB		ECX,EAX 	; ECX = Haut-DY
.PasSupMaxY:
		; traitement MinY********************************************
		MOV		EAX,[_MinY]
		SUB		EAX,EDI
		JLE		.PasInfMinY
		SUB		ECX,EAX
		MOV		EDI,[_MinY]
		CMP		DWORD [ChLarg],BYTE 32
		JLE		.Larg1DD
.Larg2DD:	IMUL		EAX,8
		MOVD		mm7,EAX
		PADDD		mm6,mm7
		JMP		SHORT .PasInfMinY
.Larg1DD:	IMUL		EAX,4
		MOVD		mm7,EAX
		PADDD		mm6,mm7
.PasInfMinY:
		MOV		[ChHaut],ECX
		MOV		[ChLarg],EDX
;************************************************
		MOV		ECX,[_ResH]
		MOV		EBP,EDX ;Largeur
		IMUL		EDI,ECX
		XOR		EAX,EAX
		ADD		ECX,EDX
		MOV		AL,[ChAvDecal]
		NEG		EDI
		MOV		[ChPlus],ECX
		ADD		EDI,[_FntX]
		ADD		EDI,EAX      ; EDI +=ChAvDecal
		MOV		EDX,[ChHaut]
		ADD		EDI,[_vlfb]
		MOVD		ESI,mm6

		MOV		AL,[_FntCol]  ;*************
		ADD		ESI,[ChAvPlus]
		MOV		EBX,[ESI]
		MOV		CL,[ChAvDecal]
		MOV		CH,[ChNbBitDat]
		ADD		ESI,BYTE 4
		SHR		EBX,CL
		JMP		SHORT .CBcDrCarHline

.CLdNext:	MOV		EBX,[ESI]
		MOV		CH,32
		ADD		ESI,BYTE 4
;ALIGN 4
.CBcDrCarHline:	TEST		BL,1
		JZ		.CPasDrPixel
		MOV		[EDI],AL
.CPasDrPixel:
		SHR		EBX,1
		INC		EDI
		DEC		EBP
		JZ		.CFinDrCarHline
		DEC		CH
		JNZ		.CBcDrCarHline
		JZ		.CLdNext
;ALIGN 4
.CFinDrCarHline:
		ADD		ESI,[ChApPlus]
		SUB		EDI,[ChPlus]
		ADD		ESI,[ChAvPlus]
		MOV		EBX,[ESI]
		MOV		CL,[ChAvDecal]
		MOV		CH,[ChNbBitDat]
		SHR		EBX,CL
		LEA		ESI,[ESI+4]
		DEC		DL
		MOV 		EBP,[ChLarg]
		JNZ		.CBcDrCarHline

.FinDrChar:;********************************************
		MOV		ESI,[ChPlusX]
		OR		ESI,ESI
		JS		.DroitGauch
		JZ		.RienZero2
		ADD		[_FntX],ESI
.DroitGauch:
.RienZero2:	OR		ESI,ESI
		JNS		.GauchDroit2
		JZ		.RienZero3
		MOV		EAX,[_FntX]
		DEC		EAX
		MOV		[_FntX],EAX
.GauchDroit2:
.RienZero3:
		JMP		SHORT .Norm

.DebNextLigne:	XOR		EAX,EAX 	      ;***debut trait Cas sp
		MOV		AL,[_FntDistLgn]
		MOV		EBX,[_FntY]
		SUB		EBX,EAX
		MOV		[_FntY],EBX
.DebLigne:	MOV		AL,[_FntSens]
		OR		AL,AL
		JZ		.GchDrt
		MOV		EBX,[_MaxX]
		JMP		SHORT .DrtGch
.GchDrt:	MOV		EBX,[_MinX]
.DrtGch:
		MOV		[_FntX],EBX
		JMP		SHORT .Norm
.TabCar:	MOV		AL,32	     ; TAB
		MOV		ESI,[_FntX]
		MOVZX		ECX,BYTE [_FntTab]
		MOVSX		EAX,BYTE [EBX+EAX*8+4] ; PlusX
		IMUL		ECX,EAX
		ADD		ESI,ECX
		MOV		[_FntX],ESI	;***********fin trait Cas sp
;*** FIN ******** Affichage et traitement **********************************
;***************************************************************************
.Norm:		MOVD		ESI,mm3
		MOVD		EAX,mm4
		MOVD		EBX,mm5
		JMP		.BcOutChar
.FinOutText:
		MOVD		ESI,mm0
		MOVD		EDI,mm1
		MOVD		EBX,mm2

	MMX_RETURN

_WidthText:
	ARG	LStr, 4
		MOVD		mm0,ESI
		MOVD		mm2,EBX

		MOV		EBX,[_FntPtr]
		XOR		ECX,ECX
		OR		EBX,EBX
		JZ		.FinLargText
		MOV		ESI,[EBP+LStr]
		XOR		EAX,EAX
.BcCalcLarg:
		LODSB
		OR		AL,AL
		JZ		.FinLargText
		CMP		AL,13
		JZ		.FinLargText
		CMP		AL,10
		JZ		.FinLargText
		CMP		AL,9	   ; Tab
		JNE		.PasTrtTab
		MOV		AL,32
		MOVSX		EDX,BYTE [EBX+EAX*8+4] ; space
		XOR		EAX,EAX
		MOV		AL,[_FntTab]
		IMUL		EDX,EAX
		ADD		ECX,EDX
		JMP		SHORT .BcCalcLarg
.PasTrtTab:
		MOVSX		EDX,BYTE [EBX+EAX*8+4]
		ADD		ECX,EDX
		JMP		SHORT .BcCalcLarg
.FinLargText:
		OR		ECX,ECX
		JNS		.Positiv
		NEG		ECX
.Positiv:	MOV		EAX,ECX
		OR		EAX,EAX
		JZ		.ZeroRien
		DEC		EAX
.ZeroRien:
		MOVD		ESI,mm0
		MOVD		EBX,mm2

	MMX_RETURN

_WidthPosText:
	ARG	LPStr, 4, LPPos, 4
		MOVD		mm0,ESI
		MOVD		mm1,EDI
		MOVD		mm2,EBX

		MOV		EBX,[_FntPtr]
		XOR		ECX,ECX
		OR		EBX,EBX
		JZ		.FinLargText
		MOV		ESI,[EBP+LPStr]
		MOV		EDI,[EBP+LPPos]
		XOR		EAX,EAX
		OR		EDI,EDI
		JZ		.FinLargText
		ADD		EDI,ESI
.BcCalcLarg:	XOR		EAX,EAX
		CMP		EDI,ESI
		JBE		.FinLargText
		LODSB
		OR		AL,AL
		JZ		.FinLargText
		CMP		AL,13
		JE		.FinLargText
		CMP		AL,10
		JE		.FinLargText
		CMP		AL,9	   ; Tab
		JNE		.PasTrtTab
		MOV		AL,32
		MOVSX		EDX,BYTE [EBX+EAX*8+4] ; space
		XOR		EAX,EAX
		MOV		AL,[_FntTab]
		IMUL		EDX,EAX
		ADD		ECX,EDX
		JMP		SHORT .BcCalcLarg
.PasTrtTab:
		MOVSX		EDX,BYTE [EBX+EAX*8+4]
		ADD		ECX,EDX
		JMP		SHORT .BcCalcLarg
.FinLargText:
		OR		ECX,ECX
		JNS		.Positiv
		NEG		ECX
.Positiv:	MOV		EAX,ECX
		OR		EAX,EAX
		JZ		.ZeroRien
		DEC		EAX
.ZeroRien:
		MOVD		ESI,mm0
		MOVD		EDI,mm1
		MOVD		EBX,mm2
		;EMMS
		MMX_RETURN

_PosWidthText:
	ARG	PLStr, 4, PLLarg, 4
		MOVD		mm0,ESI
		MOVD		mm1,EDI
		MOVD		mm2,EBX

		MOV		EBX,[_FntPtr]
		XOR		ECX,ECX
		OR		EBX,EBX
		JZ		.FinPosLargText
		MOV		ESI,[EBP+PLStr]
		XOR		EDI,EDI
.BcCalcLarg:	XOR		EAX,EAX
		CMP		ECX,[EBP+PLLarg]
		JAE		.FinPosLargText
		LODSB
		INC		EDI
		OR		AL,AL
		JZ		.FinPosLargText
		CMP		AL,13
		JE		.FinPosLargText
		CMP		AL,10
		JE		.FinPosLargText
		CMP		AL,9	   ; Tab
		JNE		.PasTrtTab
		MOV		AL,32
		MOVSX		EDX,BYTE [EBX+EAX*8+4] ; space
		XOR		EAX,EAX
		MOV		AL,[_FntTab]
		IMUL		EDX,EAX
		OR		EDX,EDX
		JS		.NegTab
		ADD		ECX,EDX
		JMP		SHORT .BcCalcLarg
.NegTab:	SUB		ECX,EDX
		JMP		SHORT .BcCalcLarg
.PasTrtTab:
		MOVSX		EDX,BYTE [EBX+EAX*8+4]
		OR		EDX,EDX
		JS		.NegNorm
		ADD		ECX,EDX
		JMP		SHORT .BcCalcLarg
.NegNorm:	SUB		ECX,EDX
		JMP		SHORT .BcCalcLarg

.FinPosLargText:
		OR		EDI,EDI
		JZ		.PosZero
		DEC		EDI
.PosZero:
		MOV		EAX,EDI

		MOVD		ESI,mm0
		MOVD		EDI,mm1
		MOVD		EBX,mm2

		MMX_RETURN


;------------------------------------------------
; 16bpp *****************************************
;------------------------------------------------

ALIGN 32
_OutText16:
	ARG	Str16, 4
		MOVD		mm0,ESI
		MOVD		mm1,EDI
		MOVD		mm2,EBX

		MOV		EBX,[_FntPtr]
		OR		EBX,EBX
		JZ		.FinOutText
		MOV		ESI,[EBP+Str16]
		XOR		EAX,EAX
.BcOutChar:
		LODSB
		MOVD		mm3,ESI
		MOVD		mm4,EAX
		MOVD		mm5,EBX
		OR		AL,AL
		JZ		.FinOutText
;**************** Affichage et traitement **********************************
;***************************************************************************
		CMP		AL,13		 ;** Debut Cas Special
		MOVSX		ESI,BYTE [EBX+EAX*8+4] ; PlusX
		JE		.DebLigne
		XOR		EDX,EDX
		CMP		AL,10
		MOV		DL,[EBX+EAX*8+7] ;Largeur
		JE		.DebNextLigne
		XOR		ECX,ECX
		CMP		AL,9
		MOV		CL,[EBX+EAX*8+6] ; Haut
		JE		.TabCar	 ;** Fin   Cas Special
		MOVSX		EDI,BYTE [EBX+EAX*8+5] ; PlusLgn
		MOV		[ChLarg],EDX
		MOV		[ChPlusX],ESI
		MOVD		mm6,[EBX+EAX*8]        ; PtrDat
		OR		ESI,ESI
		MOV		[ChPlusLgn],EDI
		MOV		[ChHaut],ECX
		JNS		.GauchDroit
		JZ		.RienZero1
		LEA		EAX,[ESI+1]
		ADD		[_FntX],EAX
.GauchDroit:
.RienZero1:
		MOV		EBP,[_FntX]  ; MinX
		ADD		EDI,[_FntY]  ; MinY
		LEA		EBX,[EBP+EDX-1] ; MaxX: EBX=MinX+Larg-1
		LEA		ESI,[EDI+ECX-1] ; MaxY: ESI=MinY+Haut-1

		CMP		EBX,[_MaxX]
		JG		.CharClip
		CMP		ESI,[_MaxY]
		JG		.CharClip
		CMP		EBP,[_MinX]
		JL		.CharClip
		CMP		EDI,[_MinY]
		JL		.CharClip
;****** trace caractere IN *****************************
.CharIn:
		MOV		ECX,[_ScanLine]
		MOV		EBP,EDX ;Largeur
		IMUL	EDI,ECX
		LEA		ECX,[ECX+EDX*2]
		NEG		EDI
		MOV		[ChPlus],ECX
		ADD		EDI,[_FntX]
		ADD		EDI,[_FntX]
		MOV		EDX,[ChHaut]
		ADD		EDI,[_vlfb]
		XOR		EAX,EAX
		MOVD		ESI,mm6
		MOV		EAX,[_FntCol]
.LdNext:	MOV		EBX,[ESI]
		MOV		CL,32
		ADD		ESI,BYTE 4
;ALIGN 4
.BcDrCarHline:	TEST		BL,1
		JZ		.PasDrPixel
		MOV		[EDI],AX
.PasDrPixel:
		SHR		EBX,1
		DEC		EBP
		LEA		EDI,[EDI+2]
		JZ		.FinDrCarHline
		DEC		CL
		JNZ		.BcDrCarHline
		JZ		.LdNext
;ALIGN 4
.FinDrCarHline:	MOV		EBX,[ESI]
		SUB		EDI,[ChPlus]
		MOV		CL,32
		LEA		ESI,[ESI+4]
		DEC		DL
		MOV 		EBP,[ChLarg]
		JNZ		.BcDrCarHline
		JMP		.FinDrChar
;****** Trace Caractere Clip ***************************
.CharClip:
		CMP		EBX,[_MinX]
		JL		.FinDrChar
		CMP		ESI,[_MinY]
		JL		.FinDrChar
		CMP		EBP,[_MaxX]
		JG		.FinDrChar
		CMP		EDI,[_MaxY]
		JG		.FinDrChar
		; traitement MaxX********************************************
		CMP		EBX,[_MaxX]	; MaxX>_MaxX
		MOV		EAX,EBX
		JLE		.PasApPlus
		SUB		EAX,[_MaxX]	; DXAp = EAX = MaxX-_MaxX
		SUB		EDX,EAX 	; EDX = Larg-DXAp
		CMP		EAX,BYTE 32
		JL		.PasApPlus
		MOV		DWORD [ChApPlus],4
		JMP		SHORT .ApPlus
.PasApPlus:	XOR		EAX,EAX
		MOV		[ChApPlus],EAX
.ApPlus:
		; traitement MinX********************************************
		MOV		EAX,[_MinX]
		SUB		EAX,EBP
		JLE		.PasAvPlus
		SUB		EDX,EAX

		CMP		EAX,BYTE 32
		JL		.PasAvPlus2
		MOV		DWORD [ChAvPlus],4
		SUB		AL,32
		MOV		AH,32
		MOV		[ChAvDecal],AL
		SUB		AH,AL
		MOV		[ChNbBitDat],AH
		JMP		SHORT .AvPlus
.PasAvPlus2:
		MOV		AH,32
		MOV		[ChAvDecal],AL
		SUB		AH,AL
		MOV		[ChNbBitDat],AH
		XOR		EAX,EAX
		MOV		[ChAvPlus],EAX
		JMP		SHORT .AvPlus
.PasAvPlus:	XOR		EAX,EAX
		MOV		BYTE [ChNbBitDat],32
		MOV		[ChAvPlus],EAX
		MOV		[ChAvDecal],AL
.AvPlus:
		; traitement MaxY********************************************
		CMP		ESI,[_MaxY]	; MaxY>_MaxY
		MOV		EAX,ESI
		JLE		.PasSupMaxY
		SUB		EAX,[_MaxY]	; DY = EAX = MaxY-_MaxY
		SUB		ECX,EAX 	; ECX = Haut-DY
.PasSupMaxY:
		; traitement MinY********************************************
		MOV		EAX,[_MinY]
		SUB		EAX,EDI
		JLE		.PasInfMinY
		SUB		ECX,EAX
		MOV		EDI,[_MinY]
		CMP		DWORD [ChLarg],BYTE 32
		JLE		.Larg1DD
.Larg2DD:	IMUL		EAX,8
		MOVD		mm7,EAX
		PADDD		mm6,mm7
		JMP		SHORT .PasInfMinY
.Larg1DD:	IMUL		EAX,4
		MOVD		mm7,EAX
		PADDD		mm6,mm7
.PasInfMinY:
		MOV		[ChHaut],ECX
		MOV		[ChLarg],EDX
;************************************************
		MOV		ECX,[_ScanLine]
		MOV		EBP,EDX ;Largeur
		IMUL	EDI,ECX
		XOR		EAX,EAX
		LEA		ECX,[ECX+EDX*2] ;
		MOV		AL,[ChAvDecal]
		NEG		EDI
		MOV		[ChPlus],ECX
		ADD		EDI,[_FntX]
		ADD		EDI,[_FntX]  ; 2*_FntX 16bpp
		LEA		EDI,[EDI+EAX*2]      ; EDI +=2*ChAvDecal
		MOV		EDX,[ChHaut]
		ADD		EDI,[_vlfb]
		MOVD		ESI,mm6

		MOV		EAX,[_FntCol]  ;*************
		ADD		ESI,[ChAvPlus]
		MOV		EBX,[ESI]
		MOV		CL,[ChAvDecal]
		MOV		CH,[ChNbBitDat]
		ADD		ESI,BYTE 4
		SHR		EBX,CL
		JMP		SHORT .CBcDrCarHline

.CLdNext:	MOV		EBX,[ESI]
		MOV		CH,32
		ADD		ESI,BYTE 4
;ALIGN 4
.CBcDrCarHline:	TEST		BL,1
		JZ		.CPasDrPixel
		MOV		[EDI],AX
.CPasDrPixel:
		SHR		EBX,1
		DEC		EBP
		LEA		EDI,[EDI+2]
		JZ		.CFinDrCarHline
		DEC		CH
		JNZ		.CBcDrCarHline
		JZ		.CLdNext
;ALIGN 4
.CFinDrCarHline:
		ADD		ESI,[ChApPlus]
		SUB		EDI,[ChPlus]
		ADD		ESI,[ChAvPlus]
		MOV		EBX,[ESI]
		MOV		CL,[ChAvDecal]
		MOV		CH,[ChNbBitDat]
		SHR		EBX,CL
		LEA		ESI,[ESI+4]
		DEC		DL
		MOV 		EBP,[ChLarg]
		JNZ		.CBcDrCarHline

.FinDrChar:;********************************************
		MOV		ESI,[ChPlusX]
		OR		ESI,ESI
		JS		.DroitGauch
		JZ		.RienZero2
		ADD		[_FntX],ESI
.DroitGauch:
.RienZero2:	OR		ESI,ESI
		JNS		.GauchDroit2
		JZ		.RienZero3
		MOV		EAX,[_FntX]
		DEC		EAX
		MOV		[_FntX],EAX
.GauchDroit2:
.RienZero3:
		JMP		SHORT .Norm

.DebNextLigne:	XOR		EAX,EAX 	      ;***debut trait Cas sp
		MOV		AL,[_FntDistLgn]
		MOV		EBX,[_FntY]
		SUB		EBX,EAX
		MOV		[_FntY],EBX
.DebLigne:	MOV		AL,[_FntSens]
		OR		AL,AL
		JZ		.GchDrt
		MOV		EBX,[_MaxX]
		JMP		SHORT .DrtGch
.GchDrt:	MOV		EBX,[_MinX]
.DrtGch:
		MOV		[_FntX],EBX
		JMP		SHORT .Norm
.TabCar:	MOV		AL,32	     ; TAB
		MOV		ESI,[_FntX]
		MOVZX		ECX,BYTE [_FntTab]
		MOVSX		EAX,BYTE [EBX+EAX*8+4] ; PlusX
		IMUL		ECX,EAX
		ADD		ESI,ECX
		MOV		[_FntX],ESI	;***********fin trait Cas sp
;*** FIN ******** Affichage et traitement **********************************
;***************************************************************************
.Norm:		MOVD		ESI,mm3
		MOVD		EAX,mm4
		MOVD		EBX,mm5
		JMP		.BcOutChar
.FinOutText:
		MOVD		ESI,mm0
		MOVD		EDI,mm1
		MOVD		EBX,mm2

		MMX_RETURN

SECTION .bss   ALIGN=32
_CurFONT:
_FntPtr			RESD   1
_FntHaut		RESB   1
_FntDistLgn		RESB   1
_FntLowPos		RESB   1
_FntHighPos		RESB   1
_FntSens		RESB   1
_FntTab			RESB   3 ; 2 DB reserv
_FntX			RESD   1
_FntY			RESD   1
_FntCol			RESD   1
FntResv			RESD   2 ;---------------------
ChHaut			RESD   1
ChLarg			RESD   1
ChPlus			RESD   1
ChPlusX			RESD   1
ChPlusLgn		RESD   1
ChAvPlus		RESD   1
ChApPlus		RESD   1
ChAvDecal		RESB   1
ChNbBitDat		RESB   1
ChResvW			RESW   1;-----------------------
