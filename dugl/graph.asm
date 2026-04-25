%include "param.mac"

; GLOBAL Function****
; 8bpp
GLOBAL  _PutPixel,_GetPixel,_line,_Line,_linemap,_LineMap,_DgSetCurSurf
GLOBAL  _DgSetSrcSurf,_DgGetCurSurf,_Clear
;********************
GLOBAL  _Poly, _RePoly,_PutSurf,_PutMaskSurf

; 16bpp
GLOBAL  _PutPixel16,_GetPixel16, _ClearSurf16, _InBar16, _Bar16, _InBarBlnd16, _BarBlnd16
GLOBAL  _PutSurf16,_PutMaskSurf16,_PutSurfBlnd16,_PutMaskSurfBlnd16
GLOBAL  _PutSurfTrans16,_PutMaskSurfTrans16
GLOBAL  _SurfCopyBlnd16,_SurfMaskCopyBlnd16,_SurfCopyTrans16,_SurfMaskCopyTrans16
GLOBAL  _line16,_Line16,_linemap16,_LineMap16,_lineblnd16,_LineBlnd16
GLOBAL  _linemapblnd16,_LineMapBlnd16,_Poly16, _RePoly16, _Clear16
GLOBAL  _ResizeViewSurf16,_MaskResizeViewSurf16,_BlndResizeViewSurf16,_MaskBlndResizeViewSurf16
GLOBAL  _TransResizeViewSurf16,_MaskTransResizeViewSurf16
GLOBAL  _OutTextBM16, _SetCurBMFont

; GLOBAL DATA
GLOBAL  _CurViewVSurf, _CurSurf, _SrcSurf
GLOBAL  _PtrTbColConv, _TbDegCol, _LastPolyStatus
GLOBAL  _CurDBMFONT
GLOBAL  _vlfb,_ResH,_ResV,_MaxX,_MaxY,_MinX, _MinY, _OrgY, _OrgX, _SizeSurf
GLOBAL  _OffVMem, _rlfb, _BitsPixel, _ScanLine, _Mask, _NegScanLine

; intern global DATA
; _CurSurf
EXTERN  QBlue16Mask, QGreen16Mask, QRed16Mask, WBGR16Mask
EXTERN  PntInitCPTDbrd
EXTERN  MaskB_RGB16, MaskG_RGB16, MaskR_RGB16, RGB16_PntNeg, Mask2B_RGB16, Mask2G_RGB16, Mask2R_RGB16
EXTERN  RGBDebMask_GGG, RGBDebMask_IGG, RGBDebMask_GIG, RGBDebMask_IIG, RGBDebMask_GGI, RGBDebMask_IGI, RGBDebMask_GII, RGBDebMask_III
EXTERN  RGBFinMask_GGG, RGBFinMask_IGG, RGBFinMask_GIG, RGBFinMask_IIG, RGBFinMask_GGI, RGBFinMask_IGI, RGBFinMask_GII, RGBFinMask_III
EXTERN  DgNanoSurf


; EXTERN DATA

SECTION .text
[BITS 32]
ALIGN 32

%include "poly.asm"
%include "hzline.asm"
%include "fill.asm"
%include "pts_line.asm"
%include "poly16.asm"
%include "hzline16.asm"
%include "pts16.asm"
%include "bmfont.asm"
%include "fill16.asm"
%include "line16.asm"


_DgSetCurSurf:
	ARG	S1, 4

		PUSH	      EDI
		PUSH	      ESI

		MOV		ESI,[EBP+S1]
		MOV		EAX,[ESI+_ResV-_CurSurf]
		CMP		EAX,MaxResV
		JG		.Error
		MOV		EDI,_CurSurf
		CopySurf
		OR		EAX,BYTE -1
		JMP		SHORT .Ok
.Error:
		XOR		EAX,EAX
.Ok:
		POP		ESI
		POP     EDI

	MMX_RETURN

ALIGN 32
_DgSetSrcSurf:
	ARG	SrcS, 4
		PUSH		EDI
		PUSH	      ESI

		MOV		ESI,[EBP+SrcS]
		MOV		EDI, _SrcSurf
		CopySurf

		POP		ESI
		POP		EDI
    MMX_RETURN

_DgGetCurSurf:
	ARG	S2, 4
		PUSH        EDI
            PUSH        ESI

		MOV		ESI,_CurSurf
		MOV		EDI,[EBP+S2]
		CopySurf

            POP         ESI
            POP         EDI

	MMX_RETURN

; structure point { DWORD X, DWORD Y }
_PutPixel:
	ARG	PtrPoint, 4, col, 4

		MOV		EDX,[EBP+PtrPoint]
		MOV		CL,[EBP+col]
		MOV		EAX,[EDX+4]
		MOV		EDX,[EDX]
		IMUL	      EAX,[_NegScanLine]
		ADD		EAX,[_vlfb]
		MOV		[EAX+EDX],CL
    RETURN

_GetPixel:
	ARG	PtrGPoint, 4

		MOV		EDX,[EBP+PtrGPoint]
		MOV		ECX,[_NegScanLine]
		IMUL	      ECX,[EDX+4]
		XOR		EAX,EAX
		MOV		EDX,[EDX]
		;ADD		ECX,EDX
		ADD		ECX,[_vlfb]
		MOV		AL,[ECX+EDX]
	RETURN

_Clear:
	ARG	clrcol, 4

		PUSH		EDI
		PUSH		ESI
		PUSH		EBX

		MOV		EAX,[EBP+clrcol]
		MOV		AH,AL
		MOV		ECX,EAX
		MOV		ESI,[_SizeSurf]
		BSWAP		EAX
		MOV		EDI,[_rlfb]
		MOV		AX,CX
		@SolidHLine

		POP		EBX
		POP		ESI
		POP		EDI

    MMX_RETURN


_RePoly:
    ARG RePtrListPt, 4, ReSSurf, 4, ReTypePoly, 4, ReColPoly, 4

            PUSH        ESI
            PUSH        EBX
            PUSH        EDI

            ;CMP         [LastPolyStatus], BYTE 'N'
            MOV         EAX,[EBP+ReTypePoly]
            MOV		EBX,[EBP+ReColPoly]
            ;JE			_Poly16.PasDrawPoly
            TEST		EAX,POLY_FLAG_DBL_SIDED
            JZ		SHORT .DblSideCheck ; not a double-sided RePoly16 ?
.doRePoly:
            AND         EAX,DEL_POLY_FLAG_DBL_SIDED
            MOV         ECX,[EBP+ReSSurf]
            MOV         [clr],EBX
            MOV         [SSSurf],ECX
            CMP         [_LastPolyStatus], BYTE 'I' ; last render IN ?
            JNE		.ClipRepoly
            JMP		[InFillPolyProc+EAX*4]
.ClipRepoly:
		JMP		[ClFillPolyProc+EAX*4]
.DblSideCheck:
		; if this is a reversed dbl_sided then skip repoly
		CMP		DWORD [PPtrListPt], ReversedPtrListPt
		JNE		SHORT .doRePoly
		JMP		_Poly.PasDrawPoly

;****************************************************************************
; struct of PtrlistPt
; 0‚		  DWORD : n count of PtrPoint
; 1‚.. n‚  DWORD : PtrP1(X1,Y1,Z1,XT1,YT1)...PtrPn(Xn,Yn,Zn,XTn,YTn)
;****************************************************************************
; TypePoly POLY_SOLID				= 0	 FIELDS USED (X,Y)
; TypePoly POLY_TEXT				= 1	 FIELDS USED (X,Y,XT,YT)
; TypePoly POLY_MASK_TEXT			= 2	 FIELDS USED (X,Y,XT,YT)
; TypePoly POLY_FLAT_DEG			= 3	 FIELDS USED (X,Y,DEG)
; TypePoly POLY_DEG		    		= 4	 FIELDS USED (X,Y,DEG)
; TypePoly POLY_FLAT_DEG_TEXT		= 5	 FIELDS USED (X,Y,XT,YT,DEG)
; TypePoly POLY_MASK_FLAT_DEG_TEXT 	= 6	 FIELDS USED (X,Y,XT,YT,DEG)
; TypePoly POLY_DEG_TEXT			= 7	 FIELDS USED (X,Y,XT,YT,DEG)
; TypePoly POLY_MASK_DEG_TEXT		= 8	 FIELDS USED (X,Y,XT,YT,DEG)
; TypePoly POLY_EFF_FDEG			= 9	 FIELDS USED (X,Y,XT,YT,DEG)
; TypePoly POLY_EFF_DEG				= 10 FIELDS USED (X,Y,XT,YT,DEG)
; TypePoly POLY_EFF_COLCONV			= 11 FIELDS USED (X,Y)
;****************************************************************************
; FLAGS :
POLY_FLAG_DBL_SIDED	EQU	0x80000000
DEL_POLY_FLAG_DBL_SIDED	EQU	0x7FFFFFFF

;****************************************************************************
ALIGN 32
_Poly:
	ARG	PtrListPt, 4, SSurf, 4, TypePoly, 4, ColPoly, 4

		PUSH        ESI
		PUSH        EBX
		MOV		ESI,[EBP+PtrListPt]
		PUSH        EDI

		LODSD		; MOV EAX,[ESI];  ADD ESI,4
            MOV         [_LastPolyStatus], BYTE 'N' ; default no render
		MOV		[NbPPoly],EAX
		MOV		ECX,[ESI+8]
		MOV		EAX,[ESI]
		MOV		EBX,[ESI+4]
		MOVQ		mm0,[EAX] ; = XP1, YP1
		MOVQ		mm1,[EBX] ; = XP2, YP2
		MOVQ		mm2,[ECX] ; = XP3, YP3
		MOVQ		mm3,mm0 ; = XP1, YP1
		MOVQ		mm4,mm1 ; = XP2, YP2
		MOVQ		[XP1],mm0 ; XP1, YP1

