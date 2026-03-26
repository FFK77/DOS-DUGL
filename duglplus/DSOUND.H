#ifndef DSOUND_H

#define DSOUND_H

typedef struct {
	int Speed,
	    LeftVol,RightVol,
	    Priority;
	int Resv[6];
} DVoicePack;

typedef struct {
	void *Ptr;
	int  Size,Freq,Type,SizeSecond;
	int  resv[19];
} DVoice;

// Voice Type
#define DS_8BITS_MONO     0
#define DS_8BITS_STEREO   1
#define DS_16BITS_MONO    2
#define DS_16BITS_STEREO  3

// VOICE Effect
#define DS_EFF_NONE	        0
#define DS_EFF_CHG_SPEED	1
#define DS_EFF_CHG_VOL	    2
// VOICE State
#define DS_VC_NORMAL	0
#define DS_VC_LOOPING	1
#define DS_VC_STOPPED	2
#define DS_VC_QUEUED	4

// SoundDRV->InitSound(...) parameters
#define DS_NOSOUND 		0  // Type8Bit & Type16Bit
#define DS_OUT8BIT 		1  // Type8Bit
#define DS_IN8BIT		2  // Type8Bit
#define DS_OUT16BIT 	1  // Type16Bit
#define DS_IN16BIT 		2  // Type16Bit
#define DS_MONO			0  // Sound Type
#define DS_STEREO		1  // sound Type
// capabilities
#define DS_CAP_OUT8			0x001
#define DS_CAP_OUT16		0x002
#define DS_CAP_OUT_MONO		0x004
#define DS_CAP_OUT_STEREO	0x008
#define DS_CAP_IN8			0x010
#define DS_CAP_IN16			0x020
#define DS_CAP_IN_MONO		0x040
#define DS_CAP_IN_STEREO	0x080
#define DS_CAP_FULLDUPLEX	0x100
#define DS_CAP_AOTODETECT	0x200


typedef struct {
unsigned int Signature,		//   ==	"FSDR"
	     SizeDrv,
	     Capabilities,Capabilities2,
	     SizeBuff,
	     Min8BitMono,Max8BitMono,
	     Min16BitMono,Max16BitMono,
	     Min8BitStereo,Max8BitStereo,
	     Min16BitStereo,Max16BitStereo,
	     Card_BasePort,Card_IRQ,Card_DMA8,Card_DMA16,
	     Cur_SampSpeed,
	     LastError,
		 Version,
	     resv[24];
	void *DrvBuffPtr;
	// sound card init/control functions
	char *CardName;
	int  InitDriverPtr;
	int  (*InstallDriver)(void *Buff,int Base,int IRQ,int DMA8,int DMA16);
	void (*UninstallDriver)();
	int  (*InitSound)(int Type8Bits,int Type16Bits,bool bStereo,int SampleSpeed);
	void (*ResetSound)();
	void (*StopSound)();
	void (*ContinueSound)();
	void (*SetMasterVolume)(int LeftVol,int RightVol);
	void (*SetVoiceVolume)(int LeftVol,int RightVol);
	void (*SetMidiVolume)(int LeftVol,int RightVol);
	void (*SetCDVolume)(int LeftVol,int RightVol);
	void (*SetLineVolume)(int LeftVol,int RightVol);
	void (*SetMicVolume)(int Vol);
	void (*SetInGain)(int LeftInGain,int RightInGain);
	void (*SetOutGain)(int LeftOutGain,int RightOutGain);
	void (*SetAutoGain)(int AutoGain);
	void (*SetTreble)(int LeftTreble,int RightTreble);
	void (*SetBass)(int LeftBass,int RightBass);
	void (*SetOutput)(int SelectOutput);
	void (*SetInput)(int SelectInputLeft,int SelectInputRight);
	int  ResvGlob[44];
	// Mixer functions
	// Add new DVoice, return 1 if success, Existing voice will not be accepted
	int  (*AddVoice)(DVoice *Vc,int VcEffect,int VcState,DVoicePack *VcP, unsigned int Pos);
	// Prepare DVoice, need to be called before any Add or Queue ..
	int  (*PrepareVoice)(DVoice *Vc);
	// to be called once DVoice isn't required and going to be Destroyed
	int  (*UnprepareVoice)(DVoice *Vc);
	int  (*GetVoiceState)(DVoice *Vc,int *VcState);
	int  (*SetVoiceState)(DVoice *Vc,int VcState);
	int  (*GetVoiceEffPack)(DVoice *Vc,int *VcEffect,DVoicePack *VcP);
	int  (*SetVoiceEffPack)(DVoice *Vc,int VcEffect,DVoicePack *VcP);
	int  (*GetVoicePos)(DVoice *Vc,int *VcVrPos);
	int  (*SetVoicePos)(DVoice *Vc,int VcVrPos);
	// delete Voice Vc
	int  (*DeleteVoice)(DVoice *Vc);
	// delete all Voices
	void (*DeleteAllVoices)();
	// return current count of all Voices including stopped ones
	int  (*GetCountVoices)();
	// return 1 if Vc has been successfully queued to toVoice, set Vc to NULL to just delete the queue
	int  (*QueueVoice)(DVoice *toVoice, DVoice *Vc,int VcEffect,int VcState,DVoicePack *VcP, unsigned int Pos);
	// return 1 if toVoice has been replace by Vc, if successfull delete also any queued DVoice, set Vc to NULL to just delete
	int  (*ReplaceVoice)(DVoice *toVoice, DVoice *Vc,int VcEffect,int VcState,DVoicePack *VcP, unsigned int Pos);
	// return true if Voice already exist, even if state stopped or Queued
	bool (*ExistVoice)(DVoice *Vc);

	int  VoiceMixer[113];
} SoundDRV;

#ifdef __cplusplus
extern "C" {
#endif

int  LoadSoundDRV(SoundDRV **SndDrv,char *Fname);
int  LoadMemSoundDRV(SoundDRV **SndDrv,void *In,int SizeIn);
void DestroySoundDRV(SoundDRV *SndDrv);

int  CreateDVoice(DVoice **Vc, bool bits16, bool stereo, int sampleSpeed, int countSamples);
void DestroyDVoice(DVoice *Vc);
int  LoadWAV(DVoice **Vc,char *Fname);
int  LoadMemWAV(DVoice **Vc,void *In,int SizeIn);
int  SaveWAV(DVoice *Vc,char *Fname);
int  SaveMemWAV(DVoice *Vc,void *Out);
int  SizeSaveWAV(DVoice *Vc);

#ifdef __cplusplus
           }
#endif

#endif  //#ifndef DSOUND_H
