#ifndef DGUI_H

#define DGUI_H

//#include <DUGL/DUGL.H>

#ifdef __cplusplus


//**************************************************************************
//********************** class STRING  *************************************
//**************************************************************************
class ListString;

class String {
public:
   char Chgd,resv[3];
   char *ExtChgd;
   char *StrPtr;
public:
   String(char *s,int MaxSize);
   String(char *s);
   String(int MaxSize);
   String(const String &S);
   String();
   ~String();

   String(String &S);   // constructeur de copie

   int		 Changed();
   int           Length();

   int 		 GetInt();
   unsigned int  GetUInt();
   long	long int GetLLongInt();
   unsigned long long int GetULLongInt();
   double	 GetDouble();

   unsigned int  AddCursChar(unsigned int PosCurs,char c,int Ins);
   unsigned int  DelCurs(unsigned int PosCurs);
   unsigned int  BackDelCurs(unsigned int PosCurs);

   friend String operator+(String s1, String s2);
   friend String operator+(String s1, double d);
   friend String operator+(double s, String s1);
   friend String operator+(String s1, int i);
   friend String operator+(int i, String s1);
   friend String operator+(String s1, unsigned int u);
   friend String operator+(unsigned int u, String s1);
   friend String operator+(String s1, long long int i);
   friend String operator+(long long int i, String s1);
   friend String operator+(String s1, unsigned long long int u);
   friend String operator+(unsigned long long int u, String s1);
   friend String operator+(String s1, char c);
   friend String operator+(char c, String s1);
   friend String operator+(String s1,char* s);
   friend String operator+(char* s, String s1);

   String& operator+=(String s1);
   String& operator+=(char * s);
   String& operator+=(double d);
   String& operator+=(int i);
   String& operator+=(unsigned int u);
   String& operator+=(long long int D);
   String& operator+=(unsigned long long int U);
   String& operator+=(char c);

   String& operator=(String s1);
   String& operator=(char *s);
   String& operator=(double d);
   String& operator=(int i);
   String& operator=(unsigned int u);
   String& operator=(long long int D);
   String& operator=(unsigned long long int U);
   String& operator=(char c);

   bool    operator > (String &S);
   bool    operator < (String &S);
   bool    operator <= (String &S);
   bool    operator >= (String &S);
   bool    operator != (String &S);
   bool    operator == (String &S);

   bool    operator > (char *S);
   bool    operator < (char *S);
   bool    operator <= (char *S);
   bool    operator >= (char *S);
   bool    operator != (char *S);
   bool    operator == (char *S);

   int FindChar(char c, int pos=0);
   String* SubString(int Pos, int Length);
   String* SubString(int Pos, char COpen, char CClose);
   ListString* Split(char *StrDelim);
   ListString* Split(char delim);
   ListString* SplitPos(int Pos,char delim);
   ListString* SplitPos(int Pos,char *StrDelim);
   void DelSpacesTabs();
   void Del13_10();
};

//**************** class ListString *******************************
//*****************************************************************
/*struct NodeString {
   String *Item;
   NodeString  *Next,*Prev;
   NodeString  *Next20,*Prev20;
};*/

struct NodeString {
   String **Items; // table of pointers to String items
   int  NbItems, // number of the available items
        NbFreeItems, // number of non used pointers (resv or not)
        NbResvItems, // number of cases used as reserve
                     // (free or non) usually reserved for ins
        NbAllItems; // size of the pointer table Items (NbItems+NbFreeItems)
   NodeString *Next,
              *Prev;
};

#define LISTSTR_FIRST 0
#define LISTSTR_LAST  -1

class ListString {
public:
   ListString();
   ListString(int NbStr,char *Str[]);
   ~ListString();
   ListString(ListString &S);   // constructeur de copie

   int    Add(String *Str);
   int    Add(char *str);
   int    Insert(int index,String *Str);
   int    Insert(int index,char *str);
   int    Delete(String *Str, int POS=0);
   int    Delete(char *str, int POS=0);
   int    Delete(int index);
   int    Index(String *Str, int POS=0);
   int    Index(char *str, int POS=0);
   void   Sort(bool bAsc = true);

   void   ClearListStr();
   int	  NbElement();
   int    Changed();

