#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <unistd.h>
#include <string.h>
#include <dugl.h>
#include "intrdugl.h"
#include "dgui.h"


//*********** class WinHandler *********************************

WinHandler::WinHandler(int resh,int resv,char BitsPixel,int backCol,void *PalBGR1024) {
   WinList=new ListWin;
   FlyList=new ListFly;
   m_GraphCtxt=new CGraphContext(BitsPixel,PalBGR1024);
   CurWinNode=NULL;
   BackCol=backCol;
   MoveCurWin=0;
   CreateSurf(&S,resh,resv,BitsPixel);
   Draw();
}

WinHandler::~WinHandler() {
   if (WinList!=NULL) delete WinList;
   if (FlyList!=NULL) delete FlyList;
   DestroySurf(S);
   if (m_GraphCtxt!=NULL) delete m_GraphCtxt;
}

void WinHandler::Scan() {
   DgSurf SaveSurf;
   NodeFly *FlyParcNode=NULL,*FlyFoundNode=NULL;
   DgGetCurSurf(&SaveSurf);

   GetAscii(&Ascii,&AsciiFLAG);
   GetKey(&Key,&KeyFLAG);

   if (!FlyList->Empty()) {
     FlyMsUp=0;
     FlyParcNode=FlyList->FirstNode;
     if (FlyParcNode!=NULL) {
        do {
          if (m_GraphCtxt->In(FlyParcNode->Item->X1,FlyParcNode->Item->Y1,
	         FlyParcNode->Item->X2,FlyParcNode->Item->Y2,MsX,MsY)) {
	    FlyMsUp=1;
	    FlyFoundNode=FlyParcNode;
	  }
          FlyParcNode=FlyParcNode->Next;
        } while (FlyParcNode!=NULL);
     }
     if (FlyFoundNode) FlyFoundNode->Item->Scan();
     else
       if (CurWinNode && CurWinNode->Item->ActivMenu)
         CurWinNode->Item->MyMenu->Scan();

/*     if (FlyFoundNode!=NULL)
       m_GraphCtxt->WinFreeMs=0;
     else
       m_GraphCtxt->WinFreeMs=1;*/

     if (FlyFoundNode!=FlyList->Nodes && FlyList->Nodes)
       FlyList->Nodes->Item->Scan();

     if (((MsButton&MS_LEFT_BUTT) || (MsButton&MS_RIGHT_BUTT)) && m_GraphCtxt->WinFreeMs &&
         (!FlyFoundNode) && (!(FlyMsLftDwn || FlyMsRghDwn)))
       DelAllFly();
     if (FlyList->Empty()) {
       if (CurWinNode)
         if (CurWinNode->Item->MyMenu && CurWinNode->Item->ActivMenu && m_GraphCtxt->WinFreeMs) {
	   CurWinNode->Item->ActivMenu=0;
	   CurWinNode->Item->MyMenu->choix=-1;
	   CurWinNode->Item->MyMenu->Redraw();
	 }
     }
   }
   //else {
     if (CurWinNode && (!CurWinNode->Item->ActivMenu)) {
       if ((MsButton&MS_LEFT_BUTT)&& MoveCurWin)
         CurWinNode->Item->Move(MsX+DMsX,MsY+DMsY);
       else
         if (MoveCurWin) { m_GraphCtxt->WinFreeMs=1; MoveCurWin=0; }
       if ((!MoveCurWin) && (MsButton&MS_LEFT_BUTT) && m_GraphCtxt->WinFreeMs &&
           m_GraphCtxt->In(CurWinNode->Item->X+3,CurWinNode->Item->Y2-5-FntHaut,
              CurWinNode->Item->X2-3,CurWinNode->Item->Y2-3,MsX,MsY)) {
         DMsX=CurWinNode->Item->X-MsX;
         DMsY=CurWinNode->Item->Y-MsY;
         MoveCurWin=1; m_GraphCtxt->WinFreeMs=0;
       }
     }
     // if mouse isn't up a fly obj and we have a current win
     if (CurWinNode && FlyFoundNode==NULL) CurWinNode->Item->Scan();
   //}
   DgSetCurSurf(&SaveSurf);
}

void WinHandler::Draw() {
   DgSurf SaveSurf;
   DgGetCurSurf(&SaveSurf);
   DgSetCurSurf(S);
   m_GraphCtxt->Clear(BackCol);
   DgSetCurSurf(&SaveSurf);
}

void WinHandler::Redraw(int x1,int y1,int x2,int y2) {
   DgSurf SaveSurf;
   NodeWin *ParcNode,*Next;
   NodeFly *FlyParcNode;
   DgView V1;

   if (x1>S->MaxX || x2<S->MinX || y1>S->MaxY || y2<S->MinY) return;
   DgGetCurSurf(&SaveSurf);
   DgSetCurSurf(S);
   GetSurfView(S,&V1);
   V1.MinX=x1; V1.MinY=y1;
   V1.MaxX=x2; V1.MaxY=y2;
   SetSurfInView(&CurSurf,&V1);
   GetSurfView(&CurSurf,&V1);
   m_GraphCtxt->bar(x1,y1,x2,y2,BackCol);
   ParcNode=WinList->FirstNode;
   if (ParcNode!=NULL) {
      do {
        if (ParcNode!=CurWinNode)
	  if (!( ParcNode->Item->X2<x1 || ParcNode->Item->X>x2 ||
	         ParcNode->Item->Y2<y1 || ParcNode->Item->Y>y2 ))
	    if (ParcNode->Item->Visible) {
	      SetSurfView(&CurSurf,&V1);
	      m_GraphCtxt->PutSurf(ParcNode->Item->S,ParcNode->Item->X,
	      	      ParcNode->Item->Y,NORM_PUT);
	    }
        ParcNode=ParcNode->Next;
      } while (ParcNode!=NULL);
   }
   if ((ParcNode=CurWinNode)) {
     if (!( ParcNode->Item->X2<x1 || ParcNode->Item->X>x2 ||
            ParcNode->Item->Y2<y1 || ParcNode->Item->Y>y2 ))
       if (ParcNode->Item->Visible) {
         SetSurfView(&CurSurf,&V1);
         m_GraphCtxt->PutSurf(ParcNode->Item->S,ParcNode->Item->X,
	          ParcNode->Item->Y,NORM_PUT);
       }
   }
   FlyParcNode=FlyList->FirstNode;
   if (FlyParcNode!=NULL) {
      do {
        if (!( FlyParcNode->Item->X2<x1 || FlyParcNode->Item->X1>x2 ||
	       FlyParcNode->Item->Y2<y1 || FlyParcNode->Item->Y1>y2 )) {
	  SetSurfView(&CurSurf,&V1);
	  FlyParcNode->Item->Draw();
	}
        FlyParcNode=FlyParcNode->Next;
      } while (FlyParcNode!=NULL);
   }
   DgSetCurSurf(&SaveSurf);
}

void WinHandler::DrawSurf(DgSurf *DestSurf) {
   DgSurf SaveSurf;
   DgGetCurSurf(&SaveSurf);
   DgSetCurSurf(DestSurf);
   m_GraphCtxt->PutSurf(S,0,0,NORM_PUT);
   if (CurWinNode) CurWinNode->Item->DrawSurf(DestSurf);
   DgSetCurSurf(&SaveSurf);
}

NodeWin * WinHandler::AddMainWin(MainWin *MainW) {
   return (WinList->Add(MainW));
}

void WinHandler::DelAfterFlyObj(FlyObj *Fly) {
  FlyList->DeleteAfter(Fly->MyNode);
}

void WinHandler::DelAllFly() {
  FlyList->ClearAll();
}

//****** class MainWin *******************
//****************************************

MainWin::MainWin(int XWin,int YWin,int Larg,int Haut,char *label,
         WinHandler *WH) {
   if (Haut>GetMaxResVSetSurf()) return;
   if (!CreateSurf(&S,Larg,Haut,WH->S->BitsPixel)) return;
   ChList = new List;
   if (label) Label=label;
   else Label="";
   MWWinHand=WH;
   m_GraphCtxt=WH->m_GraphCtxt;
   X=XWin; Y=YWin;
   X2=X+(LargHz=Larg); Y2=Y+(HautVt=Haut);
   DefaultActiv=1;
   ValidDef=0; ActivMenu=0; MyMenu=NULL;
   MWNode=WH->AddMainWin(this);
   if (!(WH->CurWinNode)) {
     Visible=Focus=1;
     WH->CurWinNode=MWNode;
   } else { Visible=Focus=0; }
   EchapNode=DefNode=CurFocusNode=NULL;
   VActiv.OrgX=VActiv.OrgY=3;
   VActiv.MinX=VActiv.MinY=0;
   VActiv.MaxX=S->ResH-7;
   VActiv.MaxY=S->ResV-11-FntHaut;
   MainWin::Draw();
   if (Focus) MWWinHand->Redraw(X,Y,X2,Y2);
}

MainWin::~MainWin() {
   if (ChList) delete ChList;
   MWWinHand->WinList->Delete(MWNode);
   if (MWNode==MWWinHand->CurWinNode) {
     MWWinHand->CurWinNode=NULL; m_GraphCtxt->WinStopScan=1; }
   DestroySurf(S);
   MWWinHand->Redraw(X,Y,X2,Y2);
}

Node * MainWin::AddChild(Child *child) {
   return (ChList->Add(child)); }

void MainWin::Scan() {
   Node *ParcNode;

   Ascii= MWWinHand->Ascii;
   AsciiFLAG= MWWinHand->AsciiFLAG;
   Key= MWWinHand->Key;
   KeyFLAG= MWWinHand->KeyFLAG;

   if (MyMenu) {
     MyMenu->Scan();
     if (ActivMenu) return;
   }
   if ((Key==0xf || Ascii==0x9) && m_GraphCtxt->WinFreeMs) {  // next focus
     MainWin::SetNextFocus();
   }
   if (CurFocusNode)
     if ((!CurFocusNode->Item->CanDefault) && (!DefaultActiv) && DefNode) {
       DefaultActiv=1;
       DefNode->Item->Redraw();
     }
   if (DefNode && DefaultActiv && ValidDef)
     if (DefNode->Item->Visible) {
       ValidDef=0; DefNode->Item->Validate(); return; }
   if (Key==0x1c || Key==0x9c || Ascii==0xd) {  // Enter
     ParcNode=CurFocusNode;
     if (ParcNode) {
       if (!ParcNode->Item->CanDefault) ValidDef=1;
       ParcNode->Item->UnsetFocus();
       ParcNode->Item->Validate(); return; }
     else {
       if (DefNode && DefaultActiv)
         if (DefNode->Item->Visible) {
           DefNode->Item->Validate(); return; }
     }
   }
   if (Key==1 || Ascii==0x1b)   // Echap
     if (EchapNode)
       if (EchapNode->Item->Visible) {
         EchapNode->Item->Validate(); return; }

   if (CurFocusNode) {
     if (CurFocusNode->Item->Visible) CurFocusNode->Item->Scan();
     else CurFocusNode->Item->UnsetFocus();
     if (m_GraphCtxt->WinStopScan) { m_GraphCtxt->WinStopScan=0; return; }
   }
   ChList->SetFirstNode();
   if (ChList->CurNode!=NULL) {
     do {
        if (ChList->CurNode!=CurFocusNode && ChList->CurNode->Item->Visible)
            ChList->CurNode->Item->Scan();
        if (m_GraphCtxt->WinStopScan) { m_GraphCtxt->WinStopScan=0; return; }
	ChList->GoNextNode();
     } while (ChList->CurNode!=NULL);
   }
}

void MainWin::Draw() {
   DgSurf SaveSurf;
   DgView SaveView,V1;
   Node *ParcNode,*Next;

   DgGetCurSurf(&SaveSurf);

   DgSetCurSurf(S);
   m_GraphCtxt->Clear(m_GraphCtxt->WinGris);
   m_GraphCtxt->CadWin(CurSurf.MinX,CurSurf.MinY,CurSurf.MaxX,CurSurf.MaxY);

   V1.OrgX=V1.OrgY=0;
   V1.MinX=CurSurf.MinX+4; V1.MinY=CurSurf.MaxY-5-FntHaut;
   V1.MaxX=CurSurf.MaxX-4; V1.MaxY=CurSurf.MaxY-4;
   if (Focus) {
     m_GraphCtxt->bar(CurSurf.MinX+3,CurSurf.MaxY-5-FntHaut,CurSurf.MaxX-3,CurSurf.MaxY-3,m_GraphCtxt->WinBleuF);
     FntCol=m_GraphCtxt->WinBlanc;
   } else {
     m_GraphCtxt->bar(CurSurf.MinX+3,CurSurf.MaxY-5-FntHaut,CurSurf.MaxX-3,CurSurf.MaxY-3,m_GraphCtxt->WinGrisF);
     FntCol=m_GraphCtxt->WinGris;
   }
   SetSurfView(&CurSurf,&V1);
   ClearText();
   m_GraphCtxt->OutText(Label.StrPtr);

   ParcNode=ChList->Nodes;  //Last Node
   if (ParcNode!=NULL) {
      do {
        SetSurfView(&CurSurf,&VActiv);
         if (ParcNode!=CurFocusNode && ParcNode->Item->Visible)
         ParcNode->Item->Draw();
         ParcNode=ParcNode->Prev;
       } while (ParcNode!=NULL);
     }
   if ((ParcNode=CurFocusNode)) {
     SetSurfView(&CurSurf,&VActiv);
     if (ParcNode->Item->Visible) ParcNode->Item->Draw();
   }
   if (MyMenu) {
     SetSurfView(&CurSurf,&VActiv);
     MyMenu->Draw();
   }
   DgSetCurSurf(&SaveSurf);
}

void MainWin::Redraw(int x1,int y1,int x2,int y2) {
   DgSurf SaveSurf;
   Node *ParcNode,*Next;
   DgView V1;
   int PlusX=X+VActiv.OrgX,PlusY=Y+VActiv.OrgY;

   if (x1>VActiv.MaxX || x2<VActiv.MinX || y1>VActiv.MaxY || y2<VActiv.MinY)
     return;
   DgGetCurSurf(&SaveSurf);
   DgSetCurSurf(S);
   SetSurfView(&CurSurf,&VActiv);
   V1=VActiv;
   V1.MinX=x1; V1.MinY=y1;
   V1.MaxX=x2; V1.MaxY=y2;
   SetSurfInView(&CurSurf,&V1);
   GetSurfView(&CurSurf,&V1);

   m_GraphCtxt->bar(V1.MinX,V1.MinY,V1.MaxX,V1.MaxY,m_GraphCtxt->WinGris);
   ParcNode=ChList->Nodes;
   if (ParcNode!=NULL) {
      do {
        if (ParcNode!=CurFocusNode)
	  if (!( ParcNode->Item->XC2<x1 || ParcNode->Item->XC1>x2 ||
	         ParcNode->Item->YC2<y1 || ParcNode->Item->YC1>y2 ))
	    if (ParcNode->Item->Visible) {
	      SetSurfView(&CurSurf,&V1);
	      ParcNode->Item->Draw();
	    }
        ParcNode=ParcNode->Prev;
      } while (ParcNode!=NULL);
   }
   if ((ParcNode=CurFocusNode)) {
     if (!( ParcNode->Item->XC2<x1 || ParcNode->Item->XC1>x2 ||
            ParcNode->Item->YC2<y1 || ParcNode->Item->YC1>y2 ))
       if (ParcNode->Item->Visible) {
         SetSurfView(&CurSurf,&V1);
         ParcNode->Item->Draw();
       }
   }
   if (MyMenu) {
     if (!( MyMenu->XC2<x1 || MyMenu->XC1>x2 ||
            MyMenu->YC2<y1 || MyMenu->YC1>y2 )) {
         SetSurfView(&CurSurf,&V1);
         MyMenu->Draw();
       }
   }
   MWWinHand->Redraw(V1.MinX+PlusX,V1.MinY+PlusY,V1.MaxX+PlusX,V1.MaxY+PlusY);
   DgSetCurSurf(&SaveSurf);
}

