#include <dos.h>
#include <dpmi.h>
#include <dos.h>
#include <go32.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <conio.h>
#include <crt0.h>
#include <unistd.h>
#include <string.h>
#include <sys/movedata.h>
#include <sys/segments.h>
#include "DUGL.h"
#include "intrdugl.h"

//********************** FONT
int  LoadMemDFONT(DFONT *F,void *In,int SizeIn)
{	HeadCHR hchr;
	int i,Size;
	void *Buff;
	if (SizeIn<sizeof(HeadCHR)) return 0;
	memcpy(&hchr,In,sizeof(HeadCHR));
	if (hchr.Sign!='RHCF') return 0;
	for (Size=0,i=1;i<256;i++)
	   Size+=((hchr.C[i].Lg<=32)?1:2)*hchr.C[i].Ht*4;
	if (hchr.SizeDataCar!=Size) return 0;
	if ((hchr.PtrBuff+hchr.SizeDataCar)<SizeIn) return 0;
	if ((Buff=malloc(hchr.SizeDataCar+2048))==NULL) return 0;
	for (i=1;i<256;i++) hchr.C[i].DatCar+=(int)(Buff+2048);
	memcpy(Buff,&hchr.C[0],2048);
	memcpy(Buff+2048,In+hchr.PtrBuff,hchr.SizeDataCar);
	F->FntPtr=(int)Buff;
	F->FntHaut=F->FntDistLgn=hchr.MaxHautFnt;
	F->FntLowPos=hchr.MinPlusLgn;   F->FntHighPos=hchr.MaxHautLgn;
	F->FntSens=hchr.SensFnt;	F->FntTab=8;
	return 1;
}

int LoadDFONT(DFONT *F,const char *FName)
{	HeadCHR hchr;
	int i,Size;
	void *Buff;
	FILE *InCHR;
	if ((InCHR=fopen(FName,"rb"))==NULL) return 0;
	if (fread(&hchr,sizeof(HeadCHR),1,InCHR)<1) return 0;
	if (hchr.Sign!='RHCF') { fclose(InCHR); return 0; }
	for (Size=0,i=1;i<256;i++)
	   Size+=((hchr.C[i].Lg<=32)?1:2)*hchr.C[i].Ht*4;
	if (hchr.SizeDataCar!=Size) { fclose(InCHR); return 0; }

	if ((Buff=malloc(hchr.SizeDataCar+2048))==NULL)
	  { fclose(InCHR); return 0; }
	for (i=1;i<256;i++) hchr.C[i].DatCar+=(int)(Buff+2048);
	memcpy(Buff,&hchr.C[0],2048);
	fseek(InCHR,hchr.PtrBuff,SEEK_SET);
	if (fread(Buff+2048,hchr.SizeDataCar,1,InCHR)<1)
	  { free(Buff); fclose(InCHR); return 0; }
	F->FntPtr=(int)Buff;
	F->FntHaut=F->FntDistLgn=hchr.MaxHautFnt;
	F->FntLowPos=hchr.MinPlusLgn;   F->FntHighPos=hchr.MaxHautLgn;
	F->FntSens=hchr.SensFnt;	F->FntTab=8;

	fclose(InCHR);
	return 1;
}

void DestroyDFONT(DFONT *F)
{	if (F->FntPtr) free((void*)(F->FntPtr));
        F->FntHaut=F->FntDistLgn=F->FntLowPos=F->FntHighPos=F->FntSens=0;
	F->FntPtr=FntCol=0;
}

void ClearText() {
	if (FntSens) FntX=CurSurf.MaxX;
	else FntX=CurSurf.MinX;
 	FntY=CurSurf.MaxY-FntHighPos;
}

void SetTextAttrib(int TX,int TY,int TCol)
{	FntX=TX; 	FntY=TY;	FntCol=TCol;
}

void SetTextPos(int TX,int TY)
{	FntX=TX; 	FntY=TY;
}

void SetTextCol(int TCol)
{	FntCol=TCol;
}