   void   Swap(int ind1, int ind2);
   String* Element(int ind);
   String*  operator[](int ind);
//   String&  operator[](int ind);

private:
   NodeString *Nodes,*FirstNode,*LastNode;
   NodeString *CurNode,*NextNode;
   int CurNodeFirstIdx;
   int NbElt;
   char Chgd,resv[3];
   NodeString* NewNodeString();
   void AddNode(NodeString* newNode);
   void DeleteCurNode();
};

//**************************************************************************
//**************************** GUI ( Graphic User Interface ) **************
//**************************************************************************
struct Node;
class List;
struct NodeWin;
class ListWin;
struct NodeFly;
class ListFly;
class FlyObj;
class Menu;
class Child;
class ContBox;
class MainWin;
class WinHandler;

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned int FlyMsUp,FlyMsLftDwn,FlyMsRghDwn;

#ifdef __cplusplus
	   }
#endif

#define DEFAULT_MAX_MSGBOX 8

void MessageBox(WinHandler *MsgBoxWH,char *NmWin, char *Msg,
     char *NmOk, void (*clickOk)(), char *NmNo, void (*clickNo)(),
     char *NmCancel, void (*clickCancel)());

void FilesBox(WinHandler *FilesBoxWH,char *NmWin,
     char *NmOk, void (*clickOk)(String *S,int typesel),
     char *NmCancel, void (*clickCancel)(),
     ListString *NomTypes,ListString *MaskTypes,int deftype=-1, bool SortFilesDirs = true, bool sortAsc = true);


// -------------------------------
// graphic context class ---------
// -------------------------------

class CGraphContext {
public:
   CGraphContext(int BitsPixel,void *PalBGR1024=NULL);
   // GUI high-level graphic functions
   void CadWin(int x1,int y1,int x2,int y2);
   void CadAppWin(int x1,int y1,int x2,int y2);
   void CadBoxWin(int x1,int y1,int x2,int y2);
   void DrawCoche(int x,int y);
   void DrawFHaut(int x,int y,int fcol);
   void DrawFBas(int x,int y,int fcol);
   void DrawFDroit(int x,int y,int fcol);
   void DrawFGauche(int x,int y,int fcol);
   void DrawButtFHaut(int x,int y,int etat);
   void DrawButtFBas(int x,int y,int etat);
   void DrawButtFDroit(int x,int y,int etat);
   void DrawButtFGauche(int x,int y,int etat);
   int In(int x1,int y1,int x2,int y2, int PosX, int PosY);
   void GHline(int x1, int x2, int y1);
   void GVline(int x, int y1, int y2);
   // low level graphics functions
   void (*Clear)(int clrcol);
   void (*ClearSurf)(int clrcol);
   void (*bar)(int X1,int Y1,int X2,int Y2,int bcol);
   void (*line)(int X1,int Y1,int X2,int Y2,int LgCol);
   void (*cputpixel)(int X,int Y,int Col);
   void (*rectmap)(int X1,int Y1,int X2,int Y2,int rcol,unsigned int rmap);
   void (*rect)(int X1,int Y1,int X2,int Y2,int rcol);
   void (*PutSurf)(DgSurf *S,int X,int Y,int PType);
   void (*PutMaskSurf)(DgSurf *S,int X,int Y,int PType);
   void (*Line)(void *Point1,void *Point2,int col);
   void (*Poly)(void *ListPt, DgSurf *SS, unsigned int TypePoly, int ColPoly);
   // low-level font
   void (*OutText)(const char *str);
   int  (*ViewOutTextMode)(DgView *V,const char *str,int Mode);
   // data
   unsigned int WinNoir,WinBlanc,WinBleuF,WinGris,WinGrisF,WinGrisC,WinRougeF;
   int WinStopScan;
   int WinFreeMs;
};


//*********** class WinHandler *********************************
class WinHandler {
   friend class MainWin;
public:
   DgSurf *S;
   CGraphContext *m_GraphCtxt;
   ListWin *WinList;
   ListFly *FlyList;
   NodeWin *CurWinNode;
   Node *SaveFocusCurWin;
   int BackCol;
   unsigned int AsciiFLAG,KeyFLAG;
   unsigned char Ascii,Key;
private:
   int DMsX,DMsY,MoveCurWin;
public:
   WinHandler(int resh,int resv,char BitsPixel,int backCol,void *PalBGR1024=NULL);
   ~WinHandler();
   void Scan();
   void Draw();
   void Redraw(int x1,int y1,int x2,int y2);
   void DrawSurf(DgSurf *DestSurf);
   void DelAfterFlyObj(FlyObj *Fly);
   void DelAllFly();
protected:
   NodeWin * AddMainWin(MainWin *MainW);
};

