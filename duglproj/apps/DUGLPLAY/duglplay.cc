/*  DUGL Dos Ultimate Game Library - DUGL Player */
/*  Video Player with GUI */
/*  History : */
/*  18 April 2007 : first release
    september 2007 : small bug fixe with FullScreen
    21 february 2008 : Update to use DUGL 1.10alfa1 and DUGL-Plus 0.1
    9 march 2008 : version 0.2
    Added Playing on 16bpp 640x480 graphic mode
    Hacked "badly" to use the 8bpp GUI in 16bpp mode
    Many bug fixes
    Reworked the rendering loop
    Added VSynch, smooth display, fps, and time
    Modified screenshot to make a jpeg file
    19 march 2008 : version 0.21
    Much faster startup with a faster lookup table building
    Better graphic cards compatiblity thanks to DUGL 1.11
    Thank you DOS386 for reporting the bug on old ATI graphic card :)
    xx march 2009 :
    Updated to full 16bpp GUI, added slider, image buttons ....
    11 agust 2009 :
    Updated with DUGL 1.15 - using the faster SurfCopy...
    17 october 2009 ver 0.4 alpha1
    Add support for theora/ogg video format
    removed completely mpeg1/2 video support
    Image buttons to play, stop/continu, exit
    Added playing progress slider
    Added loop option
    BMP screenshot instead of JPG
    27 october 2009 ver 0.4 alpha2
    Added frame dropping
    Added UV interpolation for pixel format 420 and 422
    Fixed two crashs with non ogg file, and with ogg file without theora stream
    Added multi-files screenshot from DUGLPLYR.BMP to DUGLPLYZ.BMP (9 max)
    12 february 2010 ver 0.4 alpha3
    Added a much faster YUV2RGB16 assembler MMX routine
    Added a keyboard shortcut F10 to disable/enable frame dropping
    Turned frame dropping off at start-up
    Many small source code cleaning
    Fixed Theora offset display bug
    Fixed bug writing twice screenshot
    20 february 2010 ver 0.4 alpha4
    Faster YUV2RGB16 assembler MMX routine
    Added 422YUV2RGB16 assembler MMX routine
    Added Frame dropping option on menu
    Optimized all frame decoding function 444, 422 and 420
    Removed the mouse requirement
    Fixed keyboard reactivity bug on very low fps
    Changed screenshot keyboard shortcut to Alt+S
    changed Dialog File to filter by default first *.ogv
    Removed Fast YUV2RGB as MMX routines are now faster
    Added Interpolate UV option with F2 shortcut, disable at start-up
    Added documentation DUGLPLAY.TXT contributed by DOS386
    20 march 2011 ver 0.4 alpha5
    Added support of MPEG1/2 DVD/VCD/SVCD thanks to an upgraded Berkley decoder library.
    Added multi-mask support to FileBox.
    Added saving last FileBox mask.
    2011 ver 0.5
    Added support of a config file "duglplay.cfg"
    Increased slightly the size of the video frame (GUI)
    Better gfx for buttons (GUI)
    Added Fit Screen option with F11 keyboard
    27 February 2026:
    Integrate last DJGPP build of FFMPEG  version 5.1.2
    several upgrades according to new DUGL 2.0WIP
    Config: Add [KeyboardMap] to select keyboard layout file
    ..
*/

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <dir.h>
#include <math.h>

#include <dugl.h>
#include <duglplus.h>


// bmpeg includes
#include <bmpeg.h>
// theora includes
#include <theora/theoradec.h>

#ifdef __cplusplus
extern "C" {
#endif

// FFMPEG
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
#include <libavutil/pixfmt.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <zlib.h>

// internal
void ScanYUV2RGB16(void *YSrcPtr, void *USrcPtr, void *VSrcPtr, void *RGB16DstPtr, unsigned int PixelsSize);
void Scan422YUV2RGB16(void *YSrcPtr, void *USrcPtr, void *VSrcPtr, void *RGB16DstPtr, unsigned int PixelsSize);

#ifdef __cplusplus
           }
#endif

// parameters
bool BlurDisplay=true, SynchScreen=false, DropFrames=false, FitVideo = false,
     FullScrShowTime=true,FullScrShowFps=false, InterpolateUV=false;
bool MouseSupported;
char keybMapFileName[256] = "qwerteng.map";

unsigned int screenX = 640, screenY = 480;
//unsigned int screenX = 320, screenY = 240;
//unsigned int screenX = 1024, screenY = 768;
// full screen frame - quad drawing
float DefMsPosX = 0.5, DefMsPosY = 0.5;

DgSurf *MsPtr,*MsPtr16,*rendSurf16,*blurSurf16;

unsigned char palette[1024];
String CurVidFile;

//***** VIDEO GLOBAL
typedef struct {
  unsigned char *y;
  unsigned char *u;
  unsigned char *v;
  int   y_scan;
  int   u_scan;
  int   v_scan;
  int   width;
  int   height;
} SYUVData;

void YUV2RGB_F420(DgSurf *S, SYUVData *pYUVDATA);
void YUV2RGB_F422(DgSurf *S, SYUVData *pYUVDATA);
void YUV2RGB_F444(DgSurf *S, SYUVData *pYUVDATA);

#define BUFFER_SIZE 1024*8
FILE * vidfile;
size_t size;
bool VidOpen=false,
    VidPause=false,
    VidEnded=false,
    FrameAvlbl=false,
    closeOnVidEnded=false;
float VideoFps = 0.0f;
float VideoTime = 0.0f;
unsigned int sizeVidFile = 0, readVidFileBytes = 0;
int videoWidth = 0, videoHeight = 0;
int videoFramesCount = 0;
int framenum,
    PosSynch,
    frameskipped=0;
int DefTypeOpen=0;

DgSurf *Sframe16; // Surf where the 16bpp video frame will be stored


unsigned char *uFinal = NULL;
unsigned char *vFinal = NULL;

// mpeg global -----------
SMpegInfo  curMpegInf;

// return 0 if success, code error if failed
int OpenVidMPEG(char *FileName);
// return 1 if new frame found, 0 else
int GetNextFrameMPEG(DgSurf *S16, unsigned int nFramesToDrop);
// close an opened video
void CloseVidMPEG();

// theora globals --------
ogg_sync_state          oy;
ogg_page                og;
ogg_stream_state        vo;
ogg_stream_state        to;
ogg_packet              op;
th_info                 ti;
th_comment              tc;
th_setup_info          *ts = NULL;
th_dec_ctx             *td = NULL;

int     theora_p  = 0;
int     stateflag = 0;
int     theora_processing_headers;

#define OGG_BUFFER_READS_OUT 1000 // max count of reads from an ogg file without getting an ogg page
unsigned int countBufferReads = 0;

int ogg_buffer_data(FILE *fIn, ogg_sync_state *oy);
int ogg_queue_page(ogg_page *page);
void OGG_YUV2RGB_F420(DgSurf *S, th_ycbcr_buffer &ycbcr);
void OGG_YUV2RGB_F422(DgSurf *S, th_ycbcr_buffer &ycbcr);
void OGG_YUV2RGB_F444(DgSurf *S, th_ycbcr_buffer &ycbcr);
// return 0 if success, code error if failed
int OpenVidOGG(char *FileName);
// return 1 if new frame found, 0 else
int GetNextFrameOGG(DgSurf *S16, unsigned int nFramesToDrop);
// close an opened video
void CloseVidOGG();

// YUV4MPEG global -----------
FILE *FInYUV4MPEG;
int nWithYUV4MPEG, nHeightYUV4MPEG;
char BuffReadHeadYUV4MPEG[1024];
unsigned char *BuffReadDATAYUV4MPEG;
unsigned int nColorSpaceYUV4MPEG;
unsigned int nSizeDATAYUV4MPEG;
unsigned char *BuffY, *BuffU, *BuffV;
SYUVData  yuvDataYUV4MPEG;

// return 0 if success, code error if failed
int OpenVidYUV4MPEG(char *FileName);
// return 1 if new frame found, 0 else
int GetNextFrameYUV4MPEG(DgSurf *S16, unsigned int nFramesToDrop);
// close an opened video
void CloseVidYUV4MPEG();

// -----------------------
// FFMPEG GLOBAL
AVFormatContext* pFormatCtx = NULL;
// FFmpeg codec context.
AVCodecContext* pVideoCodecCtx = NULL;
// FFmpeg codec for video.
const AVCodec* pVideoCodec = NULL;
// FFmpeg parser codec for video.
AVCodecParserContext *pVideoCodecParser = NULL;
// FFmpeg context convert image.
struct SwsContext *pImgConvertCtx = NULL;
// FFmpeg video stream
AVStream *video_stream = NULL;
// Video stream number in file
int videoStreamIndex = -1;
static enum AVPixelFormat pix_fmt;

static uint8_t *video_dst_data[4] = {NULL};
static int      video_dst_linesize[4];
static int      video_dst_bufsize;
static AVFrame  *videoFrame = NULL;
static AVPacket *pkt = NULL;
static bool videoSwapUpDown = false; // rotate 180
int FFZone = 0, FFFail = 0;


// return 0 if success, code error if failed
int OpenVidFFMPEG(char *FileName);
// return 1 if new frame found, 0 else
int GetNextFrameFFMPEG(DgSurf *S16, unsigned int nFramesToDrop);
// close an opened video
void CloseVidFFMPEG();

//******************
// FONT
DFONT F1;
// mouse View
DgView MsV;
// keyborad map
KbMAP *KM;
unsigned char keyCode;
unsigned int keyFLAG;

// GUI or full screen
int EnableGUI = 0;
// synch buffer
char SynchBuff[SIZE_SYNCH_BUFF];
// GUI *************************************
// Windows Handler
WinHandler *WH;
// Main window -------------------------------
String sMainWinName("DUGL Video Player 1.0 alpha 1");
MainWin *MWDPlayer;
GraphBox *GphBVideo;
Menu *MWMn;
ImgButton *BtPlay,*BtPauseCont,*BtExit;
Label *LbTime;
HzSlider *HSldAdv;
CocheBox *CBxLoop;
DgSurf *ImgPlay,*ImgExit,*ImgPCont;

// glabal var
int redrawVid=1;
char playTime[16];
// events
void OnMenuOpenVid(),OnMenuCloseVid(),OnMenuExit(),OnMenuFullScr(),
     OnMenuPauseCont(),OnMenuAbout();
void OnMenuVSynch(),OnMenuSmoothFS(),OnMenuFSFps(),OnMenuFSTime();
void OnMenuLoop(),OnMenuFrameDrop();
void OnChBLoopChanged(char),OnMenuInterpolateUV(), OnMenuFitScreen();

