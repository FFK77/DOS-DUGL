		struc VoicePack
.Speed		resd	1
.LeftVol	resd	1
.RightVol	resd	1
.Priority	resd	1
.resv		resd	6
.Fin:
		endstruc

		struc Voice
.Ptr:		resd	1
.Size:		resd	1
.Freq:		resd	1
.Type:		resd	1
.SizeSecond	resd	1
.Effect:	resd	1
.Pos:		resd	1
.Priority:	resd	1
.LeftVol:	resd	1
.RightVol:	resd	1
.Speed:		resd	1
.RestCoeff:	resd	1
.VcSTATE	resd	1
.VoiceResv:	resd	11
.Fin:
		endstruc

;******* PROCEDURE PROCED pour Different TYPE E/S
;************************************************

;******* PROCED SORTIE 8 Bits MONO
ProcedOut8BitsM:
		CALL		GetBaseAddress

		XOR			EAX,EAX 		; zero Buff Calc ----
		MOV			EDI,[EBX+BuffCalc8Bit]
		MOV			ECX,SizeDMABuff/2
		REP			STOSD

		MOV			ECX,[EBX+MaxNbVoice]
		LEA			ESI,[EBX+VoicePtr]
.BcTrtVoice:
		LODSD
		OR			EAX,EAX
		JZ			.FreeVoice
		CMP			DWORD [EAX+Voice.Effect],BYTE 0
		JNE			.VoiceEff
		CALL		VoiceNoEff8BM
		JMP			SHORT .VoiceNEff
.VoiceEff:
		CALL		VoiceEff8BM
.VoiceNEff:
.FreeVoice:
		DEC			ECX
		JNZ			.BcTrtVoice

		PUSH		ES		    ; Copie Vers Bloc de Sortie
		CALL		GetBaseAddress
		MOV			ESI,[EBX+BuffCalc8Bit]
		MOV			ES,[EBX+SelectorDMA8]
		MOV			EDI,[EBX+OffResCalcDMA8]
		MOV			ECX,SizeDMABuff/2
		CALL		Calc8BitM_Buff8BitM
		POP			ES

		RET

VoiceNoEff8BM:
		PUSH		EBP
		PUSH		ESI
		PUSH		ECX

		CALL		GetBaseAddress

		TEST		[EAX+Voice.VcSTATE],DWORD VcStopped
		JNZ			NEAR .FinVcNEff
;************************************************************
		MOV			ECX,[EAX+Voice.Type]
		MOV			ECX,[EBX+ECX*4+SizeIncVc_BCalc8BitM]
		MOV			EDI,[EAX+Voice.Size]
		SUB			EDI,ECX
		CMP			[EAX+Voice.Pos],EDI
		JB			NEAR .PasFinVoice
.DoRemainingVoice:
;*** do mix remaining bytes of current voice **********************
		PUSH		ESI

		MOV			ESI,[EAX+Voice.Pos]
		MOV			EDI,[EAX+Voice.Size]
		SUB			EDI,ESI
		ADD			ESI,[EAX+Voice.Ptr]

		MOV			ECX,[EAX+Voice.Type]
		MOV			ECX,[EBX+ECX*4+SizeEchant]

		XCHG		EAX,EDI
		XOR			EDX,EDX
		DIV			ECX
		MOV			ECX,EAX
		PUSH		ECX
		MOV			EAX,[EDI+Voice.Type]
		;MOV			EAX,[EBX+EAX*4+AddrAdd_BCalc8BitM]
		MOV			EDI,[EBX+BuffCalc8Bit]	; Ajout Des Voix ----
		CALL		[EBX+EAX*4+AddrAdd_BCalc8BitM] ; EAX

		POP			ECX
		POP			ESI
		; if Voice not in loop ---------------------------------------------
		MOV			EAX,[ESI-4]
		TEST		[EAX+Voice.VcSTATE],DWORD VcInBoucle
		JZ			.VcPasInBcle
		; mix remaining bytes using [Voice] pointed by EAX
.VcMixRemainBytes:
		CALL		GetBaseAddress
		MOV			EBP,EDI ; save EDI

		MOV			EDI,SizeDMABuff/2
		SUB			EDI,ECX
		MOV			ECX,EDI

		PUSH		EAX
		MOV			EDX,[EAX+Voice.Type]
		MOV			ESI,[EAX+Voice.Ptr] ; Pos 0
		MOV			EDI,EBP ; restore EDI
		;OR			ECX,ECX
		;MOV		EAX,[EBX+EDX*4+AddrAdd_BCalc8BitM]
		JECXZ		.PasAddVc
		CALL		[EBX+EDX*4+AddrAdd_BCalc8BitM];EAX
.PasAddVc:
		POP			EAX
		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI

		MOV			ECX,SizeDMABuff/2
		JMP			.FinVcNEff
.VcPasInBcle:

		TEST		[EAX+Voice.VcSTATE],DWORD VcQueued
		JZ			.VcPasQueued
; current voice ended, but queued: so use queued voice instead
		CMP			DWORD [ESI-4+MAX_VOICES*4], BYTE 0 ; check QueueVoicePtr not null
		JE			.VcPasQueued
		AND 		DWORD [EAX+Voice.VcSTATE], ~(VcQueued) ; delete queued FLAG
		MOV			EAX,[ESI-4+MAX_VOICES*4] ; read QueueVoicePtr
		MOV			[ESI-4],EAX ; write to VoicePtr
		MOV			DWORD [ESI-4+MAX_VOICES*4], 0 ; clear QueueVoicePtr
		; switch to effect ?
		CMP			DWORD [EAX+Voice.Effect],BYTE 0
		JNE			VoiceEff8BM.VcMixRemainBytes
		; else keep with no effect
		JMP			SHORT .VcMixRemainBytes
.VcPasQueued:
		XOR			EAX,EAX
		MOV			[ESI-4],EAX
		JMP			.FinVcNEff
.PasFinVoice:
.IncFreeVoice:
;*** do mix current voice mix full chunk size *************************
		MOV			EDI,[EBX+BuffCalc8Bit]	; Ajout Des Voix ----
		MOV			ESI,[EAX+Voice.Ptr]
		ADD			ESI,[EAX+Voice.Pos]
		MOV			EDX,[EAX+Voice.Type]
		PUSH		EAX
		;MOV			EAX,[EBX+EDX*4+AddrAdd_BCalc8BitM]
		MOV			ECX,SizeDMABuff/2
		CALL		[EBX+EDX*4+AddrAdd_BCalc8BitM]
		POP			EAX
		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
;***********************************************************************
.FinVcNEff:
		POP			ECX
		POP			ESI
		POP			EBP

		RET

VoiceEff8BM:
		PUSH		EBP
		PUSH		ESI
		PUSH		ECX

		CALL		GetBaseAddress

		TEST		DWORD [EAX+Voice.VcSTATE],VcStopped
		JNZ			NEAR .FinVcEff
		TEST		DWORD [EAX+Voice.Effect],~ChgSpeed
		JZ			NEAR .OnlyChgSpeed
		; Chg Speed et ou autre effet -------------------------------
		;------------------------------------------------------------
		PUSH		EBX  ; Vide Buffer de calcule
		PUSH		EAX
		MOV			EDI,[EBX+EffectVcBuff]
		XOR			EAX,EAX
		MOV			ECX,SizeDMABuff/2
		REP			STOSD
		POP			EAX
		POP			EBX
		; traite changement de vitesse ------------------
		TEST		DWORD [EAX+Voice.Effect],ChgSpeed
		JNZ			NEAR .ProcChgSpeed
;****************************************************************************
;****************************************************************************
		PUSH		ESI

		MOV			ECX,[EAX+Voice.Type]
		MOV			ECX,[EBX+ECX*4+SizeIncVc_BCalc8BitM]
		MOV			EDI,[EAX+Voice.Size]
		SUB			EDI,ECX
		CMP			[EAX+Voice.Pos],EDI
		JB			NEAR .NSPasFinVoice

		PUSH		ESI

		MOV			ESI,[EAX+Voice.Pos]
		MOV			EDI,[EAX+Voice.Size]
		SUB			EDI,ESI
		ADD			ESI,[EAX+Voice.Ptr]

		MOV			ECX,[EAX+Voice.Type]
		MOV			ECX,[EBX+ECX*4+SizeEchant]

		XCHG		EAX,EDI
		XOR			EDX,EDX
		DIV			ECX
		MOV			ECX,EAX
		PUSH		ECX
		MOV			EAX,[EDI+Voice.Type]
		MOV			EAX,[EBX+EAX*4+AddrAdd_BCalc8BitM]
		MOV			EDI,[EBX+EffectVcBuff]	; Ajout Des Voix ----
		CALL		EAX

		POP			ECX
		POP			ESI
		; si SON Pas En Boucle alors Fin ----------------------------
		MOV			EAX,[ESI-4]
		TEST		[EAX+Voice.VcSTATE],DWORD VcInBoucle
		JZ			.NSVcPasInBcle
		CALL		GetBaseAddress
		PUSH		EDI

		MOV			EDI,SizeDMABuff/2
		SUB			EDI,ECX
		MOV			ECX,EDI

		MOV			EDX,[EAX+Voice.Type]
		MOV			ESI,[EAX+Voice.Ptr] ; Pos 0
		MOV			EBP,[EAX+Voice.Ptr]
		POP			EDI  ; ----------
		PUSH		EAX
		MOV			EAX,[EBX+EDX*4+AddrAdd_BCalc8BitM]
		OR			ECX,ECX
		JZ			.PasAddVc
		CALL		EAX
.PasAddVc:
		POP			EAX
		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI

		MOV			ECX,SizeDMABuff/2
		POP			ESI
		JMP			.FinPrcTrtSpd
.NSVcPasInBcle:
		XOR			EDX,EDX
		MOV			[ESI-4],EDX
		POP			ESI
		JMP			.FinPrcTrtSpd
.NSPasFinVoice:
.IncFreeVoice:
		MOV			EDI,[EBX+EffectVcBuff]	; Ajout Des Voix ----
		MOV			ESI,[EAX+Voice.Ptr]
		ADD			ESI,[EAX+Voice.Pos]
		MOV			EDX,[EAX+Voice.Type]
		PUSH		EAX
		MOV			EAX,[EBX+EDX*4+AddrAdd_BCalc8BitM]
		MOV			ECX,SizeDMABuff/2
		CALL		EAX
		POP			EAX
		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
		MOV			ECX,SizeDMABuff/2
		POP			ESI
;****************************************************************************
;****************************************************************************
		JMP			.FinPrcTrtSpd
.ProcChgSpeed:
;****************************************************************************
		PUSH		ESI
		PUSH		EAX

		PUSH		ESI
		MOV			EDI,[EBX+TempVcCopyBuff]
		MOV			EDX,[EAX+Voice.Speed]
		PUSH		EDI
		PUSH		EDX

		MOV			EDI,EAX
		MOV			EDX,EBX
		MOV			ECX,[EDI+Voice.Type]
		MOV			EAX,[EDX+ECX*4+BuffVc_SpeedBuffVc]
		MOV			EBP,[EDI+Voice.RestCoeff]
		MOV			ESI,[EDI+Voice.Pos]
		ADD			ESI,[EDI+Voice.Ptr]
		MOV			EBX,[EDI+Voice.Size]
		ADD			EBX,[EDI+Voice.Ptr]
		DEC			EBX ; Size - 1
		MOV			ECX,SizeDMABuff/2

		POP			EDX
		POP			EDI
		CALL		EAX
		POP			EDX  ; EDX = ancien Val ESI
		MOV			EAX,[EDX-4]

		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
		MOV			[EAX+Voice.RestCoeff],EBP

		MOV			EAX,[EDX-4]
		OR			ECX,ECX
		JZ			NEAR .PasFinVoice2
		; Traitement Dernier Bloc -----------------------------------
		TEST		[EAX+Voice.VcSTATE],DWORD VcInBoucle
		JZ			.VcPasInBcle2
		; Traitement Voix en Boucle --------

.VcInBcle2:
		CALL		GetBaseAddress
		MOV			EDX,[EAX+Voice.Speed]

		PUSH		EAX	; Ptr Vc
		PUSH		ECX	; ECX nb Entite restant
		PUSH		EDI	; ptr retourne par la proc precedent
		PUSH		EDX

		MOV			EDI,EAX
		MOV			EDX,EBX
		MOV			ECX,[EDI+Voice.Type]
		MOV			EAX,[EDX+ECX*4+BuffVc_SpeedBuffVc]
		XOR			EBP,EBP
		MOV			ESI,[EDI+Voice.Ptr] ; Pos = 0
		MOV			EBX,[EDI+Voice.Size]
		ADD			EBX,[EDI+Voice.Ptr]
		DEC			EBX ; Size - 1

		POP			EDX
		POP			EDI
		POP			ECX

		CALL		EAX
		POP			EAX
		OR			ECX,ECX
		JNZ			.VcInBcle2

		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
		MOV			[EAX+Voice.RestCoeff],EBP

		JMP			SHORT .PasFinVoice2