//*********** class MainWin ************************************

class MainWin {
   friend class List;
   friend class Child;
   friend class WinHandler;
public:
   String Label;
   Menu *MyMenu;
   WinHandler *MWWinHand;
   CGraphContext *m_GraphCtxt;
   Node *CurFocusNode,*DefNode,*EchapNode;
   int  X,Y,X2,Y2;
   int LargHz,HautVt;
   DgSurf *S;
   DgView VActiv;
   unsigned int AsciiFLAG,KeyFLAG;
   unsigned char Ascii,Key;
   char DefaultActiv,Chgd,Visible,Focus,ValidDef,ActivMenu;
protected:
   List *ChList;
   NodeWin *MWNode;
public:
   MainWin(int XWin,int YWin,int Larg,int Haut,char *label,WinHandler *WH);
   ~MainWin();
   void Draw();
   void Redraw(int x1,int y1,int x2,int y2);
   void Redraw();
   void DrawSurf(DgSurf *DestSurf);
   void Scan();
   void SetNextFocus();
   void Enable();
   void Disable();
   void Show();
   void Hide();
   void Move(int NouvX,int NouvY);
protected:
   Node * AddChild(Child *child);
};

//*************** class Child ************************************
class Child {
public:
   MainWin *Main;
   CGraphContext *m_GraphCtxt;
   ContBox *MainCBox;
   Node    *ChNode,**CurFocusNode;
   List	   *MyChList;
   int     XC1,YC1,XC2,YC2;
   int     MsXDown, MsYDown;
   int     LastMsXDrag, LastMsYDrag;
   char    Focus,
           MsDown;
   char    Default,CanDefault,MsIn,Visible,Type,DblClick,LostFocus,LostMsDown;
// events: all x,y coordinates are relative to Child View(widget)
   void (*OnMsDown)(int x, int y);
   void (*OnMsUp)(int x, int y, bool UpInside);
   void (*OnMsIn)(int x, int y);
   void (*OnMsOut)(int x, int y);
   void (*OnMsDrag)(int x, int y, bool UpInside);
   void (*OnMsClick)();
   void (*OnMsDoubleClick)();

protected:
   unsigned int LastMsDownTime;
   char    OldFocus,MsDownD,oldMsIn,resv;
   DgView	  *MyVActiv;
public:
   Child(int x1,int y1,int x2,int y2,MainWin *Father,ContBox *CBox);
   ~Child();
   virtual void SetFocus();
   virtual void UnsetFocus();
   virtual void Draw();
   virtual void DrawSurf(DgSurf *DestSurf);
   virtual void Redraw();
   virtual void Scan();
   virtual void Validate();
   virtual void Show();
   virtual void Hide();
   virtual int  GetWidth();
   virtual int  GetHeight();
};

//*********** class ContBox ************************************

class ContBox : public Child {
   friend class Child;
public:
   String Label;
   Node *CurFocusInNode;
   DgView VActiv;
protected:
   List *SsChList;
public:
   ContBox(int x1,int y1,int x2,int y2,MainWin *Father,char *label);
   ~ContBox();
   void Draw();
   void Redraw(int x1,int y1,int x2,int y2);
   void Redraw();
   void DrawSurf(DgSurf *DestSurf);
   void Scan();
   void SetFocus();
   void UnsetFocus();
   void SetNextFocus();
   void SetPrevFocus();
   void ChgOrg(int Orgx,int Orgy);
   void GetOrg(int *Orgx,int *Orgy);
protected:
   Node * AddChild(Child *child);
};

//*************** class Button ************************************
class Button : public Child {
public:
   String Label;
   char Etat;
   void (*Click)();
public:
   Button(int x1,int y1,int x2,int y2,MainWin *Father,char *label,
          char def,char Echap);
   ~Button();
   void Draw();
   void Scan();
   void SetFocus();
   void UnsetFocus();
   void Validate();
};

//*************** class ImgButton ************************************
#define DGUI_IMGBUTT_CENTER   1
#define DGUI_IMGBUTT_FIT      2
#define DGUI_IMGBUTT_UNMASK   4
#define DGUI_IMGBUTT_MASK     8