void MainWin::Redraw() {
   MainWin::Draw();
   MWWinHand->Redraw(X,Y,X2,Y2);
}

void MainWin::DrawSurf(DgSurf *DestSurf) {
   DgSurf SaveSurf;
   DgView SaveView;
   Node *ParcNode;

   DgGetCurSurf(&SaveSurf);
   DgSetCurSurf(DestSurf);
   GetSurfView(DestSurf,&SaveView);
   ParcNode=ChList->Nodes;  // last node
   if (ParcNode!=NULL) {
      do {
         if (ParcNode->Item->Visible) {
   	   SetSurfView(DestSurf,&SaveView);
	   ParcNode->Item->DrawSurf(DestSurf);
	 }
         ParcNode=ParcNode->Prev;
       } while (ParcNode!=NULL);
     }
   SetSurfView(DestSurf,&SaveView);
   DgSetCurSurf(&SaveSurf);
}

void MainWin::SetNextFocus() {
   Node *ParcNode;
   if (CurFocusNode) {
     ParcNode=CurFocusNode;
     do {
       if (ParcNode) ParcNode=ParcNode->Next;
       if (!ParcNode) ParcNode=ChList->FirstNode;
       if (ParcNode)
         if (!ParcNode->Item->Visible) ParcNode=ParcNode->Next;
     } while ((!ParcNode->Item->Visible) && ParcNode!=CurFocusNode);
     if (CurFocusNode!=ParcNode)
       ParcNode->Item->SetFocus();
   } else {
     ParcNode=ChList->FirstNode;
     do {
       if (ParcNode)
         if (!ParcNode->Item->Visible) ParcNode=ParcNode->Next;
     } while ((!ParcNode->Item->Visible) && ParcNode!=NULL);
     if (ParcNode) ParcNode->Item->SetFocus();
   }
   MWWinHand->DelAllFly();
}

void MainWin::Enable() {
   NodeWin *OldWin=MWWinHand->CurWinNode;
   MWWinHand->MoveCurWin=0;
   ValidDef=0;
   if (!MWWinHand->FlyList->Empty()) MWWinHand->DelAllFly();
   if (OldWin!=MWNode) {
     ClearKeyCircBuff();
     ClearAsciiCircBuff();
     if (OldWin) {
       m_GraphCtxt->WinStopScan=1;
       if (OldWin->Item->CurFocusNode)
         OldWin->Item->CurFocusNode->Item->UnsetFocus();
       if (OldWin->Item->ActivMenu) {
         OldWin->Item->ActivMenu=0; OldWin->Item->MyMenu->choix=-1; }
       OldWin->Item->Focus=0;
       OldWin->Item->AsciiFLAG=OldWin->Item->KeyFLAG=0;
       OldWin->Item->Ascii=OldWin->Item->Key=0;
     }
     MWWinHand->CurWinNode=MWNode;
     Visible=Focus=1;
     if (OldWin) OldWin->Item->Redraw();
     Redraw();
   }
   m_GraphCtxt->WinFreeMs=1;
   m_GraphCtxt->WinStopScan=0;
}

void MainWin::Disable() {
   NodeWin *OldWin=MWWinHand->CurWinNode;
   ValidDef=0;
   MWWinHand->MoveCurWin=0;
   if (OldWin==MWNode) {
     m_GraphCtxt->WinStopScan=1;
     Focus=0;
     ClearKeyCircBuff();
     ClearAsciiCircBuff();
     AsciiFLAG=KeyFLAG=0;
     Ascii=Key=0;
     DefaultActiv=1;
     m_GraphCtxt->WinFreeMs=1;
     if (CurFocusNode) CurFocusNode->Item->UnsetFocus();
     if (ActivMenu) { ActivMenu=0; MyMenu->choix=-1; }
     MWWinHand->CurWinNode=NULL;
     Redraw();
   }
}

void MainWin::Show() {
   if (!Visible) {
     Visible=1;
     MWWinHand->Redraw(X,Y,X2,Y2);
   }
}

void MainWin::Hide() {
   if (Focus) MainWin::Disable();
   if (Visible) {
     Visible=0;
     MWWinHand->Redraw(X,Y,X2,Y2);
   }
}

void MainWin::Move(int NouvX,int NouvY) {
   int oldX=X,oldY=Y,oldX2=X2,oldY2=Y2;
   if (X!=NouvX || Y!=NouvY) {
     X=NouvX; Y=NouvY;
     X2=X+LargHz; Y2=Y+HautVt;
     if (Visible) {
       MWWinHand->Redraw(oldX,oldY,oldX2,oldY2);
       MWWinHand->Redraw(X,Y,X2,Y2); }
   }
}

//*************************** Child *****************************************
//***************************************************************************
Child::Child(int x1,int y1,int x2,int y2,MainWin *Father,ContBox *CBox) {
   Main=Father;
   m_GraphCtxt=Main->m_GraphCtxt;

   MainCBox=CBox;
   Visible=1;
   if (MainCBox && MainCBox->Main==Father) {
     MyVActiv=&MainCBox->VActiv;
     MyChList=MainCBox->SsChList;
     CurFocusNode=&MainCBox->CurFocusInNode;
   } else {
     MyVActiv=&Main->VActiv;
     MyChList=Main->ChList;
     CurFocusNode=&Main->CurFocusNode;
   }
   Type=CanDefault=Default=0; MsDownD=MsDown=0; LostFocus=OldFocus=Focus=0;
   LastMsDownTime=0;
   XC1=x1; YC1=y1; XC2=x2; YC2=y2;
   MsXDown = MsYDown = 0;
   ChNode=(MainCBox)?(MainCBox->AddChild(this)):(Main->AddChild(this));
   // events
   OnMsDown = NULL;
   OnMsUp = NULL;
   OnMsIn = NULL;
   OnMsOut = NULL;
   OnMsDrag = NULL;
   OnMsClick = NULL;
   OnMsDoubleClick = NULL;
}

Child::~Child() {
   if (MainCBox && (MainCBox->CurFocusInNode==ChNode))
     MainCBox->CurFocusNode=NULL;
   else {
     if (Main->DefNode==ChNode) { Main->DefNode=NULL; Main->DefaultActiv=0; }
     if (Main->EchapNode==ChNode) Main->EchapNode=NULL;
     if (Main->CurFocusNode==ChNode) {
       Main->CurFocusNode->Item->UnsetFocus(); Main->CurFocusNode=NULL; }
   }
   MyChList->Delete(ChNode);
   Redraw();
}

void Child::SetFocus() {
   Node *CN=*CurFocusNode;
   if (CN==ChNode) return;
   if (CN) CN->Item->UnsetFocus();
   *CurFocusNode=ChNode;
   Focus=1;
}

void Child::UnsetFocus() {
   ClearAsciiCircBuff();
   ClearKeyCircBuff();
   Main->Ascii=Main->Key=0;
   if (*CurFocusNode==ChNode) {
     Focus=0;
     if (MsDown) { MsDown=0; m_GraphCtxt->WinFreeMs=1; }
     *CurFocusNode=NULL;
   }
}

void Child::Draw() {}

void Child::DrawSurf(DgSurf *S) {}

void Child::Redraw() {
   if (MainCBox)
     MainCBox->Redraw(XC1,YC1,XC2,YC2);
   else Main->Redraw(XC1,YC1,XC2,YC2);
}

void Child::Scan() {
   int XN=Main->X+MyVActiv->OrgX,YN=Main->Y+MyVActiv->OrgY;
   int X1Ch,Y1Ch,X2Ch,Y2Ch;
   int X1VA,Y1VA,X2VA,Y2VA;
   X1Ch=XN+XC1; Y1Ch=YN+YC1;
   X2Ch=XN+XC2; Y2Ch=YN+YC2;
   X1VA=XN+MyVActiv->MinX; Y1VA=YN+MyVActiv->MinY;
   X2VA=XN+MyVActiv->MaxX; Y2VA=YN+MyVActiv->MaxY;
   MsIn=(m_GraphCtxt->In(X1Ch,Y1Ch,X2Ch,Y2Ch,MsX,MsY) &&
      m_GraphCtxt->In(X1VA,Y1VA,X2VA,Y2VA,MsX,MsY));
   LostMsDown=0;
   if (!Focus)
     LostFocus=(OldFocus)?1:0;
   OldFocus=Focus;

   if (MsDown && (MsButton&MS_LEFT_BUTT)) {
       if (LastMsXDrag != MsX || LastMsYDrag != MsY) {
         LastMsXDrag = MsX;
         LastMsYDrag = MsY;
         if (OnMsDrag != NULL) OnMsDrag(MsX-XN, MsY-YN, MsIn);
       }
   }
   if (m_GraphCtxt->WinFreeMs && (MsButton&MS_LEFT_BUTT) && MsIn) {
     m_GraphCtxt->WinFreeMs=0;
     if(!MsDown) {
       MsXDown = MsX;
       MsYDown = MsY;
       LastMsXDrag = MsX;
       LastMsYDrag = MsY;
       MsDown=1;
       if (OnMsDown!=NULL) OnMsDown(MsXDown-XN, MsYDown-YN);
     }
   } else DblClick=0;
   if ((!m_GraphCtxt->WinFreeMs) && (!(MsButton&MS_LEFT_BUTT)) && MsDown) {
     if (!MsDownD) {
        MsDownD=1;
        LastMsDownTime=DgTime;
     }
     else
       if (MsDownD && DgTimerFreq) {
         DblClick=((DgTime-LastMsDownTime)<DgTimerFreq/3 && MsIn);
         if (DblClick && OnMsDoubleClick!=NULL) OnMsDoubleClick();
         MsDownD=0;
       }
     LostMsDown=1;
     // don't call MsClick if MsDoubleClick called
     if (MsIn && OnMsClick!= NULL && !DblClick)
        OnMsClick();
     m_GraphCtxt->WinFreeMs=1;
     MsDown=0;
     if (OnMsUp!=NULL) OnMsUp(MsXDown-XN, MsYDown-YN, MsIn);
   }
   if (MsDownD && DgTimerFreq && (DgTime-LastMsDownTime)>=DgTimerFreq/2) MsDownD=0;
   if (m_GraphCtxt->WinFreeMs && MsDown) MsDown=0;
   if (!oldMsIn && MsIn && OnMsIn!=NULL) OnMsIn(MsX-XN, MsY-YN);
   if (oldMsIn && !MsIn && OnMsOut!=NULL) OnMsOut(MsX-XN, MsY-YN);

   oldMsIn = MsIn;
}

void Child::Validate() { }

void Child::Show() {
   if (!Visible) {
     Visible=1;
     Redraw();
   }
}

void Child::Hide() {
   DgSurf SaveSurf;
   if (Visible) {
     Visible=0;
     Redraw();
   }
}

int Child::GetWidth() {
   return XC2 - XC1 + 1;
}

int Child::GetHeight() {
   return YC2 - YC1 + 1;
}


//*********** class ContBox ************************************
//**************************************************************

ContBox::ContBox(int x1,int y1,int x2,int y2,MainWin *Father,char *label)
 	: Child(x1,y1,x2,y2,Father,NULL)  {
   if (label) Label=label;
   SsChList= new List;
   CurFocusInNode=NULL;
   Label.Changed();
   VActiv.OrgX=Main->VActiv.OrgX+XC1+3; VActiv.OrgY=Main->VActiv.OrgY+YC1+3;
   VActiv.MinX=VActiv.MinY=0;
   VActiv.MaxX=XC2-XC1-6;
   VActiv.MaxY=YC2-YC1-FntHaut-4;
   Redraw();
}

void ContBox::ChgOrg(int Orgx,int Orgy) {
   int OldOrgx=VActiv.OrgX-XC1-3-Main->VActiv.OrgX,
       OldOrgy=VActiv.OrgY-YC1-3-Main->VActiv.OrgY;
   if (OldOrgx==Orgx && OldOrgy==Orgy) return;
   VActiv.OrgX=XC1+3+Main->VActiv.OrgX+Orgx;
   VActiv.OrgY=YC1+3+Main->VActiv.OrgY+Orgy;
   VActiv.MinX=-Orgx;
   VActiv.MinY=-Orgy;
   VActiv.MaxX=XC2-XC1-6-Orgx; VActiv.MaxY=YC2-YC1-4-FntHaut-Orgy;
   Redraw();
}

void ContBox::GetOrg(int *Orgx,int *Orgy) {
   *Orgx=VActiv.OrgX-XC1-3-Main->VActiv.OrgX;
   *Orgy=VActiv.OrgY-YC1-3-Main->VActiv.OrgY;
}

ContBox::~ContBox() {}

Node * ContBox::AddChild(Child *child) {
   return (SsChList->Add(child)); }

void ContBox::Draw() {
   DgSurf SaveSurf;
   DgView V1;
   Node *ParcNode;
   int lgTxt=WidthText(Label.StrPtr);

   DgGetCurSurf(&SaveSurf);
   DgSetCurSurf(Main->S);
   SetSurfView(&CurSurf,&Main->VActiv);

   m_GraphCtxt->bar(XC1,YC1,XC2,YC2,m_GraphCtxt->WinGris);
   m_GraphCtxt->rect(XC1+1,YC1,XC2,YC2-4-1,m_GraphCtxt->WinGrisC);
   m_GraphCtxt->rect(XC1,YC1+1,XC2-1,YC2-4,m_GraphCtxt->WinGrisF);
   if (FntSens) {
     m_GraphCtxt->bar(XC2-lgTxt+7,YC2-FntHaut,XC2-4,YC2,m_GraphCtxt->WinGris);
     FntX=XC2-6;   FntY=YC2-FntHighPos; FntCol=m_GraphCtxt->WinNoir;
     m_GraphCtxt->OutText(Label.StrPtr);
   } else {
     m_GraphCtxt->bar(XC1+4,YC2-FntHaut,XC1+lgTxt+7,YC2,m_GraphCtxt->WinGris);
     FntX=XC1+6;   FntY=YC2-FntHighPos; FntCol=m_GraphCtxt->WinNoir;
     m_GraphCtxt->OutText(Label.StrPtr);
   }

   SetSurfInView(&CurSurf,&VActiv);
   GetSurfView(&CurSurf,&V1);

   ParcNode=SsChList->Nodes;  //Last Node
   if (ParcNode!=NULL) {
      do {
   	 SetSurfView(&CurSurf,&V1);
         if (ParcNode!=CurFocusInNode && ParcNode->Item->Visible)
	   ParcNode->Item->Draw();
         ParcNode=ParcNode->Prev;
       } while (ParcNode!=NULL);
     }
   if ((ParcNode=CurFocusInNode)) {
     SetSurfView(&CurSurf,&V1);
     if (ParcNode->Item->Visible) ParcNode->Item->Draw();
   }

   DgSetCurSurf(&SaveSurf);
}

void ContBox::Redraw(int x1,int y1,int x2,int y2) {
   int PlusX=VActiv.OrgX-MyVActiv->OrgX,
       PlusY=VActiv.OrgY-MyVActiv->OrgY;
   if (x1>VActiv.MaxX && y1>VActiv.MaxY &&
       x2<VActiv.MinX && y2<VActiv.MinY) return;
   if (x1<VActiv.MinX) x1=VActiv.MinX;   if (y1<VActiv.MinY) y1=VActiv.MinY;
   if (x2>VActiv.MaxX) x2=VActiv.MaxX;   if (y2>VActiv.MaxY) y2=VActiv.MaxY;
   Main->Redraw(x1+PlusX,y1+PlusY, x2+PlusX,y2+PlusY);
}

void ContBox::Redraw() { Child::Redraw(); }

void ContBox::DrawSurf(DgSurf *DestSurf) {
   if (CurFocusInNode && Focus) {
     if (CurFocusInNode->Item->Visible)
       CurFocusInNode->Item->DrawSurf(DestSurf);
     else CurFocusInNode->Item->UnsetFocus();
   }
}

