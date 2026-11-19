;******************
; 16bpp SUPPORT !!!
;******************
ALIGN 32
_line16:
	ARG   	LX16P1, 4, LY16P1, 4, LX16P2, 4, LY16P2, 4, lnCol16, 4

		MOVD		mm7,EBX
		MOVD		mm6,EDI
		MOVD		mm5,ESI

		MOV		EAX,[EBP+LX16P1]
		MOV		EBX,[EBP+LX16P2]
		MOV		ESI,[EBP+LY16P1]
		MOV		EDI,[EBP+LY16P2]
		MOV		ECX,[EBP+lnCol16]

		MOV		[XP1],EAX
		MOV		[XP2],EBX
		MOV		[YP1],ESI
		MOV		[YP2],EDI
		MOV		[clr],ECX
		JMP		SHORT _Line16.DoLine16

ALIGN 32
_Line16:
	ARG   	Ptr16P1, 4, Ptr16P2, 4, Col16, 4

		MOVD		mm7,EBX
		MOVD		mm6,EDI
		MOVD		mm5,ESI

		MOV		EAX,[EBP+Col16]
		MOV		ECX,[EBP+Ptr16P1]
		MOV		[clr],EAX
		MOV		EDX,[EBP+Ptr16P2]
		MOV		EAX,[ECX]   ; X1
		MOV		EBX,[EDX]   ; X2
		MOV		ESI,[ECX+4] ; Y1
		MOV		EDI,[EDX+4] ; Y2
		MOV		[XP1],EAX   ; X1
		MOV		[XP2],EBX   ; X2
		MOV		[YP1],ESI 	; Y1
		MOV		[YP2],EDI 	; Y2
.DoLine16:
		;MOV		EAX,[XP1]   ; ligne en dehors de la fenetre ?
		;MOV		EBX,[XP2]
		MOV		EDX,EAX
		CMP		EAX,EBX
		JL		.VMaxX
		XCHG		EAX,EBX
.VMaxX:		CMP		EAX,[_MaxX]
		JG		.FinLine
		CMP		EBX,[_MinX]
		JL		.FinLine
		SUB		EBX,EAX
		MOV		ESI,EBX	   ;calcul de abs(x2-x1)

		MOV		EAX,[YP1]
		MOV		EBX,EDI ; [YP2]
		MOV		ECX,EAX
		CMP		EAX,EBX
		JL		.VMaxY
		XCHG		EAX,EBX
.VMaxY:		CMP		EAX,[_MaxY]
		JG		.FinLine
		CMP		EBX,[_MinY]
		JL		.FinLine       ; fin du test
		SUB		EBX,EAX
		MOV		EDI,EBX		   ;  abs(y2-y1)

		OR		EDI,EDI
		JZ		.cas4
.PasNegEDI:	OR		ESI,ESI
		JZ		.cas2

		INC		ESI			  ; abs(x2-x1)+1
		INC		EDI			  ; abs(y2-y1)+1
		MOV		EAX,EDX			; EDX = [XP1]
		MOV		EBX,[XP2]	 ; cas 1 et cas 2
		CMP		EAX,EBX
		JL		.ClipMaxX
		XCHG		EAX,EBX
.ClipMaxX:	CMP		EAX,[_MinX]
		JL		.Aj_1_2
		CMP		EBX,[_MaxX]
		JG		.Aj_1_2
		MOV		EAX,ECX			; ECX = [YP1]
		MOV		EBX,[YP2]
		CMP		EAX,EBX
		JL		.ClipMaxY
		XCHG		EAX,EBX
.ClipMaxY:	CMP		EAX,[_MinY]
		JL		.Aj_1_2
		CMP		EBX,[_MaxY]
		JG		.Aj_1_2
		JMP		.PasAj_1_2
.Aj_1_2:	MOV		EBX,EDX			; EDX = [XP1]
		MOV		ESI,[XP2]
		MOV		EDI,[YP2]
		CMP		EBX,ESI
		JL		.MaxAj1_2X
		XCHG		EBX,ESI
		XCHG		ECX,EDI
.MaxAj1_2X:
;*********Ajustement des X
		CMP       	EBX,[_MinX]
		JNL       	.PasAjX12
		MOV       	EAX,EDI
		SUB       	EAX,ECX
		SAL       	EAX,Prec
		SUB       	ESI,EBX
		CDQ
		IDIV      	ESI
		ADD       	ESI,EBX
		MOV       	EDX,EAX
		MOV       	EAX,[_MinX]
		SUB       	EAX,EBX
		IMUL      	EAX,EDX
		SAR       	EAX,Prec
		MOV       	EBX,[_MinX]
		ADD       	ECX,EAX
.PasAjX12:	CMP       	ESI,[_MaxX]
		JNG       	.PasAjM12
		MOV       	EAX,EDI
		SUB       	EAX,ECX
		SAL       	EAX,Prec
		SUB       	ESI,EBX
		OR        	ESI,ESI
		JZ        	.FinLine
		CDQ
		IDIV      	ESI
		ADD       	ESI,EBX
		MOV       	EDX,EAX
		MOV       	EAX,ESI
		SUB       	EAX,[_MaxX]
		IMUL      	EAX,EDX
		SAR       	EAX,Prec
		MOV       	ESI,[_MaxX]
		SUB       	EDI,EAX
.PasAjM12:	CMP       	ECX,EDI
		JL        	.MaxAj1_2Y
		XCHG      	EBX,ESI
		XCHG      	ECX,EDI
.MaxAj1_2Y:	CMP       	ECX,[_MaxY]
		JG        	.FinLine
		CMP       	EDI,[_MinY]
		JL        	.FinLine
;*********Ajustement des Y
		CMP       	ECX,[_MinY]
		JNL       	.PasAjY12
		MOV       	EAX,ESI
		SUB       	EAX,EBX
		MOV	        EBP,ESI
		SAL       	EAX,Prec
		MOV       	ESI,EDI
		SUB       	ESI,ECX
		CDQ
		IDIV      	ESI
		MOV	        ESI,EBP
		MOV       	EDX,EAX
		MOV       	EAX,[_MinY]
		SUB       	EAX,ECX
		IMUL      	EAX,EDX
		SAR       	EAX,Prec
		MOV       	ECX,[_MinY]
		ADD       	EBX,EAX
		CMP       	EBX,[_MaxX]
		JG        	.FinLine
		CMP       	EBX,[_MinX]
		JL        	.FinLine
.PasAjY12:      CMP       	EDI,[_MaxY]
		JNG       	.PasAjY12X
		MOV       	EAX,ESI
		SUB       	EAX,EBX
		MOV		EBP,ESI
		SAL       	EAX,Prec
		MOV		ESI,EDI
		SUB		ESI,ECX
		CDQ
		IDIV		ESI
		MOV		ESI,EBP
		MOV		EDX,EAX
		MOV		EAX,EDI
		SUB		EAX,[_MaxY]
		IMUL		EAX,EDX
		SAR		EAX,Prec
		MOV		EDI,[_MaxY]
		SUB		ESI,EAX
.PasAjY12X:
		MOV       	[XP1],EBX
		MOV       	[YP1],ECX
		MOV       	[XP2],ESI
		MOV       	[YP2],EDI
		SUB       	ESI,EBX
		SUB       	EDI,ECX
		OR        	ESI,ESI

		JZ		.cas2
		JNS		.PasNegESI2
		NEG		ESI
.PasNegESI2:
		OR		EDI,EDI
		JZ		.cas4
		JNS		.PasNegEDI2
		NEG		EDI
.PasNegEDI2:
.PasAj_1_2:     CMP       	ESI,EDI
                JB        	.cas2