class ImgButton : public Child {
public:
   DgSurf *m_Img;
   void (*Click)();
   int ImgBlit;
   char Etat;
   bool DrawFocus;
public:
   ImgButton(int x1,int y1,int x2,int y2,MainWin *Father,DgSurf *Img,
          int btype=DGUI_IMGBUTT_CENTER|DGUI_IMGBUTT_MASK,
          char def=0,char Echap=0);
   ~ImgButton();
   void Draw();
   void Scan();
   bool SetImage(DgSurf *Img);
   void SetFocus();
   void UnsetFocus();
   void Validate();
};

//**************** class Label ************************************
class Label : public Child {
public:
   String Text;
   int Text_AJ;
public:
   Label(int x1,int y1,int x2,int y2,MainWin *Father,char *text,int text_aj);
   ~Label();
   void Draw();
   void Scan();
   void SetFocus();
   void UnsetFocus();
};

//**************** class TextBox **********************************
class TextBox : public Child {
   unsigned int PosCursor,OldIns,OldPosCursor;
   int TextX;
   char Insert,password,resv[2];
public:
   char TextInBox,charpw,resv2[2];
   unsigned int MaxLargText;
public:
   String Text;
   void (*AsciiPressed)(unsigned char *Ascii,unsigned int TAsciiFLAG);
   void (*KeyPressed)(unsigned char *Key,unsigned int TKeyFLAG);
   void (*ValidateText)(String *ValText);
   void (*FocusLost)(String *ValText);
public:
   TextBox(int x1,int y1,int x2,int y2,MainWin *Father,char *deftext=NULL,
           char textInBox=1);
   ~TextBox();
   void Draw();
   void DrawSurf(DgSurf *DestSurf);
   void Scan();
   void SetFocus();
   void UnsetFocus();
   void Validate();
   void SetPassWord(char pw);
   char GetPassWord();
protected:
   void TextCatInBox();
   void CalcTextX();
};

//**************** class GraphBox **********************************
class GraphBox : public Child {
public:
   unsigned int ContCol;
   DgView VGraphBox;
   char OldMsIn,resv[3];
   int  MouseX,MouseY;
   bool DrawFocus;
public:
   void (*ScanGraphBox)(GraphBox *MyGrBox);
   void (*GraphBoxMouseIn)(int MouseX,int MouseY,unsigned int MouseButton);
   void (*GraphBoxMouseOut)(int MouseX,int MouseY,unsigned int MouseButton);
   void (*GraphBoxDraw)(GraphBox *MyGrBox);
   void (*AsciiPressed)(unsigned char *Ascii,unsigned int TAsciiFLAG);
   void (*KeyPressed)(unsigned char *Key,unsigned int TKeyFLAG);
public:
   GraphBox(int x1,int y1,int x2,int y2,MainWin *Father,int GbCCol);
   ~GraphBox();
   void Draw();
   void Scan();
   void SetFocus();
   void UnsetFocus();
   void ChgOrg(int Orgx,int Orgy);
   void GetOrg(int *Orgx,int *Orgy);
   int  GetWidth();
   int  GetHeight();

protected:
   void SetView();
};

//**************** class CocheBox *********************************
class CocheBox : public Child {
   char Etat,resv[3];
public:
   String Label;
   char True;
   void (*Changed)(char vtrue);
public:
   CocheBox(int x1,int y1,int x2,int y2,MainWin *Fath,ContBox *Cont,char *label,char vtrue);
   ~CocheBox();
   void Draw();
   void Scan();
   void SetFocus();
   void UnsetFocus();
   void SetTrue(char ctrue);
   CocheBox& operator=(char ctrue);
};

//**************** class RadioButt ********************************
#define ID_OptionButt 	1

class OptionButt : public Child {
   char Etat,resv[3];
public:
   String Label;
   char True;
   void (*Changed)(char vtrue);
public:
   OptionButt(int x1,int y1,int x2,int y2,MainWin *Fath,ContBox *Cont,char *label,char vtrue);
   ~OptionButt();
   void Draw();
   void Scan();
   void SetFocus();
   void UnsetFocus();
   void SetTrue(char ctrue);
   OptionButt& operator=(char ctrue);
protected:
   void SetFriendFalse();
};

