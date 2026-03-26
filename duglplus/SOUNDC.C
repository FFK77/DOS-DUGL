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
#include <dugl.h>
#include "intrdugl.h"
#include "dsound.h"

int  LoadSoundDRV(SoundDRV **SndDrv,char *Fname)
{	FILE *InSoundDRV;
	SoundDRV SD;
	int Size;
	char *Buff;
	void (*InitDriver)();
	if ((InSoundDRV=fopen(Fname,"rb"))==NULL) {
		return 0;
	}
	if (fread(&SD,sizeof(SoundDRV),1,InSoundDRV)<1) return 0;
	fseek(InSoundDRV,0,SEEK_END);
	Size=ftell(InSoundDRV);
	if (SD.Signature!='RDSF' || SD.SizeDrv!=Size) {
	   fclose(InSoundDRV);
	   return 0;
	}
	if ((Buff=(char*)malloc(SD.SizeDrv+0x1f))==NULL) {
	   fclose(InSoundDRV);
	   return 0;
	}
	// ALIGN 32 BYTES
	if((int)(Buff)&0x1f)
	   *SndDrv=(SoundDRV*)&Buff[32-((unsigned int)(Buff)&0x1f)];
	else
	   *SndDrv=(SoundDRV*)Buff;

	fseek(InSoundDRV,0,SEEK_SET);
	if (fread(*SndDrv,SD.SizeDrv,1,InSoundDRV)<1) {
	  free(Buff); fclose(InSoundDRV);
	  return 0;
	}

	InitDriver=(void (*)())(SD.InitDriverPtr+(unsigned int)(*SndDrv));
	(*SndDrv)->DrvBuffPtr=Buff;
	InitDriver();
	fclose(InSoundDRV);
	return 1;
}

int  LoadMemSoundDRV(SoundDRV **SndDrv,void *In,int SizeIn)
{	SoundDRV SD;
	char *Buff;
	void (*InitDriver)();
	memcpy(&SD,In,sizeof(SoundDRV));
	if (SD.Signature!='RDSF' || SD.SizeDrv!=SizeIn) return 0;
	if ((Buff=(char*) malloc(SD.SizeDrv+0x1f))==NULL) return 0;
	// ALIGN 32 BYTES
	if((int)(Buff)&0x1f)
	   *SndDrv=(SoundDRV*)&Buff[32-((unsigned int)(Buff)&0x1f)];
	else
	   *SndDrv=(SoundDRV*)Buff;
	memcpy(Buff,In,SD.SizeDrv);
	InitDriver=(void (*)())(SD.InitDriverPtr+(unsigned int)(*SndDrv));
	(*SndDrv)->DrvBuffPtr=Buff;
	InitDriver();
	return 1;
}

void DestroySoundDRV(SoundDRV *SndDrv)
{
	free(SndDrv->DrvBuffPtr);
	memset(SndDrv,0,SndDrv->SizeDrv);
}

int  LoadWAV(DVoice **Vc,char *Fname) {
	FILE *InWAV;
	HeadWAV hwav;
	void *Buff;
	*Vc = (DVoice*) malloc(sizeof(DVoice));
	if (*Vc == NULL)
		return 0; // no mem

	memset(*Vc, 0, sizeof(DVoice));
	if ((InWAV=fopen(Fname,"rb"))==NULL) return 0;
	fread(&hwav,sizeof(HeadWAV),1,InWAV);
	if (hwav.Sign!='FFIR' || hwav.SignDATA!='atad' ||
	    (hwav.BitEchant!=8 && hwav.BitEchant!=16) ) {
		free(*Vc);
		fclose(InWAV);
		return 0;
	}

	if ((Buff=malloc(hwav.SizeDATA))==NULL) {
		free(*Vc);
		fclose(InWAV);
		return 0;
	}
	if (hwav.Type==1) {
	  if (hwav.BitEchant==8) (*Vc)->Type=0;
	     else (*Vc)->Type=2;
	} else if (hwav.Type==2) {
	  if (hwav.BitEchant==8) (*Vc)->Type=1;
	     else (*Vc)->Type=3;
	} else { free(Buff); fclose(InWAV); return 0; }

	if (fread(Buff,hwav.SizeDATA,1,InWAV)<1) {
		free(*Vc);
		free(Buff);
		fclose(InWAV);
		return 0;
	}
	(*Vc)->Ptr=Buff;
	(*Vc)->Size=hwav.SizeDATA;
	(*Vc)->Freq=hwav.SamplingSpeed;
	(*Vc)->SizeSecond=hwav.ByteOutSec;

	fclose(InWAV);
	return 1;
}

