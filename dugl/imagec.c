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
#include "DUGL.h"
#include "intrdugl.h"


//*********************** IMAGE
int  LoadMemPCX(DgSurf **S,void *In,void *PalBGR1024,int SizeIn)
{	HeadPCX hpcx;
	char PalRGB[768];
	int ResHz,ResVt,i;

    memcpy(&hpcx,In,sizeof(HeadPCX));
	if (hpcx.Sign!=0xa || hpcx.Ver<5 || hpcx.BitPixel!=8) return 0;
	ResHz=hpcx.X2-hpcx.X1+1;
	ResVt=hpcx.Y2-hpcx.Y1+1;
	memcpy(&PalRGB,&((char*)(In))[SizeIn-768],768);
	for (i=0;i<256;i++) {
	   ((char*)(PalBGR1024))[i*4]=PalRGB[i*3+2];
	   ((char*)(PalBGR1024))[i*4+1]=PalRGB[i*3+1];
	   ((char*)(PalBGR1024))[i*4+2]=PalRGB[i*3];
	  }
	if (!CreateSurf(S,ResHz,ResVt,8))
        return 0;
	if (hpcx.Comp==1)
	   InRLE(In+sizeof(HeadPCX), (unsigned char *)((*S)->rlfb), (*S)->SizeSurf);
	else
        return 0;
	return 1;
}

int  LoadPCX(DgSurf **S,const char *Fname,void *PalBGR1024)
{	FILE *InPCX;
	HeadPCX hpcx;
	void *BuffIn = NULL;
	char PalRGB[768];
	int FinIn,ResHz,ResVt,i;

	if ((InPCX=fopen(Fname,"rb"))==NULL) return 0;
	fread(&hpcx,sizeof(HeadPCX),1,InPCX);
	if (hpcx.Sign!=0xa || hpcx.Ver<5 || hpcx.BitPixel!=8)
	  { fclose(InPCX); return 0; }
	ResHz=hpcx.X2-hpcx.X1+1;
	ResVt=hpcx.Y2-hpcx.Y1+1;
	fseek(InPCX,-769,SEEK_END);
	fseek(InPCX,1,SEEK_CUR);
	fread(&PalRGB,768,1,InPCX);
	for (i=0;i<256;i++) {
	   ((char*)(PalBGR1024))[i*4]=PalRGB[i*3+2];
	   ((char*)(PalBGR1024))[i*4+1]=PalRGB[i*3+1];
	   ((char*)(PalBGR1024))[i*4+2]=PalRGB[i*3];
	}
	FinIn=ftell(InPCX);
	if (!CreateSurf(S,ResHz,ResVt,8)) {
        fclose(InPCX); return 0;
	}
  	fseek(InPCX,sizeof(HeadPCX),SEEK_SET);
	if (hpcx.Comp==1) {
		BuffIn=malloc(FinIn-sizeof(HeadPCX)+1);
		if (BuffIn==NULL) {
			DestroySurf(*S);
			fclose(InPCX);
			return 0;
		}
		fread(BuffIn,FinIn-sizeof(HeadPCX)+1,1,InPCX);
		InRLE(BuffIn,(unsigned char *)(*S)->rlfb,ResHz*ResVt);
	 }
	 else {
	   fclose(InPCX);
	   return 0;
	 }
   	free(BuffIn);
	fclose(InPCX);
	return 1;
}

