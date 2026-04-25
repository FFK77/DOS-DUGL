#ifndef DGRAPH_H
#define DGRAPH_H

typedef struct {
	int ScanLine;
    int rlfb;
    int OrgX, OrgY;
    int MaxX, MaxY, MinX, MinY;
    int Mask, ResH, ResV;
    int vlfb;
    int NegScanLine;
    int OffVMem;
    int BitsPixel;
    int SizeSurf;

} DgSurf;

typedef struct {
	int	OrgX, OrgY;
	int	MaxX, MaxY, MinX, MinY;
} DgView;

#define VMODE_SUPPORTED         1
#define VMODE_TTY               4
#define VMODE_COLOR             8
#define VMODE_GRAPHIC           16
#define VMODE_VGA               32
#define VMODE_VGAW              64
#define VMODE_LFB               128

typedef struct
{	int    	ResHz;
	int	    ResVt;
	int	    rlfb;
	short  	Mode;
	short   VModeFlag;
	char	BitPixel;
	char	VtFreq,resv;
} ModeInfo;

//** CHR FONT FORMAT ****************************
typedef struct
{	int				DatCar;
	char	        PlusX,PlusLgn;
	unsigned char   Ht,Lg;
} Caract;

typedef struct
{	int	       	Sign;  		// = "FCHR"
	char	    MaxHautFnt,
		       	MaxHautLgn,
		       	MinPlusLgn,
		       	SensFnt;
	int	       	SizeDataCar,
				PtrBuff;
	int			Resv[28];
	Caract	    C[256];
} HeadCHR;

typedef struct {
	int  			FntPtr;
	unsigned char	FntHaut,FntDistLgn;
	char			FntLowPos, FntHighPos,FntSens;
	unsigned char	FntTab,Fntrevb[2];
	int				FntX, FntY, FntCol,FntBCol,FntDresv;
} DFONT;

// DUGL Graphics Global vars
extern	int	      NbVDgSurf,NbDgfxModes;
extern	DgSurf	      *VSurf;
extern	ModeInfo      *TbDgfxModes, *CurDgfxMode;
extern	int	      CurModeVtFreq;
extern	unsigned int  SizeVMem;
extern  unsigned char VesaHiVers,VesaLoVers;

extern DgSurf CurSurf;
extern int  CurViewVSurf;
extern unsigned char TbDegCol[256*64];
extern void *PtrTbColConv;
extern char LastPolyStatus; // Warning ReadOnly! used internally by (Poly16, RePoly16) / (Poly, RePoly)
                            // Last Rendered Poly Status: ='N' not rendered, ='C' clipped, ='I' In rendererd