void GphBDrawVideo(GraphBox *Me),GphBScanVideo(GraphBox *Me);
void OnBtPlayClick();

// screen shot file name
char *scrFileName="DUGLPLYR.BMP";
// file filer string
char *TSFBName[]={ "All supported Videos",
     "MPEG1/2/4", "Theora/Ogg/Ogv", "YUV4MPEG", "All Files(*.*)" };
char *TSFBMask[]={ "*.mpg|*.mpeg|*.m2v|*.m1v|*.mpe|*.mpv|*.dat|*.ogv|*.ogg|*.oga|*.y4m|*.mp4|*.avi|*.mov|*.wmv|*.flv|*.webm|*.3gp|*.vob",
     "*.mpg|*.mpeg|*.m2v|*.m1v|*.mpe|*.mpv|*.dat|*.mp4|*.avi|*.mov|*.wmv", "*.ogv|*.ogg|*.oga", "*.y4m", "*.*" };
ListString LSMpgName(5,TSFBName),LSMpgMask(5,TSFBMask);

// Main Menu -------
NodeMenu TNM[]= {
  { "",	                        4,  &TNM[1], 1, NULL } ,
  { "File",                     3,  &TNM[5], 1, NULL } ,
  { "Play",                     3,  &TNM[8], 1, NULL } ,
  { "Options",                  8,  &TNM[12], 1, NULL } ,
  { "?",                        1,  &TNM[11], 1, NULL } ,
  { "Open        F3",           0,     NULL, 1, OnMenuOpenVid } ,
  { "Close       F4",           0,     NULL, 0, OnMenuCloseVid } ,
  { "Exit     Alt+X",           0,     NULL, 1, OnMenuExit } ,
  { "Play               Alt+P", 0,     NULL, 1, OnBtPlayClick } ,
  { "Full screen  space+enter", 0,     NULL, 1, OnMenuFullScr } ,
  { "Pause/Continue space+tab", 0,     NULL, 1, OnMenuPauseCont } ,
  { "About",                    0,     NULL, 1, OnMenuAbout },
  { "Fit Screen          F11",  0,     NULL, 1, OnMenuFitScreen },
  { "Frame dropping      F10",  0,     NULL, 1, OnMenuFrameDrop },
  { "Loop                F9",   0,     NULL, 1, OnMenuLoop },
  { "Vertical Synch      F8",   0,     NULL, 1, OnMenuVSynch },
  { "Smooth Full screen  F5",   0,     NULL, 1, OnMenuSmoothFS },
  { "Fps Full screen     F6",   0,     NULL, 1, OnMenuFSFps },
  { "Time Full screen    F7",   0,     NULL, 1, OnMenuFSTime },
  { "Interpolate UV      F2",   0,     NULL, 1, OnMenuInterpolateUV }
};

// About window -------------------------------
//CAboutDlg *DlgAbout;
MainWin *MWAbout;
GraphBox *GphBAbout;
Button *BtOkAbout;
// events
void BtOkAboutClick(),GphBDrawAbout(GraphBox *Me),OnGphBScanAbout(GraphBox *Me);
// data
int  RotPosSynch;
char RotSynchBuff[SIZE_SYNCH_BUFF];
//******* Global function ****************************
// return 0 if success, code error if failed
int OpenVid(char *FileName);
// return 1 if new frame found, 0 else
int GetNextFrame(DgSurf *S16, unsigned int nFramesToDrop);
// close an opened video
void CloseVid();
// utils
void DGWaitRetrace();
void UpdatePlayTime();
bool IsFileExist(const char *fname);
void LoadConfig();