;(XP2-XP1)*(YP3-YP2)-(XP3-XP2)*(YP2-YP1)
; s'assure que les points suive le sens inverse de l'aiguille d'une montre
.verifSens:
		PSUBD		mm1,mm0 ; = (XP2-XP1) | (YP2 - YP1)
		PSUBD		mm2,mm4 ; = (XP3-XP2) | (YP3 - YP2)
		MOVD		EAX,mm1 ; = (XP2-XP1)
		MOVD		EDX,mm2 ; = (XP3-XP2)
		PSRLQ		mm1,32
		PSRLQ		mm2,32
		MOVD		EDI,mm1 ; = (YP2-YP1)
		MOVD		EBX,mm2 ; = (YP3-YP2)
		IMUL		EDI,EDX
		IMUL		EAX,EBX
		CMP		EAX,EDI

		JL		.TstSiDblSide ; si <= 0 alors pas ok
		JZ		.PasDrawPoly ; ignore poly if first 3 points aligned

;****************
.DrawPoly:
		; Sauvegarde les parametre et libere EBP
		MOV		EAX,[EBP+TypePoly]
		MOV		EBX,[EBP+ColPoly]
		AND     	EAX,DEL_POLY_FLAG_DBL_SIDED16
		MOV		ECX,[EBP+SSurf]
		MOV		[PType],EAX
		MOV		[clr],EBX
		MOV		[PPtrListPt],ESI
		MOV		EDI,[NbPPoly]
		MOV		[SSSurf],ECX
;-new born determination--------------
		MOV		EBP,EDI
		MOVQ		mm1,mm3 ; init min = XP1 | YP1
		MOVQ		mm2,mm3 ; init max = XP1 | YP1
		DEC		EBP ; = [NbPPoly] - 1
		DEC		EDI ; " "
.PBoucMnMxXY:
		MOV		EAX,[ESI+EBP*4] ; = XN, YN
		MOVQ		mm0,[EAX] ; = XN, YN
		MOVQ		mm3,mm1 ; = min (x|y)
		MOVQ		mm4,mm2 ; = max (x|y)
		PCMPGTD	mm3,mm0 ; mm3 = min(x|y) > (xn|yn)
		PCMPGTD	mm4,mm0 ; mm4 = max(x|y) > (xn|yn)
		MOVQ		mm5,mm3 ;
		MOVQ		mm6,mm4 ;
		PAND		mm3,mm0 ; mm3 = ((xn|yn) < min(x|y)) ? (xn|yn) : (0|0)
		PANDN		mm4,mm0 ; mm4 = ((xn|yn) > max(x|y)) ? (xn|yn) : (0|0)
		PANDN		mm5,mm1 ; mm5 = ((xn|yn) > min(x|y)) ? min (x|y) : (0|0)
		PAND		mm6,mm2 ; mm6 = ((xn|yn) < max(x|y)) ? max (x|y) : (0|0)
		MOVQ		mm1,mm3
		MOVQ		mm2,mm4
		DEC		EBP
		POR		mm1,mm5
		POR		mm2,mm6
		JNZ		.PBoucMnMxXY

		MOVD		EAX,mm2 ; maxx
		MOVD		ECX,mm1 ; minx
		PSRLQ		mm2,32
		PSRLQ		mm1,32
		MOVD		EBX,mm2 ; maxy
		MOVD		EDX,mm1 ; miny

; poly clipper ? dans l'ecran ? hors de l'ecran ?
; poly clipper ?
		CMP		EAX,[_MaxX]
		JG		.PolyClip
		CMP		EBX,[_MaxY]
		JG		.PolyClip
		CMP		ECX,[_MinX]
		JL		.PolyClip
		CMP		EDX,[_MinY]
		JL		.PolyClip

; trace Poly non Clipper  **************************************************
		;JMP		.PolyClip

		MOV		ECX,[_OrgY]	 ; calcule DebYPoly, FinYPoly
		MOV		EAX,[ESI+EDI*4]
		ADD		EDX,ECX
		ADD		EBX,ECX
		MOV		[DebYPoly],EDX
		MOV		[FinYPoly],EBX
; calcule les bornes horizontal du poly
		MOV		EDX,EDI	; EDX compteur de point = NbPPoly-1
		MOV 		ECX,[EAX]
		MOV 		EBP,[EAX+4]
		MOV		[XP2],ECX
		MOV		[YP2],EBP
		@InCalculerContour
		MOV		EAX,[PType]
            MOV         [_LastPolyStatus], BYTE 'I'; In render
		JMP		[InFillPolyProc+EAX*4]
		;JMP			.PasDrawPoly
.PolyClip:
; hors de l'ecran ? alors fin
		CMP		EAX,[_MinX]
		JL		.PasDrawPoly
		CMP		EBX,[_MinY]
		JL		.PasDrawPoly
		CMP		ECX,[_MaxX]
		JG		.PasDrawPoly
		CMP		EDX,[_MaxY]
		JG		.PasDrawPoly

; trace Poly Clipper  ******************************************************
		MOV		EAX,[_MaxY]	; determine DebYPoly, FinYPoly
		MOV		ECX,[_MinY]
		CMP		EBX,EAX
		JL		.PasSupMxY
		MOV		EBX,EAX
.PasSupMxY:
		CMP		EDX,ECX
		JG		.PasInfMnY
		MOV		EDX,ECX
.PasInfMnY:
		MOV		EBP,[_OrgY]	  ; Ajuste [DebYPoly],[FinYPoly]
		MOV		EAX,[ESI+EDI*4]
		ADD		EDX,EBP
		ADD		EBX,EBP
		MOV		[DebYPoly],EDX
		MOV		[FinYPoly],EBX
		MOV		EDX,EDI ; ; EDX compteur de point = NbPPoly-1
		MOV 		ECX,[EAX]
		MOV 		EBP,[EAX+4]
		MOV		[XP2],ECX
		MOV		[YP2],EBP
		MOVD		mm4,ECX
		MOVD		mm5,EBP		; sauvegarde xp2,yp2
		@ClipCalculerContour

		CMP		DWORD [DebYPoly],BYTE (-1)
		MOV		EAX,[PType]
		JE		.PasDrawPoly
            MOV         [_LastPolyStatus], BYTE 'C' ; Clip render
		JMP		[ClFillPolyProc+EAX*4]
.PasDrawPoly:

		POP         EDI
		POP         EBX
            POP         ESI

	MMX_RETURN

.TstSiDblSide:
		TEST		BYTE [EBP+TypePoly+3],POLY_FLAG_DBL_SIDED >> 24
		MOV			ECX,[NbPPoly]
		JZ			.PasDrawPoly
		; swap all points except P1 !
		MOV         EAX,[ESI]
		MOV         EDX,ReversedPtrListPt
		DEC         ECX
		MOV         [EDX],EAX

		LEA         EDI,[ESI+ECX*4]
		LEA         EBX,[EDX+4] ; P1 already copied
		MOV         ESI,EDX ; update [PPtrListPt] In ESI
.BcSwapPts:
		MOV         EAX,[EDI]
		MOV         [EBX],EAX
		SUB         EDI,BYTE 4
		DEC         ECX
		LEA         EBX,[EBX+4]
		JNZ         SHORT .BcSwapPts
		JMP         .DrawPoly


; structure point { DWORD X, DWORD Y }
_PutPixel16:
	ARG	PtrPoint16, 4, col16, 4

		MOV		EAX,[EBP+PtrPoint16]
		MOV		EDX,[_NegScanLine]
		MOV		ECX,[EAX]
		IMUL	      EDX,[EAX+4]
		MOV		EAX,[EBP+col16]
		ADD		EDX,[_vlfb]
		MOV		[EDX+ECX*2],AX
	RETURN

_GetPixel16:
	ARG	PtrGPoint16, 4

		MOV		EDX,[EBP+PtrGPoint16]
		MOV		ECX,[_NegScanLine]
		XOR		EAX,EAX
		IMUL	      ECX,[EDX+4]
		MOV		EDX,[EDX]
		ADD		ECX,[_vlfb]
		MOV		AX,[ECX+EDX*2]
    RETURN

_Clear16:
	ARG	clrcol16, 4

		MOVD		mm7,EDI
		MOVD		mm0,[EBP+clrcol16]
		MOVD		mm6,ESI
		MOVD		mm5,EBX

		PUNPCKLWD	mm0,mm0 ; save firt 2 bytes color 16bpp
		MOV			ESI,[_SizeSurf]
		PUNPCKLDQ	mm0,mm0
		MOV			EDI,[_rlfb]
		MOVD		EAX,mm0
		SHR			ESI,1

		@SolidHLine16

		MOVD		EDI,mm7
		MOVD		ESI,mm6
		MOVD		EBX,mm5

	MMX_RETURN

_ClearSurf16:
    ARG ClearSurf16Col, 4

            PUSH        ESI
            PUSH        EBX
            PUSH        EDI

            MOVD        mm0,[EBP+ClearSurf16Col]
            MOV         EDI,[_MinY]
            PUNPCKLWD	mm0,mm0 ; mm0 = clr16 | clr16 | - | -
            MOV         ESI,[_MaxY]
            PUNPCKLDQ	mm0,mm0 ; mm0 = clr16 | clr16 | clr16 | clr16
            MOV         ECX,[_MaxX]
            SUB         ESI,EDI ; = (MaxY - MinY)
            MOV         EBX,[_MinX]
            JMP         _InBar16.CommonInBar16

