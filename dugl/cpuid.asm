%include "PARAM.MAC"

; GLOBAL Function
GLOBAL  _DetectCPUID,_ExecCPUID,_EnableMTRR

; GLOBAL DATA
GLOBAL	_OldCachMode

; EXTERN DATA

ALIGN 32
SECTION .text
[BITS 32]

_DetectCPUID:
		PUSHFD
		POP		EAX
		MOV		ECX,EAX
		XOR		EAX,0x200000
		PUSH		EAX
		POPFD
		PUSHFD
		POP		EAX
		XOR		EAX,ECX
		SETNZ		AL
		FRETURN

_ExecCPUID:
	ARG	ValEAX, 4, PtEAX, 4, PtEBX, 4, PtECX, 4, PtEDX, 4
	        PUSH		ESI
	        PUSH		EBX
	        PUSH		EDI

		MOV		EAX,[EBP+ValEAX]
		XOR		EBX,EBX
		XOR		ECX,ECX
		XOR		EDX,EDX
		CPUID
		MOV		ESI,[EBP+PtEAX]
		MOV		EDI,[EBP+PtEBX]
		MOV		[ESI],EAX
		MOV		[EDI],EBX
		MOV		ESI,[EBP+PtECX]
		MOV		EDI,[EBP+PtEDX]
		MOV		[ESI],ECX
		MOV		[EDI],EDX

		POP		EDI
		POP		EBX
		POP		ESI
	RETURN

_EnableMTRR:
		push	ebx
		mov	eax,0x1
		cpuid
		xor	cl,cl
		test	edx,0x20
		jz	fin
		mov	eax,0x1
		cpuid
		xor	cl,cl
		test	edx,0x1000
		jz	fin
		mov	ecx,0xfe
		rdmsr
		test	eax,0x400
		jz	fin
		mov	ecx,0x2ff
		rdmsr
		cli
		wbinvd
		mov	[_OldCachMode],al
		mov	al,1
		wrmsr
		sti
		mov	cl,1
fin:
		movzx	eax,cl
		pop	ebx
		FRETURN


ALIGN 32
SECTION	.data
OLD_EBX		DD	0
OLD_ESI		DD	0
OLD_EDI		DD	0
OLD_ECX		DD	0
_OldCachMode	DB	0,0,0,0