//**************** class ListBox***********************************
class ListBox : public Child {
   int OldPosCursor,LastPosCursor,FirstInd,LastFirstInd,YPosBar,HautBar;
   unsigned int MaxNbItem;
   char etatbth,etatbtb,bthdown,btbdown,bardown,scrldown,listdown,scrollact,oldbtime;
   char firstdown,pbardown,oldlistdown,resv;
   int xbth,ybth,xbtb,ybtb,msxdown,msydown,downYPosBar;
   int PosSynch;
   float TimeAcc;
   char SynchBuff[SIZE_SYNCH_BUFF];
public:
   int PosCursor;
   ListString *LStr;
public:
   void (*Changed)(String *Str,int IndSelect);
   void (*ValidateString)(String *Str,int IndSelect);
public:
   ListBox(int x1,int y1,int x2,int y2,MainWin *Father,ListString *ls=NULL);
   ~ListBox();
   void Draw();
   void Scan();
   void SetFocus();
   void UnsetFocus();
   void Validate();
   void EnsureVisible(int index);
   void PgUP();
   void PgDOWN();
protected:
   int  AjFirstInd();
};

//**************** class VtScrollBar*******************************
class VtScrollBar : public Child {
   int  MaxNbItem,YPosBar,HautBar,OldVtVal,VtMinVal,VtMaxVal,VtVal;
   char etatbth,etatbtb,bthdown,btbdown,scrldown,bardown,oldbtime;
   char firstdown,pbardown,resv[2];
   int xbth,ybth,xbtb,ybtb,msxdown,msydown,downYPosBar;
   int PosSynch;
   float TimeAcc;
   char SynchBuff[SIZE_SYNCH_BUFF];
public:
   void (*Changed)(int Val);
public:
   VtScrollBar(int x,int y1,int y2,MainWin *Father,int MinVal,int MaxVal);
   ~VtScrollBar();
   void Draw();
   void Scan();
   void SetFocus();
   void UnsetFocus();
   void SetVal(int NewVal);
   int  GetVal();
   void SetMinMaxVal(int MinVal,int MaxVal);
   void GetMinMaxVal(int *MinVal,int *MaxVal);
   VtScrollBar& operator=(int NewVal);
   void PgUP();
   void PgDOWN();
};

//**************** class HzScrollBar*******************************
class HzScrollBar : public Child {
   int  MaxNbItem,YPosBar,HautBar,OldHzVal,HzMinVal,HzMaxVal,HzVal;
   char etatbth,etatbtb,bthdown,btbdown,scrldown,bardown,oldbtime;
   char firstdown,pbardown,resv[2];
   int xbth,ybth,xbtb,ybtb,msxdown,msydown,downYPosBar;
   int PosSynch;
   float TimeAcc;
   char SynchBuff[SIZE_SYNCH_BUFF];
public:
   void (*Changed)(int Val);
public:
   HzScrollBar(int x1,int x2,int y,MainWin *Father,int MinVal,int MaxVal);
   ~HzScrollBar();
   void Draw();
   void Scan();
   void SetFocus();
   void UnsetFocus();
   void SetVal(int NewVal);
   int  GetVal();
   void SetMinMaxVal(int MinVal,int MaxVal);
   void GetMinMaxVal(int *MinVal,int *MaxVal);
   HzScrollBar& operator=(int NewVal);
   void PgUP();
   void PgDOWN();
};

//**************** class HzSlider*******************************
class HzSlider : public Child {
   int  MaxNbItem,YPosBar,HautBar,OldHzVal,HzMinVal,HzMaxVal,HzVal;
   char bardown,oldbtime;
   char firstdown;
   int xbth,ybth,xbtb,ybtb,msxdown,msydown,downYPosBar;
public:
   void (*Changed)(int Val);
public:
   HzSlider(int x1,int x2,int y,MainWin *Father,int MinVal,int MaxVal);
   ~HzSlider();
   void Draw();
   void Scan();
   void SetFocus();
   void UnsetFocus();
   void SetVal(int NewVal);
   int  GetVal();
   void SetMinMaxVal(int MinVal,int MaxVal);
   void GetMinMaxVal(int *MinVal,int *MaxVal);
   HzSlider& operator=(int NewVal);
};


//**************** class List *************************************
struct Node {
   Child *Item;
   Node  *Next,*Prev;
};

class List {
   friend class MainWin;
public:
   Node *CurNode2,*NextNode2;
   Node *CurNode,*NextNode;
   Node *Nodes,*FirstNode;
public:
   List();
   ~List();

   Node * Add(Child *NewItem);
   void   Delete(Node *ChNode);
   void   SetFirstNode();
   void   GoNextNode();
   void   SetFirstNode2();
   void   GoNextNode2();
};