_Bar16:
    ARG Bar16P1Ptr, 4, Bar16P2Ptr, 4, Bar16Col, 4

            PUSH        ESI
            PUSH        EBX
            PUSH        EDI

            MOV         ESI,[EBP+Bar16P1Ptr]
            MOV         EDI,[EBP+Bar16P2Ptr]

		MOVQ		mm1,[ESI] ; init min = XP1 | YP1
		MOVQ		mm0,[EDI] ; = XP2 | YP2
		MOVQ		mm2,mm1   ; init max = XP1 | YP1

		MOVQ		mm3,mm1 ; = min (x|y)
		MOVQ		mm4,mm2 ; = max (x|y)
		PCMPGTD	mm3,mm0 ; mm3 = min(x|y) > (xn|yn)
		PCMPGTD	mm4,mm0 ; mm4 = max(x|y) > (xn|yn)
		MOVQ		mm5,mm3 ;
		MOVQ		mm6,mm4 ;
		PAND		mm3,mm0 ; mm3 = ((xn|yn) < min(x|y)) ? (xn|yn) : (0|0)
		PANDN		mm4,mm0 ; mm4 = ((xn|yn) > max(x|y)) ? (xn|yn) : (0|0)
		PANDN		mm5,mm1 ; mm5 = ((xn|yn) > min(x|y)) ? min (x|y) : (0|0)
		PAND		mm6,mm2 ; mm6 = ((xn|yn) < max(x|y)) ? max (x|y) : (0|0)
		MOVQ		mm1,mm3
		MOVQ		mm2,mm4
		POR		mm1,mm5 ; mm1 = min(x|y)
		POR		mm2,mm6 ; mm2 = max(x|y)

            MOVD        EBX,mm1 ; = MinX
            MOVD        ECX,mm2 ; = MaxX
            PSRLQ		mm1,32 ; = MinY
            PSRLQ		mm2,32 ; = MaxY

            ; check completely out
            CMP         EBX,[_MaxX]
            JG         _InBar16.EndInBar
            CMP         ECX,[_MinX]
            JL          _InBar16.EndInBar

            MOVD        EDI,mm1 ; = MinY
            MOVD        ESI,mm2 ; = MaxY
            MOVD        mm0,[EBP+Bar16Col]
            ; check completely out
            CMP         EDI,[_MaxY]
            JG         _InBar16.EndInBar
            CMP         ESI,[_MinY]
            JL          _InBar16.EndInBar

            ; clip coordinates to current view
            MOV         EAX,[_MaxX]
            MOV         EDX,[_MaxY]
            CMP         ECX,EAX ; x2 > MaxX
            MOV         EBP,[_MinX]
            JL          SHORT .NoClipMaxX
            MOV         ECX,EAX
.NoClipMaxX:
            CMP         ESI,EDX ; y2 > MaxY
            MOV         EAX,[_MinY]
            JL          SHORT .NoClipMaxY
            MOV         ESI,EDX
.NoClipMaxY:
            CMP         EBX,EBP ; x1 < MinX
            JG          SHORT .NoClipMinX
            MOV         EBX,EBP
.NoClipMinX:
            CMP         EDI,EAX ; y1 < MinY
            JG          SHORT .NoClipMinY
            MOV         EDI,EAX
.NoClipMinY:

            PUNPCKLWD	mm0,mm0 ; mm0 = clr16 | clr16 | - | -
            PUNPCKLDQ	mm0,mm0 ; mm0 = clr16 | clr16 | clr16 | clr16
            SUB         ESI,EDI ; = (MaxY - MinY)
            JMP         _InBar16.CommonInBar16


_InBar16:
    ARG InRect16MinX, 4, InRect16MinY, 4, InRect16MaxX, 4, InRect16MaxY, 4, InRect16Col, 4

            PUSH        ESI
            PUSH        EBX
            PUSH        EDI

            MOVD        mm0,[EBP+InRect16Col]
            MOV         EDI,[EBP+InRect16MinY]
            PUNPCKLWD	mm0,mm0 ; mm0 = clr16 | clr16 | - | -
            MOV         ESI,[EBP+InRect16MaxY]
            PUNPCKLDQ	mm0,mm0 ; mm0 = clr16 | clr16 | clr16 | clr16
            MOV         ECX,[EBP+InRect16MaxX]
            SUB         ESI,EDI ; = (MaxY - MinY)
            MOV         EBX,[EBP+InRect16MinX]
.CommonInBar16:
            JLE         .EndInBar ; MinY >= MaxY ? exit
            IMUL        EDI,[_NegScanLine]
            LEA         EBP,[ESI+1]
            SUB         ECX,EBX
            ADD         EDI,[_vlfb]
            INC         ECX
            LEA         EDI,[EDI+EBX*2]
            MOV         EDX,ECX ; EDX = dest hline size
            MOVD        EAX,mm0
            MOV         EBX,EDI ; EBX = start Hline dest
            XOR         ECX,ECX ; should be zero for @SolidHLineSSE16
.BcBar:
            MOV         EDI,EBX ; start hline
            MOV         ESI,EDX ; dest hline size

            @SolidHLine16

            ADD         EBX,[_NegScanLine] ; next hline
            DEC         EBP
            JNZ         .BcBar

.EndInBar:
            POP         EDI
            POP         EBX
            POP         ESI

    MMX_RETURN


_BarBlnd16:
    ARG BarBlnd16P1Ptr, 4, BarBlnd16P2Ptr, 4, BarBlnd16Col, 4

            PUSH        ESI
            PUSH        EBX
            PUSH        EDI

            MOV         ESI,[EBP+BarBlnd16P1Ptr]
            MOV         EDI,[EBP+BarBlnd16P2Ptr]

		MOVQ		mm1,[ESI] ; init min = XP1 | YP1
		MOVQ		mm0,[EDI] ; = XP2 | YP2
		MOVQ		mm2,mm1   ; init max = XP1 | YP1

		MOVQ		mm3,mm1 ; = min (x|y)
		MOVQ		mm4,mm2 ; = max (x|y)
		PCMPGTD	mm3,mm0 ; mm3 = min(x|y) > (xn|yn)
		PCMPGTD	mm4,mm0 ; mm4 = max(x|y) > (xn|yn)
		MOVQ		mm5,mm3 ;
		MOVQ		mm6,mm4 ;
		PAND		mm3,mm0 ; mm3 = ((xn|yn) < min(x|y)) ? (xn|yn) : (0|0)
		PANDN		mm4,mm0 ; mm4 = ((xn|yn) > max(x|y)) ? (xn|yn) : (0|0)
		PANDN		mm5,mm1 ; mm5 = ((xn|yn) > min(x|y)) ? min (x|y) : (0|0)
		PAND		mm6,mm2 ; mm6 = ((xn|yn) < max(x|y)) ? max (x|y) : (0|0)
		MOVQ		mm1,mm3
		MOVQ		mm2,mm4
		POR		mm1,mm5 ; mm1 = min(x|y)
		POR		mm2,mm6 ; mm2 = max(x|y)

            MOVD        EBX,mm1 ; = MinX
            MOVD        ECX,mm2 ; = MaxX
            PSRLQ		mm1,32 ; = MinY
            PSRLQ		mm2,32 ; = MaxY

            ; check completely out
            CMP         EBX,[_MaxX]
            JG          _InBarBlnd16.EndInBlndBar
            CMP         ECX,[_MinX]
            JL          _InBarBlnd16.EndInBlndBar

            MOVD        EDI,mm1 ; = MinY
            MOVD        ESI,mm2 ; = MaxY
            ; check completely out
            CMP         EDI,[_MaxY]
            JG          _InBarBlnd16.EndInBlndBar
            CMP         ESI,[_MinY]
            JL          _InBarBlnd16.EndInBlndBar

            ; clip coordinates to current view
            MOV         EAX,[_MaxX]
            MOV         EDX,[_MaxY]
            CMP         ECX,EAX ; x2 > MaxX
            JL          SHORT .NoClipMaxX
            MOV         ECX,EAX
.NoClipMaxX:
            CMP         ESI,EDX ; y2 > MaxY
            MOV         EAX,[_MinX]
            JL          SHORT .NoClipMaxY
            MOV         ESI,EDX
.NoClipMaxY:
            MOV         EDX,[_MinY]
            CMP         EBX,EAX ; x1 < MinX
            JG          SHORT .NoClipMinX
            MOV         EBX,EAX
.NoClipMinX:
            CMP         EDI,EDX ; y1 < MinY
            JG          SHORT .NoClipMinY
            MOV         EDI,EDX
.NoClipMinY:

            MOV         EAX,[EBP+BarBlnd16Col]
            SUB         ESI,EDI ; = (MaxY - MinY)
            JMP         _InBarBlnd16.CommonInBar16

_InBarBlnd16:
    ARG InBlndRect16MinX, 4, InBlndRect16MinY, 4, InBlndRect16MaxX, 4, InBlndRect16MaxY, 4, InBlndRect16Col, 4

            PUSH        ESI
            PUSH        EBX
            PUSH        EDI

            MOV         EAX,[EBP+InBlndRect16Col]
            MOV         EDI,[EBP+InBlndRect16MinY]
            MOV         ESI,[EBP+InBlndRect16MaxY]
            MOV         ECX,[EBP+InBlndRect16MaxX]
            SUB         ESI,EDI ; = (MaxY - MinY)
            MOV         EBX,[EBP+InBlndRect16MinX]
.CommonInBar16:
            JLE         .EndInBlndBar ; MinY >= MaxY ? exit

            IMUL        EDI,[_NegScanLine]
            LEA         EBP,[ESI+1]
            SUB         ECX,EBX
            ADD         EDI,[_vlfb]
            LEA         ESI,[ECX+1] ; = dest hline size
            LEA         EDI,[EDI+EBX*2] ; = start Hline dest

; prepare blending
		MOV       	EBX,EAX ;
		MOV       	ECX,EAX ;
		MOV       	EDX,EAX ;
		AND		EBX,[QBlue16Mask] ; EBX = Bclr16 | Bclr16
		SHR		EAX,24
		AND		ECX,[QGreen16Mask] ; ECX = Gclr16 | Gclr16
		AND		AL,BlendMask ; remove any ineeded bits
		JZ		.EndInBlndBar ; nothing 0 is the source
		AND		EDX,[QRed16Mask] ; EDX = Rclr16 | Rclr16
		XOR		AL,BlendMask ; 31-blendsrc
		MOVD		mm7,EAX
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
		PUNPCKLWD	mm7,mm7
		PUNPCKLDQ	mm3,mm3
		PUNPCKLDQ	mm4,mm4

		PUNPCKLDQ	mm5,mm5
		PUNPCKLDQ	mm7,mm7