int  SavePCX(DgSurf *S,const char *Fname,void *PalBGR1024)
{	FILE *OutPCX;
	HeadPCX hpcx;
	void *BuffOut;
	char PalRGB[768];
	int FinIn,ResHz,ResVt,i;

	for (i=0;i<256;i++) {
	   PalRGB[i*3]=(((char*)(PalBGR1024))[i*4+2]);
	   PalRGB[i*3+1]=(((char*)(PalBGR1024))[i*4+1]);
	   PalRGB[i*3+2]=(((char*)(PalBGR1024))[i*4]);
	}
	hpcx.Sign=0xa; hpcx.Ver=5; hpcx.BitPixel=8;
	hpcx.X1=hpcx.Y1=0;
	hpcx.X2=S->ResH-1;	hpcx.Y2=S->ResV-1;
	hpcx.ResHzDPI=S->ResH; 	hpcx.ResVtDPI=S->ResV;
	hpcx.ResHz=S->ResH;	hpcx.ResVt=S->ResV;
	hpcx.TypePal=1;
	hpcx.NbPlan=1;
	hpcx.OctLgImg=S->ResH;
	hpcx.resv=0;
	for (i=0;i<54;i++) hpcx.resv2[i]=0;
	memcpy(&hpcx.Pal,&PalRGB,48);

	i=SizeOutRLE((void*)(S->rlfb),S->SizeSurf,S->ResH);
	hpcx.Comp=1;
	BuffOut=(void*) malloc(i);
	if (BuffOut==NULL) return 0;
	OutPCX=fopen(Fname,"wb");
	if (OutPCX==NULL) return 0;
	fwrite(&hpcx,sizeof(HeadPCX),1,OutPCX);
	OutRLE(BuffOut,(void*)(S->rlfb),S->SizeSurf,S->ResH);
	fwrite(BuffOut,i,1,OutPCX);
	i=0xc;
	fwrite(&i,1,1,OutPCX);
	fwrite(PalRGB,768,1,OutPCX);
	free(BuffOut); fclose(OutPCX);
	return 1;
}

int  SaveMemPCX(DgSurf *S,void *Out,void *PalBGR1024)
{	HeadPCX hpcx;
	char PalRGB[768];
	int PtrOut,ResHz,ResVt,i;

	for (i=0;i<256;i++) {
	   PalRGB[i*3]=(((char*)(PalBGR1024))[i*4+2]);
	   PalRGB[i*3+1]=(((char*)(PalBGR1024))[i*4+1]);
	   PalRGB[i*3+2]=(((char*)(PalBGR1024))[i*4]);
	  }
	hpcx.Sign=0xa; hpcx.Ver=5; hpcx.BitPixel=8;
	hpcx.X1=hpcx.Y1=0;
	hpcx.X2=S->ResH-1;	hpcx.Y2=S->ResV-1;
	hpcx.ResHzDPI=S->ResH;	hpcx.ResVtDPI=S->ResV;
	hpcx.ResHz=S->ResH;	hpcx.ResVt=S->ResV;
	hpcx.TypePal=1;
	hpcx.NbPlan=1;
	hpcx.OctLgImg=S->ResH;
	hpcx.resv=0;
	for (i=0;i<54;i++) hpcx.resv2[i]=0;
	memcpy(&hpcx.Pal,&PalRGB,48);
	i=SizeOutRLE((void*)(S->rlfb),S->SizeSurf,S->ResH);
	hpcx.Comp=1;
	memcpy(Out,&hpcx,(PtrOut=sizeof(HeadPCX)));
	OutRLE(Out+PtrOut,(void*)(S->rlfb),S->SizeSurf,S->ResH);
	PtrOut+=i;
	((char*)(Out+PtrOut))[i]=0xc;
	PtrOut++;
	memcpy(Out+PtrOut,&PalRGB,768);
	return 1;
}

int  SizeSavePCX(DgSurf *S)
{	return (SizeOutRLE((void*)(S->rlfb),S->SizeSurf,S->ResH)+128+769);
}

// GIF