;*********CAS 1:  (DX > DY)***************************************************
.cas1:
		MOV		EAX,[XP1]
		MOV		EBP,[_ScanLine] ; plus
		CMP		EAX,[XP2]
		JL		.PasSwap1
		XCHG		EAX,[XP2]
		MOV		[XP1],EAX
		MOV		EAX,[YP1]
		MOV		EBX,[YP2]
		MOV		[YP1],EBX
		MOV		[YP2],EAX
.PasSwap1:
		MOV		ESI,[XP2]
		MOV		EAX,[YP2]
		SUB		ESI,[XP1]
		SUB		EAX,[YP1]
		MOV		EDI,EBP  ;[_ScanLine]
		JNS		.pstvDyCas1
		NEG		EAX ; abs(deltay)
		JMP		SHORT .ngtvDyCas1
.pstvDyCas1:	NEG		EBP ; = -[_ScanLine] as ascendent y Axis
.ngtvDyCas1:
		INC		EAX
		MOV		EBX,1 << Prec ; EBX = cpt Dbrd
		SHL		EAX,Prec
		INC		ESI ; deltaX + 1
		CDQ
		IDIV		ESI
		MOV		ECX,ESI ; ECX = deltaX = number pixels
		MOV		EDX,EAX ; EDX = pnt
		IMUL		EDI,[YP1]
		NEG		EDI    ; // Y Axis  ascendent
		ADD		EDI,[XP1]
		MOV		EAX,[clr]
		ADD		EDI,[XP1] ; 2 time cause 16bpp
		MOV		ESI,1 << Prec
		ADD		EDI,[_vlfb]
ALIGN 4
.lp_line1:
		SUB		EBX,EDX
		MOV		[EDI],AX
		JA		.no_debor1 ; EDI >0
		ADD		EBX,ESI ; +  (1 << Prec)
		ADD		EDI,EBP	 ; EDI + = directional ScanLine
.no_debor1:
		DEC		ECX
		LEA		EDI,[EDI+2]	; EDI + 2
		JNZ		.lp_line1

		JMP		.FinLine
;*********CAS 2:  (DY > DX)***************************************************
.cas2:
		OR		EDI,EDI
		MOV		EAX,[YP1]
		JZ		.cas5
		CMP		EAX,[YP2]
		JL		.PasSwap2
		XCHG		EAX,[YP2]
		MOV		[YP1],EAX
		MOV		ECX,[XP2]
		MOV		EAX,[XP1]
		MOV		[XP1],ECX
		MOV		[XP2],EAX
.PasSwap2:
		OR		ESI,ESI
		JNZ		.noClipVert
		MOV		EAX,[_MinY]
		MOV		EBX,[_MaxY]
		CMP		EAX,[YP1]
		JLE		.sava21
		MOV		[YP1],EAX
.sava21:	CMP		EBX,[YP2]
		JGE		.sava22
		MOV		[YP2],EBX
.sava22:
.noClipVert:
		MOV		EAX,[XP2]
		MOV		ESI,[YP2]
		SUB		EAX,[XP1]
		JNS		.pstvDxCas2
		DEC		EAX
		JMP		SHORT .ngtvDxCas2
.pstvDxCas2:	INC		EAX
.ngtvDxCas2:
		SUB		ESI,[YP1]
		SHL		EAX,Prec
		MOV		EBP,[_ScanLine]
		INC		ESI
		MOV		EDI,[YP1]
		CDQ
		IMUL		EDI,EBP ; * ScanLine
		IDIV		ESI
		MOV		ECX,ESI ; ECX = deltaY = number pixels
		NEG		EDI
		MOV		EDX,EAX ; pente in EDX

		; start adress
		ADD		EDI,[XP1]
		NEG		EBP ; = - ScanLine
		ADD		EDI,[XP1] ; add xp1 2 times as 16 bpp
		XOR		EBX,EBX ; accum in EBX
		ADD		EDI,[_vlfb]
		OR		EDX,EDX
		MOV		EAX,[clr] ; draw color
		JNS		.line2_pstvPnt
		MOV		EBX,((1<<Prec)-1)
.line2_pstvPnt:
ALIGN 4
.lp_line2:
		MOV		ESI,EBX
		SAR		ESI,Prec
		ADD		EBX,EDX ; + pnt
		MOV		[EDI+ESI*2],AX
		DEC		ECX
		LEA		EDI,[EDI+EBP]	  ;  Axe Y Montant -ResH
		JNZ		.lp_line2

		JMP		.FinLine
;*******CAS 3 :  (DX=0)*****************************************************
;*******CAS 4 :  (DY=0)*****************************************************
.cas4:		MOV		ECX,ESI
		MOV		EAX,[XP1]
		CMP		EAX,[XP2]
		JL		.PasSwap4
		MOV		EAX,[XP1]
		MOV		EBX,[XP2]
		MOV		[XP1],EBX
		MOV		[XP2],EAX
.PasSwap4:	MOV		EAX,[_MinX]
		CMP		EAX,[XP1]
		JLE		.sava41
		MOV		[XP1],EAX
.sava41:	MOV		EAX,[_MaxX]
		CMP		EAX,[XP2]
		JGE		.sava42
		MOV		[XP2],EAX
.sava42:
		MOV		ESI,[XP2]
		SUB		ESI,[XP1]
		OR		ESI,ESI
		JZ		.cas5
		INC		ESI
		MOV		EDI,[YP1]
		IMUL		EDI,[_NegScanLine]
		MOVD		mm0,[clr]
		ADD		EDI,[_vlfb]
		PUNPCKLWD	mm0,mm0
		ADD		EDI,[XP1]
		PUNPCKLDQ	mm0,mm0
		ADD		EDI,[XP1]
		MOVD		EAX,mm0
		@SolidHLine16
		JMP		SHORT .FinLine
;********CAS 5 : (DX=0, DY=0)***********************************************
.cas5:
		MOV		EAX,[_NegScanLine]
		IMUL	EAX,[YP1]
		MOV		EDX,[XP1]
		MOV		ECX,[clr]
		ADD		EAX,[_vlfb]
		MOV		[EAX+EDX*2],CX
.FinLine:
		MOVD		EBX,mm7
		MOVD		EDI,mm6
		MOVD		ESI,mm5

	MMX_RETURN

ALIGN 32
_linemap16:
	ARG   	LMX16P1, 4, LMY16P1, 4, LMX16P2, 4, LMY16P2, 4, lnMCol16, 4, LM16Map, 4

		MOVD		mm7,EBX
		MOVD		mm6,EDI
		MOVD		mm5,ESI

		MOV		EAX,[EBP+LMX16P1]
		MOV		EBX,[EBP+LMX16P2]
		MOV		ECX,[EBP+LMY16P1]
		MOV		EDX,[EBP+LMY16P2]
		MOV		ESI,[EBP+lnMCol16]
		MOV		EDI,[EBP+LM16Map]

		MOV		[XP1],EAX
		MOV		[XP2],EBX
		MOV		[YP1],ECX
		MOV		[YP2],EDX
		MOV		[clr],ESI
		MOV		[Plus2],EDI
		JMP		SHORT _LineMap16.DoLine16