void ContBox::Scan() {
   int OldWinFreeMs=m_GraphCtxt->WinFreeMs;
   if (Label.Changed()) Redraw();
   if (CurFocusInNode) {
     if (CurFocusInNode->Item->Visible) CurFocusInNode->Item->Scan();
     else CurFocusInNode->Item->UnsetFocus();
   }
   SsChList->SetFirstNode();
   if (SsChList->CurNode!=NULL) {
     do {
        if (SsChList->CurNode!=CurFocusInNode &&
	    SsChList->CurNode->Item->Visible)
            SsChList->CurNode->Item->Scan();
	  SsChList->GoNextNode();
     } while (SsChList->CurNode!=NULL);
   }
   if (SsChList->FirstNode) Child::Scan();
   if ((MsDown && (!Focus)) || (OldWinFreeMs && (!m_GraphCtxt->WinFreeMs))) SetFocus();
   if (Focus) {
     switch (Main->Key) {
       case 0xcb: SetPrevFocus(); break;
       case 0xcd: SetNextFocus(); break;
       case 0xc8: SetPrevFocus(); break;
       case 0xd0: SetNextFocus();
     }
   }
}

void ContBox::SetFocus() {
   Node *CurNode=*CurFocusNode,*ParcNode;
   Node *CN=CurFocusInNode;
   int NoSetFoc=0;

   if (CN && (!CN->Item->Visible)) {
     CN->Item->UnsetFocus(); CN=CurFocusInNode=NULL; }
   if (CN) {
     if (CurNode) CurNode->Item->UnsetFocus();
     Focus=1; *CurFocusNode=ChNode;
     CN->Item->SetFocus();
     CN->Item->Redraw();
   } else
     if (SsChList->FirstNode) {
       ParcNode=SsChList->FirstNode;
       for (;;) {
         if (!ParcNode) break;
	 if (ParcNode->Item->Visible) break;
	 ParcNode=ParcNode->Next;
       }
       if (ParcNode) {
         if (CurNode) CurNode->Item->UnsetFocus();
         ParcNode->Item->SetFocus();
         Focus=1; *CurFocusNode=ChNode;
         CurFocusInNode->Item->Redraw();
       } else NoSetFoc=1;
     } else NoSetFoc=1;
   if (NoSetFoc) {
     Child::SetFocus();
     if (CurNode) CurNode->Item->Redraw();
     Main->SetNextFocus();
     if (Main->CurFocusNode==ChNode) UnsetFocus();
   }
}

void ContBox::UnsetFocus() {
   Node *CIN=CurFocusInNode;
   ClearAsciiCircBuff();
   ClearKeyCircBuff();
   Main->Ascii=Main->Key=0;
   Focus=0;
   *CurFocusNode=NULL;
   if (CurFocusInNode) {
     CurFocusInNode->Item->UnsetFocus();
     CIN->Item->Redraw();
   }
}

void ContBox::SetNextFocus() {
   Node *ParcNode;
   if (CurFocusInNode) {
     ParcNode=CurFocusInNode;
     do {
       if (ParcNode) ParcNode=ParcNode->Next;
       if (!ParcNode) ParcNode=SsChList->FirstNode;
       if (ParcNode)
         if (!ParcNode->Item->Visible) ParcNode=ParcNode->Next;
     } while ((!ParcNode->Item->Visible) && ParcNode!=CurFocusInNode);
     if (CurFocusInNode!=ParcNode)
       ParcNode->Item->SetFocus();
   } else {
     ParcNode=SsChList->FirstNode;
     do {
       if (ParcNode)
         if (!ParcNode->Item->Visible) ParcNode=ParcNode->Next;
     } while ((!ParcNode->Item->Visible) && ParcNode!=NULL);
     if (ParcNode) ParcNode->Item->SetFocus();
   }
}

void ContBox::SetPrevFocus() {
   Node *ParcNode;
   if (CurFocusInNode) {
     ParcNode=CurFocusInNode;
     do {
       if (ParcNode) ParcNode=ParcNode->Prev;
       if (!ParcNode) ParcNode=SsChList->Nodes;
       if (ParcNode)
         if (!ParcNode->Item->Visible) ParcNode=ParcNode->Prev;
     } while ((!ParcNode->Item->Visible) && ParcNode!=CurFocusInNode);
     if (CurFocusInNode!=ParcNode)
       ParcNode->Item->SetFocus();
   } else {
     ParcNode=SsChList->Nodes;
     do {
       if (ParcNode)
         if (!ParcNode->Item->Visible) ParcNode=ParcNode->Prev;
     } while ((!ParcNode->Item->Visible) && ParcNode!=NULL);
     if (ParcNode) ParcNode->Item->SetFocus();
   }
}

//*************** class Button ************************************
//*****************************************************************
Button::Button(int x1,int y1,int x2,int y2,MainWin *Father,char *label,
   char def,char Echap) : Child(x1,y1,x2,y2,Father,NULL)  {
   CanDefault=1;
   Click=NULL;
   if (label) Label=label;
   Label.Changed();
   if (Main->DefNode && def) {
     Main->DefNode->Item->Default=0;
     Main->DefNode->Item->Redraw();
   }
   if (def) {
     Main->DefNode=ChNode;
     Default=1;
     Main->DefaultActiv=1;
   }
   if (Echap) Main->EchapNode=ChNode;
   Default=def;
   Etat=0;
   Redraw();
}

Button::~Button() { }

void Button::Draw() {
   DgView V1;
   int hlgtext;
   m_GraphCtxt->bar(XC1,YC1,XC2,YC2,m_GraphCtxt->WinGris);

   if (Etat) m_GraphCtxt->CadAppWin(XC1,YC1,XC2,YC2);
   else m_GraphCtxt->CadWin(XC1,YC1,XC2,YC2);
   if (Focus)
     m_GraphCtxt->rectmap(XC1+3,YC1+3,XC2-3,YC2-3,m_GraphCtxt->WinNoir,0xaaaaaaaa);
   if ((Default&& Main->DefaultActiv)|| (Main->CurFocusNode==ChNode&& Focus))
     m_GraphCtxt->rect(XC1,YC1+1,XC2-1,YC2,m_GraphCtxt->WinNoir);
   V1.OrgX=XC1+3+CurSurf.OrgX;  V1.OrgY=YC1+3+CurSurf.OrgY;
   V1.MinX=V1.MinY=0;
   V1.MaxX=XC2-XC1-6; V1.MaxY=YC2-YC1-6;
   SetSurfInView(&CurSurf,&V1);
   FntCol=m_GraphCtxt->WinNoir;

   if (Etat) {
     FntY=ViewGetFntYMID(&V1)-1;
     FntX=ViewGetXOutTextMode(&V1,Label.StrPtr,AJ_MID)+1;
   } else {
     FntY=ViewGetFntYMID(&V1);
     FntX=ViewGetXOutTextMode(&V1,Label.StrPtr,AJ_MID);
   }
   m_GraphCtxt->OutText(Label.StrPtr);
}

void Button::Scan() {
   int Redr=0;
   Child::Scan();
   if (MsDown && (!Focus)) {
      Etat=(MsIn)?1:0;
      SetFocus();
      }
   if ( (Etat && (!MsIn))	     ||
        (MsDown && Etat && (!MsIn))  ||
        ((!MsDown) && Etat)	     ||
        ((!Focus) && Etat) ) {
     Etat=0; Redr=1; }
   if (MsDown && (!Etat) && MsIn) { Etat=1; Redr=1; }
   if (Label.Changed()) Redr=1;
   if (LostMsDown && MsIn) {
     UnsetFocus(); Redraw(); Validate(); return; }
   if (Redr) Redraw();
}

void Button::SetFocus() {
   Node *CN=Main->CurFocusNode,*CN2;
   Child::SetFocus();
   if (Main->DefaultActiv) {
     Main->DefaultActiv=0;
     CN2=Main->DefNode;
     if (CN2) CN2->Item->Redraw();
   }
//   if (CN) CN->Item->Redraw();
   Redraw();
}

void Button::UnsetFocus() {
   if (Focus) {
     Child::UnsetFocus();
     Etat=0;
     Redraw();
     if (Main->DefNode) {
       Main->DefaultActiv=1;
       Main->DefNode->Item->Redraw(); }
   }
}

void Button::Validate() {
   if (Click!=NULL) Click();
}
//*************** class Button ************************************
//*****************************************************************
ImgButton::ImgButton(int x1,int y1,int x2,int y2,MainWin *Father,DgSurf *img,
   int btype,char def,char Echap) : Child(x1,y1,x2,y2,Father,NULL)  {
   CanDefault=1;
   Click=NULL;
   // img should be with the same bpp as the winHandler
   if (img!=NULL && img->BitsPixel==Main->MWWinHand->S->BitsPixel) {
     ImgBlit=btype;
     m_Img=img;
     SetOrgSurf(m_Img,0,0); // force origin to left, bottom
   }
   else
     m_Img=NULL;

   DrawFocus=false; // by default don't draw focus

   if (Main->DefNode && def) {
     Main->DefNode->Item->Default=0;
     Main->DefNode->Item->Redraw();
   }
   if (def) {
     Main->DefNode=ChNode;
     Default=1;
     Main->DefaultActiv=1;
   }
   if (Echap) Main->EchapNode=ChNode;
   Default=def;
   Etat=0;
   Redraw();
}

ImgButton::~ImgButton() { }

bool ImgButton::SetImage(DgSurf *img) {
   if (img!=NULL && img->BitsPixel==Main->MWWinHand->S->BitsPixel) {
     m_Img=img;
     SetOrgSurf(m_Img,0,0); // force origin to left, bottom
     Redraw();
     return true;
   }
   return false;
}

void ImgButton::Draw() {
   DgView V1;
   int midx=(XC1+XC2)/2,midy=(YC1+YC2)/2;
   m_GraphCtxt->bar(XC1,YC1,XC2,YC2,m_GraphCtxt->WinGris);
   if (m_Img!=NULL) {
     if (Etat) {
       midx++; midy--;
     }
     if (ImgBlit&DGUI_IMGBUTT_CENTER) {
       if (ImgBlit&DGUI_IMGBUTT_UNMASK)
         m_GraphCtxt->PutSurf(m_Img,midx-m_Img->ResH/2,midy-m_Img->ResV/2,0);
       else
         m_GraphCtxt->PutMaskSurf(m_Img,midx-m_Img->ResH/2,midy-m_Img->ResV/2,0);
     }
     else
       if (ImgBlit&DGUI_IMGBUTT_FIT) {
       }
   }

   if (Etat)
     m_GraphCtxt->CadAppWin(XC1,YC1,XC2,YC2);
   else
     m_GraphCtxt->CadWin(XC1,YC1,XC2,YC2);
   if (Focus && DrawFocus)
     m_GraphCtxt->rectmap(XC1+3,YC1+3,XC2-3,YC2-3,m_GraphCtxt->WinNoir,0xaaaaaaaa);
   if ((Default&& Main->DefaultActiv)|| (Main->CurFocusNode==ChNode&& Focus))
     m_GraphCtxt->rect(XC1,YC1+1,XC2-1,YC2,m_GraphCtxt->WinNoir);
}

void ImgButton::Scan() {
   int Redr=0;
   Child::Scan();
   if (MsDown && (!Focus)) {
      Etat=(MsIn)?1:0;
      SetFocus();
      }
   if ( (Etat && (!MsIn))	     ||
        (MsDown && Etat && (!MsIn))  ||
        ((!MsDown) && Etat)	     ||
        ((!Focus) && Etat) ) {
     Etat=0; Redr=1; }
   if (MsDown && (!Etat) && MsIn) { Etat=1; Redr=1; }
   if (LostMsDown && MsIn) {
     UnsetFocus(); Redraw(); Validate(); return; }
   if (Redr) Redraw();
}

void ImgButton::SetFocus() {
   Node *CN=Main->CurFocusNode,*CN2;
   Child::SetFocus();
   if (Main->DefaultActiv) {
     Main->DefaultActiv=0;
     CN2=Main->DefNode;
     if (CN2) CN2->Item->Redraw();
   }
//   if (CN) CN->Item->Redraw();
   Redraw();
}

void ImgButton::UnsetFocus() {
   if (Focus) {
     Child::UnsetFocus();
     Etat=0;
     Redraw();
     if (Main->DefNode) {
       Main->DefaultActiv=1;
       Main->DefNode->Item->Redraw(); }
   }
}

void ImgButton::Validate() {
   if (Click!=NULL) Click();
}
//**************** class Label ************************************
//*****************************************************************
Label::Label(int x1,int y1,int x2,int y2,MainWin *Father,char *text,
   int text_aj) :  Child(x1,y1,x2,y2,Father,NULL)  {
   Text_AJ=text_aj;
   if (text) Text=text;
   Text.Changed();
   Main->Redraw(XC1,YC1,XC2,YC2);
}

Label::~Label() { }

void Label::Draw() {
   DgView V1;

   m_GraphCtxt->bar(XC1,YC1,XC2,YC2,m_GraphCtxt->WinGris);
   V1.OrgX=XC1+3+CurSurf.OrgX;  V1.OrgY=YC1+3+CurSurf.OrgY;
   V1.MinX=V1.MinY=0;
   V1.MaxX=XC2-XC1-6; V1.MaxY=YC2-YC1-6;
   SetSurfInView(&CurSurf,&V1);

   FntY=ViewGetFntYMID(&V1);
   FntCol=m_GraphCtxt->WinNoir;
   m_GraphCtxt->ViewOutTextMode(&V1,Text.StrPtr,Text_AJ);
}

void Label::Scan() {
   if (Text.Changed()) Redraw();
}

void Label::SetFocus() {
   Node *CurNode=Main->CurFocusNode;
   Child::SetFocus();
//   if (CurNode) CurNode->Item->Redraw();
   Main->SetNextFocus();
   if (Main->CurFocusNode==ChNode) UnsetFocus();
}

void Label::UnsetFocus() {
   Child::UnsetFocus();
}

//**************** class TextBox **********************************
//*****************************************************************
TextBox::TextBox(int x1,int y1,int x2,int y2,MainWin *Father,char *deftext,
         char textInBox)
   :  Child(x1,y1,x2,y2,Father,NULL)  {
   int PosLarg,len;
   OldIns=(KbFLAG&KB_INS_ACT)?1:0;
   password=0; charpw='*';
   TextInBox=textInBox;
   AsciiPressed=KeyPressed=NULL;
   FocusLost=ValidateText=NULL;
   Insert=1;
   MaxLargText=XC2-XC1-7;
   if (deftext) Text=deftext;
   Text.Changed();
   TextCatInBox();
   TextX=OldPosCursor=PosCursor=0;
   Main->Redraw(XC1,YC1,XC2,YC2);
}

TextBox::~TextBox() { }

void TextBox::Draw() {
   DgView V1;
   char *cstr;
   int l;

   if (password) {
      cstr=(char*)alloca((l=strlen(Text.StrPtr))+1);
      memset(cstr,charpw, l);
      cstr[l]=0;
   } else cstr=Text.StrPtr;

   m_GraphCtxt->bar(XC1,YC1,XC2,YC2,m_GraphCtxt->WinBlanc);
   m_GraphCtxt->CadBoxWin(XC1,YC1,XC2,YC2);
   V1.OrgX=XC1+3+CurSurf.OrgX;  V1.OrgY=YC1+3+CurSurf.OrgY;
   V1.MinX=V1.MinY=0;
   V1.MaxX=XC2-XC1-6; V1.MaxY=YC2-YC1-6;
   SetSurfInView(&CurSurf,&V1);
   FntCol=m_GraphCtxt->WinNoir;
   FntY=ViewGetFntYMID(&V1);
   if ((!Focus) || TextInBox)
     m_GraphCtxt->ViewOutTextMode(&V1,cstr,AJ_SRC);
   else {
       FntX=ViewGetXOutTextMode(&V1,cstr,AJ_SRC)+TextX;
       m_GraphCtxt->OutText(cstr);
   }
}

