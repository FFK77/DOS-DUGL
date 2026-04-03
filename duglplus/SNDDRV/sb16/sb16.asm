SECTION .text
[BITS 32]
; DRIVER HEADER ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Signature			DD	"FSDR"		; FLAT SOUND DRIVER
SizeDrv				DD	FinDrv
Capabilities		DD	Out8|Out16|OutMono|OutStereo|In8|In16|InMono|InStereo|FullDuplex|AutoDetect
Capabilities2		DD	0
SizeBuff			DD	SizeDMABuff*8
Min8BitMono			DD	5000
Max8BitMono			DD	45454
Min16BitMono		DD	5000
Max16BitMono		DD	45454
Min8BitStereo		DD	5000
Max8BitStereo		DD	45454
Min16BitStereo		DD	5000
Max16BitStereo		DD	45454
SB_BasePort			DD	0
SB_IRQ				DD	0
SB_DMA8				DD	0
SB_DMA16			DD	0
SB_SampSpeed		DD	0
Error				DD	0
Version				DB	0, 6, 'a', 1; version 0.6 alpha1
resv:				DD	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
					DD	0,0,0,0,0,0,0,0
DrvBuffPtr			DD	0
PtrCardName			DD	CardName
GlobFonc:
					DD	InitDriver,InstallDriver,UninstallDriver
					DD	InitSound,ResetSound,StopSound,ContinueSound
					DD	SetMasterVolume,SetVoiceVolume,SetMidiVolume
					DD	SetCDVolume,SetLineVolume,SetMicVolume
					DD	SetInGain,SetOutGain,SetAutoGain,SetTreble
					DD	SetBass,SetOutput,SetInput
					DD	NS,NS,NS,NS
					DD	NS,NS,NS,NS,NS,NS,NS,NS
					DD	NS,NS,NS,NS,NS,NS,NS,NS
					DD	NS,NS,NS,NS,NS,NS,NS,NS
					DD	NS,NS,NS,NS,NS,NS,NS,NS
					DD	NS,NS,NS,NS,NS,NS,NS,NS
VoiceMixer:
					DD	AddVoice,PrepVoice,UnprepVoice,GetVoiceState
					DD	SetVoiceState,GetVoiceEffPack,SetVoiceEffPack
					DD	GetVoicePos,SetVoicePos,DeleteVoice
					DD	DeleteAllVoices,GetCountVoices
					DD	QueueVoice,ReplaceVoice,ExistVoice
					DD	IsPlayingVoice,IsQueuedVoice,NS,NS,NS,NS,NS,NS,NS
					DD	NS,NS,NS,NS,NS,NS,NS,NS
					DD	NS,NS,NS,NS,NS,NS,NS,NS
					DD	NS,NS,NS,NS,NS,NS,NS,NS
					DD	NS,NS,NS,NS,NS,NS,NS,NS
					DD	NS,NS,NS,NS,NS,NS,NS,NS
					DD	NS,NS,NS,NS,NS,NS,NS,NS
					DD	NS,NS,NS,NS,NS,NS,NS,NS
					DD	NS,NS,NS,NS,NS,NS,NS,NS
					DD	NS,NS,NS,NS,NS,NS,NS,NS
					DD	NS,NS,NS,NS,NS,NS,NS,NS
					DD	NS,NS,NS,NS,NS,NS,NS,NS
					DD	NS,NS,NS,NS,NS,NS,NS,NS
					DD	NS,NS,NS,NS,NS,NS,NS,NS

; CODE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
%include "param.mac"
; CONST ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MAX_VOICES		EQU 32
; ----- Capabilities ----------------
Out8			EQU	1
Out16			EQU	2
OutMono			EQU	4
OutStereo		EQU	8
In8				EQU	16
In16			EQU	32
InMono			EQU	64
InStereo		EQU	128
FullDuplex		EQU	256
AutoDetect		EQU	512
; -----------------------------------
SizeDMABuff		EQU	512   ; pow 2
VolMask			EQU	0x1ff    ; 64 or 0x40 is Vol Norm or 1.0 max 511 or x 5.0
SpeedMask		EQU	0x7ff	 ; 128 or 0x80 is Speed Norm or 1.0 max 2047 or x 16.0
; ----- Effect ----------------------
ChgSpeed		EQU	1
ChgVol			EQU	2
;VcReverse		EQU	4
; ----- Voice State -----------------
VcInBoucle		EQU	1
VcStopped		EQU	2
VcQueued		EQU 4

DebSndLockCode:;*************************************************************

GetBaseAddress:
		MOV		EBX,0
		RET

NS:		; fonction ((N)on (S)upporte) retourne 1 dans EAX
		MOV		EAX,1
		RET

%include "sb.asm"
%include "dma.asm"
%include "vc_mix.asm"
%include "mixer.asm"

ALIGN 32
SB_Handler:
		PUSHF
		PUSHAD
		PUSH		DS
		PUSH		ES

		CALL		GetBaseAddress
		MOV			DS,[CS:EBX+SndMyDSSelector]
		MOV			ES,[EBX+SndMyDSSelector]

		@Read_Mixer 0x82
		TEST		AL,1
		JZ			.PasTrait8Bit
