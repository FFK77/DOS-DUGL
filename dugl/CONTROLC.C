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
#include "dugl.h"
#include "intrdugl.h"

//MouseEvent MsEventsStack[256];
//int MsStackNbElt=0,MsStackEnable=0;

int  LoadKbMAP(KbMAP **KMap,const char *Fname)
{	FILE *InKbMAP;
	KbMAP KM;
	int Size,i;
	unsigned int Buff;

	if ((InKbMAP=fopen(Fname,"rb"))==NULL) return 0;
	if (fread(&KM,sizeof(KbMAP),1,InKbMAP)<1) return 0;
	fseek(InKbMAP,0,SEEK_END);
	Size=ftell(InKbMAP);
	if (KM.Sign!='PAMK' || KM.SizeKbMap!=(Size-sizeof(KbMAP)))
	  { fclose(InKbMAP); return 0; }
	if ((*KMap=(KbMAP*) malloc(KM.SizeKbMap+sizeof(KbMAP)))==NULL)
	  { fclose(InKbMAP); return 0; }
	Buff=(unsigned int)(*KMap);

	fseek(InKbMAP,0,SEEK_SET);
	if (fread(*KMap,KM.SizeKbMap+sizeof(KbMAP),1,InKbMAP)<1)
	  { free(*KMap); fclose(InKbMAP); return 0; }

	// Ajuste les pointeur
	(*KMap)->KbMapPtr=(void*)((unsigned int)((*KMap)->KbMapPtr)+Buff);
	if ((*KMap)->TabPrefixKeyb!=NULL)
	  (*KMap)->TabPrefixKeyb=
	    (PrefixKeyb*)((unsigned int)((*KMap)->TabPrefixKeyb)+Buff);
	if ((*KMap)->TabNormPrefixKeyb!=NULL)
	  (*KMap)->TabNormPrefixKeyb=
	    (NormKeyb*)((unsigned int)((*KMap)->TabNormPrefixKeyb)+Buff);
	if ((*KMap)->TabNormKeyb!=NULL)
	  (*KMap)->TabNormKeyb=
	    (NormKeyb*)((unsigned int)((*KMap)->TabNormKeyb)+Buff);

	if ((*KMap)->TabNormKeyb!=NULL)
	  for (i=0;i<(*KMap)->NbNorm;i++)
	    (*KMap)->TabNormKeyb[i].Ptr=
	      (unsigned char*)((unsigned int)((*KMap)->TabNormKeyb[i].Ptr)+Buff);
	if ((*KMap)->TabNormPrefixKeyb!=NULL)
	  for (i=0;i<(*KMap)->NbNormPrefix;i++)
	    (*KMap)->TabNormPrefixKeyb[i].Ptr=
	      (unsigned char*)((unsigned int)((*KMap)->TabNormPrefixKeyb[i].Ptr)+Buff);
	if ((*KMap)->TabPrefixKeyb!=NULL)
	  for (i=0;i<(*KMap)->NbPrefix;i++) {
	    (*KMap)->TabPrefixKeyb[i].TabNormKeyb=
	      (NormKeyb*)((unsigned int)((*KMap)->TabPrefixKeyb[i].TabNormKeyb)+Buff);
	  }
	fclose(InKbMAP);
	return 1;
}