void TextBox::DrawSurf(DgSurf *DestSurf) {
   DgView V1;
   int larg,x1,l;
   char *cstr;

   if (password) {
      cstr=(char*)alloca((l=strlen(Text.StrPtr))+1);
      memset(cstr,charpw, l);
      cstr[l]=0;
   } else cstr=Text.StrPtr;
   if (Focus) {
     V1=*MyVActiv;
     V1.OrgX+=Main->X; V1.OrgY+=Main->Y;
     DgSetCurSurf(DestSurf);
     SetSurfView(&CurSurf,&V1);
     V1.MinX=XC1+3; V1.MinY=YC1+3;
     V1.MaxX=XC2-3; V1.MaxY=YC2-3;
     SetSurfInView(&CurSurf,&V1);
     if ((DgTimerFreq && ((DgTime/(DgTimerFreq/10))&1)) || (!DgTimerFreq)) {
      	larg=WidthPosText(cstr,PosCursor);
	if (larg>0) larg++;
        FntY=ViewGetFntYMID(&V1);
     	if (!FntSens) {
     	  x1=V1.MinX+larg+TextX;
          m_GraphCtxt->bar(x1,FntY+FntLowPos,x1+1,FntY+FntHighPos,m_GraphCtxt->WinNoir);
	} else {
	  x1=V1.MaxX-larg+TextX;
          m_GraphCtxt->bar(x1-1,FntY+FntLowPos,x1,FntY+FntHighPos,m_GraphCtxt->WinNoir);
	}
     }
   }
}

void TextBox::Scan() {
   String TestStr;
   unsigned int TestPosCursor;
   int l;
   char *cstr;

   Child::Scan();
   if (MsDown && (!Focus))
     SetFocus();
   if (Text.Changed()) {
     TextCatInBox();
     Main->Redraw(XC1,YC1,XC2,YC2);
   }
   if (LostFocus && FocusLost) { FocusLost(&Text); return; }
   if (Focus) {
     if (AsciiPressed && Main->Ascii && (!(Main->AsciiFLAG&KB_CTRL_PR)) &&
     	 (!(Main->AsciiFLAG&KB_LEFT_ALT_PR)) )
       AsciiPressed(&Main->Ascii,Main->AsciiFLAG);
     if (KeyPressed && Main->Key)
       KeyPressed(&Main->Key,Main->KeyFLAG);
     if (Main->Ascii && (!(Main->AsciiFLAG&KB_CTRL_PR)) &&
         (!(Main->AsciiFLAG&KB_LEFT_ALT_PR)) )
	if (TextInBox) {
          if (password) {
            cstr=(char*)alloca((l=strlen(Text.StrPtr))+1);
            memset(cstr,charpw, l); cstr[l]=0;
          } else cstr=Text.StrPtr;
	  TestStr=cstr;
          TestPosCursor=TestStr.AddCursChar(PosCursor,Main->Ascii,Insert);
	  if (WidthText(TestStr.StrPtr)<(MaxLargText)) {
	    Text.AddCursChar(PosCursor,Main->Ascii,Insert);
	    PosCursor=TestPosCursor;
	  }
        } else PosCursor=Text.AddCursChar(PosCursor,Main->Ascii,Insert);

     switch(Main->Key) {
       case 0x52:   // Ins
       case 0xd2:   // ext-Ins
	 if (KbFLAG&KB_INS_ACT)
	   Insert=(OldIns)?1:0;
	 else Insert=(OldIns)?0:1;
	 break;
       case 0xe:    // back
         PosCursor=Text.BackDelCurs(PosCursor); break;
       case 0xd3:   // ext-del
         PosCursor=Text.DelCurs(PosCursor); break;
       case 0x53:   // del
         if ( ((!(Main->KeyFLAG&KB_NUM_ACT)) && (!(Main->KeyFLAG&KB_SHIFT_PR))) ||
	      ((Main->KeyFLAG&KB_NUM_ACT) && (Main->KeyFLAG&KB_SHIFT_PR)) )
	   PosCursor=Text.DelCurs(PosCursor);
	 break;
       case 0xc7 : // ext-deb
         PosCursor=0; break;
       case 0xcf : // ext-fin
	 PosCursor=strlen(Text.StrPtr); break;
       case 0x47 : // deb
         if  ( ((!(Main->KeyFLAG&KB_NUM_ACT)) && (!(Main->KeyFLAG&KB_SHIFT_PR))) ||
	      ((Main->KeyFLAG&KB_NUM_ACT) && (Main->KeyFLAG&KB_SHIFT_PR)) )
	   PosCursor=0; break;
       case 0x4f : // fin
         if ( ((!(Main->KeyFLAG&KB_NUM_ACT)) && (!(Main->KeyFLAG&KB_SHIFT_PR))) ||
	      ((Main->KeyFLAG&KB_NUM_ACT) && (Main->KeyFLAG&KB_SHIFT_PR)) )
	   PosCursor=strlen(Text.StrPtr); break;
       }
     if (!FntSens) {
       switch(Main->Key) {
         case 0xcb : // ext-gauche
	   if (PosCursor>0) PosCursor--; break;
         case 0xcd : // ext-droit
	   if (PosCursor<strlen(Text.StrPtr)) PosCursor++; break;
         case 0x4b : // gauche
           if (PosCursor>0 && (
	      ((!(Main->KeyFLAG&KB_NUM_ACT)) && (!(Main->KeyFLAG&KB_SHIFT_PR))) ||
	      ((Main->KeyFLAG&KB_NUM_ACT) && (Main->KeyFLAG&KB_SHIFT_PR))) )
	     PosCursor--; break;
         case 0x4d : // droit
           if (PosCursor<strlen(Text.StrPtr) && (
	      ((!(Main->KeyFLAG&KB_NUM_ACT)) && (!(Main->KeyFLAG&KB_SHIFT_PR))) ||
	      ((Main->KeyFLAG&KB_NUM_ACT) && (Main->KeyFLAG&KB_SHIFT_PR))) )
	     PosCursor++; break;
       }
     } else {
       switch(Main->Key) {
         case 0xcd : // ext-gauche
	   if (PosCursor>0) PosCursor--; break;
         case 0xcb : // ext-droit
	   if (PosCursor<strlen(Text.StrPtr)) PosCursor++; break;
	 case 0x4d : // gauche
           if (PosCursor>0 && (
	      ((!(Main->KeyFLAG&KB_NUM_ACT)) && (!(Main->KeyFLAG&KB_SHIFT_PR)))  ||
	      ((Main->KeyFLAG&KB_NUM_ACT) && (Main->KeyFLAG&KB_SHIFT_PR))) )
	     PosCursor--; break;
	 case 0x4b : // droit
           if (PosCursor<strlen(Text.StrPtr) && (
	      ((!(Main->KeyFLAG&KB_NUM_ACT)) && (!(Main->KeyFLAG&KB_SHIFT_PR)))  ||
	      ((Main->KeyFLAG&KB_NUM_ACT) && (Main->KeyFLAG&KB_SHIFT_PR))) )
	     PosCursor++; break;
       }
     }
     if (PosCursor!=OldPosCursor) {
       CalcTextX();
       Main->Redraw(XC1,YC1,XC2,YC2);
       OldPosCursor=PosCursor;
     }
   }
}

void TextBox::SetFocus() {
   Node *CN=Main->CurFocusNode;
   Child::SetFocus();
//   if (CN) CN->Item->Redraw();
   PosCursor=strlen(Text.StrPtr);
   Redraw();
}

void TextBox::UnsetFocus() {
   Child::UnsetFocus();
   Redraw();
}

void TextBox::Validate() {
   if (ValidateText!=NULL) ValidateText(&Text);
}

void TextBox::SetPassWord(char pw) {
   password=pw; Redraw();
}

char TextBox::GetPassWord() {
   return password;
}

void TextBox::TextCatInBox() {
   unsigned int len,PosLarg;

   if (TextInBox) {
     if (TextInBox && (WidthText(Text.StrPtr)>=(MaxLargText)))
     {
       PosLarg=PosWidthText(Text.StrPtr,MaxLargText);
       if (PosLarg && WidthPosText(Text.StrPtr,PosLarg)>=(MaxLargText))
         PosLarg--;
       Text.StrPtr[PosLarg]='\0';
     }
   }
}

void TextBox::CalcTextX() {
    int LargPos;
    if (!TextInBox) {
      LargPos=WidthPosText(Text.StrPtr,PosCursor);
      if (FntSens) {
        if ((XC2-LargPos+TextX)<(XC1+8))
          TextX=XC1+8-XC2+LargPos;
        if (TextX-LargPos>=0) {
          TextX=LargPos;
	  if (TextX>(MaxLargText/2))
	    TextX-=(MaxLargText/2);
	  else TextX=0;
	}
      } else {
        if ((XC1+LargPos+TextX)>(XC2-8))
          TextX=XC2-8-XC1-LargPos;
        if (LargPos+TextX<=0) {
          TextX=-LargPos;
	  if ((-TextX)>(MaxLargText/2))
	    TextX+=(MaxLargText/2);
	  else TextX=0;
	}
      }
    }
}

//**************** class GraphBox **********************************
//******************************************************************

GraphBox::GraphBox(int x1,int y1,int x2,int y2,MainWin *Father,int GbCCol)
         :Child(x1,y1,x2,y2,Father,NULL)  {
   ContCol=GbCCol;
   ScanGraphBox=NULL;
   GraphBoxDraw=NULL;
   GraphBoxMouseIn=GraphBoxMouseOut=NULL;
   AsciiPressed=KeyPressed=NULL;

   DrawFocus=false;

   OldMsIn=0;
   VGraphBox.OrgX=XC1+1+MyVActiv->OrgX;
   VGraphBox.OrgY=YC1+1+MyVActiv->OrgY;
   VGraphBox.MinX=VGraphBox.MinY=0;
   VGraphBox.MaxX=XC2-XC1-2;
   VGraphBox.MaxY=YC2-YC1-2;

   Redraw();
}

GraphBox::~GraphBox() {}

void GraphBox::ChgOrg(int Orgx,int Orgy) {
   int OldOrgx=VGraphBox.OrgX-XC1-1-MyVActiv->OrgX,
       OldOrgy=VGraphBox.OrgY-YC1-1-MyVActiv->OrgY;
   if (OldOrgx==Orgx && OldOrgy==Orgy) return;
   VGraphBox.OrgX=XC1+1+MyVActiv->OrgX+Orgx;
   VGraphBox.OrgY=YC1+1+MyVActiv->OrgY+Orgy;
   VGraphBox.MinX=-Orgx;
   VGraphBox.MinY=-Orgy;
   VGraphBox.MaxX=XC2-XC1-2-Orgx; VGraphBox.MaxY=YC2-YC1-2-Orgy;
   Redraw();
}

void GraphBox::GetOrg(int *Orgx,int *Orgy) {
   *Orgx=VGraphBox.OrgX-XC1-1-MyVActiv->OrgX;
   *Orgy=VGraphBox.OrgY-YC1-1-MyVActiv->OrgY;
}

void GraphBox::Draw() {
   SetSurfView(&CurSurf,MyVActiv);
   m_GraphCtxt->rect(XC1,YC1,XC2,YC2,ContCol);
   if (Focus && DrawFocus)
     m_GraphCtxt->rectmap(XC1,YC1,XC2,YC2,m_GraphCtxt->WinNoir,0xaaaaaaaa);
   if (GraphBoxDraw) {
     SetSurfInView(&CurSurf,&VGraphBox);
     GraphBoxDraw(this);
   }
}

void GraphBox::Scan() {
   DgSurf SaveSurf;
   DgGetCurSurf(&SaveSurf);

   Child::Scan();
   if (MsDown && (!Focus)) SetFocus();
   MouseX=MsX-Main->X-VGraphBox.OrgX;
   MouseY=MsY-Main->Y-VGraphBox.OrgY;
   if (GraphBoxMouseIn && (!OldMsIn) && MsIn) {
     GraphBoxMouseIn(MouseX,MouseY,MsButton);
   }
   if (GraphBoxMouseOut && OldMsIn && (!MsIn)) {
     GraphBoxMouseOut(MouseX,MouseY,MsButton);
   }
   OldMsIn=MsIn;
   if (Focus) {
     if (AsciiPressed && Main->Ascii)
       AsciiPressed(&Main->Ascii,Main->AsciiFLAG);
     if (KeyPressed && Main->Key)
       KeyPressed(&Main->Key,Main->KeyFLAG);
   }
   if (ScanGraphBox) {
     SetView();
     ScanGraphBox(this);
   }
   DgSetCurSurf(&SaveSurf);
}

void GraphBox::SetFocus() {
   Node *CN=Main->CurFocusNode;
   Child::SetFocus();
//   if (CN) CN->Item->Redraw();
   Main->Redraw(XC1,YC1,XC2,YC2);
}

void GraphBox::UnsetFocus() {
   Child::UnsetFocus();
   Redraw();
}

void GraphBox::SetView() {
   DgSetCurSurf(Main->S);
   SetSurfView(&CurSurf,MyVActiv);
   SetSurfInView(&CurSurf,&VGraphBox);
}

int GraphBox::GetWidth() {
   if(XC2 > XC1)
     return XC2 - XC1 - 1;
   return 0;
}

int GraphBox::GetHeight() {
   if(YC2 > YC1)
     return YC2 - YC1 - 1;
   return 0;
}


//**************** class CocheBox *********************************
//*****************************************************************

CocheBox::CocheBox(int x1,int y1,int x2,int y2,MainWin *Fath,ContBox *Cont,
        char *label,char vtrue) : Child(x1,y1,x2,y2,Fath,Cont)  {
   if (label) Label=label;
   Label.Changed();
   Etat=1; True=vtrue;
   Changed=NULL;
   Redraw();
}

CocheBox::~CocheBox() { }

void CocheBox::Draw() {
   DgView V1;
   int hlgtext;

   SetSurfInView(&CurSurf,MyVActiv);
   m_GraphCtxt->bar(XC1,YC1,XC2,YC2,m_GraphCtxt->WinGris);
   GetSurfView(&CurSurf,&V1);
   V1.MinX=XC1; V1.MinY=YC1;
   V1.MaxX=XC2; V1.MaxY=YC2;
   SetSurfInView(&CurSurf,&V1);
   if (FntSens) {
     if (Etat)
       m_GraphCtxt->bar(XC2-13,YC2-16,XC2-1,YC2-4,m_GraphCtxt->WinBlanc);
     if (True)
       m_GraphCtxt->DrawCoche(XC2-10,YC2-13);
     m_GraphCtxt->CadBoxWin(XC2-13,YC2-16,XC2-1,YC2-4);
     if (Focus) {
       hlgtext=WidthText(Label.StrPtr)+4+18;
       m_GraphCtxt->rectmap(XC2-hlgtext,YC2+FntLowPos+2,
               XC2-18,YC2+FntLowPos-FntHaut-2,m_GraphCtxt->WinNoir,0xaaaaaaaa);
     }
     FntCol=m_GraphCtxt->WinNoir;
     FntX=XC2-20; FntY=YC2-FntHaut; m_GraphCtxt->OutText(Label.StrPtr);
   } else {
     if (Etat)
       m_GraphCtxt->bar(XC1+1,YC2-16,XC1+13,YC2-4,m_GraphCtxt->WinBlanc);
     if (True) m_GraphCtxt->DrawCoche(XC1+4,YC2-13);
     m_GraphCtxt->CadBoxWin(XC1+1,YC2-16,XC1+13,YC2-4);
     if (Focus) {
       hlgtext=WidthText(Label.StrPtr)+4+18;
       m_GraphCtxt->rectmap(XC1+18,YC2+FntLowPos+2,
               XC1+hlgtext,YC2+FntLowPos-FntHaut-2,m_GraphCtxt->WinNoir,0xaaaaaaaa);
     }
     FntCol=m_GraphCtxt->WinNoir;
     FntX=XC1+20; FntY=YC2-FntHaut; m_GraphCtxt->OutText(Label.StrPtr);
   }
}