int  LoadMemGIF(DgSurf **S,void *In,void *PalBGR1024,int SizeIn)
{	HeadGIF hgif;
	ExtBlock ExtBGif;
	DescImgGIF descimg;
	void *BuffIn,*BuffS,*BuffD,*BuffLZW;
	char PalRGB[768];
	unsigned char SizeExt,BuffExt[255],SizeBl;
	int OldCurInGIF,CurInGIF,ResHz,ResVt,i,j;

	if (SizeIn<sizeof(HeadGIF)) return 0;
	memcpy(&hgif,In,sizeof(HeadGIF));
	CurInGIF=sizeof(HeadGIF);
	if (hgif.Sign!='8FIG' || (hgif.IndicRes&7)!=7) return 0;
	if ((sizeof(HeadGIF)+768)>SizeIn) return 0;
	if (hgif.IndicRes&128) {
           memcpy(&PalRGB,In+CurInGIF,768);
           CurInGIF+=768;
	}
	for (;;) {
	   memcpy(&ExtBGif,In+CurInGIF,sizeof(ExtBlock));
	   if (ExtBGif.SignExt!='!') break;
	   CurInGIF+=sizeof(ExtBlock);
	   SizeExt=ExtBGif.Size;
	   for (;;) {
              if ((CurInGIF+=SizeExt)>SizeIn) return 0;
              memcpy(&SizeExt,In+CurInGIF,1); CurInGIF++;
	      if (SizeExt==0) break;
	   }
	}
	memcpy(&descimg,In+CurInGIF,sizeof(DescImgGIF));
	CurInGIF+=sizeof(DescImgGIF);
	if (CurInGIF>SizeIn) return 0;
	if (descimg.Sign!=','/* || (descimg.Indicateur&7)!=7*/)
	   return 0;

	if (descimg.Indicateur&128) {
           if (CurInGIF+768>SizeIn) return 0;
           memcpy(&PalRGB,(void*)(In+CurInGIF),768);
           CurInGIF+=768;
          }
	for (i=0;i<256;i++) {
	   ((char*)(PalBGR1024))[i*4]=PalRGB[i*3+2];
	   ((char*)(PalBGR1024))[i*4+1]=PalRGB[i*3+1];
	   ((char*)(PalBGR1024))[i*4+2]=PalRGB[i*3];
	  }
	ResHz=descimg.ResHz; ResVt=descimg.ResVt;
	if (!CreateSurf(S,ResHz,ResVt,8))
        return 0;
	if ((BuffIn=malloc(SizeIn+1-CurInGIF))==NULL) return 0;
// Preparation du buffer
	SizeBl=((unsigned char*)(In+CurInGIF+1))[0];
	memcpy(BuffIn,In+CurInGIF,SizeBl+2);
	BuffD=BuffIn+SizeBl+2;
	BuffS=In+CurInGIF+SizeBl+2;
	for (;;) {
	   SizeBl=((unsigned char*)(BuffS))[0];
	   if (SizeBl==0) break;
	   BuffS++;
	   memcpy(BuffD,BuffS,SizeBl);
	   BuffS+=SizeBl;
	   BuffD+=SizeBl;
	  }
	InLZW(BuffIn+2, (void *)((*S)->rlfb));
	free(BuffIn);
	return 1;
}