.VcPasInBcle2:
		MOV			EBP,ECX
		CALL		GetBaseAddress
		MOV			EDI,[EBX+EffectVcBuff]	; Ajout Des Voix ----
		MOV			ESI,[EBX+TempVcCopyBuff]
		MOV			ECX,[EAX+Voice.Type]
		MOV			EAX,[EBX+ECX*4+AddrAdd_BCalc8BitM]
		MOV			ECX,SizeDMABuff/2
		SUB			ECX,EBP
		PUSH		ECX
		CALL		EAX

		XOR			EBX,EBX
		MOV			[EDX-4],EBX
		POP			ECX

		POP			EAX
		POP			ESI
		JMP			SHORT .FinPrcTrtSpd
.PasFinVoice2:
		CALL		GetBaseAddress
		MOV			EDI,[EBX+EffectVcBuff]	; Ajout Des Voix ----
		MOV			ESI,[EBX+TempVcCopyBuff]
		MOV			ECX,[EAX+Voice.Type]
		MOV			EAX,[EBX+ECX*4+AddrAdd_BCalc8BitM]
		MOV			ECX,SizeDMABuff/2
		CALL		EAX
		MOV			ECX,SizeDMABuff/2

		POP			EAX
		POP			ESI
;****************************************************************************
.FinPrcTrtSpd:
		TEST		DWORD [EAX+Voice.Effect],ChgVol
		JZ			.PasChgVol
		;--- Traite changement de volume ----------------------------
		PUSH		EAX
		PUSH		ECX

		CALL		GetBaseAddress
		MOV			ESI,[EBX+EffectVcBuff]
		; ECX contient deja Nb Ent
		MOV			EDX,[EAX+Voice.LeftVol]
		MOV			EDI,[EAX+Voice.RightVol]
		CALL		ChgVolBuffCalcM

		POP			ECX
		POP			EAX
		;--- Fin traite Chg Vol -------------------------------------
.PasChgVol:
		; Ajoute Buffer d'effet  au buffer de calcul ----------------
		CALL		GetBaseAddress
		MOV			ESI,[EBX+EffectVcBuff]
		MOV			EDI,[EBX+BuffCalc8Bit]
		;ECX contient deja le Nb d'entite

.BcAddBuffEff:
		LODSD
		ADD			[EDI],EAX
		DEC			ECX
		LEA			EDI,[EDI+4]
		JNZ			.BcAddBuffEff

		JMP			.FinVcEff
.OnlyChgSpeed:	; uniquement Chg Speed --------------------------------------
		;------------------------------------------------------------
		PUSH		ESI
		MOV			EDI,[EBX+TempVcCopyBuff]
		MOV			EDX,[EAX+Voice.Speed]
		PUSH		EDI
		PUSH		EDX

		MOV			EDI,EAX
		MOV			EDX,EBX
		MOV			ECX,[EDI+Voice.Type]
		MOV			EAX,[EDX+ECX*4+BuffVc_SpeedBuffVc]
		MOV			EBP,[EDI+Voice.RestCoeff]
		MOV			ESI,[EDI+Voice.Pos]
		ADD			ESI,[EDI+Voice.Ptr]
		MOV			EBX,[EDI+Voice.Size]
		ADD			EBX,[EDI+Voice.Ptr]
		DEC			EBX ; Size - 1
		MOV			ECX,SizeDMABuff/2

		POP			EDX
		POP			EDI
		CALL		EAX
		POP			EDX  ; ancien Val ESI
		MOV			EAX,[EDX-4]

		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
		MOV			[EAX+Voice.RestCoeff],EBP

		MOV			EAX,[EDX-4]
		OR			ECX,ECX
		MOV			ESI,EDX ; restore ESI as VoicePtr
		JZ			.PasFinVoice1
		; Traitement Dernier Bloc -----------------------------------
		TEST		[EAX+Voice.VcSTATE],DWORD VcInBoucle
		JZ			.VcPasInBcle
		; Traitement Voix en Boucle --------

.VcInBcle:
.VcMixRemainBytes:
;*** do mix remaining bytes of current voice **********************
		CALL		GetBaseAddress
		MOV			EDX,[EAX+Voice.Speed]

		PUSH		EAX	; Ptr Vc
		PUSH		ECX	; ECX nb Entite restant
		PUSH		EDI	; ptr retourne par la proc precedent
		PUSH		EDX

		MOV			EDI,EAX
		MOV			EDX,EBX
		MOV			ECX,[EDI+Voice.Type]
		MOV			EAX,[EDX+ECX*4+BuffVc_SpeedBuffVc]
		XOR			EBP,EBP
		MOV			ESI,[EDI+Voice.Ptr] ; Pos = 0
		MOV			EBX,[EDI+Voice.Size]
		ADD			EBX,[EDI+Voice.Ptr]
		DEC			EBX ; Size - 1

		POP			EDX
		POP			EDI
		POP			ECX

		CALL		EAX
		POP			EAX


		; test again looping as this could be a new queued Voice not looping
		TEST		[EAX+Voice.VcSTATE],DWORD VcInBoucle
		JZ			.FinVcEff ; bypass queued check as this happen only for new Queued voice

		OR			ECX,ECX
		JNZ			.VcInBcle

		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
		MOV			[EAX+Voice.RestCoeff],EBP

		JMP			SHORT .PasFinVoice1
.VcPasInBcle:
; check if this voice is queued
		TEST		[EAX+Voice.VcSTATE],DWORD VcQueued
		JZ			.VcPasQueued
; current voice ended, but queued: so use queued voice instead
		CMP			DWORD [ESI-4+MAX_VOICES*4], BYTE 0 ; check QueueVoicePtr not null
		JE			.VcPasQueued
		AND 		DWORD [EAX+Voice.VcSTATE], ~(VcQueued) ; delete queued FLAG
		MOV			EAX,[ESI-4+MAX_VOICES*4] ; read QueueVoicePtr
		MOV			[ESI-4],EAX ; write to VoicePtr
		MOV			DWORD [ESI-4+MAX_VOICES*4], 0 ; clear QueueVoicePtr
		; switch to no effect ?
		CMP			DWORD [EAX+Voice.Effect],BYTE 0
		JE			VoiceNoEff8BM.VcMixRemainBytes
		; else keep with effect
		JMP			SHORT .VcMixRemainBytes
.VcPasQueued:
		JECXZ		.FinVcEff

		MOV			EBP,ECX
		CALL		GetBaseAddress
		MOV			EDI,[EBX+BuffCalc8Bit]	; Ajout Des Voix ----
		MOV			ESI,[EBX+TempVcCopyBuff]
		MOV			ECX,[EAX+Voice.Type]
		MOV			EAX,[EBX+ECX*4+AddrAdd_BCalc8BitM]
		MOV			ECX,SizeDMABuff/2
		SUB			ECX,EBP
		CALL		EAX

		XOR			EBX,EBX
		MOV			[EDX-4],EBX
		JMP			SHORT .FinVcEff
.PasFinVoice1:
		CALL		GetBaseAddress
		MOV			EDI,[EBX+BuffCalc8Bit]	; Ajout Des Voix ----
		MOV			ESI,[EBX+TempVcCopyBuff]
		MOV			ECX,[EAX+Voice.Type]
		MOV			EAX,[EBX+ECX*4+AddrAdd_BCalc8BitM]
		MOV			ECX,SizeDMABuff/2
		CALL		EAX
.FinVcEff:
		POP			ECX
		POP			ESI
		POP			EBP

		RET


;******* PROCED SORTIE 8 Bits STEREO
ProcedOut8BitsS:
		CALL		GetBaseAddress

		XOR			EAX,EAX 		; zero Buff Calc ----
		MOV			EDI,[EBX+BuffCalc8Bit]
		MOV			ECX,SizeDMABuff/2
		REP			STOSD

		MOV			ECX,[EBX+MaxNbVoice]
		LEA			ESI,[EBX+VoicePtr]
.BcTrtVoice:
		LODSD
		OR			EAX,EAX
		JZ			.FreeVoice
		CMP			DWORD [EAX+Voice.Effect],BYTE 0
		JNZ			.VoiceEff
		CALL		VoiceNoEff8BS
		JMP			SHORT .VoiceNEff
.VoiceEff:
		CALL		VoiceEff8BS
.VoiceNEff:
.FreeVoice:
		DEC			ECX
		JNZ			.BcTrtVoice

		PUSH		ES		    ; Copie Vers Bloc de Sortie
		CALL		GetBaseAddress
		MOV			ESI,[EBX+BuffCalc8Bit]
		MOV			ES,[EBX+SelectorDMA8]
		MOV			EDI,[EBX+OffResCalcDMA8]
		MOV			ECX,SizeDMABuff/4
		CALL		Calc8BitS_Buff8BitS
		POP			ES

		RET

VoiceNoEff8BS:
		PUSH		EBP
		PUSH		ESI
		PUSH		ECX

		CALL		GetBaseAddress

		TEST		[EAX+Voice.VcSTATE],DWORD VcStopped
		JNZ			NEAR .FinVcNEff

;************************************************************
		MOV			ECX,[EAX+Voice.Type]
		MOV			ECX,[EBX+ECX*4+SizeIncVc_BCalc8BitS]
		MOV			EDI,[EAX+Voice.Size]
		SUB			EDI,ECX
		CMP			[EAX+Voice.Pos],EDI
		JB			NEAR .PasFinVoice

		PUSH		ESI

		MOV			ESI,[EAX+Voice.Pos]
		MOV			EDI,[EAX+Voice.Size]
		SUB			EDI,ESI
		ADD			ESI,[EAX+Voice.Ptr]

		MOV			ECX,[EAX+Voice.Type]
		MOV			ECX,[EBX+ECX*4+SizeEchant]

		XCHG		EAX,EDI
		XOR			EDX,EDX
		DIV			ECX
		MOV			ECX,EAX
		PUSH		ECX
		MOV			EAX,[EDI+Voice.Type]
		;MOV			EAX,[EBX+EAX*4+AddrAdd_BCalc8BitS]
		MOV			EDI,[EBX+BuffCalc8Bit]	; Ajout Des Voix ----
		CALL		[EBX+EAX*4+AddrAdd_BCalc8BitS] ;EAX

		POP			ECX
		POP			ESI
		; si SON Pas En Boucle alors Fin ----------------------------
		MOV			EAX,[ESI-4]
		TEST		[EAX+Voice.VcSTATE],DWORD VcInBoucle
		JZ			.VcPasInBcle
		; mix remaining bytes using [Voice] pointed by EAX
.VcMixRemainBytes:

		CALL		GetBaseAddress
		MOV			EBP,EDI ; save EDI

		MOV			EDI,SizeDMABuff/4
		SUB			EDI,ECX
		MOV			ECX,EDI

		MOV			EDX,[EAX+Voice.Type]
		MOV			ESI,[EAX+Voice.Ptr] ; Pos 0
		MOV			EDI,EBP ; restore EDI
		PUSH		EAX
		;MOV			EAX,[EBX+EDX*4+AddrAdd_BCalc8BitS]
		;OR			ECX,ECX
		JECXZ		.PasAddVc
		CALL		[EBX+EDX*4+AddrAdd_BCalc8BitS]
.PasAddVc:
		POP			EAX
		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI

		MOV			ECX,SizeDMABuff/4
		JMP			.FinVcNEff
.VcPasInBcle:
		TEST		[EAX+Voice.VcSTATE],DWORD VcQueued
		JZ			.VcPasQueued
; current voice ended, but queued: so use queued voice instead
		CMP			DWORD [ESI-4+MAX_VOICES*4], BYTE 0 ; check QueueVoicePtr not null
		JE			.VcPasQueued
		AND 		DWORD [EAX+Voice.VcSTATE], ~(VcQueued) ; delete queued FLAG
		MOV			EAX,[ESI-4+MAX_VOICES*4] ; read QueueVoicePtr
		MOV			[ESI-4],EAX ; write to VoicePtr
		MOV			DWORD [ESI-4+MAX_VOICES*4], 0 ; clear QueueVoicePtr
		; switch to effect ?
		CMP			DWORD [EAX+Voice.Effect],BYTE 0
		JNE			VoiceEff8BS.VcMixRemainBytes
		; else keep with no effect
		JMP			SHORT .VcMixRemainBytes
.VcPasQueued:

		XOR			EAX,EAX
		MOV			[ESI-4],EAX
		JMP			.FinVcNEff