; end prep blending
            MOV         EBX,EDI ; EBX = start Hline dest
            MOV         EDX,ESI ; EDX = dest hline size
            XOR         ECX,ECX ; should be zero for SolidBlndHLine16
.BcBar:
            MOV         EDI,EBX ; start hline
            MOV         ESI,EDX ; dest hline size

		@SolidBlndHLine16

            ADD         EBX,[_NegScanLine] ; next hline
            DEC         EBP
            JNZ         .BcBar

.EndInBlndBar:
            POP         EDI
            POP         EBX
            POP         ESI

    MMX_RETURN

; == xxxResizeViewSurf16 =====================================

_ResizeViewSurf16:
    ARG SrcResizeSurf16, 4, ResizeRevertHz, 4, ResizeRevertVt, 4

            PUSH        ESI
            PUSH        EBX
            PUSH        EDI

            MOV         ESI,[EBP+SrcResizeSurf16]
            MOV         EDI,_SrcSurf
            XOR         EBX,EBX ; store flags revert Hz and Vt
            CopySurf  ; copy the source surface


            MOV         EAX,[EBP+ResizeRevertHz]
            MOV         EDX,[EBP+ResizeRevertVt]
            OR          EAX,EAX
            ; compute horizontal pnt in EBP
            MOV         EBP,[_MaxY]
            SETNZ       BL ; BL = RevertHz ?
            OR          EDX,EDX
            MOV         EAX,[SMaxX]
            SETNZ       BH ; BH = RevertVt ?
            MOV         EDI,[_MinY]
            MOV         ESI,[SMinX]
            PUSH        EBX ; save FLAGS Revert
            MOV         ECX,[_MaxX]
            SUB         EBP,EDI ; = (MaxY - MinY)
            MOV         EBX,[_MinX]
            INC         EBP ; = Delta_Y = (MaxY - MinY) + 1
            SUB         EAX,ESI
            IMUL        EDI,[_NegScanLine]
            SUB         ECX,EBX
            ADD         EDI,[_vlfb]
            MOV         [HzLinesCount],EBP ; count of hline
            MOVD        mm5,ESI ; SMinX
            INC         EAX
            LEA         EDI,[EDI+EBX*2]
            INC         ECX
            MOVD        mm6,EDI ; mm6 = start Hline dest
            MOVD        mm2,ECX ; mm2 = dest hline size
            MOV         EBX,[SMaxY]
            SHL         EAX,Prec
            MOV         EDI,EBP ; EDI = DeltaY
            XOR         EDX,EDX
            SUB         EBX,[SMinY]
            DIV         ECX
            INC         EBX ; Source DeltaYT
            SHL         EBX,Prec
            MOV         EBP,EAX
            XOR         EDX,EDX
            MOV         EAX,EBX
            DIV         EDI
            POP         EBX
            XOR         EDX,EDX ; EDX = acc PntX
            MOVD        mm7,[SMinY]
            PXOR        mm4,mm4 ; mm4 = acc pnt
            ;MOVD        ECX,mm5
            CMP         BL,0
            JZ          SHORT .NoRevertHz
            MOVD        mm5,[SMaxX] ; SMaxX
            MOV         EDX,[PntInitCPTDbrd+4] ; ((1<<Prec)-1)
            NEG         EBP ; revert Horizontal Pnt X
.NoRevertHz:
            CMP         BH,0
            MOV		DWORD [HzPntInit],EDX
            JZ          SHORT .NoRevertVt
            NEG         EAX ; negate PntY
            MOVD        mm7,[SMaxY] ; SMaxX
            MOVD        mm4,[PntInitCPTDbrd+4] ; ((1<<Prec)-1)
.NoRevertVt:
            MOVD        mm3,EAX ; xmm3  = pntY
            MOV		[PntPlusX],EBP

.BcResize:
            MOVQ        mm0,mm4
            MOVD        EBX,mm5 ; + [SMinX] | [SMaxX] (if RevertHz)
            PSRAD       mm0,Prec
            PADDD       mm0,mm7 ; + [SMinY] | [SMaxY] (if RevertVt)
            MOVD        EDI,mm6 ; start hline
            MOVD        ESI,mm0
            MOVD        ECX,mm2 ; dest hline size
            IMUL        ESI,[SNegScanLine] ; - 2
            LEA         ESI,[ESI+EBX*2]   ; - 4 + (XT1*2) as 16bpp
            ADD         ESI,[Svlfb] ; - 5

            @InFastTextHLineDYZ16

            PADDD       mm4,mm3 ; next source hline
            PADDD       mm6,[_NegScanLine] ; next hline
            ;DEC         ECX
            DEC		DWORD [HzLinesCount]
            MOV		EDX,DWORD [HzPntInit]
            JNZ         .BcResize

            POP         EDI
            POP         EBX
            POP         ESI
    MMX_RETURN

_MaskResizeViewSurf16:
    ARG SrcMResizeSurf16, 4, MResizeRevertHz, 4, MResizeRevertVt, 4

            PUSH        ESI
            PUSH        EBX
            PUSH        EDI

            MOV         ESI,[EBP+SrcMResizeSurf16]
            MOV         EDI,_SrcSurf
            XOR         EBX,EBX ; store flags revert Hz and Vt
            CopySurf  ; copy the source surface

            MOV         EAX,[EBP+MResizeRevertHz]
            MOV         EDX,[EBP+MResizeRevertVt]
            OR          EAX,EAX
            ; compute horizontal pnt in EBP
            MOV         EBP,[_MaxY]
            SETNZ       BL ; BL = RevertHz ?
            OR          EDX,EDX
            MOV         EAX,[SMaxX]
            SETNZ       BH ; BH = RevertVt ?
            MOV         EDI,[_MinY]
            MOV         ESI,[SMinX]
            PUSH        EBX ; save FLAGS Revert
            MOV         ECX,[_MaxX]
            SUB         EBP,EDI ; = (MaxY - MinY)
            MOV         EBX,[_MinX]
            INC         EBP ; = Delta_Y = (MaxY - MinY) + 1
            SUB         EAX,ESI
            IMUL        EDI,[_NegScanLine]
            SUB         ECX,EBX
            ADD         EDI,[_vlfb]
            MOV         [HzLinesCount],EBP ; count of hline
            MOVD        mm5,ESI ; SMinX
            INC         EAX
            LEA         EDI,[EDI+EBX*2]
            INC         ECX
            MOV         [HzLineDstAddr],EDI ; start Hline dest
            MOV         [HzLineLength],ECX ; dest hline size
            MOV         EBX,[SMaxY]
            SHL         EAX,Prec
            MOV         EDI,EBP ; EDI = DeltaY
            XOR         EDX,EDX
            SUB         EBX,[SMinY]
            DIV         ECX
            INC         EBX ; Source DeltaYT
            SHL         EBX,Prec
            MOV         EBP,EAX
            XOR         EDX,EDX
            MOV         EAX,EBX
            DIV         EDI
            POP         EBX
            XOR         EDX,EDX ; EDX = acc PntX
            MOVD        mm7,[SMinY]
            PXOR        mm4,mm4 ; mm4 = acc pnt
            ;MOVD        ECX,mm5
            CMP         BL,0
            JZ          SHORT .NoRevertHz
            MOVD        mm5,[SMaxX] ; SMaxX
            MOV         EDX,[PntInitCPTDbrd+4] ; ((1<<Prec)-1)
            NEG         EBP ; revert Horizontal Pnt X
.NoRevertHz:
            CMP         BH,0
            MOV		DWORD [HzPntInit],EDX
            JZ          SHORT .NoRevertVt
            NEG         EAX ; negate PntY
            MOVD        mm7,[SMaxY] ; SMaxX
            MOVD        mm4,[PntInitCPTDbrd+4] ; ((1<<Prec)-1)
.NoRevertVt:
            MOVD		[YT1],mm7
            MOV		[PntPlusX],EBP
            MOVD		mm7,[SMask]
            MOVD        mm3,EAX ; xmm3  = pntY
		PUNPCKLWD	mm7,mm7
		PUNPCKLDQ	mm7,mm7 ; = [QSMask16]
			;MOVQ		[QSMask16],mm7

.BcResize:
            MOVQ        mm0,mm4
            MOVD        EBX,mm5 ; + [SMinX] | [SMaxX] (if RevertHz)
            PSRAD       mm0,Prec
            MOVD        ESI,mm0
            MOV         EDI,[HzLineDstAddr] ; start hline
            ADD         ESI,[YT1] ; + [SMinY] | [SMaxY] (if RevertVt)
            MOV         ECX,[HzLineLength] ; dest hline size
            IMUL        ESI,[SNegScanLine] ; - 2
            LEA         ESI,[ESI+EBX*2]   ; - 4 + (XT1*2) as 16bpp
            ADD         ESI,[Svlfb] ; - 5

            @InFastMaskTextHLineDYZ16

		MOV		EAX,[_NegScanLine]
            PADDD       mm4,mm3 ; next source hline
            ADD         [HzLineDstAddr],EAX ; next dst hline
            ;DEC         ECX
            DEC		DWORD [HzLinesCount]
            MOV		EDX,[HzPntInit]
            JNZ         .BcResize

            POP         EDI
            POP         EBX
            POP         ESI

    MMX_RETURN