ALIGN 32
_LineMap16:
	ARG   	Map16PtrP1, 4, Map16PtrP2, 4, Map16Col, 4, Line16Map, 4

		MOVD		mm7,EBX
		MOVD		mm6,EDI
		MOVD		mm5,ESI

		MOV		EDX,[EBP+Map16PtrP1]
		MOV		ECX,[EBP+Map16PtrP2]

		MOV		EAX,[EDX]   ; X1
		MOV		EBX,[ECX]   ; X2
		MOV		ESI,[EDX+4] ; Y1
		MOV		EDI,[ECX+4] ; Y2
		MOV		[XP1],EAX   ; X1
		MOV		[XP2],EBX   ; X2
		MOV		[YP1],ESI 	; Y1
		MOV		EAX,[EBP+Map16Col]
		MOV		[YP2],EDI 	; Y2
		MOV		EBX,[EBP+Line16Map]
		MOV		[clr],EAX
		MOV		[Plus2],EBX
.DoLine16:
		MOV		EAX,[XP1]   ; ligne en dehors de la fenetre ?
		MOV		EBX,[XP2]
		MOV		EDX,EAX
		CMP		EAX,EBX
		JL		.VMaxX
		XCHG		EAX,EBX
.VMaxX:		CMP		EAX,[_MaxX]
		JG		.FinLine
		CMP		EBX,[_MinX]
		JL		.FinLine
		SUB		EBX,EAX
		MOV		ESI,EBX	   ;calcul de abs(x2-x1)

		MOV		EAX,[YP1]
		MOV		EBX,[YP2]
		MOV		ECX,EAX
		CMP		EAX,EBX
		JL		.VMaxY
		XCHG		EAX,EBX
.VMaxY:		CMP		EAX,[_MaxY]
		JG		.FinLine
		CMP		EBX,[_MinY]
		JL		.FinLine       ; fin du test
		SUB		EBX,EAX
		MOV		EDI,EBX		   ;  abs(y2-y1)

		OR		EDI,EDI
		JZ		.cas4
.PasNegEDI:	OR		ESI,ESI
		JZ		.cas2

		INC		ESI			  ; abs(x2-x1)+1
		INC		EDI			  ; abs(y2-y1)+1
		MOV		EAX,EDX			; EDX = [XP1]
		MOV		EBX,[XP2]	 ; cas 1 et cas 2
		CMP		EAX,EBX
		JL		.ClipMaxX
		XCHG		EAX,EBX
.ClipMaxX:	CMP		EAX,[_MinX]
		JL		.Aj_1_2
		CMP		EBX,[_MaxX]
		JG		.Aj_1_2
		MOV		EAX,ECX			; ECX = [YP1]
		MOV		EBX,[YP2]
		CMP		EAX,EBX
		JL		.ClipMaxY
		XCHG		EAX,EBX
.ClipMaxY:	CMP		EAX,[_MinY]
		JL		.Aj_1_2
		CMP		EBX,[_MaxY]
		JG		.Aj_1_2
		JMP		.PasAj_1_2
.Aj_1_2:	MOV		EBX,EDX			; EDX = [XP1]
		MOV		ESI,[XP2]
		MOV		EDI,[YP2]
		CMP		EBX,ESI
		JL		.MaxAj1_2X
		XCHG		EBX,ESI
		XCHG		ECX,EDI
.MaxAj1_2X:
;*********Ajustement des X
		CMP       	EBX,[_MinX]
		JNL       	.PasAjX12
		MOV       	EAX,EDI
		SUB       	EAX,ECX
		SAL       	EAX,Prec
		SUB       	ESI,EBX
		CDQ
		IDIV      	ESI
		ADD       	ESI,EBX
		MOV       	EDX,EAX
		MOV       	EAX,[_MinX]
		SUB       	EAX,EBX
		IMUL      	EAX,EDX
		SAR       	EAX,Prec
		MOV       	EBX,[_MinX]
		ADD       	ECX,EAX
.PasAjX12:	CMP       	ESI,[_MaxX]
		JNG       	.PasAjM12
		MOV       	EAX,EDI
		SUB       	EAX,ECX
		SAL       	EAX,Prec
		SUB       	ESI,EBX
		OR        	ESI,ESI
		JZ        	.FinLine
		CDQ
		IDIV      	ESI
		ADD       	ESI,EBX
		MOV       	EDX,EAX
		MOV       	EAX,ESI
		SUB       	EAX,[_MaxX]
		IMUL      	EAX,EDX
		SAR       	EAX,Prec
		MOV       	ESI,[_MaxX]
		SUB       	EDI,EAX
.PasAjM12:	CMP       	ECX,EDI
		JL        	.MaxAj1_2Y
		XCHG      	EBX,ESI
		XCHG      	ECX,EDI
.MaxAj1_2Y:	CMP       	ECX,[_MaxY]
		JG        	.FinLine
		CMP       	EDI,[_MinY]
		JL        	.FinLine
;*********Ajustement des Y
		CMP       	ECX,[_MinY]
		JNL       	.PasAjY12
		MOV       	EAX,ESI
		SUB       	EAX,EBX
		MOV	        EBP,ESI ; sauve ESI
		SAL       	EAX,Prec
		MOV       	ESI,EDI
		SUB       	ESI,ECX
		CDQ
		IDIV      	ESI
		MOV	        ESI,EBP ; rest ESI
		MOV       	EDX,EAX
		MOV       	EAX,[_MinY]
		SUB       	EAX,ECX
		IMUL      	EAX,EDX
		SAR       	EAX,Prec
		MOV       	ECX,[_MinY]
		ADD       	EBX,EAX
		CMP       	EBX,[_MaxX]
		JG        	.FinLine
		CMP       	EBX,[_MinX]
		JL        	.FinLine
.PasAjY12:      CMP       	EDI,[_MaxY]
		JNG       	.PasAjY12X
		MOV       	EAX,ESI
		SUB       	EAX,EBX
		MOV	        EBP,ESI ; sauve ESI
		SAL       	EAX,Prec
		MOV       	ESI,EDI
		SUB       	ESI,ECX
		CDQ
		IDIV      	ESI
		MOV	        ESI,EBP ; rest ESI
		MOV       	EDX,EAX
		MOV       	EAX,EDI
		SUB       	EAX,[_MaxY]
		IMUL      	EAX,EDX
		SAR       	EAX,Prec
		MOV       	EDI,[_MaxY]
		SUB       	ESI,EAX
.PasAjY12X:
		MOV		[XP1],EBX
		MOV		[YP1],ECX
		MOV       	[XP2],ESI
		MOV       	[YP2],EDI
		SUB       	ESI,EBX
		SUB       	EDI,ECX
		OR        	ESI,ESI

		JZ		.cas2
		JNS		.PasNegESI2
		NEG		ESI
.PasNegESI2:
		OR		EDI,EDI
		JZ		.cas4
		JNS		.PasNegEDI2
		NEG		EDI
.PasNegEDI2:
.PasAj_1_2:     CMP       	ESI,EDI
                JB        	.cas2

;*********CAS 1:  (DX > DY)***************************************************
.cas1:
		MOV		EAX,[XP1]
		MOV		EBP,[_ScanLine] ; plus
		CMP		EAX,[XP2]
		JL		.PasSwap1
		XCHG		EAX,[XP2]
		MOV		[XP1],EAX
		MOV		EAX,[YP1]
		MOV		EBX,[YP2]
		MOV		[YP1],EBX
		MOV		[YP2],EAX
.PasSwap1:
		MOV		ESI,[XP2]
		MOV		EAX,[YP2]
		SUB		ESI,[XP1]
		SUB		EAX,[YP1]
		MOV		EDI,EBP  ;[_ScanLine]
		JNS		.pstvDyCas1
		NEG		EAX ; abs(deltay)
		JMP		SHORT .ngtvDyCas1
