#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <unistd.h>
#include <DUGL.h>
#include "DGUI.H"


CGraphContext::CGraphContext(int BitsPixel,void *PalBGR1024) {
  // set default values
  WinNoir=0; WinBlanc=15; WinBleuF=1;
  WinGris=7; WinGrisF=8; WinGrisC=11;
  WinRougeF=4; WinStopScan = 0; WinFreeMs = 1;
  // paletted 8 bpp ?
  if (BitsPixel==8 && PalBGR1024!=NULL) {
    WinNoir=PrFindCol(0,255,   0,   0,   0, PalBGR1024,0.45);
    WinBlanc=PrFindCol(0,255, 255, 255, 255, PalBGR1024,0.45);
    WinBleuF=PrFindCol(0,255, 127,   0,   0, PalBGR1024,0.45);
    WinGris=PrFindCol(0,255, 165, 165, 165, PalBGR1024,0.45);
    WinGrisF=PrFindCol(0,255,  96,  96,  96, PalBGR1024,0.45);
    WinGrisC=PrFindCol(0,255, 224, 224, 224, PalBGR1024,0.45);
    WinRougeF=PrFindCol(0,255,   0,   0, 138, PalBGR1024,0.45);
  }
  else
    if (BitsPixel==16) {
      WinNoir=0x0;
      WinBlanc=0xFFFF;
      WinBleuF=RGB16(0, 0, 127);
      WinGris=RGB16(165, 165, 165);
      WinGrisF=RGB16(96,  96,  96);
      WinGrisC=RGB16(224, 224, 224);
      WinRougeF=RGB16( 138,   0, 0);
    }
  // assign functions graphic pointer
  switch (BitsPixel) {
   case 8 :
     Clear=::Clear;
     ClearSurf=::ClearSurf;
     bar=::bar;
     line=::line;
     cputpixel=::cputpixel;
     rectmap=::rectmap;
     rect=::rect;
     PutSurf=::PutSurf;
     PutMaskSurf=::PutMaskSurf;
     OutText=::OutText;
     ViewOutTextMode=::ViewOutTextMode;
     Line=::Line;
     Poly=::Poly;
     break;
   case 16 :
     Clear=::Clear16;
     ClearSurf=::ClearSurf16;
     bar=::bar16;
     line=::line16;
     cputpixel=::cputpixel16;
     rectmap=::rectmap16;
     rect=::rect16;
     PutSurf=::PutSurf16;
     PutMaskSurf=::PutMaskSurf16;
     OutText=::OutText16;
     ViewOutTextMode=::ViewOutText16Mode;
     Line=::Line16;
     Poly=::Poly16;
     break;
   default :
     Clear=NULL;
     ClearSurf=NULL;
     bar=NULL;
     line=NULL;
     cputpixel=NULL;
     rectmap=NULL;
     rect=NULL;
     PutSurf=NULL;
     PutMaskSurf=NULL;
     OutText=NULL;
     Line=NULL;
     Poly=NULL;
  }
}

void CGraphContext::CadWin(int x1,int y1,int x2,int y2) {
    line(x1,y1,x2,y1,WinNoir);
    line(x2,y1,x2,y2,WinNoir);
    line(x1+1,y1+1,x2-1,y1+1,WinGrisF);
    line(x2-1,y1+1,x2-1,y2-1,WinGrisF);
    line(x1+1,y1+2,x1+1,y2-1,WinGrisC);
    line(x1+1,y2-1,x2-2,y2-1,WinGrisC);
    line(x1,y1+1,x1,y2,WinGris);
    line(x1,y2,x2-1,y2,WinGris);
}

void CGraphContext::CadAppWin(int x1,int y1,int x2,int y2) {
    line(x1,y1,x2,y1,WinGrisC);
    line(x2,y1,x2,y2,WinGrisC);
    line(x1+1,y1+1,x2-1,y1+1,WinGris);
    line(x2-1,y1+1,x2-1,y2-1,WinGris);
    line(x1+1,y1+2,x1+1,y2-1,WinGrisF);
    line(x1+1,y2-1,x2-2,y2-1,WinGrisF);
    line(x1,y1+1,x1,y2,WinNoir);
    line(x1,y2,x2-1,y2,WinNoir);
}