_BlndResizeViewSurf16:
    ARG SrcBResizeSurf16, 4, BResizeRevertHz, 4, BResizeRevertVt, 4, BResizeColBlnd, 4

            PUSH        ESI
            PUSH        EBX
            PUSH        EDI

            MOV         ESI,[EBP+SrcBResizeSurf16]
            MOV         EDI,_SrcSurf
            CopySurf  ; copy the source surface

; prepare blending
            MOV       	EAX,[EBP+BResizeColBlnd] ;
            MOV       	EBX,EAX ;
            MOV       	ECX,EAX ;
            MOV       	EDX,EAX ;
            AND		EBX,[QBlue16Mask] ; EBX = Bclr16 | Bclr16
            SHR		EAX,24
            AND		ECX,[QGreen16Mask] ; ECX = Gclr16 | Gclr16
            AND		AL,BlendMask ; remove any ineeded bits

            AND		EDX,[QRed16Mask] ; EDX = Rclr16 | Rclr16
            XOR		ESI,ESI
            XOR		AL,BlendMask ; 31-blendsrc
            MOV		SI,AX
            SHL		ESI,16
            OR		SI,AX
            XOR		AL,BlendMask ; 31-blendsrc

            INC		AL
            SHR		DX,5 ; right shift red 5bits
            IMUL		BX,AX
            IMUL		CX,AX
            IMUL		DX,AX
            MOVD		mm7,ESI
            MOVD		mm3,EBX
            MOVD		mm4,ECX
            MOVD		mm5,EDX
            PUNPCKLWD	mm3,mm3
            PUNPCKLWD	mm4,mm4
            PUNPCKLWD	mm5,mm5
            PUNPCKLDQ	mm7,mm7
            PUNPCKLDQ	mm3,mm3
            PUNPCKLDQ	mm4,mm4
            PUNPCKLDQ	mm5,mm5

            MOVQ		[QBlue16Blend],mm3
            MOVQ		[QGreen16Blend],mm4
            MOVQ		[QRed16Blend],mm5
            ;============


            XOR         EBX,EBX ; store flags revert Hz and Vt
            MOV         EAX,[EBP+BResizeRevertHz]
            MOV         EDX,[EBP+BResizeRevertVt]
            OR          EAX,EAX
            ; compute horizontal pnt in EBP
            MOV         EBP,[_MaxY]
            SETNZ       BL ; BL = RevertHz ?
            OR          EDX,EDX
            MOV         EAX,[SMaxX]
            SETNZ       BH ; BH = RevertVt ?
            MOV         EDI,[_MinY]
            MOV         ESI,[SMinX]
            PUSH        EBX ; save FLAGS Revert
            MOV         ECX,[_MaxX]
            SUB         EBP,EDI ; = (MaxY - MinY)
            MOV         EBX,[_MinX]
            INC         EBP ; = Delta_Y = (MaxY - MinY) + 1
            SUB         EAX,ESI
            IMUL        EDI,[_NegScanLine]
            SUB         ECX,EBX
            ADD         EDI,[_vlfb]
            MOV         [HzLinesCount],EBP ; count of hline
            MOVD        mm5,ESI ; SMinX
            INC         EAX
            LEA         EDI,[EDI+EBX*2]
            INC         ECX
            MOV         [HzLineDstAddr],EDI ; start Hline dest
            MOV         [HzLineLength],ECX ; dest hline size
            MOV         EBX,[SMaxY]
            SHL         EAX,Prec
            MOV         EDI,EBP ; EDI = DeltaY
            XOR         EDX,EDX
            SUB         EBX,[SMinY]
            DIV         ECX
            INC         EBX ; Source DeltaYT
            SHL         EBX,Prec
            MOV         EBP,EAX
            XOR         EDX,EDX
            MOV         EAX,EBX
            DIV         EDI
            POP         EBX
            XOR         EDX,EDX ; EDX = acc PntX
            MOVD        mm1,[SMinY]
            PXOR        mm4,mm4 ; mm4 = acc pnt
            ;MOVD        ECX,mm5
            CMP         BL,0
            JZ          SHORT .NoRevertHz
            MOVD        mm5,[SMaxX] ; SMaxX
            MOV         EDX,[PntInitCPTDbrd+4] ; ((1<<Prec)-1)
            NEG         EBP ; revert Horizontal Pnt X
.NoRevertHz:
            CMP         BH,0
            MOV		DWORD [HzPntInit],EDX
            JZ          SHORT .NoRevertVt
            NEG         EAX ; negate PntY
            MOVD        mm1,[SMaxY] ; SMaxX
            MOVD        mm4,[PntInitCPTDbrd+4] ; ((1<<Prec)-1)
.NoRevertVt:
            MOV		[PntPlusX],EBP
            MOVD		[YT1],mm1
            MOVD        mm3,EAX ; xmm3  = pntY
.BcResize:
            MOVD        ESI,mm4
            MOVD        EBX,mm5 ; + [SMinX] | [SMaxX] (if RevertHz)
            SAR       	ESI,Prec
            MOV         EDI,[HzLineDstAddr] ; start hline
            ADD         ESI,[YT1] ; + [SMinY] | [SMaxY] (if RevertVt)
            MOV         ECX,[HzLineLength] ; dest hline size
            IMUL        ESI,[SNegScanLine] ; - 2
            LEA         ESI,[ESI+EBX*2]   ; - 4 + (XT1*2) as 16bpp
            ADD         ESI,[Svlfb] ; - 5

            @InFastTextBlndHLineDYZ16

            MOV		EAX,[_NegScanLine]
            PADDD       mm4,mm3 ; next source hline
            ADD         [HzLineDstAddr],EAX ; next dst hline
            ;DEC         ECX
            DEC		DWORD [HzLinesCount]
            MOV		EDX,[HzPntInit]
            JNZ         .BcResize

            POP         EDI
            POP         EBX
            POP         ESI
    MMX_RETURN

_MaskBlndResizeViewSurf16:
    ARG SrcMBResizeSurf16, 4, MBResizeRevertHz, 4, MBResizeRevertVt, 4, MBResizeColBlnd, 4

            PUSH        ESI
            PUSH        EBX
            PUSH        EDI

            MOV         ESI,[EBP+SrcMBResizeSurf16]
            MOV         EDI,_SrcSurf
            CopySurf  ; copy the source surface

; prepare blending
            MOV       	EAX,[EBP+MBResizeColBlnd] ;
            MOV       	EBX,EAX ;
            MOV       	ECX,EAX ;
            MOV       	EDX,EAX ;
            AND		EBX,[QBlue16Mask] ; EBX = Bclr16 | Bclr16
            SHR		EAX,24
            AND		ECX,[QGreen16Mask] ; ECX = Gclr16 | Gclr16
            AND		AL,BlendMask ; remove any ineeded bits

            AND		EDX,[QRed16Mask] ; EDX = Rclr16 | Rclr16
            XOR		ESI,ESI
		XOR		AL,BlendMask ; 31-blendsrc
		MOV		SI,AX
		SHL		ESI,16
		OR		SI,AX
		XOR		AL,BlendMask ; 31-blendsrc

		INC		AL
		SHR		DX,5 ; right shift red 5bits
		IMUL		BX,AX
		IMUL		CX,AX
		IMUL		DX,AX
		MOVD		mm7,ESI
		MOVD		mm3,EBX
		MOVD		mm4,ECX
		MOVD		mm5,EDX
		PUNPCKLWD	mm3,mm3
		PUNPCKLWD	mm4,mm4
		PUNPCKLWD	mm5,mm5
		PUNPCKLDQ	mm7,mm7
		PUNPCKLDQ	mm3,mm3
		PUNPCKLDQ	mm4,mm4
		PUNPCKLDQ	mm5,mm5

		MOVQ		[QBlue16Blend],mm3
		MOVQ		[QGreen16Blend],mm4
		MOVQ		[QRed16Blend],mm5
			;============


            XOR         EBX,EBX ; store flags revert Hz and Vt
            MOV         EAX,[EBP+MBResizeRevertHz]
            MOV         EDX,[EBP+MBResizeRevertVt]
            OR          EAX,EAX
            ; compute horizontal pnt in EBP
            MOV         EBP,[_MaxY]
            SETNZ       BL ; BL = RevertHz ?
            OR          EDX,EDX
            MOV         EAX,[SMaxX]
            SETNZ       BH ; BH = RevertVt ?
            MOV         EDI,[_MinY]
            MOV         ESI,[SMinX]
            PUSH        EBX ; save FLAGS Revert
            MOV         ECX,[_MaxX]
            SUB         EBP,EDI ; = (MaxY - MinY)
            MOV         EBX,[_MinX]
            INC         EBP ; = Delta_Y = (MaxY - MinY) + 1
            SUB         EAX,ESI
            IMUL        EDI,[_NegScanLine]
            SUB         ECX,EBX
            ADD         EDI,[_vlfb]
            MOV         [HzLinesCount],EBP ; count of hline
            MOVD        mm5,ESI ; SMinX
            INC         EAX
            LEA         EDI,[EDI+EBX*2]
            INC         ECX
            MOV         [HzLineDstAddr],EDI ; start Hline dest
            MOV         [HzLineLength],ECX ; dest hline size
            MOV         EBX,[SMaxY]
            SHL         EAX,Prec
            MOV         EDI,EBP ; EDI = DeltaY
            XOR         EDX,EDX
            SUB         EBX,[SMinY]
            DIV         ECX
            INC         EBX ; Source DeltaYT
            SHL         EBX,Prec
            MOV         EBP,EAX
            XOR         EDX,EDX
            MOV         EAX,EBX
            DIV         EDI
            POP         EBX
            XOR         EDX,EDX ; EDX = acc PntX
            MOVD		mm3,[SMask]
            MOVD        mm1,[SMinY]
            PUNPCKLWD	mm3,mm3
            PXOR        mm4,mm4 ; mm4 = acc pnt
            PUNPCKLDQ	mm3,mm3 ; = [QSMask16]
            MOVQ        [QSMask16],mm3
            ;MOVD        ECX,mm5
            CMP         BL,0
            JZ          SHORT .NoRevertHz
            MOVD        mm5,[SMaxX] ; SMaxX
            MOV         EDX,[PntInitCPTDbrd+4] ; ((1<<Prec)-1)
            NEG         EBP ; revert Horizontal Pnt X