.pstvDyCas1:	NEG		EBP ; = -[_ScanLine] as ascendent y Axis
.ngtvDyCas1:
		INC		EAX
		MOV		EBX,[Plus2]  ; Line MAP
		SHL		EAX,Prec
		INC		ESI ; deltaX + 1
		CDQ
		IDIV		ESI
		MOV		ECX,ESI ; ECX = deltaX = number pixels
		NEG		EDI    ; // Y Axis  ascendent
		IMUL		EDI,[YP1]
		MOV		ESI,1 << Prec ; EBX = cpt Dbrd
		MOV		EDX,EAX ; EDX = pnt
		ADD		EDI,[XP1]
		MOV		EAX,[clr]
		ADD		EDI,[XP1] ; 2 time cause 16bpp
		ADD		EDI,[_vlfb]
ALIGN 4
.lp_line1:
		TEST		BL,1
		JZ		.PasDrl1
		MOV		[EDI],AX
.PasDrl1:
		SUB		ESI,EDX
		JA		.no_debor1 ; EDI >0
		ADD		ESI,(1 << Prec)
		ADD		EDI,EBP	 ; EDI + = directional ScanLine
.no_debor1:
		ROR		EBX,1
		DEC		ECX
		LEA		EDI,[EDI+2]	; EDI + 2
		JNZ		.lp_line1

		JMP		.FinLine
;*********CAS 2:  (DY > DX)*************************************************
.cas2:
		OR		EDI,EDI
		MOV		EAX,[YP1]
		JZ		.cas5
		CMP		EAX,[YP2]
		JL		.PasSwap2
		XCHG		EAX,[YP2]
		MOV		[YP1],EAX
		MOV		ECX,[XP2]
		MOV		EAX,[XP1]
		MOV		[XP1],ECX
		MOV		[XP2],EAX
.PasSwap2:
		OR		ESI,ESI
		JNZ		.noClipVert
		MOV		EAX,[_MinY]
		MOV		EBX,[_MaxY]
		CMP		EAX,[YP1]
		JLE		.sava21
		MOV		[YP1],EAX
.sava21:	CMP		EBX,[YP2]
		JGE		.sava22
		MOV		[YP2],EBX
.sava22:
.noClipVert:
		MOV		EAX,[XP2]
		MOV		ESI,[YP2]
		SUB		EAX,[XP1]
		JNS		.pstvDxCas2
		DEC		EAX
		JMP		SHORT .ngtvDxCas2
.pstvDxCas2:	INC		EAX
.ngtvDxCas2:
		SUB		ESI,[YP1]
		SHL		EAX,Prec
		INC		ESI
		CDQ
		MOV		EBP,[_ScanLine]
		IDIV		ESI
		MOV		ECX,ESI ; ECX = deltaY = number pixels
		MOV		EDX,EAX ; pente in EDX

		; start adress
		MOV		EDI,[YP1]
		IMUL		EDI,EBP ; * ScanLine
		NEG		EDI
		ADD		EDI,[XP1]
		NEG		EBP ; = - ScanLine
		ADD		EDI,[XP1]
		XOR		EBX,EBX ; accum in EBX
		ADD		EDI,[_vlfb]
		OR		EDX,EDX
		JNS		.line2_pstvPnt
		MOV		EBX,((1<<Prec)-1)
.line2_pstvPnt:
		; draw color
		MOV		EBP,[clr]
		MOV		EAX,[Plus2] ; Line Map
ALIGN 4
.lp_line2:	MOV		ESI,EBX
		SAR		ESI,Prec
		ADD		EBX,EDX ; + pnt
		TEST		AL,1
		JZ		.PasDrPx2
		MOV		[EDI+ESI*2],BP
.PasDrPx2:
		ROR		EAX,1
		SUB		EDI,[_ScanLine]	  ;  Axe Y Montant -ResH
		DEC		ECX
		JNZ		.lp_line2

		JMP		.FinLine
;*******CAS 3 :  (DX=0)*****************************************************
;*******CAS 4 :  (DY=0)*****************************************************
.cas4:		MOV		ECX,ESI
		MOV		EAX,[XP1]
		CMP		EAX,[XP2]
		JL		.PasSwap4
		MOV		EAX,[XP1]
		MOV		EBX,[XP2]
		MOV		[XP1],EBX
		MOV		[XP2],EAX
.PasSwap4:	MOV		EAX,[_MinX]
		CMP		EAX,[XP1]
		JLE		.sava41
		MOV		[XP1],EAX
.sava41:	MOV		EAX,[_MaxX]
		CMP		EAX,[XP2]
		JGE		.sava42
		MOV		[XP2],EAX
.sava42:
		MOV		ESI,[XP2]
		SUB		ESI,[XP1]
		OR		ESI,ESI
		JZ		.cas5
		INC		ESI
		MOV		EDI,[YP1]
		MOV		ECX,[Plus2] ; Line Map
		IMUL		EDI,[_ScanLine]
		MOV		EAX,[clr]
		NEG		EDI
		ADD		EDI,[_vlfb]
		ADD		EDI,[XP1]
		ADD		EDI,[XP1]
.lp4:		TEST		CL,1
		JZ		.PasDrl4
		MOV		[EDI],AX
.PasDrl4:
		ROR		ECX,1
		DEC		ESI
		LEA		EDI,[EDI+2] ; + 2 : 16 bpp
		JNZ		.lp4

		JMP		SHORT .FinLine
;********CAS 5 : (DX=0, DY=0)***********************************************
.cas5:
		TEST		BYTE [Plus2],1
		JZ		.FinLine

		MOV		EAX,[_ScanLine]
		MOV		ECX,[XP1]
		IMUL		EAX,[YP1]
		NEG		EAX
		MOV		EDX,[clr]
		ADD		EAX,[_vlfb]
		MOV		[EAX+ECX*2],DX
.FinLine:
		MOVD		EBX,mm7
		MOVD		EDI,mm6
		MOVD		ESI,mm5

	MMX_RETURN


ALIGN 32
_lineblnd16:
	ARG   	LBX16P1, 4, LBY16P1, 4, LBX16P2, 4, LBY16P2, 4, lnBCol16, 4

		PUSH		EBX
		PUSH		EDI
		PUSH		ESI

		MOV		EAX,[EBP+LBX16P1]
		MOV		EBX,[EBP+LBX16P2]
		MOV		ESI,[EBP+LBY16P1]
		MOV		EDI,[EBP+LBY16P2]

		MOV		[XP1],EAX
		MOV		[XP2],EBX
		MOV		[YP1],ESI
		MOV		EAX,[EBP+lnBCol16]
		MOV		[YP2],EDI
		;OV		[clr],ESI
		JMP		SHORT _LineBlnd16.DoLine16

ALIGN 32
_LineBlnd16:
	ARG   	PBlnd16P1, 4, PBlnd16P2, 4, BlndCol16, 4

		PUSH		EBX
		PUSH		EDI
		PUSH		ESI

		MOV		EDX,[EBP+PBlnd16P1]
		MOV		ECX,[EBP+PBlnd16P2]

		MOV		EAX,[EDX]   ; X1
		MOV		EBX,[ECX]   ; X2
		MOV		ESI,[EDX+4] ; Y1
		MOV		EDI,[ECX+4] ; Y2
		MOV		[XP1],EAX   ; X1
		MOV		[XP2],EBX   ; X2
		MOV		[YP1],ESI 	; Y1
		MOV		EAX,[EBP+BlndCol16]
		MOV		[YP2],EDI 	; Y2
		;MOV		[clr],EAX
		; blend precomputing-------------