.PasFinVoice:
.IncFreeVoice:
		MOV			EDI,[EBX+BuffCalc8Bit]	; Ajout Des Voix ----
		MOV			ESI,[EAX+Voice.Ptr]
		ADD			ESI,[EAX+Voice.Pos]
		MOV			EDX,[EAX+Voice.Type]
		PUSH		EAX
		;MOV			EAX,[EBX+EDX*4+AddrAdd_BCalc8BitS]
		MOV			ECX,SizeDMABuff/4
		CALL		[EBX+EDX*4+AddrAdd_BCalc8BitS];EAX
		POP			EAX
		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
;***********************************************************************
.FinVcNEff:
		POP			ECX
		POP			ESI
		POP			EBP

		RET

VoiceEff8BS:
		PUSH		EBP
		PUSH		ESI
		PUSH		ECX

		CALL		GetBaseAddress

		TEST		[EAX+Voice.VcSTATE],DWORD VcStopped
		JNZ			NEAR .FinVcEff
		TEST		DWORD [EAX+Voice.Effect],~ChgSpeed
		JZ			NEAR .OnlyChgSpeed
		; Chg Speed et ou autre effet -------------------------------

		PUSH		EBX  ; Vide Buffer de calcule
		PUSH		EAX
		MOV			EDI,[EBX+EffectVcBuff]
		XOR			EAX,EAX
		MOV			ECX,SizeDMABuff/2
		REP			STOSD
		POP			EAX
		POP			EBX
		; traite changement de vitesse ------------------
		TEST		DWORD [EAX+Voice.Effect],ChgSpeed
		JNZ			NEAR .ProcChgSpeed
;****************************************************************************
;****************************************************************************
		PUSH		ESI

		MOV			ECX,[EAX+Voice.Type]
		MOV			ECX,[EBX+ECX*4+SizeIncVc_BCalc8BitS]
		MOV			EDI,[EAX+Voice.Size]
		SUB			EDI,ECX
		CMP			[EAX+Voice.Pos],EDI
		JB			NEAR .NSPasFinVoice

		PUSH		ESI

		MOV			ESI,[EAX+Voice.Pos]
		MOV			EDI,[EAX+Voice.Size]
		SUB			EDI,ESI
		ADD			ESI,[EAX+Voice.Ptr]

		MOV			ECX,[EAX+Voice.Type]
		MOV			ECX,[EBX+ECX*4+SizeEchant]

		XCHG		EAX,EDI
		XOR			EDX,EDX
		DIV			ECX
		MOV			ECX,EAX
		PUSH		ECX
		MOV			EAX,[EDI+Voice.Type]
		MOV			EAX,[EBX+EAX*4+AddrAdd_BCalc8BitS]
		MOV			EDI,[EBX+EffectVcBuff]	; Ajout Des Voix ----
		CALL		EAX

		POP			ECX
		POP			ESI
		; si SON Pas En Boucle alors Fin ----------------------------
		MOV			EAX,[ESI-4]
		TEST		[EAX+Voice.VcSTATE],DWORD VcInBoucle
		JZ			.NSVcPasInBcle
		CALL		GetBaseAddress
		PUSH		EDI

		MOV			EDI,SizeDMABuff/4
		SUB			EDI,ECX
		MOV			ECX,EDI

		MOV			EDX,[EAX+Voice.Type]
		MOV			ESI,[EAX+Voice.Ptr] ; Pos 0
		MOV			EBP,[EAX+Voice.Ptr]
		POP			EDI  ; ----------
		PUSH		EAX
		MOV			EAX,[EBX+EDX*4+AddrAdd_BCalc8BitS]
		OR			ECX,ECX
		JZ			.PasAddVc
		CALL		EAX
.PasAddVc:
		POP			EAX
		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI

		MOV			ECX,SizeDMABuff/4
		POP			ESI

		JMP			.FinPrcTrtSpd
.NSVcPasInBcle:
		XOR			EAX,EAX
		MOV			[ESI-4],EAX
		POP			ESI
		JMP			.FinPrcTrtSpd
.NSPasFinVoice:
.IncFreeVoice:
		MOV			EDI,[EBX+EffectVcBuff]	; Ajout Des Voix ----
		MOV			ESI,[EAX+Voice.Ptr]
		ADD			ESI,[EAX+Voice.Pos]
		MOV			EDX,[EAX+Voice.Type]
		PUSH		EAX
		MOV			EAX,[EBX+EDX*4+AddrAdd_BCalc8BitS]
		MOV			ECX,SizeDMABuff/4
		CALL		EAX
		POP			EAX
		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
		MOV			ECX,SizeDMABuff/4
		POP			ESI

;****************************************************************************
;****************************************************************************
		JMP			.FinPrcTrtSpd
.ProcChgSpeed:
;****************************************************************************
		PUSH		ESI
		PUSH		EAX

		PUSH		ESI
		MOV			EDI,[EBX+TempVcCopyBuff]
		MOV			EDX,[EAX+Voice.Speed]
		PUSH		EDI
		PUSH		EDX

		MOV			EDI,EAX
		MOV			EDX,EBX
		MOV			ECX,[EDI+Voice.Type]
		MOV			EAX,[EDX+ECX*4+BuffVc_SpeedBuffVc]
		MOV			EBP,[EDI+Voice.RestCoeff]
		MOV			ESI,[EDI+Voice.Pos]
		ADD			ESI,[EDI+Voice.Ptr]
		MOV			EBX,[EDI+Voice.Size]
		ADD			EBX,[EDI+Voice.Ptr]
		DEC			EBX ; Size - 1
		MOV			ECX,SizeDMABuff/4

		POP			EDX
		POP			EDI
		CALL		EAX
		POP			EDX  ; EDX = ancien Val ESI
		MOV			EAX,[EDX-4]

		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
		MOV			[EAX+Voice.RestCoeff],EBP

		MOV			EAX,[EDX-4]
		OR			ECX,ECX
		JZ			NEAR .PasFinVoice2
		; Traitement Dernier Bloc -----------------------------------
		TEST		[EAX+Voice.VcSTATE],DWORD VcInBoucle
		JZ			.VcPasInBcle2
		; Traitement Voix en Boucle --------

.VcInBcle2:
		CALL		GetBaseAddress
		MOV			EDX,[EAX+Voice.Speed]

		PUSH		EAX	; Ptr Vc
		PUSH		ECX	; ECX nb Entite restant
		PUSH		EDI	; ptr retourne par la proc precedent
		PUSH		EDX

		MOV			EDI,EAX
		MOV			EDX,EBX
		MOV			ECX,[EDI+Voice.Type]
		MOV			EAX,[EDX+ECX*4+BuffVc_SpeedBuffVc]
		XOR			EBP,EBP
		MOV			ESI,[EDI+Voice.Ptr] ; Pos = 0
		MOV			EBX,[EDI+Voice.Size]
		ADD			EBX,[EDI+Voice.Ptr]
		DEC			EBX ; Size - 1

		POP			EDX
		POP			EDI
		POP			ECX

		CALL		EAX
		POP			EAX
		OR			ECX,ECX
		JNZ			.VcInBcle2

		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
		MOV			[EAX+Voice.RestCoeff],EBP

		JMP			SHORT .PasFinVoice2
.VcPasInBcle2:
		MOV			EBP,ECX
		CALL		GetBaseAddress
		MOV			EDI,[EBX+EffectVcBuff]	; Ajout Des Voix ----
		MOV			ESI,[EBX+TempVcCopyBuff]
		MOV			ECX,[EAX+Voice.Type]
		MOV			EAX,[EBX+ECX*4+AddrAdd_BCalc8BitS]
		MOV			ECX,SizeDMABuff/4
		SUB			ECX,EBP
		PUSH		ECX
		CALL		EAX

		XOR			EBX,EBX
		MOV			[EDX-4],EBX
		POP			ECX

		POP			EAX
		POP			ESI
		JMP			SHORT .FinPrcTrtSpd
.PasFinVoice2:
		CALL		GetBaseAddress
		MOV			EDI,[EBX+EffectVcBuff]	; Ajout Des Voix ----
		MOV			ESI,[EBX+TempVcCopyBuff]
		MOV			ECX,[EAX+Voice.Type]
		MOV			EAX,[EBX+ECX*4+AddrAdd_BCalc8BitS]
		MOV			ECX,SizeDMABuff/4
		CALL		EAX
		MOV			ECX,SizeDMABuff/4

		POP			EAX
		POP			ESI
;****************************************************************************
.FinPrcTrtSpd:
		TEST		DWORD [EAX+Voice.Effect],ChgVol
		JZ			.PasChgVol
		;--- Traite changement de volume ----------------------------
		PUSH		EAX
		PUSH		ECX

		CALL		GetBaseAddress
		MOV			ESI,[EBX+EffectVcBuff]
		; ECX contient deja Nb Ent
		MOV			EDX,[EAX+Voice.LeftVol]
		MOV			EDI,[EAX+Voice.RightVol]
		CALL		ChgVolBuffCalcS

		POP			ECX
		POP			EAX
		;--- Fin traite Chg Vol -------------------------------------
.PasChgVol:
		; Ajoute Buffer d'effet  au buffer de calcul ----------------
		CALL		GetBaseAddress
		MOV			ESI,[EBX+EffectVcBuff]
		MOV			EDI,[EBX+BuffCalc8Bit]
		;ECX contient deja le Nb d'entite
		ADD			ECX,ECX ; puisque STEREO
.BcAddBuffEff:
		LODSD
		ADD			[EDI],EAX
		DEC			ECX
		LEA			EDI,[EDI+4]
		JNZ			.BcAddBuffEff


		JMP			.FinVcEff
.OnlyChgSpeed:	; uniquement Chg Speed --------------------------------------

		PUSH		ESI
		MOV			EDI,[EBX+TempVcCopyBuff]
		MOV			EDX,[EAX+Voice.Speed]
		PUSH		EDI
		PUSH		EDX

		MOV			EDI,EAX
		MOV			EDX,EBX
		MOV			ECX,[EDI+Voice.Type]
		MOV			EAX,[EDX+ECX*4+BuffVc_SpeedBuffVc]
		MOV			EBP,[EDI+Voice.RestCoeff]
		MOV			ESI,[EDI+Voice.Pos]
		ADD			ESI,[EDI+Voice.Ptr]
		MOV			EBX,[EDI+Voice.Size]
		ADD			EBX,[EDI+Voice.Ptr]
		DEC			EBX ; Size - 1
		MOV			ECX,SizeDMABuff/4

		POP			EDX
		POP			EDI
		CALL		EAX
		POP			EDX  ; ancien Val ESI
		MOV			EAX,[EDX-4]

		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
		MOV			[EAX+Voice.RestCoeff],EBP

		MOV			EAX,[EDX-4]
		OR			ECX,ECX
		MOV			ESI,EDX ; restore ESI as VoicePtr
		JZ			NEAR .PasFinVoice1
		; Traitement Dernier Bloc -----------------------------------
		TEST		[EAX+Voice.VcSTATE],DWORD VcInBoucle
		JZ			.VcPasInBcle
		; Traitement Voix en Boucle --------

.VcInBcle:
.VcMixRemainBytes:
;*** do mix remaining bytes of current voice **********************
		CALL		GetBaseAddress
		MOV			EDX,[EAX+Voice.Speed]

		PUSH		EAX	; Ptr Vc
		PUSH		ECX	; ECX nb Entite restant
		PUSH		EDI	; ptr retourne par la proc precedent
		PUSH		EDX

		MOV			EDI,EAX
		MOV			EDX,EBX
		MOV			ECX,[EDI+Voice.Type]
		MOV			EAX,[EDX+ECX*4+BuffVc_SpeedBuffVc]
		XOR			EBP,EBP
		MOV			ESI,[EDI+Voice.Ptr] ; Pos = 0
		MOV			EBX,[EDI+Voice.Size]
		ADD			EBX,[EDI+Voice.Ptr]
		DEC			EBX ; Size - 1

		POP			EDX
		POP			EDI
		POP			ECX
		CALL		EAX
		POP			EAX

		; test again looping as this could be a new queued Voice not looping
		TEST		[EAX+Voice.VcSTATE],DWORD VcInBoucle
		JZ			.FinVcEff ; bypass queued check as this happen only for new Queued voice

		OR			ECX,ECX
		JNZ			.VcInBcle

		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
		MOV			[EAX+Voice.RestCoeff],EBP

		JMP			SHORT .PasFinVoice1
.VcPasInBcle:
; check if this voice is queued
		TEST		[EAX+Voice.VcSTATE],DWORD VcQueued
		JZ			.VcPasQueued
