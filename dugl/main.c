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
#include "dugl.h"
#include "intrdugl.h"

bool dgEnableFullScreen = false;
int dgLastErrID = 0;
DgWindowResizeCallBack dgWindowResizeCallBack = NULL;
DgWindowResizeCallBack dgWindowPreResizeCallBack = NULL;

///////////////////////------------------------------


int  DgInitMainWindow(const char *title, int ResHz, int ResVt, char BitsPixel) {
    return DgInitMainWindowX(title, ResHz, ResVt, BitsPixel, -1, -1, 0, 0, 0);
}

int DgInitMainWindowX(const char *, int ResHz, int ResVt, char BitsPixel, int , int , bool , bool , bool ) {
    if ((BitsPixel != 16 && BitsPixel != 8) || ResHz < 1 || ResVt < 1)
        return 0;

/*    int posX = SDL_WINDOWPOS_UNDEFINED;
    int posY = SDL_WINDOWPOS_UNDEFINED;
    Uint32 FlagCreate = 0;

    // Main windows already initialized ?
    if (DgWindow != NULL)
        return 0;

    if (BitsPixel != 16 || ResHz <= 1 || ResVt <= 1)
        return 0;
    if (PosX >= 0) posX = PosX;
    if (PosY >= 0) posY = PosY;

    FlagCreate |= SDL_WINDOW_SHOWN;
    if (Borderless)
        FlagCreate |= SDL_WINDOW_BORDERLESS;
    if (ResizeWin)
        FlagCreate |= SDL_WINDOW_RESIZABLE;

    if (SDL_GetCurrentDisplayMode(0, &orgDispMode) != 0) {
        dgLastErrID = DG_ERSS_FAIL_QUERY_DISPLAY_MODE;
        return 0;
    }

    DgWindow = SDL_CreateWindow(title, posX, posY, ResHz, ResVt, FlagCreate);
    if (DgWindow == NULL) {
        dgLastErrID = DG_ERSS_WINDOW_CREATION_FAIL;
        return 0;
    }
    surfaceW = SDL_GetWindowSurface(DgWindow);
    if (surfaceW == NULL) {
        dgLastErrID = DG_ERSS_WINDOW_SURFACE_CREATION_FAIL;
        return 0;
    }

    // VSync
    vsyncRenderer = SDL_CreateSoftwareRenderer(surfaceW);
    if (vsyncRenderer != NULL) {
        SDL_RenderSetVSync(vsyncRenderer, 1);
        vsyncTexture = SDL_CreateTexture(vsyncRenderer, surfaceW->format->format, SDL_TEXTUREACCESS_STREAMING, 1, 1);
        if (vsyncTexture == NULL) {
            SDL_DestroyRenderer(vsyncRenderer);
            vsyncRenderer = NULL;
        }
    }

    if (!CreateSurf(&RendSurf, ResHz, ResVt, 16))
        return 0;

    // create the render Surface 16bpp
    surf16bpp = SDL_CreateRGBSurfaceWithFormatFrom((void*)(RendSurf->rlfb), ResHz, ResVt, 16, ResHz*2, SDL_PIXELFORMAT_RGB565);
    if (surf16bpp == NULL) {
        dgLastErrID = DG_ERSS_FAIL_CREATE_SYSTEM_RGB_SURF;
        return 0;
    }
    if (enableDoubleBuff) {
        if (!CreateSurf(&RendFrontSurf, ResHz, ResVt, 16))
            return 0;

        surfFront16bpp = SDL_CreateRGBSurfaceWithFormatFrom((void*)(RendFrontSurf->rlfb), ResHz, ResVt, 16, ResHz*2, SDL_PIXELFORMAT_RGB565);
        if (surfFront16bpp == NULL) {
            dgLastErrID = DG_ERSS_FAIL_CREATE_SYSTEM_RGB_SURF;
            return 0;
        }
    }

    dgEnableFullScreen = false;
    if (FullScreen) {
        DgToggleFullScreen(true);
    }

    if (MsScanEvents == 1) {
        if (LockDMutex(mutexEvents) == 0) {
            MsZ = 0;
            ClearMsEvntsStack();
            EnableMsEvntsStack();
            MsScanEvents = 1;
            if (DgWindow != NULL) {
                if (DgWindow == SDL_GetMouseFocus()) {
                    MsInWindow = 1;
                    SDL_ShowCursor(SDL_DISABLE);
                } else {
                    MsInWindow = 0;
                    SDL_ShowCursor(SDL_ENABLE);
                }
                DgView defaultMsView;
                GetSurfView(RendSurf, &defaultMsView);
                SetMouseRView(&defaultMsView);
                if (MsInWindow == 1) {
                    SDL_GetMouseState(&MsX, &MsY);
                    iSetMousePos(MsX, MsY);
                    iPushMsEvent(MS_EVNT_MOUSE_MOVE);
                }
            }
            UnlockDMutex(mutexEvents);
        }
    }
    dgWindowResizeCallBack = NULL;
    dgWindowPreResizeCallBack = NULL;
    dgResizeWinMutex = NULL;
    dgRequestResizeWinMutex = NULL;
*/
    return 1;
}