;***************Procede la E/S 8 Bits**********************

		CALL		GetBaseAddress
		MOV			EAX,[EBX+AddrProced8Bit]
		CALL		EAX

;***************FIN Precede E/S 8 Bits*********************
		CALL		GetBaseAddress
		MOV			EAX,[EBX+OffResCalcDMA8]
		ADD			EAX,SizeDMABuff/2
		AND			EAX,SizeDMABuff-1
		MOV			[EBX+OffResCalcDMA8],EAX
		MOV			EDX,[EBX+DSPStateAck8]
		IN			AL,DX               ; acquite 8 Bits
.PasTrait8Bit:

		@Read_Mixer 0x82
		TEST		AL,2
		JZ			.PasTrait16Bit
;***************Procede la E/S 16 Bits**********************

		CALL		GetBaseAddress
		MOV			EAX,[EBX+AddrProced16Bit]
		CALL		EAX

;***************FIN Precede E/S 16 Bits*********************
		CALL		GetBaseAddress
		MOV			EAX,[EBX+OffResCalcDMA16]
		ADD			EAX,SizeDMABuff
		AND			EAX,(SizeDMABuff*2)-1
		MOV			[EBX+OffResCalcDMA16],EAX
		MOV			EDX,[EBX+DSPAck16]
		IN			AL,DX               ; acquite 16 Bits
.PasTrait16Bit:
		MOV			AL,0x20
		CALL		GetBaseAddress
		CMP         BYTE [SbIRQ+EBX],8
		JB          .PasContrl2
		OUT			0xA0,AL
.PasContrl2:
		OUT			0x20,AL

		POP			ES
		POP			DS
		POPAD
		POPF
		STI
		IRET

FinSndLockCode:;*************************************************************

ResetSound:
		PUSH		EBX
		PUSH		ESI
		PUSH		EDI

		; arret SON & transfert DMA ---------------------------------
		@Write_DSP 0xD0
		@Write_DSP 0xDA
		@StopDMA8
		@Write_DSP 0xD5
		@Write_DSP 0xD9
		@StopDMA16
		; Sampling Speed -------------------------------------------
		CALL		GetBaseAddress
		XOR		EAX,EAX
		MOV		[EBX+SB_SampSpeed],EAX
		MOV		[EBX+SbSampSpeed],EAX

		; Efface Tous les voix en cours ou en attente---------------
		MOV		ECX,32*2
		LEA		EDI,[VoicePtr+EBX] ; PtrVoix+BaseAddress
		XOR		EAX,EAX
		REP		STOSD

		POP		EDI
		POP		ESI
		POP		EBX
		RET

StopSound:
		PUSH		EBX
		PUSH		ESI
		PUSH		EDI
		@Write_DSP 0xD0
		@Write_DSP 0xD5
		POP		EDI
		POP		ESI
		POP		EBX
		RET

ContinueSound:
		PUSH		EBX
		PUSH		ESI
		PUSH		EDI
		@Write_DSP 0xD4
		@Write_DSP 0xD6
		POP		EDI
		POP		ESI
		POP		EBX
		RET

SetMasterVolume:
	ARG 	LeftMasterVol, 4, RightMasterVol, 4
		PUSH		EBX

		MOV		ECX,[EBP+LeftMasterVol]
		CMP		ECX,-1
		JE		.PasChangeLeft
		AND		CL,0xF8 ; clear first 3 bits
		@Write_Mixer 	0x30,CL
.PasChangeLeft:
		MOV		ECX,[EBP+RightMasterVol]
		CMP		ECX,-1
		JE		.PasChangeRight
		AND		CL,0xF8 ; clear first 3 bits
		@Write_Mixer 	0x31,CL
.PasChangeRight:
		POP		EBX
		RETURN

; %1 Volume, %2 Port Mixer
%macro @ChangeVal4_7Bit 2
		MOV		ECX,%1
		CMP		ECX,-1
		JE		%%PasChange
		AND		CL,0xF0
		@Write_Mixer 	%2,CL
%%PasChange:
%endmacro

SetVoiceVolume:
	ARG 	LeftVoiceVol, 4, RightVoiceVol, 4
		PUSH		EBX
		@ChangeVal4_7Bit [EBP+LeftVoiceVol],0x32
		@ChangeVal4_7Bit [EBP+RightVoiceVol],0x33
		POP		EBX
		RETURN

SetMidiVolume:
	ARG 	LeftMidiVol, 4, RightMidiVol, 4
		PUSH		EBX
		@ChangeVal4_7Bit [EBP+LeftMidiVol],0x34
		@ChangeVal4_7Bit [EBP+RightMidiVol],0x35
		POP		EBX
		RETURN

SetCDVolume:
	ARG 	LeftCDVol, 4, RightCDVol, 4
		PUSH		EBX
		@ChangeVal4_7Bit [EBP+LeftCDVol],0x36
		@ChangeVal4_7Bit [EBP+RightCDVol],0x37
		POP		EBX
		RETURN