int main (int argc, char ** argv) {
    if (!DgInit()) {
        printf("DUGL init error\n");
        exit(-1);
    }

    LoadConfig();

    if (!InstallKeyboard()) {
       DgQuit(); DgUninstallTimer();
       printf("Keyboard error\n");  exit(-1);
    }

    if (CreateSurf(&rendSurf16, screenX, screenY, 16)==0) {
      printf("no mem\n"); exit(-1);
    }
    if (CreateSurf(&blurSurf16, screenX, screenY, 16)==0) {
      printf("no mem\n"); exit(-1);
    }
    if (!LoadGIF(&MsPtr,"gfx/mouseimg.gif",&palette))
      { printf("Error loading mouseimg.gif\n"); exit(-1); }
    if (CreateSurf(&MsPtr16, MsPtr->ResH, MsPtr->ResV, 16)==0) {
      printf("no mem\n"); exit(-1);
    }

    if (LoadBMP16(&ImgPlay,"gfx/play.bmp")==0) {
      printf("Error loading play.bmp\n"); exit(-1);
    }
    if (LoadBMP16(&ImgPCont,"gfx/pcont.bmp")==0) {
      printf("Error loading pcont.bmp\n"); exit(-1);
    }
    if (LoadBMP16(&ImgExit,"gfx/shut.bmp")==0) {
      printf("Error loading shut.bmp\n"); exit(-1);
    }

    if (!LoadKbMAP(&KM,keybMapFileName)) {
      printf("Error loading keyboard map '%s'\n", keybMapFileName); exit(-1); }

    // load font
    if (!LoadDFONT(&F1,"helloc.chr")) {
      printf("Error loading helloc.chr\n"); exit(-1); }

    // init the lib

    if (!DgInstallTimer(250)) {
       DgQuit(); printf("Timer error\n"); exit(-1);
    }
    if (!SetKbMAP(KM)) {
       DgUninstallTimer(); UninstallKeyboard(); DgQuit();
       printf("Error setting keyborad map\n");  exit(-1);
    }
    MouseSupported = (InstallMouse()!=0);

    // init video mode
    if (!InitVesaMode(screenX,screenY,16,1)) {
       if(screenX!=640 || screenY!=480) {
         screenX=640; screenY=480;
         if (!InitVesaMode(screenX,screenY,16,1)) {
            printf("VESA mode error\n"); DgQuit(); exit(-1);
         }
       }
    }
    if (argc>=2) {
      if (OpenVid(argv[1])!=0) {
        printf ("error opening video file: \"%s\".\n", argv[1]);
        exit(-1);
      }
      else
        TNM[6].Activ = 1; // enable menu close
    }
    else
      EnableGUI = 1; // enable GUI if no param

    DgSetCurSurf(&VSurf[0]);
    Clear16(0); // clear by black

    // set font
    SetDFONT(&F1);
    // mouse
    if (MouseSupported) {
       // set mouse pointer Orig to the upper left corner
       ConvSurf8ToSurf16Pal(MsPtr16,MsPtr,&palette);
       SetOrgSurf(MsPtr16,0,MsPtr16->ResV-1);
       // set mouse view
       GetSurfView(&VSurf[0],&MsV);
       SetMouseView(&MsV);
       SetMousePos(DefMsPosX*VSurf[0].ResH,DefMsPosY*VSurf[0].ResV);
    }
    else {
       DestroySurf(MsPtr16);
       DestroySurf(MsPtr);
    }

    //** GUI ************************************************
    // create the winHandler
    WH = new WinHandler(screenX,screenY,16,0xF|(0x1F<<5));
    //---- Main Window
    MWDPlayer= new MainWin(0,0,screenX,screenY,sMainWinName.StrPtr,WH);
    GphBVideo= new GraphBox(2,30,screenX-10,screenY-50,MWDPlayer,WH->m_GraphCtxt->WinGris);
    // set drawing handler
    GphBVideo->GraphBoxDraw=GphBDrawVideo;
    // set scan handler (enable redraw when needed)
    GphBVideo->ScanGraphBox=GphBScanVideo;
    GphBVideo->Redraw();
    // buttons
    BtPlay=new ImgButton(2,3,29,27,MWDPlayer,ImgPlay);
    BtPlay->Click=OnBtPlayClick; // set click handler
    BtPauseCont=new ImgButton(34,3,59,27,MWDPlayer,ImgPCont);
    BtPauseCont->Click=OnMenuPauseCont; // set click handler
    CBxLoop=new CocheBox(63,5,118,25,MWDPlayer,NULL,"loop",0);
    CBxLoop->Changed=OnChBLoopChanged;
    HSldAdv=new HzSlider(120,screenX-123,7,MWDPlayer,0,100);
    LbTime=new Label(screenX-118,5,screenX-43,25,MWDPlayer,"00:00:00",AJ_LEFT);

    BtExit=new ImgButton(screenX-36,3,screenX-10,27,MWDPlayer,ImgExit);
    BtExit->Click=OnMenuExit; // set click handler
    // menu
    MWMn = new Menu(MWDPlayer,&TNM[0]); // menu
    // ---- About window
    MWAbout= new MainWin(screenX/2-200,screenY/2-130,400,270,"About",WH);
    GphBAbout= new GraphBox(5,30,390,240,MWAbout,WH->m_GraphCtxt->WinGris);
    GphBAbout->GraphBoxDraw=GphBDrawAbout; GphBAbout->Redraw();
    GphBAbout->ScanGraphBox=OnGphBScanAbout;
    BtOkAbout=new Button(115,3,275,25,MWAbout,"Ok",1,0);
    BtOkAbout->Click=BtOkAboutClick;
    //*******************************************************

    // init synch for synching the screen and the opened video
    PosSynch=0;
    InitSynch(SynchBuff,&PosSynch,VideoFps);
    UpdatePlayTime();
    // main loop
    for (int j=0;;j++) {
      // synchronise
      Synch(SynchBuff,&PosSynch);
      // synch screen display
      float avgFps=SynchAverageTime(SynchBuff),
            lastFps=SynchLastTime(SynchBuff);
      //if (lastFps <= 0.1f)
      //  __dpmi_yield();


      DgSetCurSurf(rendSurf16);
      // get next frame if it's time
      if (VidOpen) {
        if (VidPause==1 && PosSynch!=framenum) {
          frameskipped+=PosSynch-framenum;
          framenum=PosSynch;
        }
        if (VidPause==0) {
          if (PosSynch!=framenum) {
             if (DropFrames) {
                if (GetNextFrame(Sframe16,PosSynch-framenum-1)) {
                   redrawVid=1;
                   if (FrameAvlbl==0) FrameAvlbl=1;
                }
                else {
                   VidPause=1; // set video as paused
                   // enable GUI if full screen, and if we are not looping
                   if (EnableGUI==0 && (!CBxLoop->True)) {
                      OnMenuFullScr();
                   }
                }
             } else {
                if (GetNextFrame(Sframe16,0)) {
                   redrawVid=1;
                   if (FrameAvlbl==0) FrameAvlbl=1;
                   frameskipped+=PosSynch-framenum-1; // we are too slow ? :(
                } else {
                   VidPause=1; // set video as paused
                   // enable GUI if full screen, and if we are not looping
                   if (EnableGUI==0 && (!CBxLoop->True)) {
                      OnMenuFullScr();
                   }
                }
             }

             framenum=PosSynch;
             UpdatePlayTime();
          }
        }
      }

      // loop ?
      if (VidEnded) {
        if (VidOpen) {
            if(CBxLoop->True) {
                String saveInputFile = CurVidFile;
                CloseVid();
                CurVidFile = saveInputFile;
                OnBtPlayClick();
            } else if (!closeOnVidEnded) {
                if (!VidPause) {
                    VidPause = true;
                    UpdatePlayTime();
                }
            }
            else
              CloseVid();
        }
      }

      // GUI
      if (EnableGUI) {
        // force playing also if menu are active
        if (VidOpen && (!VidEnded) &&
            (MWDPlayer->ActivMenu==1 || MWDPlayer->Focus==0)) {
           redrawVid=1;
           GphBVideo->Redraw();
        }
        // scan the GUI for events
        WH->Scan();
        // space + enter : toogle full screen or back to gui if space+enter
        if ((WH->KeyFLAG&KB_SPACE_PR) && (WH->KeyFLAG&KB_ENTER_PR))
             OnMenuFullScr();
        // space + tab : pause / continue
        if ((WH->KeyFLAG&KB_SPACE_PR) && (WH->KeyFLAG&KB_TAB_PR))
             OnMenuPauseCont();
        // alt+P : Play
        if (WH->Key==0x19 && (WH->KeyFLAG&KB_ALT_PR))
          OnBtPlayClick();
        if (WH->CurWinNode->Item==MWDPlayer) {
          switch (WH->Key) {
            case KB_KEY_F2 : OnMenuInterpolateUV(); break; // F2
            case KB_KEY_F3 : OnMenuOpenVid(); break; // F3
            case KB_KEY_F4 : OnMenuCloseVid(); break; // F4
            case KB_KEY_F9 : CBxLoop->SetTrue(!CBxLoop->True); break; // F9
            case KB_KEY_F10 : DropFrames = !DropFrames; break; // F10
            case KB_KEY_F11 : FitVideo = !FitVideo; redrawVid=1; break; // F11
            case KB_KEY_F8 : OnMenuVSynch(); break; // F8
            case KB_KEY_F5 : OnMenuSmoothFS(); break; // F5
            case KB_KEY_F6 : OnMenuFSFps(); break; // F6
            case KB_KEY_F7 : OnMenuFSTime(); break; // F7
          }
        }
      }
      else {
        // get key
        GetKey(&keyCode, &keyFLAG);
        // space + enter : toogle full screen or back to gui if space+enter
        if ((keyFLAG&KB_SPACE_PR) && (keyFLAG&KB_ENTER_PR))
             OnMenuFullScr();
        // space + tab : pause / continue
        if ((keyFLAG&KB_SPACE_PR) && (keyFLAG&KB_TAB_PR))
             OnMenuPauseCont();
        if (keyCode==0x19 &&  (keyFLAG&KB_ALT_PR))
          OnBtPlayClick();

        if (keyCode==KB_KEY_F9) CBxLoop->SetTrue(!CBxLoop->True);
        if (keyCode==KB_KEY_F11) { FitVideo = !FitVideo; redrawVid=1; }// F11
        if (keyCode==KB_KEY_F10) DropFrames = !DropFrames; // F10
        if (keyCode==KB_KEY_F8) OnMenuVSynch();
        if (keyCode==KB_KEY_F5) OnMenuSmoothFS();
        if (keyCode==KB_KEY_F6) OnMenuFSFps();
        if (keyCode==KB_KEY_F7) OnMenuFSTime();
        if (keyCode==KB_KEY_F2) OnMenuInterpolateUV(); // F2

        // FULL screen
        if (VidOpen)
        {

          if(FitVideo)
          {
            ResizeViewSurf16(Sframe16, 0, videoSwapUpDown);
          }
          else {
            Clear16(0); // clear by black
//            //PutSurf16(Sframe16, (CurSurf.MaxX+CurSurf.MinX-Sframe16->ResH)/2,
            PutSurf16(Sframe16, (CurSurf.MaxX+CurSurf.MinX-Sframe16->ResH)/2,
                (CurSurf.MaxY+CurSurf.MinY-Sframe16->ResV)/2, (videoSwapUpDown) ? INV_VT_PUT : 0);
          }
        }
      }

      // display
      if (EnableGUI) { // GUI
        keyCode = WH->Key;
        keyFLAG = WH->KeyFLAG;
        DgSetCurSurf(rendSurf16);
        // draw the GUI
        WH->DrawSurf(&CurSurf);
        // draw the mouse pointer
        if(MouseSupported)
           PutMaskSurf16(MsPtr16,MsX,MsY,0);
        // draw the GUI
        //DGWaitRetrace();
        SurfCopy(&VSurf[0], rendSurf16);
      } else { // full screen
         if (BlurDisplay) {
            BlurSurf16(blurSurf16,rendSurf16);
            DgSetCurSurf(blurSurf16);
         }
         else
           DgSetCurSurf(rendSurf16);
         // display AVG FPS

         int Xtext,Ytext,widthText;
         if (FullScrShowFps) {
           ClearText();
           char text[100];
           SetTextCol(0xffff);
           sprintf(text,"%03i fps",(int)(1.0/avgFps));
           Xtext=GetXOutTextMode(text,AJ_RIGHT);
           Ytext=FntY+FntLowPos-1;
           widthText=WidthText(text);
           barblnd16(Xtext,Ytext,Xtext+widthText,Ytext+FntHaut,0|(5<<24));

           OutText16Mode(text,AJ_RIGHT);
         }
         if (FullScrShowTime) {
           ClearText();
           SetTextCol(0xffff);
           Xtext=GetXOutTextMode(playTime,AJ_LEFT);
           Ytext=FntY+FntLowPos-1;
           widthText=WidthText(playTime);
           barblnd16(Xtext,Ytext,Xtext+widthText,Ytext+FntHaut,RGB16(0,0,255)|(5<<24));
           OutText16Mode(playTime,AJ_LEFT);
         }
         DGWaitRetrace();
         if (BlurDisplay)
           SurfCopy(&VSurf[0], blurSurf16);
         else
           SurfCopy(&VSurf[0], rendSurf16);
      }

      // alt+ X : exit
      if (keyCode==KB_KEY_QWERTY_X && /* 'X'|'x' */ (keyFLAG&KB_ALT_PR))
         OnMenuExit();
      // Alt + S  = bmp screen shot
      if (keyCode == KB_KEY_QWERTY_S && (keyFLAG&KB_ALT_PR)) {
         bool bSucc = false;
         for (unsigned int ci='R';ci<='Z';ci++) {
            scrFileName[7]=(char)(ci);
            if (!IsFileExist(scrFileName)) {
               SaveBMP16(&VSurf[0],scrFileName);
               bSucc = true;
               break;
            }
         }
         if(!bSucc)
            SaveBMP16(&VSurf[0],scrFileName);
      }
    }


    DgQuit();
    UninstallKeyboard();
    DgUninstallTimer();
    if(MouseSupported)
       UninstallMouse();
    TextMode();

    return 0;
}
// DUGL Util waitRetrace
void DGWaitRetrace() {
  if (!SynchScreen) return;
  if (CurDgfxMode->VModeFlag|VMODE_VGA)
     WaitRetrace(); // VGA wait retrace
  else
     ViewSurfWaitVR(0);
}

bool IsFileExist(const char *fname) {
    struct ffblk f;
    if (findfirst(fname, &f, FA_HIDDEN | FA_SYSTEM)==0)
       return true;
    return false;
}

void UpdatePlayTime() {
  unsigned int iplayTime,videoAdv=0;

  if (VidOpen) {
    iplayTime=(unsigned int)(float(framenum-frameskipped)/VideoFps);
    sprintf(playTime,"%02u:%02u:%02u",(iplayTime/3600),(iplayTime/60)%60,iplayTime%60);
    if (videoFramesCount>0) {
      videoAdv=(unsigned int)((double)(framenum-frameskipped)*100.0 / (double)(videoFramesCount));
    } else if(sizeVidFile>0) {
      videoAdv=(unsigned int)((((double)readVidFileBytes)*100.0) / (double)(sizeVidFile));
    }
    else
      videoAdv=0;
  }
  else {
    sprintf(playTime,"00:00:00");
  }
  HSldAdv->SetVal(videoAdv);
  LbTime->Text=playTime;
}


// Main window event
void FBOpenVid(String *S,int TypeSel) {
  String text;
  int res = 0;
  if ((res=OpenVid(S->StrPtr))!=0) {
    sprintf(text.StrPtr,"Error loading video File %i\n",res);
    MessageBox(WH,"Error!", text.StrPtr,"Ok",NULL,NULL,NULL,NULL,NULL);
  }
  else {
    DefTypeOpen = TypeSel;
    TNM[6].Activ = 1; // enable menu close
  }
}
void OnMenuOpenVid() {
   FilesBox(WH,"Open", "Open", FBOpenVid, "Cancel", NULL, &LSMpgName,
            &LSMpgMask, DefTypeOpen);
}

void OnMenuCloseVid() {
  CloseVid();
  TNM[6].Activ = 0; // disable menu close
  GphBVideo->Redraw();
}

void OnMenuExit() {
  CloseVid();
  DgQuit();
  UninstallKeyboard();
  DgUninstallTimer();
  if(MouseSupported)
    UninstallMouse();
  TextMode();
  exit(0);
}

// full screen or GUI
void OnMenuFullScr() {
  if (VidOpen) {
    EnableGUI=!EnableGUI;
    if (EnableGUI==1)
      redrawVid=1; // enable redraw video GUI
  }
}

// Pause/Continue
void OnMenuPauseCont() {
  VidPause=(VidPause==1)?0:1;
}

void OnMenuAbout() {
    InitSynch(RotSynchBuff,&RotPosSynch,300.0);
    MWAbout->Show(); // show about
    MWAbout->Enable(); // set as the active window
}

void GphBDrawVideo(GraphBox *Me) {

   // opened video ?
   if (VidOpen==1 &&  FrameAvlbl==1 && (!VidEnded)) {
      if(FitVideo)
      {
        ResizeViewSurf16(Sframe16, 0, 0);
      }
      else {
        ClearSurf16(WH->m_GraphCtxt->WinGrisF);
        PutSurf16(Sframe16,
                  (GphBVideo->VGraphBox.MaxX+GphBVideo->VGraphBox.MinX-Sframe16->ResH)/2,
                  (GphBVideo->VGraphBox.MaxY+GphBVideo->VGraphBox.MinY-Sframe16->ResV)/2,
                  0);
      }
      return;
   }
   ClearSurf16(WH->m_GraphCtxt->WinGrisF);
}