int  LoadGIF(DgSurf **S,const char *Fname,void *PalBGR1024)
{	FILE *InGIF;
	HeadGIF hgif;
	ExtBlock ExtBGif;
	DescImgGIF descimg;
	void *BuffIn,*BuffS,*BuffD,*BuffLZW;
	char PalRGB[768];
	unsigned char SizeExt,BuffExt[255],SizeBl;
	int FinInGIF,DebInGIF,CurInGIF,ResHz,ResVt,i,j;

	if ((InGIF=fopen(Fname,"rb"))==NULL) {
        printf("no file\n");
        return 0;
	}
	fread(&hgif,sizeof(HeadGIF),1,InGIF);
	if (hgif.Sign!='8FIG' || (hgif.IndicRes&7)!=7)
	  { fclose(InGIF); return 0; }
	if (hgif.IndicRes&128) fread(&PalRGB,768,1,InGIF);
   	CurInGIF=ftell(InGIF);
	for (;;) {
	   CurInGIF=ftell(InGIF);
	   fread(&ExtBGif,sizeof(ExtBlock),1,InGIF);
	   if (ExtBGif.SignExt!='!') break;
	   SizeExt=ExtBGif.Size;
	   for (;;) {
	      fread(&BuffExt,SizeExt,1,InGIF);
	      fread(&SizeExt,1,1,InGIF);
	      if (SizeExt==0) break;
	   }
	}
	fseek(InGIF,CurInGIF,SEEK_SET);
	fread(&descimg,sizeof(DescImgGIF),1,InGIF);
	if (descimg.Sign!=','/* || (descimg.Indicateur&7)!=7*/) {
        printf("no sign\n");
        return 0;
    }
	if (descimg.Indicateur&128) fread(&PalRGB,768,1,InGIF);
	for (i=0;i<256;i++) {
	   ((char*)(PalBGR1024))[i*4]=PalRGB[i*3+2];
	   ((char*)(PalBGR1024))[i*4+1]=PalRGB[i*3+1];
	   ((char*)(PalBGR1024))[i*4+2]=PalRGB[i*3];
	  }
	ResHz=descimg.ResHz; ResVt=descimg.ResVt;
	if (!CreateSurf(S,ResHz,ResVt,8)) {
        fclose(InGIF);
        printf("no mem\n");
        return 0;
    }
	DebInGIF=ftell(InGIF);
	fseek(InGIF,0,SEEK_END);
	FinInGIF=ftell(InGIF);
	fseek(InGIF,DebInGIF,SEEK_SET);
	BuffIn=malloc(FinInGIF-DebInGIF+4);

	if (BuffIn==NULL) {
        DestroySurf(*S);
        fclose(InGIF);
        printf("no mem2\n");
        return 0;
    }
	fread(BuffIn,FinInGIF-DebInGIF+1,1,InGIF);
// Preparation du buffer
	SizeBl=((unsigned char*)(BuffIn+1))[0];
	BuffD=BuffS=BuffIn+SizeBl+2;
	for (;;) {
	   SizeBl=((unsigned char*)(BuffS))[0];
	   if (SizeBl==0) break;
	   BuffS++;
	   memcpy(BuffD,BuffS,SizeBl);
	   BuffS+=SizeBl;
	   BuffD+=SizeBl;
	  }
	InLZW(BuffIn+2, (void *)((*S)->rlfb));
	free(BuffIn); fclose(InGIF);
	return 1;
}


int LoadGIF16(DgSurf **S16,char *filename) {
  char tmpBGRA[1024];
  DgSurf *SGIF8bpp = NULL;
  if (LoadGIF(&SGIF8bpp,filename,tmpBGRA)==0)
	return 0;
  if (CreateSurf(S16,SGIF8bpp->ResH, SGIF8bpp->ResV,16)==0) {
    DestroySurf(SGIF8bpp);
    return 0;
  }
  ConvSurf8ToSurf16Pal(*S16,SGIF8bpp,tmpBGRA);
  DestroySurf(SGIF8bpp);

  return 1;
}