SetLineVolume:
	ARG 	LeftLineVol, 4, RightLineVol, 4
		PUSH		EBX
		@ChangeVal4_7Bit [EBP+LeftLineVol],0x38
		@ChangeVal4_7Bit [EBP+RightLineVol],0x39
		POP		EBX
		RETURN

SetMicVolume:
	ARG 	MicVol, 4
		PUSH		EBX
		@ChangeVal4_7Bit [EBP+MicVol],0x3A
		POP		EBX
		RETURN

%macro @ChangeVal6_7Bit 2
		MOV		ECX,%1
		CMP		ECX,-1
		JE		%%PasChange
		AND		CL,0xC0
		@Write_Mixer 	%2,CL
%%PasChange:
%endmacro

SetInGain:
	ARG 	LeftInGainControl, 4, RightInGainControl, 4
		PUSH		EBX
		@ChangeVal6_7Bit [EBP+LeftInGainControl],0x3F
		@ChangeVal6_7Bit [EBP+RightInGainControl],0x40
		POP		EBX
		RETURN

SetOutGain:
	ARG 	LeftOutGainControl, 4, RightOutGainControl, 4
		PUSH		EBX
		@ChangeVal6_7Bit [EBP+LeftOutGainControl],0x41
		@ChangeVal6_7Bit [EBP+RightOutGainControl],0x42
		POP		EBX
		RETURN

SetAutoGain:
	ARG	AutoGainControl, 4
		PUSH		EBX
		MOV		CL,[EBP+AutoGainControl]
		AND		CL,1
		@Write_Mixer	0x43,CL
		POP		EBX
		RETURN

SetTreble:
	ARG 	LeftTreble, 4, RightTreble, 4
		PUSH		EBX
		@ChangeVal4_7Bit [EBP+LeftTreble],0x44
		@ChangeVal4_7Bit [EBP+RightTreble],0x45
		POP		EBX
		RETURN

SetBass:
	ARG 	LeftBass, 4, RightBass, 4
		PUSH		EBX
		@ChangeVal4_7Bit [EBP+LeftBass],0x46
		@ChangeVal4_7Bit [EBP+RightBass],0x47
		POP		EBX
		RETURN

SetOutput:
	ARG	SelectOutput, 4
		PUSH		EBX
		MOV		CL,[EBP+SelectOutput]
		AND		CL,0x1F
		@Write_Mixer	0x3C,CL
		POP		EBX
		RETURN

SetInput:
	ARG	SelectInputLeft, 4,SelectInputRight, 4
		PUSH		EBX
		MOV		CL,[EBP+SelectInputLeft]
		AND		CL,0x7F
		@Write_Mixer	0x3D,CL
		MOV		CL,[EBP+SelectInputRight]
		AND		CL,0x7F
		@Write_Mixer	0x3E,CL
		POP		EBX
		RETURN

;----------------------------------------------------------------------------
; Type..Bits      : 0, (no sound), 1 Out, 2 In
; GenType         : 0 MONO, 1 STEREO
; SampleSpeed     : Sampling Speed ------------------------------------------

InitSound:
	ARG	Type8Bits, 4, Type16Bits, 4, GenType, 4, SampleSpeed, 4
		PUSH		ESI
		PUSH		EDI
		PUSH		EBX

		CALL		GetBaseAddress	  ; In & Out Type nosound
		MOV		EAX,-1
		MOV		[EBX+InType],EAX
		MOV		[EBX+OutType],EAX
		LEA		ECX,[EBX+NS]
		MOV		[EBX+AddrProced8Bit],ECX   ; non supporte
		MOV		[EBX+AddrProced16Bit],ECX  ; non supporte

		MOV		AL,[EBP+Type8Bits]
		OR		AL,AL
		JZ		.CoupleOk ; le couple (0,0) est permis
		CMP		AL,[EBP+Type16Bits]
		JE		NEAR .InitSndError
.CoupleOk:
		@StopDMA8
		@StopDMA16
		; Efface Les Buffer DMA d'E/S-------------------------------
		PUSH		ES
		CALL		GetBaseAddress
		MOV		EAX,0x80808080
		MOV		ECX,SizeDMABuff/4
		MOV		ES,[EBX+SelectorDMA8]
		XOR		EDI,EDI
		REP		STOSD		   ; Efface Buffer DMA8
		XOR		EAX,EAX
		MOV		ECX,SizeDMABuff/2
		MOV		ES,[EBX+SelectorDMA16]
		XOR		EDI,EDI
		REP		STOSD		   ; Efface Buffer DMA16
		POP		ES
		; Traitement 8 Bits------------------------------------------
		MOV		EAX,[EBP+Type8Bits]
		MOV		DWORD [EBX+State8Bits],0
		OR		EAX,EAX
		JZ		NEAR .No8BitsSound
		CMP		EAX,BYTE 2
		JE		.In8Bit