void GphBScanVideo(GraphBox *Me) {
   if (redrawVid) {
     // set poly for GUI screen
     if (EnableGUI==1) {
     }
     if (VidOpen) Me->Redraw();
     redrawVid=0;
   }
}
void OnBtPlayClick() {

  OpenVid(CurVidFile.StrPtr);
}


void OnMenuVSynch() {
   SynchScreen=(!SynchScreen);
}

void OnMenuSmoothFS() {
   BlurDisplay=(!BlurDisplay);
}

void OnMenuFSFps() {
   FullScrShowFps=(!FullScrShowFps);
}

void OnMenuFSTime() {
   FullScrShowTime=(!FullScrShowTime);
}

void OnChBLoopChanged(char) {
  CBxLoop->UnsetFocus();
}

void OnMenuFitScreen() {
  FitVideo = !FitVideo; redrawVid=1;
}

void OnMenuInterpolateUV() {
    InterpolateUV=!InterpolateUV;
}

void OnMenuLoop() {
    CBxLoop->SetTrue(!CBxLoop->True);
}

void OnMenuFrameDrop() {
    DropFrames = !DropFrames;
}

// MWAbout events
void BtOkAboutClick() {
  MWAbout->Hide(); // show about
  MWDPlayer->Enable(); // enable main win

}

void OnGphBScanAbout(GraphBox *Me) {
  // synchronise
//  if (Synch(RotSynchBuff,&RotPosSynch)>0)
    Synch(RotSynchBuff,&RotPosSynch);
    Me->Redraw();

}
void GphBDrawAbout(GraphBox *Me) {
   char midText[256] = "";

   ClearSurf16(RGB16(0,0,0));
   ClearText();
   SetTextCol(WH->m_GraphCtxt->WinBlanc);
   OutText16Mode("\n", AJ_MID);
   FntCol=RGB16(0,255,0); // green
   OutText16Mode("DUGL Player 1.0 Alpha 1 - DOS Video Player\n", AJ_MID);
   FntCol=RGB16(255,255,255); // white
   OutText16Mode("(C) By FFK 27 February 2026\n\n", AJ_MID);
   OutText16Mode("Developped using :\n", AJ_MID);
   FntCol=RGB16(255,255,0); // yellow
   OutText16ModeFormat(AJ_MID, midText, 255,"DUGL %s\n",DUGL_VERSION);
   FntCol=RGB16(0,255,255); // white
   OutText16Mode("https://github.com/FFK77/DOS-DUGL\n", AJ_MID);
   FntCol=RGB16(255,255,0); // yellow
   OutText16ModeFormat(AJ_MID, midText, 255, "FFMPEG %s\n", av_version_info());
   FntCol=RGB16(255,255,255); // white
   OutText16Mode("https://ffmpeg.org/\n\n", AJ_MID);

   FntCol=0xFFFF; // white
   sprintf(midText,"VSynch(%s) Smoothing(%s) Interpolate UV(%s)\n",
        SynchScreen?"ON":"OFF",BlurDisplay?"ON":"OFF",
        InterpolateUV?"ON":"OFF" );
   OutText16Mode("\n", AJ_MID);
   OutText16Mode(midText, AJ_MID);
   sprintf(midText,"Frame dropping(%s) Fit Screen(%s)\n",
     DropFrames?"ON":"OFF", FitVideo?"ON":"OFF" );
   OutText16Mode(midText, AJ_MID);

}

void LoadConfig()
{
    char infoName[256]="";
    DFileBuffer *fileBuffer = CreateDFileBuffer(0);

    DSplitString *ListInfoLine = CreateDSplitString(0, 0);
    DSplitString *ListInfoIndex = CreateDSplitString(0, 0);
    if (fileBuffer == NULL || ListInfoLine == NULL || ListInfoIndex == NULL) {
        return;
    }
    if (!OpenFileDFileBuffer(fileBuffer, "DUGLPLAY.CFG", "rt"))
    {
        if (fileBuffer != NULL)
            DestroyDFileBuffer(fileBuffer);
        if (ListInfoLine != NULL)
            DestroyDSplitString(ListInfoLine);
        if (ListInfoIndex != NULL)
            DestroyDSplitString(ListInfoIndex);
        return;
    }
	for(;!IsEndOfFileDFileBuffer(fileBuffer);) {
		if ((ListInfoLine->globLen = GetLineDFileBuffer(fileBuffer, ListInfoLine->globStr, ListInfoLine->maxGlobLength)) == 0 &&
            IsEndOfFileDFileBuffer(fileBuffer)) {
                break;
		}
		TrimGlobStringDSplitString(ListInfoLine);
		// ignore if empty or contain comments only
		if (ListInfoLine->globStr[0] == '\0' || ListInfoLine->globStr[0] == ';')
			continue;
        // remove any middle comment
		if (splitDSplitString(ListInfoLine, NULL, ';', false) > 0) {
		    TrimStringsDSplitString(ListInfoLine);
		    unsigned int lenInfo = strlen(ListInfoLine->ListStrings[0]);
		    // check if it start with '[' and end with ']'
		    if (lenInfo <=2 || ListInfoLine->ListStrings[0][0]!='[' || ListInfoLine->ListStrings[0][lenInfo-1]!=']') {
                break; // failure
		    }
		    strncpy(infoName, &ListInfoLine->ListStrings[0][1], lenInfo-2);
		    infoName[lenInfo-2] = '\0';
		    // search for informations
            if ((ListInfoIndex->globLen = GetLineDFileBuffer(fileBuffer, ListInfoIndex->globStr, ListInfoIndex->maxGlobLength)) == 0 && IsEndOfFileDFileBuffer(fileBuffer)) break;
            TrimGlobStringDSplitString(ListInfoIndex);
            if (ListInfoIndex->globStr[0] == '\0' || ListInfoIndex->globStr[0] == ';')
                break;
            if (splitDSplitString(ListInfoIndex, NULL, ',', false) > 0) {
                TrimStringsDSplitString(ListInfoIndex);

                if(strcmp(infoName,"VideoMode") == 0 && ListInfoIndex->countStrings >= 2) {
                  screenX = atoi(ListInfoIndex->ListStrings[0]);
                  screenY = atoi(ListInfoIndex->ListStrings[1]);
                }
                else if(strcmp(infoName,"KeyboardMap") == 0  && ListInfoIndex->countStrings >= 1) {
                  if (IsFileExist(ListInfoIndex->ListStrings[0])) {
                    strcpy(keybMapFileName, ListInfoIndex->ListStrings[0]);
                  }
                }
                else if(strcmp(infoName,"MousePosition") == 0  && ListInfoIndex->countStrings >= 2) {
                  DefMsPosX = atof(ListInfoIndex->ListStrings[0]);
                  if(DefMsPosX<0.0 || DefMsPosX>1.0) DefMsPosX = 0.5;
                  DefMsPosY = atof(ListInfoIndex->ListStrings[1]);
                  if(DefMsPosY<0.0 || DefMsPosY>1.0) DefMsPosY = 0.5;
                }
                else if(strcmp(infoName,"VerticalSynch") == 0  && ListInfoIndex->countStrings >= 1) {
                  SynchScreen = (bool)(atoi(ListInfoIndex->ListStrings[0]));
                }
                else if(strcmp(infoName,"DropFrames") == 0  && ListInfoIndex->countStrings >= 1) {
                  DropFrames = (bool)(atoi(ListInfoIndex->ListStrings[0]));
                }
                else if(strcmp(infoName,"InterpolateUV") == 0  && ListInfoIndex->countStrings >= 1) {
                  InterpolateUV = (bool)(atoi(ListInfoIndex->ListStrings[0]));
                }
                else if(strcmp(infoName,"FitScreen") == 0  && ListInfoIndex->countStrings >= 1) {
                  FitVideo = (bool)(atoi(ListInfoIndex->ListStrings[0]));
                }
                else if(strcmp(infoName,"FullScrSmooth") == 0  && ListInfoIndex->countStrings >= 1) {
                  BlurDisplay = (bool)(atoi(ListInfoIndex->ListStrings[0]));
                }
                else if(strcmp(infoName,"FullScrShowTime") == 0  && ListInfoIndex->countStrings >= 1) {
                  FullScrShowTime = (bool)(atoi(ListInfoIndex->ListStrings[0]));
                }
                else if(strcmp(infoName,"FullScrShowFps") == 0  && ListInfoIndex->countStrings >= 1) {
                  FullScrShowFps = (bool)(atoi(ListInfoIndex->ListStrings[0]));
                }
            }
		}
	}
	CloseFileDFileBuffer(fileBuffer);
	DestroyDFileBuffer(fileBuffer);
	DestroyDSplitString(ListInfoLine);
	DestroyDSplitString(ListInfoIndex);
}


// MPEG --------------

// return 0 if success, code error if failed
int OpenVidMPEG(char *FileName)
{
   String myFile=FileName;
   CloseVid();
   // open the video file
   vidfile = fopen (myFile.StrPtr, "rb");

    if (vidfile==NULL)
      return 1; // failed to open

   // size file
   fseek(vidfile,0,SEEK_END);
   sizeVidFile = ftell(vidfile);
   readVidFileBytes = 0;
   fclose(vidfile); vidfile = NULL;

   if(!OpenMPEG(myFile.StrPtr, &curMpegInf))
     return 1;

    // compute video fps
    if(curMpegInf.frame_rate!=0.0f)
       VideoFps = curMpegInf.frame_rate;
    else
       VideoFps = 25.0f;

    if (CreateSurf(&Sframe16, curMpegInf.width, curMpegInf.height, 16)==0)
        return 2; // no mem

    uFinal = (unsigned char*) malloc(curMpegInf.width);
    vFinal = (unsigned char*) malloc(curMpegInf.width);

    VidOpen=1; // opened video
    if(GetNextFrameMPEG(Sframe16,0)!=1)
       return 3;  // no frame

    framenum=0; // found one frame
    countBufferReads=0;
    PosSynch=0;
    InitSynch(SynchBuff,&PosSynch,VideoFps);
    VidOpen=1; // opened video
    VidPause=0;
    VidEnded=false;
    frameskipped=0;
    FrameAvlbl=0;
    CurVidFile=myFile; // save current file
    TNM[6].Activ = 1; // enable menu close
    return 0; // success
}
// return 1 if new frame found, 0 else
int GetNextFrameMPEG(DgSurf *S16, unsigned int nFramesToDrop)
{   SYUVData  yuvData;

    if (DecodeNextMpegFrame(nFramesToDrop)) {
          yuvData.y= lastframe[0];
          yuvData.u= lastframe[1];
          yuvData.v= lastframe[2];

        yuvData.y_scan= yuvData.width= curMpegInf.width;
        yuvData.u_scan= yuvData.v_scan= curMpegInf.width/2;
        yuvData.height= curMpegInf.height;
        switch(curMpegInf.chroma_format) {
           case CHROMA_FORMAT_420 :
              YUV2RGB_F420(S16, &yuvData);
              break;
           case CHROMA_FORMAT_422 :
              YUV2RGB_F422(S16, &yuvData);
              break;
           case CHROMA_FORMAT_444 :
              yuvData.u_scan= curMpegInf.width;
              yuvData.v_scan= curMpegInf.width;
              YUV2RGB_F444(S16, &yuvData);
              break;
        }

        readVidFileBytes = GetCurMPEGFilePosition();
        return 1;
    }
    VidEnded=true; // we reached the end
    return 0;

}
// close an opened video
void CloseVidMPEG()
{
   if (VidOpen) {
     CloseMPEG();
     DestroySurf(Sframe16);
     if(uFinal) { free(uFinal); uFinal = NULL; }
     if(vFinal) { free(vFinal); vFinal = NULL; }

     sizeVidFile = 0;
     readVidFileBytes = 0;
     VidOpen=false;
     VidEnded=false;
     TNM[6].Activ = 0; // disable menu close
     framenum=0;
     frameskipped=0;
     countBufferReads=0;
     HSldAdv->SetVal(0);
     CurVidFile="";
   }
}