void DgSetMainWindowSize(int ResHz, int ResVt) {
/*    if (DgWindow != NULL) {
        SDL_SetWindowSize(DgWindow, ResHz, ResVt);
    }*/
}

void DgSetMainWindowMinSize(int minResHz, int minResVt) {
/*    if (DgWindow != NULL) {
        SDL_SetWindowMinimumSize(DgWindow, minResHz, minResVt);
    }*/
}

void DgSetMainWindowMaxSize(int maxResHz, int maxResVt) {
/*    if (DgWindow != NULL) {
        SDL_SetWindowMaximumSize(DgWindow, maxResHz, maxResVt);
    }*/
}

void DgGetMainWindowSize(int *ResHz, int *ResVt) {
/*    if (DgWindow != NULL) {
        SDL_GetWindowSize(DgWindow, ResHz, ResVt);
    }*/
}

void DgGetMainWindowMinSize(int *minResHz, int *minResVt) {
/*    if (DgWindow != NULL) {
        SDL_GetWindowMinimumSize(DgWindow, minResHz, minResVt);
    }*/
}

void DgGetMainWindowMaxSize(int *maxResHz, int *maxResVt) {
/*    if (DgWindow != NULL) {
        SDL_GetWindowMaximumSize(DgWindow, maxResHz, maxResVt);
    }*/
}

void DgSetMainWindowResizeCallBack(DgWindowResizeCallBack preresizeCallBack, DgWindowResizeCallBack resizeCallBack, PDMutex resizeMutex, bool *requestResizeMutex) {
/*    if (DgWindow != NULL) {
        dgWindowResizeCallBack = resizeCallBack;
        dgWindowPreResizeCallBack = preresizeCallBack;
        dgResizeWinMutex = resizeMutex;
        dgRequestResizeWinMutex = requestResizeMutex;
    }*/
}

DgWindowResizeCallBack GetMainWindowResizeCallBack() {
    return dgWindowResizeCallBack;
}

int DgPreferredFullSwidth = 0, DgPreferredFullSheight = 0, DgPreferredFullSrefreshRate = 0;
int DgNFSWidth = 0, DgNFSHeight = 0;

// dummy always full screen
void DgToggleFullScreen(bool) {
}

// always full screen
bool DgIsFullScreen() {
    return true;
}

void DgSetPreferredFullScreenMode(int width, int height, int refreshRate) {
    DgPreferredFullSwidth = (width <= 0) ? 0 : width;
    DgPreferredFullSheight = (height <= 0) ? 0 : height;
    DgPreferredFullSrefreshRate = (refreshRate <= 0) ? 0 : refreshRate;
}

void DgGetPreferredFullScreenMode(int *width, int *height, int *refreshRate) {
    if (width != NULL) {
        *width = DgPreferredFullSwidth;
    }
    if (height != NULL) {
        *height = DgPreferredFullSheight;
    }
    if (refreshRate != NULL) {
        *refreshRate = DgPreferredFullSrefreshRate;
    }
}

// dummy
void DgCheckEvents() {
}

// dummy
void DgSetWindowIcone(DgSurf) {
}

