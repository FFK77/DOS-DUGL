#include <dos.h>
#include <dpmi.h>
#include <dos.h>
#include <go32.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <crt0.h>
#include <unistd.h>
#include <string.h>
#include <sys/movedata.h>
#include <sys/segments.h>
#include <sys/nearptr.h>
#include "DUGL.h"
#include "intrdugl.h"

// used to convert color
// lookup table to convert from 8bits paletted BGRA to true color 15,16, 24, 32 bits
// and from 15bits true color to 8bits paletted
unsigned short      Col8To15bpp[256];
unsigned short      Col8To16bpp[256];
unsigned int        Col8To32bpp[256];
unsigned char       Col15To8bpp[256*128];

// *************
unsigned char		CurPalette[1024];
int			        WindowControlPMI=0,ViewAddressPMI=0,SetPalPMI=0;
int			        CurModeVtFreq=0,NbVDgSurf=0,NbDgfxModes=0;
unsigned int 		addr;
unsigned int     	lfb,Sizelfb,SizeVMem;
void			    *VesaPMI=0;
unsigned short		SizeVesaPMI=0;
short              	dgfxMode = 0;
unsigned short		SelMPIO,
                    SizeMPIO;
unsigned int		AddMPIO,
                    MAPAddMPIO,
                    EnableMPIO;

DgSurf			    *VSurf=0;
ModeInfo		    *TbDgfxModes = NULL;
ModeInfo		    *CurDgfxMode = NULL;
__dpmi_meminfo	    dpinf;
VesaIntro 		    VesaInt;

char				ShiftPal=0,VesaPMIOk=0,MTRRa=0,Buff_Accel=0;
unsigned char		VesaHiVers,VesaLoVers;
bool				forceBankedVESA = false;
bool				curModeVideo = false;
bool				curVideoBanked = false;
bool 				enableDoubleBuff = false;

void (*ViewSurf)(int NSurf);
void (*ViewSurfWaitVR)(int NSurf);
void (*SetPalette)(int Dbcol, int Nbcol, void *Tcol);

int  DetectMMX();
int  EnableMTRR();
int EnableVesaMTRR();
void InitVesaPMI();
void GetPaletteDAC();
void RealSetPalette(int Dbcol, int Nbcol, void *Tcol);
void RealViewSurfSched(int NbSurf);
void RealViewSurf(int NbSurf);
void RealViewSurfWaitVR(int NbSurf);
void ProtectSetPalette(int Dbcol, int Nbcol, char* Tcol);
void ProtectViewSurf(int NbSurf);
void ProtectViewSurfWaitVR(int NbSurf);

void SetForceBankedVESA(bool bForceBank) {
	forceBankedVESA = bForceBank;
}

bool GetForceBankedVESA() {
	return forceBankedVESA;
}

// standard VESA Mode
#define SIZE_STD_VBE8BPP  5
#define SIZE_STD_VBE16BPP 5
int std8bppVESAMode[SIZE_STD_VBE8BPP]=
//640x400-640x480-800x600-1024x768-1280x1024
  { 0x100,0x101,0x103,0x105,0x107 }; // 8bpp
int std16bppVESAMode[SIZE_STD_VBE16BPP]=
//320x200-640x480-800x600-1024x768-1280x1024
  { 0x10E,0x111,0x114,0x117,0x11A }; // 16bpp

