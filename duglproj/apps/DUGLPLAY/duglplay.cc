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
    28 February 2026:
    Remove all other decoders and rely only on ffmpeg
    Faster frame dropping with less quality to avoid mostly hang on high definition videos
    2 march 2026 : 1.0 alpha 2
    Add experimental seeking using the horizontal slider.
    Save/restore working directory at player startup/close to avoid changing current user directory.
    several fixes and cleanup
    26 march 2026 : 1.0 alpha 3
    Add experimental sound decoding/support using Sound Blaster 16 driver
    Improve config file with new sound audio paramters, true|false to enable disable,
    Add reverting to 640x480 if selected config file video resolution isn't available ...
    Keyboard 'P' now pause continue playing current video
*/

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dir.h>
#include <math.h>

#include <dugl.h>
#include <duglplus.h>

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
#include <libavutil/cpu.h>
#include <libswresample/swresample.h>
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
bool MouseSupported = false;
char keybMapFileName[256] = "qwerteng.map";

unsigned int screenX = 640, screenY = 480;
//unsigned int screenX = 320, screenY = 240;
//unsigned int screenX = 1024, screenY = 768;
// full screen frame - quad drawing
float DefMsPosX = 0.5, DefMsPosY = 0.5;

DgSurf *MsPtr,*MsPtr16,*rendSurf16,*blurSurf16;

char startdir[PATH_MAX]= "";
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


bool VidOpen=false,
    VidPause=false,
    VidEnded=false,
    FrameAvlbl=false,
    closeOnVidEnded=false,
    videoRotate180 = false,
    videoFlipHorizontally = false,
    videoTransluent = false;
float VideoFps = 0.0f;
float VideoTime = 0.0f;
unsigned int sizeVidFile = 0, readVidFileBytes = 0;
int videoWidth = 0, videoHeight = 0;
int videoFramesCount = 0;
int framenum,
    PosSynch,
    frameskipped=0;
int DefTypeOpen=0;
int kindVidOpened = 0; // 0 : none, 4: FFMPEG

DgSurf *Sframe16; // Surf where the 16bpp video frame will be stored

unsigned char *uFinal = NULL;
unsigned char *vFinal = NULL;

// -----------------------
// FFMPEG GLOBAL
AVFormatContext* pFormatCtx = NULL;
// FFmpeg video codec context.
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
// Audio ######
int audioStreamIndex = -1;
// FFmpeg codec for audio
const AVCodec* pAudioCodec = NULL;
// FFmpeg Audio codec context.
AVCodecContext* pAudioCodecCtx = NULL;
// Audio codec param
AVCodecParameters *pCodecAudioParam = NULL;
// FFmpeg parser codec for video.
AVCodecParserContext *pAudioCodecParser = NULL;
// audio Stream
AVStream *audio_stream = NULL;
// audio convert context
struct SwrContext *au_convert_ctx;

static uint8_t *video_dst_data[4] = {NULL};
static int      video_dst_linesize[4];
static int      video_dst_bufsize;
static AVFrame  *videoFrame = NULL;
static AVFrame  *audioFrame = NULL;
static AVPacket *pkt = NULL;
int FFZone = 0, FFFail = 0;

// audio handling
char soundDriverFileName[256] = "sb16.drv";
#define AUDIO_RING_SIZE 4
DVoice *audioRing[AUDIO_RING_SIZE];
int audioRingStart = 0;
int audioRingEnd = 0;
int audioRingCount = 0;
int audioFrameSamples = 0;
int countRingQueued = 0;
int countRingAdd = 0;
int curVoicePos = 0;
SoundDRV *SndDrv = NULL;
void *SndBuff = NULL;

bool Audio16Bits = false,
     AudioStereo = false,
     AudioEnabled = false,
     AudioMuted = false,
     SoundEnabled = true,
     AboutDebugInfo = false;
int AudioSamplingSpeed = 22000;
int VoiceSampleSize = 8000;
int MasterAudioVolume = 230;
int VoiceAudioVolume = 230;
int OutGainAudioVolume = 230;


// load sound driver, init/detect sound card, reset audioRing buffer
bool InitSound(bool bits16, bool stereo, int sampleSpeed);
// uninstall sound driver, free/reset audio ring buffer
void CloseSound();
// Voice Effect Pack
DVoicePack VP = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
// Add Voice
int AddVoice(DVoice *Vc,int State, bool updateSpeed);
// Queue Voice
int QueueVoice(DVoice *toQueueVc,DVoice *Vc,int State, bool updateSpeed);

// Create/Prepare Audio Ring DVoices
bool CreatePrepAudioRingDVoices();
// update sampling speed of AudioRingDVoice
void UpdateAudioRingDVoicesSamplingSpeed(int newSampling);
// Destroy/Unprepare Audio Ring DVoices
bool DestroyUnprepAudioRingDVoices();