.Out8Bit:;--------------------------------------------
		@StartDMA8 0x58  ; DMA Write/AutoInit
		@Write_DSP 0x41
		@Write_DSP [EBP+SampleSpeed+1]
		@Write_DSP [EBP+SampleSpeed]
		@Write_DSP 0xC6  ; Out 8 Bit AutoInit

		CMP		[EBP+GenType], BYTE 1
		JE		.StereoOut8Bit
		@Write_DSP 0x00  ; Unsigned MONO
		MOV		DWORD [EBX+OutType],0
		LEA		EAX,[EBX+ProcedOut8BitsM] ; AddrProced ------
		MOV		[EBX+AddrProced8Bit],EAX

		JMP		.Size8Bits
.StereoOut8Bit:
		@Write_DSP 0x20  ; Unsigned STEREO
		MOV		DWORD [EBX+OutType],1
		LEA		EAX,[EBX+ProcedOut8BitsS] ; AddrProced ------
		MOV		[EBX+AddrProced8Bit],EAX
		JMP		.Size8Bits
.In8Bit:;---------------------------------------------
		@StartDMA8 0x54  ; DMA Write/AutoInit
		@Write_DSP 0x42
		@Write_DSP [EBP+SampleSpeed+1]
		@Write_DSP [EBP+SampleSpeed]
		@Write_DSP 0xCE  ; In 8 Bit AutoInit
		CMP		[EBP+GenType], BYTE 1
		JE		.StereoIn8Bit
		@Write_DSP 0x00 ; Unsigned MONO
		MOV		DWORD [EBX+InType],0
		LEA		EAX,[EBX+ProcedIn8BitsM] ; AddrProced -------
		MOV		[EBX+AddrProced8Bit],EAX
		JMP		.Size8Bits
.StereoIn8Bit:
		@Write_DSP 0x20  ; Unsigned STEREO
		MOV		DWORD [EBX+InType],1
		LEA		EAX,[EBX+ProcedIn8BitsS] ; AddrProced -------
		MOV		[EBX+AddrProced8Bit],EAX

.Size8Bits:	@Write_DSP (( (SizeDMABuff/2)-1)&0xff)
		@Write_DSP ((( (SizeDMABuff/2)-1)>>8)&0xff)
		MOV		DWORD [EBX+OffResCalcDMA8],0
.No8BitsSound:

		; Traitement 16 Bits-----------------------------------------
		MOV		EAX,[EBP+Type16Bits]
		MOV		DWORD [EBX+State16Bits],0
		OR		EAX,EAX
		JZ		NEAR .No16BitsSound
		CMP		EAX,BYTE 2
		JE		NEAR .In16Bit
.Out16Bit:;--------------------------------------------
		@StartDMA16 0x58  ; DMA Read/AutoInit
		@Write_DSP 0x41
		@Write_DSP [EBP+SampleSpeed+1]
		@Write_DSP [EBP+SampleSpeed]
		@Write_DSP 0xB6  ; Out 16 Bit AutoInit

		CMP		[EBP+GenType], BYTE 1
		JE		.StereoOut16Bit
		@Write_DSP 0x10  ; Signed MONO
		MOV		DWORD [EBX+State16Bits],1
		MOV		DWORD [EBX+OutType],2
		LEA		EAX,[EBX+ProcedOut16BitsM] ; AddrProced ------
		MOV		[EBX+AddrProced16Bit],EAX
		JMP		.Size16Bits
.StereoOut16Bit:
		@Write_DSP 0x30  ; Signed STEREO
		MOV		DWORD [EBX+State16Bits],2
		MOV		DWORD [EBX+OutType],3
		LEA		EAX,[EBX+ProcedOut16BitsS] ; AddrProced ------
		MOV		[EBX+AddrProced16Bit],EAX
		JMP		.Size16Bits
.In16Bit:;---------------------------------------------
		@StartDMA16 0x54  ; Ecriture AutoInit
		@Write_DSP 0x42
		@Write_DSP [EBP+SampleSpeed+1]
		@Write_DSP [EBP+SampleSpeed]
		@Write_DSP 0xBE  ; In 16 Bit AutoInit
		CMP		[EBP+GenType], BYTE 1
		JE		.StereoIn16Bit
		@Write_DSP 0x10  ; Signed MONO
		MOV		DWORD [EBX+State16Bits],3
		MOV		DWORD [EBX+InType],2
		LEA		EAX,[EBX+ProcedIn16BitsM] ; AddrProced -------
		MOV		[EBX+AddrProced16Bit],EAX
		JMP		.Size16Bits
.StereoIn16Bit:
		@Write_DSP 0x30  ; Signed STEREO
		MOV		DWORD [EBX+State16Bits],4
		MOV		DWORD [EBX+InType],3
		LEA		EAX,[EBX+ProcedIn16BitsS] ; AddrProced -------
		MOV		[EBX+AddrProced16Bit],EAX

.Size16Bits:
		@Write_DSP (( (SizeDMABuff/2)-1)&0xff)
		@Write_DSP ((( (SizeDMABuff/2)-1)>>8)&0xff)
		MOV		DWORD [EBX+OffResCalcDMA16],0