int  LoadMemKbMAP(KbMAP **KMap,void *In,int SizeIn)
{	KbMAP KM;
	int i;
	unsigned int Buff;

	memcpy(&KM,In,sizeof(KbMAP));
	if (KM.Sign!='PAMK' || KM.SizeKbMap!=(SizeIn-sizeof(KbMAP))) return 0;
	if ((*KMap=(KbMAP*)malloc(KM.SizeKbMap+sizeof(KbMAP)))==NULL) return 0;
	Buff=(unsigned int)(*KMap);
	memcpy(*KMap,In,KM.SizeKbMap+sizeof(KbMAP));

	// Ajuste les pointeur
	(*KMap)->KbMapPtr=(void*)((unsigned int)((*KMap)->KbMapPtr)+Buff);
	if ((*KMap)->TabPrefixKeyb!=NULL)
	  (*KMap)->TabPrefixKeyb=
	    (PrefixKeyb*)((unsigned int)((*KMap)->TabPrefixKeyb)+Buff);
	if ((*KMap)->TabNormPrefixKeyb!=NULL)
	  (*KMap)->TabNormPrefixKeyb=
	    (NormKeyb*)((unsigned int)((*KMap)->TabNormPrefixKeyb)+Buff);
	if ((*KMap)->TabNormKeyb!=NULL)
	  (*KMap)->TabNormKeyb=
	    (NormKeyb*)((unsigned int)((*KMap)->TabNormKeyb)+Buff);

	if ((*KMap)->TabNormKeyb!=NULL)
	  for (i=0;i<(*KMap)->NbNorm;i++)
	    (*KMap)->TabNormKeyb[i].Ptr=
	      (unsigned char*)((unsigned int)((*KMap)->TabNormKeyb[i].Ptr)+Buff);
	if ((*KMap)->TabNormPrefixKeyb!=NULL)
	  for (i=0;i<(*KMap)->NbNormPrefix;i++)
	    (*KMap)->TabNormPrefixKeyb[i].Ptr=
	      (unsigned char*)((unsigned int)((*KMap)->TabNormPrefixKeyb[i].Ptr)+Buff);
	if ((*KMap)->TabPrefixKeyb!=NULL)
	  for (i=0;i<(*KMap)->NbPrefix;i++) {
	    (*KMap)->TabPrefixKeyb[i].TabNormKeyb=
	      (NormKeyb*)((unsigned int)((*KMap)->TabPrefixKeyb[i].TabNormKeyb)+Buff);
	  }
	return 1;
}

void DestroyKbMAP(KbMAP *KM) {
   if (KM) free(KM);
}

// time Synch

int  InitSynch(void *SynchBuff,int *Pos,float Freq) {
   SynchTime *ST;
   if (!DgTimerFreq) return 0;
   // start Sync
   StartSynch(SynchBuff,Pos);
   // save parameters
   ST=((SynchTime*)(SynchBuff));
   ST->Freq=Freq;
   return 1;
}

int  Synch(void *SynchBuff,int *Pos) {
   SynchTime *ST;
   int ipos;
   if (DgTimerFreq==0 || SynchBuff==NULL) return 0;
   ST=((SynchTime*)(SynchBuff));
   ST->LastSynchNull=0;
   // continu only if time changed
   if (ST->LastTimeValue==DgTime) {
      if (Pos!=NULL) *Pos=ST->LastPos;
      ST->NbNullSynch++;
      ST->LastSynchNull=1;
      return 0; // delta Synch 0
   }
   // reset the history table if 32 time counter reaching the end
   if (DgTime<ST->LastTimeValue)
     StartSynch(SynchBuff,Pos);

   // Add a new time value
   if (ST->hstNbItems<SYNCH_HST_SIZE) { // time table not yet full ?
     ST->hstIdxFin=(ST->hstIdxDeb+ST->hstNbItems)&(SYNCH_HST_SIZE-1);
     ST->LastTimeValue=DgTime;
     ST->TimeHst[ST->hstIdxFin]=ST->LastTimeValue;
     ST->hstNbItems++;
   }
   else { // time table full
     ST->hstIdxDeb=(ST->hstIdxDeb+1)&(SYNCH_HST_SIZE-1);
     ST->hstIdxFin=(ST->hstIdxDeb+SYNCH_HST_SIZE-1)&(SYNCH_HST_SIZE-1);
     ST->LastTimeValue=DgTime;
     ST->TimeHst[ST->hstIdxFin]=ST->LastTimeValue;
     if(ST->hstIdxFin==0)
     {
       ST->LastNbNullSynch=ST->NbNullSynch;
       ST->NbNullSynch=0;
     }
   }
   ipos = ST->LastPos;
   // increase the pos
   ST->LastPos+=SynchLastTime(SynchBuff)*ST->Freq;
   if (Pos!=NULL) *Pos=ST->LastPos;

   return ST->LastPos-ipos;
}

