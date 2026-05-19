#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <unistd.h>
#include <string.h>
#include <DUGL.h>
#include "INTRDUGL.H"
#include "DGUI.H"


//*** FlyMenu-Util ****
int MaxLargTextNodeMenu(NodeMenu *NM);
int HautMenu(NodeMenu *NM);

//*************************** ListFly ***************************************
//***************************************************************************
unsigned int FlyMsUp=0,FlyMsLftDwn=0,FlyMsRghDwn=0;

ListFly::ListFly () {
   FirstNode=CurNode=NextNode=Nodes=NULL; }

ListFly::~ListFly() {
   NodeFly *N;
   Nodes=FirstNode;
   while (Nodes != NULL) {
      N = Nodes;
      Nodes = N->Next;
      delete (N->Item);
     }
}

NodeFly * ListFly::Add(FlyObj *NewItem) {
   NodeFly *N;
   N = new NodeFly;
   if (!FirstNode) FirstNode = N;
   if (Nodes) Nodes->Next = N;
   N->Item = NewItem;
   N->Next = NULL;
   N->Prev = Nodes;
   Nodes = N;
   return N;
}

bool ListFly::Exist(FlyObj *SeekObj) {
   NodeFly *N=FirstNode,*N2=NULL;
   while (N != NULL) {
      N2 = N;
      N = N2->Next;
      if (N2->Item==SeekObj) return true;
   }
   return false;
}