; current voice ended, but queued: so use queued voice instead
		CMP			DWORD [ESI-4+MAX_VOICES*4], BYTE 0 ; check QueueVoicePtr not null
		JE			.VcPasQueued
		AND 		DWORD [EAX+Voice.VcSTATE], ~(VcQueued) ; delete queued FLAG
		MOV			EAX,[ESI-4+MAX_VOICES*4] ; read QueueVoicePtr
		MOV			[ESI-4],EAX ; write to VoicePtr
		MOV			DWORD [ESI-4+MAX_VOICES*4], 0 ; clear QueueVoicePtr
		; switch to no effect ?
		CMP			DWORD [EAX+Voice.Effect],BYTE 0
		JE			VoiceNoEff8BS.VcMixRemainBytes
		; else keep with effect
		JMP			SHORT .VcMixRemainBytes
.VcPasQueued:
		JECXZ		.FinVcEff

		MOV			EBP,ECX
		CALL		GetBaseAddress
		MOV			EDI,[EBX+BuffCalc8Bit]	; Ajout Des Voix ----
		MOV			ESI,[EBX+TempVcCopyBuff]
		MOV			ECX,[EAX+Voice.Type]
		MOV			EAX,[EBX+ECX*4+AddrAdd_BCalc8BitS]
		MOV			ECX,SizeDMABuff/4
		SUB			ECX,EBP
		CALL		EAX


		XOR			EBX,EBX
		MOV			[EDX-4],EBX
		JMP			SHORT .FinVcEff
.PasFinVoice1:
		CALL		GetBaseAddress
		MOV			EDI,[EBX+BuffCalc8Bit]	; Ajout Des Voix ----
		MOV			ESI,[EBX+TempVcCopyBuff]
		MOV			ECX,[EAX+Voice.Type]
		MOV			EAX,[EBX+ECX*4+AddrAdd_BCalc8BitS]
		MOV			ECX,SizeDMABuff/4
		CALL		EAX
.FinVcEff:
		POP			ECX
		POP			ESI
		POP			EBP

		RET

;******* PROCED SORTIE 16 Bits MONO
ProcedOut16BitsM:
		CALL		GetBaseAddress

		XOR			EAX,EAX 		; zero Buff Calc ----
		MOV			EDI,[EBX+BuffCalc16Bit]
		MOV			ECX,SizeDMABuff/2
		REP			STOSD

		MOV			ECX,[EBX+MaxNbVoice]
		LEA			ESI,[EBX+VoicePtr]
.BcTrtVoice:
		LODSD
		OR			EAX,EAX
		JZ			.FreeVoice
		CMP			DWORD [EAX+Voice.Effect],BYTE 0
		JNZ			.VoiceEff
		CALL		VoiceNoEff16BM
		JMP			SHORT .VoiceNEff
.VoiceEff:
		CALL		VoiceEff16BM
.VoiceNEff:
.FreeVoice:
		DEC			ECX
		JNZ			.BcTrtVoice

		PUSH		ES		    ; Copie Vers Bloc de Sortie
		CALL		GetBaseAddress
		MOV			ESI,[EBX+BuffCalc16Bit]
		MOV			ES,[EBX+SelectorDMA16]
		MOV			EDI,[EBX+OffResCalcDMA16]
		MOV			ECX,SizeDMABuff/2
		CALL		Calc16BitM_Buff16BitM
		POP			ES

		RET

VoiceNoEff16BM:
		PUSH		EBP
		PUSH		ESI
		PUSH		ECX


		CALL		GetBaseAddress

		TEST		[EAX+Voice.VcSTATE],DWORD VcStopped
		JNZ			NEAR .FinVcNEff

;************************************************************
		MOV			ECX,[EAX+Voice.Type]
		MOV			ECX,[EBX+ECX*4+SizeIncVc_BCalc16BitM]
		MOV			EDI,[EAX+Voice.Size]
		SUB			EDI,ECX
		CMP			[EAX+Voice.Pos],EDI
		JB			NEAR .PasFinVoice
.DoRemainingVoice:
;*** do mix remaining bytes of current voice **********************

		PUSH		ESI

		MOV			ESI,[EAX+Voice.Pos]
		MOV			EDI,[EAX+Voice.Size]
		SUB			EDI,ESI
		ADD			ESI,[EAX+Voice.Ptr]

		MOV			ECX,[EAX+Voice.Type]
		MOV			ECX,[EBX+ECX*4+SizeEchant]

		XCHG		EAX,EDI
		XOR			EDX,EDX
		DIV			ECX
		MOV			ECX,EAX
		PUSH		ECX
		MOV			EAX,[EDI+Voice.Type]
		;MOV		EAX,[EBX+EAX*4+AddrAdd_BCalc16BitM]
		MOV			EDI,[EBX+BuffCalc16Bit]	; Ajout Des Voix ----
		CALL		[EBX+EAX*4+AddrAdd_BCalc16BitM];EAX

		POP			ECX
		POP			ESI
		; si SON Pas En Boucle alors Fin ----------------------------
		MOV			EAX,[ESI-4]
		TEST		[EAX+Voice.VcSTATE],DWORD VcInBoucle
		JZ			.VcPasInBcle
		; mix remaining bytes using [Voice] pointed by EAX
.VcMixRemainBytes:
		CALL		GetBaseAddress
		MOV			EBP,EDI ; save EDI

		MOV			EDI,SizeDMABuff/2
		SUB			EDI,ECX
		MOV			ECX,EDI

		PUSH		EAX
		MOV			EDX,[EAX+Voice.Type]
		MOV			ESI,[EAX+Voice.Ptr] ; Pos 0
		MOV			EDI,EBP ; restore EDI
		;MOV		EAX,[EBX+EDX*4+AddrAdd_BCalc16BitM]
		;OR		ECX,ECX
		JECXZ		.PasAddVc
		CALL		[EBX+EDX*4+AddrAdd_BCalc16BitM]
.PasAddVc:
		POP			EAX
		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI

		MOV			ECX,SizeDMABuff/2
		JMP			.FinVcNEff
.VcPasInBcle:
		TEST		[EAX+Voice.VcSTATE],DWORD VcQueued
		JZ			.VcPasQueued
; current voice ended, but queued: so use queued voice instead
		CMP			DWORD [ESI-4+MAX_VOICES*4], BYTE 0 ; check QueueVoicePtr not null
		JE			.VcPasQueued
		AND 		DWORD [EAX+Voice.VcSTATE], ~(VcQueued) ; delete queued FLAG
		MOV			EAX,[ESI-4+MAX_VOICES*4] ; read QueueVoicePtr
		MOV			[ESI-4],EAX ; write to VoicePtr
		MOV			DWORD [ESI-4+MAX_VOICES*4], 0 ; clear QueueVoicePtr
		; switch to effect ?
		CMP			DWORD [EAX+Voice.Effect],BYTE 0
		JNE			VoiceEff16BM.VcMixRemainBytes
		; else keep with no effect
		JMP			SHORT .VcMixRemainBytes
.VcPasQueued:

		XOR			EAX,EAX
		MOV			[ESI-4],EAX
		JMP			.FinVcNEff
.PasFinVoice:
.IncFreeVoice:
		MOV			EDI,[EBX+BuffCalc16Bit]	; Ajout Des Voix ----
		MOV			ESI,[EAX+Voice.Ptr]
		ADD			ESI,[EAX+Voice.Pos]
		MOV			EDX,[EAX+Voice.Type]
		PUSH		EAX
		;MOV			EAX,[EBX+EDX*4+AddrAdd_BCalc16BitM]
		MOV			ECX,SizeDMABuff/2
		CALL		[EBX+EDX*4+AddrAdd_BCalc16BitM]
		POP			EAX
		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
;***********************************************************************

.FinVcNEff:
		POP			ECX
		POP			ESI
		POP			EBP

		RET

VoiceEff16BM:
		PUSH		EBP
		PUSH		ESI
		PUSH		ECX

		CALL		GetBaseAddress

		TEST		[EAX+Voice.VcSTATE],DWORD VcStopped
		JNZ			NEAR .FinVcEff
		TEST		DWORD [EAX+Voice.Effect],~ChgSpeed
		JZ			NEAR .OnlyChgSpeed
		; Chg Speed et ou autre effet -------------------------------

		PUSH		EBX  ; Vide Buffer de calcule
		PUSH		EAX
		MOV			EDI,[EBX+EffectVcBuff]
		XOR			EAX,EAX
		MOV			ECX,SizeDMABuff/2
		REP			STOSD
		POP			EAX
		POP			EBX
		; traite changement de vitesse ------------------
		TEST		DWORD [EAX+Voice.Effect],ChgSpeed
		JNZ			NEAR .ProcChgSpeed
;****************************************************************************
;****************************************************************************

		PUSH		ESI
		MOV			ECX,[EAX+Voice.Type]
		MOV			ECX,[EBX+ECX*4+SizeIncVc_BCalc16BitM]
		MOV			EDI,[EAX+Voice.Size]
		SUB			EDI,ECX
		CMP			[EAX+Voice.Pos],EDI
		JB			NEAR .NSPasFinVoice

		PUSH		ESI

		MOV			ESI,[EAX+Voice.Pos]
		MOV			EDI,[EAX+Voice.Size]
		SUB			EDI,ESI
		ADD			ESI,[EAX+Voice.Ptr]

		MOV			ECX,[EAX+Voice.Type]
		MOV			ECX,[EBX+ECX*4+SizeEchant]

		XCHG		EAX,EDI
		XOR			EDX,EDX
		DIV			ECX
		MOV			ECX,EAX
		PUSH		ECX
		MOV			EAX,[EDI+Voice.Type]
		MOV			EAX,[EBX+EAX*4+AddrAdd_BCalc16BitM]
		MOV			EDI,[EBX+EffectVcBuff]	; Ajout Des Voix ----
		CALL		EAX

		POP			ECX
		POP			ESI
		; si SON Pas En Boucle alors Fin ----------------------------
		MOV			EAX,[ESI-4]
		TEST		[EAX+Voice.VcSTATE],DWORD VcInBoucle
		JZ			.NSVcPasInBcle
		CALL		GetBaseAddress
		PUSH		EDI

		MOV			EDI,SizeDMABuff/2
		SUB			EDI,ECX
		MOV			ECX,EDI

		MOV			EDX,[EAX+Voice.Type]
		MOV			ESI,[EAX+Voice.Ptr] ; Pos 0
		MOV			EBP,[EAX+Voice.Ptr]
		POP			EDI  ; ----------
		PUSH		EAX
		MOV			EAX,[EBX+EDX*4+AddrAdd_BCalc16BitM]
		OR			ECX,ECX
		JZ			.PasAddVc
		CALL		EAX
.PasAddVc:
		POP			EAX
		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI

		MOV			ECX,SizeDMABuff/2
		POP			ESI

		JMP			.FinPrcTrtSpd
.NSVcPasInBcle:
		XOR			EAX,EAX
		MOV			[ESI-4],EAX
		POP			ESI
		JMP			.FinPrcTrtSpd
.NSPasFinVoice:
.IncFreeVoice:
		MOV			EDI,[EBX+EffectVcBuff]	; Ajout Des Voix ----
		MOV			ESI,[EAX+Voice.Ptr]
		ADD			ESI,[EAX+Voice.Pos]
		MOV			EDX,[EAX+Voice.Type]
		PUSH		EAX
		MOV			EAX,[EBX+EDX*4+AddrAdd_BCalc16BitM]
		MOV			ECX,SizeDMABuff/2
		CALL		EAX
		POP			EAX
		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
		MOV			ECX,SizeDMABuff/2
		POP			ESI
;****************************************************************************
;****************************************************************************
		JMP			.FinPrcTrtSpd
.ProcChgSpeed:
;****************************************************************************
		PUSH		ESI
		PUSH		EAX

		PUSH		ESI
		MOV			EDI,[EBX+TempVcCopyBuff]
		MOV			EDX,[EAX+Voice.Speed]
		PUSH		EDI
		PUSH		EDX

		MOV			EDI,EAX
		MOV			EDX,EBX
		MOV			ECX,[EDI+Voice.Type]
		MOV			EAX,[EDX+ECX*4+BuffVc_SpeedBuffVc]
		MOV			EBP,[EDI+Voice.RestCoeff]
		MOV			ESI,[EDI+Voice.Pos]
		ADD			ESI,[EDI+Voice.Ptr]
		MOV			EBX,[EDI+Voice.Size]
		ADD			EBX,[EDI+Voice.Ptr]
		DEC			EBX ; Size - 1
		MOV			ECX,SizeDMABuff/2

		POP			EDX
		POP			EDI
		CALL		EAX
		POP			EDX  ; EDX = ancien Val ESI
		MOV			EAX,[EDX-4]

		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
		MOV			[EAX+Voice.RestCoeff],EBP

		MOV			EAX,[EDX-4]
		OR			ECX,ECX
		JZ			NEAR .PasFinVoice2
		; Traitement Dernier Bloc -----------------------------------
		TEST		[EAX+Voice.VcSTATE],DWORD VcInBoucle
		JZ			.VcPasInBcle2
		; Traitement Voix en Boucle --------