void StartSynch(void *SynchBuff,int *Pos) {
   SynchTime *ST;
   if (DgTimerFreq==0 || SynchBuff==NULL) return;
   ST=((SynchTime*)(SynchBuff));
   // start Sync
   if (Pos!=NULL) {
      ST->LastPos=(float)(*Pos);
   } else {
      ST->LastPos=0.0;
   }
   ST->FirstTimeValue=DgTime;
   ST->LastTimeValue=ST->FirstTimeValue;
   bzero(&ST->TimeHst[0],SYNCH_HST_SIZE*sizeof(unsigned int));
   ST->TimeHst[0]=ST->FirstTimeValue;
   ST->hstIdxDeb=0; ST->hstIdxFin=1;
   ST->hstNbItems=1;
   ST->LastSynchNull=0;
   ST->NbNullSynch=0;
   ST->LastNbNullSynch=0;
}

float SynchAccTime(void *SynchBuff) {
   SynchTime *ST;
   if (DgTimerFreq==0 || SynchBuff==NULL) return 0;
   ST=((SynchTime*)(SynchBuff));
   return (float)(DgTime-ST->FirstTimeValue)/(float)(DgTimerFreq);
}

float SynchAverageTime(void *SynchBuff) {
   SynchTime *ST;
   unsigned int i,idxDeb,idxFin;
   int SumSyncTime=0;
   ST=((SynchTime*)(SynchBuff));
   if (DgTimerFreq==0 || ST==NULL || ST->hstNbItems<2) return 0.0;
   for (i=0;i<ST->hstNbItems-1;i++) {
     idxDeb = (ST->hstIdxDeb+i)&(SYNCH_HST_SIZE-1);
     idxFin = (ST->hstIdxDeb+i+1)&(SYNCH_HST_SIZE-1);
     SumSyncTime+=(ST->TimeHst[idxFin]-ST->TimeHst[idxDeb]);
   }
   i=ST->LastNbNullSynch;
   //ST->NbNullSynch=0;
   return (float)(SumSyncTime)/(float)((ST->hstNbItems-1+i)*DgTimerFreq);
}

float SynchLastTime(void *SynchBuff) {
   SynchTime *ST;
   int idxDeb,idxAFin;
   int SumSyncTime;
   ST=((SynchTime*)(SynchBuff));
   if (DgTimerFreq==0 || ST==NULL || ST->hstNbItems<2 || ST->LastSynchNull)
     return 0.0;
   idxDeb=(ST->hstIdxDeb+ST->hstNbItems-2)&(SYNCH_HST_SIZE-1);
   idxAFin=(ST->hstIdxDeb+ST->hstNbItems-1)&(SYNCH_HST_SIZE-1);
   SumSyncTime=ST->TimeHst[idxAFin]-ST->TimeHst[idxDeb];

   return (float)(SumSyncTime)/(float)(DgTimerFreq);
}

int  WaitSynch(void *SynchBuff,int *Pos) {
   SynchTime *ST;
   ST=((SynchTime*)(SynchBuff));
   if (DgTimerFreq==0 || ST==NULL) return 0;
   int dpos=ST->LastPos;
   for (;;) {
     Synch(SynchBuff,Pos);
     if ((int)(ST->LastPos)>dpos) break;
   }
   return 1;
}

void DgDelay(unsigned int ms) {
    unsigned int timeout = DgTime + ((ms * DgTimerFreq) / 1000);

    while (DgTime < timeout) {
        sched_yield();
    }
}