// BMP
int  LoadMemBMP(DgSurf **S,void *In,void *PalBGR1024,int SizeIn) {
	FILE *InBMP;
	int i,j,padd,CurInBMP;
	HeadBMP hbmp;
	InfoBMP ibmp;
	char *Linedata;

	if (In==NULL || SizeIn<=sizeof(HeadBMP)+sizeof(InfoBMP)+1024)
	  return 0;

	bzero(&hbmp,sizeof(HeadBMP));
	bzero(&ibmp,sizeof(InfoBMP));

	// read head
	memcpy(&hbmp,In,sizeof(HeadBMP));

	// verify signature and data offset
	if (hbmp.Sign!='MB' ||
	    (hbmp.DataOffset<sizeof(HeadBMP)+sizeof(InfoBMP)) ||
	     hbmp.DataOffset>hbmp.SizeFile)
	  return 0;

	// read info
	memcpy(&ibmp,In+sizeof(HeadBMP),sizeof(InfoBMP));

	if (ibmp.ImgWidth==0 || ibmp.ImgHeight==0 ||
	    ibmp.BitsPixel!=8 || ibmp.Compression!=0)
	   return 0;

	// no mem
	if (!CreateSurf(S,ibmp.ImgWidth,ibmp.ImgHeight,8))
	   return 0;

	// copy palette
	memcpy(PalBGR1024,In+sizeof(HeadBMP)+sizeof(InfoBMP),1024);

	// seek to data
	CurInBMP=hbmp.DataOffset;
	// read data
	padd=ibmp.ImgWidth&3;
	for (j=ibmp.ImgHeight-1;j>=0;j--) {
	  if (CurInBMP+ibmp.ImgWidth>SizeIn) break;
	  Linedata=(char*)((*S)->rlfb+(j*ibmp.ImgWidth));
	  memcpy(Linedata,In+CurInBMP,ibmp.ImgWidth);
	  CurInBMP+=ibmp.ImgWidth;
	  if (padd) CurInBMP+=(4-padd);
	}

	return 1;
}
int  LoadBMP(DgSurf **S,const char *Fname,void *PalBGR1024) {
	FILE *InBMP;
	int i,j,padd;
	HeadBMP hbmp;
	InfoBMP ibmp;
	char *Linedata;

	if ((InBMP=fopen(Fname,"rb"))==NULL) return 0;

	bzero(&hbmp,sizeof(HeadBMP));
	bzero(&ibmp,sizeof(InfoBMP));

	// read head
	fread(&hbmp,sizeof(HeadBMP),1,InBMP);

	// verify signature and data offset
	if (hbmp.Sign!='MB' ||
	    (hbmp.DataOffset<sizeof(HeadBMP)+sizeof(InfoBMP)) ||
	     hbmp.DataOffset>hbmp.SizeFile)
	  { fclose(InBMP); return 0; }

	// read info
	fread(&ibmp,sizeof(InfoBMP),1,InBMP);

	if (ibmp.ImgWidth==0 || ibmp.ImgHeight==0 ||
	    ibmp.BitsPixel!=8 || ibmp.Compression!=0) {
	   fclose(InBMP); return 0;
	}

	// no mem
	if (!CreateSurf(S,ibmp.ImgWidth,ibmp.ImgHeight,8)) {
	   fclose(InBMP); return 0;
	}

	// read palette
	fread(PalBGR1024,1024,1,InBMP);

	// seek to data
	fseek(InBMP,hbmp.DataOffset,SEEK_SET);
	// read data
	padd=ibmp.ImgWidth&3;
	for (j=ibmp.ImgHeight-1;j>=0;j--) {
	  Linedata=(char*)((*S)->rlfb+(j*ibmp.ImgWidth));
	  fread(Linedata,ibmp.ImgWidth,1,InBMP);
	  if (padd) fseek(InBMP,4-padd,SEEK_CUR);
	}

	fclose(InBMP);
	return 1;
}
int  SaveMemBMP(DgSurf *S,void *Out,void *PalBGR1024) {
	int irow,j,padd,BfPos,sizeBMPFile,sizeline;
	HeadBMP *hbmp;
	InfoBMP *ibmp;
	void *palBMP,*lineOut;

	char *Linedata;
	unsigned char *tempLine;

	if ((sizeBMPFile=SizeSaveBMP(S))==0) return 0;

	if (Out==NULL) return 0;

	hbmp=(HeadBMP*)(Out);
	ibmp=(InfoBMP*)(Out+sizeof(HeadBMP));
	palBMP=(Out+sizeof(HeadBMP)+sizeof(InfoBMP));

	bzero(hbmp,sizeof(HeadBMP));
	bzero(ibmp,sizeof(InfoBMP));

	// init header
	hbmp->Sign='MB';
	hbmp->SizeFile=sizeBMPFile;
	hbmp->DataOffset=sizeof(HeadBMP)+sizeof(InfoBMP)+1024;
	// init info
	ibmp->SizeInfo=sizeof(InfoBMP);
	ibmp->ImgWidth=S->ResH;  ibmp->ImgHeight=S->ResV;
	ibmp->Planes=1;
	ibmp->BitsPixel=8;

	// write palette
	memcpy(palBMP,PalBGR1024,1024);

	// compute padd
	padd=(ibmp->ImgWidth)&3;
	if (padd>0) padd=4-padd;
	sizeline=ibmp->ImgWidth+padd;
	// alloc temporary
	tempLine = (unsigned char*)malloc(sizeline);
	// clear
	bzero(tempLine,sizeline);

	// write data
	if (tempLine!=NULL) {
	  lineOut=Out+hbmp->DataOffset;
	  for (j=ibmp->ImgHeight-1;j>=0;j--) {
	    Linedata=(char*)(S->rlfb+(j*ibmp->ImgWidth));
	    memcpy(tempLine,Linedata,ibmp->ImgWidth);
	    memcpy(lineOut,tempLine,sizeline);
	    lineOut+=sizeline;
	  }
	  free(tempLine);
	}

	//fclose(OutBMP);
	return 1;
}