int DgInit() {
	VesaInfo VesaInf;
	__dpmi_regs r;
	unsigned int i,j,CptMode,curMode,LimitDS,BaseDS,TbMemCopy=1000,TbModeCopy=512;
	char Error=0;
	short *ListMode;

	if (!DetectMMX())
		return 0;
	if (!InitDWorkers(0))
		return 0;

	bzero(&r,sizeof(__dpmi_regs));
   	r.d.eax = 0x4f00;
	r.x.es = (__tb>>4)  & 0xffff;
	r.d.edi = __tb & 0xf;
   	__dpmi_int(0x10, &r);
	if (r.h.al!=0x4f || r.h.ah!=0) return 0;
	dosmemget(__tb,sizeof(VesaIntro),&VesaInt);
	if ((ListMode=(short *)malloc(1024*sizeof(short)))==NULL) return 0;

	if ((VesaInt.VideoPtr&0xFFFF)>(0xffff-1024)) {
	  TbMemCopy=0xffff-(VesaInt.VideoPtr&0xFFFF);
	  TbModeCopy=TbMemCopy/2;
    }
	dosmemget(((VesaInt.VideoPtr&0xffff0000)>>12)+
	             (VesaInt.VideoPtr&0xFFFF), TbMemCopy,ListMode);
	// check VESA signature
	if ((VesaInt.Sign!='ASEV'))
        return 0;
    // minimum VESA 1.2 required
	if (VesaInt.HiVers < 1 || (VesaInt.HiVers == 1 && VesaInt.LoVers <2))
        return 0;

	VesaHiVers=VesaInt.HiVers;
	VesaLoVers=VesaInt.LoVers;

	bzero(&r,sizeof(__dpmi_regs));
	r.d.eax = 0x4f01;
	r.d.ecx = 0x101;
	r.x.es = (__tb>>4) & 0xffff;
	r.d.edi = __tb & 0xf;
   	__dpmi_int(0x10, &r);
	if (r.h.al!=0x4f || r.h.ah!=0)  return 0;
	dosmemget(__tb,sizeof(VesaInfo),&VesaInf);


   	if (VesaHiVers >= 2) {
		dpinf.address=VesaInf.PhysBasePtr; // map LFB
		dpinf.size=VesaInt.Memory*64*1024;
		if (__dpmi_physical_address_mapping(&dpinf))  Error=1;
		if (__dpmi_get_segment_base_address(_my_ds(),&BaseDS)==-1) Error=1;
		LimitDS=dpinf.address+dpinf.size-1-BaseDS;
		if (__dpmi_set_segment_limit(_my_ds(),LimitDS)==-1) Error=1;
		lfb=dpinf.address-BaseDS;
   	}
	if (!__djgpp_nearptr_enable())
	  _crt0_startup_flags |=_CRT0_FLAG_NEARPTR;
	SizeVMem=Sizelfb=VesaInt.Memory*64*1024;

	if (Error) { free(ListMode); return 0; }
	i=0; while (ListMode[i]!=-1 && i<TbModeCopy) i++;
	NbDgfxModes=i;
	// add standard 8bpp VESA mode if they does not exist
	CptMode=0;
	for (j=0;j<SIZE_STD_VBE8BPP;j++) {
	  curMode=std8bppVESAMode[j];
	  for (i=0;i<NbDgfxModes;i++) {
	    if (ListMode[i]==curMode) { CptMode = curMode; }
	  }
	  if (CptMode!=curMode) {
	    ListMode[NbDgfxModes]=curMode; NbDgfxModes++; }
	}
	// add standard 16bpp VESA mode if they does not exist
	CptMode=0;
	for (j=0;j<SIZE_STD_VBE16BPP;j++) {
	  curMode=std16bppVESAMode[j];
	  for (i=0;i<NbDgfxModes;i++) {
	    if (ListMode[i]==curMode) { CptMode = curMode; }
	  }
	  if (CptMode!=curMode) {
	    ListMode[NbDgfxModes]=curMode; NbDgfxModes++; }
	}

	if ((TbDgfxModes=(ModeInfo *)malloc(sizeof(ModeInfo)*(NbDgfxModes+2)))==NULL)
	  return 0;

	for (CptMode=i=0;i<NbDgfxModes;i++) {
        bzero(&r,sizeof(__dpmi_regs));
	    r.x.ax = 0x4f01;
	    r.x.cx = ListMode[i];
	    r.x.es = (__tb>>4) & 0xffff;
	    r.d.edi = __tb & 0xf;
     	__dpmi_int(0x10, &r);
	    dosmemget(__tb,sizeof(VesaInfo),&VesaInf);
	     if ( (VesaInf.ModeFlag& VMODE_SUPPORTED)    &&
	          (VesaInf.ModeFlag& VMODE_COLOR)   &&
	          (VesaInf.ModeFlag& VMODE_GRAPHIC)   &&
	          ((VesaInf.ModeFlag& VMODE_LFB) > 0 || (VesaInf.ModeFlag&VMODE_BANKED) == 0)     &&
	          ((VesaInf.BitPixel==8) ||       // 8 Bpp or 16bpp RGB 5:6:5
              (VesaInf.BitPixel==16 && VesaInf.RedMaxSize==5 &&
               VesaInf.GreenMaxSize==6 && VesaInf.BlueMaxSize==5)) ) {
			TbDgfxModes[CptMode].Mode=ListMode[i];
	        TbDgfxModes[CptMode].ResHz=VesaInf.ResX;
	        TbDgfxModes[CptMode].ResVt=VesaInf.ResY;
	        TbDgfxModes[CptMode].VModeFlag=VesaInf.ModeFlag;
		    TbDgfxModes[CptMode].VModeFlag^=VMODE_VGA; // reverse VGA compatible bit
	        TbDgfxModes[CptMode].BitPixel=VesaInf.BitPixel;
	        TbDgfxModes[CptMode].rlfb=VesaInf.PhysBasePtr+VesaInf.OffScreenOfs;
	        TbDgfxModes[CptMode].VtFreq = 60;
		    TbDgfxModes[CptMode].VModeFlag^=VMODE_BANKED; // reverse banked/windowed bit
		    if ((VesaInf.WinFlag1 & 0x7) == 0x7) {
                TbDgfxModes[CptMode].WinID =  0;
                TbDgfxModes[CptMode].WinSeg = VesaInf.SegWin1;
		    } else if ((VesaInf.WinFlag2 & 0x7) == 0x7) {
                TbDgfxModes[CptMode].WinID =  1;
                TbDgfxModes[CptMode].WinSeg = VesaInf.SegWin2;
		    } else {
                TbDgfxModes[CptMode].WinID =  -1;
                TbDgfxModes[CptMode].WinSeg = 0;
		    }
	        CptMode++;
	     }
	}
	// final count of valid gfx modes
	NbDgfxModes=CptMode;
	free(ListMode);

	InitVesaPMI();
	if (VesaPMIOk) {
	  SetPalette=ProtectSetPalette;
	  ViewSurf=ProtectViewSurf;
	  ViewSurfWaitVR=ProtectViewSurfWaitVR;
	} else {
	  SetPalette=RealSetPalette;
	  ViewSurf=RealViewSurf;
	  ViewSurfWaitVR=RealViewSurfWaitVR;
	}
	//if (VesaHiVers>=3) ViewSurf=RealViewSurfSched;
	EnableVesaMTRR();
	return 1;
}
bool AllocInitVSurf() {
	int cvlfb = 0;
	int pixelsize=GetPixelSize(CurDgfxMode->BitPixel);
	int ScreenSize = CurDgfxMode->ResHz*CurDgfxMode->ResVt*pixelsize;
	if ((ScreenSize*NbVDgSurf)>Sizelfb)
		return 0;

	if (curModeVideo && VSurf!=NULL) {
		free(VSurf);
		VSurf=NULL;
	}
	VSurf=(DgSurf *)malloc(sizeof(DgSurf)*(NbVDgSurf));
	if (VSurf==NULL)
		return 0;
	cvlfb=lfb;
	for (int j=0;j<NbVDgSurf;j++) {
		VSurf[j].vlfb=VSurf[j].rlfb= cvlfb;
		VSurf[j].OffVMem= cvlfb-lfb;
		VSurf[j].ResH= CurDgfxMode->ResHz;
		VSurf[j].ResV= CurDgfxMode->ResVt;

		VSurf[j].MaxX= CurDgfxMode->ResHz-1;
		VSurf[j].MaxY= VSurf[j].MinX= 0;
		VSurf[j].MinY= -CurDgfxMode->ResVt+1; //axe Y montant
		VSurf[j].SizeSurf= ScreenSize;
		VSurf[j].OrgX= 0;
		VSurf[j].OrgY= CurDgfxMode->ResVt-1;
		VSurf[j].BitsPixel=CurDgfxMode->BitPixel;
		VSurf[j].ScanLine=CurDgfxMode->ResHz*pixelsize;
		VSurf[j].NegScanLine = -VSurf[j].ScanLine;


		SetOrgSurf(&VSurf[j],0,0);
		cvlfb+= ScreenSize;
	}
}

