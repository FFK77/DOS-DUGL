 %include "PARAM.MAC"

; Multi thread safe functions

; GLOBAL Function****
GLOBAL	_SurfCopy,_DgMemCpy
GLOBAL	_ProtectSetPalette,_ProtectViewSurf
GLOBAL	_ProtectViewSurfWaitVR,_WaitRetrace,_GetMaxResVSetSurf
GLOBAL  _SensPoly,_ValidSPoly

; GLOBAL DATA
GLOBAL  _DgNanoSurf, _dummyDBMFONT
GLOBAL  QBlue16Mask, QGreen16Mask, QRed16Mask, WBGR16Mask
GLOBAL  PntInitCPTDbrd, DGDQ0_1_2_3, DGDQInitCPTDbrd, NegDecPosInc, MaxPolyDeltaDim
GLOBAL  MaskB_RGB16, MaskG_RGB16, MaskR_RGB16, RGB16_PntNeg, Mask2B_RGB16, Mask2G_RGB16, Mask2R_RGB16
GLOBAL  RGBDebMask_GGG, RGBDebMask_IGG, RGBDebMask_GIG, RGBDebMask_IIG, RGBDebMask_GGI, RGBDebMask_IGI, RGBDebMask_GII, RGBDebMask_III
GLOBAL  RGBFinMask_GGG, RGBFinMask_IGG, RGBFinMask_GIG, RGBFinMask_IIG, RGBFinMask_GGI, RGBFinMask_IGI, RGBFinMask_GII, RGBFinMask_III


; EXTERN DATA
EXTERN	_SetPalPMI, _CurPalette, _ShiftPal, _ViewAddressPMI, _VSurf, _NbVDgSurf
EXTERN	_EnableMPIO,_SelMPIO, _CurViewVSurf

SECTION .text
[BITS 32]
ALIGN 32


ALIGN 32
_DgMemCpy:
    ARG	PMemCDst, 4, PMemCSrc, 4, PMemCSize, 4
		PUSH	    ESI
		PUSH		EDI

		MOV		    ESI,[EBP+PMemCSrc]
		MOV		    EDI,[EBP+PMemCDst]
		MOV		    EBP,[EBP+PMemCSize]
        JMP         _SurfCopy.DoMemCpy

ALIGN 32
_SurfCopy:
	ARG	PDstSrf, 4, PSrcSrf, 4
		PUSH	    ESI
		PUSH		EDI

		MOV		    ESI,[EBP+PSrcSrf]
		MOV		    EDI,[EBP+PDstSrf]
		MOV		    EBP,[ESI+DuglSurf.SizeSurf]

		MOV		    EDI,[EDI+DuglSurf.rlfb]
		MOV		    ESI,[ESI+DuglSurf.rlfb]
.DoMemCpy:
		CMP         EBP,8
        LEA         ECX,[ECX-ECX]
		JL          .CopyDAp

		TEST		EDI,0x7
		JZ		    .CpyMMX
.CopyBAv:
        TEST		EDI,0x1
		JZ		    .PasCopyBAv
		OR		    EBP,EBP
		JZ		    .FinSurfCopy
		DEC		    EBP
		MOVSB
.PasCopyBAv:
.CopyWAv:
        TEST		EDI,0x2
		JZ		    .PasCopyWAv
		CMP		    EBP,BYTE 2
		JL		    .CopyBAp
		SUB		    EBP,BYTE 2
		MOVSW
.PasCopyWAv:
.CopyDAv:
        TEST		EDI,0x4
		JZ		    .PasCopyDAv
		CMP		    EBP,BYTE 4
		JL		    .CopyWAp
		SUB		    EBP,BYTE 4
		MOVSD
.PasCopyDAv:
.CpyMMX:
        SHLD        ECX,EBP,26 ; ECX = EBP >> 6  ECX should be equal to zero

		JZ		    .PasCpyMMXBloc
		AND		    EBP,BYTE 0x3F
.BcCpyMMXBloc:
		MOVQ		mm0,[ESI]
		MOVQ		mm1,[ESI+32]
		MOVQ		mm2,[ESI+8]
		MOVQ		mm3,[ESI+40]
		MOVQ		mm4,[ESI+16]
		MOVQ		mm5,[ESI+48]
		MOVQ		mm6,[ESI+24]
		MOVQ		mm7,[ESI+56]
		MOVQ		[EDI],mm0
		MOVQ		[EDI+32],mm1
		MOVQ		[EDI+8],mm2
		MOVQ		[EDI+40],mm3
		MOVQ		[EDI+16],mm4
		MOVQ		[EDI+48],mm5
		MOVQ		[EDI+24],mm6
		MOVQ		[EDI+56],mm7
		DEC		    ECX
		LEA		    ESI,[ESI+64]
		LEA		    EDI,[EDI+64]
		JNZ		    .BcCpyMMXBloc