int  SaveBMP(DgSurf *S,const char *Fname,void *PalBGR1024) {
	FILE *OutBMP;
	int irow,j,padd,BfPos,sizeBMPFile,sizeline;
	HeadBMP hbmp;
	InfoBMP ibmp;
	short *Linedata;
	unsigned char *tempLine;

	if ((sizeBMPFile=SizeSaveBMP(S))==0) return 0;

	if ((OutBMP=fopen(Fname,"wb"))==NULL) return 0;

	bzero(&hbmp,sizeof(HeadBMP));
	bzero(&ibmp,sizeof(InfoBMP));

	// init header
	hbmp.Sign='MB';
	hbmp.SizeFile=sizeBMPFile;
	hbmp.DataOffset=sizeof(HeadBMP)+sizeof(InfoBMP)+1024;
	// init info
	ibmp.SizeInfo=sizeof(InfoBMP);
	ibmp.ImgWidth=S->ResH;  ibmp.ImgHeight=S->ResV;
	ibmp.Planes=1;
	ibmp.BitsPixel=8;


	// write header
	fwrite(&hbmp,sizeof(HeadBMP),1,OutBMP);
	// write info
	fwrite(&ibmp,sizeof(InfoBMP),1,OutBMP);
	// write palette
	fwrite(PalBGR1024,1024,1,OutBMP);

	// compute padd
	padd=(ibmp.ImgWidth)&3;
	if (padd>0) padd=4-padd;
	sizeline=ibmp.ImgWidth+padd;
	// alloc temporary
	tempLine = (unsigned char*)malloc(sizeline);
	// clear
	bzero(tempLine,sizeline);

	// write data
	if (tempLine!=NULL) {
	  for (j=ibmp.ImgHeight-1;j>=0;j--) {
        Linedata = (short *)(S->rlfb + (j * ibmp.ImgWidth));
	    memcpy(tempLine,Linedata,ibmp.ImgWidth);
	    fwrite(tempLine,sizeline,1,OutBMP);
	  }
	  free(tempLine);
	}

	fclose(OutBMP);
	return 1;
}
int  SizeSaveBMP(DgSurf *S) {
	int padd=0;
	if (S->rlfb==0 || S->BitsPixel!=8 || S->ResH<=0 || S->ResV<=0)
	  return 0;
	padd=(S->ResH)&3;
	if (padd>0) padd=4-padd;
	return sizeof(HeadBMP)+sizeof(InfoBMP)+(((S->ResH)+padd)*S->ResV)+1024;
}