// Ogg --------------------------------------------
// ------------------------------------------------
int OpenVidOGG(char *FileName) {
    String myFile=FileName;
    int failed = 0;
    int OggStreamID = 0; // should be always equal 'OggS' ?
    // save file name
    // close if any opened
    CloseVid();

    // open the video file
    vidfile = fopen (myFile.StrPtr, "rb");

    if (vidfile==NULL)
      return 1; // failed to open

    fread(&OggStreamID, 1, sizeof(int), vidfile);
    if (OggStreamID != 'SggO')
      return 2; // corrupt file not ogg

    // size file
    fseek(vidfile,0,SEEK_END);
    sizeVidFile = ftell(vidfile);
    readVidFileBytes = 0;
    fseek(vidfile,0,SEEK_SET);

    // ogg init -----------
    ogg_sync_init(&oy);

    th_comment_init(&tc);
    th_info_init(&ti);
    // --------------------
    // only interested in theora streams
    stateflag = 0;
    while (!stateflag) {
        int ret = ogg_buffer_data(vidfile, &oy);
        if (ret == 0) break;
        while (ogg_sync_pageout(&oy, &og)) {
            int got_packet;
            ogg_stream_state test;
            if (!ogg_page_bos(&og)) {
                ogg_queue_page(&og);
                stateflag = 1;
                break;
            }
            ogg_stream_init(&test, ogg_page_serialno(&og));
            ogg_stream_pagein(&test, &og);
            got_packet = ogg_stream_packetpeek(&test, &op);
            if (got_packet==1 && (!theora_p) && (theora_processing_headers=
                th_decode_headerin(&ti, &tc, &ts, &op)) >= 0) {
                // it's theora -> save this stream state
                memcpy(&to, &test, sizeof(test));
                theora_p = 1;
                if (theora_processing_headers)
                    ogg_stream_packetout(&to, NULL);
            } else {
                ogg_stream_clear(&test);
            }

        }
    }
    // we're expecting more headers
    while (theora_p && theora_processing_headers) {
        int ret;
        while (theora_processing_headers && (ret=ogg_stream_packetpeek(&to,&op))) {
            if (ret<0) continue;
            theora_processing_headers = th_decode_headerin(&ti, &tc, &ts, &op);
            if (theora_processing_headers < 0) {
                return 2; // corrupt file
            }
            else if (theora_processing_headers>0) {
              // advance past the successfully processed header
              ogg_stream_packetout(&to, NULL);
            }
            theora_p++;
        }
        // stop new so we don't fail if there aren't enough pages in a short stream
        if (!(theora_p && theora_processing_headers)) break;
        // the header pages/packet come first or it's an invalid stream
        if (ogg_sync_pageout(&oy, &og) > 0) {
            ogg_queue_page(&og); // demux into the appropriate stream
        }
        else {
            int ret = ogg_buffer_data(vidfile, &oy); // get more data
            if (ret==0) {
                return 3; // reached end of file
            }
        }
    }
    // succeded !! initialize decoders
    if (theora_p) {
        //dump_comments(&tc);
        td = th_decode_alloc(&ti, ts);
    } else {
        // tear down partial theora setup
        th_info_clear(&ti);
        th_comment_clear(&tc);
    }

    th_setup_free(ts);
    ts=NULL;
    // queue any ramaining pages from data we buffered
    while(ogg_sync_pageout(&oy,&og)>0) {
        ogg_queue_page(&og);
    }
    // compute video fps
    if (ti.fps_denominator>0)
      VideoFps = (float)(ti.fps_numerator)/(float)(ti.fps_denominator);
    else
      VideoFps = 25;

    if (ti.pixel_fmt>=4 || ti.pixel_fmt==TH_PF_RSVD)
        failed=4; // invalid pixel format

    if (failed==0 && CreateSurf(&Sframe16, ti.frame_width, ti.frame_height, 16)==0)
        failed=5; // no mem

    uFinal = (unsigned char*) malloc(ti.frame_width);
    vFinal = (unsigned char*) malloc(ti.frame_width);

    VidOpen=1; // opened video
    if(failed==0 && GetNextFrameOGG(Sframe16,0)!=1)
        failed=6; // no frame

    if (failed>0) {
        if (theora_p) {
            ogg_stream_clear(&to);
            th_decode_free(td);
            th_comment_clear(&tc);
            th_info_clear(&ti);
            theora_p=0;
         }
        ogg_sync_clear(&oy);
        if (failed==6) DestroySurf(Sframe16);
        VidOpen=0; // opened video
        return failed;
    }

    framenum=0; // found one frame
    countBufferReads=0;
    PosSynch=0;
    InitSynch(SynchBuff,&PosSynch,VideoFps);
    VidOpen=1; // opened video
    VidPause=0;
    VidEnded=false;
    frameskipped=0;
    FrameAvlbl=0;
    CurVidFile=myFile; // save current file
    TNM[6].Activ = 1; // enable menu close
    return 0; // success
}

int GetNextFrameOGG(DgSurf *S16,unsigned int nFramesToDrop) {
    String text;
    DgSurf OldSurf;
    th_ycbcr_buffer ycbcr;
    int idx,iw;
    int th_decode_res       = -1;
    ogg_int64_t video_granulpos= -1;
    unsigned int nDrops     = nFramesToDrop;
    unsigned char *yFrm     = NULL;
    unsigned char *uFrm     = NULL;
    unsigned char *vFrm     = NULL;
    unsigned short *scanImg = NULL;
    bool bVideoBuffReady    = false;
    unsigned char UVxShift  = 0;
    unsigned char UVyShift  = 0;

    if (!VidOpen) return 0;
    for (;theora_p;) {
        while (theora_p && !bVideoBuffReady) {
            if (ogg_stream_packetout(&to, &op)>0) {
                if ((th_decode_res=th_decode_packetin(td,&op,&video_granulpos))>=0) {
                    if (nDrops==0)
                      bVideoBuffReady = true;
                    else
                      nDrops--;
                }
            }
            else
                break;
        }
        if (bVideoBuffReady) break;
        if (ogg_buffer_data(vidfile, &oy)==0) return 0;
        while(ogg_sync_pageout(&oy,&og)>0) {
            ogg_queue_page(&og);
        }
    }
    if (th_decode_res==TH_DUPFRAME)
        return 1;
    if (th_decode_res==0) {
        th_decode_ycbcr_out(td, ycbcr);
        switch(ti.pixel_fmt) {
           case TH_PF_420 :
              OGG_YUV2RGB_F420(S16, ycbcr); break;
           case TH_PF_422 :
              OGG_YUV2RGB_F422(S16, ycbcr); break;
           case TH_PF_444 :
              OGG_YUV2RGB_F444(S16, ycbcr); break;
        }

        return 1;
    }

    return 0;
}

void CloseVidOGG() {
   if (VidOpen) {
     if(vidfile != NULL) {
       fclose(vidfile); vidfile = NULL;
     }
     // ogg ------
     if (theora_p) {
        ogg_stream_clear(&to);
        ogg_stream_reset(&to);
        th_decode_free(td);
        th_comment_clear(&tc);
        th_info_clear(&ti);
        theora_p = 0;
     }
     ogg_sync_clear(&oy);
     //-----------
     DestroySurf(Sframe16);
     if(uFinal) { free(uFinal); uFinal = NULL; }
     if(vFinal) { free(vFinal); vFinal = NULL; }

     sizeVidFile = 0;
     readVidFileBytes = 0;
     VidOpen=false;
     VidEnded=false;
     TNM[6].Activ = 0; // disable menu close
     framenum=0;
     frameskipped=0;
     countBufferReads=0;
     HSldAdv->SetVal(0);
     CurVidFile="";
   }
}

int ogg_buffer_data(FILE *fIn, ogg_sync_state *oy) {
    if (oy == NULL)
        return 0;
    char *buffer = ogg_sync_buffer(oy, BUFFER_SIZE);
    int bytes = fread(buffer, 1, BUFFER_SIZE, fIn);
    readVidFileBytes += bytes;
    ogg_sync_wrote(oy, bytes);
    countBufferReads++;
    // too many reads without any ogg page ? force ending ogg buffer reads
    if (countBufferReads>=OGG_BUFFER_READS_OUT || bytes==0) {
       VidEnded=true; // we reached the end
       return 0;
    }

    return (bytes);
}

int ogg_queue_page(ogg_page *page) {
    if (theora_p)
        ogg_stream_pagein(&to, page);
    countBufferReads = 0;
    return 0;
}


void OGG_YUV2RGB_F420(DgSurf *S, th_ycbcr_buffer &ycbcr) {
    SYUVData  yuvData = { ycbcr[0].data, ycbcr[1].data, ycbcr[2].data,
                          ycbcr[0].stride, ycbcr[1].stride, ycbcr[2].stride,
                          ycbcr[0].width, ycbcr[0].height };

    YUV2RGB_F420(S, &yuvData);
}

void OGG_YUV2RGB_F422(DgSurf *S, th_ycbcr_buffer &ycbcr) {
    SYUVData  yuvData = { ycbcr[0].data, ycbcr[1].data, ycbcr[2].data,
                          ycbcr[0].stride, ycbcr[1].stride, ycbcr[2].stride,
                          ycbcr[0].width, ycbcr[0].height };

    YUV2RGB_F422(S, &yuvData);

}