.PasCpyMMXBloc:
        SHLD        ECX,EBP,29 ; ECX = EBP >> 3  ECX should be equal to zero
		JZ		    .PasCpyMMX
		AND		    EBP,BYTE 7
.BcCpyMMX:
		MOVQ		mm0,[ESI]
		MOVQ		[EDI],mm0
		DEC		    ECX
		LEA		    ESI,[ESI+8]
		LEA		    EDI,[EDI+8]
		JNZ		    .BcCpyMMX

.PasCpyMMX:
.CopyDAp:
        CMP		    EBP,BYTE 4
		JL		    .CopyWAp
		SUB		    EBP,BYTE 4
		MOVSD
.PasCopyDAp:
.CopyWAp:
        CMP		    EBP,BYTE 2
		JL		    .CopyBAp
		SUB		    EBP,BYTE 2
		MOVSW
.PasCopyWAp:
.CopyBAp:
        OR		    EBP,EBP
		JZ		    .FinSurfCopy
		MOVSB
.PasCopyBAp:
.FinSurfCopy:

		POP		    EDI
		POP		    ESI
    MMX_RETURN

_GetMaxResVSetSurf:
		MOV		EAX,MaxResV
		FRETURN


_ProtectSetPalette:
	ARG	Dbcol, 4, Nbcol, 4, Tcol, 4

		PUSH		EDI
		PUSH		ESI
		PUSH		EBX
		PUSH		ES

		MOV		EDX,[EBP+Dbcol]
		AND		EDX,BYTE -1
		MOV		ECX,[EBP+Nbcol]
		OR		ECX,ECX
		JS		.FinProtectSP
		JZ		.FinProtectSP
		CMP		ECX,256
		JG		.FinProtectSP

		MOV		AL,[_ShiftPal]
		OR		AL,AL
		JZ		.CopyPal

		MOV		ESI,[EBP+Tcol]
		LEA		EDI,[_CurPalette+EDX*4]
.BcShiftPal:	LODSD
		MOV		EBX,EAX
		SHR		AL,2
		SHR		EBX,16
		SHR		AH,2
		SHR		BL,2
		BSWAP		EAX
		MOV		AH,BL
		DEC		ECX
		BSWAP		EAX
		STOSD
		JNZ		.BcShiftPal
		JMP		SHORT .PasCopyPal
.CopyPal:
		MOV		ESI,[EBP+Tcol]
		LEA		EDI,[_CurPalette+EDX*4]
		REP		MOVSD
.PasCopyPal:
		MOV		EDX,[EBP+Dbcol]
		MOV		ECX,[EBP+Nbcol]
		AND		EDX,BYTE -1
		CMP		BYTE [_EnableMPIO],0
		JE		SHORT .NoMPIOSel
		MOV		AX,[_SelMPIO]
		MOV		ES,AX
.NoMPIOSel:
		MOV		EAX,[_SetPalPMI]
		LEA		EDI,[_CurPalette+EDX*4]
		XOR		EBX,EBX
		CALL		EAX
.FinProtectSP:
		POP		ES
		POP		EBX
		POP		ESI
		POP		EDI

	RETURN

_ProtectViewSurf:
	ARG	NbSurf, 4
		MOVD		mm0,EBX
		MOVD		mm2,ESI
		PUSH		ES

		MOV			ECX,[EBP+NbSurf]
		CMP			ECX,[_NbVDgSurf]
		JNB			.FInNbSurf
		MOV			[_CurViewVSurf],ECX
		IMUL    	ECX,BYTE SurfUtilSize ;SHL ECX,6
		ADD			ECX,DuglSurf.OffVMem ; OffVMem
		ADD			ECX,[_VSurf]
		MOV			EAX,[ECX]   ; EAX = VSurf[NbSurf].OffVMem
		ROR			EAX,2
		XOR			ECX,ECX
		MOV			CX,AX ; CX = OffVMem[2..17]
		SHR			EAX,16
		XOR			EDX,EDX
		MOV			DX,AX ; DX = OffVMem[18..31][0..1]
		XOR			EBX,EBX ; EBX 0
		CMP			BYTE [_EnableMPIO],0
		JE			SHORT .NoMPIOSel
		MOV			AX,[_SelMPIO]
		MOV			ES,AX