int  LoadMemBMP16(DgSurf **S,void *In,int SizeIn) {
	int irow,j,padd,CurInBMP,BfPos;
	HeadBMP hbmp;
	InfoBMP ibmp;
	unsigned short *Linedata;
	unsigned char *tempLine;

	if (In==NULL || SizeIn<=sizeof(HeadBMP)+sizeof(InfoBMP))
	  return 0;

	bzero(&hbmp,sizeof(HeadBMP));
	bzero(&ibmp,sizeof(InfoBMP));

	// read head
	memcpy(&hbmp,In,sizeof(HeadBMP));

	// verify signature and data offset
	if (hbmp.Sign!='MB' ||
	    (hbmp.DataOffset<sizeof(HeadBMP)+sizeof(InfoBMP)) ||
	     hbmp.DataOffset>hbmp.SizeFile)
	  return 0;

	// read info
	memcpy(&ibmp,In+sizeof(HeadBMP),sizeof(InfoBMP));


	if (ibmp.ImgWidth==0 || ibmp.ImgHeight==0 ||
	    ibmp.BitsPixel!=24 || ibmp.Compression!=0)
	   return 0;

	// no mem
	if (!CreateSurf(S,ibmp.ImgWidth,ibmp.ImgHeight,16))
	   return 0;


	// seek to data
	CurInBMP=hbmp.DataOffset;
	// read data
	padd=(ibmp.ImgWidth*3)&3;
	for (j=ibmp.ImgHeight-1;j>=0;j--) {
	  if (CurInBMP+ibmp.ImgWidth*3>SizeIn) break;
	  Linedata=(unsigned short*)((*S)->rlfb+(j*ibmp.ImgWidth*2));
	  tempLine=(unsigned char*)(In+CurInBMP);
	  BfPos=0;
	  for (irow=0;irow<ibmp.ImgWidth;irow++) {
	    Linedata[irow]=(tempLine[BfPos]>>3)|((tempLine[BfPos+1]>>2)<<5)|
		((tempLine[BfPos+2]>>3)<<11);
	      BfPos+=3;
	  }

	  CurInBMP+=ibmp.ImgWidth*3;
	  if (padd) CurInBMP+=(4-padd);
	}

	return 1;
}
int  LoadBMP16(DgSurf **S,const char *Fname) {
	FILE *InBMP;
	int irow,j,padd,BfPos;
	HeadBMP hbmp;
	InfoBMP ibmp;
	short *Linedata;
	unsigned char *tempLine;

	if ((InBMP=fopen(Fname,"rb"))==NULL) return 0;

	bzero(&hbmp,sizeof(HeadBMP));
	bzero(&ibmp,sizeof(InfoBMP));

	// read head
	fread(&hbmp,sizeof(HeadBMP),1,InBMP);

	// verify signature and data offset
	if (hbmp.Sign!='MB' ||
	    (hbmp.DataOffset<sizeof(HeadBMP)+sizeof(InfoBMP)) ||
	     hbmp.DataOffset>hbmp.SizeFile)
	  { fclose(InBMP); return 0; }

	// read info
	fread(&ibmp,sizeof(InfoBMP),1,InBMP);

	if (ibmp.ImgWidth==0 || ibmp.ImgHeight==0 ||
	    ibmp.BitsPixel!=24 || ibmp.Compression!=0) {
	   fclose(InBMP); return 0;
	}

	// no mem
	if (!CreateSurf(S,ibmp.ImgWidth,ibmp.ImgHeight,16)) {
	   fclose(InBMP); return 0;
	}

	// seek to data
	fseek(InBMP,hbmp.DataOffset,SEEK_SET);
	// alloc temporary
	tempLine = (unsigned char*)malloc(ibmp.ImgWidth*3);

	// read data
	if (tempLine!=NULL) {
	  padd=(ibmp.ImgWidth*3)&3;
	  for (j=ibmp.ImgHeight-1;j>=0;j--) {
	    Linedata=(short*)((*S)->rlfb+(j*ibmp.ImgWidth*2));
	    fread(tempLine,ibmp.ImgWidth*3,1,InBMP);
	    if (padd) fseek(InBMP,4-padd,SEEK_CUR);
	    BfPos=0;
	    for (irow=0;irow<ibmp.ImgWidth;irow++) {
	      Linedata[irow]=(tempLine[BfPos]>>3)|((tempLine[BfPos+1]>>2)<<5)|
		((tempLine[BfPos+2]>>3)<<11);
	      BfPos+=3;
	    }
	  }
	  free(tempLine);
	}

	fclose(InBMP);
	return 1;

}
int  SaveMemBMP16(DgSurf *S,void *Out) {
	int irow,j,padd,BfPos,sizeBMPFile,sizeline;
	HeadBMP *hbmp;
	InfoBMP *ibmp;
	short *Linedata;
	unsigned char *tempLine;

	if (Out==NULL || (sizeBMPFile=SizeSaveBMP16(S))==0) return 0;

	hbmp=(HeadBMP*)(Out);
	ibmp=(InfoBMP*)(Out+sizeof(HeadBMP));

	bzero(hbmp,sizeof(HeadBMP));
	bzero(ibmp,sizeof(InfoBMP));

	// init header
	hbmp->Sign='MB';
	hbmp->SizeFile=sizeBMPFile;
	hbmp->DataOffset=sizeof(HeadBMP)+sizeof(InfoBMP);
	// init info
	ibmp->SizeInfo=sizeof(InfoBMP);
	ibmp->ImgWidth=S->ResH;  ibmp->ImgHeight=S->ResV;
	ibmp->Planes=1;
	ibmp->BitsPixel=24;

	// compute padd
	padd=(ibmp->ImgWidth*3)&3;
	if (padd>0) padd=4-padd;
	sizeline=ibmp->ImgWidth*3+padd;
	tempLine =
	  (unsigned char*)(Out+sizeof(HeadBMP)+sizeof(InfoBMP));

	// read data
	if (tempLine!=NULL) {
	  for (j=ibmp->ImgHeight-1;j>=0;j--) {
	    Linedata=(short*)(S->rlfb+(j*ibmp->ImgWidth*2));
	    BfPos=0;
	    // BGR 16 -> RGB 24
	    for (irow=0;irow<S->ResH;irow++) {
	      tempLine[BfPos]=(Linedata[irow]&0x1f)<<3;
	      tempLine[BfPos+1]=(Linedata[irow]&0x7e0)>>3;
	      tempLine[BfPos+2]=(Linedata[irow]&0xf800)>>8;
	      BfPos+=3;
	    }
	    tempLine+=sizeline;
	  }
	}

	return 1;
}

