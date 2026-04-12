
AddVoice:
	ARG	PtrAddVoice, 4, VcEff, 4, VcStt, 4, VcPckPtr, 4, AddVcPos, 4
		PUSH		EBX
		PUSH		ESI
		PUSH		EDI

		CLI

		MOV			EDX,[EBP+PtrAddVoice]
		CALL		SearchVoiceIn
		OR			EAX,EAX
		JZ			.FreeNtFnd  ; Voice already exist, => error

		MOV			EDX,[EBP+PtrAddVoice]
		OR			EDX,EDX
		JZ			.FreeNtFnd

		LEA			EDI,[EDX+Voice.Effect]
		XOR			EAX,EAX
		MOV			ECX,19
		REP			STOSD

		CALL		GetBaseAddress

		MOV			ECX,MAX_VOICES
		LEA			ESI,[EBX+VoicePtr]
.BcSrchFree:
		LODSD
		OR			EAX,EAX
		JZ			.FreeFnd
		DEC			ECX
		JNZ			.BcSrchFree
.FreeNtFnd:
		XOR			EAX,EAX
		JMP			SHORT .End
.FreeFnd:	; Ajoute le Voix --------------------------------------------
		MOV			EAX,[EBP+VcStt]
		MOV			[ESI-4],EDX
		MOV			[EDX+Voice.VcSTATE],EAX
		MOV			ECX,[EBP+VcEff]
		OR			ECX,ECX
		JZ			.VcNoEff
		MOV			[EDX+Voice.Effect],ECX
		MOV			ESI,[EBP+VcPckPtr]

		TEST		ECX,ChgSpeed
		JZ			.PasEffChgSpeed
		MOV			EAX,[ESI+VoicePack.Speed]
		AND			EAX,SpeedMask
		MOV			[EDX+Voice.Speed],EAX
.PasEffChgSpeed:
		TEST		ECX,ChgVol
		JZ			.PasEffChgVol
		MOV			EAX,[ESI+VoicePack.LeftVol]
		MOV			EDX,[ESI+VoicePack.RightVol]
		AND			EAX,VolMask
		AND			EDX,VolMask
		MOV			[EDX+Voice.LeftVol],EAX
		MOV			[EDX+Voice.RightVol],EDX
.PasEffChgVol:
.VcNoEff:
		MOV			EAX,[EBP+AddVcPos]
		OR			EAX,EAX
		JZ			.PosZero
		MOV			EDI,[EDX+Voice.Size]
		CMP			EAX,EDI
		JAE			.PosZero
		CALL		GetBaseAddress
		MOV			EDI,EDX
		MOV			ECX,[EDI+Voice.Type]
		XOR			EDX,EDX
		MOV			ECX,[EBX+ECX*4+SizeEchant]
		DIV			ECX
		MUL			ECX
		MOV			[EDI+Voice.Pos],EAX
		JMP			SHORT .PasPosZero
.PosZero:
		MOV			[EDX+Voice.Pos],EAX
.PasPosZero:
		OR			EAX,BYTE -1
.End:
		STI

		POP		EDI
		POP		ESI
		POP		EBX
		RETURN

PrepVoice:
	ARG	PrepPtrVoice, 4

		MOV			EDX,[EBP+PrepPtrVoice]
		MOV			EAX,EDX
		CALL		PrcLockVoice   ; Lock Voice
		OR			EAX,EAX
		JNZ			.Error ; Erreur
		MOV			EAX,EDX
		CALL		PrcLockVoiceData   ; Lock Voice Data
		OR			EAX,EAX
		JZ			.OkVcLocked ; PasErreur
		MOV			EAX,EDX
		CALL		PrcUnlockVoice   ; unlock Voice
		JMP			SHORT .Error	; Erreur
.OkVcLocked:
		OR			EAX,BYTE -1
		JMP			SHORT .Fin
.Error:
		XOR		EAX,EAX
.Fin:
		RETURN

UnprepVoice:
	ARG	UnprepPtrVoice, 4

		MOV			EDX,[EBP+PrepPtrVoice]
		MOV			EAX,EDX
		CALL		PrcUnlockVoice   ; Lock Voice
		MOV			EAX,EDX
		CALL		PrcUnlockVoiceData   ; Lock Voice Data

		RETURN

