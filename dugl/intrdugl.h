// ----------------------
//	Standard VESA 2.0
#define VesaLFB		0x4000
#define	Preserve	0x8000

#define	SuppMode	1
//#define	Col		8
#define	graph		16
#define	SuppLFB		128

#define FLAG_SUPP	1
#define FLAG_TTY 	4
#define FLAG_COLOR	8
#define FLAG_GRAPH	16
#define FLAG_NVGA	32
#define FLAG_NWVGA	64
#define FLAG_LFB	128


typedef struct
{	int 	Sign		  __attribute__ ((packed));
	unsigned char LoVers	  __attribute__ ((packed));
	unsigned char HiVers	  __attribute__ ((packed));
	int     ConstucteurPtr	  __attribute__ ((packed));
	int 	Capabilities	  __attribute__ ((packed));
	unsigned long VideoPtr 	  __attribute__ ((packed));
	short	Memory		  __attribute__ ((packed));
/*2.0*/	short 	OemSoftwareRev	  __attribute__ ((packed));
	int	OemVendorNamePtr  __attribute__ ((packed));
	int	OemProductNamePtr __attribute__ ((packed));
	int	OemProductRevPtr  __attribute__ ((packed));
    	char	resv[222]	  __attribute__ ((packed));
	int	OemData[256];
} VesaIntro;

typedef struct
{	short	ModeFlag	    __attribute__ ((packed));
	char 	WinFlag1	    __attribute__ ((packed));
	char	WinFlag2	    __attribute__ ((packed));
	short  	Granul		    __attribute__ ((packed));
	short  	WinSize 	    __attribute__ ((packed));
	short  	SegWin1 	    __attribute__ ((packed));
	short  	SegWin2 	    __attribute__ ((packed));
	int 	FuncPtr 	    __attribute__ ((packed));
	short  	Scan		    __attribute__ ((packed));
/*1.2*/	short  	ResX		    __attribute__ ((packed));
	short  	ResY	 	    __attribute__ ((packed));
	char 	MatriceX	    __attribute__ ((packed));
	char	MatriceY	    __attribute__ ((packed));
	char	NbrePlan	    __attribute__ ((packed));
	char	BitPixel	    __attribute__ ((packed));
	char	NbreBlockMem	    __attribute__ ((packed));
	char	ModeleMem	    __attribute__ ((packed));
	char	SizeBlock	    __attribute__ ((packed));
	char	NbreImg 	    __attribute__ ((packed));
    	char	resv		    __attribute__ ((packed));
	char	RedMaxSize	    __attribute__ ((packed));
	char	RedFieldPosition    __attribute__ ((packed));
	char	GreenMaxSize	    __attribute__ ((packed));
	char	GreenFieldPosition  __attribute__ ((packed));
	char	BlueMaxSize	    __attribute__ ((packed));
	char	BlueFieldPosition   __attribute__ ((packed));
	char	RsvdMaxSize	    __attribute__ ((packed));
	char	RsvdFieldPosition   __attribute__ ((packed));
	char	DirectColorModeInfo __attribute__ ((packed));
/*2.0*/	int	PhysBasePtr	    __attribute__ ((packed));
	int	OffScreenOfs	    __attribute__ ((packed));
	short 	OffScreenSize	    __attribute__ ((packed));
    	char	rev2[206];
} VesaInfo;

// from graph.asm
extern int TexXDeb[2048];
extern int TexXFin[2048],TexYDeb[2048],TexYFin[2048];
extern int PColDeb[2048],PColFin[2048];
extern int TPolyAdDeb[2048],TPolyAdFin[2048];
// transf.asm
extern int TrImgResHz,TrImgResVt;
extern void *TrBuffImgSrc, *TrBuffImgDst, *TrBuffImgSrcPal;

#ifdef __cplusplus
   extern "C" {
#endif

 void TransfB8ToB16();
 void TransfB8ToB16Pal();
 void TransfB8ToB15();
 void TransfB16ToB8();

#ifdef __cplusplus
              }
#endif