int  SaveBMP16(DgSurf *S,const char *Fname) {
	FILE *OutBMP;
	int irow,j,padd,BfPos,sizeBMPFile,sizeline;
	HeadBMP hbmp;
	InfoBMP ibmp;
	short *Linedata;
	unsigned char *tempLine;

	if ((sizeBMPFile=SizeSaveBMP16(S))==0) return 0;

	if ((OutBMP=fopen(Fname,"wb"))==NULL) return 0;

	bzero(&hbmp,sizeof(HeadBMP));
	bzero(&ibmp,sizeof(InfoBMP));

	// init header
	hbmp.Sign='MB';
	hbmp.SizeFile=sizeBMPFile;
	hbmp.DataOffset=sizeof(HeadBMP)+sizeof(InfoBMP);
	// init info
	ibmp.SizeInfo=sizeof(InfoBMP);
	ibmp.ImgWidth=S->ResH;  ibmp.ImgHeight=S->ResV;
	ibmp.Planes=1;
	ibmp.BitsPixel=24;


	// write header
	fwrite(&hbmp,sizeof(HeadBMP),1,OutBMP);
	// read info
	fwrite(&ibmp,sizeof(InfoBMP),1,OutBMP);

	// compute padd
	padd=(ibmp.ImgWidth*3)&3;
	if (padd>0) padd=4-padd;
	sizeline=ibmp.ImgWidth*3+padd;
	// alloc temporary
	tempLine = (unsigned char*)malloc(sizeline);

	// read data
	if (tempLine!=NULL) {
	  for (j=ibmp.ImgHeight-1;j>=0;j--) {
	    Linedata=(short*)(S->rlfb+(j*ibmp.ImgWidth*2));
	    //if (padd) fseek(InBMP,4-padd,SEEK_CUR);
	    BfPos=0;
	    // BGR 16 -> RGB 24
	    for (irow=0;irow<S->ResH;irow++) {
	      tempLine[BfPos]=(Linedata[irow]&0x1f)<<3;
	      tempLine[BfPos+1]=(Linedata[irow]&0x7e0)>>3;
	      tempLine[BfPos+2]=(Linedata[irow]&0xf800)>>8;
	      BfPos+=3;
	    }
	    fwrite(tempLine,sizeline,1,OutBMP);
	  }
	  free(tempLine);
	}

	fclose(OutBMP);
	return 1;
}

int  SizeSaveBMP16(DgSurf *S) {
	int padd=0;
	if (S->rlfb==0 || S->BitsPixel!=16 || S->ResH<=0 || S->ResV<=0)
	  return 0;
	padd=(S->ResH*3)&3;
	if (padd>0) padd=4-padd;
	return sizeof(HeadBMP)+sizeof(InfoBMP)+(((S->ResH*3)+padd)*S->ResV);
}