int  LoadMemWAV(DVoice **Vc,void *In,int SizeIn)
{	HeadWAV hwav;
	void *Buff = NULL;
	memcpy(&hwav,In,sizeof(HeadWAV));
	if (hwav.Sign!='FFIR' || hwav.SignDATA!='atad' ||
	    (hwav.BitEchant!=8 && hwav.BitEchant!=16) ) {
		return 0;
	}
	*Vc = (DVoice*) malloc(sizeof(DVoice));
	if (*Vc == NULL)
		return 0; // no mem
	memset(*Vc, 0, sizeof(DVoice));

	if ((Buff=malloc(hwav.SizeDATA))==NULL) {
		free(*Vc);
		*Vc = NULL;
		return 0;
	}
	if (hwav.Type==1) {
	  if (hwav.BitEchant==8) (*Vc)->Type=0;
	     else (*Vc)->Type=2;
	} else if (hwav.Type==2) {
	  if (hwav.BitEchant==8) (*Vc)->Type=1;
	     else (*Vc)->Type=3;
	} else {
		free(Buff);
		return 0;
	}
	if ((sizeof(HeadWAV)+hwav.SizeDATA)<SizeIn) {
		free(*Vc);
		*Vc = NULL;
		free(Buff);
		return 0;
	}
	memcpy(Buff,In+sizeof(HeadWAV),hwav.SizeDATA);
	(*Vc)->Ptr=Buff;
	(*Vc)->Size=hwav.SizeDATA;
	(*Vc)->Freq=hwav.SamplingSpeed;
	(*Vc)->SizeSecond=hwav.ByteOutSec;

	return 1;
}

int  CreateDVoice(DVoice **Vc, bool bits16, bool stereo, int sampleSpeed, int countSamples) {
	static int typeToSampleSize[] = { 1, 2, 2, 4 };
	void *Buff = NULL;

	if (sampleSpeed < 1 || countSamples < 1)
		return 0;
	*Vc = (DVoice*) malloc(sizeof(DVoice));
	if (*Vc == NULL)
		return 0; // no mem
	memset(*Vc, 0, sizeof(DVoice));

	if (stereo) { // stereo
		if (!bits16)
			(*Vc)->Type=1;
	    else
			(*Vc)->Type=3;
	} else { // mono
		if (!bits16)
			(*Vc)->Type=0;
	    else
			(*Vc)->Type=2;
	}
	(*Vc)->Size = countSamples * typeToSampleSize[(*Vc)->Type];
	if ((Buff=malloc((*Vc)->Size))==NULL) {
		free(*Vc);
		*Vc = NULL;
		return 0;
	}
	memset(Buff, 0, (*Vc)->Size);
	(*Vc)->Ptr=Buff;
	(*Vc)->Freq = sampleSpeed;
	(*Vc)->SizeSecond =  sampleSpeed * typeToSampleSize[(*Vc)->Type];
	return 1;
}

void DestroyDVoice(DVoice *Vc) {
	if (Vc != NULL) {
		if (Vc->Ptr!=NULL) {
			free(Vc->Ptr);
			Vc->Ptr=NULL;
		}
		free(Vc);
	}
}