int InitVesaMode(int ResHz, int ResVt, char BitPixel, int NbPage)
{
	__dpmi_regs r;
	int i;

 	if (NbPage < 1 || (BitPixel != 8 && BitPixel != 16))
		return 0;
 	NbVDgSurf = NbPage;
 	for (i=0;i<NbDgfxModes;i++) {

		if ( TbDgfxModes[i].ResHz==ResHz &&   TbDgfxModes[i].ResVt==ResVt && TbDgfxModes[i].BitPixel==BitPixel &&
			((TbDgfxModes[i].VModeFlag&VMODE_LFB > 0)  || (TbDgfxModes[i].VModeFlag&VMODE_BANKED > 0)) ) {
			// forced banked but no banked in this mode ?
			if (forceBankedVESA && ((TbDgfxModes[i].VModeFlag&VMODE_BANKED) == 0))
				continue;

			CurDgfxMode=&TbDgfxModes[i];
			CurModeVtFreq=TbDgfxModes[i].VtFreq;
			dgfxMode=CurDgfxMode->Mode;
			if (!forceBankedVESA && ((TbDgfxModes[i].VModeFlag&VMODE_LFB) > 0)) {
				dgfxMode |= VBE_SETMODE_LFB;
				curVideoBanked = false;
			} else {
				curVideoBanked = true;
			}
			// check if enough VRAM

			if (!AllocInitVSurf())
				return 0;

			// init VESA mode
			bzero(&r,sizeof(__dpmi_regs));
			r.x.ax = 0x4f02;
			r.x.bx = dgfxMode;
			_go32_dpmi_simulate_int(0x10, &r);
			if (r.h.al != 0x4f || r.h.ah != 0) {
				if (VSurf!=NULL) {
					free(VSurf);
					VSurf = NULL;
				}
                return 0;
			}
			curModeVideo=true;

			DgSetCurSurf(&VSurf[0]);

			// Get DAC Palette format
			if (BitPixel==8) {
				GetPaletteDAC();
			}
			return 1;
		}
	}
	return 0;
}