void DgUpdateWindow() {
/*    SDL_Surface *tmpSDLSurf = surf16bpp;
    DgSurf *tmpSurf;
    surfaceW = SDL_GetWindowSurface(DgWindow);

    if (RendSurf->rlfb != 0 && surfaceW != NULL) {
        SDL_BlitSurface( tmpSDLSurf, NULL, surfaceW, NULL );
        SDL_UpdateWindowSurface(DgWindow);

        if (enableDoubleBuff) {
            tmpSurf = RendSurf;

            surf16bpp = surfFront16bpp;
            RendSurf = RendFrontSurf;

            surfFront16bpp = tmpSDLSurf;
            RendFrontSurf = tmpSurf;
        }
    }*/
}



//////////////////////-------------------------------

int DgGetLastErr() {
    return dgLastErrID;
}


void BlurSurf16(DgSurf *S16Dst, DgSurf *S16Src) {
    if (S16Dst==NULL || S16Src==NULL ||
            S16Dst->BitsPixel!=16 || S16Src->BitsPixel!=16 ||
            S16Dst->ResH!=S16Src->ResH ||
            S16Dst->ResV!=S16Src->ResV) return;
    Blur16((void*)(S16Dst->rlfb), (void*)(S16Src->rlfb), S16Src->ResH, S16Src->ResV, 0, (S16Src->ResV - 1));
}

void ConvSurf16ToSurf8(DgSurf *S8Dst, DgSurf *S16Src)
{	if (S8Dst==NULL || S16Src==NULL ||
	    S8Dst->BitsPixel!=8 || S16Src->BitsPixel!=16 ||
	    S8Dst->ResH!=S16Src->ResH ||
	    S8Dst->ResV!=S16Src->ResV) return;
	// convert buffers
	TrImgResHz=S8Dst->ResH;
	TrImgResVt=S8Dst->ResV;
	TrBuffImgSrc=(void*)(S16Src->rlfb);
	TrBuffImgDst=(void*)(S8Dst->rlfb);
	TransfB16ToB8();
}

void ConvSurf8ToSurf16(DgSurf *S16Dst, DgSurf *S8Src)
{	if (S8Src==NULL || S16Dst==NULL ||
	    S16Dst->BitsPixel!=16 || S8Src->BitsPixel!=8 ||
	    S8Src->ResH!=S16Dst->ResH ||
	    S8Src->ResV!=S16Dst->ResV) return;
	// convert buffers
	TrImgResHz=S8Src->ResH;
	TrImgResVt=S8Src->ResV;
	TrBuffImgSrc=(void*)(S8Src->rlfb);
	TrBuffImgDst=(void*)(S16Dst->rlfb);
	TransfB8ToB16();
}

void ConvSurf8ToSurf16Pal(DgSurf *S16Dst, DgSurf *S8Src,void *PalBGR1024)
{	if (S8Src==NULL || S16Dst==NULL ||
	    S16Dst->BitsPixel!=16 || S8Src->BitsPixel!=8 ||
	    S8Src->ResH!=S16Dst->ResH ||
	    S8Src->ResV!=S16Dst->ResV) return;
	// convert buffers
	TrImgResHz=S8Src->ResH;
	TrImgResVt=S8Src->ResV;
	TrBuffImgSrc=(void*)(S8Src->rlfb);
	TrBuffImgDst=(void*)(S16Dst->rlfb);
	TrBuffImgSrcPal=PalBGR1024;
	TransfB8ToB16Pal();
}

int GetPixelSize(int bitsPixel) {
   switch (bitsPixel) {
      case 8:  return 1;
      case 15: return 2;
      case 16: return 2;
      case 24: return 3;
      case 32: return 4;
      default : return 0;
   }

}

void SetOrgSurf(DgSurf *S,int LOrgX,int LOrgY)
{	int dx,dy;
	int pixelsize=GetPixelSize(S->BitsPixel);
	dx=LOrgX-S->OrgX;
	dy=LOrgY-S->OrgY;
	S->MinX-= dx;
	S->MaxX-= dx;
	S->MinY-= dy;
	S->MaxY-= dy;
	S->OrgX= LOrgX;
	S->OrgY= LOrgY;
	if (pixelsize>1)
		S->vlfb= S->rlfb+(S->OrgX*pixelsize)-((S->OrgY-(S->ResV-1))*S->ResH*pixelsize);
	else
		S->vlfb= S->rlfb+S->OrgX-(S->OrgY-(S->ResV-1))*S->ResH;
}