.NoRevertHz:
            CMP         BH,0
            MOV		DWORD [HzPntInit],EDX
            JZ          SHORT .NoRevertVt
            NEG         EAX ; negate PntY
            MOVD        mm1,[SMaxY] ; SMaxX
            MOVD        mm4,[PntInitCPTDbrd+4] ; ((1<<Prec)-1)
.NoRevertVt:
            MOV		[PntPlusX],EBP
            MOVD		[YT1],mm1
            MOV         [PntPlusY],EAX ; pntY
.BcResize:
            MOVD        ESI,mm4
            MOVD        EBX,mm5 ; + [SMinX] | [SMaxX] (if RevertHz)
            SAR       	ESI,Prec
            MOV         EDI,[HzLineDstAddr] ; start hline
            ADD         ESI,[YT1] ; + [SMinY] | [SMaxY] (if RevertVt)
            MOV         ECX,[HzLineLength] ; dest hline size
            IMUL        ESI,[SNegScanLine] ; - 2
            LEA         ESI,[ESI+EBX*2]   ; - 4 + (XT1*2) as 16bpp
            ADD         ESI,[Svlfb] ; - 5

            @InFastMaskTextBlndHLineDYZ16

		MOV		EAX,[_NegScanLine]
            PADDD       mm4,[PntPlusY] ; next source hline
            ADD         [HzLineDstAddr],EAX ; next dst hline
            ;DEC         ECX
            DEC		DWORD [HzLinesCount]
            MOV		EDX,[HzPntInit]
            JNZ         .BcResize

            POP         EDI
            POP         EBX
            POP         ESI
    MMX_RETURN

_TransResizeViewSurf16:
    ARG SrcTResizeSurf16, 4, TResizeRevertHz, 4, TResizeRevertVt, 4, TResizeTrans, 4

            PUSH        ESI
            PUSH        EBX
            PUSH        EDI

; prepare transparency
		MOV       	EAX,[EBP+TResizeTrans] ;
            AND		EAX,BYTE BlendMask
            JZ		.EndResizeView
            ; copy source Surf
            MOV         ESI,[EBP+SrcTResizeSurf16]
            MOV         EDI,_SrcSurf
            CopySurf

            MOV		EDX,EAX ;
            INC		EAX

            XOR		DL,BlendMask ; 31-blendsrc
            MOVD		mm7,EAX
            MOVD		mm6,EDX
            PUNPCKLWD	mm7,mm7
            PUNPCKLWD	mm6,mm6
            PUNPCKLDQ	mm7,mm7
            PUNPCKLDQ	mm6,mm6
			;============

            XOR         EBX,EBX ; store flags revert Hz and Vt
            MOV         EAX,[EBP+TResizeRevertHz]
            MOV         EDX,[EBP+TResizeRevertVt]
            OR          EAX,EAX
            ; compute horizontal pnt in EBP
            MOV         EBP,[_MaxY]
            SETNZ       BL ; BL = RevertHz ?
            OR          EDX,EDX
            MOV         EAX,[SMaxX]
            SETNZ       BH ; BH = RevertVt ?
            MOV         EDI,[_MinY]
            MOV         ESI,[SMinX]
            PUSH        EBX ; save FLAGS Revert
            MOV         ECX,[_MaxX]
            SUB         EBP,EDI ; = (MaxY - MinY)
            MOV         EBX,[_MinX]
            INC         EBP ; = Delta_Y = (MaxY - MinY) + 1
            SUB         EAX,ESI
            IMUL        EDI,[_NegScanLine]
            SUB         ECX,EBX
            ADD         EDI,[_vlfb]
            MOV         [HzLinesCount],EBP ; count of hline
            MOVD        mm5,ESI ; SMinX
            INC         EAX
            LEA         EDI,[EDI+EBX*2]
            INC         ECX
            MOV         [HzLineDstAddr],EDI ; start Hline dest
            MOV         [HzLineLength],ECX ; dest hline size
            MOV         EBX,[SMaxY]
            SHL         EAX,Prec
            MOV         EDI,EBP ; EDI = DeltaY
            XOR         EDX,EDX
            SUB         EBX,[SMinY]
            DIV         ECX
            INC         EBX ; Source DeltaYT
            SHL         EBX,Prec
            MOV         EBP,EAX
            XOR         EDX,EDX
            MOV         EAX,EBX
            DIV         EDI
            POP         EBX
            XOR         EDX,EDX ; EDX = acc PntX
            MOVD        mm1,[SMinY]
            PXOR        mm4,mm4 ; mm4 = acc pnt
            CMP         BL,0
            JZ          SHORT .NoRevertHz
            MOVD        mm5,[SMaxX] ; SMaxX
            MOV         EDX,[PntInitCPTDbrd+4] ; ((1<<Prec)-1)
            NEG         EBP ; revert Horizontal Pnt X
.NoRevertHz:
            CMP         BH,0
            MOV		DWORD [HzPntInit],EDX
            JZ          SHORT .NoRevertVt
            NEG         EAX ; negate PntY
            MOVD        mm1,[SMaxY] ; SMaxX
            MOVD        mm4,[PntInitCPTDbrd+4] ; ((1<<Prec)-1)
.NoRevertVt:
            MOV		[PntPlusX],EBP
            MOV         [PntPlusY],EAX ; pntY
            MOVD        [yPntAcc],mm4
            MOVD		[YT1],mm1
            MOVD        [XT1],mm5
.BcResize:
            MOV         ESI,[yPntAcc]
            MOV         EBX,[XT1] ; + [SMinX] | [SMaxX] (if RevertHz)
            SAR       	ESI,Prec
            MOV         EDI,[HzLineDstAddr] ; start hline
            ADD         ESI,[YT1] ; + [SMinY] | [SMaxY] (if RevertVt)
            MOV         ECX,[HzLineLength] ; dest hline size
            IMUL        ESI,[SNegScanLine] ; - 2
            LEA         ESI,[ESI+EBX*2]   ; - 4 + (XT1*2) as 16bpp
            ADD         ESI,[Svlfb] ; - 5

            @InFastTransTextHLineDYZ16

		MOV		EAX,[_NegScanLine]
            MOV         EBX,[PntPlusY] ; next source hline
            ADD         [HzLineDstAddr],EAX ; next dst hline
            ADD         [yPntAcc],EBX ; increase PntY acc
            ;DEC         ECX
            DEC		DWORD [HzLinesCount]
            MOV		EDX,[HzPntInit]
            JNZ         .BcResize

.EndResizeView:
            POP         EDI
            POP         EBX
            POP         ESI
    MMX_RETURN

_MaskTransResizeViewSurf16:
    ARG SrcMTResizeSurf16, 4, MTResizeRevertHz, 4, MTResizeRevertVt, 4, MTResizeTrans, 4

            PUSH        ESI
            PUSH        EBX
            PUSH        EDI

; prepare transparency
            MOV       	EAX,[EBP+MTResizeTrans] ;
            AND		EAX,BYTE BlendMask
            JZ		.EndResizeView
            ; copy source Surf
            MOV         ESI,[EBP+SrcMTResizeSurf16]
            MOV         EDI,_SrcSurf
            CopySurf

            MOV		EDX,EAX ;
            INC		EAX

            XOR		DL,BlendMask ; 31-blendsrc
            MOVD		mm3,[SMask]
            MOVD		mm7,EAX
            MOVD		mm6,EDX
            PUNPCKLWD	mm3,mm3
            PUNPCKLWD	mm7,mm7
            PUNPCKLWD	mm6,mm6
            PUNPCKLDQ	mm3,mm3 ; = [QSMask16]
            PUNPCKLDQ	mm7,mm7
            PUNPCKLDQ	mm6,mm6
            MOVQ        [QSMask16],mm3
			;============

            XOR         EBX,EBX ; store flags revert Hz and Vt
            MOV         EAX,[EBP+MTResizeRevertHz]
            MOV         EDX,[EBP+MTResizeRevertVt]
            OR          EAX,EAX
            ; compute horizontal pnt in EBP
            MOV         EBP,[_MaxY]
            SETNZ       BL ; BL = RevertHz ?
            OR          EDX,EDX
            MOV         EAX,[SMaxX]
            SETNZ       BH ; BH = RevertVt ?
            MOV         EDI,[_MinY]
            MOV         ESI,[SMinX]
            PUSH        EBX ; save FLAGS Revert
            MOV         ECX,[_MaxX]
            SUB         EBP,EDI ; = (MaxY - MinY)
            MOV         EBX,[_MinX]
            INC         EBP ; = Delta_Y = (MaxY - MinY) + 1
            SUB         EAX,ESI
            IMUL        EDI,[_NegScanLine]
            SUB         ECX,EBX
            ADD         EDI,[_vlfb]
            MOV         [HzLinesCount],EBP ; count of hline
            MOVD        mm5,ESI ; SMinX
            INC         EAX
            LEA         EDI,[EDI+EBX*2]
            INC         ECX
            MOV         [HzLineDstAddr],EDI ; start Hline dest
            MOV         [HzLineLength],ECX ; dest hline size
            MOV         EBX,[SMaxY]
            SHL         EAX,Prec
            MOV         EDI,EBP ; EDI = DeltaY
            XOR         EDX,EDX
            SUB         EBX,[SMinY]
            DIV         ECX
            INC         EBX ; Source DeltaYT
            SHL         EBX,Prec
            MOV         EBP,EAX
            XOR         EDX,EDX
            MOV         EAX,EBX
            DIV         EDI
            POP         EBX
            XOR         EDX,EDX ; EDX = acc PntX
            MOVD        mm1,[SMinY]
            PXOR        mm4,mm4 ; mm4 = acc pnt
            CMP         BL,0
            JZ          SHORT .NoRevertHz
            MOVD        mm5,[SMaxX] ; SMaxX
            MOV         EDX,[PntInitCPTDbrd+4] ; ((1<<Prec)-1)
            NEG         EBP ; revert Horizontal Pnt X
.NoRevertHz:
            CMP         BH,0
            MOV		DWORD [HzPntInit],EDX
            JZ          SHORT .NoRevertVt
            NEG         EAX ; negate PntY
            MOVD        mm1,[SMaxY] ; SMaxX
            MOVD        mm4,[PntInitCPTDbrd+4] ; ((1<<Prec)-1)
.NoRevertVt:
            MOV		[PntPlusX],EBP
            MOV         [PntPlusY],EAX ; pntY
            MOVD        [yPntAcc],mm4
            MOVD		[YT1],mm1
            MOVD        [XT1],mm5
.BcResize:
            MOV         ESI,[yPntAcc]
            MOV         EBX,[XT1] ; + [SMinX] | [SMaxX] (if RevertHz)
            SAR       	ESI,Prec
            MOV         EDI,[HzLineDstAddr] ; start hline
            ADD         ESI,[YT1] ; + [SMinY] | [SMaxY] (if RevertVt)
            MOV         ECX,[HzLineLength] ; dest hline size
            IMUL        ESI,[SNegScanLine] ; - 2
            LEA         ESI,[ESI+EBX*2]   ; - 4 + (XT1*2) as 16bpp
            ADD         ESI,[Svlfb] ; - 5

            @InFastMaskTransTextHLineDYZ16

		MOV		EAX,[_NegScanLine]
            MOV         EBX,[PntPlusY] ; next source hline
            ADD         [HzLineDstAddr],EAX ; next dst hline
            ADD         [yPntAcc],EBX ; increase PntY acc
            ;DEC         ECX
            DEC		DWORD [HzLinesCount]
            MOV		EDX,[HzPntInit]
            JNZ         .BcResize

.EndResizeView:
            POP         EDI
            POP         EBX
            POP         ESI
    MMX_RETURN

; ==== Poly16 and RePoly16 =============================

_RePoly16:
    ARG RePtrListPt16, 4, ReSSurf16, 4, ReTypePoly16, 4, ReColPoly16, 4

            PUSH        ESI
            PUSH        EBX
            PUSH        EDI

            ;CMP         [LastPolyStatus], BYTE 'N'
            MOV         EAX,[EBP+ReTypePoly16]
            MOV		EBX,[EBP+ReColPoly16]
            ;JE		_Poly16.PasDrawPoly
            TEST		EAX,POLY_FLAG_DBL_SIDED16
            JZ		SHORT .DblSideCheck ; not a double-sided RePoly16 ?
.doRePoly:
            AND         EAX,DEL_POLY_FLAG_DBL_SIDED16
            MOV         ECX,[EBP+ReSSurf16]
            MOV         [clr],EBX
            MOV         [SSSurf],ECX
            CMP         [_LastPolyStatus], BYTE 'I' ; last render IN ?
            JNE		.ClipRepoly16
            JMP		[InFillPolyProc16+EAX*4]
.ClipRepoly16:
            JMP		[ClFillPolyProc16+EAX*4]
.DblSideCheck:
		; if this is a reversed dbl_sided then skip repoly
		CMP		DWORD [PPtrListPt], ReversedPtrListPt
		JNE		SHORT .doRePoly
		JMP		_Poly16.PasDrawPoly

;****************************************************************************
;struct of PtrlistPt
;0‚		  DWORD : n count of PtrPoint
;1‚.. n‚  DWORD : PtrP1(X1,Y1,Z1,XT1,YT1)...PtrPn(Xn,Yn,Zn,XTn,YTn)
;****************************************************************************
; TypePoly POLY16_SOLID				= 0	 FIELDS USED (X,Y)
; TypePoly POLY16_TEXT				= 1	 FIELDS USED (X,Y,XT,YT)
; TypePoly POLY16_MASK_TEXT			= 2	 FIELDS USED (X,Y,XT,YT)
; TypePoly POLY16_TEXT_TRANS		= 10 FIELDS USED (X,Y,XT,YT)
; TypePoly POLY16_MASK_TEXT_TRANS	= 11 FIELDS USED (X,Y,XT,YT)
; TypePoly POLY16_SOLID_BLND		= 13 FIELDS USED (X,Y)
; TypePoly POLY16_TEXT_BLND			= 14 FIELDS USED (X,Y,XT,YT)
; TypePoly POLY16_MASK_TEXT_BLND	= 15 FIELDS USED (X,Y,XT,YT)
;****************************************************************************
; FLAGS :
POLY_FLAG_DBL_SIDED16		EQU	0x80000000
DEL_POLY_FLAG_DBL_SIDED16	EQU	0x7FFFFFFF

;****************************************************************************
ALIGN 32
_Poly16:
	ARG	PtrListPt16, 4, SSurf16, 4, TypePoly16, 4, ColPoly16, 4

		PUSH        ESI
		PUSH        EBX
		MOV		ESI,[EBP+PtrListPt16]
		PUSH        EDI

		LODSD		; MOV EAX,[ESI];  ADD ESI,4
            MOV         [_LastPolyStatus], BYTE 'N' ; default no render
		MOV		[NbPPoly],EAX
		MOV		ECX,[ESI+8]
		MOV		EAX,[ESI]
		MOV		EBX,[ESI+4]
		MOVQ		mm0,[EAX] ; = XP1, YP1
		MOVQ		mm1,[EBX] ; = XP2, YP2
		MOVQ		mm2,[ECX] ; = XP3, YP3
		MOVQ		mm3,mm0 ; = XP1, YP1
		MOVQ		mm4,mm1 ; = XP2, YP2
		MOVQ		[XP1],mm0 ; XP1, YP1

;(XP2-XP1)*(YP3-YP2)-(XP3-XP2)*(YP2-YP1)
; s'assure que les points suive le sens inverse de l'aiguille d'une montre
.verifSens:
		PSUBD		mm1,mm0 ; = (XP2-XP1) | (YP2 - YP1)
		PSUBD		mm2,mm4 ; = (XP3-XP2) | (YP3 - YP2)
		MOVD		EAX,mm1 ; = (XP2-XP1)
		MOVD		EDX,mm2 ; = (XP3-XP2)
		PSRLQ		mm1,32
		PSRLQ		mm2,32
		MOVD		EDI,mm1 ; = (YP2-YP1)
		MOVD		EBX,mm2 ; = (YP3-YP2)
		IMUL		EDI,EDX
		IMUL		EAX,EBX
		CMP		EAX,EDI

		JL		.TstSiDblSide ; si <= 0 alors pas ok
		JZ		.PasDrawPoly ; ignore poly if first 3 points aligned
;****************
.DrawPoly:
		; Sauvegarde les parametre et libere EBP
		MOV		EAX,[EBP+TypePoly16]
		MOV		EBX,[EBP+ColPoly16]
		AND     	EAX,DEL_POLY_FLAG_DBL_SIDED16
		MOV		ECX,[EBP+SSurf16]
		MOV		[PType],EAX
		MOV		[clr],EBX
		MOV		[PPtrListPt],ESI
		MOV		EDI,[NbPPoly]
		MOV		[SSSurf],ECX
;-new born determination--------------
		MOV		EBP,EDI
		MOVQ		mm1,mm3 ; init min = XP1 | YP1
		MOVQ		mm2,mm3 ; init max = XP1 | YP1
		DEC		EBP ; = [NbPPoly] - 1
		DEC		EDI ; " "
.PBoucMnMxXY:
		MOV		EAX,[ESI+EBP*4] ; = XN, YN
		MOVQ		mm0,[EAX] ; = XN, YN
		MOVQ		mm3,mm1 ; = min (x|y)
		MOVQ		mm4,mm2 ; = max (x|y)
		PCMPGTD	mm3,mm0 ; mm3 = min(x|y) > (xn|yn)
		PCMPGTD	mm4,mm0 ; mm4 = max(x|y) > (xn|yn)
		MOVQ		mm5,mm3 ;
		MOVQ		mm6,mm4 ;
		PAND		mm3,mm0 ; mm3 = ((xn|yn) < min(x|y)) ? (xn|yn) : (0|0)
		PANDN		mm4,mm0 ; mm4 = ((xn|yn) > max(x|y)) ? (xn|yn) : (0|0)
		PANDN		mm5,mm1 ; mm5 = ((xn|yn) > min(x|y)) ? min (x|y) : (0|0)
		PAND		mm6,mm2 ; mm6 = ((xn|yn) < max(x|y)) ? max (x|y) : (0|0)
		MOVQ		mm1,mm3
		MOVQ		mm2,mm4
		DEC		EBP
		POR		mm1,mm5
		POR		mm2,mm6
		JNZ		.PBoucMnMxXY
		MOVD		EAX,mm2 ; maxx
		MOVD		ECX,mm1 ; minx
		PSRLQ		mm2,32
		PSRLQ		mm1,32
		MOVD		EBX,mm2 ; maxy
		MOVD		EDX,mm1 ; miny
;-----------------------------------------

; poly clipper ? dans l'ecran ? hors de l'ecran ?
		CMP		EAX,[_MaxX]
		JG		.PolyClip
		CMP		ECX,[_MinX]
		JL		.PolyClip
		CMP		EBX,[_MaxY]
		JG		.PolyClip
		CMP		EDX,[_MinY]
		JL		.PolyClip

; trace Poly non Clipper  **************************************************
		;JMP		.PolyClip

		MOV		ECX,[_OrgY]	 ; calcule DebYPoly, FinYPoly
		MOV		EAX,[ESI+EDI*4]
		ADD		EDX,ECX
		ADD		EBX,ECX
		MOV		[DebYPoly],EDX
		MOV		[FinYPoly],EBX