void DgSetEnabledDoubleBuff(bool dblBuffEnabled) {
    bool reallocVSurf = false;
    if (enableDoubleBuff && !dblBuffEnabled) { // disable
        // destroy not required data
        if (RendFrontSurf != NULL && RendFrontSurf->rlfb != 0) {
            DestroySurf(RendFrontSurf);
            memset(RendFrontSurf, 0, sizeof(DgSurf));
            RendFrontSurf = NULL;
        }
        reallocVSurf = true;
        enableDoubleBuff = false;
        NbVDgSurf = 1;
    } else if (!enableDoubleBuff && dblBuffEnabled) { // enable
        if (!CreateSurf(&RendFrontSurf, RendSurf->ResH, RendSurf->ResV, RendSurf->BitsPixel))
            return; // failed to enable double buff
        reallocVSurf = true;
        enableDoubleBuff = true;
        NbVDgSurf = 1;
    }
    if (reallocVSurf)
		AllocInitVSurf();
}


bool DgGetEnabledDoubleBuff() {
    return enableDoubleBuff;
}

void SurfCopy(DgSurf *Sdst, DgSurf *Ssrc) {
	if (!curVideoBanked || !curModeVideo) {
		SurfCopyRaw(Sdst, Ssrc);
	}
	// check if it's a VSurf Pointer
	long vsurfStart = (long)VSurf;
	long vsurfEnd = (long)(&(((char*)&VSurf[NbVDgSurf-1])[sizeof(DgSurf)]));
	int idxVSurf = -1;
	if ((long)Sdst >= vsurfStart && (long)Sdst < vsurfEnd) {
		idxVSurf = ((long)(Sdst) - vsurfStart) / (long)(sizeof(DgSurf));

//        __djgpp_conventional_base
//        void putpixel(int x, int y, int c) {
//  char *screen = 0xa0000 + __djgpp_conventional_base;
//  *(screen + x + y * 320) = c;
//}

	} else { // not a video Surf
		SurfCopyRaw(Sdst, Ssrc);
	}
}

void DgQuit() {
   __dpmi_meminfo memMPIO;
   if (MTRRa) {
     MTRRa=0;
     asm ("	cli \n"
	  "	wbinvd \n"
	  " 	movl	$0x2ff,%ecx \n"
	  "	rdmsr  \n"
	  "	mov	_OldCachMode,%al \n"
	  "	wrmsr \n"
	  "	sti				\n");
   }
   if (TbDgfxModes!=NULL) {
	 free(TbDgfxModes);
	 TbDgfxModes = NULL;
   }
   CurDgfxMode = NULL;
   if (VesaPMIOk) {
     free(VesaPMI); VesaPMIOk=0;
     if (EnableMPIO) {
	EnableMPIO=0;
	memMPIO.address=MAPAddMPIO;
	__dpmi_free_physical_address_mapping(&memMPIO);
	__dpmi_free_ldt_descriptor(SelMPIO);
     }
   }
   if (curModeVideo) {
		free(VSurf);
        VSurf=NULL;
        curModeVideo=false;
	}
   __djgpp_nearptr_disable();
   __dpmi_free_physical_address_mapping(&dpinf);
   ShiftPal=0;
   DestroyDWorkers();
   TextMode();
}

void InitVesaPMI() {
// initialise VESA PMI
	__dpmi_regs r;
	__dpmi_meminfo memMPIO;
	unsigned short *PortMem;
	int *adressMPIO;
	unsigned short *sizeMPIO;
	unsigned short *markerMPIO;
	int i;
	bzero(&r,sizeof(__dpmi_regs));
	r.x.ax = 0x4f0a;
	r.h.bl = 0x0;
	_go32_dpmi_simulate_int(0x10, &r);
	EnableMPIO=0;
	VesaPMIOk=0;
	if (r.h.ah==0) {	  // VESA PMI failed ?
        if ((VesaPMI=malloc(r.x.cx))==NULL) return;
           _go32_dpmi_lock_data( VesaPMI, r.x.cx);
		dosmemget(((unsigned int)(r.x.es)<<4)+(unsigned int)(r.x.di),
		  (unsigned int)(r.x.cx), VesaPMI);
	    WindowControlPMI=VesaPMI+((unsigned short*)(VesaPMI))[0];
	    ViewAddressPMI=VesaPMI+((unsigned short*)(VesaPMI))[1];
	    SizeVesaPMI=r.x.cx;
	    SetPalPMI=VesaPMI+((unsigned short*)(VesaPMI))[2];
	    // scan for io ports
	    PortMem=VesaPMI+((unsigned short*)(VesaPMI))[3];

	    // jump over io ports
	    for (i=0;;i++) {
			if (PortMem[i]==0xffff) {
			break;
			}
	    }
		if (PortMem[i+1]!=0xffff) {
			adressMPIO=(int*)&PortMem[i+1];
			sizeMPIO=&PortMem[i+3];
			markerMPIO=&PortMem[i+4];
			SizeMPIO=(*sizeMPIO)*2;
			AddMPIO=*adressMPIO;
			// map the MPIO
			memMPIO.address=AddMPIO;
			memMPIO.size=SizeMPIO;
			// mapped
			if (__dpmi_physical_address_mapping(&memMPIO)==0) {
				MAPAddMPIO=memMPIO.address;
				__dpmi_lock_linear_region(&memMPIO);
				// create a selector
				SelMPIO = __dpmi_allocate_ldt_descriptors(1);
				if (SelMPIO!=-1) {
					__dpmi_set_segment_base_address(SelMPIO, MAPAddMPIO);
					__dpmi_set_segment_limit(SelMPIO, 0xffff);
					EnableMPIO=1;
				}
				else
				__dpmi_free_physical_address_mapping(&memMPIO);

			}
	   }
	   else {
	     EnableMPIO=0;
	   }
		VesaPMIOk=1;
	}
}