;----------------------------------------------------------------------

GetVoicePos:
	ARG	GVcPos, 4, PtVrGtVcPos, 4

		MOV			EDX,[EBP+GVcPos]
		CLI
		CALL		SearchVoiceIn
		OR			EAX,EAX
		JZ			.VcFnd
		JMP			SHORT .VcNtFnd
.VcFnd:
		MOV			EAX,[EDX+Voice.Pos]
		MOV			EDX,[EBP+PtVrGtVcPos]
		MOV			[EDX],EAX
		OR			EAX,BYTE -1
		JMP			SHORT .Ok
.VcNtFnd:
		XOR			EAX,EAX
.Ok:
		STI
		RETURN


SetVoicePos:
	ARG	SVcPos, 4, StVrVcPos, 4
		PUSH		EDI
		PUSH		EBX

		MOV			EDX,[EBP+SVcPos]
		CLI
		CALL		SearchVoiceIn
		OR			EAX,EAX
		JZ			.VcFnd
		JMP			SHORT .VcNtFnd
.VcFnd:
		MOV			EAX,[EBP+StVrVcPos]
		CMP			EAX,[EDX+Voice.Size]
		JAE			.VcNtFnd
		CALL		GetBaseAddress
		MOV			EDI,EDX
		MOV			ECX,[EDI+Voice.Type]
		XOR			EDX,EDX
		MOV			ECX,[EBX+ECX*4+SizeEchant]
		DIV			ECX
		XOR			EBX,EBX
		MUL			ECX
		MOV			[EDI+Voice.Pos],EAX
		MOV			[EDI+Voice.RestCoeff],EBX
		OR			EAX,BYTE -1

		JMP			SHORT .Ok
.VcNtFnd:
		XOR		EAX,EAX
.Ok:
		STI
		POP		EBX
		POP		EDI
		RETURN


;----------------------------------------------------------------------

GetVoiceState:
	ARG	GVcStt, 4, PtVrGtVcStt, 4

		MOV			EDX,[EBP+GVcStt]
		CLI
		CALL		SearchVoiceIn
		OR			EAX,EAX
		JZ			.VcFnd
		JMP			SHORT .VcNtFnd
.VcFnd:
		MOV			EAX,[EDX+Voice.VcSTATE]
		MOV			EDX,[EBP+PtVrGtVcStt]
		MOV			[EDX],EAX
		OR			EAX,BYTE -1
		JMP			SHORT .Ok
.VcNtFnd:
		XOR			EAX,EAX
.Ok:
		STI
		RETURN


SetVoiceState:
	ARG	SVcStt, 4, StStt, 4

		MOV			EDX,[EBP+GVcStt]
		CLI
		CALL		SearchVoiceIn
		OR			EAX,EAX
		JZ			.VcFnd
		JMP			SHORT .VcNtFnd

.VcFnd:
		MOV			EAX,[EBP+StStt]
		MOV			[EDX+Voice.VcSTATE],EAX
		OR			EAX,BYTE -1
		JMP			SHORT .Ok
.VcNtFnd:
		XOR			EAX,EAX
.Ok:
		STI
		RETURN

GetVoiceEffPack:
	ARG	GVcEPck, 4, PtrVrGEff, 4, PtrGtVcPck, 4
		PUSH		EBX
		PUSH		ESI
		PUSH		EDI

		MOV			EDX,[EBP+GVcEPck]
		CLI
		CALL		SearchVoiceIn
		OR			EAX,EAX
		JZ			.VcFnd
		JMP			SHORT .VcNtFnd

.VcFnd:
		MOV			ECX,[EDX+Voice.Effect]
		MOV			ESI,[EBP+PtrGtVcPck]
		MOV			EDI,[EBP+PtrVrGEff]
		MOV			[EDI],ECX

		OR			ECX,ECX
		JZ			.VcNoEff

		TEST		ECX,ChgSpeed
		JZ			.PasEffChgSpeed
		MOV			EAX,[EDX+Voice.Speed]
		MOV			[ESI+VoicePack.Speed],EAX
.PasEffChgSpeed:
		TEST		ECX,ChgVol
		JZ			.PasEffChgVol
		MOV			EAX,[EDX+Voice.LeftVol]
		MOV			EDI,[EDX+Voice.RightVol]
		MOV			[ESI+VoicePack.LeftVol],EAX
		MOV			[ESI+VoicePack.RightVol],EDI