void CocheBox::Scan() {
   char Redr=0;
   Child::Scan();
   if (Label.Changed()) Redr=1;
   if (MsDown && (!Focus)) { SetFocus(); Redr=1; }
   if (Focus) {
     if (LostMsDown) {
       Redr=1;
       if (MsIn) {
         True=(True)?0:1;
	 if (Changed) { Redraw(); Changed(True); return; }
       }
     }
     if (Main->Ascii==0x20) {    // espace
       Redr=1;
       True=(True)?0:1;
       if (Changed) { Redraw(); Changed(True); return; }
     }
     if (Etat && MsDown && MsIn) { Etat=0; Redr=1; }
   }
   if ((!Etat) && ((!MsDown) || (!MsIn)) ) { Etat=1; Redr=1; }
   if (Redr) Redraw();
}

void CocheBox::SetFocus() {
   Node *CN=Main->CurFocusNode;
   if (!Focus) {
     if (MainCBox) {
       CN=MainCBox->CurFocusInNode;
       if (CN) CN->Item->UnsetFocus();
       Focus=1; MainCBox->CurFocusInNode=ChNode;
       if (!MainCBox->Focus) MainCBox->SetFocus();
     } else {
       Child::SetFocus();
     }
//     if (CN) CN->Item->Redraw();
     Redraw();
   }
}

void CocheBox::UnsetFocus() {
   if (Focus) {
     if (MainCBox) {
       ClearAsciiCircBuff();
       ClearKeyCircBuff();
       Focus=Main->Ascii=Main->Key=0;
       Etat=1;
     } else {
       ClearAsciiCircBuff();
       ClearKeyCircBuff();
       Main->Ascii=Main->Key=0;
       Etat=1;
       Child::UnsetFocus();
     }
     Redraw();
   }
}

void CocheBox::SetTrue(char ctrue) {
   char oldtrue=True;
   True=ctrue;
   if (True!=oldtrue) {
     Redraw();
     if (Changed) Changed(True);
   }
}

CocheBox& CocheBox::operator=(char ctrue) {
   char oldtrue=True;
   True=ctrue;
   if (True!=oldtrue) {
     Redraw();
     if (Changed) Changed(True);
   }
   return *this;
}

//**************** class RadioButt *********************************
//*****************************************************************

OptionButt::OptionButt(int x1,int y1,int x2,int y2,MainWin *Fath,ContBox *Cont,
        char *label,char vtrue) : Child(x1,y1,x2,y2,Fath,Cont)  {
   if (label) Label=label;
   Label.Changed();
   Etat=1; True=vtrue;
   Type=ID_OptionButt;
   Changed=NULL;
   Redraw();
}

OptionButt::~OptionButt() { }

void OptionButt::Draw() {
   DgView V1;
   int hlgtext;
   int x1,y1,x2,y2;
   int Pt[16];
   int ListPt[9]={ 8,(int)&Pt[0], (int)&Pt[2], (int)&Pt[4], (int)&Pt[6],
   		     (int)&Pt[8], (int)&Pt[10], (int)&Pt[12], (int)&Pt[14] };

   SetSurfInView(&CurSurf,MyVActiv);
   m_GraphCtxt->bar(XC1,YC1,XC2,YC2,m_GraphCtxt->WinGris);
   GetSurfView(&CurSurf,&V1);
   V1.MinX=XC1; V1.MinY=YC1;
   V1.MaxX=XC2; V1.MaxY=YC2;
   SetSurfInView(&CurSurf,&V1);
   if (FntSens) {
     x1=XC2-13; y1=YC2-16; x2=XC2-1; y2=YC2-4;
     Pt[0 ]= x1+ 4; Pt[1 ]= y1+ 0;     Pt[2 ]= x2- 4; Pt[3 ]= y1+ 0;
     Pt[4 ]= x2+ 0; Pt[5 ]= y1+ 4;     Pt[6 ]= x2+ 0; Pt[7 ]= y2- 4;
     Pt[8 ]= x2- 4; Pt[9 ]= y2+ 0;     Pt[10]= x1+ 4; Pt[11]= y2+ 0;
     Pt[12]= x1+ 0; Pt[13]= y2- 4;     Pt[14]= x1+ 0; Pt[15]= y1+ 4;
     if (Etat) m_GraphCtxt->Poly(&ListPt,NULL,POLY_SOLID,m_GraphCtxt->WinBlanc);
     if (True) {
       m_GraphCtxt->bar(XC2-8,YC2-12,XC2-6,YC2-8,m_GraphCtxt->WinNoir);
       m_GraphCtxt->bar(XC2-9,YC2-11,XC2-5,YC2-9,m_GraphCtxt->WinNoir); }
     m_GraphCtxt->Line((void*)ListPt[1],(void*)ListPt[2],m_GraphCtxt->WinGrisC);
     m_GraphCtxt->Line((void*)ListPt[2],(void*)ListPt[3],m_GraphCtxt->WinGrisC);
     m_GraphCtxt->Line((void*)ListPt[3],(void*)ListPt[4],m_GraphCtxt->WinGrisC);
     m_GraphCtxt->Line((void*)ListPt[4],(void*)ListPt[5],m_GraphCtxt->WinGrisC);
     m_GraphCtxt->Line((void*)ListPt[5],(void*)ListPt[6],m_GraphCtxt->WinGrisF);
     m_GraphCtxt->Line((void*)ListPt[6],(void*)ListPt[7],m_GraphCtxt->WinGrisF);
     m_GraphCtxt->Line((void*)ListPt[7],(void*)ListPt[8],m_GraphCtxt->WinGrisF);
     m_GraphCtxt->Line((void*)ListPt[8],(void*)ListPt[1],m_GraphCtxt->WinGrisF);
     x1++; y1++; x2--; y2--;
     Pt[0 ]= x1+ 3; Pt[1 ]= y1+ 0;     Pt[2 ]= x2- 3; Pt[3 ]= y1+ 0;
     Pt[4 ]= x2+ 0; Pt[5 ]= y1+ 3;     Pt[6 ]= x2+ 0; Pt[7 ]= y2- 3;
     Pt[8 ]= x2- 3; Pt[9 ]= y2+ 0;     Pt[10]= x1+ 3; Pt[11]= y2+ 0;
     Pt[12]= x1+ 0; Pt[13]= y2- 3;     Pt[14]= x1+ 0; Pt[15]= y1+ 3;
     m_GraphCtxt->Line((void*)ListPt[1],(void*)ListPt[2],m_GraphCtxt->WinGris);
     m_GraphCtxt->Line((void*)ListPt[2],(void*)ListPt[3],m_GraphCtxt->WinGris);
     m_GraphCtxt->Line((void*)ListPt[3],(void*)ListPt[4],m_GraphCtxt->WinGris);
     m_GraphCtxt->Line((void*)ListPt[4],(void*)ListPt[5],m_GraphCtxt->WinGris);
     m_GraphCtxt->Line((void*)ListPt[5],(void*)ListPt[6],m_GraphCtxt->WinNoir);
     m_GraphCtxt->Line((void*)ListPt[6],(void*)ListPt[7],m_GraphCtxt->WinNoir);
     m_GraphCtxt->Line((void*)ListPt[7],(void*)ListPt[8],m_GraphCtxt->WinNoir);
     m_GraphCtxt->Line((void*)ListPt[8],(void*)ListPt[1],m_GraphCtxt->WinNoir);
     if (Focus) {
       hlgtext=WidthText(Label.StrPtr)+4+18;
       m_GraphCtxt->rectmap(XC2-hlgtext,YC2+FntLowPos+2,
           XC2-18,YC2+FntLowPos-FntHaut-2,m_GraphCtxt->WinNoir,0xaaaaaaaa);
     }
     FntCol=m_GraphCtxt->WinNoir;
     FntX=XC2-20; FntY=YC2-FntHaut; m_GraphCtxt->OutText(Label.StrPtr);
   } else {
     x1=XC1+1; y1=YC2-16; x2=XC1+13; y2=YC2-4;
     Pt[0 ]= x1+ 4; Pt[1 ]= y1+ 0;     Pt[2 ]= x2- 4; Pt[3 ]= y1+ 0;
     Pt[4 ]= x2+ 0; Pt[5 ]= y1+ 4;     Pt[6 ]= x2+ 0; Pt[7 ]= y2- 4;
     Pt[8 ]= x2- 4; Pt[9 ]= y2+ 0;     Pt[10]= x1+ 4; Pt[11]= y2+ 0;
     Pt[12]= x1+ 0; Pt[13]= y2- 4;     Pt[14]= x1+ 0; Pt[15]= y1+ 4;
     if (Etat)
       m_GraphCtxt->Poly(&ListPt,NULL,POLY_SOLID,m_GraphCtxt->WinBlanc);
     m_GraphCtxt->Line((void*)ListPt[1],(void*)ListPt[2],m_GraphCtxt->WinGrisC);
     m_GraphCtxt->Line((void*)ListPt[2],(void*)ListPt[3],m_GraphCtxt->WinGrisC);
     m_GraphCtxt->Line((void*)ListPt[3],(void*)ListPt[4],m_GraphCtxt->WinGrisC);
     m_GraphCtxt->Line((void*)ListPt[4],(void*)ListPt[5],m_GraphCtxt->WinGrisC);
     m_GraphCtxt->Line((void*)ListPt[5],(void*)ListPt[6],m_GraphCtxt->WinGrisF);
     m_GraphCtxt->Line((void*)ListPt[6],(void*)ListPt[7],m_GraphCtxt->WinGrisF);
     m_GraphCtxt->Line((void*)ListPt[7],(void*)ListPt[8],m_GraphCtxt->WinGrisF);
     m_GraphCtxt->Line((void*)ListPt[8],(void*)ListPt[1],m_GraphCtxt->WinGrisF);

     x1++; y1++; x2--; y2--;
     Pt[0 ]= x1+ 3; Pt[1 ]= y1+ 0;     Pt[2 ]= x2- 3; Pt[3 ]= y1+ 0;
     Pt[4 ]= x2+ 0; Pt[5 ]= y1+ 3;     Pt[6 ]= x2+ 0; Pt[7 ]= y2- 3;
     Pt[8 ]= x2- 3; Pt[9 ]= y2+ 0;     Pt[10]= x1+ 3; Pt[11]= y2+ 0;
     Pt[12]= x1+ 0; Pt[13]= y2- 3;     Pt[14]= x1+ 0; Pt[15]= y1+ 3;

     m_GraphCtxt->Line((void*)ListPt[1],(void*)ListPt[2],m_GraphCtxt->WinGris);
     m_GraphCtxt->Line((void*)ListPt[2],(void*)ListPt[3],m_GraphCtxt->WinGris);
     m_GraphCtxt->Line((void*)ListPt[3],(void*)ListPt[4],m_GraphCtxt->WinGris);
     m_GraphCtxt->Line((void*)ListPt[4],(void*)ListPt[5],m_GraphCtxt->WinGris);
     m_GraphCtxt->Line((void*)ListPt[5],(void*)ListPt[6],m_GraphCtxt->WinNoir);
     m_GraphCtxt->Line((void*)ListPt[6],(void*)ListPt[7],m_GraphCtxt->WinNoir);
     m_GraphCtxt->Line((void*)ListPt[7],(void*)ListPt[8],m_GraphCtxt->WinNoir);
     m_GraphCtxt->Line((void*)ListPt[8],(void*)ListPt[1],m_GraphCtxt->WinNoir);

     if (True) {
       m_GraphCtxt->bar(XC1+6,YC2-12,XC1+8,YC2-8,m_GraphCtxt->WinNoir);
       m_GraphCtxt->bar(XC1+5,YC2-11,XC1+9,YC2-9,m_GraphCtxt->WinNoir); }
     if (Focus) {
       hlgtext=WidthText(Label.StrPtr)+4+18;
       m_GraphCtxt->rectmap(XC1+18,YC2+FntLowPos+2,
               XC1+hlgtext,YC2+FntLowPos-FntHaut-2,m_GraphCtxt->WinNoir,0xaaaaaaaa);
     }
     FntCol=m_GraphCtxt->WinNoir;
     FntX=XC1+20; FntY=YC2-FntHaut; m_GraphCtxt->OutText(Label.StrPtr);
   }
}

void OptionButt::Scan() {
   char Redr=0,callchg=0;
   Child::Scan();
   if (Label.Changed()) Redr=1;
   if (MsDown && (!Focus)) { SetFocus(); Redr=1; }
   if (Focus) {
     if (LostMsDown && MsIn) {
       if (!True) callchg=1;
       Redr=1; True=1; SetFriendFalse();
       if (Changed && callchg) {
         Changed(True); return; }    }
     if (Main->Ascii==0x20 ) { SetTrue(1);  } // espace
     if (Etat && MsDown && MsIn) { Etat=0; Redr=1; }
   }
   if ((!Etat) && ((!MsDown) || (!MsIn)) ) { Etat=1; Redr=1; }
   if (Redr) Redraw();
}

void OptionButt::SetFocus() {
   Node *CN;
   if (!Focus) {
     if (MainCBox) {
       CN=MainCBox->CurFocusInNode;
       if (CN) CN->Item->UnsetFocus();
       Focus=1; MainCBox->CurFocusInNode=ChNode;
       if (!MainCBox->Focus) MainCBox->SetFocus();
     } else {
       Child::SetFocus();
     }
     Redraw();
   }
}

void OptionButt::UnsetFocus() {
   if (Focus) {
     if (MainCBox) {
       ClearAsciiCircBuff();
       ClearKeyCircBuff();
       Focus=Main->Ascii=Main->Key=0;
       Etat=1;
     } else {
       Child::UnsetFocus();
     }
     Redraw();
   }
}

void OptionButt::SetTrue(char ctrue) {
   if (True!=ctrue) {
     if ((True=ctrue)) SetFriendFalse();
     Redraw();
     if (Changed) Changed(True);
   }
}

OptionButt& OptionButt::operator=(char ctrue) {
   if (True!=ctrue) {
     if ((True=ctrue)) SetFriendFalse();
     Redraw();
     if (Changed) Changed(True);
   }
   return *this;
}

void OptionButt::SetFriendFalse() {
   List *PList=MyChList;
   PList->SetFirstNode2();
   if (PList->CurNode2!=NULL) {
     do {
        if (PList->CurNode2!=ChNode &&
	    PList->CurNode2->Item->Visible &&
	    PList->CurNode2->Item->Type==ID_OptionButt)
            ((OptionButt*)(PList->CurNode2->Item))->SetTrue(0);
	  PList->GoNextNode2();
     } while (PList->CurNode2!=NULL);
   }
}

//**************** class ListBox***********************************
//*****************************************************************

ListBox::ListBox(int x1,int y1,int x2,int y2,MainWin *Father,ListString *ls)
        : Child(x1,y1,x2,y2,Father,NULL)  {
   Changed=NULL;  ValidateString=NULL;
   etatbth=etatbtb=bthdown=btbdown=bardown=scrldown=pbardown=listdown=0;
   PosCursor=OldPosCursor=LastPosCursor=-1;
   LastFirstInd=FirstInd=0; MaxNbItem=(y2-y1-6)/(FntHaut+1);
   LStr= new ListString;
   if (ls) {
     int nb=ls->NbElement();
     for (int i=0;i<nb;i++)
       LStr->Add((*ls)[i]->StrPtr);
   }
   LStr->Changed();
   TimeAcc=0.0;
   xbth=x2-14; ybth=y2-14;
   xbtb=x2-14; ybtb=y1+2;
   YPosBar=0;
   if ((scrollact=(LStr->NbElement()>MaxNbItem && XC2-XC1>30 && YC2-YC1>64))) {
     HautBar=((ybth-ybtb-14)*MaxNbItem)/LStr->NbElement();
     if (HautBar<8) HautBar=8;
     YPosBar=((ybth-ybtb-14-HautBar)*FirstInd)/(LStr->NbElement()-MaxNbItem);
   }
   Redraw();
}

ListBox::~ListBox() {
   delete LStr;
}

