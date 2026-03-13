;---------------------------------------------------------------------------
%macro @Write_DSP 1
		MOV		AL,%1
		CALL		Write_DSP
%endmacro

Write_DSP:
		MOV			EDI,EAX
		XOR			ESI,ESI

		CALL		GetBaseAddress
.InitW:
		MOV			ECX,0x30000
		MOV       	EDX,[EBX+DSPStateCommand]
.BoucleW:
		DEC			ECX
		JS			.AutreEssai
		IN			AL,DX
		OR			AL,AL
		JS			.BoucleW
.Ecrire_DSP:
		MOV			EAX,EDI
		MOV       	EDX,[EBX+DSPStateCommand]
		OUT			DX,AL
		RET

.AutreEssai:
		OR			ESI,ESI
		JNZ       	.Ecrire_DSP
		INC       	ESI
		MOV			EDX,[EBX+DSPStateAck8]
		IN			AL,DX
		JMP			SHORT .InitW
		RET

;---------------------------------------------------------------------------
%macro @Read_DSP 0
		CALL      	Read_DSP
%endmacro

Read_DSP:
		CALL		GetBaseAddress
		MOV			EDX,[EBX+DSPStateAck8]

		MOV			ECX,0x30000
.BoucleR:
		IN			AL,DX
		OR			AL,AL
		JS			.Lire_DSP
		DEC			ECX
		JNZ			.BoucleR
		JMP			SHORT .PasLire
.Lire_DSP:
		MOV			EDX,[EBX+DSPData]
		IN			AL,DX
.PasLire:
		RET

;---------------------------------------------------------------------------
%macro @DetectSBBase 1
		MOV			EAX,%1
		CALL		DetectSBBase
%endmacro

DetectSBBase:
		LEA			EDX,[EAX+6]
		MOV			ESI,EAX
		MOV			AL,1
		OUT			DX,AL
		MOV			CL,14
.BcWait:
		IN			AL,DX
		DEC			CL
		JNZ			.BcWait
		MOV			AL,0
		OUT			DX,AL
		MOV			ECX,0x10000
.BcDetect:
		LEA			EDX,[ESI+0xE]
		IN			AL,DX
		OR			AL,AL
		JNS			.PasLire0xA
		LEA			EDX,[ESI+0xA]
		IN			AL,DX
		CMP			AL,0xAA
		JE			.SbDetected
.PasLire0xA:
		DEC			ECX
		JNZ			.BcDetect
.SbNotDetected:
		XOR			EAX,EAX
		RET
.SbDetected:
		OR			EAX,BYTE -1
		RET
;--------------------------------------------------------------------------

%macro @Write_Mixer 2	; %1 Port, %2 Val
		CALL		GetBaseAddress
		MOV			EDX,[EBX+MixerIndex]
		MOV			AL,%1
		OUT			DX,AL
		MOV			AL,%2
		INC			EDX
		OUT			DX,AL
%endmacro

%macro @Read_Mixer 1	; %1 Port
		CALL		GetBaseAddress
		MOV			EDX,[EBX+MixerIndex]
		MOV			AL,%1
		OUT			DX,AL
		INC			EDX
		IN			AL,DX
%endmacro