.PasEffChgVol:
.VcNoEff:
		OR			EAX,BYTE -1
		JMP			SHORT .Ok
.VcNtFnd:
		XOR			EAX,EAX
.Ok:
		STI

		POP		EDI
		POP		ESI
		POP		EBX
		RETURN

SetVoiceEffPack:
	ARG	SVcEPck, 4, StEffPck, 4, PtrStVcPck, 4
		PUSH		EBX
		PUSH		ESI
		PUSH		EDI

		MOV			EDX,[EBP+SVcEPck]
		CLI
		CALL		SearchVoiceIn
		OR			EAX,EAX
		JZ			.VcFnd
		JMP			SHORT .VcNtFnd

.VcFnd:
		MOV			ECX,[EBP+StEffPck]
		MOV			[EDX+Voice.Effect],ECX
		MOV			ESI,[EBP+PtrStVcPck]

		OR			ECX,ECX
		JZ			.VcNoEff

		TEST		ECX,ChgSpeed
		JZ			.PasEffChgSpeed
		MOV			EAX,[ESI+VoicePack.Speed]
		XOR			EDI,EDI
		AND			EAX,SpeedMask
		MOV			[EDX+Voice.Speed],EAX
		MOV			[EDX+Voice.RestCoeff],EDI
.PasEffChgSpeed:
		TEST		ECX,ChgVol
		JZ			.PasEffChgVol
		MOV			EAX,[ESI+VoicePack.LeftVol]
		MOV			EDI,[ESI+VoicePack.RightVol]
		AND			EAX,VolMask
		AND			EDI,VolMask
		MOV			[EDX+Voice.LeftVol],EAX
		MOV			[EDX+Voice.RightVol],EDI
.PasEffChgVol:
.VcNoEff:

		OR			EAX,BYTE -1
		JMP			SHORT .Ok
.VcNtFnd:
		XOR			EAX,EAX
.Ok:
		STI

		POP		EDI
		POP		ESI
		POP		EBX
		RETURN

DeleteVoice:
	ARG	PtrDelVc, 4
		PUSH		EBX
		PUSH		ESI

		CALL		GetBaseAddress
		MOV			ECX,[EBX+MaxNbVoice]
		LEA			ESI,[EBX+VoicePtr]
		MOV			EDX,[EBP+PtrDelVc]
.BcSrchVc:
		LODSD
		CMP			EDX,EAX
		JE			.VcFnd
		DEC			ECX
		JNZ			.BcSrchVc
		JMP			SHORT .Error
.VcFnd:
		XOR			EAX,EAX
		MOV			[ESI-4],EAX
		OR			EAX,BYTE -1
		JMP			SHORT .Ok
.Error:
		XOR			EAX,EAX
.Ok:
		POP			ESI
		POP			EBX
		RETURN

DeleteAllVoices:
		PUSH		EDI
		PUSH		EBX

		CLI

		XOR			ECX,ECX
		CALL		GetBaseAddress
		MOV			CL,32
		LEA			EDI,[EBX+VoicePtr]
		XOR			EAX,EAX
		REP			STOSD
		MOV			CL,32
		LEA			EDI,[EBX+QueueVoicePtr]
		REP			STOSD

		STI

		POP			EBX
		POP			EDI
		RET

GetCountVoices:
		PUSH		ESI
		PUSH		EBX

		CALL		GetBaseAddress
		MOV			ECX,[EBX+MaxNbVoice]
		LEA			ESI,[EBX+VoicePtr]
		XOR			EDX,EDX
		;CLI
.BcCmptVc:
		LODSD
		OR			EAX,EAX
		JZ			.VcNtActv
		INC			EDX
.VcNtActv:
		DEC			ECX
		JNZ			.BcCmptVc
		;STI
		MOV			EAX,EDX

		POP			EBX
		POP			ESI
		RET

QueueVoice:
	ARG	PtrQAddToVoice, 4, PtrQAddVoice, 4, QVcEff, 4, QVcStt, 4, QVcPckPtr, 4, QVcPos, 4, QVcReplaceExist, 4
		PUSH		EBX
		PUSH		ESI
		PUSH		EDI

		CLI

		; search if the voice to be added to queue exist
		MOV			EDX,[EBP+PtrQAddVoice]
		CALL		SearchVoiceIn
		OR			EAX,EAX
		JZ			SHORT .exitErrFail  ; Voice already exist => exit, error

		; search if the voice to be queued exist
		MOV			EDX,[EBP+PtrQAddToVoice]
		CALL		SearchVoiceIn
		OR			EAX,EAX
		JNZ			SHORT .exitErrFail  ; Voice not found => exit, error

		; check if no queued Voice already Exist
		CALL		GetBaseAddress
		LEA			ESI,[EBX+EDX*4+QueueVoicePtr]
		; replace even if exist
		CMP			DWORD [EBP+QVcReplaceExist], BYTE 0
		JNE			SHORT .ProceedAddV ;
		CMP			DWORD [ESI], BYTE 0
		JNE			SHORT .exitErrFail  ; Voice already queued => exit, error
		JMP			SHORT .ProceedAddV
.exitErrFail:
		XOR			EAX,EAX
		JMP			.End

; Add voice InQueue EDX Index  --------------------------------------------
.ProceedAddV:
		MOV			EAX,[EBP+PtrQAddToVoice]
		MOV			EDX,[EBP+PtrQAddVoice]
		AND 		DWORD [EAX+Voice.VcSTATE], ~(VcQueued) ; delete queued FLAG
		OR			EDX,EDX
		; save new queued voice or delete if pointer NULL
		MOV			[ESI],EDX
		; new queue voice NULL so operation is to delete the queue
		JZ			.PasPosZero

		OR			DWORD [EAX+Voice.VcSTATE], VcQueued ; enable queue flag
		LEA			EDI,[EDX+Voice.Effect]
		XOR			EAX,EAX
		MOV			ECX,19
		REP			STOSD

		MOV			EAX,[EBP+QVcStt]
		MOV			ECX,[EBP+QVcEff]
		MOV			[EDX+Voice.VcSTATE],EAX
		OR			ECX,ECX
		JZ			.VcNoEff
		MOV			ESI,[EBP+QVcPckPtr]
		CMP			ESI,BYTE 0
		JZ			.VcNoEff
		MOV			[EDX+Voice.Effect],ECX

		TEST		ECX,ChgSpeed
		JZ			.PasEffChgSpeed
		MOV			EAX,[ESI+VoicePack.Speed]
		AND			EAX,SpeedMask
		MOV			[EDX+Voice.Speed],EAX
.PasEffChgSpeed:
		TEST		ECX,ChgVol
		JZ			.PasEffChgVol
		MOV			EAX,[ESI+VoicePack.LeftVol]
		MOV			EDX,[ESI+VoicePack.RightVol]
		AND			EAX,VolMask
		AND			EDX,VolMask
		MOV			[EDX+Voice.LeftVol],EAX
		MOV			[EDX+Voice.RightVol],EDX
.PasEffChgVol:
.VcNoEff:
		MOV			EAX,[EBP+QVcPos]
		OR			EAX,EAX
		JZ			.PosZero
		MOV			EDI,[EDX+Voice.Size]
		CMP			EAX,EDI
		JAE			.PosZero
		CALL		GetBaseAddress
		MOV			EDI,EDX
		MOV			ECX,[EDI+Voice.Type]
		XOR			EDX,EDX
		MOV			ECX,[EBX+ECX*4+SizeEchant]
		DIV			ECX
		MUL			ECX
		MOV			[EDI+Voice.Pos],EAX
		JMP			SHORT .PasPosZero
.PosZero:
		MOV			[EDX+Voice.Pos],EAX
.PasPosZero:
		MOV			EAX,1
.End:
		STI

		POP			EDI
		POP			ESI
		POP			EBX
	RETURN