// return 0 if success, code error if failed
int OpenVidFFMPEG(char *FileName);
// return 1 if new frame found, 0 else
int GetNextFrameFFMPEG(DgSurf *S16, unsigned int nFramesToDrop);
// try to seek to FrameNum return 1 successfull, 0 else
int SeekFrameFFMPEG(DgSurf *S16, unsigned int FrameNum);
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
String sMainWinName("DUGL Video Player 1.0 alpha 3");
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
int ignoreSeekSliderChange = 0;

char playTime[16];
// events
void OnMenuOpenVid(),OnMenuCloseVid(),OnMenuExit(),OnMenuFullScr(),
     OnMenuPauseCont(),OnMenuAbout();
void OnMenuVSynch(),OnMenuSmoothFS(),OnMenuFSFps(),OnMenuFSTime();
void OnMenuLoop(),OnMenuFrameDrop();
void OnChBLoopChanged(char),OnMenuInterpolateUV(), OnMenuFitScreen();

void GphBDrawVideo(GraphBox *Me),GphBScanVideo(GraphBox *Me);
void OnBtPlayClick();
void OnSeekSliderChange(int val);

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
bool StringToBool(char *str);
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

    // save starting dir
    getcwd(startdir, PATH_MAX - 1);
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
    // sound
    InitSound(Audio16Bits, AudioStereo, AudioSamplingSpeed);

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
    HSldAdv->Changed = OnSeekSliderChange;
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
                if (GetNextFrame(Sframe16, (PosSynch > (framenum))  ? (PosSynch-framenum-1) : 0)) {
                   redrawVid=1;
                   if (!FrameAvlbl) FrameAvlbl=true;
                }
                else {
                   VidPause=1; // set video as paused
                   // enable GUI if full screen, and if we are not looping
                   if (EnableGUI==0 && (!CBxLoop->True)) {
                      OnMenuFullScr();
                   }
                }
             } else {
                if (GetNextFrame(Sframe16,0)==1) {
                   if (audioRingCount > 0) {
                     int previousToStartIdx = (audioRingStart+AUDIO_RING_SIZE-1)%AUDIO_RING_SIZE;
                     if (!SndDrv->ExistVoice(audioRing[previousToStartIdx])) {
                          if(SndDrv->GetCountVoices() == 0) {
                             AddVoice(audioRing[audioRingStart], 0, true);
                             countRingAdd++;
                             audioRingCount--;
                             audioRingStart=(audioRingStart+1)%AUDIO_RING_SIZE;
                          }
                     } else {
                         if (!QueueVoice(audioRing[previousToStartIdx],audioRing[audioRingStart], 0, true)) {
                             AddVoice(audioRing[audioRingStart], 0, true);
                             countRingAdd++;
                         } else {
                             countRingQueued++;
                         }
                         audioRingCount--;
                         audioRingStart=(audioRingStart+1)%AUDIO_RING_SIZE;
                     }
                   }

                   redrawVid=1;
                   if (!FrameAvlbl) FrameAvlbl=true;
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
        // space + tab  or key P: pause / continue
        if (((WH->KeyFLAG&KB_SPACE_PR) && (WH->KeyFLAG&KB_TAB_PR)) || (WH->Key==KB_KEY_QWERTY_P))
             OnMenuPauseCont();
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
        // space + tab or key P: pause / continue
        if (((keyFLAG&KB_SPACE_PR) && (keyFLAG&KB_TAB_PR)) || (keyCode==KB_KEY_QWERTY_P))
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
            ResizeViewSurf16(Sframe16, 0, videoRotate180);
          }
          else {
            Clear16(0); // clear by black
//            //PutSurf16(Sframe16, (CurSurf.MaxX+CurSurf.MinX-Sframe16->ResH)/2,
            PutSurf16(Sframe16, (CurSurf.MaxX+CurSurf.MinX-Sframe16->ResH)/2,
                (CurSurf.MaxY+CurSurf.MinY-Sframe16->ResV)/2, (videoRotate180) ? INV_VT_PUT : 0);
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

    CloseVid();
    CloseSound();
    DgQuit();
    UninstallKeyboard();
    DgUninstallTimer();
    if(MouseSupported)
       UninstallMouse();

    TextMode();
    // restore starting dir
    chdir(startdir);

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

bool StringToBool(char *str) {
    int lenStr = strlen(str);
    if (lenStr == 1 && isdigit(str[0])) {
        return (bool)atoi(str);
    } else if (lenStr >=4) {
        // to lower
        for (int i =0;i<lenStr;i++)
            str[i] = tolower(str[i]);
        if (strcmp(str, "true") == 0)
            return true;
    }
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
  if (ignoreSeekSliderChange != 1) {
      ignoreSeekSliderChange = 1;
      HSldAdv->SetVal(videoAdv);
      ignoreSeekSliderChange = 0;
  }
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
    CloseSound();
    DgQuit();
    UninstallKeyboard();
    DgUninstallTimer();
    if(MouseSupported)
        UninstallMouse();
    TextMode();
    // restore starting dir
    chdir(startdir);

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
    MWAbout->Show(); // show about
    MWAbout->Enable(); // set as the active window
}

void GphBDrawVideo(GraphBox *Me) {

   // opened video ?
   if (VidOpen) {
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
      FrameAvlbl=false;
      return;
   }
   ClearSurf16(WH->m_GraphCtxt->WinGrisF);
}

void GphBScanVideo(GraphBox *Me) {
   if (redrawVid) {
     if (VidOpen)
        Me->Redraw();
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

void OnSeekSliderChange(int val) {
    unsigned int targetFrame = 0;
    if (!VidOpen) {
        HSldAdv->SetVal(0);
        return;
    }
    if (ignoreSeekSliderChange == 1)
        return;
    switch(kindVidOpened) {
        case 4:
            targetFrame = val * videoFramesCount / 100;
            if (SeekFrameFFMPEG(Sframe16, targetFrame) == 1) {
                VidPause = true;
                framenum = targetFrame;
                frameskipped = 0;
                redrawVid = 1;
                UpdatePlayTime();
            } else { // fail, restore slider position
                UpdatePlayTime();
            }
            return;
        break;
        default:
            return;
   }
}

// MWAbout events
void BtOkAboutClick() {
  MWAbout->Hide(); // show about
  MWDPlayer->Enable(); // enable main win

}

void OnGphBScanAbout(GraphBox *Me) {
  // synchronise
  Me->Redraw();

}

void GphBDrawAbout(GraphBox *Me) {
   char midText[256] = "";

   ClearSurf16(RGB16(0,0,0));
   ClearText();
   SetTextCol(WH->m_GraphCtxt->WinBlanc);
   if (!AboutDebugInfo) OutText16Mode("\n", AJ_MID);
   FntCol=RGB16(0,255,0); // green
   OutText16Mode("DUGL Player 1.0 Alpha 3 - DOS Video Player\n", AJ_MID);
   FntCol=RGB16(255,255,255); // white
   OutText16Mode("(C) By FFK 26 March 2026\n\n", AJ_MID);
   OutText16Mode("Developped using :\n", AJ_MID);
   FntCol=RGB16(255,255,0); // yellow
   OutText16ModeFormat(AJ_MID, midText, 255,"DUGL %s\n",DUGL_VERSION);
   FntCol=RGB16(0,255,255); // white
   OutText16Mode("https://github.com/FFK77/DOS-DUGL\n", AJ_MID);
   FntCol=RGB16(255,255,0); // yellow
   OutText16ModeFormat(AJ_MID, midText, 255, "FFMPEG %s\n", av_version_info());
   FntCol=RGB16(255,255,255); // white
   OutText16Mode("https://ffmpeg.org/\n", AJ_MID);

   FntCol=0xFFFF; // white
   sprintf(midText,"VSynch(%s) Smoothing(%s) Interpolate UV(%s)\n",
        SynchScreen?"ON":"OFF",BlurDisplay?"ON":"OFF",
        InterpolateUV?"ON":"OFF" );
   //OutText16Mode("\n", AJ_MID);
   OutText16Mode(midText, AJ_MID);
   sprintf(midText,"Frame dropping(%s) Fit Screen(%s)\n",
     DropFrames?"ON":"OFF", FitVideo?"ON":"OFF" );
   OutText16Mode(midText, AJ_MID);
   OutText16ModeFormat(AJ_MID, midText, 255, "Audio %s%s%s\n",
        (AudioEnabled)?"ON":"OFF",
        (AudioEnabled)?": ":"",
        (AudioEnabled)?SndDrv->CardName:"");
    if (AudioEnabled) {
        OutText16ModeFormat(AJ_MID, midText, 255, "%sbits | %s | %iHz\n",
            (Audio16Bits) ? "16":"8",
            (AudioStereo) ? "STEREO":"MONO",
            AudioSamplingSpeed);
        if (AboutDebugInfo && pCodecAudioParam != NULL) {
            //char errV[256] = "";
            //av_strerror(audioFrameSamples, errV, 255);
            OutText16ModeFormat(AJ_MID, midText, 255, "pos %i fs %i af %i,channels %i,rate %i,f \n count %i start %i end %i queued %i add %i\n", curVoicePos, audioFrameSamples,
                pAudioCodecCtx->frame_size, pAudioCodecCtx->channels, pAudioCodecCtx->sample_rate, audioRingCount, audioRingStart, audioRingEnd, countRingQueued, countRingAdd);
        }
    }

}

void LoadConfig()
{
    char infoName[256]="";
    DFileBuffer *fileBuffer = CreateDFileBuffer(0);
    float tempFloat = 0.0f;
    int tempInt = 0;
    int vwidth = 0, vheight = 0, vbpp = 0, vrefreshRate = 0;
    DSplitString *ListInfoLine = CreateDSplitString(0, 0);
    DSplitString *ListInfoIndexCmnt = CreateDSplitString(0, 0);
    DSplitString *ListInfoIndex = CreateDSplitString(0, 0);
    bool videoModeFound = false;
    if (fileBuffer == NULL || ListInfoLine == NULL || ListInfoIndexCmnt == NULL || ListInfoIndex == NULL) {
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
            if ((ListInfoIndexCmnt->globLen = GetLineDFileBuffer(fileBuffer, ListInfoIndexCmnt->globStr, ListInfoIndexCmnt->maxGlobLength)) == 0 && IsEndOfFileDFileBuffer(fileBuffer)) break;
            TrimGlobStringDSplitString(ListInfoIndexCmnt);
            // remove any middle comment
            if (splitDSplitString(ListInfoIndexCmnt, NULL, ';', true) > 0) {
                TrimStringsDSplitString(ListInfoIndexCmnt);
                // empty ? ignore
                if (ListInfoIndexCmnt->ListStrings[0][0] == '\0')
                    break;
                if (splitDSplitString(ListInfoIndex, ListInfoIndexCmnt->ListStrings[0], ',', true) > 0) {
                    TrimStringsDSplitString(ListInfoIndex);

                    if(strcmp(infoName,"VideoMode") == 0 && ListInfoIndex->countStrings >= 2) {
                      screenX = atoi(ListInfoIndex->ListStrings[0]);
                      screenY = atoi(ListInfoIndex->ListStrings[1]);

                      DgGetFirstDisplayMode(&vwidth, &vheight, &vbpp, &vrefreshRate);
                      videoModeFound = (vwidth == screenX && vheight == screenY && vbpp == 16);
                      while (!videoModeFound && DgGetNextDisplayMode(&vwidth, &vheight, &vbpp, &vrefreshRate)) {
                        videoModeFound = (vwidth == screenX && vheight == screenY && vbpp == 16);
                      }
                      // video mode not found, switch to default 640x480
                      if (!videoModeFound) {
                        screenX = 640;
                        screenY = 480;
                      }
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
                      SynchScreen = StringToBool(ListInfoIndex->ListStrings[0]);
                    }
                    else if(strcmp(infoName,"DropFrames") == 0  && ListInfoIndex->countStrings >= 1) {
                      DropFrames = StringToBool(ListInfoIndex->ListStrings[0]);
                    }
                    else if(strcmp(infoName,"InterpolateUV") == 0  && ListInfoIndex->countStrings >= 1) {
                      InterpolateUV = StringToBool(ListInfoIndex->ListStrings[0]);
                    }
                    else if(strcmp(infoName,"FitScreen") == 0  && ListInfoIndex->countStrings >= 1) {
                      FitVideo = StringToBool(ListInfoIndex->ListStrings[0]);
                    }
                    else if(strcmp(infoName,"FullScrSmooth") == 0  && ListInfoIndex->countStrings >= 1) {
                      BlurDisplay = StringToBool(ListInfoIndex->ListStrings[0]);
                    }
                    else if(strcmp(infoName,"FullScrShowTime") == 0  && ListInfoIndex->countStrings >= 1) {
                      FullScrShowTime = StringToBool(ListInfoIndex->ListStrings[0]);
                    }
                    else if(strcmp(infoName,"FullScrShowFps") == 0  && ListInfoIndex->countStrings >= 1) {
                      FullScrShowFps = StringToBool(ListInfoIndex->ListStrings[0]);
                    }
                    else if(strcmp(infoName,"EnableSound") == 0  && ListInfoIndex->countStrings >= 1) {
                      SoundEnabled = StringToBool(ListInfoIndex->ListStrings[0]);
                    }
                    else if(strcmp(infoName,"SoundDriver") == 0  && ListInfoIndex->countStrings >= 1) {
                      if (IsFileExist(ListInfoIndex->ListStrings[0])) {
                        strcpy(soundDriverFileName, ListInfoIndex->ListStrings[0]);
                      }
                    }
                    else if(strcmp(infoName,"SoundSampling") == 0  && ListInfoIndex->countStrings >= 1) {
                      tempInt = atoi(ListInfoIndex->ListStrings[0]);
                      if (tempInt >= 8000 && tempInt <= 44100)
                        AudioSamplingSpeed = tempInt;
                    }
                    else if(strcmp(infoName,"VoiceSampleSize") == 0  && ListInfoIndex->countStrings >= 1) {
                      VoiceSampleSize = atoi(ListInfoIndex->ListStrings[0]);
                    }
                    else if(strcmp(infoName,"SoundStereo") == 0  && ListInfoIndex->countStrings >= 1) {
                      AudioStereo = StringToBool(ListInfoIndex->ListStrings[0]);
                    }
                    else if(strcmp(infoName,"Sound16Bits") == 0  && ListInfoIndex->countStrings >= 1) {
                      Audio16Bits = StringToBool(ListInfoIndex->ListStrings[0]);
                    }
                    else if(strcmp(infoName,"MasterSoundVolume") == 0  && ListInfoIndex->countStrings >= 1) {
                      tempFloat = atof(ListInfoIndex->ListStrings[0]);
                      if (tempFloat >= 0.0f && tempFloat <= 1.0f)
                        MasterAudioVolume = (int)(255.0f * tempFloat);
                    }
                    else if(strcmp(infoName,"VoiceSoundVolume") == 0  && ListInfoIndex->countStrings >= 1) {
                      tempFloat = atof(ListInfoIndex->ListStrings[0]);
                      if (tempFloat >= 0.0f && tempFloat <= 1.0f)
                        VoiceAudioVolume = (int)(255.0f * tempFloat);
                    }
                    else if(strcmp(infoName,"OutputGain") == 0  && ListInfoIndex->countStrings >= 1) {
                      tempFloat = atof(ListInfoIndex->ListStrings[0]);
                      if (tempFloat >= 0.0f && tempFloat <= 1.0f)
                        OutGainAudioVolume = (int)(255.0f * tempFloat);
                    }
                    else if(strcmp(infoName,"AboutDebugInfo") == 0  && ListInfoIndex->countStrings >= 1) {
                      AboutDebugInfo = StringToBool(ListInfoIndex->ListStrings[0]);
                    }
                }
            }
		}
	}
	CloseFileDFileBuffer(fileBuffer);
	DestroyDFileBuffer(fileBuffer);
	DestroyDSplitString(ListInfoLine);
	DestroyDSplitString(ListInfoIndexCmnt);
	DestroyDSplitString(ListInfoIndex);
}





///////////////////////////////////////


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
    if (VidOpen && kindVidOpened > 0) {
      char errBuff[128]="";
      av_make_error_string(errBuff, 127, FFFail);
      sprintf(InfImg.StrPtr,"%ix%i %3.1ffps %3.1f secs", Sframe16->ResH, Sframe16->ResV, VideoFps, VideoTime);
      fnsplit(CurVidFile.StrPtr, tdrv, tpath, tfile, text);
      sFinalLabel = sFinalLabel + '<' + tfile + text + ">" + InfImg;
      redrawVid = 1;
      FrameAvlbl = true;
      framenum = true;
      audioRingStart = 0;
      audioRingEnd = 0;
      audioRingCount = 0;
      curVoicePos = 0;
      countRingQueued = 0;
      countRingAdd = 0;
    }

    MWDPlayer->Label = sFinalLabel;
    MWDPlayer->Redraw();

    return retFF;
}
// return 1 if new frame found, 0 else
int GetNextFrame(DgSurf *S16, unsigned int nFramesToDrop)
{
    if (!VidOpen)
        return 0;

   switch(kindVidOpened) {
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

	av_log_set_level(AV_LOG_QUIET);

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
    videoStreamIndex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    VideoTime = (float)(pFormatCtx->duration)/AV_TIME_BASE;
    if (VideoTime < 0.0f)
        VideoTime = 0.0f;

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

    int64_t FrameToVideoTimeBase = ((int64_t(video_stream->time_base.num) * AV_TIME_BASE) / int64_t(video_stream->time_base.den)) * (video_stream->time_base.den / video_stream->avg_frame_rate.num);

    AVDictionaryEntry *rotate_tag = av_dict_get(video_stream->metadata, "rotate", NULL, 0);
    if (rotate_tag != NULL) {
        videoRotate180 = (atoi(rotate_tag->value) == 180);
    } else {
        videoRotate180 = false;
    }

    /* dump input information to stderr */
    av_dump_format(pFormatCtx, 0, FileName, 0);
    /* allocate frame */
    videoFrame = av_frame_alloc();
    audioFrame = av_frame_alloc();
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
    if(pAudioCodecParser == NULL) {
        audioFrameSamples = 555;
        audioStreamIndex = -1;
    }

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
    if (AudioEnabled) {
        // handle optional audio stream
        audioStreamIndex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
        if (audioStreamIndex >= 0) {
            av_dump_format(pFormatCtx, audioStreamIndex, NULL, false);
            // Find the decoder for the video stream
            pAudioCodec = avcodec_find_decoder(pFormatCtx->streams[audioStreamIndex]->codecpar->codec_id);
            if (pAudioCodec != NULL) {
                pAudioCodecCtx = avcodec_alloc_context3(pAudioCodec);
                pCodecAudioParam = pFormatCtx->streams[audioStreamIndex]->codecpar;
                audio_stream = pFormatCtx->streams[audioStreamIndex];
                /* Copy codec parameters from input stream to output codec context */
                if (avcodec_parameters_to_context(pAudioCodecCtx, audio_stream->codecpar) < 0) {
                    audioStreamIndex = -1;
                    audioFrameSamples = 444;
                }
                if (audioStreamIndex != -1) {
                    //av_dict_set(pAudioCodecCtx->priv_data, "packet_size", "256", 0);
                    //pAudioCodecCtx->request_sample_fmt = AV_SAMPLE_FMT_S16;
                    if (avcodec_open2(pAudioCodecCtx, pAudioCodec, NULL) < 0) {
                        audioStreamIndex = -1;
                        audioFrameSamples = 111;
                    } else {
                        pAudioCodecParser = av_parser_init(pAudioCodec->id);

                        int dst_linesize = 0;
                        uint64_t iInputLayout                    = av_get_default_channel_layout(pAudioCodecCtx->channels);
                        enum AVSampleFormat eInputSampleFormat   = (AVSampleFormat)pAudioCodecCtx->sample_fmt;
                        int         iInputSampleRate             = pAudioCodecCtx->sample_rate;

                        uint64_t iOutputLayout                   = (AudioStereo) ? AV_CH_LAYOUT_STEREO : AV_CH_LAYOUT_MONO;
                        enum AVSampleFormat eOutputSampleFormat  = (Audio16Bits) ? AV_SAMPLE_FMT_S16 : AV_SAMPLE_FMT_U8;
                        int         iOutputSampleRate            = pAudioCodecCtx->sample_rate;

                        au_convert_ctx= swr_alloc_set_opts(NULL,iOutputLayout, eOutputSampleFormat, iOutputSampleRate,
                            iInputLayout,eInputSampleFormat, iInputSampleRate, 0, NULL);

                        UpdateAudioRingDVoicesSamplingSpeed(pAudioCodecCtx->sample_rate);

                        swr_init(au_convert_ctx);
                        audioFrameSamples = 333;
                        SndDrv->ContinueSound();
                    }
                }

            } else {
                audioFrameSamples = 222;
                audioStreamIndex = -1;
            }
        }
    }

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

    // create Surf that will contain final frame
    if((retfunc = GetNextFrameFFMPEG(Sframe16,0))!=1) {
        CloseVidFFMPEG();
        return retfunc;
        //return 16;  // no frame
    }
    // Need for convert time to ffmpeg time.
    videoFramesCount = (VideoTime>0) ? ((int)(VideoFps*VideoTime)) : 1;

    framenum=0; // found one frame
    PosSynch=0;
    InitSynch(SynchBuff,&PosSynch,VideoFps);
    VidOpen=true; // opened video
    VidPause=0;
    VidEnded=false;
    frameskipped=0;
    FrameAvlbl = true;
    redrawVid=1;
    CurVidFile=FileName; // save current file
    TNM[6].Activ = 1; // enable menu close
    return 0;
}

// return 1 if new frame found, 0 else
int GetNextFrameFFMPEG(DgSurf *S16, unsigned int nFramesToDrop) {
    unsigned int nDrops     = nFramesToDrop;
    int ret_av = 0;
    int retfunc = 0;
    bool frameFound = false;
    DgSurf *Surf8bpp = NULL;
    if (!VidOpen || videoStreamIndex < 0 ) return 2;

    pVideoCodecCtx->skip_frame = AVDISCARD_NONKEY;
    while (nDrops > 0) {
        while (nDrops >0 && !frameFound && (ret_av = av_read_frame(pFormatCtx, pkt)) >= 0 ) {

            audioFrameSamples = pkt->stream_index;
            if (pkt->stream_index == videoStreamIndex) {

                if ((retfunc = avcodec_send_packet(pVideoCodecCtx, pkt)) < 0) {
                    av_packet_unref(pkt);
                    if (retfunc == AVERROR(EAGAIN)) {
                        if (nDrops>0) nDrops--;
                        continue;
                    }
                    VidEnded=true; // we reached the end
                    return 0; // no frame
                }
                frameFound = true;
            } else if (AudioEnabled && pkt->stream_index == audioStreamIndex) {
                audioFrameSamples = 0; //audioFrame->nb_samples;

                if ((retfunc = avcodec_send_packet(pAudioCodecCtx, pkt)) == 0) {
                    if (AudioEnabled) {
                        if (avcodec_receive_frame(pAudioCodecCtx, audioFrame) == 0) {
                            //int outframes = swr_convert(au_convert_ctx, (uint8_t **)&audioRing[audioRingStart]->Ptr, pCodecAudioParam->frame_size, (const uint8_t **) audioFrame->data, audioFrame->nb_samples);
                            //SndDrv->AddVoice(audioRing[audioRingStart], 0, 0, NULL, 0);
                            //audioRingStart = (audioRingStart+1)%AUDIO_RING_SIZE;

                            av_frame_unref(audioFrame);
                        }
                    }
                    av_packet_unref(pkt);
                    //av_samples_get_buffer_size()
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
        if (avcodec_receive_frame(pVideoCodecCtx, videoFrame)>=0)
            av_frame_unref(videoFrame);

        if (nDrops>0)
            nDrops--;
        frameFound = false;
    }

    // available frame ?
    pVideoCodecCtx->skip_frame = AVDISCARD_DEFAULT;
    while (1) {
        while (!frameFound && (ret_av = av_read_frame(pFormatCtx, pkt)) >= 0) {
            // submit the packet to the decoder
            // check if the packet belongs to a stream we are interested in, otherwise
            // skip it
            if (pkt->stream_index == videoStreamIndex) {

                if ((retfunc = avcodec_send_packet(pVideoCodecCtx, pkt)) < 0) {
                    av_packet_unref(pkt);
                    if (retfunc == AVERROR(EAGAIN))
                        continue;
                    VidEnded=true; // we reached the end
                    return 0; // no frame
                }
                // submit the packet to the decoder
                // new Video found it should contain a frame
                frameFound = true;
                av_packet_unref(pkt);
            } else if (AudioEnabled &&  pkt->stream_index == audioStreamIndex) {
                avcodec_send_packet(pAudioCodecCtx, pkt);
                while (avcodec_receive_frame(pAudioCodecCtx, audioFrame) == 0) {
                    static int typeToSampleSize[] = { 1, 2, 2, 4 };
                    int curVoiceOneSample = typeToSampleSize[audioRing[audioRingEnd]->Type];

                    uint8_t *PtrDstVoice =(uint8_t *) audioRing[audioRingEnd]->Ptr;
                    uint8_t *pDstVoiceData[1] = { (uint8_t *)&PtrDstVoice[curVoicePos] };
                    int outframes = swr_convert(au_convert_ctx,
                                                    pDstVoiceData,
                                                    VoiceSampleSize-curVoicePos/curVoiceOneSample,
                                                    (const uint8_t **) audioFrame->data,
                                                    (VoiceSampleSize-(curVoicePos/curVoiceOneSample) > audioFrame->nb_samples) ? audioFrame->nb_samples: (VoiceSampleSize-curVoicePos/curVoiceOneSample));
                    if (curVoicePos/curVoiceOneSample + outframes /*audioFrame->nb_samples*/ >= VoiceSampleSize) {
                        if (audioRingCount == 0) {
                            audioRingStart = audioRingEnd;
                            audioRingEnd = (audioRingEnd+1)%AUDIO_RING_SIZE;
                            audioRingCount = 1;
                        } else {
                            if (audioRingCount<AUDIO_RING_SIZE) {
                                audioRingCount++;
                                audioRingEnd = (audioRingEnd+1)%AUDIO_RING_SIZE;
                            } else { // overlapped increase both start and end
                                audioRingStart = (audioRingStart+1)%AUDIO_RING_SIZE;
                                audioRingEnd = (audioRingEnd+1)%AUDIO_RING_SIZE;
                            }
                        }
                        curVoicePos = (curVoicePos/curVoiceOneSample + outframes - VoiceSampleSize) * curVoiceOneSample;
                        // last chunk ?
                        if (curVoicePos > 0) {
                            uint8_t *pLastDstVoiceData[1] = { (uint8_t *)audioRing[audioRingEnd]->Ptr };
                            outframes = swr_convert(au_convert_ctx,
                                        pLastDstVoiceData,
                                        VoiceSampleSize,
                                        (const uint8_t **) audioFrame->data,
                                        curVoicePos/curVoiceOneSample);
                            curVoicePos = outframes*curVoiceOneSample;
                        }
                    } else {
                        curVoicePos += outframes*curVoiceOneSample; //audioFrame->nb_samples;
                    }

                    av_frame_unref(audioFrame);
                }

                av_packet_unref(pkt);
            } else {
                //audioFrameSamples = 666;
                av_packet_unref(pkt);
            }
        }
        // last read failure ?
        if (ret_av < 0 /*&& ret_av != AVERROR(EAGAIN)*/) {
            VidEnded = true;
            return 0;
        }
        //if ((framenum&1) > 0) return 1;
        if ((retfunc = avcodec_receive_frame(pVideoCodecCtx, videoFrame)>=0)) {
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
            frameFound = false;
            FrameAvlbl = true;
            return 1;
        } else {
            frameFound = false;
            av_packet_unref(pkt);
        }

    }

    //av_frame_unref(videoFrame);
    VidEnded=true; // we reached the end
    return 0;
}

int SeekFrameFFMPEG(DgSurf *S16, unsigned int FrameNum) {
    if (!VidOpen || videoStreamIndex < 0 )
        return 0;
    int64_t target_pts = (int64_t)(FrameNum * (1.0 / av_q2d(video_stream->avg_frame_rate)) / av_q2d(video_stream->time_base));
    av_seek_frame(pFormatCtx, videoStreamIndex, target_pts, AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(pVideoCodecCtx);
    return GetNextFrameFFMPEG(S16,0);
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

    if (pAudioCodecCtx) {
        avcodec_send_packet(pAudioCodecCtx, NULL);
        avcodec_free_context(&pAudioCodecCtx);
        pAudioCodecCtx = NULL;
    }

    if (audioFrame) {
        av_frame_free(&audioFrame);
        audioFrame = NULL;
    }
    if (au_convert_ctx != NULL) {
        swr_free(&au_convert_ctx);
        au_convert_ctx = NULL;
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
    audioStreamIndex = -1;
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


// AUDIO handling //////////////////////////////////////////////////


// load sound driver, init/detect sound card, reset audioRing buffer
bool InitSound(bool bits16, bool stereo, int sampleSpeed) {
    int iniSoundRes = false;

    if (!SoundEnabled || AudioEnabled)
        return false;
    AudioEnabled = false;
    memset(audioRing, 0, sizeof(audioRing));
    audioRingStart = 0;
    audioRingEnd = 0;
    audioRingCount = 0;

    // load the sound driver
    if (!LoadSoundDRV(&SndDrv,soundDriverFileName)) {
        return false; // load driver error
    }
    // alloc the memory buffer needed by the sound driver
	if ((SndBuff=malloc(SndDrv->SizeBuff))==NULL) {
        DestroySoundDRV(SndDrv);
        return false; // no mem
    }
    // try to install the sound driver -1 means AUTODETECT
	if (!SndDrv->InstallDriver(SndBuff,-1,5,1,-1)) {
        DestroySoundDRV(SndDrv);
        free(SndBuff);
        return false; // failure to detect sound card
	}
	// init sound output
    if (bits16)
        iniSoundRes = SndDrv->InitSound(DS_NOSOUND, DS_OUT16BIT, stereo, sampleSpeed);
    else // 8 bits
        iniSoundRes = SndDrv->InitSound(DS_OUT8BIT, DS_NOSOUND, stereo, sampleSpeed);

    if (!iniSoundRes) {
        DestroySoundDRV(SndDrv);
        free(SndBuff);
        return false; // failure to detect sound card
	}

    SndDrv->SetMasterVolume(MasterAudioVolume,MasterAudioVolume);
    SndDrv->SetVoiceVolume(VoiceAudioVolume,VoiceAudioVolume);
    SndDrv->SetOutGain(OutGainAudioVolume,OutGainAudioVolume);
    AudioEnabled = true;
    CreatePrepAudioRingDVoices();

    return true;
}

// uninstall sound driver, free/reset audio ring buffer
void CloseSound() {
    if (AudioEnabled) {
        SndDrv->StopSound();
        SndDrv->UninstallDriver();
        DestroyUnprepAudioRingDVoices();
        DestroySoundDRV(SndDrv);
        SndDrv = NULL;
        free(SndBuff);
        AudioEnabled = false;
    }
}

int AddVoice(DVoice *Vc,int State, bool updateSpeed)
{
    // adjust the speed of the voice if it's speed is inequal with
    // the current sampling speed
    if (updateSpeed && SndDrv->Cur_SampSpeed!=Vc->Freq) {
        VP.Speed=(128*Vc->Freq)/SndDrv->Cur_SampSpeed;
        return SndDrv->AddVoice(Vc,DS_EFF_CHG_SPEED,State,&VP,0);
	} else // else add as it*/
        return SndDrv->AddVoice(Vc,0,State,NULL,0);
}

int QueueVoice(DVoice *toQueueVc,DVoice *Vc,int State, bool updateSpeed)
{
    // adjust the speed of the voice if it's speed is inequal with
    // the current sampling speed
    if (updateSpeed && SndDrv->Cur_SampSpeed!=Vc->Freq) {
        VP.Speed=(128*Vc->Freq)/SndDrv->Cur_SampSpeed;
        return SndDrv->QueueVoice(toQueueVc,Vc,DS_EFF_CHG_SPEED,State,&VP,0);
	} else // else add as it*/
        return SndDrv->QueueVoice(toQueueVc,Vc,0,State,NULL,0);
}

// Create/Prepare Audio Ring DVoices
bool CreatePrepAudioRingDVoices() {
    int resCreate = 0;
    int i=0;

    audioRingStart = 0;
    audioRingEnd = 0;
    audioRingCount = 0;
    if (!AudioEnabled)
        return false;
    for (i=0; i < AUDIO_RING_SIZE; i++) {
        resCreate = CreateDVoice(&audioRing[i], Audio16Bits, AudioStereo, AudioSamplingSpeed/*pAudioCodecCtx->sample_rate*/, VoiceSampleSize);
        if (!resCreate)
            break;
    }
    if (!resCreate) {
        for (i=0; i < AUDIO_RING_SIZE; i++) {
            if (audioRing[i] != NULL)
                DestroyDVoice(audioRing[i]);
        }
        return false;
    }
    for (i=0; i < AUDIO_RING_SIZE; i++) {
        SndDrv->PrepareVoice(audioRing[i]);
    }

    return true;
}

// update sampling speed of AudioRingDVoice
void UpdateAudioRingDVoicesSamplingSpeed(int newSampling) {
    if (AudioEnabled && newSampling>4000) {
        for (int i=0; i < AUDIO_RING_SIZE; i++) {
            if (audioRing[i] != NULL)
                audioRing[i]->Freq = newSampling;
        }
    }
}

// Destroy/Unprepare Audio Ring DVoices
bool DestroyUnprepAudioRingDVoices() {
    int i=0;

    if (!AudioEnabled)
        return false;
    SndDrv->DeleteAllVoices();
    for (i=0; i < AUDIO_RING_SIZE; i++) {
        if (audioRing[i] != NULL) {
            SndDrv->UnprepareVoice(audioRing[i]);
            DestroyDVoice(audioRing[i]);
            audioRing[i] = NULL;
        }
    }
    audioRingStart = 0;
    audioRingEnd = 0;
    audioRingCount = 0;

    return true;
}