.DoLine16:
		;MOV       	EAX,[clr] ;
		MOV       	EBX,EAX ;
		MOV       	ECX,EAX ;
		MOV       	EDX,EAX ;
		AND			EBX,[QBlue16Mask] ; EBX = Bclr16
		SHR			EAX,24
		AND			ECX,[QGreen16Mask] ; ECX = Gclr16
		AND			AL,BlendMask ; remove any ineeded bits
		JZ			.FinLine ; nothing 0 is the source
		AND			EDX,[QRed16Mask] ; EDX = Rclr16
		XOR			AL,BlendMask ; 31-blendsrc
		MOV			BP,AX
		SHL			EBP,16
		OR			BP,AX
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
		MOVD		mm7,EBP
		PUNPCKLDQ	mm5,mm5
		PUNPCKLDQ	mm7,mm7
		;- end blend prepare-------------

		MOV		EAX,[XP1]   ; ligne en dehors de la fenetre ?
		MOV		EBX,[XP2]
		MOV		EDX,EAX
		CMP		EAX,EBX
		JL		.VMaxX
		XCHG		EAX,EBX
.VMaxX:		CMP		EAX,[_MaxX]
		JG		.FinLine
		CMP		EBX,[_MinX]
		JL		.FinLine
		SUB		EBX,EAX
		MOV		ESI,EBX	   ;calcul de abs(x2-x1)

		MOV		EAX,[YP1]
		MOV		EBX,EDI; [YP2]
		MOV		ECX,EAX
		CMP		EAX,EBX
		JL		.VMaxY
		XCHG		EAX,EBX
.VMaxY:		CMP		EAX,[_MaxY]
		JG		.FinLine
		CMP		EBX,[_MinY]
		JL		.FinLine       ; fin du test
		SUB		EBX,EAX
		MOV		EDI,EBX		   ;  abs(y2-y1)

		OR		EDI,EDI
		JZ		.cas4
.PasNegEDI:	OR		ESI,ESI
		JZ		.cas2

		INC		ESI			  ; abs(x2-x1)+1
		INC		EDI			  ; abs(y2-y1)+1
		MOV		EAX,EDX			; EDX = [XP1]
		MOV		EBX,[XP2]	 ; cas 1 et cas 2
		CMP		EAX,EBX
		JL		.ClipMaxX
		XCHG		EAX,EBX
.ClipMaxX:	CMP		EAX,[_MinX]
		JL		.Aj_1_2
		CMP		EBX,[_MaxX]
		JG		.Aj_1_2
		MOV		EAX,ECX			; ECX = [YP1]
		MOV		EBX,[YP2]
		CMP		EAX,EBX
		JL		.ClipMaxY
		XCHG		EAX,EBX
.ClipMaxY:	CMP		EAX,[_MinY]
		JL		.Aj_1_2
		CMP		EBX,[_MaxY]
		JG		.Aj_1_2
		JMP		.PasAj_1_2
.Aj_1_2:	MOV		EBX,EDX			; EDX = [XP1]
		MOV		ESI,[XP2]
		MOV		EDI,[YP2]
		CMP		EBX,ESI
		JL		.MaxAj1_2X
		XCHG		EBX,ESI
		XCHG		ECX,EDI
.MaxAj1_2X:
;*********Ajustement des X
		CMP       	EBX,[_MinX]
		JNL       	.PasAjX12
		MOV       	EAX,EDI
		SUB       	EAX,ECX
		SAL       	EAX,Prec
		SUB       	ESI,EBX
		CDQ
		IDIV      	ESI
		ADD       	ESI,EBX
		MOV       	EDX,EAX
		MOV       	EAX,[_MinX]
		SUB       	EAX,EBX
		IMUL      	EAX,EDX
		SAR       	EAX,Prec
		MOV       	EBX,[_MinX]
		ADD       	ECX,EAX
.PasAjX12:	CMP       	ESI,[_MaxX]
		JNG       	.PasAjM12
		MOV       	EAX,EDI
		SUB       	EAX,ECX
		SAL       	EAX,Prec
		SUB       	ESI,EBX
		OR        	ESI,ESI
		JZ        	.FinLine
		CDQ
		IDIV      	ESI
		ADD       	ESI,EBX
		MOV       	EDX,EAX
		MOV       	EAX,ESI
		SUB       	EAX,[_MaxX]
		IMUL      	EAX,EDX
		SAR       	EAX,Prec
		MOV       	ESI,[_MaxX]
		SUB       	EDI,EAX
.PasAjM12:	CMP       	ECX,EDI
		JL        	.MaxAj1_2Y
		XCHG      	EBX,ESI
		XCHG      	ECX,EDI
.MaxAj1_2Y:	CMP       	ECX,[_MaxY]
		JG        	.FinLine
		CMP       	EDI,[_MinY]
		JL        	.FinLine
;*********Ajustement des Y
		CMP       	ECX,[_MinY]
		JNL       	.PasAjY12
		MOV       	EAX,ESI
		SUB       	EAX,EBX
		MOV	        EBP,ESI
		SAL       	EAX,Prec
		MOV       	ESI,EDI
		SUB       	ESI,ECX
		CDQ
		IDIV      	ESI
		MOV	        ESI,EBP
		MOV       	EDX,EAX
		MOV       	EAX,[_MinY]
		SUB       	EAX,ECX
		IMUL      	EAX,EDX
		SAR       	EAX,Prec
		MOV       	ECX,[_MinY]
		ADD       	EBX,EAX
		CMP       	EBX,[_MaxX]
		JG        	.FinLine
		CMP       	EBX,[_MinX]
		JL        	.FinLine
.PasAjY12:      CMP       	EDI,[_MaxY]
		JNG       	.PasAjY12X
		MOV       	EAX,ESI
		SUB       	EAX,EBX
		MOV		EBP,ESI
		SAL       	EAX,Prec
		MOV		ESI,EDI
		SUB		ESI,ECX
		CDQ
		IDIV		ESI
		MOV		ESI,EBP
		MOV		EDX,EAX
		MOV		EAX,EDI
		SUB		EAX,[_MaxY]
		IMUL		EAX,EDX
		SAR		EAX,Prec
		MOV		EDI,[_MaxY]
		SUB		ESI,EAX
.PasAjY12X:
		MOV       	[XP1],EBX
		MOV       	[YP1],ECX
		MOV       	[XP2],ESI
		MOV       	[YP2],EDI
		SUB       	ESI,EBX
		SUB       	EDI,ECX
		OR        	ESI,ESI

		JZ		.cas2
		JNS		.PasNegESI2
		NEG		ESI
.PasNegESI2:
		OR		EDI,EDI
		JZ		.cas4
		JNS		.PasNegEDI2
		NEG		EDI
.PasNegEDI2:
.PasAj_1_2:     CMP       	ESI,EDI
                JB        	.cas2
;*********CAS 1:  (DX > DY)***************************************************
.cas1:
		MOV		EAX,[XP1]
		MOV		EBP,[_ScanLine] ; plus
		CMP		EAX,[XP2]
		JL		.PasSwap1
		XCHG		EAX,[XP2]
		MOV		[XP1],EAX
		MOV		EAX,[YP1]
		MOV		EBX,[YP2]
		MOV		[YP1],EBX
		MOV		[YP2],EAX
.PasSwap1:
		MOV		ESI,[XP2]
		MOV		EAX,[YP2]
		SUB		ESI,[XP1]
		SUB		EAX,[YP1]
		MOV		EDI,EBP  ;[_ScanLine]
		JNS		.pstvDyCas1
		NEG		EAX ; abs(deltay)
		JMP		SHORT .ngtvDyCas1