void ListBox::Draw() {
   int i,nb=LStr->NbElement();
   DgView V1,V2;
   m_GraphCtxt->bar(XC1,YC1,XC2,YC2,m_GraphCtxt->WinBlanc/*+(random()%100)*/);
   m_GraphCtxt->CadBoxWin(XC1,YC1,XC2,YC2);
   V1.OrgX=XC1+3+CurSurf.OrgX;  V1.OrgY=YC1+3+CurSurf.OrgY;
   V1.MinX=V1.MinY=0;
   V1.MaxX=XC2-XC1-6; V1.MaxY=YC2-YC1-6;
   if (scrollact) GetSurfView(&CurSurf,&V2);
   SetSurfInView(&CurSurf,&V1);

   ViewClearText(&V1); FntY--;
   for (i=FirstInd;i<FirstInd+MaxNbItem;i++) {
     if (i>=nb || i<0) break;
     if (i==PosCursor) {
       m_GraphCtxt->bar(XC1+3-V1.OrgX+MyVActiv->OrgX,YC2-FntHaut-4-(i-FirstInd)*(FntHaut+1)-V1.OrgY+MyVActiv->OrgY,
           XC2-3-V1.OrgX+MyVActiv->OrgX,YC2-3-(i-FirstInd)*(FntHaut+1)-V1.OrgY+MyVActiv->OrgY,m_GraphCtxt->WinRougeF);
       if (Focus)
         m_GraphCtxt->rectmap(XC1+3-V1.OrgX+MyVActiv->OrgX,YC2-FntHaut-4-(i-FirstInd)*(FntHaut+1)-V1.OrgY+MyVActiv->OrgY,
	   XC2-3-V1.OrgX+MyVActiv->OrgX,YC2-3-(i-FirstInd)*(FntHaut+1)-V1.OrgY+MyVActiv->OrgY,
           m_GraphCtxt->WinBlanc,0xaaaaaaaa);
       FntCol=m_GraphCtxt->WinBlanc;
       m_GraphCtxt->OutText((*LStr)[i]->StrPtr);
     } else {
       FntCol=m_GraphCtxt->WinNoir;
       m_GraphCtxt->OutText((*LStr)[i]->StrPtr);
     }
     FntX=V1.MinX;
     FntY-=(FntHaut+1); }

   if (scrollact) {
     SetSurfView(&CurSurf,&V2);
     V2.MinX=XC1; V2.MinY=YC1;
     V2.MaxX=XC2; V2.MaxY=YC2;
     SetSurfInView(&CurSurf,&V2);
     m_GraphCtxt->DrawButtFHaut(xbth,ybth,etatbth);
     m_GraphCtxt->bar(xbtb,ybtb+13,xbth+12,ybth-1,m_GraphCtxt->WinGrisC);
     m_GraphCtxt->DrawButtFBas(xbtb,ybtb,etatbtb);

     m_GraphCtxt->bar(xbth,ybth-1-YPosBar-HautBar,xbth+12,ybth-1-YPosBar,m_GraphCtxt->WinGris);
     m_GraphCtxt->CadWin(xbth,ybth-1-YPosBar-HautBar,xbth+12,ybth-1-YPosBar);
   }
}

void ListBox::Scan() {
   int MyMsX=MsX-Main->X-MyVActiv->OrgX,MyMsY=MsY-Main->Y-MyVActiv->OrgY;
   int oldPosSynch,canDelaystep;
   Child::Scan();
   if (MsDown && (!Focus)) SetFocus();
   oldlistdown=listdown;
   if (LStr->NbElement()) {
     if (LStr->Changed()) {
       if (FirstInd+MaxNbItem-1>=LStr->NbElement()) {
         FirstInd=LStr->NbElement()-MaxNbItem-1;
         if (FirstInd<0) FirstInd=0;
       }
       if ((scrollact=(LStr->NbElement()>MaxNbItem && XC2-XC1>30 && YC2-YC1>64))) {
         HautBar=((ybth-ybtb-14)*MaxNbItem)/LStr->NbElement();
	 if (HautBar<8) HautBar=8;
         YPosBar=((ybth-ybtb-14-HautBar)*FirstInd)/(LStr->NbElement()-MaxNbItem);
       }
       LastFirstInd=FirstInd; Redraw();
       if (PosCursor>=0 && PosCursor<LStr->NbElement()) {
         if (Changed) Changed((*LStr)[PosCursor],PosCursor);
       }
     }
     if (PosCursor!=-1 && (PosCursor<0 || PosCursor>=LStr->NbElement())) {
       LastPosCursor=PosCursor=-1;
       LastFirstInd=FirstInd=0; Redraw();
       if (Changed) Changed(NULL,PosCursor);
       return;
     }
     if (MsDown&& (!bthdown)&& (!btbdown)&& (!bardown)&&
         (!listdown)&& (!pbardown) && (!scrldown)) {
       if (scrollact && m_GraphCtxt->In(xbtb,ybtb,xbtb+12,ybtb+12,MyMsX,MyMsY)) btbdown=1;
       else if (scrollact && m_GraphCtxt->In(xbth,ybth,xbth+12,ybth+12,MyMsX,MyMsY)) bthdown=1;
         else if (scrollact && m_GraphCtxt->In(xbth,ybth-YPosBar-HautBar-1,xbth+12,ybth-YPosBar-1,MyMsX,MyMsY))
	  { bardown=1; msxdown=MyMsX; msydown=MyMsY; downYPosBar=YPosBar; }
           else {
             if (scrollact && MyMsX>=xbth)
               scrldown=1;
             else listdown=1;
           }
       if (listdown) {
         if (DgTimerFreq) oldbtime=((DgTime/(DgTimerFreq/8))&1);
       } else if (DgTimerFreq) oldbtime=((DgTime/(DgTimerFreq/16))&1);
       InitSynch(SynchBuff,&PosSynch,DGUI_REPEAT_RATE);
       TimeAcc=0.0;
       firstdown=1; Redraw();
     }
     if (MsDown) {
       //if (LStr->NbElement()>0)
       //  *(LStr->Element(0))=TimeAcc;
       oldPosSynch=PosSynch;
       Synch(SynchBuff,&PosSynch);
       canDelaystep=(oldPosSynch!=PosSynch)?1:0;
       TimeAcc=SynchAccTime(SynchBuff);
       //firstdown=0;
     }
     if (Focus) {

       if (Main->Key)
         switch(Main->Key) {
           case 0xc8:  // haut
	     if (PosCursor==-1) PosCursor=0;
             if (PosCursor) PosCursor--;
	     if (AjFirstInd()) Redraw(); break;
           case 0xd0:  // bas
	     if (PosCursor==-1) PosCursor=0;
             if (PosCursor<LStr->NbElement()-1) PosCursor++;
	     if (AjFirstInd()) Redraw(); break;
           case 0xc9:  // PG UP
             PgUP();
             break;
           case 0xd1:  // PG DOWN
             PgDOWN();
             break;
           case 0xc7:  // DEB
	     if (PosCursor==-1) PosCursor=0;
             OldPosCursor=PosCursor; PosCursor=0;
	     if (AjFirstInd()|| OldPosCursor!=PosCursor) Redraw(); break;
           case 0xcf:  // FIN
	     if (PosCursor==-1) PosCursor=0;
             OldPosCursor=PosCursor;
	     PosCursor=LStr->NbElement()-1;
	     if (AjFirstInd()|| OldPosCursor!=PosCursor) Redraw();
         }

       if ((!MsDown)&& (btbdown|| bthdown|| bardown|| pbardown|| listdown|| scrldown)) {
         btbdown=bthdown=bardown=pbardown=scrldown=listdown=0; Redraw(); }


       if (bardown) {
         if (YPosBar!=(downYPosBar+(msydown-MyMsY))) {
	   YPosBar=(downYPosBar+(msydown-MyMsY));
           if (YPosBar>ybth-ybtb-14-HautBar) YPosBar=ybth-ybtb-14-HautBar;
	   if (YPosBar<0) YPosBar=0;
           FirstInd=(YPosBar*(LStr->NbElement()-MaxNbItem))/(ybth-ybtb-14-HautBar);
           YPosBar=((ybth-ybtb-14-HautBar)*FirstInd)/(LStr->NbElement()-MaxNbItem);
	   Redraw();
	 }
       }
       // PG UP or PG DOWN ?
       if (MsDown && scrollact && scrldown) {
          if (m_GraphCtxt->In(xbth,ybth-YPosBar-HautBar-1,xbth+12,ybth-YPosBar-1,MyMsX,MyMsY)) {
            scrldown=0; bardown=1; msxdown=MyMsX; msydown=MyMsY; downYPosBar=YPosBar; }
          else {
            if (MyMsY<ybth-YPosBar-1 && ((TimeAcc>DGUI_DELAY_REPEAT && canDelaystep) || firstdown))
              PgDOWN();
            else {
              if ((TimeAcc>DGUI_DELAY_REPEAT && canDelaystep) || firstdown) PgUP();
            }
            firstdown=0;
          }
       }
       if (listdown) {
         if ((MyMsY<=YC2-3 && MyMsY>(YC2-3-(MaxNbItem*(FntHaut+1)))) || (!DgTimerFreq)) {
           OldPosCursor=PosCursor;
	   if (YC2-3>=MyMsY)
	     PosCursor=FirstInd+(YC2-3-MyMsY)/(FntHaut+1);
	   else PosCursor=FirstInd-(MyMsY-YC2+3)/(FntHaut+1)-1;
	   if (PosCursor<0) PosCursor=0;
	   if (PosCursor>=LStr->NbElement()) PosCursor=LStr->NbElement()-1;
	   if (AjFirstInd()|| OldPosCursor!=PosCursor) Redraw();
         } else if ((DgTimerFreq && (oldbtime^((DgTime/(DgTimerFreq/8))&1))) || firstdown) {
             OldPosCursor=PosCursor; firstdown=0;
  	     if (DgTimerFreq) oldbtime=((DgTime/(DgTimerFreq/8))&1);
	     if (YC2-3>=MyMsY)
	       PosCursor=FirstInd+(YC2-3-MyMsY)/(FntHaut+1);
	     else PosCursor=FirstInd-((MyMsY-YC2+3)/(FntHaut+1))-1;
	     if (PosCursor<0) PosCursor=0;
	     if (PosCursor>=LStr->NbElement()) PosCursor=LStr->NbElement()-1;
	     if (AjFirstInd()|| OldPosCursor!=PosCursor) Redraw();
         }
       }
       if (scrollact) {
         if (MsIn && btbdown && m_GraphCtxt->In(xbtb,ybtb,xbtb+12,ybtb+12,MyMsX,MyMsY)) {
           if (DgTimerFreq) {
             if ((FirstInd+MaxNbItem)<LStr->NbElement() &&
	         (/*(oldbtime^((Time/(TimerFreq/16))&1))*/canDelaystep || firstdown))
               {
                 if (TimeAcc>DGUI_DELAY_REPEAT || firstdown) FirstInd++;
                 etatbtb=1; Redraw();
	         if (!firstdown) oldbtime=((DgTime/(DgTimerFreq/16))&1);
	         firstdown=0; }
               else if (!etatbtb) { etatbtb=1; Redraw(); }
             } else {
               if ((FirstInd+MaxNbItem)<LStr->NbElement())
                 { FirstInd++; etatbtb=1; Redraw(); }
                 else if (!etatbtb) { etatbtb=1; Redraw(); }
             }
         } else if (etatbtb) { etatbtb=0; Redraw(); }

         if (MsIn && bthdown && m_GraphCtxt->In(xbth,ybth,xbth+12,ybth+12,MyMsX,MyMsY)) {
           if (DgTimerFreq) {
             if (FirstInd &&
               (/*(oldbtime^((Time/(TimerFreq/16))&1))*/canDelaystep || firstdown)) {
                 if (TimeAcc>DGUI_DELAY_REPEAT || firstdown) FirstInd--;
                 etatbth=1; Redraw();
	         if (!firstdown) oldbtime=((DgTime/(DgTimerFreq/16))&1);
	         firstdown=0; }
             else if (!etatbth) { etatbth=1; Redraw(); }
           } else {
             if (FirstInd) { FirstInd--; etatbth=1; Redraw(); }
             else if (!etatbth) { etatbth=1; Redraw(); }
           }
         } else if (etatbth) { etatbth=0; Redraw(); }
       }
     }
     if (DblClick && oldlistdown && MyMsY<=YC2-3 && LastPosCursor==PosCursor &&
         MyMsY>(YC2-3-(MaxNbItem*(FntHaut+1))))
       { UnsetFocus(); Validate(); return; }
     if (LastFirstInd!=FirstInd && scrollact) {
       HautBar=((ybth-ybtb-14)*MaxNbItem)/LStr->NbElement();
       if (HautBar<8) HautBar=8;
       YPosBar=((ybth-ybtb-14-HautBar)*FirstInd)/(LStr->NbElement()-MaxNbItem);
       LastFirstInd=FirstInd; Redraw(); }


     if (LastPosCursor!=PosCursor) {
       if (Changed) {
         if (PosCursor!=-1 && PosCursor>=0 && PosCursor<LStr->NbElement()) {
           LastPosCursor=PosCursor; Redraw();
           Changed((*LStr)[PosCursor],PosCursor);
	 }
	 else {
           LastPosCursor=PosCursor=-1; Redraw();
           Changed(NULL,PosCursor);
	 }
       }
       else { LastPosCursor=PosCursor; Redraw(); }
       return;
     }
   }
   else
     if (PosCursor!=-1 || LStr->Changed()) {
       PosCursor=-1; scrollact=0;
       LastPosCursor=PosCursor;
       LastFirstInd=FirstInd=0; Redraw();
       if (Changed) Changed(NULL,PosCursor);
       return;
     }
}

void ListBox::SetFocus() {
   Child::SetFocus();
   Redraw();
}

void ListBox::UnsetFocus() {
   Child::UnsetFocus();
   Redraw();
}

void ListBox::Validate() {
   if (ValidateString) ValidateString((*LStr)[PosCursor],PosCursor);
}

void ListBox::EnsureVisible(int index) {
   if (index>=LStr->NbElement()) return;
   if (index>=(FirstInd+MaxNbItem)) {
     FirstInd=index-MaxNbItem+1;
     Redraw();
   }
   else
     if (index<FirstInd && index>=0) {
       FirstInd=index;
       Redraw();
     }
}

void ListBox::PgUP() {
  int OldPosCursor;
  PosCursor-=(MaxNbItem-1);
  if (PosCursor<0) PosCursor=0;
  OldPosCursor=PosCursor;
  if (AjFirstInd()|| OldPosCursor!=PosCursor)
    Redraw();
}

void ListBox::PgDOWN() {
  int OldPosCursor;
  if (PosCursor==-1) PosCursor=0;
  OldPosCursor=PosCursor;
  if (PosCursor<LStr->NbElement()-MaxNbItem-1 && LStr->NbElement()>MaxNbItem)
    PosCursor+=(MaxNbItem-1);
  else
    PosCursor=LStr->NbElement()-1;
  if (AjFirstInd()|| OldPosCursor!=PosCursor)
    Redraw();
}

int ListBox::AjFirstInd() {
   if (PosCursor>=(FirstInd+MaxNbItem)) {
     FirstInd=PosCursor-MaxNbItem+1;
     return 1;
   }
   else
     if (PosCursor<FirstInd && PosCursor!=-1) {
       FirstInd=PosCursor; return 1;
     }

   return 0;
}

//**************** class VtScrollBar*******************************
//*****************************************************************

VtScrollBar::VtScrollBar(int x,int y1,int y2,MainWin *Father,
  int MinVal,int MaxVal) : Child(x,y1,x+16,y2,Father,NULL)  {
   xbth=XC2-14; ybth=YC2-14;
   xbtb=XC2-14; ybtb=YC1+2;
   VtMinVal=MinVal; VtMaxVal=MaxVal;
   etatbth=etatbtb=bthdown=btbdown=bardown=pbardown=scrldown=0;
   Changed=NULL;
   YPosBar=0; OldVtVal=VtVal=VtMinVal;
   MaxNbItem=ybth-ybtb-14;
   HautBar=(MaxNbItem)/(VtMaxVal-VtMinVal+1);
   if (HautBar<8) HautBar=8;
   YPosBar=(VtMaxVal!=VtMinVal)?((float)(MaxNbItem-14-HautBar)*(float)(VtVal-VtMinVal))/(VtMaxVal-VtMinVal):0;
   Redraw();
}