.NoMPIOSel:
		MOV			EAX,[_ViewAddressPMI]
		CALL		EAX

.FInNbSurf:
		POP			ES
		MOVD		EBX,mm0
		MOVD		ESI,mm2
		;EMMS
		MMX_RETURN

_ProtectViewSurfWaitVR:
	ARG	NbVRSurf, 4
		MOVD		mm0,EBX
		MOVD		mm2,ESI
		PUSH		ES

		MOV			ECX,[EBP+NbVRSurf]
		CMP			ECX,[_NbVDgSurf]
		JNB			.FInNbSurf
		MOV			[_CurViewVSurf],ECX
		IMUL		ECX,BYTE SurfUtilSize ;SHL		ECX,6
		ADD			ECX,DuglSurf.OffVMem ; OffVMem
		ADD			ECX,[_VSurf]
		MOV			EAX,[ECX]   ; EAX = VSurf[NbSurf].OffVMem
		ROR			EAX,2
		XOR			ECX,ECX
		MOV			CX,AX ; CX = OffVMem[2..17]
		SHR			EAX,16
		XOR			EDX,EDX
		MOV			DX,AX ; DX = OffVMem[18..31][0..1]
		XOR             EBX,EBX
		MOV			BL,0x80
		CMP			BYTE [_EnableMPIO],0
		JE			SHORT .NoMPIOSel
		MOV			AX,[_SelMPIO]
		MOV			ES,AX
.NoMPIOSel:
		MOV			EAX,[_ViewAddressPMI]
		CALL		EAX

.FInNbSurf:	POP		ES
		MOVD		EBX,mm0
		MOVD		ESI,mm2

    MMX_RETURN



_WaitRetrace:
		MOV		EDX,0x3da
.wait1:		IN		AL,DX
		TEST		AL,0x08
		JNZ		.wait1
.wait2:		IN		AL,DX
		TEST		AL,0x08
		JZ		.wait2
		FRETURN




_ValidSPoly:
	ARG	VPtrListPt, 4
		MOVD		mm7,ESI
		MOVD		mm6,EBX

		MOV			ESI,[EBP+VPtrListPt]

		MOV			ECX,[ESI+8]
		MOV			EAX,[ESI]
		MOV			EBX,[ESI+4]
		MOVQ		mm0,[EAX] ; = XP1, YP1
		MOVQ		mm1,[EBX] ; = XP2, YP2
		MOVQ		mm2,[ECX] ; = XP3, YP3
		MOVQ		mm3,mm0 ; = XP1, YP1
		MOVQ		mm4,mm1 ; = XP2, YP2

;(XP2-XP1)*(YP3-YP2)-(XP3-XP2)*(YP2-YP1)
; s'assure que les points suive le sens inverse de l'aiguille d'une montre
.verifSens:
		PSUBD		mm1,mm0 ; = (XP2-XP1) | (YP2 - YP1)
		PSUBD		mm2,mm4 ; = (XP3-XP2) | (YP3 - YP2)
		MOVD		ECX,mm1 ; = (XP2-XP1)
		MOVD		EDX,mm2 ; = (XP3-XP2)
		PSRLQ		mm1,32
		PSRLQ		mm2,32
		MOVD		ESI,mm1 ; = (YP2-YP1)
		MOVD		EBX,mm2 ; = (YP3-YP2)
		IMUL		ESI,EDX
		IMUL		ECX,EBX
		XOR			EAX,EAX
		CMP			ECX,ESI
		SETG		AL

		MOVD		EBX,mm6
		MOVD		ESI,mm7

	MMX_RETURN

_SensPoly:
	ARG	VSPtrListPt, 4

		MOVD		mm7,ESI
		MOVD		mm6,EBX

		MOV			ESI,[EBP+VSPtrListPt]

		MOV			ECX,[ESI+8]
		MOV			EAX,[ESI]
		MOV			EBX,[ESI+4]
		MOVQ		mm0,[EAX] ; = XP1, YP1
		MOVQ		mm1,[EBX] ; = XP2, YP2
		MOVQ		mm2,[ECX] ; = XP3, YP3
		MOVQ		mm3,mm0 ; = XP1, YP1
		MOVQ		mm4,mm1 ; = XP2, YP2