void OutTextXY(int TX,int TY,const char *str)
{	FntX=TX; 	FntY=TY;
	OutText(str);
}

// Mode : 0 CurPos, 1 mid, 2 AjusteSrc, 3 AjusteI-src, 4 AjLeft, 5 AjRight
int  OutTextMode(const char *str,int Mode) {
	int L=0,x=0;
	switch (Mode) {
	  case 0:
	    break;
	  case 1:
	    L=WidthText(str);
	    if (FntSens) FntX=(CurSurf.MinX+CurSurf.MaxX+L)/2;
	    else FntX=(CurSurf.MinX+CurSurf.MaxX-L)/2;
	    break;
	  case 2:
	    if (FntSens) FntX=CurSurf.MaxX;
	    else FntX=CurSurf.MinX;
	    break;
	  case 3:
	    L=WidthText(str);
	    if (FntSens) FntX=CurSurf.MinX+L;
	    else FntX=CurSurf.MaxX-L;
	    break;
	  case 4:
	    L=WidthText(str);
	    if (FntSens) FntX=CurSurf.MinX+L;
	    else FntX=CurSurf.MinX;
	    break;
	  case 5:
	    L=WidthText(str);
	    if (FntSens) FntX=CurSurf.MaxX;
	    else FntX=CurSurf.MaxX-L;
	    break;
	  default:
	    return 0;
	}
	x=FntX;
	OutText(str);
	return x;
}

int  GetXOutTextMode(const char *str,int Mode) {
	int L=0,x=0;
	switch (Mode) {
	  case 0:
  	    x=FntX; break;
	  case 1:
	    L=WidthText(str);
	    if (FntSens) x=(CurSurf.MinX+CurSurf.MaxX+L)/2;
	    else x=(CurSurf.MinX+CurSurf.MaxX-L)/2;
	    break;
	  case 2:
	    if (FntSens) x=CurSurf.MaxX;
	    else x=CurSurf.MinX;
	    break;
	  case 3:
	    L=WidthText(str);
	    if (FntSens) x=CurSurf.MinX+L;
	    else x=CurSurf.MaxX-L;
	    break;
	  case 4:
	    L=WidthText(str);
	    if (FntSens) x=CurSurf.MinX+L;
	    else x=CurSurf.MinX;
	    break;
	  case 5:
	    L=WidthText(str);
	    if (FntSens) x=CurSurf.MaxX;
	    else x=CurSurf.MaxX-L;
	    break;
	  default:
	    return -1;
	}
	return x;
}

int GetFntYMID() {
    return (CurSurf.MaxY+CurSurf.MinY)/2-FntHaut/2-FntLowPos;
}

int  OutTextYMode(int TY,const char *str,int Mode)
{	FntY=TY;
	return OutTextMode(str,Mode);
}

void ViewClearText(DgView *V)
{	if (FntSens) FntX=V->MaxX;
	else FntX=V->MinX;
 	FntY=V->MaxY-FntHighPos;
}

void OutTextModeFormat(int Mode, char *midStr, unsigned int sizeMidStr, char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsprintf(midStr, fmt, args);
    va_end(args);
    OutTextMode(midStr, Mode);
}

void OutTextFormat(char *midStr, unsigned int sizeMidStr, char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsprintf(midStr, fmt, args);
    va_end(args);
    OutText(midStr);
}

// Mode : 0 CurPos, 1 mid, 2 AjusteSrc, 3 AjusteI-src, 4 AjLeft, 5 AjRight
int  ViewOutTextMode(DgView *V,const char *str,int Mode) {
   DgView saveView;
   int x;
   GetSurfView(&CurSurf, &saveView);
   x=OutTextMode(str,Mode);
   SetSurfView(&CurSurf, &saveView);
   return x;
}