void SetOrgVSurf(int OrgX,int OrgY)
{	int i;
 	for (i=0;i<NbVDgSurf;i++)
	   SetOrgSurf(&VSurf[i],OrgX,OrgY);
}

int  CreateSurf(DgSurf **S, int ResHz, int ResVt, char BitsPixel)
{	int cvlfb;
    int pixelsize=GetPixelSize(BitsPixel);
    if (BitsPixel != 8 && BitsPixel != 16)
		return 0;
	if (pixelsize == 0 || ResHz<MIN_DGSURF_WIDTH || ResVt<MIN_DGSURF_HEIGHT)
		return 0;
    *S = (DgSurf*)malloc(sizeof(DgSurf)+ResHz*ResVt*pixelsize);
    if ((*S) == NULL) {
        return 0;
    }
	cvlfb=(void*)(&(*S)[1]);
	if (cvlfb!=NULL) {
	  (*S)->vlfb=(*S)->rlfb= cvlfb;
	  (*S)->OffVMem= -1;
	  (*S)->ResH= ResHz;
	  (*S)->ResV= ResVt;

	  (*S)->MaxX= ResHz-1;
	  (*S)->MaxY= (*S)->MinX= 0;
	  (*S)->MinY= -ResVt+1;      //axe Y montant
	  (*S)->SizeSurf= ResHz*ResVt*pixelsize;
	  (*S)->Mask= 0;
	  (*S)->OrgX= 0;
	  (*S)->OrgY= ResVt-1;
      (*S)->BitsPixel= BitsPixel;
	  (*S)->ScanLine= ResHz *pixelsize;
	  (*S)->NegScanLine = -((*S)->ScanLine);
	  SetOrgSurf((*S),0,0);
	  return 1;
	}
	free((*S));
	return 0;
}

int  CreateSurfBuff(DgSurf **S, int ResHz, int ResVt, char BitsPixel,void *Buff) {
    int pixelsize=GetPixelSize(BitsPixel);

    if (BitsPixel != 8 && BitsPixel != 16)
		return 0;
	if (pixelsize == 0 || ResHz<MIN_DGSURF_WIDTH || ResVt<MIN_DGSURF_HEIGHT || Buff == NULL) {
		return 0;
	}
    *S = (DgSurf*)malloc(sizeof(DgSurf));
    if ((*S) == NULL)
        return 0;
    (*S)->vlfb=(*S)->rlfb= Buff;
    (*S)->OffVMem= -1;
    (*S)->ResH= ResHz;
    (*S)->ResV= ResVt;
    (*S)->MaxX= ResHz-1;
    (*S)->MaxY= (*S)->MinX= 0;
    (*S)->MinY= -ResVt+1;      //axe Y montant
    (*S)->SizeSurf= ResHz*ResVt*pixelsize;
    (*S)->Mask= 0;
    (*S)->OrgX= 0;
    (*S)->OrgY= ResVt-1;
    (*S)->BitsPixel= BitsPixel;
    (*S)->ScanLine= ResHz *pixelsize;
    (*S)->NegScanLine = -((*S)->ScanLine);
    SetOrgSurf((*S),0,0);
    return 1;
}

