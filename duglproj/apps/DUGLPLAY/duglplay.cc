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
    13 April 2026: 1.0 alpha 4
    - Implement decoding for audio only files
    - Add master volume control widget
    - Implement the three usual time progress modes (progress, remaining time, progress / total time)
      user can switch between modes by mouse clicking on the time
    - Implement seeking of audio track with video track
    - Improve playing mode widget, now image Button (looping or single play)
    - severals parameters added to config file
    - Improved sound quality, bug fixes
    25 April 2026: 1.0 alpha 5
    - Add/implement new button to select playing speed with ratio 1/2, 1(default), 2, 4
    - Implement audio only files fast forward/rewind with slider
    - Improve GUI design with progress slider at full window width and increasing precision to 1000 (was 100)
    - Add possibility display audio curve for audio only files, with additional config file fields
      (enable; background color; curve color; render mode: blit|transluent)
    - Revert time display to hh:mm:ss instead of xxhxxmxxs
    - Improve Open Dialog File filters
*/

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dir.h>
#include <math.h>

#include <DUGL.h>
#include <DUGLPLUS.H>

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
static int cptLog = 0;

#define FLOG(formatMsg, ...) { \
    FILE *LOGFILE = fopen("./log.txt", "at");\
    if (LOGFILE!=NULL) {\
        fprintf(LOGFILE, formatMsg, __VA_ARGS__);\
        fclose(LOGFILE);\
        cptLog++; \
    }\
}

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
    VidAudioEnded=false,
    VidVideoEnded=false,
    FrameAvlbl=false,
    closeOnVidEnded=false,
    videoRotate180 = false,
    videoFlipHorizontally = false,
    videoTransluent = false,
    VidOpenHasVideo = false,
    VidOpenHasAudio = false,
    AudioEnabled = false,
    SoundEnabled = true,
    PlayLooping = false,
    UseOldFFMPEGResampler = true,
    DisplaySoundCurve = true;
int ProgressTimeMode = 2;
float VideoFps = 0.0f;
float VideoTime = 0.0f;
float AudioTime = 0.0f;
unsigned int sizeVidFile = 0, readVidFileBytes = 0;
int videoWidth = 0, videoHeight = 0;
int videoFramesCount = 0;
int framenum,
    PosSynch,
    frameskipped=0;
int DefTypeOpen=0;
int kindVidOpened = 0; // 0 : none, 4: FFMPEG
int CountSpeedID = 4;
int CurrentSpeedID = 1;
float CurrentSpeedCoef = 1.0f;
int DisplaySoundBackCol = RGB16(0,0,0),
    DisplaySoundCurveCol = RGB16(255,255,255),
    DisplaySoundMode = 0,
    DisplaySoundModeTransLevel = 10;

DgSurf *Sframe16 = NULL, *Slastframe16= NULL; // Surf where the 16bpp video frame will be stored

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
AVFormatContext* pAudioFormatCtx = NULL;
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
struct SwrContext *au_convert_ctx = NULL;

static uint8_t *video_dst_data[4] = {NULL};
static int      video_dst_linesize[4];
static AVFrame  *videoFrame = NULL;
static AVPacket *pkt = NULL;
static uint8_t **audio_dst_data = {NULL};
static int      audio_dst_linesize;
static int      max_dst_nb_samples = 0;
static AVFrame  *audioFrame = NULL;
static AVPacket *pktAudio = NULL;
int FFZone = 0, FFFail = 0;

// audio handling
char soundDriverFileName[256] = "sb16.drv";
//#define AUDIO_RING_SIZE 16
DVoice **audioRing = NULL;
int AUDIO_RING_SIZE = 6;
int MaxVoicesRingCount = 2;
int audioRingStart = 0;
int audioRingCount = 0;
int audioFrameSamples = 0;
int audioLastAddIdx = -1;
int audioLastQueueIdx = -1;
int audioLastCurveIdx = -1;
int countRingQueued = 0;
int countRingAdd = 0;
int countRingOverWritten = 0;
int curVoicePos = 0;
int iInputSampleRate = 0;
int iOutputSampleRate = 0;

SoundDRV *SndDrv = NULL;
void *SndBuff = NULL;
int retOpenAudio = 0;
bool Audio16Bits = false,
     AudioStereo = false,
     AudioMuted = false,
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
int QueueVoice(DVoice *toQueueVc,DVoice *Vc,int State, bool updateSpeed, bool replaceExisting);

// Create/Prepare Audio Ring DVoices
bool CreatePrepAudioRingDVoices();
// update sampling speed of AudioRingDVoice
void UpdateAudioRingDVoicesSamplingSpeed(int newSampling);
// Destroy/Unprepare Audio Ring DVoices
bool DestroyUnprepAudioRingDVoices();
// render last audio
void RenderAudioLastAdd();


// return 0 if success, code error if failed
int OpenFFMPEG(char *FileName);
int OpenVideoFFMPEG(char *FileName);
int OpenAudioFFMPEG(char *FileName);
// return 1 if new frame found, 0 else
int GetNextFrameFFMPEG(DgSurf *S16, unsigned int nFramesToDrop);
// return 1 if new frame found, 0 else
int GetNextAudioFrameFFMPEG();
// try to seek to FrameNum return 1 successfull, 0 else
int SeekFrameFFMPEG(DgSurf *S16, unsigned int FrameNum);
// try to seek audio stream to targetTimeSeconds return 1 successfull, 0 else
int SeekAudioFFMPEG(float targetTimeSeconds);
// free any memory ressources allocated by OpenFFMPEG
void DestroyFFMPEG();
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
String sMainWinName("DUGL Player 1.0 alpha5");
MainWin *MWDPlayer;
GraphBox *GphBVideo;
Menu *MWMn;
ImgButton *BtPlay,*BtPauseCont,*BtPlaySpeed, *BtPlayMode,*BtExit;
GraphBox *WidgetAudioVolume;
Label *LbTime;
HzSlider *HSldAdv;

DgSurf *ImgPlay,*ImgExit, *ImgLoop, *ImgPlayOnce, *ImgPCont;

DgSurf *TImgsSpeed[4] = { NULL, NULL, NULL, NULL };

// glabal var
int redrawVid=1;
int ignoreSeekSliderChange = 0;

char playTime[128];
// events
void OnMenuOpenVid(),OnMenuCloseVid(),OnMenuExit(),OnMenuFullScr(),
     OnMenuPauseCont(),OnMenuAbout();
void OnMenuVSynch(),OnMenuSmoothFS(),OnMenuFSFps(),OnMenuFSTime();
void OnMenuLoop(),OnMenuFrameDrop();
void OnMenuInterpolateUV(), OnMenuFitScreen();

void GphBDrawVideo(GraphBox *Me),GphBScanVideo(GraphBox *Me);
void GphBDrawVolume(GraphBox *Me);
void OnMsDownGphBVolume(int x, int y);
void OnMsDragGphBVolume(int x, int y, bool MsIn);

void OnBtPlayClick();
void OnBtPlaySpeedClick();
void OnBtPlayModeClick();
void OnMsClickSwitchProgressTimeMode();
void OnSeekSliderChange(int val);

// screen shot file name
char *scrFileName="DUGLPLYR.BMP";
// file filer string
char *TSFBName[]={ "All supported Files",
     "All Supported Video", "All Supported Audio", "All Files(*.*)" };