.No16BitsSound:
		; Sampling Speed -------------------------------------------
		MOV		EAX,[EBP+SampleSpeed]
		MOV		[EBX+SB_SampSpeed],EAX
		MOV		[EBX+SbSampSpeed],EAX

		; Efface Tous les voix en cours ou en attente---------------
		MOV		ECX,MAX_VOICES*2
		LEA		EDI,[VoicePtr+EBX] ; PtrVoix+BaseAddress
		XOR		EAX,EAX
		REP		STOSD

		JMP		SHORT .InitSndOk
.InitSndError:
		XOR		EAX,EAX
		JMP		SHORT .PasInitSOk
.InitSndOk:
		CALL	StopSound
		OR		EAX,BYTE -1
.PasInitSOk:
		POP		EBX
		POP		EDI
		POP		ESI
		RETURN

InitDriver:
		PUSH		EBP
		PUSH		EBX
		PUSH		ESI
		PUSH		EDI

		; Patch des adresses des fonctions--- ***** -----------------
		; et de la procedure GetBaseAddress--------------------------
		CALL		.BaseAdressEIP  ; Base adress dans la pile
.BaseAdressEIP:	POP		EBX
		SUB		EBX,.BaseAdressEIP
		MOV		[EBX+GetBaseAddress+1],EBX
		; patche l'instruction  MOV EBX,(0 -> adress base)
		LEA		ESI,[EBX+PtrCardName]
		MOV		ECX,193
.BcPatchAdd:	ADD		[ESI],EBX
		DEC		ECX
		LEA		ESI,[ESI+4]
		JNZ		.BcPatchAdd

		; Patche les adresses des fonction d'addition des voix
		; au buffer de calcul
		LEA		ESI,[EBX+AddrProcAddBCalc]
		MOV		ECX,16+4
.BcPatchAddBCl:	ADD		[ESI],EBX
		DEC		ECX
		LEA		ESI,[ESI+4]
		JNZ		.BcPatchAddBCl

		POP		EDI
		POP		ESI
		POP		EBX
		POP		EBP
		RET

InstallDriver:
	ARG	PtrBuff, 4, Base, 4, IRQ, 4, DMA8, 4, DMA16, 4
		PUSH		ESI
		PUSH		EDI
		PUSH		EBX

		; Detection de la carte SB16---------------------------------
		CMP		DWORD [EBP+Base], -1
		JE		.AutoDetectBase
		MOV		EDI,[EBP+Base]
		@DetectSBBase 	EDI
		OR		EAX,EAX
		JNZ		.SbDetected
		JMP		SHORT .SbNotDetected
.AutoDetectBase:
		MOV		EDI,0x210; EDI pas utiliser par @DetectSBBase
.BcDtctBase:	@DetectSBBase 	EDI
		OR		EAX,EAX
		JNZ		.SbDetected
		ADD		EDI,BYTE 0x10
		CMP		EDI,0x270
		JNE		.PasIncDetect
		ADD		EDI,BYTE 0x10
.PasIncDetect:
		CMP		EDI,0x290
		JNE		.BcDtctBase
.SbNotDetected:	JMP		.SoundError
.SbDetected:	CALL		GetBaseAddress
		MOV		[EBX+SbBase],EDI
		MOV		[EBX+SB_BasePort],EDI
		LEA		ECX,[EDI+0x4]
		MOV		[EBX+MixerIndex],ECX
		LEA		ECX,[EDI+0x5]
		MOV		[EBX+MixerData],ECX
		LEA		ECX,[EDI+0x6]
		MOV		[EBX+DSPReset],ECX
		LEA		ECX,[EDI+0xA]
		MOV		[EBX+DSPData],ECX
		LEA		ECX,[EDI+0xC]
		MOV		[EBX+DSPStateCommand],ECX
		LEA		ECX,[EDI+0xE]
		MOV		[EBX+DSPStateAck8],ECX
		LEA		ECX,[EDI+0xF]
		MOV		[EBX+DSPAck16],ECX
		@Write_DSP 	0xE1	 ; Verification de la version
		@Read_DSP		 ; de la carte SB16
		MOV		AH,AL
		@Read_DSP
		CMP		AH,4	 ; version < 4 => SB Pro ou moin
		JB		NEAR .SoundError
		; Initialise Mixer
		@Write_Mixer 0,0
		; detecte IRQ
		@Read_Mixer 	0x80
		MOV		AH,2
		TEST		AL,1
		JNZ		.IRQDetected
		MOV		AH,5
		TEST		AL,2
		JNZ		.IRQDetected
		MOV		AH,7
		TEST		AL,4
		JNZ		.IRQDetected
		MOV		AH,10
		TEST		AL,8
		JNZ		.IRQDetected
.IRQNtDetected:
		JMP		.SoundError