void OGG_YUV2RGB_F444(DgSurf *S, th_ycbcr_buffer &ycbcr) {
    SYUVData  yuvData = { ycbcr[0].data, ycbcr[1].data, ycbcr[2].data,
                          ycbcr[0].stride, ycbcr[1].stride, ycbcr[2].stride,
                          ycbcr[0].width, ycbcr[0].height };

    YUV2RGB_F444(S, &yuvData);
}

// YUV4MPEG --------------

// return 0 if success, code error if failed
int OpenVidYUV4MPEG(char *FileName)
{
   ListString *LSParams;
   ListString *LSSubParams;


   String myFile=FileName;
   CloseVid();
   String strHeader(1024);
   // open the video file
   vidfile = fopen (myFile.StrPtr, "rb");

   if (vidfile==NULL)
     return 1; // failed to open

   // size file
   fseek(vidfile,0,SEEK_END);
   sizeVidFile = ftell(vidfile);
   readVidFileBytes = 0;
   fseek(vidfile,0,SEEK_SET);
   if(fgets(BuffReadHeadYUV4MPEG, 1024, vidfile) == NULL)
       return 1; // failed to read header
   readVidFileBytes += strlen(BuffReadHeadYUV4MPEG);

   strHeader = BuffReadHeadYUV4MPEG;
   strHeader.Del13_10();
   if ((LSParams = strHeader.Split(' ')) != NULL )
   {
     // at least we need YUV4MPEG signature, width and height
     if(LSParams->NbElement() < 3 || *(*LSParams)[0] != "YUV4MPEG2")
        return 1;
     // initialize parameters
     VideoFps = 25.0f; // default YUV4MPEG
     nWithYUV4MPEG = -1; // undefined
     nHeightYUV4MPEG = -1; // undefined
     nColorSpaceYUV4MPEG = 420; // DFAULT YUV4MPEG

     for (int nIdx = 1; nIdx < LSParams->NbElement(); nIdx++)
     {
       if((*LSParams)[nIdx]->Length() > 2)
       {
           switch((*LSParams)[nIdx]->StrPtr[0])
           {
           case 'W': // width
               (*LSParams)[nIdx]->DelCurs(0); // del first char
               nWithYUV4MPEG = (*LSParams)[nIdx]->GetInt();
               break;
           case 'H': // height
               (*LSParams)[nIdx]->DelCurs(0); // del first char
               nHeightYUV4MPEG = (*LSParams)[nIdx]->GetInt();
               break;
           case 'F': // fps
               (*LSParams)[nIdx]->DelCurs(0); // del first char
               if((LSSubParams = (*LSParams)[nIdx]->Split(':')) != NULL)
               {
                 if(LSSubParams->NbElement()==2 && (*LSSubParams)[0]->GetInt()>0 && (*LSSubParams)[1]->GetInt() > 0)
                 {
                     VideoFps = float((*LSSubParams)[0]->GetInt()) / float((*LSSubParams)[1]->GetInt());
                 }
                 delete LSSubParams;
               }
               break;
           case 'C': // color space
               (*LSParams)[nIdx]->DelCurs(0); // del first char
               nColorSpaceYUV4MPEG = (*LSParams)[nIdx]->GetInt();
               if(nColorSpaceYUV4MPEG != 420 && nColorSpaceYUV4MPEG != 422 && nColorSpaceYUV4MPEG != 444)
               {
                 delete LSParams;
                 fclose(vidfile);
                 vidfile = NULL;
                 return 1; // invalid color space
               }
               break;
           }
       }
     }
     delete LSParams;
   }
   if(nWithYUV4MPEG <= 0 && nHeightYUV4MPEG <= 0)
     return 1;
   yuvDataYUV4MPEG.height = nHeightYUV4MPEG;
   yuvDataYUV4MPEG.y_scan = yuvDataYUV4MPEG.width= nWithYUV4MPEG;
   switch(nColorSpaceYUV4MPEG)
   {
   case 444:
       nSizeDATAYUV4MPEG = nWithYUV4MPEG * nHeightYUV4MPEG * 3;
       BuffReadDATAYUV4MPEG = (unsigned char*)malloc(nSizeDATAYUV4MPEG);
       if(BuffReadDATAYUV4MPEG != NULL)
       {
           yuvDataYUV4MPEG.y = BuffY = BuffReadDATAYUV4MPEG;
           yuvDataYUV4MPEG.u = BuffU = &BuffReadDATAYUV4MPEG[nWithYUV4MPEG * nHeightYUV4MPEG];
           yuvDataYUV4MPEG.v = BuffV = &BuffReadDATAYUV4MPEG[nWithYUV4MPEG * nHeightYUV4MPEG * 2];
           yuvDataYUV4MPEG.u_scan = yuvDataYUV4MPEG.v_scan = nWithYUV4MPEG;
       }
       else {
         fclose(vidfile);
         vidfile = NULL;
         return 2; // no mem
       }
       break;
   case 422:
       nSizeDATAYUV4MPEG = nWithYUV4MPEG * nHeightYUV4MPEG * 2;
       BuffReadDATAYUV4MPEG = (unsigned char*)malloc(nSizeDATAYUV4MPEG);
       if(BuffReadDATAYUV4MPEG != NULL)
       {
           yuvDataYUV4MPEG.y = BuffY = BuffReadDATAYUV4MPEG;
           yuvDataYUV4MPEG.u = BuffU = &BuffReadDATAYUV4MPEG[nWithYUV4MPEG * nHeightYUV4MPEG];
           yuvDataYUV4MPEG.v = BuffV = &BuffReadDATAYUV4MPEG[nWithYUV4MPEG * (nHeightYUV4MPEG + nHeightYUV4MPEG/2)];
           yuvDataYUV4MPEG.u_scan = yuvDataYUV4MPEG.v_scan = nWithYUV4MPEG / 2;
       }
       else {
         fclose(vidfile);
         vidfile = NULL;
         return 2; // no mem
       }
       break;
   case 420:
       nSizeDATAYUV4MPEG = (nWithYUV4MPEG * nHeightYUV4MPEG * 3) / 2;
       BuffReadDATAYUV4MPEG = (unsigned char*)malloc(nSizeDATAYUV4MPEG);
       if(BuffReadDATAYUV4MPEG != NULL)
       {
           yuvDataYUV4MPEG.y = BuffY = BuffReadDATAYUV4MPEG;
           yuvDataYUV4MPEG.u = BuffU = &BuffReadDATAYUV4MPEG[nWithYUV4MPEG * nHeightYUV4MPEG];
           yuvDataYUV4MPEG.v = BuffV = &BuffReadDATAYUV4MPEG[nWithYUV4MPEG * nHeightYUV4MPEG + ((nWithYUV4MPEG/2) * (nHeightYUV4MPEG/2))];
           yuvDataYUV4MPEG.u_scan = yuvDataYUV4MPEG.v_scan = nWithYUV4MPEG / 2;
       }
       else {
         fclose(vidfile);
         vidfile = NULL;
         return 2; // no mem
       }
       break;
   }

    if (CreateSurf(&Sframe16, nWithYUV4MPEG, nHeightYUV4MPEG, 16)==0) {
      free(BuffReadDATAYUV4MPEG);
      BuffReadDATAYUV4MPEG = BuffY = BuffU = BuffV = NULL;
      fclose(vidfile);
      vidfile = NULL;
      return 2; // no mem
    }

    uFinal = (unsigned char*) malloc(nWithYUV4MPEG);
    vFinal = (unsigned char*) malloc(nWithYUV4MPEG);
    if(uFinal == NULL || vFinal == NULL)
    {
      if (uFinal!=NULL) { free(uFinal); uFinal = NULL; }
      if (vFinal!=NULL) { free(vFinal); vFinal = NULL; }
      DestroySurf(Sframe16);
      free(BuffReadDATAYUV4MPEG);
      BuffReadDATAYUV4MPEG = BuffY = BuffU = BuffV = NULL;
      fclose(vidfile);
      vidfile = NULL;
      return 2; // no mem
    }

    VidOpen= true; // opened video

    framenum=0; // found one frame
    countBufferReads=0;
    PosSynch=0;
    InitSynch(SynchBuff,&PosSynch,VideoFps);
    VidPause=0;
    VidEnded=false;
    frameskipped=0;
    FrameAvlbl=0;
    CurVidFile=myFile; // save current file
    TNM[6].Activ = 1; // enable menu close
    return 0; // success
}
// return 1 if new frame found, 0 else
int GetNextFrameYUV4MPEG(DgSurf *S16, unsigned int nFramesToDrop)
{
   String strHeader(1024);
   String strSubFrame(1024);
   String *pTmpStr = NULL;
   ListString *LSParams;
   int nPosF = -1;
   unsigned int cntFrames2Drop = nFramesToDrop;
   bool bFoundFrame = false;
   // find frame header
   if(vidfile == NULL ||  !VidOpen)
      return 0;
   for(;;)
   {
        if(fgets(BuffReadHeadYUV4MPEG, 1024-1, vidfile) == NULL) {
            VidEnded=true; // we reached the end
            return 0; // failed to read header
        }

        readVidFileBytes += strlen(BuffReadHeadYUV4MPEG);

        if(readVidFileBytes == 0) {
            VidEnded=true; // we reached the end
            return 0;
        }

        strHeader = BuffReadHeadYUV4MPEG;
        strHeader.Del13_10();
        nPosF = strHeader.FindChar('F', 0); // find F - FRAME
        if(nPosF == -1)
            continue;

        if((pTmpStr = strHeader.SubString(nPosF, 1024)) != NULL)
        {
            strSubFrame = *pTmpStr;
            delete pTmpStr;
            if ((LSParams = strSubFrame.Split(' ')) != NULL) {
              nPosF = LSParams->Index("FRAME");
              delete LSParams;
              if (nPosF != -1) {
                if(cntFrames2Drop == 0)
                  break; // FRAME found :)
                else
                {
                  cntFrames2Drop--;
                  if(fseek(vidfile, nSizeDATAYUV4MPEG, SEEK_CUR) == 0)
                    readVidFileBytes += nSizeDATAYUV4MPEG;
                  else
                  {
                    VidEnded=true; // we reached the end
                    return 0;
                  }
                }
              }
            }
        }
   }

   // tri to read frame DATA
   if(fread(BuffReadDATAYUV4MPEG, nSizeDATAYUV4MPEG, 1,  vidfile) == 0) {
      VidEnded=true; // we reached the end
      return 0;
   }

   readVidFileBytes += nSizeDATAYUV4MPEG;

   switch(nColorSpaceYUV4MPEG) {
      case 420 :
         YUV2RGB_F420(S16, &yuvDataYUV4MPEG);
         break;
      case 422 :
         YUV2RGB_F422(S16, &yuvDataYUV4MPEG);
         break;
      case 444 :
         YUV2RGB_F444(S16, &yuvDataYUV4MPEG);
         break;
   }

   return 1;
}
// close an opened video
void CloseVidYUV4MPEG()
{
   if (VidOpen) {
     free(BuffReadDATAYUV4MPEG);
     BuffReadDATAYUV4MPEG = BuffY = BuffU = BuffV = NULL;
     fclose(vidfile);
     vidfile = NULL;
     DestroySurf(Sframe16);
     if(uFinal) { free(uFinal); uFinal = NULL; }
     if(vFinal) { free(vFinal); vFinal = NULL; }

     sizeVidFile = 0;
     readVidFileBytes = 0;
     VidOpen=false;
     VidEnded=false;
     TNM[6].Activ = 0; // disable menu close
     framenum=0;
     frameskipped=0;
     countBufferReads=0;
     HSldAdv->SetVal(0);
     CurVidFile="";
   }
}