int CreateSurfBuffView(DgSurf **S, int ResHz, int ResVt, char BitsPixel, void *Buff, DgView *V) {
    if ((*S = (DgSurf*)malloc(sizeof(DgSurf))) == NULL) {
        //dgLastErrID = DG_ERSS_NO_MEM;
        return 0;
    }
    int pixelsize=GetPixelSize(BitsPixel);

    memset(*S, 0, sizeof(DgSurf));
    if (pixelsize == 0 || ResHz<MIN_DGSURF_WIDTH || ResVt<MIN_DGSURF_HEIGHT || Buff == NULL) {
        free(*S);
        *S = NULL;
        //dgLastErrID = DG_ERSS_INVALID_DGSURF_FORMAT;
        return 0;
    }
    (*S)->ResH= ResHz;
    (*S)->ResV= ResVt;
    (*S)->MaxX= V->MaxX;
    (*S)->MaxY= V->MaxY;
    (*S)->MinX= V->MinX;
    (*S)->MinY= V->MinX;
    (*S)->SizeSurf= ResHz*ResVt*pixelsize;
    (*S)->OrgX= V->OrgX;
    (*S)->OrgY= V->OrgY;
    (*S)->BitsPixel= BitsPixel;
    (*S)->ScanLine= ResHz *pixelsize;
    (*S)->Mask= 0;
    (*S)->NegScanLine = -((*S)->ScanLine);
    (*S)->rlfb = (int)(Buff);
    (*S)->vlfb = (int)(Buff)+(V->OrgX*pixelsize)-((V->OrgY-(ResVt-1))*ResHz*pixelsize);
    return 1;
}

void DestroySurf(DgSurf *S) {
    if (S != NULL && S->OffVMem == -1) {
	  S->vlfb=S->rlfb=S->OffVMem=0;
	  free(S);
	}
}

void SetSurfView(DgSurf *S, DgView *V) {
    int pixelsize = GetPixelSize(S->BitsPixel);
    // clip if required
    int RMaxX= ((V->MaxX+V->OrgX)<S->ResH) ? V->MaxX+V->OrgX : S->ResH-1;
    int RMaxY= ((V->MaxY+V->OrgY)<S->ResV) ? V->MaxY+V->OrgY : S->ResV-1;
    int RMinX= ((V->MinX+V->OrgX)>=0) ? V->MinX+V->OrgX : 0;
    int RMinY= ((V->MinY+V->OrgY)>=0) ? V->MinY+V->OrgY : 0;

    S->OrgX= V->OrgX;
    S->OrgY= V->OrgY;
    S->MaxX= RMaxX-S->OrgX;
    S->MinX= RMinX-S->OrgX;
    S->MaxY= RMaxY-S->OrgY;
    S->MinY= RMinY-S->OrgY;
    if (pixelsize > 1)
        S->vlfb= S->rlfb+(S->OrgX*pixelsize)-((S->OrgY-(S->ResV-1))*S->ResH*pixelsize);
    else
        S->vlfb= S->rlfb+S->OrgX-(S->OrgY-(S->ResV-1))*S->ResH;
}

// les coordonnees des limite sont relative a l'origine de l'ecran
void SetSurfInView(DgSurf *S, DgView *V) {
    int pixelsize = GetPixelSize(S->BitsPixel);
    int RMaxX= S->MaxX+S->OrgX;
    int RMaxY= S->MaxY+S->OrgY;
    int RMinX= S->MinX+S->OrgX;
    int RMinY= S->MinY+S->OrgY;

    // clip DgView if required
    if ((V->MaxX+V->OrgX)<RMaxX) {
        RMaxX = V->MaxX+V->OrgX;
    }
    if ((V->MaxY+V->OrgY)<RMaxY) {
        RMaxY= V->MaxY+V->OrgY;
    }
    if ((V->MinX+V->OrgX)>RMinX) {
        RMinX= V->MinX+V->OrgX;
    }
    if ((V->MinY+V->OrgY)>RMinY) {
        RMinY= V->MinY+V->OrgY;
    }
    S->OrgX= V->OrgX;
    S->OrgY= V->OrgY;
    S->MaxX= RMaxX-S->OrgX;
    S->MaxY= RMaxY-S->OrgY;
    S->MinX= RMinX-S->OrgX;
    S->MinY= RMinY-S->OrgY;
    if (pixelsize > 1)
        S->vlfb= S->rlfb+(S->OrgX*pixelsize)-((S->OrgY-(S->ResV-1))*S->ResH*pixelsize);
    else
        S->vlfb= S->rlfb+S->OrgX-(S->OrgY-(S->ResV-1))*S->ResH;
}