VtScrollBar::~VtScrollBar() {}

void VtScrollBar::Draw() {
   m_GraphCtxt->DrawButtFHaut(xbth,ybth,etatbth);
   m_GraphCtxt->bar(xbtb,ybtb+13,xbth+12,ybth-1,m_GraphCtxt->WinGrisC);
   m_GraphCtxt->DrawButtFBas(xbtb,ybtb,etatbtb);
   m_GraphCtxt->bar(xbth,ybth-1-YPosBar-HautBar,xbth+12,ybth-1-YPosBar,m_GraphCtxt->WinGris);
   if (Focus)
     m_GraphCtxt->rectmap(xbth+2,ybth+1-YPosBar-HautBar,xbth+10,ybth-3-YPosBar,m_GraphCtxt->WinNoir,0xaaaaaaaa);
   m_GraphCtxt->CadWin(xbth,ybth-1-YPosBar-HautBar,xbth+12,ybth-1-YPosBar);
}

void VtScrollBar::Scan() {
   int MyMsX=MsX-Main->X-MyVActiv->OrgX,MyMsY=MsY-Main->Y-MyVActiv->OrgY;
   int oldPosSynch,canDelaystep;
   char redr=0;
   Child::Scan();
   if (MsDown && (!Focus)) SetFocus();
   if (MsDown&& (!bthdown)&& (!btbdown)&& (!bardown)&& (!pbardown) && (!scrldown)) {
     if (m_GraphCtxt->In(xbtb,ybtb,xbtb+12,ybtb+12,MyMsX,MyMsY))
       btbdown=1;
       else if (m_GraphCtxt->In(xbth,ybth,xbth+12,ybth+12,MyMsX,MyMsY))
         bthdown=1;
         else if (m_GraphCtxt->In(xbth,ybth-YPosBar-HautBar-1,xbth+12,ybth-YPosBar-1,MyMsX,MyMsY))
   	 { bardown=1; msxdown=MyMsX; msydown=MyMsY; downYPosBar=YPosBar; }
         else
           scrldown=1;
      firstdown=1;
      InitSynch(SynchBuff,&PosSynch,DGUI_REPEAT_RATE);
      firstdown=1; Redraw();
   }
   if (MsDown) {
     oldPosSynch=PosSynch;
     Synch(SynchBuff,&PosSynch);
     canDelaystep=(oldPosSynch!=PosSynch)?1:0;
     TimeAcc=SynchAccTime(SynchBuff);
       //firstdown=0;
   }
   if (Focus) {
     if (Main->Key)
       switch(Main->Key) {
         case 0xc8:  // haut
           if (VtVal>VtMinVal) VtVal--; redr=1; Main->Key=0; break;
         case 0xd0:  // bas
           if (VtVal<VtMaxVal) VtVal++; redr=1; Main->Key=0; break;
         case 0xc9:  // PG UP
           PgUP();
	   redr=1; Main->Key=0; break;
         case 0xd1:  // PG DOWN
           PgDOWN();
	   Main->Key=0; redr=1; break;
         case 0xc7:  // DEB
           OldVtVal=VtVal; VtVal=VtMinVal; redr=1; Main->Key=0; break;
         case 0xcf:  // FIN
           OldVtVal=VtVal; VtVal=VtMaxVal; redr=1; Main->Key=0; break;
       }
     if (MsIn && btbdown && m_GraphCtxt->In(xbtb,ybtb,xbtb+12,ybtb+12,MyMsX,MyMsY)) {
       if (DgTimerFreq) {
         if (VtVal<VtMaxVal &&
           (canDelaystep || firstdown)) {
	   if (TimeAcc>DGUI_DELAY_REPEAT || firstdown) VtVal++; etatbtb=1; redr=1;
           firstdown=0;
         } else if (!etatbtb) { etatbtb=1; redr=1; }
       } else {
         if (VtVal<VtMaxVal)
         { VtVal++; etatbtb=1; redr=1; }
         else if (!etatbtb) { etatbtb=1; redr=1; }
       }
     } else if (etatbtb) { etatbtb=0; redr=1; }

     if (MsIn && bthdown && m_GraphCtxt->In(xbth,ybth,xbth+12,ybth+12,MyMsX,MyMsY)) {
       if (DgTimerFreq) {
         if (VtVal>VtMinVal && (canDelaystep || firstdown)) {
           if (TimeAcc>DGUI_DELAY_REPEAT || firstdown) VtVal--;
           etatbth=1; redr=1;
           firstdown=0;
         }
         else if (!etatbth) { etatbth=1; redr=1; }
       } else {
         if (VtVal>VtMinVal) {
            VtVal--; etatbth=1; redr=1; }
          else if (!etatbth) { etatbth=1; redr=1; }
       }
     } else if (etatbth) { etatbth=0; redr=1; }
     if (bardown && VtMaxVal>VtMinVal) {
       if (YPosBar!=(downYPosBar+(msydown-MyMsY))) {
         YPosBar=(downYPosBar+(msydown-MyMsY));
         if (YPosBar>ybth-ybtb-14-HautBar) YPosBar=ybth-ybtb-14-HautBar;
         if (YPosBar<0) YPosBar=0;
         VtVal=(((float)YPosBar*(float)(VtMaxVal-VtMinVal))/(ybth-ybtb-14-HautBar))+VtMinVal;
	 YPosBar=(VtMaxVal!=VtMinVal)?((float)(ybth-ybtb-14-HautBar)*(float)(VtVal-VtMinVal))/(VtMaxVal-VtMinVal):0;
         redr=1;
       }
     }
     // PG UP or PG DOWN ?
     if (MsDown && scrldown) {
       if (m_GraphCtxt->In(xbth,ybth-YPosBar-HautBar-1,xbth+12,ybth-YPosBar-1,MyMsX,MyMsY)) {
         scrldown=0; bardown=1; msxdown=MyMsX; msydown=MyMsY; downYPosBar=YPosBar; }
       else {
         if (MyMsY<ybth-YPosBar-1 && ((TimeAcc>DGUI_DELAY_REPEAT && canDelaystep) || firstdown))
           PgDOWN();
         else {
           if ((TimeAcc>DGUI_DELAY_REPEAT && canDelaystep) || firstdown) PgUP();
         }
         redr=1;
         firstdown=0;
       }
     }

   }
   if (!MsDown) {
     btbdown=bthdown=bardown=pbardown=scrldown=0; redr=1; }
   if (OldVtVal!=VtVal) {
     YPosBar=(VtMaxVal!=VtMinVal)?((float)(ybth-ybtb-14-HautBar)*(float)(VtVal-VtMinVal))/(VtMaxVal-VtMinVal):0;
     if (redr) { Redraw(); redr=0; }
     OldVtVal=VtVal;
     if (Changed) { Changed(VtVal); return; }
   }
   if (redr) {
     Redraw();
   }
}

void VtScrollBar::SetFocus() {
   Child::SetFocus();
   Redraw();
}

void VtScrollBar::UnsetFocus() {
   Child::UnsetFocus();
   Redraw();
}

void VtScrollBar::SetVal(int NewVal) {
   OldVtVal=VtVal;
   if (NewVal!=VtVal) {
     VtVal=NewVal;
     if (VtVal<VtMinVal) VtVal=VtMinVal;
     if (VtVal>VtMaxVal) VtVal=VtMaxVal;
     if (OldVtVal!=VtVal) {
       YPosBar=(VtMaxVal!=VtMinVal)?((float)(MaxNbItem-14-HautBar)*(float)(VtVal-VtMinVal))/(VtMaxVal-VtMinVal):0;
       Redraw(); if (Changed) Changed(VtVal); }
   }
   Redraw();
}

int  VtScrollBar::GetVal() {
   return VtVal;
}

void VtScrollBar::SetMinMaxVal(int MinVal,int MaxVal) {
   int OldVtVal=VtVal;
   if (MinVal==VtMinVal && MaxVal==VtMaxVal) return;
   VtMinVal=MinVal; VtMaxVal=MaxVal;
   HautBar=(MaxNbItem)/(VtMaxVal-VtMinVal+1);
   if (HautBar<8) HautBar=8;
   if (VtVal<VtMinVal) VtVal=VtMinVal;
   if (VtVal>VtMaxVal) VtVal=VtMaxVal;
   YPosBar=(VtMaxVal!=VtMinVal)?((float)(MaxNbItem-14-HautBar)*(float)(VtVal-VtMinVal))/(VtMaxVal-VtMinVal):0;
   Redraw();
   if (VtVal!=OldVtVal)
     if (Changed) Changed(VtVal);
}

void VtScrollBar::GetMinMaxVal(int *MinVal,int *MaxVal) {
   *MinVal=VtMinVal; *MaxVal=VtMaxVal;
}

void VtScrollBar::PgUP() {
   if ((VtMaxVal-VtMinVal)>=((ybth-ybtb-14)/HautBar))
     VtVal-=(VtMaxVal-VtMinVal)/((ybth-ybtb-14)/HautBar);
   else
     VtVal=VtMinVal;
   if (VtVal<VtMinVal) VtVal=VtMinVal;
}

void VtScrollBar::PgDOWN() {
   if ((VtMaxVal-VtMinVal)>=((ybth-ybtb-14)/HautBar))
     VtVal+=(VtMaxVal-VtMinVal)/((ybth-ybtb-14)/HautBar);
   else
     VtVal=VtMaxVal;
   if (VtVal>VtMaxVal) VtVal=VtMaxVal;
}

VtScrollBar& VtScrollBar::operator=(int NewVal) {
   OldVtVal=VtVal;
   if (NewVal!=VtVal) {
     VtVal=NewVal;
     if (VtVal<VtMinVal) VtVal=VtMinVal;
     if (VtVal>VtMaxVal) VtVal=VtMaxVal;
     if (OldVtVal!=VtVal) {
       YPosBar=(VtMaxVal!=VtMinVal)?((float)(MaxNbItem-14-HautBar)*(float)(VtVal-VtMinVal))/(VtMaxVal-VtMinVal):0;
       Redraw(); if (Changed) Changed(VtVal); }
   }
   return *this;
}

//**************** class HzScrollBar*******************************
//*****************************************************************

HzScrollBar::HzScrollBar(int x1,int x2,int y,MainWin *Father,
  int MinVal,int MaxVal) : Child(x1,y,x2,y+16,Father,NULL)  {
   xbth=XC1;    ybth=YC2-14;
   xbtb=XC2-14; ybtb=YC2-14;
   HzMinVal=MinVal; HzMaxVal=MaxVal;
   etatbth=etatbtb=bthdown=btbdown=bardown=pbardown=scrldown=0;
   Changed=NULL;
   YPosBar=0; OldHzVal=HzVal=HzMinVal;
   MaxNbItem=xbtb-xbth-14;
   HautBar=(MaxNbItem)/(HzMaxVal-HzMinVal+1);
   if (HautBar<8) HautBar=8;
   YPosBar=(HzMaxVal!=HzMinVal)?((float)(xbtb-xbth-14-HautBar)*(float)(HzVal-HzMinVal))/(HzMaxVal-HzMinVal):0;
   Redraw();
}

HzScrollBar::~HzScrollBar() {}

void HzScrollBar::Draw() {
   m_GraphCtxt->DrawButtFGauche(xbth,ybth,etatbth);
   m_GraphCtxt->bar(xbth+13,ybth,xbtb-1,ybth+12,m_GraphCtxt->WinGrisC);
   m_GraphCtxt->DrawButtFDroit(xbtb,ybtb,etatbtb);
   m_GraphCtxt->bar(xbth+YPosBar+13,ybth,xbth+YPosBar+HautBar+13,ybth+12,m_GraphCtxt->WinGris);
   if (Focus)
     m_GraphCtxt->rectmap(xbth+YPosBar+15,ybth+2,xbth+YPosBar+HautBar+11,ybth+10,m_GraphCtxt->WinNoir,0xaaaaaaaa);
   m_GraphCtxt->CadWin(xbth+YPosBar+13,ybth,xbth+YPosBar+HautBar+13,ybth+12);
}

void HzScrollBar::Scan() {
   int MyMsX=MsX-Main->X-MyVActiv->OrgX,MyMsY=MsY-Main->Y-MyVActiv->OrgY;
   int oldPosSynch,canDelaystep,loldval;
   char redr=0;
   Child::Scan();
   if (MsDown && (!Focus)) SetFocus();
   if (MsDown&& (!bthdown)&& (!btbdown)&& (!bardown)&& (!pbardown) && (!scrldown)) {
     if (m_GraphCtxt->In(xbtb,ybtb,xbtb+12,ybtb+12,MyMsX,MyMsY)) btbdown=1;
       else if (m_GraphCtxt->In(xbth,ybth,xbth+12,ybth+12,MyMsX,MyMsY)) bthdown=1;
         else if (m_GraphCtxt->In(xbth+YPosBar+13,ybth,xbth+YPosBar+HautBar+13,ybth+12,MyMsX,MyMsY))
   	 { bardown=1; msxdown=MyMsX; msydown=MyMsY; downYPosBar=YPosBar; }
         else
           scrldown=1;
     firstdown=1;
     InitSynch(SynchBuff,&PosSynch,DGUI_REPEAT_RATE);
     firstdown=1; Redraw();
   }
   if (MsDown) {
     oldPosSynch=PosSynch;
     Synch(SynchBuff,&PosSynch);
     canDelaystep=(oldPosSynch!=PosSynch)?1:0;
     TimeAcc=SynchAccTime(SynchBuff);
   }
   if (Focus) {
     if (Main->Key)
       switch(Main->Key) {
         case 0xcb:  // gauche
           if (HzVal>HzMinVal) HzVal--; redr=1; Main->Key=0; break;
         case 0xcd:  // droit
           if (HzVal<HzMaxVal) HzVal++; redr=1; Main->Key=0; break;
         case 0xc9:  // PG UP
           PgUP();
	   redr=1; Main->Key=0; break;
         case 0xd1:  // PG DOWN
           PgDOWN();
           Main->Key=0; redr=1; break;
         case 0xc7:  // DEB
           OldHzVal=HzVal; HzVal=HzMinVal; redr=1; Main->Key=0; break;
         case 0xcf:  // FIN
           OldHzVal=HzVal; HzVal=HzMaxVal; redr=1; Main->Key=0; break;
       }
     if (MsIn && btbdown && m_GraphCtxt->In(xbtb,ybtb,xbtb+12,ybtb+12,MyMsX,MyMsY)) {
       if (DgTimerFreq) {
         if (HzVal<HzMaxVal && (canDelaystep || firstdown)) {
           if (TimeAcc>DGUI_DELAY_REPEAT || firstdown) HzVal++;
           etatbtb=1; redr=1;
           firstdown=0;
         } else if (!etatbtb) { etatbtb=1; redr=1; }
       } else {
         if (HzVal<HzMaxVal)
         { HzVal++; etatbtb=1; redr=1; }
         else if (!etatbtb) { etatbtb=1; redr=1; }
       }
     } else if (etatbtb) { etatbtb=0; redr=1; }

     if (MsIn && bthdown && m_GraphCtxt->In(xbth,ybth,xbth+12,ybth+12,MyMsX,MyMsY)) {
       if (DgTimerFreq) {
         if (HzVal>HzMinVal && (canDelaystep || firstdown)) {
           if (TimeAcc>DGUI_DELAY_REPEAT || firstdown) HzVal--;
           etatbth=1; redr=1;
           firstdown=0;
         }
         else if (!etatbth) { etatbth=1; redr=1; }
       } else {
         if (HzVal>HzMinVal) { HzVal--; etatbth=1; redr=1; }
          else if (!etatbth) { etatbth=1; redr=1; }
       }
     } else if (etatbth) { etatbth=0; redr=1; }
     if (bardown && HzMaxVal>HzMinVal) {
       if (YPosBar!=(downYPosBar+(MyMsX-msxdown))) {
         YPosBar=(downYPosBar+(MyMsX-msxdown));
         if (YPosBar>xbtb-xbth-14-HautBar) YPosBar=xbtb-xbth-14-HautBar;
         if (YPosBar<0) YPosBar=0;
         HzVal=(((float)YPosBar*(float)(HzMaxVal-HzMinVal))/(xbtb-xbth-14-HautBar))+HzMinVal;
         YPosBar=(HzMaxVal!=HzMinVal)?((float)(xbtb-xbth-14-HautBar)*(float)(HzVal-HzMinVal))/(HzMaxVal-HzMinVal):0;
         redr=1;
       }
     }
     if (scrldown && ((canDelaystep && TimeAcc>DGUI_DELAY_REPEAT) || firstdown)) {
       loldval=HzVal;
       if (m_GraphCtxt->In(xbth+YPosBar+13,ybth,xbth+YPosBar+HautBar+13,ybth+12,MyMsX,MyMsY)) {
         scrldown=0; bardown=1; msxdown=MyMsX; msydown=MyMsY; downYPosBar=YPosBar;
       }
       else {
         if (MyMsX<xbth+YPosBar+13)
           PgUP();
         else
           PgDOWN();
       }
       firstdown=0;
       if (loldval!=HzVal) redr=1;
     }

   }
   if (!MsDown) {
     btbdown=bthdown=bardown=pbardown=scrldown=0; redr=1; }
   if (OldHzVal!=HzVal) {
     YPosBar=(HzMaxVal!=HzMinVal)?((float)(xbtb-xbth-14-HautBar)*(float)(HzVal-HzMinVal))/(HzMaxVal-HzMinVal):0;
     if (redr) { Redraw(); redr=0; }
     OldHzVal=HzVal;
     if (Changed) { Changed(HzVal); return; }
   }
   if (redr) {
     Redraw();
   }
}