//**************** class ListWin **********************************
struct NodeWin {
   MainWin *Item;
   NodeWin  *Next,*Prev;
};

class ListWin {
   friend class MainWin;
public:
   NodeWin *Nodes,*FirstNode;
public:
   NodeWin *CurNode,*NextNode;
public:
   ListWin();
   ~ListWin();

   NodeWin * Add(MainWin *NewItem);
   void Delete(NodeWin *ChNode);
   void SetFirstNode();
   void GoNextNode();
};

//*********** class FlyObj ************************************

class FlyObj {
   friend class WinHandler;
   friend class NodeFly;
   friend class ListFly;

public:
   int  X1,Y1,X2,Y2;
   int LargHz,HautVt;
   NodeFly *MyNode;
   ListFly *MyList;
   WinHandler *MyWinHand;
   CGraphContext *m_GraphCtxt;
   void (*Destructed)();
public:
   FlyObj(int X,int Y,int Larg,int Haut,WinHandler *WH);
   ~FlyObj();
   virtual void Draw();
   virtual void Redraw();
   virtual void Scan();
   virtual void Move(int NewX,int NewY);
};

//************* class FlyMenu *********************************
struct NodeMenu {
   char 	*Name;
   unsigned int NbNodeMenu;
   NodeMenu     *TbNodeMenu;
   unsigned int	Activ;
   void (*Click)();
};

class FlyMenu : public FlyObj {
public:
   int mcol,choix,oldchoix;
   NodeMenu *NdMenu;
public:
   FlyMenu(int x,int y,NodeMenu *NdMn,WinHandler *WH);
   ~FlyMenu();
   void Draw();
   void Scan();
};

//************* class FlyListBox *********************************

class FlyListBox : public FlyObj {
   int OldPosCursor,LastPosCursor,FirstInd,LastFirstInd,YPosBar,HautBar;
   int MaxNbItem;
   char etatbth,etatbtb,bthdown,btbdown,bardown,listdown,scrollact,oldbtime;
   char firstdown,pbardown,resv[2];
   int xbth,ybth,xbtb,ybtb,msxdown,msydown,downYPosBar;
public:
   int  PosCursor;
   ListString *LStr;
   int  *Terminate,*Result;
   void (*ValidateList)(FlyListBox *FLB,int IndSelect);
public:
   FlyListBox(int x,int y,int Larg,int Haut,WinHandler *WH,ListString *ls=NULL);
   ~FlyListBox();
   void Draw();
   void Scan();
protected:
   int  AjFirstInd();
};

//*************** class Menu **************************************
class Menu : public Child {
public:
   int mcol,choix,oldchoix;
   NodeMenu *NdMenu;
   MainWin *MyWin;
public:
   Menu(MainWin *Father,NodeMenu *NdMn);
   ~Menu();
   void Draw();
   void Scan();
   void SetFocus();
   void UnsetFocus();
};

//*************** class ComBox **************************************
class ComBox : public Child {
   int OldSelect,EnableFLB,ResFLB,TermFLB,ActFLB;
   String *OldString;
public:
   int Select;
   ListString *LStr;
   MainWin *MyWin;
   FlyListBox *MyFLB;
   void (*LoadList)(ListString *LS);
   void (*Changed)(String *S,int Select);
public:
   ComBox(int x,int y,int Larg,int Haut,MainWin *Father,ListString *ls=NULL,
          int defsel=-1);
   ~ComBox();
   void Draw();
   void Scan();
   void SetFocus();
   void UnsetFocus();
   int  Chgd();
private:
   void ActiveFLB();
   void DesactiveFLB();
};

//**************** class ListFly **********************************
struct NodeFly {
   FlyObj *Item;
   NodeFly  *Next,*Prev;
};

class ListFly {
   friend class FlyObj;
public:
   NodeFly *Nodes,*FirstNode;
   NodeFly *CurNode,*NextNode;
public:
   ListFly();
   ~ListFly();

   NodeFly * Add(FlyObj *NewObj);
   bool Exist(FlyObj *SeekObj);
   void Delete(NodeFly *Node);
   void DeleteAfter(NodeFly *Node);
   void ClearAll();
   void SetFirstNode();
   void GoNextNode();
   int  Empty();
};

#endif // #ifdef __cplusplus

#endif // #define DGUI_H