char *TSFBMask[]={ "*.mpg|*.mpeg|*.m2v|*.m1v|*.mpe|*.mpv|*.dat|*.ogv|*.ogg|*.y4m|*.mp4|*.avi|*.mov|*.wmv|*.flv|*.webm|*.3gp|*.vob|*.jpg|*.jpeg|*.gif|*.bmp|*.png|*.oga|*.wav|*.mp3|*.mp2|*.aac|*.wma|*.m4a|*.flac",
     "*.mpg|*.mpeg|*.m2v|*.m1v|*.mpe|*.mpv|*.dat|*.ogv|*.ogg|*.y4m|*.mp4|*.avi|*.mov|*.wmv|*.flv|*.webm|*.3gp|*.vob", "*.ogg|*.oga|*.wav|*.mp3|*.mp2|*.aac|*.wma|*.m4a|*.flac", "*.*" };
ListString LSMpgName(4,TSFBName),LSMpgMask(4,TSFBMask);

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
// return 1 if new audio frame found, 0 else
int GetNextAudioFrame();
// close an opened video
void CloseVid();
// utils
void DGWaitRetrace();
void UpdatePlayTime();
bool IsFileExist(const char *fname);
bool StringToBool(char *str);
void LoadConfig();
void timeToStr(float timeInSec, char *outStr, size_t outStrSize);