.pstvDyCas1:	NEG		EBP ; = -[_ScanLine] as ascendent y Axis
.ngtvDyCas1:
		INC		EAX
		MOV		EBX,1 << Prec ; EBX = cpt Dbrd
		SHL		EAX,Prec
		INC		ESI ; deltaX + 1
		CDQ
		IDIV		ESI
		MOV		ECX,ESI ; ECX = deltaX = number pixels
		MOV		EDX,EAX ; EDX = pnt
		IMUL		EDI,[YP1]
		NEG		EDI    ; // Y Axis  ascendent
		ADD		EDI,[XP1]
		;MOV		EAX,[clr]
		ADD		EDI,[XP1] ; 2 time cause 16bpp
		MOV		ESI,1 << Prec
		ADD		EDI,[_vlfb]
ALIGN 4
.lp_line1:
		SUB		EBX,EDX
		MOV		AX,[EDI]
		MOVD		mm0,EAX ; B
		MOVD		mm1,EAX ; G
		MOVD		mm2,EAX  ; R
		@SolidBlndQ
		MOVD		EAX,mm0
		MOV		[EDI],AX
		JA		.no_debor1 ; EDI >0
		ADD		EBX,ESI ; +  (1 << Prec)
		ADD		EDI,EBP	 ; EDI + = directional ScanLine
.no_debor1:
		DEC		ECX
		LEA		EDI,[EDI+2]	; EDI + 2
		JNZ		.lp_line1

		JMP		.FinLine
;*********CAS 2:  (DY > DX)***************************************************
.cas2:
		OR		EDI,EDI
		MOV		EAX,[YP1]
		JZ		.cas5
		CMP		EAX,[YP2]
		JL		.PasSwap2
		XCHG		EAX,[YP2]
		MOV		[YP1],EAX
		MOV		ECX,[XP2]
		MOV		EAX,[XP1]
		MOV		[XP1],ECX
		MOV		[XP2],EAX
.PasSwap2:
		OR		ESI,ESI
		JNZ		.noClipVert
		MOV		EAX,[_MinY]
		MOV		EBX,[_MaxY]
		CMP		EAX,[YP1]
		JLE		.sava21
		MOV		[YP1],EAX
.sava21:	CMP		EBX,[YP2]
		JGE		.sava22
		MOV		[YP2],EBX
.sava22:
.noClipVert:
		MOV		EAX,[XP2]
		MOV		ESI,[YP2]
		SUB		EAX,[XP1]
		JNS		.pstvDxCas2
		DEC		EAX
		JMP		SHORT .ngtvDxCas2
.pstvDxCas2:	INC		EAX
.ngtvDxCas2:
		SUB		ESI,[YP1]
		SHL		EAX,Prec
		MOV		EBP,[_ScanLine]
		INC		ESI
		MOV		EDI,[YP1]
		CDQ
		IMUL		EDI,EBP ; * ScanLine
		IDIV		ESI
		MOV		ECX,ESI ; ECX = deltaY = number pixels
		NEG		EDI
		MOV		EDX,EAX ; pente in EDX

		; start adress
		ADD		EDI,[XP1]
		NEG		EBP ; = - ScanLine
		ADD		EDI,[XP1] ; add xp1 2 times as 16 bpp
		XOR		EBX,EBX ; accum in EBX
		ADD		EDI,[_vlfb]
		OR		EDX,EDX
		;MOV		EAX,[clr] ; draw color
		JNS		.line2_pstvPnt
		MOV		EBX,((1<<Prec)-1)
.line2_pstvPnt:
ALIGN 4
.lp_line2:
		MOV		ESI,EBX
		SAR		ESI,Prec
		ADD		EBX,EDX ; + pnt
		MOV		AX,[EDI+ESI*2]
		MOVD		mm0,EAX ; B
		MOVD		mm1,EAX ; G
		MOVD		mm2,EAX	  ; R
		@SolidBlndQ
		MOVD		EAX,mm0
		MOV		[EDI+ESI*2],AX
		DEC		ECX
		LEA		EDI,[EDI+EBP]	  ;  Axe Y Montant -ResH
		JNZ		.lp_line2

		JMP		.FinLine
;*******CAS 3 :  (DX=0)*****************************************************
;*******CAS 4 :  (DY=0)*****************************************************
.cas4:		MOV		ECX,ESI
		MOV		EAX,[XP1]
		CMP		EAX,[XP2]
		JL		.PasSwap4
		MOV		EAX,[XP1]
		MOV		EBX,[XP2]
		MOV		[XP1],EBX
		MOV		[XP2],EAX
.PasSwap4:	MOV		EAX,[_MinX]
		CMP		EAX,[XP1]
		JLE		.sava41
		MOV		[XP1],EAX
.sava41:	MOV		EAX,[_MaxX]
		CMP		EAX,[XP2]
		JGE		.sava42
		MOV		[XP2],EAX
.sava42:
		MOV		ESI,[XP2]
		SUB		ESI,[XP1]
		OR		ESI,ESI
		JZ		.cas5
		INC		ESI
		MOV		EDI,[YP1]
		IMUL		EDI,[_ScanLine]
		;MOV		EAX,[clr]
		NEG		EDI
		ADD		EDI,[_vlfb]
		ADD		EDI,[XP1]
		;MOV		EBX,EAX ; save firt 2 bytes color 16bpp
		ADD		EDI,[XP1]
		;SHL		EAX,16  ; shift high
		;OR		AX,BX ; assign the 16bpp color to the low
		XOR			ECX,ECX

		@SolidBlndHLine16
		;@SolidHLine16
		JMP		SHORT .FinLine
;********CAS 5 : (DX=0, DY=0)***********************************************
.cas5:
		MOV		EDI,[_ScanLine]
		IMUL		EDI,[YP1]
		MOV		EDX,[XP1]
		NEG		EDI
		ADD		EDI,[_vlfb]
		MOV		AX,[EDI+EDX*2]
		MOVD		mm0,EAX ; B
		MOVD		mm1,EAX ; G
		MOVD		mm2,EAX	  ; R
		@SolidBlndQ
		MOVD		EAX,mm0
		MOV		[EDI+EDX*2],AX

.FinLine:
		POP		ESI
		POP		EDI
		POP		EBX

    MMX_RETURN


ALIGN 32
_linemapblnd16:
	ARG   	LMBX16P1, 4, LMBY16P1, 4, LMBX16P2, 4, LMBY16P2, 4, lnMBCol16, 4, LMB16Map, 4

		PUSH		EBX
		PUSH		EDI
		PUSH		ESI

		MOV		EAX,[EBP+LMBX16P1]
		MOV		EBX,[EBP+LMBX16P2]
		MOV		ESI,[EBP+LMBY16P1]
		MOV		EDI,[EBP+LMBY16P2]
		MOV		ECX,[EBP+LMB16Map]

		MOV		[XP1],EAX
		MOV		[XP2],EBX
		MOV		[YP1],ESI
		MOV		EAX,[EBP+lnMBCol16]
		MOV		[YP2],EDI
		;MOV		[clr],ESI
		MOV		[Plus2],ECX
		JMP		SHORT _LineMapBlnd16.DoLine16

ALIGN 32
_LineMapBlnd16:
	ARG   	MapB16PtrP1, 4, MapB16PtrP2, 4, MapB16Col, 4, LineB16Map, 4

		PUSH		EBX
		PUSH		EDI
		PUSH		ESI

		MOV		EDX,[EBP+MapB16PtrP1]
		MOV		ECX,[EBP+MapB16PtrP2]

		MOV		EAX,[EDX]   ; X1
		MOV		EBX,[ECX]   ; X2
		MOV		ESI,[EDX+4] ; Y1
		MOV		EDI,[ECX+4] ; Y2
		MOV		[XP1],EAX   ; X1
		MOV		[XP2],EBX   ; X2
		MOV		[YP1],ESI 	; Y1
		MOV		EAX,[EBP+MapB16Col]
		MOV		[YP2],EDI 	; Y2
		MOV		EBX,[EBP+LineB16Map]
		;MOV		[clr],EAX
		MOV		[Plus2],EBX
		; blend precomputing-------------
		;MOV       	EAX,[clr] ;