;(XP2-XP1)*(YP3-YP2)-(XP3-XP2)*(YP2-YP1)
; s'assure que les points suive le sens inverse de l'aiguille d'une montre
.verifSens:
		PSUBD		mm1,mm0 ; = (XP2-XP1) | (YP2 - YP1)
		PSUBD		mm2,mm4 ; = (XP3-XP2) | (YP3 - YP2)
		MOVD		EAX,mm1 ; = (XP2-XP1)
		MOVD		EDX,mm2 ; = (XP3-XP2)
		PSRLQ		mm1,32
		PSRLQ		mm2,32
		MOVD		ESI,mm1 ; = (YP2-YP1)
		MOVD		EBX,mm2 ; = (YP3-YP2)
		IMUL		ESI,EDX
		IMUL		EAX,EBX
		SUB			EAX,ESI

		MOVD		EBX,mm6
		MOVD		ESI,mm7

	MMX_RETURN

SECTION .data   ALIGN=32

_DgNanoSurf:
NScanLine           DD      2
Nrlfb               DD      NOffVMem
NOrgX               DD      0
NOrgY               DD      0
NMaxX               DD      0
NMaxY               DD      0
NMinX               DD      0
NMinY               DD      0;-----------------------
NMask               DD      0
NResH               DD      1
NResV               DD      1
Nvlfb               DD      NOffVMem
NNegScanLine        DD      -2
NOffVMem            DD      0
NBitsPixel          DD      16
NSizeSurf           DD      2;-----------------------
_dummyDBMFONT:
dBMCharsSSurfs      DD    256 dup (_DgNanoSurf)
dBMCharsPlusX       DD    256 dup (0)
dBMCharsWidth       DD    256 dup (1)
dBMCharsHeight      DD    256 dup (1)
dBMCharsXOffset     DD    256 dup (0)
dBMCharsYOffset     DD    256 dup (0)
dBMCharsGHeight     DD    1
dBMCharsGLineHeight DD    1
dBMCharX            DD    0
dBMCharY            DD    0
dBMCharCurChar      DD    0
dBMCharsMainSurf    DD    0
dBMCharsRendX       DD    0
dBMCharsRendY       DD    0;--------------


PntInitCPTDbrd		DD	0,((1<<Prec)-1)
Temp2				DD	0
MaskB_RGB16			DD	0x1f	 ; blue bits 0->4
MaskG_RGB16			DD	0x3f<<5  ; green bits 5->10
MaskR_RGB16			DD	0x1f<<11 ; red bits 11->15
RGB16_PntNeg		DD	((1<<Prec)-1) ;----------
Mask2B_RGB16		DD	0x1f,0x1f ; blue bits 0->4
Mask2G_RGB16		DD	0x3f<<5,0x3f<<5  ; green bits 5->10 ;----------
Mask2R_RGB16		DD	0x1f<<11,0x1f<<11 ; red bits 11->15
RGBDebMask_GGG		DD	0,0,0,0
RGBDebMask_IGG		DD	((1<<Prec)-1),0,0,0
RGBDebMask_GIG		DD	0,((1<<(Prec+5))-1),0,0
RGBDebMask_IIG		DD	((1<<Prec)-1),((1<<(Prec+5))-1),0,0
RGBDebMask_GGI		DD	0,0,((1<<(Prec+11))-1),0
RGBDebMask_IGI		DD	((1<<Prec)-1),0,((1<<(Prec+11))-1),0
RGBDebMask_GII		DD	0,((1<<(Prec+5))-1),((1<<(Prec+11))-1),0
RGBDebMask_III		DD	((1<<Prec)-1),((1<<(Prec+5))-1),((1<<(Prec+11))-1),0

RGBFinMask_GGG		DD	((1<<Prec)-1),((1<<(Prec+5))-1),((1<<(Prec+11))-1),0
RGBFinMask_IGG		DD	0,((1<<(Prec+5))-1),((1<<(Prec+11))-1),0
RGBFinMask_GIG		DD	((1<<Prec)-1),0,((1<<(Prec+11))-1),0
RGBFinMask_IIG		DD	0,0,((1<<(Prec+11))-1),0
RGBFinMask_GGI		DD	((1<<Prec)-1),((1<<(Prec+5))-1),0,0
RGBFinMask_IGI		DD	0,((1<<(Prec+5))-1),0,0
RGBFinMask_GII		DD	((1<<Prec)-1),0,0,0
RGBFinMask_III		DD	0,0,0,0
; BLENDING 16BPP ----------
QBlue16Mask			DW	CMaskB_RGB16,CMaskB_RGB16,CMaskB_RGB16,CMaskB_RGB16
QGreen16Mask		DW	CMaskG_RGB16,CMaskG_RGB16,CMaskG_RGB16,CMaskG_RGB16
QRed16Mask			DW	CMaskR_RGB16,CMaskR_RGB16,CMaskR_RGB16,CMaskR_RGB16