.IRQDetected:	MOVZX		EAX,AH
		CALL		GetBaseAddress
		MOV		[EBX+SB_IRQ],EAX
		MOV		[EBX+SbIRQ],EAX
		; detecte DMA8
		@Read_Mixer 	0x81
		MOV		AH,0
		TEST		AL,1
		JNZ		.DMA8Detect
		MOV		AH,1
		TEST		AL,2
		JNZ		.DMA8Detect
		MOV		AH,3
		TEST		AL,8
		JNZ		.DMA8Detect
.DMA8NtDetect:
		JMP		.SoundError
.DMA8Detect:	MOVZX		EAX,AH
		CALL		GetBaseAddress
		MOV		[EBX+SB_DMA8],EAX
		MOV		[EBX+SbDMA8],EAX
		; detecte DMA16
		@Read_Mixer 	0x81
		MOV		AH,5
		TEST		AL,32
		JNZ		.DMA16Detect
		MOV		AH,6
		TEST		AL,64
		JNZ		.DMA16Detect
		MOV		AH,7
		TEST		AL,128
		JNZ		.DMA16Detect
.DMA16NtDetect:
		JMP		.SoundError
.DMA16Detect:	MOVZX		EAX,AH
		CALL		GetBaseAddress

		MOV		[EBX+SB_DMA16],EAX
		MOV		[EBX+SbDMA16],EAX
		; Allocation Des Buffers DMA---------------------------------

		@AllocDOSMemDMA8 SizeDMABuff
		OR		EAX,EAX
		JZ		NEAR .SoundError

		@AllocDOSMemDMA16 SizeDMABuff*2
		OR		EAX,EAX
		JNZ		NEAR .MemDMA16Ok
		CALL		GetBaseAddress
		MOV		AX,0x101
		MOV		EDX,[EBX+SelectorDMA8]
		INT		0x31
		JMP		.SoundError
.MemDMA16Ok:
		CALL		GetBaseAddress
		MOV		EAX,[EBX+PhysAddDMA8]
		MOV		ECX,[EBX+PhysAddDMA16]
		MOV		ESI,EAX
		MOV		EDI,ECX
		AND		EAX,0xffff
		SHR		ECX,1
		AND		ECX,0xffff
		MOV		[EBX+OffDMA8],EAX
		MOV		[EBX+OffDMA16],ECX
		SHR		ESI,16
		SHR		EDI,16
		MOV		[EBX+PageDMA8],ESI
		MOV		[EBX+PageDMA16],EDI

		; Installe nouveau IRQ Handler-------------------------------
		CLI

		CALL		GetBaseAddress
		MOV		AX,0x204 ; Get Old SB_IRQ Handler
		MOV		EBX,[EBX+SbIRQ]
		CMP		BL,8
		JAE		.GetPIC2
		ADD		BL,8
		JMP		SHORT .GetPIC1
.GetPIC2:	ADD		BL,0x70-8
.GetPIC1:
		INT		0x31
		CALL		GetBaseAddress

		MOV		[EBX+OldHdSndOff],EDX
		MOV		[EBX+OldHdSndSeg],ECX

		XOR		EAX,EAX
		IN		AL,0x21
		MOV		[EBX+Old0x21],EAX
		IN		AL,0xA1
		MOV		[EBX+Old0xA1],EAX
		MOV		EDI,[EBX+SbIRQ]
		XOR		ESI,ESI    ; 0x21
		XOR		EDX,EDX    ; 0xA1
		MOV		ECX,EDI
		CMP		CL,8
		JAE		.IRQContr2
.IRQContr1:	MOV		ESI,1
		SHL		ESI,CL
		JMP		SHORT .PasContr2
.IRQContr2:	SUB		ECX,8
		MOV		ESI,4
		MOV		EDX,1
		SHL		EDX,CL
.PasContr2:	NOT		EDX
		NOT		ESI
		AND		EDX,[EBX+Old0xA1]
		AND		ESI,[EBX+Old0x21]
		MOV		EAX,EDX
		OUT		0xA1,AL
		MOV		EAX,ESI
		OUT		0x21,AL

		MOV		AX,0x205 ; Set New SB_IRQ Handler
		MOV		EDX,SB_Handler  ; offset SB_Handler
		ADD		EDX,EBX         ; + Base Address
		MOV		EBX,[EBX+SbIRQ]
		CMP		BL,8
		JAE		.InstPIC2
		ADD		BL,8
		JMP		SHORT .InstPIC1
.InstPIC2:
		ADD		BL,0x70-8
.InstPIC1:
		MOV		ECX,CS
		INT		0x31
		JNC		.SetHandlerOk
		CALL		GetBaseAddress
		MOV		EAX,[EBX+Old0x21]
		OUT		0x21,AL
		MOV		EAX,[EBX+Old0xA1]
		OUT		0xA1,AL
		STI
		JMP		.SoundError