.DoLine16:	MOV       	EBX,EAX ;
		MOV       	ECX,EAX ;
		MOV       	EDX,EAX ;
		AND		EBX,[QBlue16Mask] ; EBX = Bclr16 | Bclr16
		SHR		EAX,24
		AND		ECX,[QGreen16Mask] ; ECX = Gclr16 | Gclr16
		AND		AL,BlendMask ; remove any ineeded bits
		JZ		.FinLine ; nothing 0 is the source
		AND		EDX,[QRed16Mask] ; EDX = Rclr16 | Rclr16
		XOR		AL,BlendMask ; 31-blendsrc
		MOV		BP,AX
		SHL		EBP,16
		OR		BP,AX
		XOR		AL,BlendMask ; 31-blendsrc
		INC		AL
		SHR		DX,5 ; right shift red 5bits
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
		MOVD		mm7,EBP
		PUNPCKLDQ	mm5,mm5
		PUNPCKLDQ	mm7,mm7
		;- end blend prepare-------------

		MOV		EAX,[XP1]   ; ligne en dehors de la fenetre ?
		MOV		EBX,[XP2]
		MOV		EDX,EAX
		CMP		EAX,EBX
		JL		.VMaxX
		XCHG		EAX,EBX
.VMaxX:		CMP		EAX,[_MaxX]
		JG		.FinLine
		CMP		EBX,[_MinX]
		JL		.FinLine
		SUB		EBX,EAX
		MOV		ESI,EBX	   ;calcul de abs(x2-x1)

		MOV		EAX,[YP1]
		MOV		EBX,EDI; [YP2]
		MOV		ECX,EAX
		CMP		EAX,EBX
		JL		.VMaxY
		XCHG		EAX,EBX
.VMaxY:		CMP		EAX,[_MaxY]
		JG		.FinLine
		CMP		EBX,[_MinY]
		JL		.FinLine       ; fin du test
		SUB		EBX,EAX
		MOV		EDI,EBX		   ;  abs(y2-y1)

		OR		EDI,EDI
		JZ		.cas4
.PasNegEDI:	OR		ESI,ESI
		JZ		.cas2

		INC		ESI			  ; abs(x2-x1)+1
		INC		EDI			  ; abs(y2-y1)+1
		MOV		EAX,EDX			; EDX = [XP1]
		MOV		EBX,[XP2]	 ; cas 1 et cas 2
		CMP		EAX,EBX
		JL		.ClipMaxX
		XCHG		EAX,EBX
.ClipMaxX:	CMP		EAX,[_MinX]
		JL		.Aj_1_2
		CMP		EBX,[_MaxX]
		JG		.Aj_1_2
		MOV		EAX,ECX			; ECX = [YP1]
		MOV		EBX,[YP2]
		CMP		EAX,EBX
		JL		.ClipMaxY
		XCHG		EAX,EBX
.ClipMaxY:	CMP		EAX,[_MinY]
		JL		.Aj_1_2
		CMP		EBX,[_MaxY]
		JG		.Aj_1_2
		JMP		.PasAj_1_2
.Aj_1_2:	MOV		EBX,EDX			; EDX = [XP1]
		MOV		ESI,[XP2]
		MOV		EDI,[YP2]
		CMP		EBX,ESI
		JL		.MaxAj1_2X
		XCHG		EBX,ESI
		XCHG		ECX,EDI
.MaxAj1_2X:
;*********Ajustement des X
		CMP       	EBX,[_MinX]
		JNL       	.PasAjX12
		MOV       	EAX,EDI
		SUB       	EAX,ECX
		SAL       	EAX,Prec
		SUB       	ESI,EBX
		CDQ
		IDIV      	ESI
		ADD       	ESI,EBX
		MOV       	EDX,EAX
		MOV       	EAX,[_MinX]
		SUB       	EAX,EBX
		IMUL      	EAX,EDX
		SAR       	EAX,Prec
		MOV       	EBX,[_MinX]
		ADD       	ECX,EAX
.PasAjX12:	CMP       	ESI,[_MaxX]
		JNG       	.PasAjM12
		MOV       	EAX,EDI
		SUB       	EAX,ECX
		SAL       	EAX,Prec
		SUB       	ESI,EBX
		OR        	ESI,ESI
		JZ        	.FinLine
		CDQ
		IDIV      	ESI
		ADD       	ESI,EBX
		MOV       	EDX,EAX
		MOV       	EAX,ESI
		SUB       	EAX,[_MaxX]
		IMUL      	EAX,EDX
		SAR       	EAX,Prec
		MOV       	ESI,[_MaxX]
		SUB       	EDI,EAX
.PasAjM12:	CMP       	ECX,EDI
		JL        	.MaxAj1_2Y
		XCHG      	EBX,ESI
		XCHG      	ECX,EDI
.MaxAj1_2Y:	CMP       	ECX,[_MaxY]
		JG        	.FinLine
		CMP       	EDI,[_MinY]
		JL        	.FinLine
;*********Ajustement des Y
		CMP       	ECX,[_MinY]
		JNL       	.PasAjY12
		MOV       	EAX,ESI
		SUB       	EAX,EBX
		MOV	        EBP,ESI ; sauve ESI
		SAL       	EAX,Prec
		MOV       	ESI,EDI
		SUB       	ESI,ECX
		CDQ
		IDIV      	ESI
		MOV	        ESI,EBP ; rest ESI
		MOV       	EDX,EAX
		MOV       	EAX,[_MinY]
		SUB       	EAX,ECX
		IMUL      	EAX,EDX
		SAR       	EAX,Prec
		MOV       	ECX,[_MinY]
		ADD       	EBX,EAX
		CMP       	EBX,[_MaxX]
		JG        	.FinLine
		CMP       	EBX,[_MinX]
		JL        	.FinLine
.PasAjY12:      CMP       	EDI,[_MaxY]
		JNG       	.PasAjY12X
		MOV       	EAX,ESI
		SUB       	EAX,EBX
		MOV	        EBP,ESI ; sauve ESI
		SAL       	EAX,Prec
		MOV       	ESI,EDI
		SUB       	ESI,ECX
		CDQ
		IDIV      	ESI
		MOV	        ESI,EBP ; rest ESI
		MOV       	EDX,EAX
		MOV       	EAX,EDI
		SUB       	EAX,[_MaxY]
		IMUL      	EAX,EDX
		SAR       	EAX,Prec
		MOV       	EDI,[_MaxY]
		SUB       	ESI,EAX
.PasAjY12X:
		MOV		[XP1],EBX
		MOV		[YP1],ECX
		MOV       	[XP2],ESI
		MOV       	[YP2],EDI
		SUB       	ESI,EBX
		SUB       	EDI,ECX
		OR        	ESI,ESI

		JZ		.cas2
		JNS		.PasNegESI2
		NEG		ESI
.PasNegESI2:
		OR		EDI,EDI
		JZ		.cas4
		JNS		.PasNegEDI2
		NEG		EDI
.PasNegEDI2:
.PasAj_1_2:     CMP       	ESI,EDI
                JB        	.cas2