void SetSurfViewBounds(DgSurf *S, DgView *V) {
    // clip if required
    int RMaxX= ((V->MaxX+S->OrgX)<S->ResH) ? V->MaxX+S->OrgX : S->ResH-1;
    int RMaxY= ((V->MaxY+S->OrgY)<S->ResV) ? V->MaxY+S->OrgY : S->ResV-1;
    int RMinX= ((V->MinX+S->OrgX)>=0) ? V->MinX+S->OrgX : 0;
    int RMinY= ((V->MinY+S->OrgY)>=0) ? V->MinY+S->OrgY : 0;

    S->MaxX= RMaxX-S->OrgX;
    S->MinX= RMinX-S->OrgX;
    S->MaxY= RMaxY-S->OrgY;
    S->MinY= RMinY-S->OrgY;
}

void SetSurfInViewBounds(DgSurf *S, DgView *V) {
    int RMaxX= S->MaxX+S->OrgX;
    int RMaxY= S->MaxY+S->OrgY;
    int RMinX= S->MinX+S->OrgX;
    int RMinY= S->MinY+S->OrgY;

    // clip View if required
    if ((V->MaxX+S->OrgX)<RMaxX) {
        RMaxX = V->MaxX+S->OrgX;
    }
    if ((V->MaxY+S->OrgY)<RMaxY) {
        RMaxY= V->MaxY+S->OrgY;
    }
    if ((V->MinX+S->OrgX)>RMinX) {
        RMinX= V->MinX+S->OrgX;
    }
    if ((V->MinY+S->OrgY)>RMinY) {
        RMinY= V->MinY+S->OrgY;
    }
    S->MaxX= RMaxX-S->OrgX;
    S->MaxY= RMaxY-S->OrgY;
    S->MinX= RMinX-S->OrgX;
    S->MinY= RMinY-S->OrgY;
}


void GetSurfView(DgSurf *S, DgView *V)
{	V->OrgX=S->OrgX;  V->OrgY=S->OrgY;
	V->MaxX=S->MaxX;  V->MaxY=S->MaxY;
	V->MinX=S->MinX;  V->MinY=S->MinY;
}


int dgCurDisplayMode = -1;

int DgGetFirstDisplayMode(int *width, int *height, int *bpp, int *refreshRate) {
    if (TbDgfxModes != NULL && NbDgfxModes > 0) {
			*width = TbDgfxModes[0].ResHz;
			*height = TbDgfxModes[0].ResVt;
			*bpp = TbDgfxModes[0].BitPixel;
			*refreshRate = TbDgfxModes[0].VtFreq;
			dgCurDisplayMode = 1;
			return NbDgfxModes;
    }

    dgCurDisplayMode = -1;
    return 0;
}

bool DgGetNextDisplayMode(int *width, int *height, int *bpp, int *refreshRate) {
    if (TbDgfxModes != NULL && NbDgfxModes > 0 && dgCurDisplayMode != -1 && dgCurDisplayMode < NbDgfxModes) {
			*width = TbDgfxModes[dgCurDisplayMode].ResHz;
			*height = TbDgfxModes[dgCurDisplayMode].ResVt;
			*bpp = TbDgfxModes[dgCurDisplayMode].BitPixel;
			*refreshRate = TbDgfxModes[dgCurDisplayMode].VtFreq;
			dgCurDisplayMode++;
			return true;
    }

    dgCurDisplayMode = -1;
    return false;
}



// -----------------------
// 16 bpp helper function
// -----------------------

/*void line16(int X1,int Y1,int X2,int Y2,int LgCol) {
	int P[4];
	P[0]=X1; P[1]=Y1;
	P[2]=X2; P[3]=Y2;
	Line16(&P[0],&P[2],LgCol);
}

void linemap16(int X1,int Y1,int X2,int Y2,int LgCol,unsigned int Map) {
	int P[4];
	P[0]=X1; P[1]=Y1;
	P[2]=X2; P[3]=Y2;
	LineMap16(&P[0],&P[2],LgCol,Map);
}

void lineblnd16(int X1,int Y1,int X2,int Y2,int LgCol) {
	int P[4];
 	P[0]=X1; P[1]=Y1;
 	P[2]=X2; P[3]=Y2;
	LineBlnd16(&P[0],&P[2],LgCol);
}

void linemapblnd16(int X1,int Y1,int X2,int Y2,int LgCol,unsigned int Map) {
	int P[4];
 	P[0]=X1; P[1]=Y1;
 	P[2]=X2; P[3]=Y2;
	LineMapBlnd16(&P[0],&P[2],LgCol,Map);
}*/