#ifdef __cplusplus
extern "C" {
#endif

// *NEW* DUGL 1.1+ should be called before any floating point processing
#define FREE_MMX()  asm("EMMS\n")

// init all ressources required to run DUGL
// return 1 if success 0 if fail
int DgInit();
// free all ressources allocated to run DUGL
void DgQuit();
// CPU tools
int  DetectCPUID();
void ExecCPUID(unsigned int VEAX,unsigned int *PEAX, unsigned int *PEBX,
	       unsigned int *PECX,unsigned int *PEDX);
int  DetectMMX();
// Init Any available VESA 8bpp or 16bpp Mode
int  InitVesaMode(int ResHz, int ResVt, char BitPixel,int NbVPage);
// Enumerate full screen display modes
// return count of display modes, and fill attributes of first display mode, if any
int DgGetFirstDisplayMode(int *width, int *height, int *bpp, int *refreshRate);
// fill next Display mode attributes if any and return true, else return false
bool DgGetNextDisplayMode(int *width, int *height, int *bpp, int *refreshRate);
// Init the standard mode 0x3 text mode
void TextMode();
// standard VGA Wait Retrace
void WaitRetrace();

// Universal 8bpp 16bpp Surf Manipulation Functions
// ------------------------------------------------

// Set Current DgSurf for rendering
void DgSetCurSurf(DgSurf *S);
// Get copy of CurSurf
void DgGetCurSurf(DgSurf *S);
// Set Source DgSurf
void DgSetSrcSurf(DgSurf *S);

int  GetMaxResVSetSurf(); // Max Height in pixels for a surf used with SetSurf
void SetOrgSurf(DgSurf *S,int LOrgX,int LOrgY);
// sets DgSurf View
void SetSurfView(DgSurf *S, DgView *V);
// sets View port clipped inside current DgSurf view port
void SetSurfInView(DgSurf *S, DgView *V);
// sets DgSurf View Bounds (ignoring the new View Origin)
void SetSurfViewBounds(DgSurf *S, DgView *V);
// sets View port Bounds clipped inside current DgSurf view port (ignoring the new View Origin)
void SetSurfInViewBounds(DgSurf *S, DgView *V);
// Get DgSurf View
void GetSurfView(DgSurf *S, DgView *V);
void SetOrgVSurf(int OrgX,int OrgY);
int CreateSurf(DgSurf **S, int ResHz, int ResVt, char BitsPixel);
int CreateSurfBuff(DgSurf **S, int ResHz, int ResVt, char BitsPixel,void *Buff);
int CreateSurfBuffView(DgSurf **S, int ResHz, int ResVt, char BitsPixel, void *Buff, DgView *V);
void DestroySurf(DgSurf *S);
// ** WARNING ** should be called only after a successfull InitVESA call
// set current visible Surf Index
extern  void (*ViewSurf)(int NbSurf);
// wait vertical retrace and set current visible Surf Index
extern  void (*ViewSurfWaitVR)(int NbSurf);

// Surf conversion Functions
// -------------------------
void ConvSurf16ToSurf8(DgSurf *S8Dst, DgSurf *S16Src);
void ConvSurf8ToSurf16(DgSurf *S16Dst, DgSurf *S8Src);
void ConvSurf8ToSurf16Pal(DgSurf *S16Dst, DgSurf *S8Src,void *PalBGR1024);
void Blur16(void *BuffImgDst, void *BuffImgSrc, int ImgWidth, int ImgHeight, int StartLine, int EndLine);
void BlurSurf16(DgSurf *S16Dst, DgSurf *S16Src); // use Blur16

// 16 bpp Surf Copy/Filter
// -----------------------
void DgMemCpy(void *dst, void *src, unsigned int sizeCopy);
void SurfCopy(DgSurf *Sdst, DgSurf *Ssrc);
void SurfCopyBlnd16(DgSurf *S16Dst, DgSurf *S16Src,int colBlnd);
void SurfMaskCopyBlnd16(DgSurf *S16Dst, DgSurf *S16Src,int colBlnd);
void SurfCopyTrans16(DgSurf *S16Dst, DgSurf *S16Src,int trans);
void SurfMaskCopyTrans16(DgSurf *S16Dst, DgSurf *S16Src,int trans);


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

int  ValidSPoly(void *ListPt);
int  SensPoly(void *ListPt);

// Surf blitting functions
// PType : How the surf is blitted over the surf
#define NORM_PUT	0 // as it
#define INV_HZ_PUT	1 // reversed horizontally
#define INV_VT_PUT	2 // reversed vertically
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

// 16 bpp drawing functions
// ------------------------

#define RGB16(r,g,b) (((b)>>3)|(((g)>>2)<<5)|(((r)>>3)<<11))

void PutPixel16(void *Point,int col);
int GetPixel16(void *Point);
void Clear16(int clrcol);  // clear the entire surface, ignoring the view
void Line16(void *Point1,void *Point2,int col);
void LineMap16(void *Point1,void *Point2,int col,unsigned int Map);
void LineBlnd16(void *Point1,void *Point2,int col);
void LineMapBlnd16(void *Point1,void *Point2,int col,unsigned int Map);

#define POLY16_SOLID			0
#define POLY16_TEXT				1
#define POLY16_MASK_TEXT		2
#define POLY16_TEXT_TRANS       10
#define POLY16_MASK_TEXT_TRANS  11
#define POLY16_RGB              12
#define POLY16_SOLID_BLND		13
#define POLY16_TEXT_BLND		14
#define POLY16_MASK_TEXT_BLND	15
#define POLY16_MAX_TYPE			15
#define POLY16_FLAG_DBL_SIDED	0x80000000
void Poly16(void *ListPt, DgSurf *SS, unsigned int TypePoly, int ColPoly);
// Redo the last rendered Poly16: *ListPt is ignored in this call,
// if the last Poly16 is a reversed Double sided poly and POLY16_FLAG_DBL_SIDED isn't enabled the RePoly16 will be skipped
// user can update *SS, TypePoly, ColPoly and texture coordinates[U,V] using the same Point List pointers the Poly16 was called with
// Should be used through REPOLY16 else LastPolyStatus 'N' will not be ignored
// if used after Poly /not Poly16 behavior will be undefined
void RePoly16(void *ListPt, DgSurf *SS, unsigned int TypePoly, int ColPoly);
// REPOLY16 provided for convenience as RePoly16 handle only drawn polygones with status 'C' or 'I' to avoid useless calls
#define REPOLY16(ListPt, SS, TypePoly, ColPoly) if (LastPolyStatus!='N') RePoly16(ListPt, SS, TypePoly, ColPoly);

void ClearSurf16(int clrcol);	// Clear the current Surf view with clrcol
void line16(int X1,int Y1,int X2,int Y2,int LgCol);
void linemap16(int X1,int Y1,int X2,int Y2,int LgCol,unsigned int Map);
void lineblnd16(int X1,int Y1,int X2,int Y2,int LgCol);
void linemapblnd16(int X1,int Y1,int X2,int Y2,int LgCol,unsigned int Map);
void putpixel16(int X,int Y,int pcol);	// use PutPixel
void cputpixel16(int X,int Y,int pcol); // clipped
int  getpixel16(int X,int Y);	// use GetPixel
int  cgetpixel16(int X,int Y); // clipped
void cputpixelblnd16(int X,int Y,int pcol); // clipped
void CPutPixelBlnd16(void *Pt1,int pcol); // clipped

void InBar16(int minx,int miny,int maxx, int maxy,int bcol);
void Bar16(void *Pt1,void *Pt2,int bcol);
void bar16(int x1,int y1,int x2,int y2,int bcol);
void InBarBlnd16(int minx,int miny,int maxx, int maxy,int bcol);
void BarBlnd16(void *Pt1,void *Pt2,int bcol);
void barblnd16(int x1,int y1,int x2,int y2,int bcol);
void rect16(int x1,int y1,int x2,int y2,int rcol);
void rectmap16(int x1,int y1,int x2,int y2,int rcol,unsigned int rmap);
void rectblnd16(int x1,int y1,int x2,int y2,int rcol);
void rectmapblnd16(int x1,int y1,int x2,int y2,int rcol,unsigned int rmap);

// 16bpp Surf blitting functions
void PutSurf16(DgSurf *S,int X,int Y,int PType);
void PutMaskSurf16(DgSurf *S,int X,int Y,int PType);
void PutSurfBlnd16(DgSurf *S,int X,int Y,int PType,int colBlnd);
void PutMaskSurfBlnd16(DgSurf *S,int X,int Y,int PType,int colBlnd);
void PutSurfTrans16(DgSurf *S,int X,int Y,int PType,int trans);
void PutMaskSurfTrans16(DgSurf *S,int X,int Y,int PType,int trans);

// resize SSrcSurf into CurSurf taking account of source and destination Views
// call to those functions will change SrcSurf, SSrcSurf could be null if there is a valid SrcSurf
void ResizeViewSurf16(DgSurf *SSrcSurf, int swapHz, int swapVt); // fast resize source view => into dest view
void MaskResizeViewSurf16(DgSurf *SSrcSurf, int swapHz, int swapVt); // use SrcSurf::Mask to mask pixels
void BlndResizeViewSurf16(DgSurf *SSrcSurf, int swapHz, int swapVt, int colBlnd); // ColBnd =  color16 | (blend << 24),  blend 0->31 (31 color16)
void MaskBlndResizeViewSurf16(DgSurf *SSrcSurf, int swapHz, int swapVt, int colBlnd); // ColBnd =  color16 | (blend << 24),  blend 0->31 (31 color16)
void TransResizeViewSurf16(DgSurf *SSrcSurf, int swapHz, int swapVt, int transparency); // transparency 0->31 (31 completely opaq)
void MaskTransResizeViewSurf16(DgSurf *SSrcSurf, int swapHz, int swapVt, int transparency); // Mask pixels with value Mask, transparency 0->31 (31 completely opaq)

#ifdef __cplusplus
           }
#endif

#endif //#ifndef DGRAPH_H