void HzScrollBar::SetFocus() {
   Child::SetFocus();
   Redraw();
}

void HzScrollBar::UnsetFocus() {
   Child::UnsetFocus();
   Redraw();
}

void HzScrollBar::PgUP() {
   if ((HzMaxVal-HzMinVal)>=((xbtb-xbth-14)/HautBar))
     HzVal-=(HzMaxVal-HzMinVal)/((xbtb-xbth-14)/HautBar);
   else HzVal=HzMinVal;
     if (HzVal<HzMinVal) HzVal=HzMinVal;
}

void HzScrollBar::PgDOWN() {
   if ((HzMaxVal-HzMinVal)>=((xbtb-xbth-14)/HautBar))
     HzVal+=(HzMaxVal-HzMinVal)/((xbtb-xbth-14)/HautBar);
   else HzVal=HzMaxVal;
     if (HzVal>HzMaxVal) HzVal=HzMaxVal;
}

void HzScrollBar::SetVal(int NewVal) {
   OldHzVal=HzVal;
   if (NewVal!=HzVal) {
     HzVal=NewVal;
     if (HzVal<HzMinVal) HzVal=HzMinVal;
     if (HzVal>HzMaxVal) HzVal=HzMaxVal;
     if (OldHzVal!=HzVal) {
       YPosBar=(HzMaxVal!=HzMinVal)?((float)(xbtb-xbth-14-HautBar)*(float)(HzVal-HzMinVal))/(HzMaxVal-HzMinVal):0;
       Redraw(); if (Changed) Changed(HzVal);
     }
   }
   Redraw();
}

int  HzScrollBar::GetVal() {
   return HzVal;
}

void HzScrollBar::SetMinMaxVal(int MinVal,int MaxVal) {
   OldHzVal=HzVal;
   if (MinVal==HzMinVal && MaxVal==HzMaxVal) return;
   HzMinVal=MinVal; HzMaxVal=MaxVal;
   HautBar=(MaxNbItem)/(HzMaxVal-HzMinVal+1);
   if (HautBar<8) HautBar=8;
   if (HzVal<HzMinVal) HzVal=HzMinVal;
   if (HzVal>HzMaxVal) HzVal=HzMaxVal;
   YPosBar=(HzMaxVal!=HzMinVal)?((float)(xbtb-xbth-14-HautBar)*(float)(HzVal-HzMinVal))/(HzMaxVal-HzMinVal):0;
   Redraw();
   if (HzVal!=OldHzVal)
     if (Changed) Changed(HzVal);
}

void HzScrollBar::GetMinMaxVal(int *MinVal,int *MaxVal) {
   *MinVal=HzMinVal; *MaxVal=HzMaxVal;
}

HzScrollBar& HzScrollBar::operator=(int NewVal) {
   OldHzVal=HzVal;
   if (NewVal!=HzVal) {
     HzVal=NewVal;
     if (HzVal<HzMinVal) HzVal=HzMinVal;
     if (HzVal>HzMaxVal) HzVal=HzMaxVal;
     if (OldHzVal!=HzVal) {
       YPosBar=(HzMaxVal!=HzMinVal)?((float)(xbtb-xbth-14-HautBar)*(float)(HzVal-HzMinVal))/(HzMaxVal-HzMinVal):0;
       Redraw(); if (Changed) Changed(HzVal);
     }
   }
   return *this;
}



//**************** class HzSlider*******************************
//*****************************************************************

HzSlider::HzSlider(int x1,int x2,int y,MainWin *Father,
  int MinVal,int MaxVal) : Child(x1,y,x2,y+16,Father,NULL)  {
   xbth=XC1;    ybth=YC2-14;
   xbtb=XC2-7;  ybtb=YC2-14;
   HzMinVal=MinVal; HzMaxVal=MaxVal;
   bardown=0;
   Changed=NULL;
   YPosBar=0; OldHzVal=HzVal=HzMinVal;
   MaxNbItem=xbtb-xbth-7;
   HautBar=8;
   YPosBar=(HzMaxVal!=HzMinVal)?((float)(xbtb-xbth-7-HautBar)*(float)(HzVal-HzMinVal))/(HzMaxVal-HzMinVal):0;
   Redraw();
}

HzSlider::~HzSlider() {}

void HzSlider::Draw() {
   m_GraphCtxt->CadWin(xbth,ybth,xbth+6,ybth+12); // left limit
   m_GraphCtxt->CadWin(xbtb,ybtb,xbtb+6,ybtb+12); // right limit
   // pointer
   m_GraphCtxt->GHline(xbth+7, xbtb-1, ybth+7);
   m_GraphCtxt->CadWin(xbth+YPosBar+7,ybth+6,xbth+YPosBar+HautBar+7,ybth+12);
   m_GraphCtxt->bar(xbth+YPosBar+7+2,ybth+6,xbth+YPosBar+HautBar+7-2,ybth+12-2,m_GraphCtxt->WinGris);
   m_GraphCtxt->line(xbth+YPosBar+7+1,ybth+6,xbth+YPosBar+HautBar/2+7,ybth+2,m_GraphCtxt->WinGrisC);
   m_GraphCtxt->line(xbth+YPosBar+7,ybth+6,xbth+YPosBar+7,ybth+6,m_GraphCtxt->WinGrisC);
   m_GraphCtxt->line(xbth+YPosBar+HautBar/2+6,ybth+2,xbth+YPosBar+HautBar+6,ybth+6,m_GraphCtxt->WinGrisF);
   m_GraphCtxt->line(xbth+YPosBar+HautBar/2+7,ybth+2,xbth+YPosBar+HautBar+7,ybth+6,m_GraphCtxt->WinNoir);
   if (Focus)
     m_GraphCtxt->rectmap(xbth+YPosBar+9,ybth+8,xbth+YPosBar+HautBar+5,ybth+10,m_GraphCtxt->WinNoir,0xaaaaaaaa);
}

void HzSlider::Scan() {
   int MyMsX=MsX-Main->X-MyVActiv->OrgX,MyMsY=MsY-Main->Y-MyVActiv->OrgY;
   int loldval;
   char redr=0;
   Child::Scan();
   if (MsDown && (!Focus)) SetFocus();
   if (MsDown && (!bardown)) {
     if (m_GraphCtxt->In(xbth+YPosBar+7,ybth,xbth+YPosBar+HautBar+7,ybth+12,MyMsX,MyMsY))
   	 { bardown=1; msxdown=MyMsX; msydown=MyMsY; downYPosBar=YPosBar; }
     firstdown=1;
   }
   if (Focus) {
     if (Main->Key)
       switch(Main->Key) {
         case 0xcb:  // gauche
           if (HzVal>HzMinVal) HzVal--; redr=1; Main->Key=0; break;
         case 0xcd:  // droit
           if (HzVal<HzMaxVal) HzVal++; redr=1; Main->Key=0; break;
         case 0xc7:  // DEB
           OldHzVal=HzVal; HzVal=HzMinVal; redr=1; Main->Key=0; break;
         case 0xcf:  // FIN
           OldHzVal=HzVal; HzVal=HzMaxVal; redr=1; Main->Key=0; break;
       }
     if (bardown && HzMaxVal>HzMinVal) {
       if (YPosBar!=(downYPosBar+(MyMsX-msxdown))) {
         YPosBar=(downYPosBar+(MyMsX-msxdown));
         if (YPosBar>xbtb-xbth-7-HautBar) YPosBar=xbtb-xbth-7-HautBar;
         if (YPosBar<0) YPosBar=0;
         HzVal=(((float)YPosBar*(float)(HzMaxVal-HzMinVal))/(xbtb-xbth-7-HautBar))+HzMinVal;
         YPosBar=(HzMaxVal!=HzMinVal)?((float)(xbtb-xbth-7-HautBar)*(float)(HzVal-HzMinVal))/(HzMaxVal-HzMinVal):0;
         redr=1;
       }
     }
   }
   if (!MsDown) {
     bardown=0; redr=1; }
   if (OldHzVal!=HzVal) {
     YPosBar=(HzMaxVal!=HzMinVal)?((float)(xbtb-xbth-7-HautBar)*(float)(HzVal-HzMinVal))/(HzMaxVal-HzMinVal):0;
     OldHzVal=HzVal;
     if (redr) { redr=1; }
     if (Changed!=NULL) Changed(HzVal);
   }
   if (redr) {
     Redraw();
   }
}

void HzSlider::SetFocus() {
   Child::SetFocus();
   Redraw();
}

void HzSlider::UnsetFocus() {
   Child::UnsetFocus();
   Redraw();
}

void HzSlider::SetVal(int NewVal) {
   OldHzVal=HzVal;
   if (NewVal!=HzVal) {
     HzVal=NewVal;
     if (HzVal<HzMinVal) HzVal=HzMinVal;
     if (HzVal>HzMaxVal) HzVal=HzMaxVal;
     if (OldHzVal!=HzVal) {
       YPosBar=(HzMaxVal!=HzMinVal)?((float)(xbtb-xbth-7-HautBar)*(float)(HzVal-HzMinVal))/(HzMaxVal-HzMinVal):0;
       OldHzVal=HzVal;
       if (Changed!=NULL) Changed(HzVal);
       Redraw();
     }
   }
}

int  HzSlider::GetVal() {
   return HzVal;
}

void HzSlider::SetMinMaxVal(int MinVal,int MaxVal) {
   OldHzVal=HzVal;
   if (MinVal==HzMinVal && MaxVal==HzMaxVal) return;
   HzMinVal=MinVal; HzMaxVal=MaxVal;
   MaxNbItem=xbtb-xbth-7;
   HautBar=(MaxNbItem)/(HzMaxVal-HzMinVal+1);
   HautBar=8;
   if (HzVal<HzMinVal) HzVal=HzMinVal;
   if (HzVal>HzMaxVal) HzVal=HzMaxVal;
   OldHzVal=HzVal;
   YPosBar=(HzMaxVal!=HzMinVal)?((float)(xbtb-xbth-7-HautBar)*(float)(HzVal-HzMinVal))/(HzMaxVal-HzMinVal):0;
   Redraw();
   if (HzVal!=OldHzVal)
     if (Changed!=NULL) Changed(HzVal);

}

void HzSlider::GetMinMaxVal(int *MinVal,int *MaxVal) {
   *MinVal=HzMinVal; *MaxVal=HzMaxVal;
}

HzSlider& HzSlider::operator=(int NewVal) {
   SetVal(NewVal);
   return *this;
}

//*************************** List ******************************************
//***************************************************************************

List::List () {
   FirstNode=CurNode=NextNode=Nodes=NULL; }

List::~List() {
   Node *N;
   Nodes=FirstNode;
   while (Nodes != NULL) {
      N = Nodes;
      Nodes = N->Next;
      delete (N->Item);
     }
}

Node * List::Add(Child *NewItem) {
   Node *N;
   N = new Node;
   if (!FirstNode) FirstNode = N;
   if (Nodes) Nodes->Next = N;
   N->Item = NewItem;
   N->Next = NULL;
   N->Prev = Nodes;
   Nodes = N;
   return N;
}

void List::Delete(Node *ChNode) {
   if (ChNode!=NULL) {
      if (ChNode==FirstNode) FirstNode=FirstNode->Next;
      if (ChNode==CurNode) CurNode=CurNode->Next;
      if (ChNode==NextNode) NextNode=CurNode->Next;
      if (ChNode==CurNode2) CurNode2=CurNode2->Next;
      if (ChNode==NextNode2) NextNode2=CurNode2->Next;
      if (ChNode==Nodes) Nodes=Nodes->Prev;
      if (ChNode->Next!=NULL) { ChNode->Next->Prev=ChNode->Prev; }
      if (ChNode->Prev!=NULL) { ChNode->Prev->Next=ChNode->Next; }
      delete ChNode;
     }
}

void List::SetFirstNode() {
   CurNode=FirstNode;
   if (CurNode) NextNode=CurNode->Next;
   else NextNode=NULL;
}

void List::GoNextNode() {
   if (CurNode) {
     CurNode=NextNode;
     if (CurNode) NextNode=CurNode->Next;
     else NextNode=NULL;
   }
}

void List::SetFirstNode2() {
   CurNode2=FirstNode;
   if (CurNode2) NextNode2=CurNode2->Next;
   else NextNode2=NULL;
}

void List::GoNextNode2() {
   if (CurNode2) {
     CurNode2=NextNode2;
     if (CurNode2) NextNode2=CurNode2->Next;
     else NextNode2=NULL;
   }
}

//*************************** ListWin ***************************************
//***************************************************************************

ListWin::ListWin () {
   FirstNode=CurNode=NextNode=Nodes=NULL; }

ListWin::~ListWin() {
   NodeWin *N;
   Nodes=FirstNode;
   while (Nodes != NULL) {
      N = Nodes;
      Nodes = N->Next;
      delete (N->Item);
     }
}

NodeWin * ListWin::Add(MainWin *NewItem) {
   NodeWin *N;
   N = new NodeWin;
   if (!FirstNode) FirstNode = N;
   if (Nodes) Nodes->Next = N;
   N->Item = NewItem;
   N->Next = NULL;
   N->Prev = Nodes;
   Nodes = N;
   return N;
}

void ListWin::Delete(NodeWin *ChNode) {
   if (ChNode!=NULL) {
      if (ChNode==FirstNode) FirstNode=FirstNode->Next;
      if (ChNode==CurNode) CurNode=CurNode->Next;
      if (ChNode==NextNode) NextNode=CurNode->Next;
      if (ChNode==Nodes) Nodes=Nodes->Prev;
      if (ChNode->Next!=NULL) { ChNode->Next->Prev=ChNode->Prev; }
      if (ChNode->Prev!=NULL) { ChNode->Prev->Next=ChNode->Next; }
      delete ChNode;
   }
}

void ListWin::SetFirstNode() {
   CurNode=FirstNode;
   if (CurNode) NextNode=CurNode->Next;
   else NextNode=NULL;
}

void ListWin::GoNextNode() {
   if (CurNode) {
     CurNode=NextNode;
     if (CurNode) NextNode=CurNode->Next;
     else NextNode=NULL;
   }
}