int main (int argc, char ** argv) {
    if (!DgInit()) {
        printf("DUGL init error\n");
        exit(-1);
    }
    LoadConfig();

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
    if (LoadBMP16(&ImgPlayOnce,"gfx/once.bmp")==0) {
      printf("Error loading once.bmp\n"); exit(-1);
    }
    if (LoadBMP16(&ImgLoop,"gfx/loop.bmp")==0) {
      printf("Error loading loop.bmp\n"); exit(-1);
    }
    if (LoadBMP16(&TImgsSpeed[0],"gfx/speed12.bmp")==0) {
      printf("Error loading loop.bmp\n"); exit(-1);
    }
    if (LoadBMP16(&TImgsSpeed[1],"gfx/speed.bmp")==0) {
      printf("Error loading loop.bmp\n"); exit(-1);
    }
    if (LoadBMP16(&TImgsSpeed[2],"gfx/speed2.bmp")==0) {
      printf("Error loading loop.bmp\n"); exit(-1);
    }
    if (LoadBMP16(&TImgsSpeed[3],"gfx/speed4.bmp")==0) {
      printf("Error loading loop.bmp\n"); exit(-1);
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
    if (!InstallKeyboard()) {
       DgQuit(); DgUninstallTimer();
       printf("Keyboard error\n");  exit(-1);
    }

    if (!SetKbMAP(KM)) {
       DgUninstallTimer(); UninstallKeyboard(); DgQuit();
       printf("Error setting keyborad map\n");  exit(-1);
    }
    MouseSupported = (InstallMouse()!=0);

    // sound
    InitSound(Audio16Bits, AudioStereo, AudioSamplingSpeed);

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
        printf ("error opening video/audio file: \"%s\".\n", argv[1]);
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
    GphBVideo= new GraphBox(2,30+14,screenX-10,screenY-50,MWDPlayer,WH->m_GraphCtxt->WinGris);
    // set drawing handler
    GphBVideo->GraphBoxDraw=GphBDrawVideo;
    // set scan handler (enable redraw when needed)
    GphBVideo->ScanGraphBox=GphBScanVideo;
    GphBVideo->Redraw();
    // buttons
    BtPlay=new ImgButton(2,3+15,29,27+15,MWDPlayer,ImgPlay);
    BtPlay->Click=OnBtPlayClick; // set click handler
    BtPauseCont=new ImgButton(32,3+15,57,27+15,MWDPlayer,ImgPCont);
    BtPauseCont->Click=OnMenuPauseCont; // set click handler
    BtPlaySpeed = new ImgButton(60,3+15,85,27+15,MWDPlayer,TImgsSpeed[CurrentSpeedID]);
    BtPlaySpeed->Click = OnBtPlaySpeedClick;
    BtPlayMode=new ImgButton(60+28,3+15,85+28,27+15,MWDPlayer,(!PlayLooping)?ImgPlayOnce:ImgLoop);
    BtPlayMode->Click= OnBtPlayModeClick;
    LbTime=new Label(screenX-238,5+13,screenX-82,25+13,MWDPlayer,"",AJ_RIGHT);
    LbTime->OnMsClick = OnMsClickSwitchProgressTimeMode;
    WidgetAudioVolume = new GraphBox(screenX-81,19,screenX-38,41,MWDPlayer, WH->m_GraphCtxt->WinGris);
    WidgetAudioVolume->GraphBoxDraw = GphBDrawVolume;
    WidgetAudioVolume->OnMsDown = OnMsDownGphBVolume;
    WidgetAudioVolume->OnMsDrag = OnMsDragGphBVolume;
    WidgetAudioVolume->Redraw();
    BtExit=new ImgButton(screenX-36,3+15,screenX-10,27+15,MWDPlayer,ImgExit);
    BtExit->Click=OnMenuExit; // set click handler
    HSldAdv=new HzSlider(2,screenX-8,0,MWDPlayer,0,1000);
    HSldAdv->Changed = OnSeekSliderChange;
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
           if (VidOpenHasAudio) {
              while (audioRingCount < MaxVoicesRingCount && !VidAudioEnded)
                  GetNextAudioFrame();

              // update of the state of playing/queued voice
              if (audioLastAddIdx != -1 && !SndDrv->IsPlayingVoice(audioRing[audioLastAddIdx])) {
                audioLastAddIdx = -1;
              }
              if (audioLastQueueIdx != -1) {
                  if (!SndDrv->IsQueuedVoice(audioRing[audioLastQueueIdx])) {
                    if (SndDrv->IsPlayingVoice(audioRing[audioLastQueueIdx])) {
                        audioLastAddIdx = audioLastQueueIdx;
                        audioLastQueueIdx = -1;
                    } else {
                        audioLastQueueIdx = -1;
                    }
                  }
              }

              if (audioRingCount > 0) {
                if (audioLastAddIdx == -1) {
                    AddVoice(audioRing[audioRingStart], 0, true);
                    audioLastAddIdx = audioRingStart;
                    audioLastQueueIdx = -1;
                    countRingAdd++;
                    audioRingCount--;
                    audioRingStart=(audioRingStart+1)%AUDIO_RING_SIZE;
                }
                if (audioRingCount > 0 && audioLastAddIdx != -1) {
                    if (QueueVoice(audioRing[audioLastAddIdx],audioRing[audioRingStart], 0, true, false)) {
                        audioLastQueueIdx = audioRingStart;
                        countRingQueued++;
                        audioRingCount--;
                        audioRingStart=(audioRingStart+1)%AUDIO_RING_SIZE;
                        if (!VidOpenHasVideo) {
                            UpdatePlayTime();
                        }
                    }
                }
              }
              // handle curve display
              if (DisplaySoundCurve && !VidOpenHasVideo) {
                 bool renderCurve = false;
                 if (audioLastCurveIdx == -1) {
                    if (audioLastAddIdx != -1) {
                        audioLastCurveIdx = audioLastAddIdx;
                        renderCurve = true;
                    }
                 } else if (audioLastAddIdx != -1 && audioLastCurveIdx != audioLastAddIdx) {
                    audioLastCurveIdx = audioLastAddIdx;
                    renderCurve = true;
                 }
                 if (renderCurve) {
                    RenderAudioLastAdd();
                    redrawVid=1;
                 }
              }
           }

          if (VidOpenHasVideo && !VidVideoEnded) {
              if (PosSynch!=framenum) {
                 if (DropFrames) {
                    if (GetNextFrame(Sframe16, (PosSynch > (framenum))  ? (PosSynch-framenum-1) : 0)) {
                       redrawVid=1;
                       if (!FrameAvlbl) FrameAvlbl=true;
                    }
                    else {
                       // enable GUI if full screen, and if we are not looping
                       if (EnableGUI==0 && (!PlayLooping)) {
                          OnMenuFullScr();
                       }
                    }
                 } else {
                    if (GetNextFrame(Sframe16,0)==1) {
                       redrawVid=1;

                       if (!FrameAvlbl) FrameAvlbl=true;
                       frameskipped+=PosSynch-framenum-1; // we are too slow ? :(
                    } else {
                       // enable GUI if full screen, and if we are not looping
                       if (EnableGUI==0 && (!PlayLooping)) {
                          OnMenuFullScr();
                       }
                    }
                 }

                 framenum=PosSynch;
                 UpdatePlayTime();
              }
            }
        }
    }

    VidEnded = VidAudioEnded && VidVideoEnded;
      // loop ?
      if (VidEnded) {
        if (VidOpen) {
            if(PlayLooping) {
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
           //redrawVid=1;
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
            case KB_KEY_F9 : OnBtPlayModeClick(); break; // F9
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

        if (keyCode==KB_KEY_F9) OnBtPlayModeClick();
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
  unsigned int iplayTime=0,videoAdv=0;
  unsigned int iremainTime=0;
  unsigned int itotalTime=0;
  char str_progressT[32]="";
  char str_remainT[32]="";

  videoAdv=0;
  if (VidOpen) {
    if (VidOpenHasVideo) {
        iplayTime=(unsigned int)(float(framenum-frameskipped)/VideoFps);
        if (videoFramesCount>0) {
          videoAdv=(unsigned int)((double)(framenum-frameskipped)*1000.0 / (double)(videoFramesCount));
        } else if(sizeVidFile>0) {
          videoAdv=(unsigned int)((((double)readVidFileBytes)*1000.0) / (double)(sizeVidFile));
        }
        else
          videoAdv=0;
        iremainTime = (unsigned int)(VideoTime)-iplayTime;
        itotalTime = (unsigned int)(VideoTime);
    } else if (VidOpenHasAudio) {
        iplayTime = (int)(((double)(VoiceSampleSize) * (double)(countRingQueued + countRingAdd + countRingOverWritten)) /(double)iOutputSampleRate);

        videoAdv = ((double)(iplayTime) * 1000.0) / (double)(AudioTime);
        iremainTime = (unsigned int)(AudioTime)-iplayTime;
        itotalTime = (unsigned int)(AudioTime);
    }
    switch (ProgressTimeMode) {
        case 0: // simple progress
            timeToStr(iplayTime, playTime, 127);
            break;
        case 1: // remaining
            timeToStr(iremainTime, str_remainT, 31);
            sprintf(playTime,"-%s",str_remainT);
            break;
        case 2: // progress/total
            timeToStr(iplayTime, str_progressT, 31);
            timeToStr(itotalTime, str_remainT, 31);
            sprintf(playTime,"%s/%s",str_progressT,str_remainT);
            break;
    }
  }
  else {
    playTime[0]='\0';
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

// GphBVideo events

void GphBDrawVideo(GraphBox *Me) {

   // opened video ?
   if (VidOpen) {
      if(FitVideo || !VidOpenHasVideo)
      {
         ResizeViewSurf16(Sframe16, 0, 0);
         return;
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

// WidgetAudioVolume events
void GphBDrawVolume(GraphBox *Me) {
    ClearSurf16(WH->m_GraphCtxt->WinGris);//RGB16(230,231,236));
    if (!SoundEnabled)
        return;
    int viewWidth = Me->VGraphBox.MaxX - Me->VGraphBox.MinX - 6;
    int viewHeight = Me->VGraphBox.MaxY - Me->VGraphBox.MinY - 6;
    if (viewWidth < 10 || viewHeight < 10)
        return;
    int posVolX = ((MasterAudioVolume * viewWidth) / 255) + 3 + Me->VGraphBox.MinX;
    int posVolY = Me->VGraphBox.MaxY - 1;

    int ListPtsRGB[18] = {
        // X     ,    Y,                                              Z,      U,      V,  COL
        3 + Me->VGraphBox.MinX, Me->VGraphBox.MinY + 2,               0,      0,      0,  RGB16(230,231,236),
        3 + viewWidth + Me->VGraphBox.MinX, Me->VGraphBox.MinY + 2,   0,      0,      0,  RGB16(255,0,0),
        3 + viewWidth + Me->VGraphBox.MinX, Me->VGraphBox.MaxY - 6,   0,      0,      0,  RGB16(255,0,0)  };
    int PolyListPtRGB[4] = { 3, (int)&ListPtsRGB[0], (int)&ListPtsRGB[6], (int)&ListPtsRGB[12]};
    int ListPts[6] = {
        // X     ,    Y,
        posVolX-3, posVolY,
        posVolX+3, posVolY,
        posVolX, posVolY - 3};
    int PolyListPt[4] = { 3, (int)&ListPts[0], (int)&ListPts[2], (int)&ListPts[4]};

    Poly16(&PolyListPtRGB, NULL, POLY16_RGB|POLY16_FLAG_DBL_SIDED, RGB16(0,0,0));
    Poly16(&PolyListPt, NULL, POLY16_SOLID|POLY16_FLAG_DBL_SIDED, RGB16(0,0,0));
}

void OnMsDownGphBVolume(int x, int y) {
    if (!SoundEnabled)
        return;
    int viewWidth = WidgetAudioVolume->VGraphBox.MaxX - WidgetAudioVolume->VGraphBox.MinX - 6;

    if (x<=3) // set volume to min
        MasterAudioVolume = 0;
    else if (x>=viewWidth+3) // set volume to min
        MasterAudioVolume = 255;
    else {
        MasterAudioVolume = ((x - 3) * 255) / viewWidth;
    }
    SndDrv->SetMasterVolume(MasterAudioVolume, MasterAudioVolume);
    WidgetAudioVolume->Redraw();
}

void OnMsDragGphBVolume(int x, int y, bool MsIn) {
    if (!SoundEnabled)
        return;
    if (MsIn)
        OnMsDownGphBVolume(x, y);
}



void OnBtPlayClick() {

  OpenVid(CurVidFile.StrPtr);
}

void OnBtPlaySpeedClick() {
    int OldPosSynch = 0;
    CurrentSpeedID = (CurrentSpeedID+1) % CountSpeedID;
    BtPlaySpeed->SetImage(TImgsSpeed[CurrentSpeedID]);
    switch (CurrentSpeedID) {
    case 0:
        CurrentSpeedCoef = 0.5f;
        break;
    case 1:
        CurrentSpeedCoef = 1.0f;
        break;
    case 2:
        CurrentSpeedCoef = 2.0f;
        break;
    case 3:
        CurrentSpeedCoef = 4.0f;
        break;
    }
    if (VidOpen && VidOpenHasVideo) {
        OldPosSynch = PosSynch;
        InitSynch(SynchBuff,&PosSynch,VideoFps*CurrentSpeedCoef);
        PosSynch = OldPosSynch;
    }
}

void OnBtPlayModeClick() {
    PlayLooping = !PlayLooping;
    BtPlayMode->SetImage((!PlayLooping)?ImgPlayOnce:ImgLoop);
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

void OnMenuFitScreen() {
  FitVideo = !FitVideo; redrawVid=1;
}

void OnMenuInterpolateUV() {
    InterpolateUV=!InterpolateUV;
}

void OnMenuLoop() {
    OnBtPlayModeClick();
}

void OnMenuFrameDrop() {
    DropFrames = !DropFrames;
}

void OnMsClickSwitchProgressTimeMode() {
    ProgressTimeMode = (ProgressTimeMode+1)%3;
    UpdatePlayTime();
}

void OnSeekSliderChange(int val) {
    unsigned int targetFrame = 0;
    float targetSecond = 0.0;
    if (!VidOpen) {
        HSldAdv->SetVal(0);
        return;
    }
    if (ignoreSeekSliderChange == 1)
        return;
    switch(kindVidOpened) {
        case 4:
            if (VidOpenHasVideo) {
                targetFrame = val * videoFramesCount / 1000;
                if (SeekFrameFFMPEG(Sframe16, targetFrame) == 1) {
                    //VidPause = true;
                    framenum = targetFrame;
                    frameskipped = 0;
                    redrawVid = 1;
                    UpdatePlayTime();
                } else { // fail, restore slider position
                    UpdatePlayTime();
                }
            } else if (VidOpenHasAudio) {
                targetSecond = (AudioTime * (float)(val)) / 1000.0f;
                if (SeekAudioFFMPEG(targetSecond) == 1) {
                    countRingQueued = (targetSecond * (float)iOutputSampleRate) / (double)(VoiceSampleSize);
                    countRingAdd = 1;
                    countRingQueued--;
                    UpdatePlayTime();
                } else { // fail, restore slider position
                    UpdatePlayTime();
                }
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
   OutText16Mode("DUGL Player 1.0 Alpha5 - DOS Audio/Video Player\n", AJ_MID);
   FntCol=RGB16(255,255,255); // white
   OutText16Mode("(C) By FFK 25 April 2026\n\n", AJ_MID);
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
        if (AboutDebugInfo) {
            //OutText16ModeFormat(AJ_MID, midText, 255, "video %i audio %i/ret %i\n", VidOpenHasVideo, VidOpenHasAudio,retOpenAudio);
            //char errV[256] = "";
            //av_strerror(audioFrameSamples, errV, 255);
            if (pAudioCodecCtx != NULL) {
                OutText16ModeFormat(AJ_MID, midText, 255, "pos %i fs %i af %i,channels %i,rate %i,f \n count %i start %i queued %i add %i C %i %i/%i", curVoicePos, audioFrameSamples,
                                    pAudioCodecCtx->frame_size, pAudioCodecCtx->ch_layout.nb_channels, pAudioCodecCtx->sample_rate, audioRingCount, audioRingStart, countRingQueued, countRingAdd, SndDrv->GetCountVoices(), audioLastAddIdx, audioLastQueueIdx);
                if (audioLastAddIdx != -1 && SndDrv->IsPlayingVoice(audioRing[audioLastAddIdx]))
                    OutText16("P-|>");

            } else {
                OutText16ModeFormat(AJ_MID, midText, 255, "NO AUDIO STREAM\n");
            }
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
                    else if(strcmp(infoName,"ProgressTimeMode") == 0  && ListInfoIndex->countStrings >= 1) {
                      tempInt = atoi(ListInfoIndex->ListStrings[0]);
                      if (tempInt >= 0 && tempInt <= 2)
                        ProgressTimeMode = tempInt;
                      else
                        ProgressTimeMode = 0;
                    }
                    else if(strcmp(infoName,"EnableSound") == 0  && ListInfoIndex->countStrings >= 1) {
                      SoundEnabled = StringToBool(ListInfoIndex->ListStrings[0]);
                    }
                    else if(strcmp(infoName,"UseOldFFMPEGResampler") == 0  && ListInfoIndex->countStrings >= 1) {
                      UseOldFFMPEGResampler = StringToBool(ListInfoIndex->ListStrings[0]);
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
                    else if(strcmp(infoName,"VoicesRingSize") == 0  && ListInfoIndex->countStrings >= 1) {
                      tempInt = atoi(ListInfoIndex->ListStrings[0]);
                      if (tempInt >= 3)
                        AUDIO_RING_SIZE = tempInt;
                    }
                    else if(strcmp(infoName,"MaxVoicesRingCount") == 0  && ListInfoIndex->countStrings >= 1) {
                      tempInt = atoi(ListInfoIndex->ListStrings[0]);
                      if (tempInt >= 1 && tempInt < AUDIO_RING_SIZE)
                        MaxVoicesRingCount = tempInt;
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
                    else if(strcmp(infoName,"DisplaySoundCurve") == 0  && ListInfoIndex->countStrings >= 1) {
                      DisplaySoundCurve = StringToBool(ListInfoIndex->ListStrings[0]);
                    }
                    else if(strcmp(infoName,"DisplaySoundBackCol") == 0  && ListInfoIndex->countStrings >= 3) {
                      DisplaySoundBackCol = RGB16(atoi(ListInfoIndex->ListStrings[0])&0xff,
                                      atoi(ListInfoIndex->ListStrings[1])&0xff,
                                      atoi(ListInfoIndex->ListStrings[2])&0xff);

                    }
                    else if(strcmp(infoName,"DisplaySoundCurveCol") == 0  && ListInfoIndex->countStrings >= 3) {
                      DisplaySoundCurveCol = RGB16(atoi(ListInfoIndex->ListStrings[0])&0xff,
                                      atoi(ListInfoIndex->ListStrings[1])&0xff,
                                      atoi(ListInfoIndex->ListStrings[2])&0xff);
                    }
                    else if(strcmp(infoName,"DisplaySoundMode") == 0  && ListInfoIndex->countStrings >= 2) {
                      tempInt = atoi(ListInfoIndex->ListStrings[0]);
                      if (tempInt >= 0 && tempInt <= 1) // accept only 0 or 1
                        DisplaySoundMode = tempInt;
                      tempInt = atoi(ListInfoIndex->ListStrings[1]);
                      if (tempInt >= 0 && tempInt <= 31) // [ 0, 31 ] transparency
                        DisplaySoundModeTransLevel = tempInt;
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
    char strTime[256]="";

    int    OpenVidWidth = 0;
    int    OpenVidHeight = 0;
    char   tdrv[MAXDRIVE], tpath[MAXDIR], tfile[MAXFILE], text[MAXEXT];
    String sFinalLabel = sMainWinName;

    CloseVid();
    audioRingStart = 0;
    audioRingCount = 0;
    curVoicePos = 0;


    if((retFF=OpenFFMPEG(myFile.StrPtr)) == 0)
       kindVidOpened = 4;
    if (kindVidOpened > 0) {
      if (!VidOpenHasVideo) {
         if (CreateSurf(&Sframe16, 500, 500, 16)) {
            SetOrgSurf(Sframe16, 0, Sframe16->ResV/2);
            DgSetCurSurf(Sframe16);
            ClearSurf16(DisplaySoundBackCol);
         }

         if (CreateSurf(&Slastframe16, 500, 500, 16)) {
            SetOrgSurf(Slastframe16, 0, Slastframe16->ResV/2);
            DgSetCurSurf(Slastframe16);
            ClearSurf16(DisplaySoundBackCol);
         }
      }
    }

    if (VidOpen && kindVidOpened > 0) {
      if (VidOpenHasVideo) {
        timeToStr(VideoTime, strTime, 255);
        sprintf(InfImg.StrPtr,"%ix%i||%3.1ffps||%s", Sframe16->ResH, Sframe16->ResV, VideoFps, strTime);
      } else {
        timeToStr(AudioTime, strTime, 255);
        sprintf(InfImg.StrPtr,"%iHz||%iCh||%s", pAudioCodecCtx->sample_rate, pAudioCodecCtx->ch_layout.nb_channels, strTime);
      }
      fnsplit(CurVidFile.StrPtr, tdrv, tpath, tfile, text);
      sFinalLabel = sFinalLabel + '<' + tfile + text + ">" + InfImg;
      redrawVid = 1;
      FrameAvlbl = true;
      framenum = true;
      countRingQueued = 0;
      countRingAdd = 0;
      countRingOverWritten = 0;
      MWDPlayer->Label = sFinalLabel;
      MWDPlayer->Redraw();
    }

    return retFF;
}
// return 1 if new frame found, 0 else
int GetNextFrame(DgSurf *S16, unsigned int nFramesToDrop)
{
    if (!VidOpen || !VidOpenHasVideo)
        return 0;

   switch(kindVidOpened) {
     case 4:
       return GetNextFrameFFMPEG(S16, nFramesToDrop);
     default:
       return 0;
   }
}

// return 1 if new audio frame found, 0 else
int GetNextAudioFrame()
{
    if (!VidOpen || !VidOpenHasAudio)
        return 0;

    retOpenAudio = 22222;
   switch(kindVidOpened) {
     case 4:
       retOpenAudio = 33333;
       return GetNextAudioFrameFFMPEG();
     default:
       return 0;
   }
}

// close an opened video
void CloseVid() {
  if (VidOpen && kindVidOpened > 0) {
    MWDPlayer->Label = sMainWinName;
    MWDPlayer->Redraw();
    if (AudioEnabled) {
        while (SndDrv->GetCountVoices() > 0);
    }
    switch(kindVidOpened) {
      case 4:
        DestroyFFMPEG();
        if (pkt) {
            av_packet_free(&pkt);
            pkt = NULL;
        }
        break;
    }

    kindVidOpened = 0;
    VideoFps = 0.0f;
    VideoTime = 0.0f;
    sizeVidFile = 0;
    readVidFileBytes = 0;
    videoWidth = 0;
    videoHeight = 0;
    videoFramesCount = 0;
    audioLastCurveIdx = -1;
    framenum = 0;
    VidOpen = false;
    VidOpenHasAudio = false;
    VidOpenHasVideo = false;
    UpdatePlayTime();
  }
}
// FFMPEG /////////////////////////////


void DestroyVideoFFMPEG();
void DestroyAudioFFMPEG();
// return 0 if success, code error if failed
int OpenFFMPEG(char *FileName) {

    VidOpenHasAudio = false;
    VidOpenHasVideo = false;
    OpenVideoFFMPEG(FileName);
    OpenAudioFFMPEG(FileName);

    if (!VidOpenHasVideo && !VidOpenHasAudio)
        return 1;

    VidOpen=true; // opened video
    VidAudioEnded=false;
    VidVideoEnded=false;

    if (VidOpenHasVideo) {
        // allocate buffer required for u,v interpolation
        uFinal = (unsigned char*) malloc(videoWidth);
        vFinal = (unsigned char*) malloc(videoWidth);
        if (uFinal == NULL || vFinal == NULL) {
            VidOpenHasVideo = false;
            DestroyVideoFFMPEG();
        } else if (CreateSurf(&Sframe16, videoWidth, videoHeight, 16)==0) {
            VidOpenHasVideo = false;
            DestroyVideoFFMPEG();
        } else if(GetNextFrameFFMPEG(Sframe16,0)!=1) {
            // create Surf that will contain final frame
            VidOpenHasVideo = false;
            DestroyVideoFFMPEG();
        }
    }

    if (VidOpenHasAudio) {
        retOpenAudio = 11111;
        audioRingCount = 0;
        audioRingStart=0;
        if(GetNextAudioFrameFFMPEG()!=1) {
            VidOpenHasAudio = false;
            DestroyAudioFFMPEG();
        } else {
            audioLastAddIdx = -1;
            audioLastQueueIdx = -1;
        }
    }

    if (!VidOpenHasAudio && !VidOpenHasVideo) {
        VidOpen=false; // opened video
        return 14; // no audio or video
    }

    if (!VidOpenHasAudio)
        VidAudioEnded=true;
    if (!VidOpenHasVideo)
        VidVideoEnded=true;

    framenum=0; // found one frame
    PosSynch=0;
    if (VidOpenHasVideo)
        InitSynch(SynchBuff,&PosSynch,VideoFps*CurrentSpeedCoef);
    else
        InitSynch(SynchBuff,&PosSynch,20.0);

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

int OpenVideoFFMPEG(char *FileName) {
    int retfunc = 0;

	av_log_set_level(AV_LOG_QUIET);

	// Open media file.
	if (avformat_open_input(&pFormatCtx, FileName, NULL, NULL) != 0) {
        avformat_close_input(&pFormatCtx);
        pFormatCtx = NULL;
		return 1;
	}
	// Get format info.
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        avformat_close_input(&pFormatCtx);
        pFormatCtx = NULL;
		return 2;
	}
    videoStreamIndex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

    if (videoStreamIndex >= 0 && pFormatCtx->streams[videoStreamIndex]->disposition & AV_DISPOSITION_ATTACHED_PIC)
        videoStreamIndex = -1; // ignore any attachement

    if (videoStreamIndex < 0) {
        avformat_close_input(&pFormatCtx);
        pFormatCtx = NULL;
        return 3;
    }

    VideoTime = (float)(pFormatCtx->duration)/AV_TIME_BASE;
    if (VideoTime < 0.0f)
        VideoTime = 0.0f;

    AVStream *st;
    st = pFormatCtx->streams[videoStreamIndex];
    /* find decoder for the stream */
    pVideoCodec = avcodec_find_decoder(st->codecpar->codec_id);
    if (pVideoCodec == NULL) {
        avformat_close_input(&pFormatCtx);
        pFormatCtx = NULL;
        videoStreamIndex = -1;
        return 4;
    }
    if ((pkt = av_packet_alloc()) == NULL) {
        avformat_close_input(&pFormatCtx);
        pFormatCtx = NULL;
        videoStreamIndex = -1;
        return 4;
    }

    /* Allocate a codec context for the decoder */
    pVideoCodecCtx = avcodec_alloc_context3(pVideoCodec);
    if (pVideoCodecCtx == NULL) {
        DestroyVideoFFMPEG();
        return 5;
    }
    /* Copy codec parameters from input stream to output codec context */
    if (avcodec_parameters_to_context(pVideoCodecCtx, st->codecpar) < 0) {
        DestroyVideoFFMPEG();
        return 6;
    }
    /* Init the decoders */

    if (avcodec_open2(pVideoCodecCtx, pVideoCodec, NULL) < 0) {
        DestroyVideoFFMPEG();
        return 7;
    }

    video_stream = pFormatCtx->streams[videoStreamIndex];

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
    if (!videoFrame) {
        DestroyVideoFFMPEG();
        return 8;
    }

    // get frames per second
    VideoFps = (float)av_q2d(video_stream->r_frame_rate);
    // Need for convert time to ffmpeg time.
    videoFramesCount = (VideoTime>0) ? ((int)(VideoFps*VideoTime)) : 1;

    pVideoCodecParser = av_parser_init(pVideoCodec->id);
    if(pVideoCodecParser == NULL) {
        DestroyVideoFFMPEG();
        return 10;  // failed parser init
    }
    /* allocate image where the decoded image will be put */
    videoWidth = pVideoCodecCtx->width;
    videoHeight = pVideoCodecCtx->height;
    pix_fmt = pVideoCodecCtx->pix_fmt;

    if ((retfunc = av_image_alloc(video_dst_data, video_dst_linesize,
                       videoWidth, videoHeight, AV_PIX_FMT_RGB565BE, 1)) < 0) {
        DestroyVideoFFMPEG();
        return 11; //"Could not allocate raw video buffer\n"
    }

    VidOpenHasVideo = true;
    return 0;
}

int OpenAudioFFMPEG(char *FileName) {
    if (AudioEnabled) {
        SndDrv->DeleteAllVoices();
	// Open media file.
        if (avformat_open_input(&pAudioFormatCtx, FileName, NULL, NULL) != 0) {
            return 1;
        }
        // Get format info.
        if (avformat_find_stream_info(pAudioFormatCtx, NULL) < 0) {
            DestroyFFMPEG();
            return 2;
        }
        if ((pktAudio = av_packet_alloc()) == NULL) {
            avformat_close_input(&pAudioFormatCtx);
            pAudioFormatCtx = NULL;
            audioStreamIndex = -1;
            return 4;
        }

        // handle optional audio stream
        audioStreamIndex = av_find_best_stream(pAudioFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
        if (audioStreamIndex >= 0) {
            av_dump_format(pAudioFormatCtx, audioStreamIndex, NULL, false);

            // Find the decoder for the video stream
            pAudioCodec = avcodec_find_decoder(pAudioFormatCtx->streams[audioStreamIndex]->codecpar->codec_id);
            if (pAudioCodec != NULL) {
                /* dump input information to stderr */
                av_dump_format(pAudioFormatCtx, 0, FileName, 0);

                pAudioCodecCtx = avcodec_alloc_context3(pAudioCodec);
                pCodecAudioParam = pAudioFormatCtx->streams[audioStreamIndex]->codecpar;
                audio_stream = pAudioFormatCtx->streams[audioStreamIndex];
                /* Copy codec parameters from input stream to output codec context */
                if (avcodec_parameters_to_context(pAudioCodecCtx, audio_stream->codecpar) < 0) {
                    audioStreamIndex = -1;
                    audioFrameSamples = 444;
                }
                if (audioStreamIndex >= 0) {
                    //av_dict_set(pAudioCodecCtx->priv_data, "packet_size", "256", 0);
                    //pAudioCodecCtx->request_sample_fmt = AV_SAMPLE_FMT_S16;
                    if (avcodec_open2(pAudioCodecCtx, pAudioCodec, NULL) < 0) {
                        audioStreamIndex = -1;
                        audioFrameSamples = 111;
                    } else {
                        pAudioCodecParser = av_parser_init(pAudioCodec->id);

                        uint64_t iInputLayout                    = av_get_default_channel_layout(pAudioCodecCtx->ch_layout.nb_channels);
                        enum AVSampleFormat eInputSampleFormat   = (AVSampleFormat)pAudioCodecCtx->sample_fmt;
                        iInputSampleRate             = pAudioCodecCtx->sample_rate;

                        uint64_t iOutputLayout                   = (AudioStereo) ? AV_CH_LAYOUT_STEREO : AV_CH_LAYOUT_MONO;
                        enum AVSampleFormat eOutputSampleFormat  = (Audio16Bits) ? AV_SAMPLE_FMT_S16 : AV_SAMPLE_FMT_U8;
                        iOutputSampleRate            = AudioSamplingSpeed; //pAudioCodecCtx->sample_rate;

                        AVChannelLayout outLayout;
                        av_channel_layout_default(&outLayout, (AudioStereo) ? 2 : 1);
                        int dst_linesize = 0;



                        // allocate audio dest resample buffers (rounded by 1024)
                        max_dst_nb_samples = ((VoiceSampleSize/1024)+1)*1024;
                        if (av_samples_alloc_array_and_samples(&audio_dst_data, &audio_dst_linesize, (Audio16Bits) ? 2 : 1,
                            max_dst_nb_samples, eOutputSampleFormat, 0) < 0) {
                            audioStreamIndex = -1;
                            audioFrameSamples = 555;
                        } else {


                            if (UseOldFFMPEGResampler) {
                                au_convert_ctx= swr_alloc_set_opts(NULL,iOutputLayout, eOutputSampleFormat, iOutputSampleRate,
                                    iInputLayout,eInputSampleFormat, iInputSampleRate, 0, NULL);
                                if (au_convert_ctx == NULL) {
                                    audioStreamIndex = -1;
                                }
                            } else {
                                if (swr_alloc_set_opts2(&au_convert_ctx,
                                    &outLayout, eOutputSampleFormat, iOutputSampleRate,
                                    &pAudioCodecCtx->ch_layout, eInputSampleFormat, iInputSampleRate,
                                    0, NULL) != 0) {
                                    audioStreamIndex = -1;
                                }
                            }

                            audioFrame = av_frame_alloc();

                            if (audioStreamIndex != -1 && audioFrame != NULL) {
                                AudioTime = (float)(pAudioFormatCtx->duration)/AV_TIME_BASE;
                                if (AudioTime < 0.0f)
                                    AudioTime = 0.0f;

                                UpdateAudioRingDVoicesSamplingSpeed(iOutputSampleRate);

                                if (swr_init(au_convert_ctx) != 0) {
                                    audioStreamIndex = -1;
                                }
                                audioFrameSamples = 333;
                            } else {
                                audioStreamIndex = -1;
                            }

                        }
                    }
                }

            }
        }
    }
    if (audioStreamIndex < 0) {
        DestroyAudioFFMPEG();
        VidOpenHasAudio = false;
        return 1; // no audio
    }
    retOpenAudio = 7777777;
    VidOpenHasAudio = true;
    audioLastAddIdx = -1;
    audioLastQueueIdx = -1;
    audioRingStart = 0;
    audioRingCount = 0;
    curVoicePos = 0;

    return 0;
}

// return 1 if new frame found, 0 else
int GetNextFrameFFMPEG(DgSurf *S16, unsigned int nFramesToDrop) {
    unsigned int nDrops     = nFramesToDrop;
    int ret_av = 0;
    int retfunc = 0;
    bool frameFound = false;
    DgSurf *Surf8bpp = NULL;
    if (!VidOpen || !VidOpenHasVideo ) return 2;

    pVideoCodecCtx->skip_frame = AVDISCARD_NONKEY;
    while (nDrops > 0) {
        while (nDrops >0 && !frameFound && (ret_av = av_read_frame(pFormatCtx, pkt)) >= 0 ) {

            if (pkt->stream_index == videoStreamIndex) {

                if ((retfunc = avcodec_send_packet(pVideoCodecCtx, pkt)) < 0) {
                    av_packet_unref(pkt);
                    if (retfunc == AVERROR(EAGAIN)) {
                        if (nDrops>0) nDrops--;
                        avcodec_flush_buffers(pVideoCodecCtx);
                        continue;
                    }
                    VidVideoEnded=true; // we reached the end
                    return 0; // no frame
                }
                frameFound = true;
            }
            av_packet_unref(pkt);
        }
        // last read failure ?
        if (ret_av < 0) {
            VidVideoEnded = true;
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
                    VidVideoEnded = true;
                    return 0; // no frame
                }
                // submit the packet to the decoder
                // new Video found it should contain a frame
                frameFound = true;
                av_packet_unref(pkt);
            } else {
                //audioFrameSamples = 666;
                av_packet_unref(pkt);
            }
        }
        // last read failure ?
        if (ret_av < 0 /*&& ret_av != AVERROR(EAGAIN)*/) {
            VidVideoEnded = true;
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
    VidVideoEnded=true; // we reached the end
    return 0;
}




int GetNextAudioFrameFFMPEG() {
    int ret_av = 0;
    int retfunc = 0;
    int countAudFrameDecoded = 0;
    if (!VidOpen || !VidOpenHasAudio ) return 0;
    bool voiceAdded = false;

    FREE_MMX();
    while (1) {
        while ((ret_av = av_read_frame(pAudioFormatCtx, pktAudio)) >= 0) {
            retOpenAudio = 1;

            if (pktAudio->stream_index == audioStreamIndex) {
                avcodec_send_packet(pAudioCodecCtx, pktAudio);
                retOpenAudio = 2;
                while (avcodec_receive_frame(pAudioCodecCtx, audioFrame) >= 0) {
                    static int typeToSampleSize[] = { 1, 2, 2, 4 };
                    int curVoiceOneSample = typeToSampleSize[audioRing[0]->Type];

                    int out_samples = swr_get_out_samples(au_convert_ctx, audioFrame->nb_samples);
                    // reallocate/resize resample buffer if required
                    int dst_nb_samples = ((out_samples/1024)+1)*1024;
                    if (dst_nb_samples > max_dst_nb_samples) {
                        av_freep(&audio_dst_data[0]);
                        if (av_samples_alloc(audio_dst_data, &audio_dst_linesize, (Audio16Bits) ? 2 : 1,
                                       dst_nb_samples, (Audio16Bits) ? AV_SAMPLE_FMT_S16 : AV_SAMPLE_FMT_U8, 1) < 0)
                        {
                            // failure to reallocate
                            av_frame_unref(audioFrame);
                            av_packet_unref(pktAudio);
                            return 0;
                        }
                        max_dst_nb_samples = dst_nb_samples;
                    }

                    int outframes =  swr_convert(au_convert_ctx,
                                                 audio_dst_data,
                                                 max_dst_nb_samples,
                                                (const uint8_t **) audioFrame->data,
                                                audioFrame->nb_samples);

                    if (outframes > 0) {
                        countAudFrameDecoded ++;
                    } else {
                        /*char errbuf[256];
                        av_strerror(outframes, errbuf, sizeof(errbuf));
                        FLOG("Error swr_convert: %s\n", errbuf);*/
                        av_frame_unref(audioFrame);
                        continue;
                    }
                    int LastVoiceIdx = (audioRingStart+audioRingCount)%AUDIO_RING_SIZE;


                    if (curVoicePos/curVoiceOneSample + outframes >= VoiceSampleSize) {

                        if (audioRingCount<AUDIO_RING_SIZE) {
                                audioRingCount++;
                        } else {
                            // ring buffer full overwrite oldest voice
                            countRingOverWritten ++;
                            audioRingStart = (audioRingStart+1)%AUDIO_RING_SIZE;
                        }
                        // copy remaining required data into current Voice
                        int remainCopy = (VoiceSampleSize-(curVoicePos/curVoiceOneSample))*curVoiceOneSample;

                        // completely fill last voice
                        uint8_t *PtrDest = (uint8_t *)audioRing[LastVoiceIdx]->Ptr;
                        if (remainCopy > 0) {
                            memcpy(&PtrDest[curVoicePos], audio_dst_data[0], remainCopy);
                        }
                        int16_t *genPtr = (int16_t *)audioRing[LastVoiceIdx]->Ptr;

                        curVoicePos = (outframes*curVoiceOneSample) - remainCopy;
                        LastVoiceIdx = (audioRingStart+audioRingCount)%AUDIO_RING_SIZE;
                        // handle case where outframes bigger than VoiceSampleSize
                        while (curVoicePos/curVoiceOneSample > VoiceSampleSize) {
                            memcpy(audioRing[LastVoiceIdx]->Ptr, &audio_dst_data[0][remainCopy], VoiceSampleSize*curVoiceOneSample);
                            remainCopy += VoiceSampleSize*curVoiceOneSample;
                            curVoicePos -= VoiceSampleSize*curVoiceOneSample;

                            if (audioRingCount<AUDIO_RING_SIZE) {
                                    audioRingCount++;
                            } else {
                                // ring buffer full: overwrite oldest voice
                                countRingOverWritten ++;
                                audioRingStart = (audioRingStart+1)%AUDIO_RING_SIZE;
                            }
                            LastVoiceIdx = (audioRingStart+audioRingCount)%AUDIO_RING_SIZE;
                        }
                        // last chunk ?
                        if (curVoicePos > 0) {
                            memset(audioRing[LastVoiceIdx]->Ptr, 0, audioRing[LastVoiceIdx]->Size);
                            memcpy(audioRing[LastVoiceIdx]->Ptr, &audio_dst_data[0][remainCopy], curVoicePos);
                        }

                        retOpenAudio = 9;
                        voiceAdded = true;
                    } else if (outframes > 0) {
                        uint8_t *PtrDest = (uint8_t *)audioRing[LastVoiceIdx]->Ptr;
                        memcpy(&PtrDest[curVoicePos], audio_dst_data[0], outframes*curVoiceOneSample);
                        curVoicePos += outframes*curVoiceOneSample; //audioFrame->nb_samples;
                    }

                    /*swr_convert(au_convert_ctx,
                                                 audio_dst_data,
                                                 max_dst_nb_samples,
                                                NULL,
                                                0);*/

                    av_frame_unref(audioFrame);
                }

                av_packet_unref(pktAudio);
            } else {
                //audioFrameSamples = 666;
                av_packet_unref(pktAudio);
            }
            if (voiceAdded)
                return 1;
        }
        // file ended but there is some remaining audio data
        if (countAudFrameDecoded > 0) {
            if (audioRingCount<AUDIO_RING_SIZE) {
                    audioRingCount++;
            } else {
                // ring buffer full: overwrite oldest voice
                audioRingStart = (audioRingStart+1)%AUDIO_RING_SIZE;
            }
            return 1;
        }
        // last read failure ?
        if (ret_av < 0 /*&& ret_av != AVERROR(EAGAIN)*/) {
            VidAudioEnded = true;
            retOpenAudio = 8;
            return 0;
        }
    }
    VidAudioEnded = true;
    retOpenAudio = 6;
    return 0;
}

int SeekFrameFFMPEG(DgSurf *S16, unsigned int FrameNum) {
    if (!VidOpen || videoStreamIndex < 0 )
        return 0;
    int64_t target_pts = (int64_t)(FrameNum * (1.0 / av_q2d(video_stream->avg_frame_rate)) / av_q2d(video_stream->time_base));
    avcodec_flush_buffers(pVideoCodecCtx);
    av_seek_frame(pFormatCtx, videoStreamIndex, target_pts, AVSEEK_FLAG_BACKWARD);
    if (VidOpenHasAudio) {
        avcodec_flush_buffers(pAudioCodecCtx);
        if (av_seek_frame(pAudioFormatCtx, videoStreamIndex, target_pts, AVSEEK_FLAG_BACKWARD) == 0) {
            VidAudioEnded = false;
            audioRingCount = 0; // invalidate audio ring buffer contents
        }
    }
    return GetNextFrameFFMPEG(S16,0);
}

int SeekAudioFFMPEG(float targetTimeSeconds) {
    if (!VidOpen || videoStreamIndex >= 0 )
        return 0;

    if (VidOpenHasAudio) {
        int64_t target_ts = av_rescale_q(targetTimeSeconds * AV_TIME_BASE, AV_TIME_BASE_Q, audio_stream->time_base);
        avcodec_flush_buffers(pAudioCodecCtx);
        if (av_seek_frame(pAudioFormatCtx, audioStreamIndex, target_ts, AVSEEK_FLAG_BACKWARD)== 0) {
            VidAudioEnded = false;
            return 1;
        }
        audioRingCount = 0; // invalidate audio ring buffer contents
    }
    return 0;
}

// destroy/free as much as possible ffmpeg allocated mem/ressources
void DestroyVideoFFMPEG() {
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
    videoStreamIndex = -1;
    video_stream = NULL;
    VideoTime = 0.0f;

    if(uFinal) { free(uFinal); uFinal = NULL; }
    if(vFinal) { free(vFinal); vFinal = NULL; }
}

void DestroyAudioFFMPEG() {

    if (pAudioFormatCtx) {
        avformat_close_input(&pAudioFormatCtx);
        pAudioFormatCtx = NULL;
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
        swr_convert(au_convert_ctx, audio_dst_data, max_dst_nb_samples, NULL, 0);
        swr_free(&au_convert_ctx);


        au_convert_ctx = NULL;
    }
    if (pktAudio) {
        av_packet_free(&pktAudio);
        pktAudio = NULL;
    }
    if (audio_dst_data) {
        av_freep(&audio_dst_data[0]);
        audio_dst_data[0] = NULL;
        av_freep(&audio_dst_data);
        audio_dst_data = NULL;
    }
    AudioTime = 0.0f;
    audioStreamIndex = -1;
    audioLastAddIdx = -1;
    audioLastQueueIdx = -1;
    max_dst_nb_samples = 0;
    audio_stream = NULL;
}

void DestroyFFMPEG() {
    // wait if any sound is still playing

    DestroyVideoFFMPEG();
    DestroyAudioFFMPEG();
    if (Sframe16!=NULL) {
        DestroySurf(Sframe16);
        Sframe16 = NULL;
    }
    if (Slastframe16!=NULL) {
        DestroySurf(Sframe16);
        Sframe16 = NULL;
    }

    videoFramesCount = 0;
}

// close an opened video
void CloseVidFFMPEG() {
    if (!VidOpen)
        return;
    DestroyFFMPEG();
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

    // load the sound driver
    if (!LoadSoundDRV(&SndDrv,soundDriverFileName)) {
        return false; // load driver error
    }
    // alloc the memory buffer needed by the sound driver
	if ((SndBuff=malloc(SndDrv->SizeBuff))==NULL) {
        DestroySoundDRV(SndDrv);
        return false; // no mem
    }
    memset(SndBuff, 0, SndDrv->SizeBuff);
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
    audioRingStart = 0;
    audioRingCount = 0;

    SndDrv->ContinueSound();

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
    int currentSpeed = (int)(CurrentSpeedCoef * (float)Vc->Freq);
    // adjust the speed of the voice if it's speed is inequal with
    // the current sampling speed
    if (updateSpeed && SndDrv->Cur_SampSpeed!=currentSpeed) {
        VP.Speed=(128*currentSpeed)/SndDrv->Cur_SampSpeed;
        return SndDrv->AddVoice(Vc,DS_EFF_CHG_SPEED,State,&VP,0);
	} else // else add as it
        return SndDrv->AddVoice(Vc,0,State,NULL,0);
}

int QueueVoice(DVoice *toQueueVc,DVoice *Vc,int State, bool updateSpeed, bool replaceExisting)
{
    int currentSpeed = (int)(CurrentSpeedCoef * (float)Vc->Freq);
    // adjust the speed of the voice if it's speed is inequal with
    // the current sampling speed
    if (updateSpeed && SndDrv->Cur_SampSpeed!=currentSpeed) {
        VP.Speed=(128*currentSpeed)/SndDrv->Cur_SampSpeed;
        return SndDrv->QueueVoice(toQueueVc,Vc,DS_EFF_CHG_SPEED,State,&VP,0,replaceExisting);
	} else // else add as it
        return SndDrv->QueueVoice(toQueueVc,Vc,0,State,NULL,0,replaceExisting);
}

// Create/Prepare Audio Ring DVoices
bool CreatePrepAudioRingDVoices() {
    int resCreate = 0;
    int i=0;

    audioRingStart = 0;
    audioRingCount = 0;
    if (!AudioEnabled || AUDIO_RING_SIZE <= 0)
        return false;
    audioRing = (DVoice**)malloc(sizeof(DVoice*)*AUDIO_RING_SIZE);
    if (audioRing == NULL)
        return false;

    for (i=0; i < AUDIO_RING_SIZE; i++) {
        resCreate = CreateDVoice(&audioRing[i], Audio16Bits, AudioStereo, AudioSamplingSpeed, VoiceSampleSize);
        if (!resCreate)
            break;
    }
    if (!resCreate) {
        for (i=0; i < AUDIO_RING_SIZE; i++) {
            if (audioRing[i] != NULL)
                DestroyDVoice(audioRing[i]);
        }
        free(audioRing);
        audioRing = NULL;
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
    free(audioRing);
    audioRing = NULL;
    audioRingStart = 0;
    audioRingCount = 0;

    return true;
}

void RenderAudioLastAdd() {
    if (!VidOpen || !VidOpenHasAudio || audioLastCurveIdx < 0)
        return;
    short *data16 = NULL;
    unsigned char *data8 = NULL;
    int step = 0;
    int startZ = 0;
    int y0 = 0, y1 = 0;
    DgSetCurSurf(Sframe16);

    Clear16(DisplaySoundBackCol);
    switch (audioRing[audioLastCurveIdx]->Type) {
    case 0: // 8 bits mono
        data8 = (unsigned char*)(audioRing[audioLastCurveIdx]->Ptr);
        step = 1;
        break;
    case 1: // 8 bits stereo
        data8 = (unsigned char*)(audioRing[audioLastCurveIdx]->Ptr);
        step = 2;
        break;
    case 2: // 16 bits mono
        data16 = (short*)(audioRing[audioLastCurveIdx]->Ptr);
        step = 1;
        break;
    case 3: // 16 bits stereo
        data16 = (short*)(audioRing[audioLastCurveIdx]->Ptr);
        step = 2;
        break;
    }
    // search for first 0 index
    if (data8 != NULL) {
        for (startZ=0; startZ < VoiceSampleSize-500; startZ++) {
            if (data8[startZ*step] == 128) break;
        }
    } else if (data16 != NULL) {
        for (startZ=0; startZ < VoiceSampleSize-500; startZ++) {
            if ((data16[startZ*step]>>8) == 0) break;
        }
    }

    // render curve
    if (data8 != NULL) {
        y0 = (int)(data8[startZ])-128;
        for (int i=1; i < 500; i++) {
            y1 = (int)(data8[startZ+i*step])-128;
            line16(i-1, y0, i, y1, DisplaySoundCurveCol);
            line16(i-1, y0-1, i, y1+1, DisplaySoundCurveCol);
            y0 = y1;
        }
    } else if (data16 != NULL) {
        y0 = data16[startZ]>>8;
        for (int i=1; i < 500; i++) {
            y1 = data16[startZ+i*step]>>8;
            line16(i-1, y0, i, y1, DisplaySoundCurveCol);
            line16(i-1, y0-1, i, y1+1, DisplaySoundCurveCol);
            y0 = y1;
        }
    }

    if (DisplaySoundMode == 1) {
        SurfCopyTrans16(Slastframe16, Sframe16, DisplaySoundModeTransLevel);
        SurfCopy(Sframe16, Slastframe16);
    }
}

void timeToStr(float timeInSec, char *outStr, size_t outStrSize) {
    if (timeInSec >= 3600) {
        snprintf(outStr, outStrSize,"%u:%02u:%02u", (int)(timeInSec/3600),(int)(timeInSec/60)%60,(int)timeInSec%60);
    } else if (timeInSec >= 60) {
        snprintf(outStr, outStrSize, "%u:%02u", (int)(timeInSec/60)%60,(int)timeInSec%60);
    } else {
        snprintf(outStr, outStrSize, "%u",(int)timeInSec);
    }

}