int EnableVesaMTRR() {
    if (DetectCPUID())
	  if (!(_my_cs()&3)) {
	    return (MTRRa=EnableMTRR()); }
	return 0;
}

/*int EnableMTRR() {
	asm ("	push	%ebx\n"
             "  mov	$1,%eax\n"
             "  cpuid\n"
             "  xor	%cl,%cl\n"
             "   test	$0x20,%edx\n"
             "  jz	fin\n"
             "  mov	$1,%eax\n"
             "	cpuid\n"
             "	xor	%cl,%cl\n"
             "	test	$0x1000,%edx\n"
             "	jz	fin\n"
             "	mov	$0xfe,%ecx\n"
             "	rdmsr\n"
             "	test    $400,%eax\n"
             "	jz	fin\n"
             "	mov	$0x2ff,%ecx\n"
             "	rdmsr\n"
             "	cli\n"
             "	wbinvd\n"
             "	mov	%al,_OldCachMode\n"
             "	mov	$1,%al\n"
             "	wrmsr\n"
             "	sti\n"
             "	mov	$1,%cl  \n");
fin:
	asm("	movzx	%cl,%eax\n"
	    "	pop	%ebx		");
}*/


int DetectMMX() {
   unsigned int reax,rebx,recx,redx;
   unsigned int MaxVEAX;
   if (DetectCPUID()) {
     ExecCPUID(0,&MaxVEAX,&rebx,&recx,&redx);
     if (MaxVEAX>0) {
       ExecCPUID(1,&reax,&rebx,&recx,&redx);
       if (redx&0x800000) return 1;   // check if bit 23 of edx
     }
     ExecCPUID(0x80000000,&MaxVEAX,&rebx,&recx,&redx); // check extended cpuid
     if (MaxVEAX>0x80000000) {
       ExecCPUID(0x80000001,&reax,&rebx,&recx,&redx);
       if (redx&0x800000) return 1;   // check if bit 23 of edx
     }
   }
   return 0;
}

void TextMode()
{	if (curModeVideo) {
		free(VSurf);
		VSurf=CurModeVtFreq=0;
		curModeVideo = false;
	}
	asm ("	mov	$0x3,%eax \n"
	     "	int	$0x10	");
}

void GetPaletteDAC()
{	__dpmi_regs r;
	if (VesaInt.Capabilities & 1) {  // D0 ( DAC switch)
	  bzero(&r,sizeof(__dpmi_regs));
   	  r.x.ax = 0x4f08;
	  r.h.bl = 0;	// Set Palette format
	  r.h.bh = 8;	// to 8 Bit
   	  _go32_dpmi_simulate_int(0x10, &r);
	  ShiftPal=(r.h.bh==8)?0:1;
	} else ShiftPal=1;
}



void PrBuildTbColConv(void *PalBGR1024,float PropBonus)
{	int	i;
	unsigned char *Pal=PalBGR1024;
	FREE_MMX();
	// 8bpp -> 15bpp
	for (i=0;i<256;i++)
	  Col8To15bpp[i]= ((short)(Pal[i*4])>>3)|
	  	(((short)(Pal[i*4+1])>>3)<<5)|(((short)(Pal[i*4+2])>>3)<<10);
	// 8bpp -> 16bpp
	for (i=0;i<256;i++)
	  Col8To16bpp[i]= ((short)(Pal[i*4])>>3)|(((short)(Pal[i*4+1])>>2)<<5)|(((short)(Pal[i*4+2])>>3)<<11);
	// 8bpp -> 16bpp
	for (i=0;i<256;i++)
	  Col8To32bpp[i]= ((int)(Pal[i*4]))|((int)(Pal[i*4+1])<<8)|
	  	((int)(Pal[i*4+2])<<16);
	// 15bpp -> 8bpp
	for (i=0;i<256*128;i++)
//	  Col15To8bpp[i] = PrFindCol(1,255,((i>>10)&0x1f)<<3,((i>>5)&0x1f)<<3,(i&0x1f)<<3,Pal,PropBonus);
	  Col15To8bpp[i] = PrFindCol(1,255,(i&0x1f)<<3,((i>>5)&0x1f)<<3,((i>>10)&0x1f)<<3,Pal,PropBonus);
}