.VcInBcle2:
		CALL		GetBaseAddress
		MOV			EDX,[EAX+Voice.Speed]

		PUSH		EAX	; Ptr Vc
		PUSH		ECX	; ECX nb Entite restant
		PUSH		EDI	; ptr retourne par la proc precedent
		PUSH		EDX

		MOV			EDI,EAX
		MOV			EDX,EBX
		MOV			ECX,[EDI+Voice.Type]
		MOV			EAX,[EDX+ECX*4+BuffVc_SpeedBuffVc]
		XOR			EBP,EBP
		MOV			ESI,[EDI+Voice.Ptr] ; Pos = 0
		MOV			EBX,[EDI+Voice.Size]
		ADD			EBX,[EDI+Voice.Ptr]
		DEC			EBX ; Size - 1

		POP			EDX
		POP			EDI
		POP			ECX

		CALL		EAX
		POP			EAX
		OR			ECX,ECX
		JNZ			.VcInBcle2

		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
		MOV			[EAX+Voice.RestCoeff],EBP

		JMP			SHORT .PasFinVoice2
.VcPasInBcle2:
		MOV			EBP,ECX
		CALL		GetBaseAddress
		MOV			EDI,[EBX+EffectVcBuff]	; Ajout Des Voix ----
		MOV			ESI,[EBX+TempVcCopyBuff]
		MOV			ECX,[EAX+Voice.Type]
		MOV			EAX,[EBX+ECX*4+AddrAdd_BCalc16BitM]
		MOV			ECX,SizeDMABuff/2
		SUB			ECX,EBP
		PUSH		ECX
		CALL		EAX

		XOR			EBX,EBX
		MOV			[EDX-4],EBX
		POP			ECX

		POP			EAX
		POP			ESI
		JMP			SHORT .FinPrcTrtSpd
.PasFinVoice2:
		CALL		GetBaseAddress
		MOV			EDI,[EBX+EffectVcBuff]	; Ajout Des Voix ----
		MOV			ESI,[EBX+TempVcCopyBuff]
		MOV			ECX,[EAX+Voice.Type]
		MOV			EAX,[EBX+ECX*4+AddrAdd_BCalc16BitM]
		MOV			ECX,SizeDMABuff/2
		CALL		EAX
		MOV			ECX,SizeDMABuff/2

		POP			EAX
		POP			ESI
;****************************************************************************
.FinPrcTrtSpd:
		TEST		DWORD [EAX+Voice.Effect],ChgVol
		JZ			.PasChgVol
		;--- Traite changement de volume ----------------------------
		PUSH		EAX
		PUSH		ECX

		CALL		GetBaseAddress
		MOV			ESI,[EBX+EffectVcBuff]
		; ECX contient deja Nb Ent
		MOV			EDX,[EAX+Voice.LeftVol]
		MOV			EDI,[EAX+Voice.RightVol]
		CALL		ChgVolBuffCalcM

		POP			ECX
		POP			EAX
		;--- Fin traite Chg Vol -------------------------------------
.PasChgVol:
		; Ajoute Buffer d'effet  au buffer de calcul ----------------
		CALL		GetBaseAddress
		MOV			ESI,[EBX+EffectVcBuff]
		MOV			EDI,[EBX+BuffCalc16Bit]
		;ECX contient deja le Nb d'entite
.BcAddBuffEff:
		LODSD
		ADD			[EDI],EAX
		DEC			ECX
		LEA			EDI,[EDI+4]
		JNZ			.BcAddBuffEff


		JMP			.FinVcEff
.OnlyChgSpeed:	; uniquement Chg Speed --------------------------------------

		PUSH		ESI
		MOV			EDI,[EBX+TempVcCopyBuff]
		MOV			EDX,[EAX+Voice.Speed]
		PUSH		EDI
		PUSH		EDX

		MOV			EDI,EAX
		MOV			EDX,EBX
		MOV			ECX,[EDI+Voice.Type]
		MOV			EAX,[EDX+ECX*4+BuffVc_SpeedBuffVc]
		MOV			EBP,[EDI+Voice.RestCoeff]
		MOV			ESI,[EDI+Voice.Pos]
		ADD			ESI,[EDI+Voice.Ptr]
		MOV			EBX,[EDI+Voice.Size]
		ADD			EBX,[EDI+Voice.Ptr]
		DEC			EBX ; Size - 1
		MOV			ECX,SizeDMABuff/2

		POP			EDX
		POP			EDI
		CALL		EAX
		POP			EDX  ; ancien Val ESI
		MOV			EAX,[EDX-4]

		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
		MOV			[EAX+Voice.RestCoeff],EBP

		MOV			EAX,[EDX-4]
		OR			ECX,ECX
		MOV			ESI,EDX ; restore ESI as VoicePtr
		JZ			NEAR .PasFinVoice1
		; Traitement Dernier Bloc -----------------------------------
		TEST		[EAX+Voice.VcSTATE],DWORD VcInBoucle
		JZ			.VcPasInBcle
		; Traitement Voix en Boucle --------

.VcInBcle:
.VcMixRemainBytes:
;*** do mix remaining bytes of current voice **********************
		CALL		GetBaseAddress
		MOV			EDX,[EAX+Voice.Speed]

		PUSH		EAX	; Ptr Vc
		PUSH		ECX	; ECX nb Entite restant
		PUSH		EDI	; ptr retourne par la proc precedent
		PUSH		EDX

		MOV			EDI,EAX
		MOV			EDX,EBX
		MOV			ECX,[EDI+Voice.Type]
		MOV			EAX,[EDX+ECX*4+BuffVc_SpeedBuffVc]
		XOR			EBP,EBP
		MOV			ESI,[EDI+Voice.Ptr] ; Pos = 0
		MOV			EBX,[EDI+Voice.Size]
		ADD			EBX,[EDI+Voice.Ptr]
		DEC			EBX ; Size - 1

		POP			EDX
		POP			EDI
		POP			ECX

		CALL		EAX
		POP			EAX

		; test again looping as this could be a new queued Voice not looping
		TEST		[EAX+Voice.VcSTATE],DWORD VcInBoucle
		JZ			.FinVcEff ; bypass queued check as this happen only for new Queued voice

		OR			ECX,ECX
		JNZ			.VcInBcle

		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
		MOV			[EAX+Voice.RestCoeff],EBP

		JMP			SHORT .PasFinVoice1
.VcPasInBcle:
; check if this voice is queued
		TEST		[EAX+Voice.VcSTATE],DWORD VcQueued
		JZ			.VcPasQueued
; current voice ended, but queued: so use queued voice instead
		CMP			DWORD [ESI-4+MAX_VOICES*4], BYTE 0 ; check QueueVoicePtr not null
		JE			.VcPasQueued
		AND 		DWORD [EAX+Voice.VcSTATE], ~(VcQueued) ; delete queued FLAG
		MOV			EAX,[ESI-4+MAX_VOICES*4] ; read QueueVoicePtr
		MOV			[ESI-4],EAX ; write to VoicePtr
		MOV			DWORD [ESI-4+MAX_VOICES*4], 0 ; clear QueueVoicePtr
		; switch to no effect ?
		CMP			DWORD [EAX+Voice.Effect],BYTE 0
		JE			VoiceNoEff16BM.VcMixRemainBytes
		; else keep with effect
		JMP			SHORT .VcMixRemainBytes
.VcPasQueued:
		JECXZ		.FinVcEff

		MOV			EBP,ECX
		CALL		GetBaseAddress
		MOV			EDI,[EBX+BuffCalc16Bit]	; Ajout Des Voix ----
		MOV			ESI,[EBX+TempVcCopyBuff]
		MOV			ECX,[EAX+Voice.Type]
		MOV			EAX,[EBX+ECX*4+AddrAdd_BCalc16BitM]
		MOV			ECX,SizeDMABuff/2
		SUB			ECX,EBP
		CALL		EAX

		XOR			EBX,EBX
		MOV			[EDX-4],EBX
		JMP			SHORT .FinVcEff
.PasFinVoice1:
		CALL		GetBaseAddress
		MOV			EDI,[EBX+BuffCalc16Bit]	; Ajout Des Voix ----
		MOV			ESI,[EBX+TempVcCopyBuff]
		MOV			ECX,[EAX+Voice.Type]
		MOV			EAX,[EBX+ECX*4+AddrAdd_BCalc16BitM]
		MOV			ECX,SizeDMABuff/2
		CALL		EAX
.FinVcEff:
		POP			ECX
		POP			ESI
		POP			EBP

		RET


;******* PROCED SORTIE 16 Bits STEREO
ProcedOut16BitsS:
		CALL		GetBaseAddress

		XOR			EAX,EAX 		; zero Buff Calc ----
		MOV			EDI,[EBX+BuffCalc16Bit]
		MOV			ECX,SizeDMABuff/2
		REP			STOSD

		MOV			ECX,[EBX+MaxNbVoice]
		LEA			ESI,[EBX+VoicePtr]
.BcTrtVoice:		LODSD
		OR			EAX,EAX
		JZ			.FreeVoice
		CMP			DWORD [EAX+Voice.Effect], BYTE 0
		JNZ			.VoiceEff
		CALL		VoiceNoEff16BS
		JMP			SHORT .VoiceNEff
.VoiceEff:
		CALL		VoiceEff16BS
.VoiceNEff:
.FreeVoice:
		DEC		ECX
		JNZ		.BcTrtVoice

		PUSH		ES		; Copie Vers Bloc de Sortie
		CALL		GetBaseAddress
		MOV			ESI,[EBX+BuffCalc16Bit]
		MOV			ES,[EBX+SelectorDMA16]
		MOV			EDI,[EBX+OffResCalcDMA16]
		MOV			ECX,SizeDMABuff/4
		CALL		Calc16BitS_Buff16BitS
		POP			ES

		RET

VoiceNoEff16BS:
		PUSH		EBP
		PUSH		ESI
		PUSH		ECX

		CALL		GetBaseAddress

		TEST		[EAX+Voice.VcSTATE],DWORD VcStopped
		JNZ			NEAR .FinVcNEff
;************************************************************
		MOV			ECX,[EAX+Voice.Type]
		MOV			ECX,[EBX+ECX*4+SizeIncVc_BCalc16BitS]
		MOV			EDI,[EAX+Voice.Size]
		SUB			EDI,ECX
		CMP			[EAX+Voice.Pos],EDI
		JB			NEAR .PasFinVoice

		PUSH		ESI

		MOV			ESI,[EAX+Voice.Pos]
		MOV			EDI,[EAX+Voice.Size]
		SUB			EDI,ESI
		ADD			ESI,[EAX+Voice.Ptr]

		MOV			ECX,[EAX+Voice.Type]
		MOV			ECX,[EBX+ECX*4+SizeEchant]

		XCHG		EAX,EDI
		XOR			EDX,EDX
		DIV			ECX
		MOV			ECX,EAX
		PUSH		ECX
		MOV			EAX,[EDI+Voice.Type]
		;MOV			EAX,[EBX+EAX*4+AddrAdd_BCalc16BitS]
		MOV			EDI,[EBX+BuffCalc16Bit]	; Ajout Des Voix ----
		CALL		[EBX+EAX*4+AddrAdd_BCalc16BitS]; EAX

		POP			ECX
		POP			ESI
		; si SON Pas En Boucle alors Fin ----------------------------
		MOV			EAX,[ESI-4]
		TEST		[EAX+Voice.VcSTATE],DWORD VcInBoucle
		JZ			.VcPasInBcle
		; mix remaining bytes using [Voice] pointed by EAX
.VcMixRemainBytes:

		CALL		GetBaseAddress
		MOV			EBP,EDI ; save EDI

		MOV			EDI,SizeDMABuff/4
		SUB			EDI,ECX
		MOV			ECX,EDI

		MOV			EDX,[EAX+Voice.Type]
		MOV			ESI,[EAX+Voice.Ptr] ; Pos 0
		MOV			EDI,EBP ; restore EDI
		PUSH		EAX
		;MOV			EAX,[EBX+EDX*4+AddrAdd_BCalc16BitS]
		;OR			ECX,ECX
		JECXZ		.PasAddVc
		CALL		[EBX+EDX*4+AddrAdd_BCalc16BitS]
.PasAddVc:
		POP			EAX
		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI

		MOV			ECX,SizeDMABuff/4
		JMP			.FinVcNEff
.VcPasInBcle:
		TEST		[EAX+Voice.VcSTATE],DWORD VcQueued
		JZ			.VcPasQueued