///////////////////////////////////////

int kindVidOpened = 0; // 0 : none, 1 : MPEG, 2 : OGG, 3 : YUV4MPEG
// return 0 if success, code error if failed
int OpenVid(char *FileName)
{
    int    ret    = 0;
    int    retFF  = 0;
    String myFile = FileName;
    String InfImg(256);

    int    OpenVidWidth = 0;
    int    OpenVidHeight = 0;
    char   tdrv[MAXDRIVE], tpath[MAXDIR], tfile[MAXFILE], text[MAXEXT];
    String sFinalLabel = sMainWinName;

    CloseVid();

    if((retFF=OpenVidFFMPEG(myFile.StrPtr)) == 0)
       kindVidOpened = 4;
    else
    if((ret=OpenVidYUV4MPEG(myFile.StrPtr)) == 0)
       kindVidOpened = 3;
    else
    if((ret=OpenVidMPEG(myFile.StrPtr)) == 0)
       kindVidOpened = 1;
    else
    if((ret=OpenVidOGG(myFile.StrPtr)) == 0)
       kindVidOpened = 2;
    if (VidOpen && kindVidOpened > 0) {
      char errBuff[128]="";
      av_make_error_string(errBuff, 127, FFFail);
      sprintf(InfImg.StrPtr,"%ix%i %3.1ffps %3.1f secs", Sframe16->ResH, Sframe16->ResV, VideoFps, VideoTime);
      fnsplit(CurVidFile.StrPtr, tdrv, tpath, tfile, text);
      sFinalLabel = sFinalLabel + '<' + tfile + text + ">" + InfImg;
    }

    MWDPlayer->Label = sFinalLabel;
    MWDPlayer->Redraw();

    return ret;
}
// return 1 if new frame found, 0 else
int GetNextFrame(DgSurf *S16, unsigned int nFramesToDrop)
{
   switch(kindVidOpened) {
     case 1:
       return GetNextFrameMPEG(S16, nFramesToDrop);
     case 2:
       return GetNextFrameOGG(S16, nFramesToDrop);
     case 3:
       return GetNextFrameYUV4MPEG(S16, nFramesToDrop);
     case 4:
       return GetNextFrameFFMPEG(S16, nFramesToDrop);
     default:
       return 0;
   }
}
// close an opened video
void CloseVid() {
  if (VidOpen && kindVidOpened > 0) {
    MWDPlayer->Label = sMainWinName;
    MWDPlayer->Redraw();
    switch(kindVidOpened) {
      case 1:
        CloseVidMPEG(); break;
      case 2:
        CloseVidOGG(); break;
      case 3:
        CloseVidYUV4MPEG(); break;
      case 4:
        CloseVidFFMPEG(); break;
    }
    kindVidOpened = 0;
    VideoFps = 0.0f;
    VideoTime = 0.0f;
    sizeVidFile = 0;
    readVidFileBytes = 0;
    videoWidth = 0;
    videoHeight = 0;
    videoFramesCount = 0;
    framenum = 0;
    UpdatePlayTime();
  }
}
// FFMPEG /////////////////////////////

void DestroyFFMPEG(); // internal cleanup
// return 0 if success, code error if failed
int OpenVidFFMPEG(char *FileName) {
    int retfunc = 0;

	// Open media file.
	if (avformat_open_input(&pFormatCtx, FileName, NULL, NULL) != 0) {
        DestroyFFMPEG();
		return 1;
	}
	// Get format info.
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        DestroyFFMPEG();
		return 2;
	}

    videoStreamIndex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);;
    VideoTime = (double)(pFormatCtx->duration)/AV_TIME_BASE;

    if (videoStreamIndex < 0) {
        DestroyFFMPEG();
        return 3;
    }
    AVStream *st;
    st = pFormatCtx->streams[videoStreamIndex];
    /* find decoder for the stream */
    pVideoCodec = avcodec_find_decoder(st->codecpar->codec_id);
    if (pVideoCodec == NULL) {
        DestroyFFMPEG();
        return 4;
    }
    /* Allocate a codec context for the decoder */
    pVideoCodecCtx = avcodec_alloc_context3(pVideoCodec);
    if (pVideoCodecCtx == NULL) {
        DestroyFFMPEG();
        return 5;
    }
    /* Copy codec parameters from input stream to output codec context */
    if (avcodec_parameters_to_context(pVideoCodecCtx, st->codecpar) < 0) {
        DestroyFFMPEG();
        return 6;
    }
    /* Init the decoders */

    if (avcodec_open2(pVideoCodecCtx, pVideoCodec, NULL) < 0) {
        DestroyFFMPEG();
        return 7;
    }

    video_stream = pFormatCtx->streams[videoStreamIndex];

    FFZone = 77777;
    AVDictionaryEntry *rotate_tag = av_dict_get(video_stream->metadata, "rotate", NULL, 0);
    if (rotate_tag != NULL) {
        videoSwapUpDown = ((FFZone = atoi(rotate_tag->value)) == 180);
    }

    /* dump input information to stderr */
    av_dump_format(pFormatCtx, 0, FileName, 0);
    /* allocate frame */
    videoFrame = av_frame_alloc();
    if (!videoFrame) {
        DestroyFFMPEG();
        return 8;
    }

    /* allocate packet */
    pkt = av_packet_alloc();
    if (!pkt) {
        DestroyFFMPEG();
        return 9;
    }

    // get frames per second
    VideoFps = (float)av_q2d(video_stream->r_frame_rate);
    // init parser
    pVideoCodecParser = av_parser_init(pVideoCodec->id);
    if(pVideoCodecParser == NULL) {
        DestroyFFMPEG();
        return 10;  // failed parser init
    }
    /* allocate image where the decoded image will be put */
    videoWidth = pVideoCodecCtx->width;
    videoHeight = pVideoCodecCtx->height;
    pix_fmt = pVideoCodecCtx->pix_fmt;

    if ((retfunc = av_image_alloc(video_dst_data, video_dst_linesize,
                       videoWidth, videoHeight, AV_PIX_FMT_RGB565BE, 1)) < 0) {
        DestroyFFMPEG();
        return 11; //"Could not allocate raw video buffer\n"
    }
    video_dst_bufsize = retfunc;
    if (CreateSurf(&Sframe16, videoWidth, videoHeight, 16)==0) {
        DestroyFFMPEG();
        return 12; // no mem
    }

    VidOpen=true; // opened video

    // allocate buffer required for u,v interpolation
    uFinal = (unsigned char*) malloc(videoWidth);
    vFinal = (unsigned char*) malloc(videoWidth);
    if (uFinal == NULL || vFinal == NULL) {
        CloseVidFFMPEG();
        return 13;  // no mem
    }
    // try to read first videoStream packet
    bool foundVideoStream = false;
    while ((retfunc = av_read_frame(pFormatCtx, pkt)) >= 0) {
        // check if the packet belongs to a stream we are interested in, otherwise
        // skip it
        if (pkt->stream_index == videoStreamIndex) {
            // submit the packet to the decoder
            if (avcodec_send_packet(pVideoCodecCtx, pkt) >= 0) {
                foundVideoStream = true;
                break;
            } else {
                av_packet_unref(pkt);
                CloseVidFFMPEG();
                return 14;  // failed to read first packet or empty file!
            }
        } else {
            av_packet_unref(pkt);
        }
    }

    if (!foundVideoStream) {
        CloseVidFFMPEG();
        return 15;  // failed to find video stream
    }

    // create Surf that will contain final frame
    if(GetNextFrameFFMPEG(Sframe16,0)!=1) {
        CloseVidFFMPEG();
        return 16;  // no frame
    }
    // Need for convert time to ffmpeg time.
    videoFramesCount = (int)(VideoFps*VideoTime);

    framenum=0; // found one frame
    countBufferReads=0;
    PosSynch=0;
    InitSynch(SynchBuff,&PosSynch,VideoFps);
    VidOpen=true; // opened video
    VidPause=0;
    VidEnded=false;
    frameskipped=0;
    FrameAvlbl=0;
    CurVidFile=FileName; // save current file
    TNM[6].Activ = 1; // enable menu close
    return 0;
}