void BuildTbColConv(void *PalBGR1024)
{	int	i;
	unsigned char *Pal=PalBGR1024;
	FREE_MMX();
	// 8bpp -> 15bpp
	for (i=0;i<256;i++)
	  Col8To15bpp[i]= ((short)(Pal[i*4])>>3)|
	  	(((short)(Pal[i*4+1])>>3)<<5)|(((short)(Pal[i*4+2])>>3)<<10);
	// 8bpp -> 16bpp
	for (i=0;i<256;i++)
	  Col8To16bpp[i]= ((short)(Pal[i*4])>>3)|(((short)(Pal[i*4+1])>>2)<<5)|(((short)(Pal[i*4+2])>>3)<<11);
	// 8bpp -> 16bpp
	for (i=0;i<256;i++)
	  Col8To32bpp[i]= ((int)(Pal[i*4]))|((int)(Pal[i*4+1])<<8)|
	  	((int)(Pal[i*4+2])<<16);
	// 15bpp -> 8bpp
	for (i=0;i<256*128;i++)
	  Col15To8bpp[i] = FindCol(0,255,(i&0x1f)<<3,((i>>5)&0x1f)<<3,((i>>10)&0x1f)<<3,Pal);
}

void BuildTbDegCol(void *PalBGR1024)
{	int i,j,Black,White;
	float sb,sg,sr,db,dg,dr,pb,pg,pr;
	unsigned char *Pal=PalBGR1024;
	int ND=64;
	FREE_MMX();

	Black=FindCol(1,255,0,0,0,Pal);
	White=FindCol(1,255,255,255,255,Pal);
	for (i=0;i<256;i++) {
	  TbDegCol[i*ND]=Black;
	  TbDegCol[i*ND+ND-1]=White;
	  TbDegCol[i*ND+(ND/2)-1]=TbDegCol[i*ND+(ND/2)]=i;
	}
	for (i=0;i<256;i++) {
	  sb=Pal[TbDegCol[i*ND+(ND/2)-1]*4];
	  sg=Pal[TbDegCol[i*ND+(ND/2)-1]*4+1];
	  sr=Pal[TbDegCol[i*ND+(ND/2)-1]*4+2];
	  db=Pal[TbDegCol[i*ND]*4];
	  dg=Pal[TbDegCol[i*ND]*4+1];
	  dr=Pal[TbDegCol[i*ND]*4+2];
	  pb=(db-sb)/(float)((ND/2)-1); pg=(dg-sg)/(float)((ND/2)-1); pr=(dr-sr)/(float)((ND/2)-1);
	  for (j=(ND/2)-2;j>0;j--) {
	    sb+=pb; sg+=pg; sr+=pr;
	    TbDegCol[i*ND+j]=FindCol(1,255,sb,sg,sr,Pal);
	  }

	  sb=Pal[TbDegCol[i*ND+((ND/2)-1)]*4];
	  sg=Pal[TbDegCol[i*ND+((ND/2)-1)]*4+1];
	  sr=Pal[TbDegCol[i*ND+((ND/2)-1)]*4+2];
	  db=Pal[TbDegCol[i*ND+(ND-1)]*4];
	  dg=Pal[TbDegCol[i*ND+(ND-1)]*4+1];
	  dr=Pal[TbDegCol[i*ND+(ND-1)]*4+2];
	  pb=(db-sb)/(float)((ND/2)-1); pg=(dg-sg)/(float)((ND/2)-1); pr=(dr-sr)/(float)((ND/2)-1);
	  for (j=(ND/2+1);j<ND-1;j++) {
	    sb+=pb; sg+=pg; sr+=pr;
	    TbDegCol[i*ND+j]=FindCol(1,255,sb,sg,sr,Pal);
	  }
	}
}

void PrBuildTbDegCol(void *PalBGR1024,float PropBonus)
{	int i,j,Black,White;
	float sb,sg,sr,db,dg,dr,pb,pg,pr;
	unsigned char *Pal=PalBGR1024;
	int ND=64;
	FREE_MMX();
	Black=FindCol(1,255,0,0,0,Pal);
	White=FindCol(1,255,255,255,255,Pal);
	for (i=0;i<256;i++) {
	  TbDegCol[i*ND]=Black;
	  TbDegCol[i*ND+ND-1]=White;
	  TbDegCol[i*ND+(ND/2)-1]=TbDegCol[i*ND+(ND/2)]=i;
	}
	for (i=0;i<256;i++) {
	  sb=Pal[TbDegCol[i*ND+(ND/2)-1]*4];
	  sg=Pal[TbDegCol[i*ND+(ND/2)-1]*4+1];
	  sr=Pal[TbDegCol[i*ND+(ND/2)-1]*4+2];
	  db=Pal[TbDegCol[i*ND]*4];
	  dg=Pal[TbDegCol[i*ND]*4+1];
	  dr=Pal[TbDegCol[i*ND]*4+2];
	  pb=(db-sb)/(float)((ND/2)-1); pg=(dg-sg)/(float)((ND/2)-1); pr=(dr-sr)/(float)((ND/2)-1);
	  for (j=(ND/2)-2;j>0;j--) {
	    sb+=pb; sg+=pg; sr+=pr;
	    TbDegCol[i*ND+j]=PrFindCol(1,255,sb,sg,sr,Pal,PropBonus);
	  }

	  sb=Pal[TbDegCol[i*ND+((ND/2)-1)]*4];
	  sg=Pal[TbDegCol[i*ND+((ND/2)-1)]*4+1];
	  sr=Pal[TbDegCol[i*ND+((ND/2)-1)]*4+2];
	  db=Pal[TbDegCol[i*ND+(ND-1)]*4];
	  dg=Pal[TbDegCol[i*ND+(ND-1)]*4+1];
	  dr=Pal[TbDegCol[i*ND+(ND-1)]*4+2];
	  pb=(db-sb)/(float)((ND/2)-1); pg=(dg-sg)/(float)((ND/2)-1); pr=(dr-sr)/(float)((ND/2)-1);
	  for (j=(ND/2+1);j<ND-1;j++) {
	    sb+=pb; sg+=pg; sr+=pr;
	    TbDegCol[i*ND+j]=PrFindCol(1,255,sb,sg,sr,Pal,PropBonus);
	  }
	}
}