.SetHandlerOk:	STI
		; Get DS Base Address----------------------------------------
		MOV		AX,6
		MOV		BX,DS
		INT		0x31
		SHL		ECX,16
		AND		EDX,0xffff
		CALL		GetBaseAddress
		OR 		EDX,ECX
		MOV		[EBX+DSBaseAddress],EDX
		MOV		EAX,DS	     ; sauvegarde DS
		MOV		[EBX+SndMyDSSelector],EAX
		; lock code--------------------------------------------------
		MOV		AX,0x600
		MOV		ECX,DebSndLockCode     ; LOW WORD LAddr
		ADD		ECX,[EBX+DSBaseAddress]
		ADD		ECX,EBX ; + Base Address
		MOV		EBX,ECX     ; HIGH WORD LAddr
		SHR		EBX,16
		MOV		EDI,(FinSndLockCode-DebSndLockCode+1)
		MOV		ESI,(FinSndLockCode-DebSndLockCode+1)>>16
		INT		0x31
		JC		NEAR .SoundError
		; lock data--------------------------------------------------
		CALL		GetBaseAddress
		MOV		AX,0x600
		MOV		ECX,DebSndLockData     ; LOW WORD LAddr
		ADD		ECX,[EBX+DSBaseAddress]
		ADD		ECX,EBX ; + Base Address
		MOV		EBX,ECX     ; HIGH WORD LAddr
		SHR		EBX,16
		MOV		EDI,(FinSndLockData-DebSndLockData+1)
		MOV		ESI,((FinSndLockData-DebSndLockData+1)>>16)
		INT		0x31
		JC		NEAR .SoundError
		; lock Buff--------------------------------------------------
		CALL		GetBaseAddress
		MOV		AX,0x600
		MOV		ECX,[EBP+PtrBuff]    ; LOW WORD LAddr
		MOV		[EBX+BuffPtr],ECX    ; sauvegarde l'adresse
		MOV		[EBX+BuffCalc8Bit],ECX    ; sauvegarde l'adresse
		ADD		ECX,SizeDMABuff*2
		MOV		[EBX+TempVcCopyBuff],ECX
		ADD		ECX,SizeDMABuff*2
		MOV		[EBX+BuffCalc16Bit],ECX    ; sauvegarde l'adresse
		ADD		ECX,SizeDMABuff*2
		MOV		[EBX+EffectVcBuff],ECX

		MOV		ECX,[EBX+BuffPtr]
		ADD		ECX,[EBX+DSBaseAddress]
		MOV		EDI,[EBX+SizeBuff]
		MOV		ESI,[EBX+SizeBuff]
		SHR		ESI,16
		MOV		EBX,ECX     ; HIGH WORD LAddr
		SHR		EBX,16
		INT		0x31
		JC		NEAR .SoundError

		JMP		SHORT .SoundOk
.SoundError:	XOR		EAX,EAX
		JMP		NEAR .PasSndOk
.SoundOk:	OR		EAX,BYTE -1
.PasSndOk:
		POP		EBX
		POP		EDI
		POP		ESI
		RETURN

UninstallDriver:
		PUSH		ESI
		PUSH		EDI
		PUSH		EBX

		; stop sound &  DMA transfert -------------------------------
		@Write_DSP 0xD0
		@Write_DSP 0xDA
		@StopDMA8
		@Write_DSP 0xD5
		@Write_DSP 0xD9
		@StopDMA16
		; restore old interrupt handler SB_IRQ ----------------------
		CLI
		CALL		GetBaseAddress
		MOV		EAX,[EBX+Old0xA1]
		OUT		0xA1,AL
		MOV		EAX,[EBX+Old0x21]
		OUT		0x21,AL

		MOV		AX,0x205 ; Restore Old SB_IRQ Handler
		MOV		EDX,[EBX+OldHdSndOff]
		MOV		ECX,[EBX+OldHdSndSeg]
		MOV		EBX,[EBX+SbIRQ]
		CMP		EBX,8
		JAE		.InstPIC2
		ADD		EBX,8
		JMP		SHORT .InstPIC1
.InstPIC2:	ADD		EBX,0x70-8
.InstPIC1:
		INT		0x31

		STI

		; free DMA buffers ------------------------------------------
		CALL		GetBaseAddress
		MOV		AX,0x101
		MOV		EDX,[EBX+SelectorDMA8]
		INT		0x31
		CALL		GetBaseAddress
		MOV		AX,0x101
		MOV		EDX,[EBX+SelectorDMA16]
		INT		0x31
		; unlock code------------------------------------------------
		CALL		GetBaseAddress
		MOV		AX,0x601
		MOV		ECX,DebSndLockCode     ; LOW WORD LAddr
		ADD		ECX,[EBX+DSBaseAddress]
		ADD		ECX,EBX ; + Base Address
		MOV		EBX,ECX     ; HIGH WORD LAddr
		SHR		EBX,16
		MOV		EDI,(FinSndLockCode-DebSndLockCode+1)
		MOV		ESI,(FinSndLockCode-DebSndLockCode+1)>>16
		INT		0x31
		; unlock data------------------------------------------------
		CALL		GetBaseAddress
		MOV		AX,0x601
		MOV		ECX,DebSndLockCode     ; LOW WORD LAddr
		ADD		ECX,[EBX+DSBaseAddress]
		ADD		ECX,EBX ; + Base Address
		MOV		EBX,ECX     ; HIGH WORD LAddr
		SHR		EBX,16
		MOV		EDI,(FinSndLockData-DebSndLockData+1)
		MOV		ESI,(FinSndLockData-DebSndLockData+1)>>16
		INT		0x31
		; unlock Buff------------------------------------------------
		CALL		GetBaseAddress
		MOV		AX,0x601
		MOV		ECX,[EBX+BuffPtr]    ; LOW WORD LAddr
		ADD		ECX,[EBX+DSBaseAddress]
		MOV		EDI,[EBX+SizeBuff]
		MOV		ESI,[EBX+SizeBuff]
		SHR		ESI,16
		MOV		EBX,ECX     ; HIGH WORD LAddr
		SHR		EBX,16
		INT		0x31

		POP		EBX
		POP		EDI
		POP		ESI
		RET