ReplaceVoice:
	ARG	PtrRAddToVoice, 4, PtrRAddVoice, 4, RVcEff, 4, RVcStt, 4, RVcPckPtr, 4, RVcPos, 4
		PUSH		EBX
		PUSH		ESI
		PUSH		EDI

		CLI

		; search if the voice to be added to queue exist
		MOV			EDX,[EBP+PtrRAddVoice]
		CALL		SearchVoiceIn
		OR			EAX,EAX
		JZ			SHORT .exitErrFail  ; Voice already exist => exit, error

		; search if the voice to be queued exist
		MOV			EDX,[EBP+PtrRAddToVoice]
		CALL		SearchVoiceIn
		OR			EAX,EAX
		JNZ			SHORT .exitErrFail  ; Voice not found => exit, error

		; delete any existing queued voice
		CALL		GetBaseAddress
		MOV			DWORD [EBX+EDX*4+QueueVoicePtr], 0
		LEA			ESI,[EBX+EDX*4+VoicePtr]
		JMP			SHORT .ProceedAddV
.exitErrFail:
		XOR			EAX,EAX
		JMP			.End

; replace voice In EDX Index  --------------------------------------------
.ProceedAddV:
		MOV			EDX,[EBP+PtrRAddVoice]
		XOR			EAX,EAX
		OR			EDX,EDX
		MOV			[ESI],EDX ; save new voice
		; new voice ptr NULL, just delete and exit
		JZ			.PasPosZero
		LEA			EDI,[EDX+Voice.Effect]
		MOV			ECX,19
		REP			STOSD

		MOV			EAX,[EBP+RVcStt]
		MOV			ECX,[EBP+RVcEff]
		MOV			[EDX+Voice.VcSTATE],EAX
		OR			ECX,ECX
		JZ			.VcNoEff
		MOV			ESI,[EBP+RVcPckPtr]
		CMP			ESI,BYTE 0
		JZ			.VcNoEff
		MOV			[EDX+Voice.Effect],ECX

		TEST		ECX,ChgSpeed
		JZ			.PasEffChgSpeed
		MOV			EAX,[ESI+VoicePack.Speed]
		AND			EAX,SpeedMask
		MOV			[EDX+Voice.Speed],EAX
.PasEffChgSpeed:
		TEST		ECX,ChgVol
		JZ			.PasEffChgVol
		MOV			EAX,[ESI+VoicePack.LeftVol]
		MOV			EDX,[ESI+VoicePack.RightVol]
		AND			EAX,VolMask
		AND			EDX,VolMask
		MOV			[EDX+Voice.LeftVol],EAX
		MOV			[EDX+Voice.RightVol],EDX
.PasEffChgVol:
.VcNoEff:
		MOV			EAX,[EBP+RVcPos]
		OR			EAX,EAX
		JZ			.PosZero
		MOV			EDI,[EDX+Voice.Size]
		CMP			EAX,EDI
		JAE			.PosZero
		CALL		GetBaseAddress
		MOV			EDI,EDX
		MOV			ECX,[EDI+Voice.Type]
		XOR			EDX,EDX
		MOV			ECX,[EBX+ECX*4+SizeEchant]
		DIV			ECX
		MUL			ECX
		MOV			[EDI+Voice.Pos],EAX
		JMP			SHORT .PasPosZero
.PosZero:
		MOV			[EDX+Voice.Pos],EAX
.PasPosZero:
		MOV			EAX,1
.End:
		STI

		POP			EDI
		POP			ESI
		POP			EBX
	RETURN

ExistVoice:
	ARG	PtrVoiceSearch, 4
		PUSH		EBX
		PUSH		ESI
		;CLI

		MOV			EDX,[EBP+PtrVoiceSearch]
		CALL		SearchVoiceIn
		OR			EAX,EAX
		JZ			SHORT .exist
		JMP			SHORT .NotExist
.exist:
		MOV			AL,1
.NotExist:

		;STI
		POP			ESI
		POP			EBX
	RETURN

IsPlayingVoice:
	ARG	PlayPtrVoiceSearch, 4
		PUSH		EBX
		PUSH		ESI
		;CLI

		MOV			EDX,[EBP+PlayPtrVoiceSearch]
		CALL		GetBaseAddress
		MOV			ECX,[EBX+MaxNbVoice]
		LEA			ESI,[EBX+VoicePtr]
		JMP			SHORT IsQueuedVoice.BcCmptVc

IsQueuedVoice:
	ARG	QueuePtrVoiceSearch, 4
		PUSH		EBX
		PUSH		ESI
		;CLI

		MOV			EDX,[EBP+QueuePtrVoiceSearch]
		CALL		GetBaseAddress
		MOV			ECX,[EBX+MaxNbVoice]
		LEA			ESI,[EBX+QueueVoicePtr]