; current voice ended, but queued: so use queued voice instead
		CMP			DWORD [ESI-4+MAX_VOICES*4], BYTE 0 ; check QueueVoicePtr not null
		JE			.VcPasQueued
		AND 		DWORD [EAX+Voice.VcSTATE], ~(VcQueued) ; delete queued FLAG
		MOV			EAX,[ESI-4+MAX_VOICES*4] ; read QueueVoicePtr
		MOV			[ESI-4],EAX ; write to VoicePtr
		MOV			DWORD [ESI-4+MAX_VOICES*4], 0 ; clear QueueVoicePtr
		; switch to effect ?
		CMP			DWORD [EAX+Voice.Effect],BYTE 0
		JNE			VoiceEff16BS.VcMixRemainBytes
		; else keep with no effect
		JMP			SHORT .VcMixRemainBytes
.VcPasQueued:

		XOR			EAX,EAX
		MOV			[ESI-4],EAX
		JMP			.FinVcNEff
.PasFinVoice:
.IncFreeVoice:
		MOV			EDI,[EBX+BuffCalc16Bit]	; Ajout Des Voix ----
		MOV			ESI,[EAX+Voice.Ptr]
		ADD			ESI,[EAX+Voice.Pos]
		MOV			EDX,[EAX+Voice.Type]
		PUSH		EAX
		;MOV			EAX,[EBX+EDX*4+AddrAdd_BCalc16BitS]
		MOV			ECX,SizeDMABuff/4
		CALL		[EBX+EDX*4+AddrAdd_BCalc16BitS] ; EAX
		POP			EAX
		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
;***********************************************************************

.FinVcNEff:
		POP			ECX
		POP			ESI
		POP			EBP

		RET

VoiceEff16BS:
		PUSH		EBP
		PUSH		ESI
		PUSH		ECX

		CALL		GetBaseAddress

		TEST		[EAX+Voice.VcSTATE],DWORD VcStopped
		JNZ			NEAR .FinVcEff
		TEST		DWORD [EAX+Voice.Effect],~ChgSpeed
		JZ			NEAR .OnlyChgSpeed
		; Chg Speed et ou autre effet -------------------------------

		PUSH		EBX  ; Vide Buffer de calcule
		PUSH		EAX
		MOV			EDI,[EBX+EffectVcBuff]
		XOR			EAX,EAX
		MOV			ECX,SizeDMABuff/2
		REP			STOSD
		POP			EAX
		POP			EBX
		; traite changement de vitesse ------------------
		TEST		DWORD [EAX+Voice.Effect],ChgSpeed
		JNZ			NEAR .ProcChgSpeed
;****************************************************************************
;****************************************************************************

		PUSH		ESI
		MOV			ECX,[EAX+Voice.Type]
		MOV			ECX,[EBX+ECX*4+SizeIncVc_BCalc16BitS]
		MOV			EDI,[EAX+Voice.Size]
		SUB			EDI,ECX
		CMP			[EAX+Voice.Pos],EDI
		JB			NEAR .NSPasFinVoice

		PUSH		ESI

		MOV			ESI,[EAX+Voice.Pos]
		MOV			EDI,[EAX+Voice.Size]
		SUB			EDI,ESI
		ADD			ESI,[EAX+Voice.Ptr]

		MOV			ECX,[EAX+Voice.Type]
		MOV			ECX,[EBX+ECX*4+SizeEchant]

		XCHG		EAX,EDI
		XOR			EDX,EDX
		DIV			ECX
		MOV			ECX,EAX
		PUSH		ECX
		MOV			EAX,[EDI+Voice.Type]
		MOV			EAX,[EBX+EAX*4+AddrAdd_BCalc16BitS]
		MOV			EDI,[EBX+EffectVcBuff]	; Ajout Des Voix ----
		CALL		EAX

		POP			ECX
		POP			ESI
		; si SON Pas En Boucle alors Fin ----------------------------
		MOV			EAX,[ESI-4]
		TEST		[EAX+Voice.VcSTATE],DWORD VcInBoucle
		JZ			.NSVcPasInBcle
		CALL		GetBaseAddress
		PUSH		EDI

		MOV			EDI,SizeDMABuff/4
		SUB			EDI,ECX
		MOV			ECX,EDI

		MOV			EDX,[EAX+Voice.Type]
		MOV			ESI,[EAX+Voice.Ptr] ; Pos 0
		MOV			EBP,[EAX+Voice.Ptr]
		POP			EDI  ; ----------
		PUSH		EAX
		MOV			EAX,[EBX+EDX*4+AddrAdd_BCalc16BitS]
		OR			ECX,ECX
		JZ			.PasAddVc
		CALL		EAX
.PasAddVc:
		POP			EAX
		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI

		MOV			ECX,SizeDMABuff/4
		POP			ESI
		JMP			.FinPrcTrtSpd
.NSVcPasInBcle:
		XOR			EAX,EAX
		MOV			[ESI-4],EAX
		POP			ESI
		JMP			.FinPrcTrtSpd
.NSPasFinVoice:
.IncFreeVoice:
		MOV			EDI,[EBX+EffectVcBuff]	; Ajout Des Voix ----
		MOV			ESI,[EAX+Voice.Ptr]
		ADD			ESI,[EAX+Voice.Pos]
		MOV			EDX,[EAX+Voice.Type]
		PUSH		EAX
		MOV			EAX,[EBX+EDX*4+AddrAdd_BCalc16BitS]
		MOV			ECX,SizeDMABuff/4
		CALL		EAX
		POP			EAX
		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
		MOV			ECX,SizeDMABuff/4
		POP			ESI
;****************************************************************************
;****************************************************************************
		JMP		.FinPrcTrtSpd
.ProcChgSpeed:
;****************************************************************************
		PUSH		ESI
		PUSH		EAX

		PUSH		ESI
		MOV			EDI,[EBX+TempVcCopyBuff]
		MOV			EDX,[EAX+Voice.Speed]
		PUSH		EDI
		PUSH		EDX

		MOV			EDI,EAX
		MOV			EDX,EBX
		MOV			ECX,[EDI+Voice.Type]
		MOV			EAX,[EDX+ECX*4+BuffVc_SpeedBuffVc]
		MOV			EBP,[EDI+Voice.RestCoeff]
		MOV			ESI,[EDI+Voice.Pos]
		ADD			ESI,[EDI+Voice.Ptr]
		MOV			EBX,[EDI+Voice.Size]
		ADD			EBX,[EDI+Voice.Ptr]
		DEC			EBX ; Size - 1
		MOV			ECX,SizeDMABuff/4

		POP			EDX
		POP			EDI
		CALL		EAX
		POP			EDX  ; EDX = ancien Val ESI
		MOV			EAX,[EDX-4]

		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
		MOV			[EAX+Voice.RestCoeff],EBP

		MOV			EAX,[EDX-4]
		OR			ECX,ECX
		JZ			NEAR .PasFinVoice2
		; Traitement Dernier Bloc -----------------------------------
		TEST		[EAX+Voice.VcSTATE],DWORD VcInBoucle
		JZ			.VcPasInBcle2
		; Traitement Voix en Boucle --------

.VcInBcle2:
		CALL		GetBaseAddress
		MOV			EDX,[EAX+Voice.Speed]

		PUSH		EAX	; Ptr Vc
		PUSH		ECX	; ECX nb Entite restant
		PUSH		EDI	; ptr retourne par la proc precedent
		PUSH		EDX

		MOV			EDI,EAX
		MOV			EDX,EBX
		MOV			ECX,[EDI+Voice.Type]
		MOV			EAX,[EDX+ECX*4+BuffVc_SpeedBuffVc]
		XOR			EBP,EBP
		MOV			ESI,[EDI+Voice.Ptr] ; Pos = 0
		MOV			EBX,[EDI+Voice.Size]
		ADD			EBX,[EDI+Voice.Ptr]
		DEC			EBX ; Size - 1

		POP			EDX
		POP			EDI
		POP			ECX

		CALL		EAX
		POP			EAX
		OR			ECX,ECX
		JNZ			.VcInBcle2

		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
		MOV			[EAX+Voice.RestCoeff],EBP

		JMP			SHORT .PasFinVoice2
.VcPasInBcle2:
		MOV			EBP,ECX
		CALL		GetBaseAddress
		MOV			EDI,[EBX+EffectVcBuff]	; Ajout Des Voix ----
		MOV			ESI,[EBX+TempVcCopyBuff]
		MOV			ECX,[EAX+Voice.Type]
		MOV			EAX,[EBX+ECX*4+AddrAdd_BCalc16BitS]
		MOV			ECX,SizeDMABuff/4
		SUB			ECX,EBP
		PUSH		ECX
		CALL		EAX

		XOR			EBX,EBX
		MOV			[EDX-4],EBX
		POP			ECX

		POP			EAX
		POP			ESI
		JMP			SHORT .FinPrcTrtSpd
.PasFinVoice2:
		CALL		GetBaseAddress
		MOV			EDI,[EBX+EffectVcBuff]	; Ajout Des Voix ----
		MOV			ESI,[EBX+TempVcCopyBuff]
		MOV			ECX,[EAX+Voice.Type]
		MOV			EAX,[EBX+ECX*4+AddrAdd_BCalc16BitS]
		MOV			ECX,SizeDMABuff/4
		CALL		EAX
		MOV			ECX,SizeDMABuff/4

		POP			EAX
		POP			ESI
;****************************************************************************

.FinPrcTrtSpd:
		TEST		DWORD [EAX+Voice.Effect],ChgVol
		JZ			.PasChgVol
		;--- Traite changement de volume ----------------------------
		PUSH		EAX
		PUSH		ECX

		CALL		GetBaseAddress
		MOV			ESI,[EBX+EffectVcBuff]
		; ECX contient deja Nb Ent
		MOV			EDX,[EAX+Voice.LeftVol]
		MOV			EDI,[EAX+Voice.RightVol]
		CALL		ChgVolBuffCalcS

		POP			ECX
		POP			EAX
		;--- Fin traite Chg Vol -------------------------------------
.PasChgVol:
		; Ajoute Buffer d'effet  au buffer de calcul ----------------
		CALL		GetBaseAddress
		MOV			ESI,[EBX+EffectVcBuff]
		MOV			EDI,[EBX+BuffCalc16Bit]
		ADD			ECX,ECX ; puisque STEREO
		;ECX contient deja le Nb d'entite
.BcAddBuffEff:
		LODSD
		ADD			[EDI],EAX
		DEC			ECX
		LEA			EDI,[EDI+4]
		JNZ			.BcAddBuffEff

		JMP			.FinVcEff
.OnlyChgSpeed:	; uniquement Chg Speed --------------------------------------

		PUSH		ESI
		MOV			EDI,[EBX+TempVcCopyBuff]
		MOV			EDX,[EAX+Voice.Speed]
		PUSH		EDI
		PUSH		EDX

		MOV			EDI,EAX
		MOV			EDX,EBX
		MOV			ECX,[EDI+Voice.Type]
		MOV			EAX,[EDX+ECX*4+BuffVc_SpeedBuffVc]
		MOV			EBP,[EDI+Voice.RestCoeff]
		MOV			ESI,[EDI+Voice.Pos]
		ADD			ESI,[EDI+Voice.Ptr]
		MOV			EBX,[EDI+Voice.Size]
		ADD			EBX,[EDI+Voice.Ptr]
		DEC			EBX ; Size - 1
		MOV			ECX,SizeDMABuff/4

		POP			EDX
		POP			EDI
		CALL		EAX
		POP			EDX  ; ancien Val ESI
		MOV			EAX,[EDX-4]

		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
		MOV			[EAX+Voice.RestCoeff],EBP

		MOV			EAX,[EDX-4]
		OR			ECX,ECX
		MOV			ESI,EDX ; restore ESI as VoicePtr
		JZ			.PasFinVoice1
		; Traitement Dernier Bloc -----------------------------------
		TEST		[EAX+Voice.VcSTATE],DWORD VcInBoucle
		JZ			.VcPasInBcle
		; Traitement Voix en Boucle --------

.VcInBcle:
.VcMixRemainBytes:
;*** do mix remaining bytes of current voice **********************
		CALL		GetBaseAddress
		MOV			EDX,[EAX+Voice.Speed]

		PUSH		EAX	; Ptr Vc
		PUSH		ECX	; ECX nb Entite restant
		PUSH		EDI	; ptr retourne par la proc precedent
		PUSH		EDX

		MOV			EDI,EAX
		MOV			EDX,EBX
		MOV			ECX,[EDI+Voice.Type]
		MOV			EAX,[EDX+ECX*4+BuffVc_SpeedBuffVc]
		XOR			EBP,EBP
		MOV			ESI,[EDI+Voice.Ptr] ; Pos = 0
		MOV			EBX,[EDI+Voice.Size]
		ADD			EBX,[EDI+Voice.Ptr]
		DEC			EBX ; Size - 1

		POP			EDX
		POP			EDI
		POP			ECX

		CALL		EAX
		POP			EAX

		; test again looping as this could be a new queued Voice not looping
		TEST		[EAX+Voice.VcSTATE],DWORD VcInBoucle
		JZ			.FinVcEff ; bypass queued check as this happen only for new Queued voice

		OR			ECX,ECX
		JNZ			.VcInBcle

		SUB			ESI,[EAX+Voice.Ptr]
		MOV			[EAX+Voice.Pos],ESI
		MOV			[EAX+Voice.RestCoeff],EBP

		JMP			SHORT .PasFinVoice1
.VcPasInBcle:
; check if this voice is queued
		TEST		[EAX+Voice.VcSTATE],DWORD VcQueued
		JZ			.VcPasQueued
; current voice ended, but queued: so use queued voice instead
		CMP			DWORD [ESI-4+MAX_VOICES*4], BYTE 0 ; check QueueVoicePtr not null
		JE			.VcPasQueued
		AND 		DWORD [EAX+Voice.VcSTATE], ~(VcQueued) ; delete queued FLAG
		MOV			EAX,[ESI-4+MAX_VOICES*4] ; read QueueVoicePtr
		MOV			[ESI-4],EAX ; write to VoicePtr
		MOV			DWORD [ESI-4+MAX_VOICES*4], 0 ; clear QueueVoicePtr
		; switch to no effect ?
		CMP			DWORD [EAX+Voice.Effect],BYTE 0
		JE			VoiceNoEff16BS.VcMixRemainBytes
		; else keep with effect
		JMP			SHORT .VcMixRemainBytes
.VcPasQueued:
		JECXZ		.FinVcEff

		MOV			EBP,ECX
		CALL		GetBaseAddress
		MOV			EDI,[EBX+BuffCalc16Bit]	; Ajout Des Voix ----
		MOV			ESI,[EBX+TempVcCopyBuff]
		MOV			ECX,[EAX+Voice.Type]
		MOV			EAX,[EBX+ECX*4+AddrAdd_BCalc16BitS]
		MOV			ECX,SizeDMABuff/4
		SUB			ECX,EBP
		CALL		EAX

		XOR			EBX,EBX
		MOV			[EDX-4],EBX
		JMP			SHORT .FinVcEff
.PasFinVoice1:
		CALL		GetBaseAddress
		MOV			EDI,[EBX+BuffCalc16Bit]	; Ajout Des Voix ----
		MOV			ESI,[EBX+TempVcCopyBuff]
		MOV			ECX,[EAX+Voice.Type]
		MOV			EAX,[EBX+ECX*4+AddrAdd_BCalc16BitS]
		MOV			ECX,SizeDMABuff/4
		CALL		EAX
.FinVcEff:
		POP			ECX
		POP			ESI
		POP			EBP

		RET


;******* PROCED ENTREE 8 Bits MONO
ProcedIn8BitsM:


		RET

;******* PROCED ENTREE 8 Bits STEREO
ProcedIn8BitsS:


		RET

;******* PROCED ENTREE 16 Bits MONO
ProcedIn16BitsM:


		RET

;******* PROCED ENTREE 16 Bits STEREO
ProcedIn16BitsS:


		RET

;****************************************************
;*** Addition Different type VOIX vers Buffer de calcul 8 Bits MONO
;****************************************************
; ESI : son source, EDI : pointeur Buff Calc, ECX : Taille en entite
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AddVc8BitM_BCalc8BitM:
.Bc:
		XOR		EAX,EAX
		LODSB
		SUB		EAX,0x80
		ADD		[EDI],EAX
		DEC		ECX
		LEA		EDI,[EDI+4]
		JNZ		.Bc
		RET

AddVc8BitS_BCalc8BitM:
.Bc:
		XOR		EAX,EAX       ;-----------
		LODSB
		SUB		EAX,0x80
		SAR		EAX,1
		ADD		[EDI],EAX
		XOR		EAX,EAX       ;-----------
		LODSB
		SUB		EAX,0x80
		SAR		EAX,1
		ADD		[EDI],EAX

		DEC		ECX
		LEA		EDI,[EDI+4]
		JNZ		.Bc
		RET

AddVc16BitM_BCalc8BitM:
.Bc:
		LODSW
		MOVSX	EAX,AX
		SAR		EAX,8
		ADD		[EDI],EAX
		DEC		ECX
		LEA		EDI,[EDI+4]
		JNZ		.Bc
		RET

AddVc16BitS_BCalc8BitM:
.Bc:
		LODSW			      ;-----------
		MOVSX	EAX,AX
		SAR		EAX,9         ; div / 2 (8+1) as both channels added to same dword
		ADD		[EDI],EAX
		LODSW			      ;-----------
		MOVSX	EAX,AX
		SAR		EAX,9         ; div / 2 (8+1) as both channels added to same dword
		ADD		[EDI],EAX

		DEC		ECX
		LEA		EDI,[EDI+4]
		JNZ		.Bc
		RET



;******************************************************
;*** Addition Different type de VOIX vers Buffer de calcul 8 Bits STEREO
;******************************************************
; ESI : son source, EDI : pointeur Buff Calc, ECX : Taille en entite
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

AddVc8BitM_BCalc8BitS:
.Bc:
		XOR		EAX,EAX
		LODSB
		SUB		EAX,0x80
		ADD		[EDI],EAX
		ADD		[EDI+4],EAX
		DEC		ECX
		LEA		EDI,[EDI+8]
		JNZ		.Bc
		RET


AddVc8BitS_BCalc8BitS:
.Bc:
		XOR		EAX,EAX      ;---------
		LODSB
		SUB		EAX,0x80
		ADD		[EDI],EAX
		XOR		EAX,EAX      ;---------
		LODSB
		SUB		EAX,0x80
		ADD		[EDI+4],EAX
		DEC		ECX
		LEA		EDI,[EDI+8]
		JNZ		.Bc
		RET

AddVc16BitM_BCalc8BitS:
.Bc:
		LODSW
		MOVSX		EAX,AX
		SAR		EAX,8
		ADD		[EDI],EAX
		ADD		[EDI+4],EAX
		DEC		ECX
		LEA		EDI,[EDI+8]
		JNZ		.Bc
		RET

AddVc16BitS_BCalc8BitS:
.Bc:
		LODSW		;---------
		MOVSX		EAX,AX
		SAR			EAX,8
		ADD			[EDI],EAX
		LODSW		;---------
		MOVSX		EAX,AX
		SAR			EAX,8
		ADD			[EDI+4],EAX
		DEC			ECX
		LEA			EDI,[EDI+8]
		JNZ			.Bc
		RET

;****************************************************
;*** Addition different type de VOIX  vers Buffer de calcul 16 Bits MONO
;****************************************************
; ESI : son source, EDI : pointeur Buff Calc, ECX : Taille en entite
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

AddVc8BitM_BCalc16BitM:
.Bc:
		XOR		EAX,EAX
		LODSB
		SUB		EAX,0x80
		SHL		EAX,8
		ADD		[EDI],EAX
		DEC		ECX
		LEA		EDI,[EDI+4]
		JNZ		.Bc
		RET


AddVc8BitS_BCalc16BitM:
.Bc:
		XOR		EAX,EAX      ;----------
		LODSB
		SUB		EAX,0x80
		SHL		EAX,7        ; div 2 (8-1), as both channels will be added to the same DWORD
		ADD		[EDI],EAX
		XOR		EAX,EAX      ;----------
		LODSB
		SUB		EAX,0x80
		SHL		EAX,7        ; div 2 (8-1), as both channels will be added to the same DWORD
		ADD		[EDI],EAX
		DEC		ECX
		LEA		EDI,[EDI+4]
		JNZ		.Bc
		RET


AddVc16BitM_BCalc16BitM:
.Bc:
		LODSW
		MOVSX	EAX,AX
		ADD		[EDI],EAX
		DEC		ECX
		LEA		EDI,[EDI+4]
		JNZ		.Bc
		RET

AddVc16BitS_BCalc16BitM:
.Bc:
		LODSW			     ;---------
		MOVSX		EAX,AX
		SAR			EAX,1    ; channel divided by 2 as both channel will be added to the mono channel
		ADD			[EDI],EAX
		LODSW			     ;---------
		MOVSX		EAX,AX
		SAR			EAX,1    ; channel divided by 2 as both channel will be added to the mono channel
		ADD			[EDI],EAX
		DEC			ECX
		LEA			EDI,[EDI+4]
		JNZ			.Bc
		RET


;****************************************************
;*** Addition different type de VOIX vers Buffer de calcul 16 Bits STEREO
;****************************************************
; ESI : son source, EDI : pointeur Buff Calc, ECX : Taille en entite
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AddVc8BitM_BCalc16BitS:
.Bc:
		XOR		EAX,EAX
		LODSB
		SUB		EAX,0x80
		SHL		EAX,8
		ADD		[EDI],EAX
		ADD		[EDI+4],EAX
		DEC		ECX
		LEA		EDI,[EDI+8]
		JNZ		.Bc
		RET

AddVc8BitS_BCalc16BitS:
.Bc:
		XOR		EAX,EAX   ;-------
		LODSB
		SUB		EAX,0x80
		SHL		EAX,8
		ADD		[EDI],EAX

		XOR		EAX,EAX   ;-------
		LODSB
		SUB		EAX,0x80
		SHL		EAX,8
		ADD		[EDI+4],EAX
		DEC		ECX
		LEA		EDI,[EDI+8]
		JNZ		.Bc
		RET

AddVc16BitM_BCalc16BitS:
.Bc:
		LODSW
		MOVSX	EAX,AX
		ADD		[EDI],EAX
		ADD		[EDI+4],EAX
		DEC		ECX
		LEA		EDI,[EDI+8]
		JNZ		.Bc
		RET

AddVc16BitS_BCalc16BitS:
.Bc:
		LODSW			     ;---------
		MOVSX	EAX,AX
		ADD		[EDI],EAX
		LODSW			     ;---------
		MOVSX	EAX,AX
		ADD		[EDI+4],EAX

		DEC		ECX
		LEA		EDI,[EDI+8]
		JNZ		.Bc
		RET

;****************************************************************************


; Calc 8 Bits MONO vers Buff 8 Bits MONO
; Calc src [DS:ESI] -> Buff Dest [ES:EDI] ,  ECX Taille
Calc8BitM_Buff8BitM:
		PUSH	ESI      ; sauvegarde Ptr Src
		PUSH	ECX	 ;   "        Taille
		XOR		EBX,EBX  ; recherche valeur absolu MAX
.RechMax:
		LODSD
		OR		EAX,EAX
		JNS		.PasABS
		NEG		EAX
.PasABS:
		CMP		EAX,EBX
		JBE		.PasNvMax
		MOV		EBX,EAX
.PasNvMax:
		DEC		ECX
		JNZ		.RechMax
		POP		ECX	 ;   "	   Taille
		POP		ESI	 ; restore Ptr Src

		CMP		EBX,127	; si MAX <= 127 alors pas ajustement
		JBE		.PasAj

		MOV		EAX,127*128 ;127 Val Max& 128 Prec sur 7 Bits
		XOR		EDX,EDX
		DIV		EBX
		MOV		EDX,EAX
.BcAjCopy:
		LODSD
		IMUL	EAX,EDX
		SAR		EAX,7
		ADD		EAX,0x80
		STOSB
		DEC		ECX
		JNZ		.BcAjCopy
		JMP		.Fin
.PasAj:

.BcCopyNorm:	LODSD
		ADD		EAX,0x80
		STOSB
		DEC		ECX
		JNZ		.BcCopyNorm
.Fin:
		RET


; Calc 8 Bits STEREO vers Buff 8 Bits STEREO
; Calc src [DS:ESI] -> Buff Dest [ES:EDI] ,  ECX Taille
Calc8BitS_Buff8BitS:
		;-------------- CANAL GAUCHE
		PUSH		ESI      ; sauvegarde Ptr Src
		PUSH		ECX	 ;   "        Taille
		XOR		EBX,EBX  ; rech valeur absolu MAX gauche
.RechMaxLf:
		LODSD
		OR		EAX,EAX
		JNS		.PasABSLf
		NEG		EAX
.PasABSLf:
		CMP		EAX,EBX
		JBE		.PasNvMaxLf
		MOV		EBX,EAX
.PasNvMaxLf:
		DEC		ECX
		LEA		ESI,[ESI+4]
		JNZ		.RechMaxLf
		POP		ECX	;   "	   Taille
		POP		ESI	; restore Ptr Src

		PUSH	ESI
		PUSH	EDI
		PUSH	ECX
		CMP		EBX,127	; si MAX <= 127 alors pas ajustement
		JBE		.PasAjLf

		MOV		EAX,127*128 ;127 Val Max& 128 Prec sur 7 Bits
		XOR		EDX,EDX
		DIV		EBX
		MOV		EDX,EAX