void ListFly::Delete(NodeFly *ChNode) {
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

void ListFly::DeleteAfter(NodeFly *Node) {
   NodeFly *N=NULL,*N2=NULL;
   if (Node) N=Node->Next;
   while (N != NULL) {
      N2 = N;
      N = N2->Next;
      delete N2->Item;
   }
}

void ListFly::ClearAll() {
   NodeFly *N=FirstNode,*N2=NULL;
   while (N != NULL) {
      N2 = N;
      N = N2->Next;
      delete N2->Item;
   }
   FlyMsLftDwn=FlyMsRghDwn=0;
}

void ListFly::SetFirstNode() {
   CurNode=FirstNode;
   if (CurNode) NextNode=CurNode->Next;
   else NextNode=NULL;
}

void ListFly::GoNextNode() {
   if (CurNode) {
     CurNode=NextNode;
     if (CurNode) NextNode=CurNode->Next;
     else NextNode=NULL;
   }
}

int  ListFly::Empty() {
   return (FirstNode==NULL);
}

//*************************** FlyObj ****************************************
//***************************************************************************

FlyObj::FlyObj(int X,int Y,int Larg,int Haut,WinHandler *WH) {
   MyWinHand=WH;
   m_GraphCtxt=MyWinHand->m_GraphCtxt;
   MyList=WH->FlyList;
   MyNode=MyList->Add(this);
   X2=(X1=X)+(LargHz=Larg); Y2=(Y1=Y)+(HautVt=Haut);
   Destructed=NULL;
   if (WH->CurWinNode && WH->CurWinNode->Item->CurFocusNode)
     WH->CurWinNode->Item->CurFocusNode->Item->UnsetFocus();
}

FlyObj::~FlyObj() {
   if (Destructed) Destructed();
   MyList->Delete(MyNode);
   MyWinHand->Redraw(X1,Y1,X2,Y2);
}

void FlyObj::Draw() {}

void FlyObj::Redraw() { MyWinHand->Redraw(X1,Y1,X2,Y2); }

void FlyObj::Scan() {
   if (m_GraphCtxt->In(X1,Y1,X2,Y2,MsX,MsY)) {
     if ((!FlyMsLftDwn) && (MsButton&MS_LEFT_BUTT)) FlyMsLftDwn=1;
     if ((!FlyMsRghDwn) && (MsButton&MS_RIGHT_BUTT)) FlyMsRghDwn=1;
   }
}

void FlyObj::Move(int NewX,int NewY) {
   int OldX1=X1,OldY1=Y1;
   if (X1!=NewX || Y1!=NewY) {
     X2=(X1=NewX)+LargHz; Y2=(Y1=NewY)+HautVt;
     MyWinHand->Redraw(OldX1,OldY1,OldX1+LargHz,OldY1+HautVt);
     MyWinHand->Redraw(X1,Y1,X2,Y2);
   }
}

//************************** FlyMenu **************************************
//*************************************************************************

FlyMenu::FlyMenu(int x,int y,NodeMenu *NdMn,WinHandler *WH)
        :FlyObj(x,y,MaxLargTextNodeMenu(NdMn)+30,HautMenu(NdMn)+8,WH) {
   mcol=m_GraphCtxt->WinGris; NdMenu=NdMn;
   choix=-1; Redraw();
}

FlyMenu::~FlyMenu() { }

void FlyMenu::Draw() {
   int i,xd=X1+5,yd=Y2-FntHighPos-4;
   m_GraphCtxt->bar(X1,Y1,X2,Y2,mcol);
   m_GraphCtxt->CadWin(X1,Y1,X2,Y2);
   FntCol=m_GraphCtxt->WinNoir;
   for (i=0;i<NdMenu->NbNodeMenu;i++) {
     FntX=xd; FntY=yd-i*(FntHaut+1);
     if (!strlen(NdMenu->TbNodeMenu[i].Name))
       m_GraphCtxt->GHline(xd,X2-4,FntY+FntLowPos+FntHaut/2);
     else {
       if (choix==i) {
         m_GraphCtxt->bar(xd-1,FntY+FntLowPos-1,X2-3,FntY+FntHighPos,m_GraphCtxt->WinRougeF);
	 FntCol=m_GraphCtxt->WinBlanc;
         m_GraphCtxt->OutText(NdMenu->TbNodeMenu[i].Name);
         if (NdMenu->TbNodeMenu[i].NbNodeMenu &&
	     NdMenu->TbNodeMenu[i].TbNodeMenu)
           m_GraphCtxt->DrawFDroit(X2-10,FntY,m_GraphCtxt->WinBlanc);
       } else {
         if (NdMenu->TbNodeMenu[i].NbNodeMenu&& NdMenu->TbNodeMenu[i].TbNodeMenu)
           if (NdMenu->TbNodeMenu[i].Activ)
             m_GraphCtxt->DrawFDroit(X2-10,FntY,m_GraphCtxt->WinNoir);
	   else {
	     m_GraphCtxt->DrawFDroit(X2-10,FntY,m_GraphCtxt->WinGrisF);
             m_GraphCtxt->DrawFDroit(X2-9,FntY-1,m_GraphCtxt->WinGrisC);
	   }
         if (NdMenu->TbNodeMenu[i].Activ) {
           FntCol=m_GraphCtxt->WinNoir;
           m_GraphCtxt->OutText(NdMenu->TbNodeMenu[i].Name);
	 } else {
	   FntCol=m_GraphCtxt->WinGrisF;
           m_GraphCtxt->OutText(NdMenu->TbNodeMenu[i].Name);
	   FntX=xd+1; FntY--;
	   FntCol=m_GraphCtxt->WinGrisC;
           m_GraphCtxt->OutText(NdMenu->TbNodeMenu[i].Name);
	 }
       }
     }
   }
}

void FlyMenu::Scan() {
   int chx,l;
   FlyMenu *NFM;
   void (*Click)();

   if (!NdMenu->NbNodeMenu) return;
   FlyObj::Scan();
   if (m_GraphCtxt->In(X1,Y1+4,X2,Y2-4,MsX,MsY) ) {
     chx=(Y2-4-MsY)/(FntHaut+1);
     if (chx>NdMenu->NbNodeMenu-1) { choix=-1; Redraw(); }
     else
       if (chx!=choix && NdMenu->TbNodeMenu[chx].Activ &&
           strlen(NdMenu->TbNodeMenu[chx].Name)) {
         choix=chx;
         MyWinHand->DelAfterFlyObj(this);
	 if (NdMenu->TbNodeMenu[chx].NbNodeMenu &&
	     NdMenu->TbNodeMenu[chx].TbNodeMenu)
	   NFM=new FlyMenu(X2-3,Y2-5-(FntHaut+1)*chx-(HautMenu(&NdMenu->TbNodeMenu[chx])+8),
	                   &NdMenu->TbNodeMenu[chx],MyWinHand);
         Redraw();
       }
       else
         if (chx<=NdMenu->NbNodeMenu-1 &&
	     ((!NdMenu->TbNodeMenu[chx].Activ) ||
	      (!strlen(NdMenu->TbNodeMenu[chx].Name))) ) {
	   choix=-1; MyWinHand->DelAfterFlyObj(this); Redraw(); }
   }
   else
     if (choix!=-1) { choix=-1; Redraw(); }
   if (FlyMsLftDwn && choix!=-1 && NdMenu->TbNodeMenu[choix].NbNodeMenu==0 &&
       (!(MsButton&MS_LEFT_BUTT))) {
     FlyMsLftDwn=FlyMsRghDwn=0;
     MyWinHand->DelAllFly();
     Click=NdMenu->TbNodeMenu[choix].Click;
     m_GraphCtxt->WinFreeMs=1;
     if (Click) Click();
   }
   else {
     if (FlyMsLftDwn && (!(MsButton&MS_LEFT_BUTT))) FlyMsLftDwn=0;
     if (FlyMsRghDwn && (!(MsButton&MS_RIGHT_BUTT))) FlyMsRghDwn=0;
   }
}

//************************** FlyListBox **************************************
//*************************************************************************

FlyListBox::FlyListBox(int x,int y,int Larg,int Haut,WinHandler *WH,ListString *ls)
           :FlyObj(x,y,Larg,Haut,WH) {
   etatbth=etatbtb=bthdown=btbdown=bardown=pbardown=listdown=0;
   PosCursor=OldPosCursor=LastPosCursor=0; ValidateList=NULL;
   Terminate=Result=NULL;
   LastFirstInd=FirstInd=0; MaxNbItem=((Y2-Y1-5)/(FntHaut+1));
   LStr= new ListString;
   if (ls) {
     int nb=ls->NbElement();
     for (int i=0;i<nb;i++)
       LStr->Add((*ls)[i]->StrPtr);
   }
   xbth=X2-14; ybth=Y2-14;
   xbtb=X2-14; ybtb=Y1+2;
   YPosBar=0;
   if ((scrollact=(LStr->NbElement()>MaxNbItem && X2-X1>30 && Y2-Y1>64))) {
     HautBar=((ybth-ybtb-14)*MaxNbItem)/LStr->NbElement();
     if (HautBar<8) HautBar=8;
     YPosBar=((ybth-ybtb-14-HautBar)*FirstInd)/(LStr->NbElement()-MaxNbItem);
   }
   Redraw();
}

FlyListBox::~FlyListBox() {
   if (Terminate) *Terminate=1;
   if (Result) *Result=-1;
   delete LStr;
}

void FlyListBox::Draw() {
   int i,nb=LStr->NbElement();
   DgView V1,V2;
   m_GraphCtxt->bar(X1,Y1,X2,Y2,m_GraphCtxt->WinBlanc);
   m_GraphCtxt->rect(X1,Y1,X2,Y2,m_GraphCtxt->WinNoir);
   GetSurfView(&CurSurf,&V1);
   V1.MinX=X1+3; V1.MinY=Y1+3;
   V1.MaxX=X2-3; V1.MaxY=Y2-3;
   if (scrollact) GetSurfView(&CurSurf,&V2);
   SetSurfInView(&CurSurf,&V1);

   ViewClearText(&V1); FntY--;
   for (i=FirstInd;i<FirstInd+MaxNbItem;i++) {
     if (i>=nb) break;
     if (i==PosCursor) {
       m_GraphCtxt->bar(X1+3,Y2-FntHaut-4-(i-FirstInd)*(FntHaut+1),
           X2-3,Y2-3-(i-FirstInd)*(FntHaut+1),m_GraphCtxt->WinRougeF);
       m_GraphCtxt->rectmap(X1+3,Y2-FntHaut-4-(i-FirstInd)*(FntHaut+1),
	   X2-3,Y2-3-(i-FirstInd)*(FntHaut+1), m_GraphCtxt->WinBlanc,0xaaaaaaaa);
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
     V2.MinX=X1; V2.MinY=Y1;
     V2.MaxX=X2; V2.MaxY=Y2;
     SetSurfInView(&CurSurf,&V2);
     m_GraphCtxt->DrawButtFHaut(xbth,ybth,etatbth);
     m_GraphCtxt->bar(xbtb,ybtb+13,xbth+12,ybth-1,m_GraphCtxt->WinGrisC);
     m_GraphCtxt->DrawButtFBas(xbtb,ybtb,etatbtb);

     m_GraphCtxt->bar(xbth,ybth-1-YPosBar-HautBar,xbth+12,ybth-1-YPosBar,m_GraphCtxt->WinGris);
     m_GraphCtxt->CadWin(xbth,ybth-1-YPosBar-HautBar,xbth+12,ybth-1-YPosBar);
   }
}

void FlyListBox::Scan() {
   int XK1,YK1,XK2,YK2,redr=0;

   FlyObj::Scan();
   if (MyList->Nodes!=MyNode) {
     if (FlyMsLftDwn && (!(MsButton&MS_LEFT_BUTT))) FlyMsLftDwn=0;
     if (FlyMsRghDwn && (!(MsButton&MS_RIGHT_BUTT))) FlyMsRghDwn=0;
     return;
   }
   XK1=X1+3; YK1=Y1+3; YK2=Y2-3;
   if (scrollact) XK2=X2-15;
   else XK2=X2-3;
   if (FlyMsLftDwn&& (!bthdown)&& (!btbdown)&& (!bardown)&&
       (!listdown)&& (!pbardown)) {
     if (scrollact && m_GraphCtxt->In(xbtb,ybtb,xbtb+12,ybtb+12,MsX,MsY)) btbdown=1;
     else
       if (scrollact && m_GraphCtxt->In(xbth,ybth,xbth+12,ybth+12,MsX,MsY)) bthdown=1;
       else
         if (scrollact && m_GraphCtxt->In(xbth,ybth-YPosBar-HautBar-1,xbth+12,ybth-YPosBar-1,MsX,MsY)) {
	   bardown=1; msxdown=MsX; msydown=MsY; downYPosBar=YPosBar; }
         else
	   if (m_GraphCtxt->In(XK1,YK1,XK2,YK2,MsX,MsY)) listdown=1;
       if (listdown) {
         if (DgTimerFreq) oldbtime=((DgTime/(DgTimerFreq/8))&1);
       } else if (DgTimerFreq) oldbtime=((DgTime/(DgTimerFreq/16))&1);
       firstdown=1; redr=1;
     }
   if (LStr->NbElement()) {
     if (LStr->Changed()) {
       if ((scrollact=(LStr->NbElement()>MaxNbItem && X2-X1>30 && Y2-Y1>64))) {
         HautBar=((ybth-ybtb-14)*MaxNbItem)/LStr->NbElement();
	 if (HautBar<8) HautBar=8;
         YPosBar=((ybth-ybtb-14-HautBar)*FirstInd)/(LStr->NbElement()-MaxNbItem);
       }
       LastFirstInd=FirstInd; redr=1; }

     if (!m_GraphCtxt->In(XK1,YK1,XK2,YK2,MsX,MsY)) {
       if (listdown && (!(MsButton&MS_LEFT_BUTT))) {
         if (Terminate) *Terminate=1;
         if (Result) *Result=-1;
         Terminate=Result=NULL;
	 if (ValidateList) ValidateList(this,-1);
	 FlyMsLftDwn=FlyMsRghDwn=0; MyWinHand->DelAllFly();
	 return;
       }
       if (MyWinHand->Key) {
         OldPosCursor=PosCursor;
         switch(MyWinHand->Key) {
           case 0xc8:  // up
             if (PosCursor>0) PosCursor--;
	     break;
           case 0xd0:  // down
             if (PosCursor<(LStr->NbElement()-1)) PosCursor++;
	     break;
           case 0xc9:  // PG UP
             if (PosCursor>=MaxNbItem) PosCursor-=MaxNbItem;
	     else PosCursor=0;
	     break;
           case 0xd1:  // PG DOWN
             if (PosCursor<LStr->NbElement()-MaxNbItem-1 &&
	         LStr->NbElement()>MaxNbItem)
	       PosCursor+=MaxNbItem;
	     else PosCursor=LStr->NbElement()-1;
	     break;
           case 0xc7:  // DEB
             PosCursor=0;
	     break;
           case 0xcf:  // FIN
	     PosCursor=LStr->NbElement()-1;
         }
         if (AjFirstInd()|| OldPosCursor!=PosCursor) {
           redr=1; OldPosCursor=PosCursor;
           Redraw(); return;
         }
       }
     }
     else {
       if (listdown && (!(MsButton&MS_LEFT_BUTT))) {
         if (Terminate) *Terminate=1;
         if (Result) *Result=PosCursor;
         Terminate=Result=NULL;
	 if (ValidateList) ValidateList(this,PosCursor);
	 FlyMsLftDwn=FlyMsRghDwn=0; MyWinHand->DelAllFly();
	 return;
       }
       if (m_GraphCtxt->In(XK1,YK1,XK2,YK2,MsX,MsY) &&
           !(btbdown|| bthdown|| bardown|| pbardown|| listdown)) {
         /*if (MsY>=Y2-3) PosCursor=FirstInd;
         else*/
         if (MsY<=(Y2-3-((MaxNbItem-1)*(FntHaut+1))))
	   PosCursor=FirstInd+MaxNbItem-1;
	 else
	   PosCursor=FirstInd+(Y2-3-MsY)/(FntHaut+1);
       }
     }
     if (!FlyMsLftDwn) {
       btbdown=bthdown=bardown=pbardown=listdown=0; redr=1; }
     if (bardown) {
       if (YPosBar!=(downYPosBar+(msydown-MsY))) {
         YPosBar=(downYPosBar+(msydown-MsY));
         if (YPosBar>ybth-ybtb-14-HautBar) YPosBar=ybth-ybtb-14-HautBar;
         if (YPosBar<0) YPosBar=0;
         FirstInd=(YPosBar*(LStr->NbElement()-MaxNbItem))/(ybth-ybtb-14-HautBar);
         YPosBar=((ybth-ybtb-14-HautBar)*FirstInd)/(LStr->NbElement()-MaxNbItem);
         redr=1;
       }
     }
     if (listdown) {
       if ((MsY<=Y2-3 && MsY>(Y2-3-(MaxNbItem*(FntHaut+1))))|| (!DgTimerFreq)) {
         OldPosCursor=PosCursor;
	 if (Y2-3>=MsY)
           PosCursor=FirstInd+(Y2-3-MsY)/(FntHaut+1);
	 else PosCursor=FirstInd-(MsY-Y2+3)/(FntHaut+1)-1;
	 if (PosCursor<0) PosCursor=0;
	 if (PosCursor>=LStr->NbElement()) PosCursor=LStr->NbElement()-1;
	 if (AjFirstInd()|| OldPosCursor!=PosCursor) redr=1;
       }
       else
         if ((DgTimerFreq && (oldbtime^((DgTime/(DgTimerFreq/8))&1))) || firstdown) {
           OldPosCursor=PosCursor; firstdown=0;
           if (DgTimerFreq) oldbtime=((DgTime/(DgTimerFreq/8))&1);
	     if (Y2-3>=MsY)
	       PosCursor=FirstInd+(Y2-3-MsY)/(FntHaut+1);
	     else PosCursor=FirstInd-((MsY-Y2+3)/(FntHaut+1))-1;
	     if (PosCursor<0) PosCursor=0;
	     if (PosCursor>=LStr->NbElement()) PosCursor=LStr->NbElement()-1;
	     if (AjFirstInd()|| OldPosCursor!=PosCursor) redr=1;
         }
       }
     if (scrollact) {
       if (btbdown && m_GraphCtxt->In(xbtb,ybtb,xbtb+12,ybtb+12,MsX,MsY)) {
         if (DgTimerFreq) {
           if ((FirstInd+MaxNbItem)<LStr->NbElement() &&
             ((oldbtime^((DgTime/(DgTimerFreq/16))&1)) || firstdown)) {
	     FirstInd++; etatbtb=1; redr=1;
	     if (!firstdown) oldbtime=((DgTime/(DgTimerFreq/16))&1);
	         firstdown=0;
	   }
           else
	     if (!etatbtb) { etatbtb=1; redr=1; }
         }
	 else {
           if ((FirstInd+MaxNbItem)<LStr->NbElement()) {
	     FirstInd++; etatbtb=1; redr=1; }
           else if (!etatbtb) { etatbtb=1; redr=1; }
         }
       } else if (etatbtb) { etatbtb=0; redr=1; }

       if (bthdown && m_GraphCtxt->In(xbth,ybth,xbth+12,ybth+12,MsX,MsY)) {
         if (DgTimerFreq) {
           if (FirstInd && ((oldbtime^((DgTime/(DgTimerFreq/16))&1)) || firstdown)) {
	     FirstInd--; etatbth=1; redr=1;
	     if (!firstdown) oldbtime=((DgTime/(DgTimerFreq/16))&1);
	     firstdown=0;
	   }
           else
	     if (!etatbth) { etatbth=1; redr=1; }
         } else {
           if (FirstInd) { FirstInd--; etatbth=1; redr=1; }
           else if (!etatbth) { etatbth=1; redr=1; }
         }
       } else if (etatbth) { etatbth=0; redr=1; }
     }
     if (LastFirstInd!=FirstInd && scrollact) {
       HautBar=((ybth-ybtb-14)*MaxNbItem)/LStr->NbElement();
       if (HautBar<8) HautBar=8;
       YPosBar=((ybth-ybtb-14-HautBar)*FirstInd)/(LStr->NbElement()-MaxNbItem);
       LastFirstInd=FirstInd; redr=1;
     }
     if (LastPosCursor!=PosCursor) {
       LastPosCursor=PosCursor; redr=1;
     }
     if (MyWinHand->Key==0x1c || MyWinHand->Key==0x9c) {
       if (Terminate) *Terminate=1;
       if (Result) *Result=PosCursor;
       Terminate=Result=NULL;
       if (ValidateList) ValidateList(this,PosCursor);
       FlyMsLftDwn=FlyMsRghDwn=0; MyWinHand->DelAllFly();
       return;
     }
     if (redr) Redraw();
   }
   else
     if (listdown && (!(MsButton&MS_LEFT_BUTT))) {
       if (Terminate) *Terminate=1;
       if (Result) *Result=-1;
       Terminate=Result=NULL;
       if (ValidateList) ValidateList(this,-1);
       FlyMsLftDwn=FlyMsRghDwn=0; MyWinHand->DelAllFly();
       return;
     }
   if (FlyMsLftDwn && (!(MsButton&MS_LEFT_BUTT))) FlyMsLftDwn=0;
   if (FlyMsRghDwn && (!(MsButton&MS_RIGHT_BUTT))) FlyMsRghDwn=0;
}

int  FlyListBox::AjFirstInd() {
   if (PosCursor>=(FirstInd+MaxNbItem))
    { FirstInd=PosCursor-MaxNbItem+1; return 1; }
   else if (PosCursor<FirstInd) { FirstInd=PosCursor; return 1; }
   return 0;
}

//********************** class Menu **************************************
//************************************************************************

Menu::Menu(MainWin *Father,NodeMenu *NdMn)
     :Child(Father->VActiv.MinX,Father->VActiv.MaxY-(FntHaut+6),
            Father->VActiv.MaxX,Father->VActiv.MaxY,Father,NULL)  {
   MyWin=Father; NdMenu=NdMn;
   MyWin->MyMenu=this; choix=-1;
   Redraw();
}

Menu::~Menu() {}

void Menu::Draw() {
   int i,SX;
   m_GraphCtxt->bar(XC1,YC1,XC2,YC2,m_GraphCtxt->WinGris);
   if (NdMenu->NbNodeMenu && NdMenu->TbNodeMenu) {
     FntX=XC1+6; FntY=YC2-FntHighPos-3;
     for (i=0;i<NdMenu->NbNodeMenu;i++) {
       if (NdMenu->TbNodeMenu[i].Activ) {
         if (i==choix) {
	   m_GraphCtxt->bar(FntX-6,FntY+FntLowPos-1,FntX+WidthText(NdMenu->TbNodeMenu[i].Name)+6,FntY+FntHighPos+1,m_GraphCtxt->WinRougeF);
	   FntCol=m_GraphCtxt->WinBlanc;
           m_GraphCtxt->OutText(NdMenu->TbNodeMenu[i].Name); FntX+=12;
	 }
	 else {
           FntCol=m_GraphCtxt->WinNoir;
           m_GraphCtxt->OutText(NdMenu->TbNodeMenu[i].Name); FntX+=12;
	 }
       }
       else {
         SX=FntX;
	 FntCol=m_GraphCtxt->WinGrisF;
         m_GraphCtxt->OutText(NdMenu->TbNodeMenu[i].Name);
	 FntX=SX+1; FntY--;
	 FntCol=m_GraphCtxt->WinGrisC;
         m_GraphCtxt->OutText(NdMenu->TbNodeMenu[i].Name);
	 FntY++; FntX+=11;
       }
     }
   }
   m_GraphCtxt->line(XC1,YC1+1,XC2,YC1+1,m_GraphCtxt->WinGrisF);
   m_GraphCtxt->line(XC1,YC1,XC2,YC1,m_GraphCtxt->WinNoir);
}

void Menu::Scan() {
   FlyMenu *FM;
   int XN=Main->X+MyVActiv->OrgX,YN=Main->Y+MyVActiv->OrgY;
   int RMsX=MsX-XN,RMsY=MsY-YN,i,oldx,newx,chx;
   if (!MyWin->ActivMenu) {
     if (m_GraphCtxt->In(XC1,YC2-FntHaut-4,XC2,YC2-2,RMsX,RMsY) &&
         m_GraphCtxt->WinFreeMs && (MsButton&MS_LEFT_BUTT)) {
       if (NdMenu->NbNodeMenu && NdMenu->TbNodeMenu) {
         choix=-1; oldx=XC1;
     	 for (i=0;i<NdMenu->NbNodeMenu;i++) {
	   newx=oldx+WidthText(NdMenu->TbNodeMenu[i].Name)+13;
           if (NdMenu->TbNodeMenu[i].Activ) {
	     if (RMsX>=oldx && RMsX<newx) choix=i;
	   }
	   if (choix!=-1) break;
	   oldx=newx;
	 }
	 if (choix!=-1) {
	   m_GraphCtxt->WinFreeMs=0;
	   if (MyWin->CurFocusNode) MyWin->CurFocusNode->Item->UnsetFocus();
	   MyWin->ActivMenu=1;
	   FM=new FlyMenu(oldx+XN,YC1+YN+2-8-HautMenu(&NdMenu->TbNodeMenu[choix]),
	                  &NdMenu->TbNodeMenu[choix],MyWin->MWWinHand);
	   Redraw();
	 }
       }
     }
   }
   if (MyWin->ActivMenu) {
     if ((!m_GraphCtxt->WinFreeMs) && (!(MsButton&MS_LEFT_BUTT)))
       m_GraphCtxt->WinFreeMs=1;
     if (m_GraphCtxt->In(XC1,YC2-FntHaut-4,XC2,YC2-2,RMsX,RMsY)) {
       chx=-1;
       if (NdMenu->NbNodeMenu && NdMenu->TbNodeMenu) {
         oldx=XC1;
     	 for (i=0;i<NdMenu->NbNodeMenu;i++) {
	   newx=oldx+WidthText(NdMenu->TbNodeMenu[i].Name)+13;
           if (NdMenu->TbNodeMenu[i].Activ) {
	     if (RMsX>=oldx && RMsX<=newx) chx=i;
	   }
	   if (chx!=-1) break;
	   oldx=newx;
	 }
       }
       if (chx!=-1 && chx!=choix) {
         choix=chx;
	 Redraw();
	 MyWin->MWWinHand->DelAllFly();
         FM=new FlyMenu(oldx+XN,YC1+YN+2-8-HautMenu(&NdMenu->TbNodeMenu[choix]),
                        &NdMenu->TbNodeMenu[choix],MyWin->MWWinHand);
       }
     }
   }
}

void Menu::SetFocus() {
   Node *CurNode=Main->CurFocusNode;
   Child::SetFocus();
   Main->SetNextFocus();
   if (Main->CurFocusNode==ChNode) UnsetFocus();
}

void Menu::UnsetFocus() {
   Child::UnsetFocus();
}

//*************** class ComBox *******************************************
//************************************************************************

ComBox::ComBox(int x,int y,int Larg,int Haut,MainWin *Father,
               ListString *ls,int defsel)
       :Child(x,y,x+Larg,y+Haut,Father,NULL)  {
   MyFLB=NULL;
   EnableFLB=1;  LoadList=NULL; Changed=NULL;
   ActFLB=TermFLB=0; ResFLB=-1;
   LStr= new ListString;
   OldString = new String();
   if (ls) {
     int nb=ls->NbElement();
     for (int i=0;i<nb;i++)
       LStr->Add((*ls)[i]->StrPtr);
   }
   if (defsel>=0 && defsel<LStr->NbElement()) {
     Select=OldSelect=defsel;
     *OldString=*((*LStr)[Select]);
   }
   else {
     Select=OldSelect=-1;
     *OldString="";
   }
   Redraw();
}

ComBox::~ComBox() { delete OldString; delete LStr; }

void ComBox::Draw() {
   DgView V1,VS;
   m_GraphCtxt->bar(XC1,YC1,XC2,YC2,m_GraphCtxt->WinBlanc);
   m_GraphCtxt->CadBoxWin(XC1,YC1,XC2,YC2);

   GetSurfView(&CurSurf,&VS);
   V1.OrgX=XC1+3+CurSurf.OrgX; V1.OrgY=YC1+3+CurSurf.OrgY;
   V1.MinX=V1.MinY=0;
   V1.MaxX=XC2-XC1-6; V1.MaxY=YC2-YC1-6;
   SetSurfInView(&CurSurf,&V1);
   if (Focus) {
     m_GraphCtxt->ClearSurf(m_GraphCtxt->WinRougeF);
     m_GraphCtxt->rectmap(CurSurf.MinX,CurSurf.MinY,CurSurf.MaxX,CurSurf.MaxY,m_GraphCtxt->WinBlanc,0xaaaaaaaa);
     FntCol=m_GraphCtxt->WinBlanc;
   }
   else FntCol=m_GraphCtxt->WinNoir;
   FntY=ViewGetFntYMID(&V1);
   if (Select!=-1) m_GraphCtxt->ViewOutTextMode(&V1,((*LStr)[Select])->StrPtr,AJ_SRC);
   SetSurfView(&CurSurf,&VS);

   m_GraphCtxt->bar(XC2-14,YC1+2,XC2-2,YC2-2,m_GraphCtxt->WinGris);
   m_GraphCtxt->DrawFBas(XC2-10,YC2-10,m_GraphCtxt->WinNoir);
   m_GraphCtxt->CadWin(XC2-14,YC1+2,XC2-2,YC2-2);
}

void ComBox::Scan() {
   Child::Scan();
   if (MsDown && (!Focus))
     SetFocus();
   if (Focus && (!EnableFLB)) {
     if (!Main->MWWinHand->FlyList->Exist(MyFLB))
       EnableFLB=1;
   }
   if (LostMsDown && MsIn && EnableFLB) {
     ActiveFLB();
     return;
   }
   if (TermFLB) { ActFLB=TermFLB=0; EnableFLB=1; Select=ResFLB; }
   //if (ActFLB && (!MsDown)) { ActFLB=0; EnableFLB=1; }
   if ((Select!=-1 && (Select<0 || Select>=LStr->NbElement())) ||
       ((!LStr->NbElement()) && Select!=-1)) {
     Select=OldSelect=-1; *OldString=""; Redraw();
     if (Changed) Changed(NULL,Select); return;
   }
   else
     if (LStr->Changed() || (OldSelect!=Select) ||
         (Select!=-1&& strcmp(((*LStr)[Select])->StrPtr,OldString->StrPtr) ) ) {
       if (Select!=-1) *OldString=*((*LStr)[Select]);
       else *OldString="";
       OldSelect=Select;
       Redraw();
       if (Changed) {
         if (Select!=-1) Changed(((*LStr)[Select]),Select);
	 else Changed(NULL,Select);
       }
       return;
     }
   if (OldSelect!=Select) {
     if (Select>=0 && Select<LStr->NbElement()) {
       *OldString=*((*LStr)[Select]);
       OldSelect=Select; Redraw();
     }
     else {
       OldSelect=Select=-1; *OldString="";
       Redraw();
     }
   }

   //if (LostMsDown && (!EnableFLB)) EnableFLB=1;
   //if ((!MsDown) && (MsButton&MS_LEFT_BUTT) && (!EnableFLB))
   //  EnableFLB=1;
   //if (Focus && (!MsDown) && (!EnableFLB)) EnableFLB=1;
   if (Focus && (EnableFLB)) {
     switch(Main->Key) {
       case 0x50:  // bas key-pad
	 if (((!(Main->KeyFLAG&KB_NUM_ACT)) && (!(Main->KeyFLAG&KB_SHIFT_PR))) ||
	     (((Main->KeyFLAG&KB_NUM_ACT) && (Main->KeyFLAG&KB_SHIFT_PR))) ) {
	   ActiveFLB();
	 }
	 break;
       case 0xd0: // ext-bas
	 ActiveFLB();
     }
   }
}
void ComBox::SetFocus() {  Child::SetFocus(); Redraw();  }
void ComBox::UnsetFocus() {
  //DesactiveFLB();
  Child::UnsetFocus();
  Redraw();
}
int  ComBox::Chgd() {
   if ((Select<0 && Select>=LStr->NbElement()) ||
       ((!LStr->NbElement()) && Select!=-1)) {
     Select=OldSelect=-1; *OldString="";
     Redraw();
     return 1;
   }
   if ((OldSelect!=Select) ||
       (!strcmp(((*LStr)[Select])->StrPtr,OldString->StrPtr)) ) {
       *OldString=*((*LStr)[Select]);
       OldSelect=Select;
       Redraw();
       return 1;
   }
   else return 0;
}

void ComBox::ActiveFLB() {
    if (!EnableFLB) return;
    int px=Main->X+MyVActiv->OrgX,py=Main->Y+MyVActiv->OrgY,haut=(FntHaut+1)*5+6;
    if (LoadList) LoadList(LStr);
    MyFLB= new
      FlyListBox(XC1+px,YC1-1-haut+py,XC2-XC1,haut, Main->MWWinHand,LStr);
    ActFLB=1; TermFLB=0;
    MyFLB->Terminate=&TermFLB;
    MyFLB->Result=&ResFLB;
    EnableFLB=0; SetFocus();
}

void ComBox::DesactiveFLB() {
    if (MyFLB!=NULL) {
      delete MyFLB;
      MyFLB=NULL;
    }
    ActFLB=0;
    EnableFLB=1;
}


//*** FlyMenu-Util ****
int MaxLargTextNodeMenu(NodeMenu *NM) {
   int MaxLarg=0,i,l;
   if (NM)
     for (i=0;i<NM->NbNodeMenu;i++)
       if ((l=WidthText(NM->TbNodeMenu[i].Name))>MaxLarg) MaxLarg=l;
   return MaxLarg;
}

int HautMenu(NodeMenu *NM) {
   int Haut=0;
   if (NM) Haut=NM->NbNodeMenu*(FntHaut+1);
   return Haut;
}