//************FORMAT ** PCX**************************
typedef struct
{	char 	Sign		__attribute__ ((packed));
 	char	Ver		__attribute__ ((packed));
	char	Comp		__attribute__ ((packed));
	char	BitPixel	__attribute__ ((packed));
	short	X1		__attribute__ ((packed));
	short	Y1		__attribute__ ((packed));
	short	X2		__attribute__ ((packed));
	short	Y2		__attribute__ ((packed));
	short	ResHzDPI	__attribute__ ((packed));
	short	ResVtDPI	__attribute__ ((packed));
	char	Pal[48]		__attribute__ ((packed));
	char	resv		__attribute__ ((packed));
	char	NbPlan		__attribute__ ((packed));
	short	OctLgImg	__attribute__ ((packed));
	short	TypePal 	__attribute__ ((packed));
	short	ResHz		__attribute__ ((packed));
	short	ResVt		__attribute__ ((packed));
	char	resv2[54];
} HeadPCX;
//************FORMAT ** BMP**************************
typedef struct
{	short 		Sign		__attribute__ ((packed)); // == 'BM'
 	unsigned int	SizeFile	__attribute__ ((packed)); // size in bytes of the file
	short		Reserved0	__attribute__ ((packed)); // 0
	short		Reserved1	__attribute__ ((packed)); // 0
	unsigned int	DataOffset	__attribute__ ((packed));
} HeadBMP;
typedef struct
{	unsigned int	SizeInfo	__attribute__ ((packed)), // size in bytes of the info struct
			ImgWidth	__attribute__ ((packed)),
			ImgHeight	__attribute__ ((packed));
	short 		Planes		__attribute__ ((packed)), // == 1
	 		BitsPixel	__attribute__ ((packed)); // bits per pixel
	unsigned int	Compression	__attribute__ ((packed)), // == 0 no compression
			SizeCompData	__attribute__ ((packed)), // == 0 no compression or the size in byte of the comp data
			PixXPerMeter	__attribute__ ((packed)), // == 0
			PixYPerMeter	__attribute__ ((packed)), // == 0
			NBUsedColors	__attribute__ ((packed)), // == 0
			ImportantColors	__attribute__ ((packed)); // == 0 if all colors important
} InfoBMP;
//************FORMAT ** GIF**************************
typedef struct
{	int  	Sign		__attribute__ ((packed)); // == "GIF8"
 	short	Ver		__attribute__ ((packed)); // == "7a" | "9a"
	short	LargEcran	__attribute__ ((packed));
	short	HautEcran	__attribute__ ((packed));
	char	IndicRes	__attribute__ ((packed));
	char	FondCol 	__attribute__ ((packed));
	char	PAspcRation	__attribute__ ((packed));
} HeadGIF;

typedef struct
{	char	Sign		__attribute__ ((packed)); // == ','
 	short	XPos		__attribute__ ((packed));
	short	YPos		__attribute__ ((packed));
	short	ResHz		__attribute__ ((packed));
	short	ResVt		__attribute__ ((packed));
	char	Indicateur	__attribute__ ((packed));
} DescImgGIF;

typedef struct
{	unsigned char 	SignExt __attribute__ ((packed)); // == '!'
 	char		code	__attribute__ ((packed));
	unsigned char	Size	__attribute__ ((packed));
} ExtBlock;


//***********TIMER ***********************************
/*typedef struct
{	float Freq,TickVal;
	unsigned int DebSynchTime,LastSynchTime,LastPos;
} SynchTime;*/

#define SYNCH_HST_SIZE  32
typedef struct
{       unsigned int TimeHst[SYNCH_HST_SIZE];
	float Freq,  // freq / per sec
              LastPos;
        unsigned int FirstTimeValue,LastTimeValue;
	unsigned int NbNullSynch,LastSynchNull,LastNbNullSynch;
        unsigned int hstNbItems,hstIdxDeb,hstIdxFin;
} SynchTime;



//*************** IPX ***************************
typedef struct
{	unsigned short Checksum           __attribute__ ((packed)),
		       Length		  __attribute__ ((packed));
	unsigned char  TransportControl   __attribute__ ((packed)),
		       Type		  __attribute__ ((packed));
	IPXNetwork     DNetwork		  __attribute__ ((packed));
	IPXNode        DNode		  __attribute__ ((packed));
	IPXSocket      DSocket		  __attribute__ ((packed));
	IPXNetwork     SNetwork		  __attribute__ ((packed));
	IPXNode        SNode		  __attribute__ ((packed));
	IPXSocket      SSocket		  __attribute__ ((packed));
} RmIPXPacket;

typedef struct
{	unsigned short LinkOff		  __attribute__ ((packed)),
		       LinkSeg		  __attribute__ ((packed)),
		       ESROff		  __attribute__ ((packed)),
		       ESRSeg		  __attribute__ ((packed));
	unsigned char  InUse		  __attribute__ ((packed)),
		       CompletitionCode	  __attribute__ ((packed));
	IPXSocket      Socket		  __attribute__ ((packed));
	unsigned char  WorkSpace[16]	  __attribute__ ((packed));
	IPXNode	       ImmediateAddress	  __attribute__ ((packed));
	unsigned short FragmentCount	  __attribute__ ((packed)),
		       AddressOff	  __attribute__ ((packed)),
		       AddressSeg	  __attribute__ ((packed)),
		       Size		  __attribute__ ((packed));
	RmIPXPacket    RmPacket 	  __attribute__ ((packed));
} RmIPXECB;

#ifdef __cplusplus
extern "C" {
#endif

void ReverseBuffBytes(void *BuffPtr, unsigned int BuffSize);

#ifdef __cplusplus
           }
#endif

// DWorker ------------------------------------------------

#define DWORKERS_DEFAULT_MAX_COUNT  128

#ifdef __cplusplus
extern "C" {
#endif

bool InitDWorkers(unsigned int MAX_DWorker);
void DestroyDWorkers();

#ifdef __cplusplus
           }
#endif

typedef struct
{   int         	Sign;       // = "DMTX"
    pthread_mutex_t mutex;
} DMutex;

// utils

#define DMAX(a,b) ((a) > (b) ? a : b)
#define DMIN(a,b) ((a) < (b) ? a : b)

// DgSurf
#define MIN_DGSURF_WIDTH    1
#define MIN_DGSURF_HEIGHT   1