.BcAjCopyLf:
		LODSD
		IMUL	EAX,EDX
		SAR		EAX,7
		ADD		EAX,0x80
		STOSB
		DEC		ECX
		LEA		ESI,[ESI+4]
		LEA		EDI,[EDI+1]
		JNZ		.BcAjCopyLf
		JMP		.FinLf
.PasAjLf:
.BcCopyNormLf:
		LODSD
		ADD		EAX,0x80
		STOSB
		DEC		ECX
		LEA		ESI,[ESI+4]
		LEA		EDI,[EDI+1]
		JNZ		.BcCopyNormLf
.FinLf:
		POP		ECX
		POP		EDI
		POP		ESI

		;-------------- CANAL DROIT
		PUSH	ESI      ; sauvegarde Ptr Src
		PUSH	ECX	 ;   "        Taille
		XOR		EBX,EBX  ; rech valeur absolu MAX droite
		ADD		ESI,BYTE 4
.RechMaxRg:
		LODSD
		OR		EAX,EAX
		JNS		.PasABSRg
		NEG		EAX
.PasABSRg:
		CMP		EAX,EBX
		JBE		.PasNvMaxRg
		MOV		EBX,EAX
.PasNvMaxRg:
		DEC		ECX
		LEA		ESI,[ESI+4]
		JNZ		.RechMaxRg
		POP		ECX	;   "	   Taille
		POP		ESI	; restore Ptr Src
		LEA		ESI,[ESI+4]   ; saute canal gauche
		INC		EDI  ; EDI = EDI + 1 ; saute canal gauche

		CMP		EBX,127	; si MAX <= 127 alors pas ajustement
		JBE		.PasAjRg

		MOV		EAX,127*128 ;127 Val Max& 128 Prec sur 7 Bits
		XOR		EDX,EDX
		DIV		EBX
		MOV		EDX,EAX
.BcAjCopyRg:
		LODSD
		IMUL	EAX,EDX
		SAR		EAX,7
		ADD		EAX,0x80
		STOSB
		DEC		ECX
		LEA		ESI,[ESI+4]  ; saute canal gauche
		LEA		EDI,[EDI+1]  ;  "
		JNZ		.BcAjCopyRg
		JMP		.FinRg
.PasAjRg:
.BcCopyNormRg:
		LODSD
		ADD		EAX,0x80
		STOSB
		DEC		ECX
		LEA		ESI,[ESI+4]
		LEA		EDI,[EDI+1]
		JNZ		.BcCopyNormRg
.FinRg:
		RET


; Calc 16 Bits MONO vers Buff 16 Bits MONO
; Calc src [DS:ESI] -> Buff Dest [ES:EDI] ,  ECX Taille
Calc16BitM_Buff16BitM:
		PUSH		ESI      ; sauvegarde Ptr Src
		PUSH		ECX	 ;   "        Taille
		XOR		EBX,EBX  ; recherche valeur absolu MAX
.RechMax:
		LODSD
		OR		EAX,EAX
		JNS		.PasABS
		NEG		EAX
.PasABS:
		CMP		EAX,EBX
		JBE		.PasNvMax
		MOV		EBX,EAX
.PasNvMax:
		DEC		ECX
		JNZ		.RechMax
		POP		ECX	 ;   "	   Taille
		POP		ESI	 ; restore Ptr Src

		CMP		EBX,32767 ; si MAX <= 127 alors pas ajustement
		JBE		.PasAj

		MOV		EAX,32767*128 ;32767 Val Max& 128 Prec sur 7 Bits
		XOR		EDX,EDX
		DIV		EBX
		MOV		EDX,EAX
.BcAjCopy:
		LODSD
		IMUL	EAX,EDX
		SAR		EAX,7
		STOSW
		DEC		ECX
		JNZ		.BcAjCopy
		JMP		.Fin
.PasAj:

.BcCopyNorm:
		LODSD
		STOSW
		DEC		ECX
		JNZ		.BcCopyNorm
.Fin:
		RET


; Calc 16 Bits STEREO vers Buff 16 Bits STEREO
; Calc src [DS:ESI] -> Buff Dest [ES:EDI] ,  ECX Taille
Calc16BitS_Buff16BitS:
		;-------------- CANAL GAUCHE
		PUSH	ESI      ; sauvegarde Ptr Src
		PUSH	ECX	 ;   "        Taille
		XOR		EBX,EBX  ; rech valeur absolu MAX gauche
.RechMaxLf:
		LODSD
		OR		EAX,EAX
		JNS		.PasABSLf
		NEG		EAX
.PasABSLf:
		CMP		EAX,EBX
		JBE		.PasNvMaxLf
		MOV		EBX,EAX
.PasNvMaxLf:
		DEC		ECX
		LEA		ESI,[ESI+4]
		JNZ		.RechMaxLf
		POP		ECX	;   "	   Taille
		POP		ESI	; restore Ptr Src

		PUSH	ESI
		PUSH	EDI
		PUSH	ECX
		CMP		EBX,32767 ; si MAX <= 32767 alors pas ajustement
		JBE		.PasAjLf

		MOV		EAX,32767*128 ;32767 Val Max& 128 Prec sur 7 Bits
		XOR		EDX,EDX
		DIV		EBX
		MOV		EDX,EAX
.BcAjCopyLf:
		LODSD
		IMUL	EAX,EDX
		SAR		EAX,7
		STOSW
		DEC		ECX
		LEA		ESI,[ESI+4]
		LEA		EDI,[EDI+2]
		JNZ		.BcAjCopyLf
		JMP		.FinLf
.PasAjLf:
.BcCopyNormLf:
		LODSD
		STOSW
		DEC		ECX
		LEA		ESI,[ESI+4]  ; saute canal gauche
		LEA		EDI,[EDI+2]
		JNZ		.BcCopyNormLf
.FinLf:
		POP		ECX
		POP		EDI
		POP		ESI

		;-------------- CANAL DROIT
		PUSH	ESI      ; sauvegarde Ptr Src
		PUSH	ECX	 ;   "        Taille
		XOR		EBX,EBX  ; rech valeur absolu MAX droite
		ADD		ESI,BYTE 4
.RechMaxRg:
		LODSD
		OR		EAX,EAX
		JNS		.PasABSRg
		NEG		EAX
.PasABSRg:
		CMP		EAX,EBX
		JBE		.PasNvMaxRg
		MOV		EBX,EAX
.PasNvMaxRg:
		DEC		ECX
		LEA		ESI,[ESI+4]
		JNZ		.RechMaxRg
		POP		ECX	;   "	   Taille
		POP		ESI	; restore Ptr Src
		LEA		ESI,[ESI+4]   ; saute canal gauche
		ADD		EDI,BYTE 2 ; EDI = EDI + 2 ; saute canal gauche

		CMP		EBX,32767; si MAX <= 127 alors pas ajustement
		JBE		.PasAjRg

		MOV		EAX,32767*128 ;127 Val Max& 128 Prec sur 7 Bits
		XOR		EDX,EDX
		DIV		EBX
		MOV		EDX,EAX
.BcAjCopyRg:
		LODSD
		IMUL	EAX,EDX
		SAR		EAX,7
		STOSW
		DEC		ECX
		LEA		ESI,[ESI+4]  ; saute canal gauche
		LEA		EDI,[EDI+2]  ;  "
		JNZ		.BcAjCopyRg
		JMP		.FinRg
.PasAjRg:
.BcCopyNormRg:
		LODSD
		STOSW
		DEC		ECX
		LEA		ESI,[ESI+4]  ; saute canal gauche
		LEA		EDI,[EDI+2]
		JNZ		.BcCopyNormRg
.FinRg:
		RET


;****************************************************************************


; Buff 8 Bits MONO vers Speed Buff 8 Bits MONO
; IN --------
; Calc src [DS:ESI] -> Buff Dest [ES:EDI], ECX Taille, EDX Speed, EBX Max ESI
; EBP Pente Speed
; OUT -------
; ECX reste Taile, ESI Pos

PrecSpeed	EQU	7

Buff8BitM_SpeedBuff8BitM:
		MOV		AL,[ESI]
.BcCopy:
		STOSB
		ADD		EBP,EDX
		TEST		EBP,~((1<<PrecSpeed)-1)
		JZ		.PasChgPos
		MOV		EAX,EBP
		SHR		EAX,PrecSpeed
		AND		EBP,BYTE (1<<PrecSpeed)-1
		ADD		ESI,EAX
		CMP		ESI,EBX
		JA		.Fin
		MOV		AL,[ESI]
.PasChgPos:
		DEC		ECX
		JNZ		.BcCopy
.Fin:
		RET

; Buff 8 Bits STEREO vers Speed Buff 8 Bits STEREO
; IN --------
; Calc src [DS:ESI] -> Buff Dest [ES:EDI], ECX Taille, EDX Speed, EBX Max ESI
; EBP Pente Speed
; OUT -------
; ECX reste Taile, ESI Pos

Buff8BitS_SpeedBuff8BitS:
		MOV		AX,[ESI]
.BcCopy:
		STOSW
		ADD		EBP,EDX
		TEST	EBP,~((1<<PrecSpeed)-1)
		JZ		.PasChgPos

		MOV		EAX,EBP
		SHR		EAX,PrecSpeed
		AND		EBP,BYTE (1<<PrecSpeed)-1
		LEA		ESI,[ESI+EAX*2]
		CMP		ESI,EBX
		JA		.Fin
		MOV		AX,[ESI]

.PasChgPos:
		DEC		ECX
		JNZ		.BcCopy
.Fin:
		RET


; Buff 16 Bits MONO vers Speed Buff 16 Bits MONO
; IN --------
; Calc src [DS:ESI] -> Buff Dest [ES:EDI], ECX Taille, EDX Speed, EBX Max ESI
; EBP Pente Speed
; OUT -------
; ECX reste Taile, ESI Pos

Buff16BitM_SpeedBuff16BitM:
		MOV		AX,[ESI]
.BcCopy:
		STOSW
		ADD		EBP,EDX
		TEST	EBP,~((1<<PrecSpeed)-1)
		JZ		.PasChgPos

		MOV		EAX,EBP
		SHR		EAX,PrecSpeed
		AND		EBP,BYTE (1<<PrecSpeed)-1
		LEA		ESI,[ESI+EAX*2]
		CMP		ESI,EBX
		JA		.Fin
		MOV		AX,[ESI]

.PasChgPos:
		DEC		ECX
		JNZ		.BcCopy
.Fin:
		RET


; Buff 16 Bits STEREO vers Speed Buff 16 Bits STEREO
; IN --------
; Calc src [DS:ESI] -> Buff Dest [ES:EDI], ECX Taille, EDX Speed, EBX Max ESI
; EBP Pente Speed
; OUT -------
; ECX reste Taile, ESI Pos

Buff16BitS_SpeedBuff16BitS:
		MOV		EAX,[ESI]
.BcCopy:
		CMP		ESI,EBX
		JAE		.Fin
		STOSD
		ADD		EBP,EDX
		TEST	EBP,~((1<<PrecSpeed)-1)
		JZ		.PasChgPos

		MOV		EAX,EBP
		SHR		EAX,PrecSpeed
		AND		EBP,BYTE (1<<PrecSpeed)-1
		LEA		ESI,[ESI+EAX*4]
		MOV		EAX,[ESI]

.PasChgPos:
		DEC		ECX
		JNZ		.BcCopy
.Fin:
		RET

;****** CHANGE VOLUME BUFF CALC *********************************************
;----------------------------------------------------------------------------

VolPrec		EQU	6
; [DS:ESI] Ptr Buff MONO Calc, ECX Nb Ent, EDX Vol Left, EDI Vol Right
ChgVolBuffCalcM:
		ADD		EDX,EDI
		SHR		EDX,1
.BcChgVol:
		LODSD
		IMUL	EAX,EDX
		SAR		EAX,VolPrec
		DEC		ECX
		MOV		[ESI-4],EAX
		JNZ		.BcChgVol
		RET

; [DS:ESI] Ptr Buff STEREO Calc, ECX Nb Ent, EDX Vol Left, EDI Vol Right
ChgVolBuffCalcS:
.BcChgVol:
		LODSD
		IMUL	EAX,EDX
		SAR		EAX,VolPrec
		MOV		[ESI-4],EAX
		LODSD
		IMUL	EAX,EDI
		SAR		EAX,VolPrec
		DEC		ECX
		MOV		[ESI-4],EAX
		JNZ		.BcChgVol
		RET