int  ViewGetXOutTextMode(DgView *V,const char *str,int Mode) {
	int L=0,x=0;
	switch (Mode) {
	  case 0:
  	    x=FntX; break;
	  case 1:
	    L=WidthText(str);
	    if (FntSens) x=(V->MinX+V->MaxX+L)/2;
	    else x=(V->MinX+V->MaxX-L)/2;
	    break;
	  case 2:
	    if (FntSens) x=V->MaxX;
	    else x=V->MinX;
	    break;
	  case 3:
	    L=WidthText(str);
	    if (FntSens) x=V->MinX+L;
	    else x=V->MaxX-L;
	    break;
	  case 4:
	    L=WidthText(str);
	    if (FntSens) x=V->MinX+L;
	    else x=V->MinX;
	    break;
	  case 5:
	    L=WidthText(str);
	    if (FntSens) x=V->MaxX;
	    else x=V->MaxX-L;
	    break;
	  default:
	    return 0;
	}
	return x;
}

int  ViewOutTextYMode(DgView *V,int TY,const char *str,int Mode)
{	FntY=TY;
	return ViewOutTextMode(V,str,Mode);
}

int  ViewOutTextXYMode(DgView *V,int TXY,int TY,const char *str)
{	FntX=TXY; FntY=TY;
	return ViewOutTextMode(V,str,AJ_CUR_POS);
}


int ViewGetFntYMID(DgView *V) {
    return (V->MaxY+V->MinY)/2-FntHaut/2-FntLowPos;
}


// 16 bpp OutText ---------

void OutText16XY(int TX,int TY,const char *str)
{	FntX=TX; 	FntY=TY;
	OutText16(str);
}

// Mode : 0 CurPos, 1 mid, 2 AjusteSrc, 3 AjusteI-src, 4 AjLeft, 5 AjRight
int  OutText16Mode(const char *str,int Mode) {
	int L=0,x=0;
	switch (Mode) {
	  case 0:
	    break;
	  case 1:
	    L=WidthText(str);
	    if (FntSens) FntX=(CurSurf.MinX+CurSurf.MaxX+L)/2;
	    else FntX=(CurSurf.MinX+CurSurf.MaxX-L)/2;
	    break;
	  case 2:
	    if (FntSens) FntX=CurSurf.MaxX;
	    else FntX=CurSurf.MinX;
	    break;
	  case 3:
	    L=WidthText(str);
	    if (FntSens) FntX=CurSurf.MinX+L;
	    else FntX=CurSurf.MaxX-L;
	    break;
	  case 4:
	    L=WidthText(str);
	    if (FntSens) FntX=CurSurf.MinX+L;
	    else FntX=CurSurf.MinX;
	    break;
	  case 5:
	    L=WidthText(str);
	    if (FntSens) FntX=CurSurf.MaxX;
	    else FntX=CurSurf.MaxX-L;
	    break;
	  default:
	    return 0;
	}
	x=FntX;
	OutText16(str);
	return x;
}

int  OutText16YMode(int TY,const char *str,int Mode)
{	FntY=TY;
	return OutText16Mode(str,Mode);
}

void OutText16ModeFormat(int Mode, char *midStr, unsigned int sizeMidStr, char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsprintf(midStr, fmt, args);
    va_end(args);
    OutText16Mode(midStr, Mode);
}

void OutText16Format(char *midStr, unsigned int sizeMidStr, char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsprintf(midStr, fmt, args);
    va_end(args);
    OutText16(midStr);
}

// Mode : 0 CurPos, 1 mid, 2 AjusteSrc, 3 AjusteI-src, 4 AjLeft, 5 AjRight
int  ViewOutText16Mode(DgView *V,const char *str,int Mode)
{
   DgView saveView;
   int x;
   GetSurfView(&CurSurf, &saveView);
   x=OutText16Mode(str,Mode);
   SetSurfView(&CurSurf, &saveView);
   return x;
}

int  ViewOutText16YMode(DgView *V,int TY,const char *str,int Mode)
{	FntY=TY;
	return ViewOutText16Mode(V,str,Mode);
}

int  ViewOutText16XYMode(DgView *V,int TXY,int TY,const char *str)
{	FntX=TXY; FntY=TY;
	return ViewOutText16Mode(V,str,AJ_CUR_POS);
}