// return 1 if new frame found, 0 else
int GetNextFrameFFMPEG(DgSurf *S16, unsigned int nFramesToDrop) {
    unsigned int nDrops     = nFramesToDrop;
    int ret_av = 0;
    DgSurf *Surf8bpp = NULL;
    bool frameFound = false;
    if (!VidOpen || videoStreamIndex < 0 ) return 0;

    // available frame ?
    while (!frameFound) {
        while (nDrops > 0) {
            if ((ret_av = avcodec_receive_frame(pVideoCodecCtx, videoFrame))>=0) {
                av_frame_unref(videoFrame);
                nDrops--;
            } else {
                break;
            }
        }
        if (ret_av >=0 && avcodec_receive_frame(pVideoCodecCtx, videoFrame)>=0) {
            frameFound = true;
            continue;
        }
        while ((ret_av = av_read_frame(pFormatCtx, pkt)) >= 0) {
            // check if the packet belongs to a stream we are interested in, otherwise
            // skip it
            if (pkt->stream_index == videoStreamIndex) {
                // submit the packet to the decoder
                if (avcodec_send_packet(pVideoCodecCtx, pkt) < 0) {
                    av_packet_unref(pkt);
                    VidEnded=true; // we reached the end
                    return 0; // no frame
                } else {
                    // new packet found try if it contains frames
                    av_packet_unref(pkt);
                    break;
                }
            } else {
                av_packet_unref(pkt);
            }
        }
        // last read failure ?
        if (ret_av < 0) {
            VidEnded = true;
            return 0;
        }
    }

    if (frameFound) {
        SYUVData  yuvData;
        // preliminary setting if format is yuv
        yuvData.y= (unsigned char*)videoFrame->data[0];
        yuvData.u= (unsigned char*)videoFrame->data[1];
        yuvData.v= (unsigned char*)videoFrame->data[2];

        yuvData.y_scan= yuvData.width= videoWidth;
        yuvData.u_scan= yuvData.v_scan= videoWidth/2;
        yuvData.height= videoHeight;

        // convert image to RGB16(565)
        switch (pVideoCodecCtx->pix_fmt) {
            case AV_PIX_FMT_YUV420P:
            case AV_PIX_FMT_YUVJ420P:
                YUV2RGB_F420(S16, &yuvData);
                break;
            case AV_PIX_FMT_YUV422P:
            case AV_PIX_FMT_YUVJ422P:
                YUV2RGB_F422(S16, &yuvData);
                break;
            case AV_PIX_FMT_YUV444P:
            case AV_PIX_FMT_YUVJ444P:
                yuvData.u_scan= videoWidth;
                yuvData.v_scan= videoWidth;
                YUV2RGB_F444(S16, &yuvData);
                break;
            case AV_PIX_FMT_PAL8:
                // not working
                if (CreateSurfBuff(&Surf8bpp, videoWidth, videoHeight, 8, videoFrame->data[0])) {
                    ConvSurf8ToSurf16Pal(S16,Surf8bpp,videoFrame->data[1]);
                    DestroySurf(Surf8bpp);
                };
                break;
            case AV_PIX_FMT_RGB24:
                for (int i=0; i < S16->SizeSurf /2 ; i++) {
                    ((unsigned short*)S16->rlfb)[i] = RGB16(videoFrame->data[0][i*3+0], videoFrame->data[0][i*3+1], videoFrame->data[0][i*3+2]);
                }
                break;
            case AV_PIX_FMT_BGR24:
                for (int i=0; i < S16->SizeSurf /2 ; i++) {
                    ((unsigned short*)S16->rlfb)[i] = RGB16(videoFrame->data[0][i*3+2], videoFrame->data[0][i*3+1], videoFrame->data[0][i*3+0]);
                }
                break;
            case AV_PIX_FMT_RGBA:
                for (int i=0; i < S16->SizeSurf /2 ; i++) {
                    ((unsigned short*)S16->rlfb)[i] = RGB16(videoFrame->data[0][i*4+0], videoFrame->data[0][i*4+1], videoFrame->data[0][i*4+2]);
                }
                break;
            case AV_PIX_FMT_BGRA:
                for (int i=0; i < S16->SizeSurf /2 ; i++) {
                    ((unsigned short*)S16->rlfb)[i] = RGB16(videoFrame->data[0][i*4+2], videoFrame->data[0][i*4+1], videoFrame->data[0][i*4+0]);
                }
                break;
            default:
                DgSurf saveSurf;
                DgGetCurSurf(&saveSurf);
                DgSetCurSurf(S16);
                Clear16(RGB16(0,0,0));
                char interstr[64];
                ClearText();
                SetTextCol(RGB16(255,255,255));
                OutText16Mode("oops ! unsupported ffmpeg pixel_format!\n", AJ_MID);
                OutText16ModeFormat(AJ_MID, interstr, 63, "ID %i\n", (int)pVideoCodecCtx->pix_fmt);
                DgSetCurSurf(&saveSurf);
                break;
        }
        // free videoFrame
        av_frame_unref(videoFrame);
        return 1;
    }
    VidEnded=true; // we reached the end
    return 0;
}

// destroy/free as much as possible ffmpeg allocated mem/ressources
void DestroyFFMPEG() {

    if (pVideoCodecCtx) {
        avcodec_send_packet(pVideoCodecCtx, NULL);
        avcodec_free_context(&pVideoCodecCtx);
        pVideoCodecCtx = NULL;
    }

    if (pFormatCtx) {
        avformat_close_input(&pFormatCtx);
        pFormatCtx = NULL;
    }

    if (pVideoCodecParser) {
        av_parser_close(pVideoCodecParser);
        pVideoCodecParser = NULL;
    }
    if (videoFrame) {
        av_frame_free(&videoFrame);
        videoFrame = NULL;
    }
    if (pkt) {
        av_packet_free(&pkt);
        pkt = NULL;
    }

    if(uFinal) { free(uFinal); uFinal = NULL; }
    if(vFinal) { free(vFinal); vFinal = NULL; }
    if (Sframe16!=NULL) {
        DestroySurf(Sframe16);
        Sframe16 = NULL;
    }
    videoStreamIndex = -1;
    videoFramesCount = 0;
}

// close an opened video
void CloseVidFFMPEG() {
    if (!VidOpen)
        return;
    DestroyFFMPEG();
    VidOpen = false;
}


///////////////////////////////////////
// general YUV 2 RGB conversion routine
///////////////////////////////////////

void YUV2RGB_F420(DgSurf *S, SYUVData *pYUVDATA) {
    unsigned char *yFrm    =NULL;
    unsigned char *uFrm    =NULL;
    unsigned char *vFrm    =NULL;
    unsigned char *uFrmNL  =NULL;
    unsigned char *vFrmNL  =NULL;
    unsigned int scanlinePtr = S->rlfb;

    if(InterpolateUV) {
       for (int idx = 0; idx <pYUVDATA->height; idx++) {
           yFrm = (unsigned char *)pYUVDATA->y+(pYUVDATA->y_scan*idx);
           uFrm = (unsigned char *)pYUVDATA->u+(pYUVDATA->u_scan*(idx>>1));
           vFrm = (unsigned char *)pYUVDATA->v+(pYUVDATA->v_scan*(idx>>1));
           // next line pointer
           if (idx&1) {
              if (idx<(pYUVDATA->height-1)) {
                 uFrmNL = (unsigned char *)pYUVDATA->u+(pYUVDATA->u_scan*((idx>>1)+1));
                 vFrmNL = (unsigned char *)pYUVDATA->v+(pYUVDATA->v_scan*((idx>>1)+1));
              } else {
                 uFrmNL = uFrm;
                 vFrmNL = vFrm;
              }
           }
           for (int iw=0;iw<pYUVDATA->width;iw++) {
               // interpolate u and v values for odd lines or columns
               if (idx&1) {
                  if ((iw&1) && iw<pYUVDATA->width-1) {
                     uFinal[iw] = (uFrm[iw/2]+uFrmNL[iw/2]+uFrm[iw/2+1]+uFrmNL[iw/2+1])/4;
                     vFinal[iw] = (vFrm[iw/2]+vFrmNL[iw/2]+vFrm[iw/2+1]+vFrmNL[iw/2+1])/4;
                  } else {
                     uFinal[iw] = (uFrm[iw/2]+uFrmNL[iw/2])/2;
                     vFinal[iw] = (vFrm[iw/2]+vFrmNL[iw/2])/2;
                  }
               } else {
                  if ((iw&1) && iw<pYUVDATA->width-1) {
                     uFinal[iw] = (uFrm[iw/2]+uFrm[iw/2+1])/2;
                     vFinal[iw] = (vFrm[iw/2]+vFrm[iw/2+1])/2;
                  } else {
                     uFinal[iw] = uFrm[iw>>1];
                     vFinal[iw] = vFrm[iw>>1];
                  }
               }
           }
           ScanYUV2RGB16(yFrm, uFinal, vFinal, (unsigned short *)(scanlinePtr), pYUVDATA->width);
           scanlinePtr+=S->ScanLine;
       }
    }
    else {
       for (int idx = 0; idx <pYUVDATA->height; idx++)
       {
           Scan422YUV2RGB16((unsigned char *)(pYUVDATA->y+(pYUVDATA->y_scan*idx)),
                (unsigned char *)(pYUVDATA->u+(pYUVDATA->u_scan*(idx>>1))),
                (unsigned char *)(pYUVDATA->v+(pYUVDATA->v_scan*(idx>>1))),
                (unsigned short *)(scanlinePtr), pYUVDATA->width);
           scanlinePtr+=S->ScanLine;
       }
    }
}

void YUV2RGB_F422(DgSurf *S, SYUVData *pYUVDATA) {
    unsigned char *yFrm    =NULL;
    unsigned char *uFrm    =NULL;
    unsigned char *vFrm    =NULL;
    unsigned int scanlinePtr = S->rlfb;

    if(InterpolateUV) {
       for (int idx = 0; idx <pYUVDATA->height; idx++) {
           yFrm = (unsigned char *)pYUVDATA->y+(pYUVDATA->y_scan*idx);
           uFrm = (unsigned char *)pYUVDATA->u+(pYUVDATA->u_scan*idx);
           vFrm = (unsigned char *)pYUVDATA->v+(pYUVDATA->v_scan*idx);
           for (int iw=0;iw<pYUVDATA->width;iw++) {
               if ((iw&1) && iw<pYUVDATA->width-1) {
                   uFinal[iw] = (uFrm[iw/2]+uFrm[iw/2+1])/2;
                   vFinal[iw] = (vFrm[iw/2]+vFrm[iw/2+1])/2;
               } else {
                  uFinal[iw] = uFrm[iw/2];
                  vFinal[iw] = vFrm[iw/2];
               }
           }
           ScanYUV2RGB16(yFrm, uFinal, vFinal, (unsigned short *)(scanlinePtr), pYUVDATA->width);
           scanlinePtr+=S->ScanLine;
       }
    }
    else {
       for (int idx = 0; idx <pYUVDATA->height; idx++)
       {
           Scan422YUV2RGB16((unsigned char *)(pYUVDATA->y+(pYUVDATA->y_scan*idx)),
                (unsigned char *)(pYUVDATA->u+(pYUVDATA->u_scan*idx)),
                (unsigned char *)(pYUVDATA->v+(pYUVDATA->v_scan*idx)),
                (unsigned short *)(scanlinePtr), pYUVDATA->width);
           scanlinePtr+=S->ScanLine;
       }
    }
}


void YUV2RGB_F444(DgSurf *S, SYUVData *pYUVDATA) {
    unsigned int width       = pYUVDATA->width;
    unsigned int scanlinePtr = S->rlfb;
    unsigned int strides     = 0;


    for (int idx = 0; idx <pYUVDATA->height; idx++)
    {
        ScanYUV2RGB16((unsigned char *)pYUVDATA->y+strides,
                        (unsigned char *)pYUVDATA->u+strides,
                        (unsigned char *)pYUVDATA->v+strides,
                        (unsigned short *)(scanlinePtr), width);
        scanlinePtr+=S->ScanLine;
        strides+=pYUVDATA->y_scan;
    }
}