void CGraphContext::CadBoxWin(int x1,int y1,int x2,int y2) {
    line(x1,y1,x2,y1,WinGrisC);
    line(x2,y1,x2,y2,WinGrisC);
    line(x1+1,y1+1,x2-1,y1+1,WinGris);
    line(x2-1,y1+1,x2-1,y2-1,WinGris);
    line(x1+1,y1+2,x1+1,y2-1,WinNoir);
    line(x1+1,y2-1,x2-2,y2-1,WinNoir);
    line(x1,y1+1,x1,y2,WinGrisF);
    line(x1,y2,x2-1,y2,WinGrisF);
}

void CGraphContext::DrawCoche(int x,int y) {
   line(x,y+2,x+2,y,WinNoir); line(x,y+3,x+2,y+1,WinNoir); line(x,y+4,x+2,y+2,WinNoir);
   line(x+2,y,x+6,y+4,WinNoir); line(x+2,y+1,x+6,y+5,WinNoir); line(x+2,y+2,x+6,y+6,WinNoir);
}

void CGraphContext::DrawFHaut(int x,int y,int fcol) {
   line(x,y,x+4,y,fcol); line(x+1,y+1,x+3,y+1,fcol);
   cputpixel(x+2,y+2,fcol);
}

void CGraphContext::DrawFBas(int x,int y,int fcol) {
   line(x,y+2,x+4,y+2,fcol); line(x+1,y+1,x+3,y+1,fcol);
   cputpixel(x+2,y,fcol);
}

void CGraphContext::DrawFDroit(int x,int y,int fcol) {
   line(x,y,x,y+4,fcol); line(x+1,y+1,x+1,y+3,fcol);
   cputpixel(x+2,y+2,fcol);
}

void CGraphContext::DrawFGauche(int x,int y,int fcol) {
   line(x+2,y,x+2,y+4,fcol); line(x+1,y+1,x+1,y+3,fcol);
   cputpixel(x,y+2,fcol);
}

void CGraphContext::DrawButtFHaut(int x,int y,int etat) {
  bar(x,y,x+12,y+12,WinGris);
  if (etat) {
    CadAppWin(x,y,x+12,y+12); DrawFHaut(x+5,y+4,WinNoir);
  } else { CadWin(x,y,x+12,y+12); DrawFHaut(x+4,y+5,WinNoir); }
}

void CGraphContext::DrawButtFBas(int x,int y,int etat) {
  bar(x,y,x+12,y+12,WinGris);
  if (etat) {
    CadAppWin(x,y,x+12,y+12); DrawFBas(x+5,y+4,WinNoir);
  } else { CadWin(x,y,x+12,y+12); DrawFBas(x+4,y+5,WinNoir); }
}

void CGraphContext::DrawButtFDroit(int x,int y,int etat) {
  bar(x,y,x+12,y+12,WinGris);
  if (etat) {
    CadAppWin(x,y,x+12,y+12); DrawFDroit(x+6,y+3,WinNoir);
  } else { CadWin(x,y,x+12,y+12); DrawFDroit(x+5,y+4,WinNoir); }
}

void CGraphContext::DrawButtFGauche(int x,int y,int etat) {
  bar(x,y,x+12,y+12,WinGris);
  if (etat) {
    CadAppWin(x,y,x+12,y+12); DrawFGauche(x+6,y+3,WinNoir);
  } else { CadWin(x,y,x+12,y+12); DrawFGauche(x+5,y+4,WinNoir); }
}

int CGraphContext::In(int x1,int y1,int x2,int y2, int PosX, int PosY) {
    if (PosX<x1 || PosY<y1 || PosX>x2 || PosY>y2) return 0;
    else return 1;
}

void CGraphContext::GHline(int x1, int x2, int y1) {
   line(x1,y1,x2,y1,WinGrisF);
   line(x1,y1-1,x2,y1-1,WinGrisC);
}

void CGraphContext::GVline(int x, int y1, int y2) {
   line(x,y1,x,y2,WinGrisF);
   line(x+1,y1,x+1,y2,WinGrisC);
}