int FindCol(int DebCol,int FinCol,int B,int G,int R,void *PalBGR1024)
{	int i,c=DebCol,ray,ray2;
	int bs,gs,rs;
	unsigned char *Pal=PalBGR1024;
	ray=17000000;
	DebCol&=0xff; FinCol&=0xff; B&=0xff; G&=0xff; R&=0xff;
	for (i=DebCol;i<FinCol+1;i++) {
	  bs=B-Pal[i*4]; gs=G-Pal[i*4+1]; rs=R-Pal[i*4+2];
          ray2=bs*bs+gs*gs+rs*rs;
	  if (ray2<ray) { c=i; ray=ray2; }
	  if (ray==0) break;
	}
	return c;
}

int Prop(int B,int G,int R) {
	int ind=0;
	if (B > G)
		ind=0x1;
	else
		ind = (B == G) ? 0x2 : 0x3;
	if (B > R)
		ind |= 0x10;
	else
	    ind |= (B == R) ? 0x20 : 0x30;
	if (G > R)
		ind |= 0x100;
	else
	    ind |= (G == R) ? 0x200 : 0x300;

	return ind;
}

int PrFindCol(int DebCol,int FinCol,int SearchB,int SearchG,int SearchR,
            void *PalBGR1024, float PropBonus)
{	int i = 0,
		PropBGR = 0,
		ray = 0,
		ray2 = 0,
		deltaB = 0,
		deltaG = 0,
		deltaR = 0,
		colB = 0,
		colG = 0,
		colR = 0,
		c = DebCol;
	unsigned char *Pal=PalBGR1024;
	float bonusPow2 = PropBonus * PropBonus;

	ray=17000000; // ~ 255*255*255 highest possible
	DebCol&=0xff; FinCol&=0xff; SearchB&=0xff; SearchG&=0xff; SearchR&=0xff;
	PropBGR=Prop(SearchB,SearchG,SearchR);
	for (i=DebCol;i<=FinCol;i++) {
	    colB=(int)Pal[i*4];
	    colG=(int)Pal[i*4+1];
	    colR=(int)Pal[i*4+2];
	    deltaB=SearchB-colB;
	    deltaG=SearchG-colG;
	    deltaR=SearchR-colR;
		ray2=(deltaB*deltaB)+(deltaG*deltaG)+(deltaR*deltaR);
		if (PropBGR == Prop(colB,colG,colR)) {
			ray2 = (int)((float)(ray2) * bonusPow2);
		}
		if (ray2<ray) {
			c=i;
			ray=ray2;
		}
		if (ray==0) break;
	}
	return c;
}

void RealViewSurf(int NbSurf) {
	__dpmi_regs r;
	if (NbSurf>=NbVDgSurf) return;
	CurViewVSurf=NbSurf;
   	bzero(&r,sizeof(__dpmi_regs));
	r.d.eax = 0x4f07;
	r.h.bl = 0;
	r.h.bh = 0;
	r.d.ecx = 0; // x = 0
        r.d.edx = VSurf[0].ResV*NbSurf; // y


   	_go32_dpmi_simulate_int(0x10, &r);
}

void RealViewSurfSched(int NbSurf) {
	if (NbSurf>=NbVDgSurf) return;
	CurViewVSurf=NbSurf;
	addr= VSurf[NbSurf].OffVMem;

	asm (" push  %ebx \n"
	     "  mov   $0x4f07,%eax \n"
	     "  mov   _addr,%ecx   \n"
	     "  mov   $0x2,%ebx	   \n"
	     "  int   $0x10	  \n"
	     "  pop   %ebx         \n");
}


void RealViewSurfWaitVR(int NbSurf) {
	__dpmi_regs r;
	if (NbSurf>=NbVDgSurf) return;
	CurViewVSurf=NbSurf;
   	bzero(&r,sizeof(__dpmi_regs));
	r.d.eax = 0x4f07;
	r.h.bl = 0x80;
	r.h.bh = 0;
	r.d.ecx = 0; // x = 0
        r.d.edx = VSurf[0].ResV*NbSurf; // y

   	_go32_dpmi_simulate_int(0x10, &r);
}