void cputpixelblnd16(int X,int Y,int pcol) {
	int Pt[2];
	Pt[0]=X; Pt[0]=Y;
	LineBlnd16(Pt,Pt,pcol);
}

void CPutPixelBlnd16(void *Pt1,int pcol) {
	LineBlnd16(Pt1,Pt1,pcol);
}

void bar16(int x1,int y1,int x2,int y2,int bcol) {
	if (x1==x2 || y1==y2) {
	  line16(x1,y1,x2,y2,bcol);
	  return;
    }
    Bar16(&x1, &x2, bcol);
}

void barblnd16(int x1,int y1,int x2,int y2,int bcol) {
    BarBlnd16(&x1, &x2, bcol);
}

void rect16(int x1,int y1,int x2,int y2,int rcol) {
    line16(x1,y1,x2,y1,rcol);
    line16(x1,y2,x2,y2,rcol);
    if (y2>y1) {
      line16(x1,y1+1,x1,y2-1,rcol);
      line16(x2,y1+1,x2,y2-1,rcol);
    }
    else
      if (y2<y1) {
        line16(x1,y1-1,x1,y2+1,rcol);
        line16(x2,y1-1,x2,y2+1,rcol);
      }
}

void rectmap16(int x1,int y1,int x2,int y2,int rcol,unsigned int rmap) {
    linemap16(x1,y1,x2,y1,rcol,rmap);
    linemap16(x1,y2,x2,y2,rcol,rmap);
    if (y2>y1) {
      linemap16(x1,y1+1,x1,y2-1,rcol,rmap);
      linemap16(x2,y1+1,x2,y2-1,rcol,rmap);
    }
    else
      if (y2<y1) {
        linemap16(x1,y1-1,x1,y2+1,rcol,rmap);
        linemap16(x2,y1-1,x2,y2+1,rcol,rmap);
      }
}

void rectblnd16(int x1,int y1,int x2,int y2,int rcol) {
    lineblnd16(x1,y1,x2,y1,rcol);
    lineblnd16(x1,y2,x2,y2,rcol);
    if (y2>y1) {
      lineblnd16(x1,y1+1,x1,y2-1,rcol);
      lineblnd16(x2,y1+1,x2,y2-1,rcol);
    }
    else
      if (y2<y1) {
        lineblnd16(x1,y1-1,x1,y2+1,rcol);
        lineblnd16(x2,y1-1,x2,y2+1,rcol);
      }
}

void rectmapblnd16(int x1,int y1,int x2,int y2,int rcol,unsigned int rmap) {
    linemapblnd16(x1,y1,x2,y1,rcol,rmap);
    linemapblnd16(x1,y2,x2,y2,rcol,rmap);
    if (y2>y1) {
      linemapblnd16(x1,y1+1,x1,y2-1,rcol,rmap);
      linemapblnd16(x2,y1+1,x2,y2-1,rcol,rmap);
    }
    else
      if (y2<y1) {
        linemapblnd16(x1,y1-1,x1,y2+1,rcol,rmap);
        linemapblnd16(x2,y1-1,x2,y2+1,rcol,rmap);
      }
}


void putpixel16(int X,int Y,int pcol) { // utilise PutPixel
	int P[2];
	P[0]=X; P[1]=Y;
	PutPixel16(&P,pcol);
}

void cputpixel16(int X,int Y,int pcol) { // avec clip
	int P[2];
	if (X>CurSurf.MaxX || X<CurSurf.MinX || Y>CurSurf.MaxY || Y<CurSurf.MinY) return;
	P[0]=X; P[1]=Y;
	PutPixel16(&P,pcol);
}

int getpixel16(int X,int Y) { // utilise GetPixel
	int P[2];
	P[0]=X; P[1]=Y;
	return GetPixel16(&P);
}

int cgetpixel16(int X,int Y) { // avec clip
	int P[2];
	if (X>CurSurf.MaxX || X<CurSurf.MinX || Y>CurSurf.MaxY || Y<CurSurf.MinY) return 0;
	P[0]=X; P[1]=Y;
	return GetPixel16(&P);
}