;*********CAS 1:  (DX > DY)***************************************************
.cas1:
		MOV		EAX,[XP1]
		MOV		EBP,[_ScanLine] ; plus
		CMP		EAX,[XP2]
		JL		.PasSwap1
		XCHG		EAX,[XP2]
		MOV		[XP1],EAX
		MOV		EAX,[YP1]
		MOV		EBX,[YP2]
		MOV		[YP1],EBX
		MOV		[YP2],EAX
.PasSwap1:
		MOV		ESI,[XP2]
		MOV		EAX,[YP2]
		SUB		ESI,[XP1]
		SUB		EAX,[YP1]
		MOV		EDI,EBP  ;[_ScanLine]
		JNS		.pstvDyCas1
		NEG		EAX ; abs(deltay)
		JMP		SHORT .ngtvDyCas1
.pstvDyCas1:	NEG		EBP ; = -[_ScanLine] as ascendent y Axis
.ngtvDyCas1:
		INC		EAX
		MOV		EBX,[Plus2]  ; Line MAP
		SHL		EAX,Prec
		INC		ESI ; deltaX + 1
		CDQ
		IDIV		ESI
		MOV		ECX,ESI ; ECX = deltaX = number pixels
		NEG		EDI    ; // Y Axis  ascendent
		IMUL		EDI,[YP1]
		MOV		ESI,1 << Prec ; EBX = cpt Dbrd
		MOV		EDX,EAX ; EDX = pnt
		ADD		EDI,[XP1]
		;MOV		EAX,[clr]
		ADD		EDI,[XP1] ; 2 time cause 16bpp
		ADD		EDI,[_vlfb]
ALIGN 4
.lp_line1:
		TEST		BL,1
		JZ		.PasDrl1
		MOV		AX,[EDI]
		MOVD		mm0,EAX ; B
		MOVD		mm1,EAX ; G
		MOVD		mm2,EAX  ; R
		@SolidBlndQ
		MOVD		EAX,mm0
		MOV		[EDI],AX
.PasDrl1:
		SUB		ESI,EDX
		JA		.no_debor1 ; EDI >0
		ADD		ESI,(1 << Prec)
		ADD		EDI,EBP	 ; EDI + = directional ScanLine
.no_debor1:
		ROR		EBX,1
		DEC		ECX
		LEA		EDI,[EDI+2]	; EDI + 2
		JNZ		.lp_line1

		JMP		.FinLine
;*********CAS 2:  (DY > DX)*************************************************
.cas2:
		OR		EDI,EDI
		MOV		EAX,[YP1]
		JZ		.cas5
		CMP		EAX,[YP2]
		JL		.PasSwap2
		XCHG		EAX,[YP2]
		MOV		[YP1],EAX
		MOV		ECX,[XP2]
		MOV		EAX,[XP1]
		MOV		[XP1],ECX
		MOV		[XP2],EAX
.PasSwap2:
		OR		ESI,ESI
		JNZ		.noClipVert
		MOV		EAX,[_MinY]
		MOV		EBX,[_MaxY]
		CMP		EAX,[YP1]
		JLE		.sava21
		MOV		[YP1],EAX
.sava21:	CMP		EBX,[YP2]
		JGE		.sava22
		MOV		[YP2],EBX
.sava22:
.noClipVert:
		MOV		EAX,[XP2]
		MOV		ESI,[YP2]
		SUB		EAX,[XP1]
		JNS		.pstvDxCas2
		DEC		EAX
		JMP		SHORT .ngtvDxCas2
.pstvDxCas2:	INC		EAX
.ngtvDxCas2:
		SUB		ESI,[YP1]
		SHL		EAX,Prec
		INC		ESI
		CDQ
		MOV		EBP,[_ScanLine]
		IDIV		ESI
		MOV		ECX,ESI ; ECX = deltaY = number pixels
		MOV		EDX,EAX ; pente in EDX

		; start adress
		MOV		EDI,[YP1]
		IMUL		EDI,EBP ; * ScanLine
		NEG		EDI
		ADD		EDI,[XP1]
		NEG		EBP ; = - ScanLine
		ADD		EDI,[XP1]
		XOR		EBP,EBP ; accum in EBX
		ADD		EDI,[_vlfb]
		OR		EDX,EDX
		JNS		.line2_pstvPnt
		MOV		EBP,((1<<Prec)-1)
.line2_pstvPnt:
		; draw color
		;MOV		EBP,[clr]
		MOV		EBX,[Plus2] ; Line Map
ALIGN 4
.lp_line2:	MOV		ESI,EBP
		SAR		ESI,Prec
		ADD		EBP,EDX ; + pnt
		TEST		BL,1
		JZ		.PasDrPx2
		MOV		AX,[EDI+ESI*2]
		MOVD		mm0,EAX ; B
		MOVD		mm1,EAX ; G
		MOVD		mm2,EAX  ; R
		@SolidBlndQ
		MOVD		EAX,mm0
		MOV		[EDI+ESI*2],AX
.PasDrPx2:
		ROR		EBX,1
		SUB		EDI,[_ScanLine]	  ;  Axe Y Montant -ResH
		DEC		ECX
		JNZ		.lp_line2

		JMP		.FinLine
;*******CAS 3 :  (DX=0)*****************************************************
;*******CAS 4 :  (DY=0)*****************************************************
.cas4:		MOV		ECX,ESI
		MOV		EAX,[XP1]
		CMP		EAX,[XP2]
		JL		.PasSwap4
		MOV		EAX,[XP1]
		MOV		EBX,[XP2]
		MOV		[XP1],EBX
		MOV		[XP2],EAX
.PasSwap4:	MOV		EAX,[_MinX]
		CMP		EAX,[XP1]
		JLE		.sava41
		MOV		[XP1],EAX
.sava41:	MOV		EAX,[_MaxX]
		CMP		EAX,[XP2]
		JGE		.sava42
		MOV		[XP2],EAX
.sava42:
		MOV		ESI,[XP2]
		SUB		ESI,[XP1]
		OR		ESI,ESI
		JZ		.cas5
		INC		ESI
		MOV		EDI,[YP1]
		MOV		ECX,[Plus2] ; Line Map
		IMUL		EDI,[_ScanLine]
		MOV		EAX,[clr]
		NEG		EDI
		ADD		EDI,[_vlfb]
		ADD		EDI,[XP1]
		ADD		EDI,[XP1]
.lp4:		TEST		CL,1
		JZ		.PasDrl4
		MOV		AX,[EDI]
		MOVD		mm0,EAX ; B
		MOVD		mm1,EAX ; G
		MOVD		mm2,EAX  ; R
		@SolidBlndQ
		MOVD		EAX,mm0
		MOV		[EDI],AX
.PasDrl4:
		ROR		ECX,1
		DEC		ESI
		LEA		EDI,[EDI+2] ; + 2 : 16 bpp
		JNZ		.lp4

		JMP		.FinLine
;********CAS 5 : (DX=0, DY=0)***********************************************
.cas5:
		TEST		BYTE [Plus2],1
		JZ		.FinLine

		MOV		EDI,[_ScanLine]
		MOV		ECX,[XP1]
		IMUL		EDI,[YP1]
		NEG		EDI
		MOV		EDX,[clr]
		ADD		EDI,[_vlfb]
		MOV		AX,[EDI+ECX*2]
		MOVD		mm0,EAX ; B
		MOVD		mm1,EAX ; G
		MOVD		mm2,EAX  ; R
		@SolidBlndQ
		MOVD		EAX,mm0
		MOV		[EDI+ECX*2],AX
.FinLine:
		POP		ESI
		POP		EDI
		POP		EBX

    MMX_RETURN