void RealSetPalette(int Dbcol, int Nbcol, void *Tcol) {
	__dpmi_regs r;
	int i,j;
	Dbcol&=0xff;
//        if (CurMode.VModeFlag|VMODE_VGA) ShiftPal=1;
	if (Nbcol<1 || Nbcol>256) return;
	if (ShiftPal) {
		for (i=Dbcol;i<Dbcol+Nbcol;i++) {
			j=i-Dbcol;
			CurPalette[i*4]=(((unsigned char*)(Tcol))[j*4])>>2; // B
			CurPalette[i*4+1]=(((unsigned char*)(Tcol))[j*4+1])>>2; // G
			CurPalette[i*4+2]=(((unsigned char*)(Tcol))[j*4+2])>>2; // R
		}
	} else memcpy(&CurPalette[Dbcol*4],Tcol,Nbcol*4);

//        if (CurMode.VModeFlag|VMODE_VGA)
//        {
			outb(0x3c8,Dbcol);
			for (i=Dbcol;i<Dbcol+Nbcol;i++) {
				outb(0x3c9,CurPalette[i*4+2]);
				outb(0x3c9,CurPalette[i*4+1]);
				outb(0x3c9,CurPalette[i*4]);
			}
//        }
//        else {
			dosmemput(&CurPalette[Dbcol*4], 4*Nbcol, __tb);
			bzero(&r,sizeof(__dpmi_regs));
			r.d.eax = 0x4f09;
			r.d.ebx = 0;
			r.d.ecx = Nbcol;
			r.d.edx = Dbcol;
			r.x.es = (__tb>>4) & 0xffff;
			r.d.edi = __tb & 0xf;
			_go32_dpmi_simulate_int(0x10, &r);
//        }
}


void putpixel(int X,int Y,int pcol) { // utilise PutPixel
	int P[2];
	P[0]=X; P[1]=Y;
	PutPixel(&P,pcol);
}

void cputpixel(int X,int Y,int pcol) { // avec clip
	int P[2];
	if (X>CurSurf.MaxX || X<CurSurf.MinX || Y>CurSurf.MaxY || Y<CurSurf.MinY) return;
	P[0]=X; P[1]=Y;
	PutPixel(&P,pcol);
}

int getpixel(int X,int Y) { // utilise GetPixel
	int P[2];
	P[0]=X; P[1]=Y;
	return GetPixel(&P);
}

int cgetpixel(int X,int Y) { // avec clip
	int P[2];
	if (X>CurSurf.MaxX || X<CurSurf.MinX || Y>CurSurf.MaxY || Y<CurSurf.MinY) return 0;
	P[0]=X; P[1]=Y;
	return GetPixel(&P);
}

void ClearSurf(int clrcol) {
	bar(CurSurf.MinX,CurSurf.MinY,CurSurf.MaxX,CurSurf.MaxY,clrcol);
}

void Bar(void *Pt1,void *Pt2,int bcol)
{	bar(((int*)(Pt1))[0], ((int*)(Pt1))[1],
		((int*)(Pt2))[0],((int*)(Pt2))[1], bcol);
}

void bar(int x1,int y1,int x2,int y2,int bcol)
{	int CBar[8], ACBar[5];
	if (x1==x2 || y1==y2) {
	  line(x1,y1,x2,y2,bcol); return; }
	CBar[0]= CBar[6]= x2;
	CBar[2]= CBar[4]= x1;
	CBar[5]= CBar[7]= y1;
	CBar[1]= CBar[3]= y2;
 	ACBar[0]=4;
	ACBar[1]=&CBar[0]; ACBar[2]=&CBar[2];
	ACBar[3]=&CBar[4]; ACBar[4]=&CBar[6];
	Poly(&ACBar, NULL, POLY_SOLID|POLY_FLAG_DBL_SIDED, bcol);
}

void rect(int x1,int y1,int x2,int y2,int rcol) {
    line(x1,y1,x2,y1,rcol);
    line(x1,y2,x2,y2,rcol);
    if (y2>y1) {
      line(x1,y1+1,x1,y2-1,rcol);
      line(x2,y1+1,x2,y2-1,rcol);
    }
    else
      if (y2<y1) {
        line(x1,y1-1,x1,y2+1,rcol);
        line(x2,y1-1,x2,y2+1,rcol);
      }
}

void rectmap(int x1,int y1,int x2,int y2,int rcol,unsigned int rmap) {
    linemap(x1,y1,x2,y1,rcol,rmap);
    linemap(x1,y2,x2,y2,rcol,rmap);
    if (y2>y1) {
      linemap(x1,y1+1,x1,y2-1,rcol,rmap);
      linemap(x2,y1+1,x2,y2-1,rcol,rmap);
    }
    else
      if (y2<y1) {
        linemap(x1,y1-1,x1,y2+1,rcol,rmap);
        linemap(x2,y1-1,x2,y2+1,rcol,rmap);
      }
}
