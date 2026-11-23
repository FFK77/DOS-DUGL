#ifndef DDOSCORE_H_INCLUDED
#define DDOSCORE_H_INCLUDED


#define VMODE_SUPPORTED         ( 1 << 0 )  // 1 if this video mode is supported by hardware, else 0
#define VMODE_TTY               ( 1 << 2 )  // 1 output bios functions available, else 0
#define VMODE_COLOR             ( 1 << 3 )  // 1 if color mode, else mono (black/white)
#define VMODE_GRAPHIC           ( 1 << 4 )  // 1 if graphic mode, else 0 text mode
#define VMODE_VGA               ( 1 << 5 )  // 1 if VGA compatible mode, else 0
#define VMODE_BANKED            ( 1 << 6 )  // 1 if this mode support BANK switching, else 0
#define VMODE_LFB               ( 1 << 7 )  // 1 if this mode support LFB (Linear Frame Buffer), else 0

typedef struct
{	int    	ResHz;
	int	    ResVt;
	int	    rlfb;
	short  	Mode;
	short   VModeFlag;
	short   WinGranularity; // smallest WIN boundary, in KB
    short   WinSeg;
	char	BitPixel;
	char	VtFreq;
	char    WinID; // valid/writable winID; 0:WinA, 1:WinB
} ModeInfo;


extern	int	            NbVDgSurf,NbDgfxModes;
extern	DgSurf	        *VSurf;
extern	ModeInfo        *TbDgfxModes, *CurDgfxMode;
extern	int	            CurModeVtFreq;
extern	unsigned int    SizeVMem;
extern  unsigned char   VesaHiVers,VesaLoVers;

extern int              CurViewVSurf;
extern unsigned char    TbDegCol[256*64];
extern void             *PtrTbColConv;

#ifdef __cplusplus
extern "C" {
#endif

// CPU tools
int  DetectCPUID();
void ExecCPUID(unsigned int VEAX,unsigned int *PEAX, unsigned int *PEBX,
	       unsigned int *PECX,unsigned int *PEDX);
int  DetectMMX();
// Init Any available VESA 8bpp or 16bpp Mode
int  InitVesaMode(int ResHz, int ResVt, char BitPixel,int NbVPage);
// Init the standard mode 0x3 text mode
void TextMode();
// force banked mode when calling InitVesaMode, else LFB is initialised by default if available
void SetForceBankedVESA(bool bForceBank);
bool GetForceBankedVESA();

// 8 bpp Color palette and light table helper function
// ---------------------------------------------------
void PrBuildTbColConv(void *PalBGR1024,float PropBonus);
void BuildTbColConv(void *PalBGR1024);
void BuildTbDegCol(void *PalBGR1024);
void PrBuildTbDegCol(void *PalBGR1024,float PropBonus);
int  FindCol(int DebCol,int FinCol,int B,int G,int R,void *PalBGR1024);
int  PrFindCol(int DebCol,int FinCol,
               int B,int G,int R,void *PalBGR1024,float PropBonus);
// Set palette :** WARNING ** should be called only after a successfull InitVESA call
extern  void (*SetPalette)(int Dbcol, int Nbcol, void *BGRA);

// 8 bpp drawing functions
// -----------------------

void Clear(int clrcol);  // Clear All the current Surf with clrcol
// *Point is pointer to a struct { int X, int Y };
void PutPixel(void *Point,int col); // unclipped PutPixel
int  GetPixel(void *Point); // unclipped GetPixel
void Line(void *Point1,void *Point2,int col); // Clipped Line
void LineMap(void *Point1,void *Point2,int col,unsigned int Map); // Mapped line
// TypePoly
#define POLY_SOLID				0
#define POLY_TEXT				1
#define POLY_MASK_TEXT			2
#define POLY_FLAT_DEG			3
#define POLY_DEG				4
#define POLY_FLAT_DEG_TEXT		5
#define POLY_MASK_FLAT_DEG_TEXT	6
#define POLY_DEG_TEXT			7
#define POLY_MASK_DEG_TEXT		8
#define POLY_EFF_FLAT_DEG		9
#define POLY_EFF_DEG			10
#define POLY_EFF_COLCONV		11
#define POLY_MAX_TYPE			11
#define POLY_FLAG_DBL_SIDED		0x80000000
void Poly(void *ListPt, DgSurf *SS, unsigned int TypePoly, int ColPoly);
// Redo the last rendered Poly: *ListPt is ignored in this call,
// if the last Poly is a reversed Double sided poly and POLY_FLAG_DBL_SIDED isn't enabled the RePoly will be skipped
// user can update *SS, TypePoly, ColPoly and texture coordinates[U,V] using the same Point List pointers the Poly was called with
// Should be used through REPOLY else LastPolyStatus 'N' will not be ignored
// if used after Poly16 /not Poly behavior, will be undefined
void RePoly(void *ListPt, DgSurf *SS, unsigned int TypePoly, int ColPoly);
// REPOLY provided for convenience as RePoly16 handle only drawn polygones with status 'C' or 'I' to avoid useless calls
#define REPOLY(ListPt, SS, TypePoly, ColPoly) if (LastPolyStatus!='N') RePoly(ListPt, SS, TypePoly, ColPoly);

// Surf blitting functions
void PutSurf(DgSurf *S,int X,int Y,int PType);
void PutMaskSurf(DgSurf *S,int X,int Y,int PType);

// 8 bpp Drawing helper functions provided for convenience
// -------------------------------------------------------

void ClearSurf(int clrcol);	// use Poly, clear current View
void line(int X1,int Y1,int X2,int Y2,int LgCol);
void linemap(int X1,int Y1,int X2,int Y2,int LgCol,unsigned int Map);
void putpixel(int X,int Y,int pcol);	// use PutPixel
void cputpixel(int X,int Y,int pcol); // clipped
int  getpixel(int X,int Y);	// use GetPixel
int  cgetpixel(int X,int Y); // clipped
void Bar(void *Pt1,void *Pt2,int bcol);  // use Poly
void bar(int x1,int y1,int x2,int y2,int bcol);  // use Poly
void rect(int x1,int y1,int x2,int y2,int rcol);
void rectmap(int x1,int y1,int x2,int y2,int rcol,unsigned int rmap);

// ** WARNING ** should be called only after a successfull InitVESA call
// set current visible Surf Index
extern  void (*ViewSurf)(int NbSurf);
// wait vertical retrace and set current visible Surf Index
extern  void (*ViewSurfWaitVR)(int NbSurf);

#ifdef __cplusplus
           }
#endif

#endif // DDOSCORE_H_INCLUDED