SECTION .data
; DONNEE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OldHdSndOff			DD	0
OldHdSndSeg			DD	0
DSBaseAddress		DD	0
Old0x21				DD	0
Old0xA1				DD	0
DebSndLockData:;*************************************************************
SndMyDSSelector		DD	0
BuffPtr				DD	0
PhysAddDMA8			DD	0
SelectorDMA8		DD	0
OffDMA8				DD	0
PageDMA8			DD	0
OffResCalcDMA8		DD	0
PhysAddDMA16		DD	0
SelectorDMA16		DD	0
OffDMA16			DD	0
PageDMA16			DD	0
OffResCalcDMA16		DD	0
SbBase				DD	0
SbIRQ				DD	0
SbDMA8				DD	0
SbDMA16 			DD	0
SbSampSpeed			DD	0
MixerIndex			DD	0
MixerData			DD	0
DSPReset			DD	0
DSPData				DD	0
DSPStateCommand		DD	0
DSPStateAck8		DD	0
DSPAck16			DD	0
State16Bits			DD	0
State8Bits			DD	0
OutType				DD	0
InType				DD	0
BuffCalc8Bit		DD	0
BuffCalc16Bit		DD	0
TempVcCopyBuff		DD	0
EffectVcBuff		DD	0
;-----Pointeur Fonction --
AddrProced8Bit		DD	0
AddrProced16Bit		DD	0
;------Voices Data Out ---
MaxNbVoice			DD	MAX_VOICES
VoicePtr:			TIMES MAX_VOICES DD 0
QueueVoicePtr:		TIMES MAX_VOICES DD 0
;------Voice Data In -----
RecordState			DD	0
RecordBuffPtr		DD	0
RecordNbBloc		DD	0
RecordSizeBloc		DD	0
Record1stBlocPtr	DD	0
;------Adresse de fonction d'addition au Buffer de calcul
AddrProcAddBCalc:
AddrAdd_BCalc8BitM:
					DD	AddVc8BitM_BCalc8BitM,AddVc8BitS_BCalc8BitM
					DD	AddVc16BitM_BCalc8BitM,AddVc16BitS_BCalc8BitM
AddrAdd_BCalc8BitS:
					DD	AddVc8BitM_BCalc8BitS,AddVc8BitS_BCalc8BitS
					DD	AddVc16BitM_BCalc8BitS,AddVc16BitS_BCalc8BitS
AddrAdd_BCalc16BitM:
					DD	AddVc8BitM_BCalc16BitM,AddVc8BitS_BCalc16BitM
					DD	AddVc16BitM_BCalc16BitM,AddVc16BitS_BCalc16BitM
AddrAdd_BCalc16BitS:
					DD	AddVc8BitM_BCalc16BitS,AddVc8BitS_BCalc16BitS
					DD	AddVc16BitM_BCalc16BitS,AddVc16BitS_BCalc16BitS
BuffVc_SpeedBuffVc:
					DD	Buff8BitM_SpeedBuff8BitM
					DD	Buff8BitS_SpeedBuff8BitS
					DD	Buff16BitM_SpeedBuff16BitM
					DD	Buff16BitS_SpeedBuff16BitS
SizeIncVc_BCalc:
SizeIncVc_BCalc8BitM:
					DD	SizeDMABuff/2,SizeDMABuff
					DD	SizeDMABuff,SizeDMABuff*2
SizeIncVc_BCalc8BitS:
					DD	SizeDMABuff/4,SizeDMABuff/2
					DD	SizeDMABuff/2,SizeDMABuff
SizeIncVc_BCalc16BitM:
					DD	SizeDMABuff/2,SizeDMABuff
					DD	SizeDMABuff,SizeDMABuff*2
SizeIncVc_BCalc16BitS:
					DD	SizeDMABuff/4,SizeDMABuff/2
					DD	SizeDMABuff/2,SizeDMABuff
SizeEchant:
					DD	1,2,2,4
FinSndLockData:;*************************************************************

; NOM DE LA CARTE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CardName		DB	"Sound Blaster 16 or 100% compatible",0
FinDrv:

