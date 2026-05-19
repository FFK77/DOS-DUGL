%include "PARAM.MAC"

; GLOBAL Function*************************************************************
;***
GLOBAL _ScanYUV2RGB16,_Scan422YUV2RGB16

; GLOBAL DATA*****************************************************************
;***

ALIGN 32
SECTION .text
[BITS 32]
;***
_ScanYUV2RGB16:
	ARG	YSrcPtr, 4, USrcPtr, 4,  VSrcPtr, 4, RGB16DstPtr, 4, PixelsSize, 4
            PUSH        EBX
            PUSH        ESI
            PUSH        EDI

			MOV			ECX,[EBP+PixelsSize]
			MOV			EDX,[EBP+VSrcPtr]
			MOV			EBX,[EBP+USrcPtr]
			MOV			ESI,[EBP+YSrcPtr]
			MOV			EAX,QHalfMaxUV
			MOV			EDI,[EBP+RGB16DstPtr]
			SHR			ECX,2
			PXOR		mm7,mm7
ALIGN 4
.BcYUV2RGB:
            MOVD        mm6,[EDX] ; mm6 = B V0 | B V1 | B V2 | B V3 || D 0
            MOVD        mm0,[ESI] ; mm0 = B Y0 | B Y1 | B Y2 | B Y3 || D 0
            PUNPCKLBW   mm6,mm7 ; mm6 = W V0 | W V1 | W V2 | W V3
            PUNPCKLBW   mm0,mm7 ; mm0 = W Y0 | W Y1 | W Y2 | W Y3
            PSUBW       mm6,[EAX+AQHalfMaxUV] ; mm6 = W V0-128 | W V1-128 | W V2-128 | W V3-128
            PSUBW       mm0,[EAX+AQWF0_0625] ; mm0 = W Y0 - 0.0625 | W Y1 - 0.0625 | ....
            MOVQ        mm5,mm6
            PMULLW      mm0,[EAX+AQWF1_1643]
            MOVD        mm4,[EBX] ; mm4 = B U0 | B U1 | B U2 | B U3 || D 0
            PMULLW      mm6,[EAX+AQWF1_5958]
            PSRAW       mm0,Prec ; mm0 = W (Y0-0.0625)*1.1643 | W (Y1-0.0625)*1.1643 | ...
            PUNPCKLBW   mm4,mm7 ; mm4 = W U0 | W U1 | W U2 | W U3

            PMULLW      mm5,[EAX+AQWF0_8129]
            PSUBW       mm4,[EAX+AQHalfMaxUV] ; mm4 = W U0-128 | W U1-128 | W U2-128 | W U3-128
            PSRAW       mm6,Prec ; mm0 = W (V0)*1.5958 | W (V1)*1.5958 | ...
            MOVQ        mm3,mm4
            MOVQ        mm2,mm4
            PSRAW       mm5,Prec ; mm0 = W (V0)*0.8129 | W (V1)*0.8129 | ...
            PMULLW      mm3,[EAX+AQWF0_0170]
            PMULLW      mm4,[EAX+AQWF0_3917]
            PSLLW       mm2,1    ; mm2 = W (U0)*2.0000 | W (U1)*2.0000 | ...
            PSRAW       mm3,Prec ; mm3 = W (U0)*0.0170 | W (U1)*0.0170 | ...
            PSRAW       mm4,Prec ; mm4 = W (U0)*0.3917 | W (U1)*0.3917 | ...
            PADDW		mm3,mm0
            MOVQ        mm1,mm0
            PADDW       mm3,mm2  ; mm3 = blue 0 | blue 1 | blue 2 | blue 3

            PSUBW       mm1,mm4
            PADDW       mm0,mm6  ; mm0 = red 0 | red 1 | red 2 | red 3
			PSUBW		mm1,mm5  ; mm1 = green 0 | green 1 | green 2 | green 3
            ; adjust color boundaries --
			PACKUSWB	mm0,mm0
			PACKUSWB	mm3,mm3
			PACKUSWB	mm1,mm1
			PUNPCKLBW	mm0,mm7 ; mm0 = W r0 | W r1 | W r2 | W r3
			PUNPCKLBW	mm3,mm7 ; mm3 = W b0 | W b1 | W b2 | W b3
			PUNPCKLBW	mm1,mm7 ; mm1 = W g0 | W g1 | W g2 | W g3

            ;- end adjust color boundaries--
            PSRLW       mm0,3
            PSRLW       mm3,3
            PSLLW       mm0,11
            PSRLW       mm1,2
            POR         mm0,mm3
            PSLLW       mm1,5

            DEC         ECX
            POR         mm0,mm1
            LEA         ESI,[ESI+4]
            MOVQ        [EDI],mm0
            LEA         EDX,[EDX+4]
            LEA         EBX,[EBX+4]
            LEA         EDI,[EDI+8]
            JNZ         .BcYUV2RGB

		    POP         EDI
		    POP		    ESI
		    POP         EBX
		    MMX_RETURN

ALIGN 32
_Scan422YUV2RGB16:
	ARG	YSrcPtr42, 4, USrcPtr42, 4,  VSrcPtr42, 4, RGB16DstPtr42, 4, PixelsSize42, 4
            PUSH        EBX
            PUSH        ESI
            PUSH        EDI

			MOV			ECX,[EBP+PixelsSize42]
			MOV			EDX,[EBP+VSrcPtr42]
			MOV			EBX,[EBP+USrcPtr42]
			MOV			ESI,[EBP+YSrcPtr42]
			MOV			EAX,QHalfMaxUV
			MOV			EDI,[EBP+RGB16DstPtr42]
			SHR			ECX,2
			PXOR		mm7,mm7
ALIGN 4
.BcYUV2RGB:
            MOVD        mm6,[EDX] ; mm6 = B V0 | B V1 | B V2 | B V3 || D 0
            MOVD        mm0,[ESI] ; mm0 = B Y0 | B Y1 | B Y2 | B Y3 || D 0
            PUNPCKLBW   mm6,mm6 ; mm6 = B V0 | B V0 | B V1 | B V1 || ...
            PUNPCKLBW   mm0,mm7 ; mm0 = W Y0 | W Y1 | W Y2 | W Y3
            PUNPCKLBW   mm6,mm7 ; mm6 = W V0 | W V1 | W V2 | W V3
            PSUBW       mm0,[EAX+AQWF0_0625] ; mm0 = W Y0 - 0.0625 | W Y1 - 0.0625 | ....
            PSUBW       mm6,[EAX+AQHalfMaxUV] ; mm6 = W V0-128 | W V1-128 | W V2-128 | W V3-128
            MOVD        mm4,[EBX] ; mm4 = B U0 | B U1 | B U2 | B U3 || D 0
            MOVQ        mm5,mm6
            PMULLW      mm0,[EAX+AQWF1_1643]
            PMULLW      mm6,[EAX+AQWF1_5958]
            PUNPCKLBW   mm4,mm4 ; mm4 = B U0 | B U0 | B U1 | B U1 || ....
            PSRAW       mm0,Prec ; mm0 = W (Y0-0.0625)*1.1643 | W (Y1-0.0625)*1.1643 | ...
            PUNPCKLBW   mm4,mm7 ; mm4 = W U0 | W U1 | W U2 | W U3

            PMULLW      mm5,[EAX+AQWF0_8129]
            PSUBW       mm4,[EAX+AQHalfMaxUV] ; mm4 = W U0-128 | W U1-128 | W U2-128 | W U3-128
            PSRAW       mm6,Prec ; mm0 = W (V0)*1.5958 | W (V1)*1.5958 | ...
            MOVQ        mm3,mm4
            MOVQ        mm2,mm4
            PSRAW       mm5,Prec ; mm0 = W (V0)*0.8129 | W (V1)*0.8129 | ...
            PMULLW      mm3,[EAX+AQWF0_0170]
            PMULLW      mm4,[EAX+AQWF0_3917]
            PSLLW       mm2,1    ; mm2 = W (U0)*2.0000 | W (U1)*2.0000 | ...
            PSRAW       mm3,Prec ; mm3 = W (U0)*0.0170 | W (U1)*0.0170 | ...
            PSRAW       mm4,Prec ; mm4 = W (U0)*0.3917 | W (U1)*0.3917 | ...
            PADDW		mm3,mm0
            MOVQ        mm1,mm0
            PADDW       mm3,mm2  ; mm3 = blue 0 | blue 1 | blue 2 | blue 3

            PSUBW       mm1,mm4
            PADDW       mm0,mm6  ; mm0 = red 0 | red 1 | red 2 | red 3
			PSUBW		mm1,mm5  ; mm1 = green 0 | green 1 | green 2 | green 3
            ; adjust color boundaries --
			PACKUSWB	mm0,mm0
			PACKUSWB	mm3,mm3
			PACKUSWB	mm1,mm1
			PUNPCKLBW	mm0,mm7 ; mm0 = W r0 | W r1 | W r2 | W r3
			PUNPCKLBW	mm3,mm7 ; mm3 = W b0 | W b1 | W b2 | W b3
			PUNPCKLBW	mm1,mm7 ; mm1 = W g0 | W g1 | W g2 | W g3

            ;- end adjust color boundaries--
            PSRLW       mm0,3
            PSRLW       mm3,3
            PSLLW       mm0,11
            PSRLW       mm1,2
            POR         mm0,mm3
            PSLLW       mm1,5

            DEC         ECX
            POR         mm0,mm1
            LEA         ESI,[ESI+4]
            MOVQ        [EDI],mm0
            LEA         EDX,[EDX+2]
            LEA         EBX,[EBX+2]
            LEA         EDI,[EDI+8]
            JNZ         .BcYUV2RGB

		    POP         EDI
		    POP		    ESI
		    POP         EBX
		    MMX_RETURN

ALIGN 32
SECTION	.data
Prec        EQU         6
; constants
F_0_0625    EQU         16 ; 256*0.0625
F_1_1643    EQU         (11643*((1<<Prec)-1))/10000 ; 1.1643
F_1_5958    EQU         (15958*((1<<Prec)-1))/10000 ; 1.5958
F_0_3917    EQU         (03917*((1<<Prec)-1))/10000 ; 0.3917
F_0_8129    EQU         (08129*((1<<Prec)-1))/10000 ; 0.8129
F_0_0170    EQU         (00170*((1<<Prec)-1))/10000 ; 2.0170 = 2 + 0.0170

AQHalfMaxUV EQU          0
AQWF0_0625  EQU          8
AQWF1_1643  EQU          16
AQWF1_5958  EQU          24
AQWF0_8129  EQU          32
AQWF0_3917  EQU          40
AQWF0_0170  EQU          48

;*** data
QHalfMaxUV  DW          128, 128, 128, 128
QWF0_0625   DW          F_0_0625, F_0_0625, F_0_0625, F_0_0625
QWF1_1643   DW          F_1_1643, F_1_1643, F_1_1643, F_1_1643
QWF1_5958   DW          F_1_5958, F_1_5958, F_1_5958, F_1_5958
QWF0_8129   DW          F_0_8129, F_0_8129, F_0_8129, F_0_8129
QWF0_3917   DW          F_0_3917, F_0_3917, F_0_3917, F_0_3917
QWF0_0170   DW          F_0_0170, F_0_0170, F_0_0170, F_0_0170