.BcCmptVc:
		LODSD
		CMP			EAX,EDX
		JE			.Exist
		DEC			ECX
		JNZ			.BcCmptVc
		JMP			SHORT .NotExist
.Exist:
		LEA			EAX,[1]
		JMP			SHORT .End
.NotExist:
		XOR 		EAX,EAX
.End:

		;STI
		POP			ESI
		POP			EBX
	RETURN

;IN EDX Ptr Vc -----------------
;OUT EAX= 0 found, else not found, EDX voice index found

SearchVoiceIn:
		PUSH		ECX
		PUSH		ESI
		PUSH		EBX

		CALL		GetBaseAddress
		XOR			EAX,EAX ; init found
		OR			EDX,EDX
		MOV			ECX,MAX_VOICES
		JZ			SHORT .Error ; check if null pointer
		LEA			ESI,[EBX+VoicePtr]
.BcSrchVc:
		CMP			EDX,[ESI]
		JE			.VcFnd
		CMP			EDX,[ESI+MAX_VOICES*4] ; search in queue QueueVoicePtr
		JE			.VcFnd
		DEC			ECX
		LEA			ESI,[ESI+4]
		JNZ			.BcSrchVc
		JMP			SHORT .Error
.VcFnd:
		MOV			EDX,MAX_VOICES
		SUB			EDX,ECX ; EDX = index found
		JMP			SHORT .Ok
.Error:
		OR			EAX,BYTE 1
.Ok:
		POP			EBX
		POP			ESI
		POP			ECX
	RET


;******* Lock & Unlock Vc&Vc DATA ******************************************
; EAX Ptr Voice
PrcLockVoice:
		PUSHAD

		CALL		GetBaseAddress
		MOV			ECX,EAX     ; LOW WORD LAddr
		ADD			ECX,[EBX+DSBaseAddress]
		MOV			EBX,ECX     ; HIGH WORD LAddr
		SHR			EBX,16
		MOV			EDI,(Voice.Fin)
		MOV			ESI,(Voice.Fin)>>16
		MOV			AX,0x600
		INT			0x31
		JC			.Error
		POPAD
		XOR			EAX,EAX
		JMP			SHORT .Fin
.Error:
		POPAD
		OR			EAX,BYTE -1
.Fin:

		RET

; EAX Ptr Voice
PrcUnlockVoice:
		PUSHAD

		CALL		GetBaseAddress
		MOV			ECX,EAX     ; LOW WORD LAddr
		ADD			ECX,[EBX+DSBaseAddress]
		MOV			EBX,ECX     ; HIGH WORD LAddr
		SHR			EBX,16
		MOV			EDI,(Voice.Fin)
		MOV			ESI,(Voice.Fin)>>16
		MOV			AX,0x601
		INT			0x31

		POPAD
		RET

; EAX Ptr Voice
PrcLockVoiceData:
		PUSHAD

		CALL		GetBaseAddress
		MOV			ECX,[EAX+Voice.Ptr]     ; LOW WORD LAddr
		OR			ECX,ECX
		JZ			.Error
		ADD			ECX,[EBX+DSBaseAddress]
		MOV			EBX,ECX     ; HIGH WORD LAddr
		SHR			EBX,16
		MOV			EDI,[EAX+Voice.Size]
		MOV			ESI,[EAX+Voice.Size]
		SHR			ESI,16
		MOV			AX,0x600
		INT			0x31
		JC			.Error
		POPAD
		XOR			EAX,EAX
		JMP			SHORT .Fin
.Error:
		POPAD
		OR			EAX,BYTE -1
.Fin:
		RET

; EAX Ptr Voice
PrcUnlockVoiceData:
		PUSHAD

		CALL		GetBaseAddress
		MOV			ECX,[EAX+Voice.Ptr]     ; LOW WORD LAddr
		ADD			ECX,[EBX+DSBaseAddress]
		MOV			EBX,ECX     ; HIGH WORD LAddr
		SHR			EBX,16
		MOV			EDI,[EAX+Voice.Size]
		MOV			ESI,[EAX+Voice.Size]
		SHR			ESI,16
		MOV			AX,0x601
		INT			0x31

		POPAD
		RET