; calcule les bornes horizontal du poly
		MOV		EDX,EDI ; = NbPPoly - 1
		MOV		ECX,[EAX]
		MOV		EBP,[EAX+4]
		MOV		[XP2],ECX
		MOV		[YP2],EBP
		@InCalculerContour16
		MOV		EAX,[PType]
            MOV         [_LastPolyStatus], BYTE 'I'; In render
		JMP		[InFillPolyProc16+EAX*4]
		;JMP			.PasDrawPoly

.PolyClip:
; outside view ? now draw !
		CMP		EAX,[_MinX]
		JL		.PasDrawPoly
		CMP		EBX,[_MinY]
		JL		.PasDrawPoly
		CMP		ECX,[_MaxX]
		JG		.PasDrawPoly
		CMP		EDX,[_MaxY]
		JG		.PasDrawPoly
; Drop too big poly
		; drop too BIG tri
		SUB		ECX,EAX  ; deltaY
		SUB		EDX,EBX  ; deltaX
		CMP		ECX,MaxDeltaDim
		JGE		.PasDrawPoly
		CMP		EDX,MaxDeltaDim
		JGE		.PasDrawPoly
		ADD		ECX,EAX ; restor MaxY
		ADD		EDX,EBX ; restor MaxX

; trace Poly Clipper  ******************************************************
		MOV		EAX,[_MaxY]	; determine DebYPoly, FinYPoly
		MOV		ECX,[_MinY]
		CMP		EBX,EAX
		JL		.PasSupMxY
		MOV		EBX,EAX
.PasSupMxY:
		CMP		EDX,ECX
		JG		.PasInfMnY
		MOV		EDX,ECX
.PasInfMnY:
		MOV		EBP,[_OrgY]	  ; Ajuste [DebYPoly],[FinYPoly]
		MOV		EAX,[ESI+EDI*4]
		ADD		EDX,EBP
		ADD		EBX,EBP
		MOV		[DebYPoly],EDX
		MOV		[FinYPoly],EBX
		MOV		EDX,EDI ; EDX compteur de point = NbPPoly-1
		MOV 		ECX,[EAX]
		MOV 		EBP,[EAX+4]
		MOV		[XP2],ECX
		MOV		[YP2],EBP
		MOVD		mm4,ECX
		MOVD		mm5,EBP		; sauvegarde xp2,yp2
		@ClipCalculerContour ; use same as 8bpp as it compute xdeb and xfin for eax hzline

		CMP		DWORD [DebYPoly],BYTE (-1)
		MOV		EAX,[PType]
		JE		.PasDrawPoly
            MOV         [_LastPolyStatus], BYTE 'C' ; Clip render
		JMP		[ClFillPolyProc16+EAX*4]

.PasDrawPoly:
            POP         EDI
            POP         EBX
            POP         ESI

    MMX_RETURN

.TstSiDblSide:
		TEST		BYTE [EBP+TypePoly+3],POLY_FLAG_DBL_SIDED16 >> 24
		MOV		ECX,[NbPPoly]
		JZ		.PasDrawPoly
		; swap all points except P1 !
		MOV         EAX,[ESI]
		MOV         EDX,ReversedPtrListPt
		DEC         ECX
		MOV         [EDX],EAX

		LEA         EDI,[ESI+ECX*4]
		LEA         EBX,[EDX+4] ; P1 already copied
		MOV         ESI,EDX ; update [PPtrListPt] In ESI
.BcSwapPts:
		MOV         EAX,[EDI]
		MOV         [EBX],EAX
		SUB         EDI,BYTE 4
		DEC         ECX
		LEA         EBX,[EBX+4]
		JNZ         SHORT .BcSwapPts
		JMP         .DrawPoly



SECTION .bss   ALIGN=32
_CurSurf:
_ScanLine          	RESD   1
_rlfb              	RESD   1
_OrgX              	RESD   1
_OrgY              	RESD   1
_MaxX              	RESD   1
_MaxY              	RESD   1
_MinX              	RESD   1
_MinY              	RESD   1;-----------------------
_Mask              	RESD   1
_ResH              	RESD   1
_ResV              	RESD   1
_vlfb              	RESD   1
_NegScanLine       	RESD   1
_OffVMem           	RESD   1
_BitsPixel         	RESD   1
_SizeSurf          	RESD   1 ;-----------------------
; source texture
_SrcSurf:
SScanLine         	RESD   1
Srlfb             	RESD   1
SOrgX             	RESD   1
SOrgY             	RESD   1
SMaxX             	RESD   1
SMaxY             	RESD   1
SMinX             	RESD   1
SMinY             	RESD   1;-----------------------
SMask             	RESD   1
SResH             	RESD   1
SResV             	RESD   1
Svlfb             	RESD   1
SNegScanLine      	RESD   1
SOffVMem          	RESD   1
SBitsPixel        	RESD   1
SSizeSurf         	RESD   1;-----------------------

XP1					RESD   1
YP1					RESD   1
XP2					RESD   1
YP2					RESD   1
XP3					RESD   1
YP3					RESD   1
Plus				RESD   1
clr					RESD   1;-----------------------
XT1					RESD   1
YT1					RESD   1
XT2					RESD   1
YT2					RESD   1
Col1				RESD   1
Col2				RESD   1
revCol				RESD   1
_CurViewVSurf		RESD   1;-----------------------
PutSurfMaxX     	RESD   1
PutSurfMaxY     	RESD   1
PutSurfMinX     	RESD   1
PutSurfMinY     	RESD   1
NbPPoly				RESD   1
DebYPoly			RESD   1
FinYPoly			RESD   1
PType				RESD   1;-----------------------
PType2				RESD   1
PPtrListPt			RESD   1
SSSurf				RESD   1
PntPlusX			RESD   1
PntPlusY			RESD   1
PlusX				RESD   1
PlusY				RESD   1
Plus2				RESD   1;-----------------------
Temp				RESD   2
QMulSrcBlend		RESD   2
QMulDstBlend		RESD   2
PlusCol				RESD   1
PtrTbDegCol			RESD   1;-----------------------

; used by Poly and Poly16
_TbDegCol			RESB	256*64
_TPolyAdDeb			RESD	MaxResV
_TPolyAdFin			RESD	MaxResV
_TexXDeb 			RESD	MaxResV
_TexXFin 			RESD	MaxResV
_TexYDeb 			RESD	MaxResV
_TexYFin 			RESD	MaxResV
_PColDeb 			RESD	MaxResV
_PColFin 			RESD	MaxResV

; bitmap fonts data
_CurDBMFONT:
BMCharsSSurfs      RESD    256
BMCharsPlusX       RESD    256
BMCharsWidth       RESD    256
BMCharsHeight      RESD    256
BMCharsXOffset     RESD    256
BMCharsYOffset     RESD    256
BMCharsGHeight     RESD    1
BMCharsGLineHeight RESD    1
BMCharX            RESD    1
BMCharY            RESD    1
BMCharCurChar      RESD    1
BMCharsMainSurf    RESD    1
BMCharsRendX       RESD    1
BMCharsRendY       RESD    1;--------------


; reversed PPtrListPt pointer
ReversedPtrListPt   RESD  MaxDblSidePolyPts

; glob variables for Poly/Poly16 ..
QBlue16Blend		RESD	2
QGreen16Blend		RESD	2
QRed16Blend			RESD	2
QSMask16			RESD	2
QHLineOrg			RESD	2
DebStartAddr		RESD	1
HzLinesCount		RESD	1
HzLineLength		RESD	1
HzLineDstAddr		RESD	1
HzPntInit			RESD	1
HzLineLengthCount	RESD	1
yPntAcc             RESD	1
ClipHStartAddr		RESD	1
_PtrTbColConv		RESD	1
_LastPolyStatus 	RESD  	1
ChPlus				RESD	1

SECTION .data   ALIGN=32

;* 8bpp poly proc****
InFillPolyProc:
					DD	InFillSOLID,InFillTEXT,InFillMASK_TEXT,InFillFLAT_DEG,InFillDEG
					DD	InFillFLAT_DEG_TEXT,InFillMASK_FLAT_DEG_TEXT
					DD	InFillDEG_TEXT,InFillMASK_DEG_TEXT,InFillEFF_FDEG
					DD	InFillEFF_DEG,InFillEFF_COLCONV
ClFillPolyProc:		DD	ClipFillSOLID,ClipFillTEXT,ClipFillMASK_TEXT,ClipFillFLAT_DEG,ClipFillDEG
					DD	ClipFillFLAT_DEG_TEXT,ClipFillMASK_FLAT_DEG_TEXT
					DD	ClipFillDEG_TEXT,ClipFillMASK_DEG_TEXT
					DD	ClipFillEFF_FDEG,ClipFillEFF_DEG,ClipFillEFF_COLCONV

;* 16bpp poly proc****
InFillPolyProc16:
					DD	InFillSOLID16,InFillTEXT16,InFillMASK_TEXT16,dummyFill16,dummyFill16
					DD	dummyFill16,dummyFill16
					DD	dummyFill16,dummyFill16
					DD	dummyFill16,InFillTEXT_TRANS16,InFillMASK_TEXT_TRANS16
					DD	InFillRGB16,InFillSOLID_BLND16,InFillTEXT_BLND16,InFillMASK_TEXT_BLND16

ClFillPolyProc16:
					DD	ClipFillSOLID16,ClipFillTEXT16,ClipFillMASK_TEXT16,dummyFill16,dummyFill16
					DD	dummyFill16,dummyFill16
					DD	dummyFill16,dummyFill16
					DD	dummyFill16,ClipFillTEXT_TRANS16, ClipFillMASK_TEXT_TRANS16
					DD	ClipFillRGB16,ClipFillSOLID_BLND16,ClipFillTEXT_BLND16,ClipFillMASK_TEXT_BLND16

