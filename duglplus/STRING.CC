#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <unistd.h>
#include <string.h>
#include <DUGL.h>
#include "DGUI.H"

//**************** class ListString *******************************
//*****************************************************************

ListString::ListString() {
   FirstNode=CurNode=NextNode=Nodes=NULL;   NbElt=0;
   FirstNode=NewNodeString();
   if (FirstNode!=NULL) {
     CurNode=FirstNode;
     LastNode=FirstNode;
     CurNodeFirstIdx=0;
   }
}

ListString::ListString(int NbStr,char *Str[]) {
   FirstNode=CurNode=NextNode=Nodes=NULL;   NbElt=0;
   FirstNode=NewNodeString();
   if (FirstNode!=NULL) {
     CurNode=FirstNode;
     LastNode=FirstNode;
     CurNodeFirstIdx=0;
   }
   for (int i=0;i<NbStr;i++) Add(Str[i]);
}

void ListString::DeleteCurNode() {
  NodeString *delNode = CurNode;
  // don't delete if it's the last node,
  if (CurNode->Next==NULL && CurNode->Prev==NULL) {
    return;
  }

  // free items
  if (CurNode->NbItems>0) {
    for (int i=0;i<CurNode->NbItems;i++)
      delete (CurNode->Items[i]);
    NbElt-=CurNode->NbItems;
  }

  // free table of pointers
  free(CurNode->Items);

  if (CurNode->Next!=NULL) {
    CurNode=CurNode->Next;
    CurNode->Prev=delNode->Prev;
    if (delNode==FirstNode) FirstNode=CurNode;
  }
  else { // deleting last Node
    CurNode=CurNode->Prev;
    CurNode->Next=NULL;
    LastNode = CurNode;
    CurNodeFirstIdx -= CurNode->NbItems;
  }
  CurNodeFirstIdx=-1;
  CurNode=NULL;

  delete delNode;
}

#define LISTSTR_DEFAULT_FREE   32
#define LISTSTR_DEFAULT_RESV   0
#define LISTSTR_FULL_RESVPLUS  8

// create an empty new node - internal
NodeString* ListString::NewNodeString() {
   // alloc a new node
   NodeString *myNodeString=new NodeString;
   if (myNodeString==NULL) return NULL; // no memory
   // alloc a pointer table
   myNodeString->Items = (String**) malloc(sizeof(String*)*
        (LISTSTR_DEFAULT_FREE+LISTSTR_DEFAULT_RESV));
   if (myNodeString->Items==NULL) { // no memory
        delete myNodeString;
        return NULL;
   }
   myNodeString->NbItems=0; // number of the available items
   myNodeString->NbFreeItems=LISTSTR_DEFAULT_FREE; // number of non used pointers
   myNodeString->NbResvItems=LISTSTR_DEFAULT_RESV; // number of reserve items used for inserts
   myNodeString->NbAllItems=LISTSTR_DEFAULT_FREE+LISTSTR_DEFAULT_RESV; // size of the pointer table Items
   myNodeString->Next=NULL;
   myNodeString->Prev=NULL;

   return myNodeString;
}

// add a new node the last
void ListString::AddNode(NodeString* newNode) {
   if (LastNode==NULL) return;
   // chain the new Node
   LastNode->Next = newNode;
   newNode->Prev= LastNode;
   newNode->Next= NULL;
   // the new node will be the last one
   LastNode = newNode;
}

// free all strings and memory used by ListString
ListString::~ListString() {
   NodeString *N=NULL;
   Nodes=FirstNode;
   while (Nodes != NULL) {   // until end of list
      N = Nodes;
      Nodes = N->Next;       // point to next node
      // delete all the available items
      for (int i=0;i<N->NbItems;i++) delete (N->Items[i]);
      free(N->Items);
      delete N;
   }
}

ListString::ListString(ListString &LS) {
/*   NodeString *N,*ParcNode;
   ParcNode=LS.FirstNode;
   while (ParcNode != NULL) {
      N = ParcNode;
      Add(N->Item);
      ParcNode = N->Next;
     }*/
}

int  ListString::Add(String *NewStr) {
   NodeString *N,*ParcNode;             // N is pointer to a node

   if (LastNode->NbFreeItems>0) {
      if (NewStr!=NULL) { // insert the new string
         Chgd=1;
         LastNode->Items[LastNode->NbItems]=NewStr;
         LastNode->NbItems++;
         LastNode->NbFreeItems--;
         NbElt++;
         return (NbElt-1);
      }
      else
         return -1; // failed
   }
   else {
        N = NewNodeString();
        if (N!=NULL) {
          AddNode(N); // add the node,
          Add(NewStr); // recall (recursive call) this function with a sucess this time :)
        }
        else
          return -1; // failed
   }
//   return Add(NewStr->StrPtr);
}


int  ListString::Add(char *str) {
   NodeString *N,*ParcNode;             // N is pointer to a node

   if (LastNode->NbFreeItems>LastNode->NbResvItems) {
      String *Str=new String(str); // new string
      if (Str!=NULL) { // insert the new string
         Chgd=1;
         LastNode->Items[LastNode->NbItems]=Str;
         LastNode->NbItems++;
         LastNode->NbFreeItems--;
         NbElt++;
         return (NbElt-1);
      }
      else
         return -1; // failed
   }
   else {
        N = NewNodeString();
        if (N!=NULL) {
          AddNode(N); // add the node,
          Add(str); // recall (recursive call) this function with a sucess this time :)
        }
        else
          return -1; // failed
   }
}

int ListString::Insert(int index,String *Str) {
  // insert at the last ?
  if (NbElement()==0 || index>NbElement()-1)
     return Add(Str);
  String *InsStr=Element(index); // Seek to the where index
  if (InsStr==NULL) return -1;
  int idxInsStr=index-CurNodeFirstIdx;
  // available space ? if not resize
  if (CurNode->NbFreeItems>0) {
     String **newItems =  (String**) malloc(sizeof(String*)*
        CurNode->NbAllItems+LISTSTR_FULL_RESVPLUS);
     if (newItems==NULL) return -1; // failed
     // copy old Items
     memcpy(newItems, CurNode->Items, sizeof(String*)*CurNode->NbAllItems);
     free(CurNode->Items); // free old buffer
     CurNode->Items=newItems; // update pointer
     CurNode->NbAllItems+=LISTSTR_FULL_RESVPLUS;
     CurNode->NbFreeItems+=LISTSTR_FULL_RESVPLUS;
  }
  // insert at the end ?
  if (idxInsStr>CurNode->NbItems-1) {
     CurNode->Items[CurNode->NbItems]=Str;
  }
  else {
     // shift up items table
     for (int i=CurNode->NbItems;i>idxInsStr;i--)
        CurNode->Items[i]=CurNode->Items[i-1];
     // insert the new String
     CurNode->Items[idxInsStr]=Str;
  }
  CurNode->NbItems++;
  CurNode->NbFreeItems--;
  NbElt++;
  return (NbElt-1);
}
/*
struct NodeString {
   String **Items; // table of pointers to String items
   int  NbItems, // number of the available items
        NbFreeItems, // number of non used pointers
        NbResvItems, // number of reserve items used for inserts
        NbAllItems; // size of the pointer table Items
   NodeString *Next,
              *Prev;
};
*/


int ListString::Insert(int index,char *str) {
  // insert at the last ?
  if (NbElement()==0 || index>NbElement()-1)
     return Add(str);
  String *InsStr=Element(index); // Seek to the where index
  if (InsStr==NULL) return -1;
  int idxInsStr=index-CurNodeFirstIdx;
  // available space ? if not resize
  if (CurNode->NbFreeItems>0) {
     String **newItems =  (String**) malloc(sizeof(String*)*
        CurNode->NbAllItems+LISTSTR_FULL_RESVPLUS);
     if (newItems==NULL) return -1; // failed
     // copy old Items
     memcpy(newItems, CurNode->Items, sizeof(String*)*CurNode->NbAllItems);
     free(CurNode->Items); // free old buffer
     CurNode->Items=newItems; // update pointer
     CurNode->NbAllItems+=LISTSTR_FULL_RESVPLUS;
     CurNode->NbFreeItems+=LISTSTR_FULL_RESVPLUS;
  }
  String *Str=new String(str); // new string
  if (Str==NULL) return -1;
  // insert at the end ?
  if (idxInsStr>CurNode->NbItems-1) {
     CurNode->Items[CurNode->NbItems]=Str;
  }
  else {
     // shift up items table
     for (int i=CurNode->NbItems;i>idxInsStr;i--)
        CurNode->Items[i]=CurNode->Items[i-1];
     // insert the new String
     CurNode->Items[idxInsStr]=Str;
  }
  CurNode->NbItems++;
  CurNode->NbFreeItems--;
  NbElt++;
  return (NbElt-1);
}


int  ListString::Delete(String *Str, int POS) {
  int pos=Index(Str,POS);
  if (pos>=0) {
    Delete(pos);
    return pos;
  }
  else
    return -1;
}

int  ListString::Delete(char *str,int POS) {
  int pos=Index(str,POS);
  if (pos>=0) {
    Delete(pos);
    return pos;
  }
  else
    return -1;
}

int  ListString::Delete(int index) {
  String *DelStr=Element(index); // Seek to the where index
  int i; // counter
  if (DelStr==NULL) return 0; // index invalid
  int idxDelStr=index-CurNodeFirstIdx;
  if (idxDelStr<0) return 0; //error
  if (CurNode->NbItems==1) {
    if (CurNode->Next==NULL && CurNode->Prev==NULL) { // last node
      delete CurNode->Items[0];
      CurNode->NbItems=0;
      CurNode->NbFreeItems++;
    }
    else
      DeleteCurNode();
  }
  else {
    // shift the pointer table
    for (i=idxDelStr;i<CurNode->NbItems-1;i++)
      CurNode->Items[i]=CurNode->Items[i+1];
    CurNode->NbItems--;
    CurNode->NbFreeItems++;
    delete DelStr;
  }
  NbElt--;
  Chgd=1;
  return 1;
}

int ListString::Index(String *Str, int POS) {
  return Index(Str->StrPtr);
}

int ListString::Index(char *strfind, int POS) {
  int i,j,k; // counter
  char *str;
  // find the first occ of strfind
  if (POS==LISTSTR_FIRST) {
    for (i=0;i<NbElt;i++) {
      str = Element(i)->StrPtr;
      if (strcmp(str,strfind)==0)
        return i;
    }
    return -1;
  }
  // find the last occ of strfind
  if (POS==LISTSTR_LAST) {
    for (i=NbElt-1;i>=0;i--) {
      str = Element(i)->StrPtr;
      if (strcmp(str,strfind)==0)
        return i;
    }
    return -1;
  }
  // find the occ of strfind number POS or the last one
  for (i=0,j=0;i<NbElt;i++) {
    str = Element(i)->StrPtr;
    if (strcmp(str,strfind)==0) {
      if (j==POS) return i;
      k=i; // save last found position
      j++; // increase count of str found
    }
    if (j>0) return k;
  }
  return -1;
}

void ListString::ClearListStr() {
   NodeString *N;
   Nodes=FirstNode;
   while (Nodes != NULL) {   // until end of list
      N = Nodes;
      Nodes = N->Next;       // point to next node
      // delete all the available items
      for (int i=0;i<N->NbItems;i++) delete (N->Items[i]);
      free(N->Items);
      delete N;
   }
   Chgd=1;
   Nodes=FirstNode=CurNode=NextNode=NULL;
   CurNodeFirstIdx= -1;
   NbElt=0;
   FirstNode=NewNodeString();
   if (FirstNode!=NULL) {
     CurNode=FirstNode;
     LastNode=FirstNode;
     CurNodeFirstIdx=0;
   }
}

void QuickSortLS(ListString *LS, int begin, int end, bool bAsc)
{
  int left  = begin +1;
  int right = end;
  int pivot = 0;
  String *StrPiv = NULL;
  String *StrLeft = NULL;
  String *StrRight = NULL;

  if(begin>=end || LS==NULL)
     return;

  StrPiv = (*LS)[begin];
  if(StrPiv==NULL)
    return;

  while(left <= right)
  {
    if(bAsc) {
       while((left <= end) && (*(*LS)[left] <= *StrPiv)) left++;
       while((right >= begin) && (*(*LS)[right] > *StrPiv)) right--;
    }
    else {
       while((left <= end) && (*(*LS)[left] >= *StrPiv)) left++;
       while((right >= begin) && (*(*LS)[right] < *StrPiv)) right--;
    }
    if(left < right)
      LS->Swap(left, right);
  }
  LS->Swap(begin, right);

  QuickSortLS(LS, begin, right-1, bAsc);
  QuickSortLS(LS, right+1, end, bAsc);
}


/*void QuickSortLS(ListString *LS, int left, int right)
{
   if(left < right)
   {
      int pivot = PartitionLS(LS, left, right);

      QuickSortLS(LS, left, pivot);
      QuickSortLS(LS, pivot +1, right);
   }
}

int PartitionLS(ListString *LS, int left, int right)
{
    int pivot = (*LS)[left]->Length();
    String *pStr = NULL;
    while(true) {

      while((*LS)[right]->Length() > pivot) right --;
      while((*LS)[left]->Length() < pivot) left ++;
      if(left < right && (*LS)[left]->Length() != (*LS)[right]->Length())
         LS->Swap(left, right);
      else
         return right;
    }
}*/

void ListString::Sort(bool bAsc) {
   if(NbElt>1)
     QuickSortLS(this, 0, NbElt-1, bAsc);
}


int  ListString::NbElement() {
   return NbElt;
}

int  ListString::Changed() {
   char OldChgd=Chgd;
   Chgd=0;
   return OldChgd;
}

void  ListString::Swap(int ind1, int ind2)
{
   String *str1 = NULL;
   NodeString *NodeS1 = NULL;
   int indS1;
   String *str2 = NULL;
   NodeString *NodeS2 = NULL;
   int indS2;

   if((str1 = Element(ind1)) != NULL) {
      NodeS1 = CurNode;
      indS1 = ind1-CurNodeFirstIdx;

      if((str2 = Element(ind2)) != NULL) {
         CurNode->Items[ind2-CurNodeFirstIdx] = str1;
         NodeS1->Items[indS1] = str2;
      }

   }
}

String* ListString::Element(int ind) {
   NodeString *ParcNode;
//   int i,lim20;
   // ind error
   if (ind<0 || ind>NbElt-1) return NULL;
   // CurNodeFirstIdx invalid then start from the begin
   if (CurNodeFirstIdx==-1 && CurNode!=NULL) {
     // init to the start of the list
     CurNode=FirstNode;
     CurNodeFirstIdx=0;
   }
   // loop until failure or success ;)
   for (;;) {
     // immediat result
     if (CurNodeFirstIdx!=-1 && ind>=CurNodeFirstIdx &&
        ind<CurNodeFirstIdx+CurNode->NbItems) {
        return CurNode->Items[ind-CurNodeFirstIdx];
     }
     else
        // go to the next or prev node if possible
        if ((CurNode->Next!=NULL && ind>CurNodeFirstIdx)  ||
            (CurNode->Prev!=NULL && ind<CurNodeFirstIdx) ) {
          if (ind>CurNodeFirstIdx) { // go forward
            CurNodeFirstIdx+=CurNode->NbItems;
            CurNode=CurNode->Next;
          }
          else { // go backward
            CurNodeFirstIdx-=CurNode->Prev->NbItems;
            CurNode=CurNode->Prev;
            if (CurNode==FirstNode) CurNodeFirstIdx=0;
          }
        }
        else
          break;
   }
   return NULL;
}

String* ListString::operator[](int ind) {
   return Element(ind);
}


//**************************************************************************
//********************** class STRING  *************************************
//**************************************************************************


String::String(char *s,int MaxSize) {
   StrPtr=new char[MaxSize];
   Chgd=0; ExtChgd=NULL;
   strcpy(StrPtr,s);
}

String::String(char *s) {
   int l=strlen(s),maxsize;
   maxsize=(l<360)?360:l+1;
   Chgd=0; ExtChgd=NULL;
   StrPtr=new  char[maxsize];
   strcpy(StrPtr,s);
}

String::String(int MaxSize) {
   StrPtr=new char[MaxSize];
   Chgd=0; ExtChgd=NULL;
   StrPtr[0]='\0';
}

String::String() {
   StrPtr=new  char[360];
   Chgd=0; ExtChgd=NULL;
   StrPtr[0]='\0';
}

String::String(String &S) {   // constructeur de copie
   if (S.Length()<360)
     StrPtr=new char [360];
   else
     StrPtr=new char [S.Length()+1];
   Chgd=1; ExtChgd=S.ExtChgd;
   strcpy(StrPtr,S.StrPtr);
}

String::~String() {	     // Destructeur
   free(StrPtr);
}

int String::Changed() {
   int savChgd=Chgd;
   Chgd=0;
   return savChgd;
}

int String::Length() {
  return strlen(StrPtr);
}


int String::GetInt() {
   return atoi(StrPtr);
}

unsigned int String::GetUInt() {
   return (unsigned int)(atof(StrPtr));
}

long long int String::GetLLongInt() {
   return (long long int)(atof(StrPtr));
}

unsigned long long int String::GetULLongInt() {
   return (unsigned long long int)(atof(StrPtr));
}

double String::GetDouble() {
   return atof(StrPtr);
}

unsigned int  String::AddCursChar(unsigned int PosCurs,char c,int Ins) {
   unsigned int len=strlen(StrPtr),pc,i;
   char *tmp;
   if (PosCurs>len) pc=len;
   else pc=PosCurs;
   if (!c) return pc;
   if (pc==len) {
     *this+=c;
     return len+1;
   }
   if (Ins) {
     tmp=new char[len-pc];
     memcpy(tmp,&StrPtr[pc],len-pc);
     memcpy(&StrPtr[pc+1],tmp,len-pc);
     StrPtr[len+1]='\0';
     delete tmp;
   }
   StrPtr[pc]=c;
   Chgd=1;
   if (ExtChgd!=NULL) *ExtChgd=1;
   return pc+1;
}

unsigned int  String::DelCurs(unsigned int PosCurs) {
   unsigned int len=strlen(StrPtr),pc;
   char *tmp;
   if (PosCurs>len) pc=len;
   else pc=PosCurs;
   if (pc==len || len==0) return pc;
   Chgd=1;
   if (ExtChgd!=NULL) *ExtChgd=1;
   if (pc==(len-1)) {
     StrPtr[pc]='\0'; return pc; }

   tmp=new char[len-pc-1];
   memcpy(tmp,&StrPtr[pc+1],len-pc-1);
   memcpy(&StrPtr[pc],tmp,len-pc-1);
   StrPtr[len-1]='\0';
   delete tmp;
   return pc;
}

unsigned int  String::BackDelCurs(unsigned int PosCurs) {
   unsigned int len=strlen(StrPtr),pc;
   char *tmp;
   if (PosCurs>len) pc=len;
   else pc=PosCurs;
   if (pc==0 || len==0) return pc;
   Chgd=1;
   if (ExtChgd) *ExtChgd=1;
   if (pc==len) { StrPtr[len-1]='\0'; return pc-1; }
   tmp=new char[len-pc];
   memcpy(tmp,&StrPtr[pc],len-pc);
   memcpy(&StrPtr[pc-1],tmp,len-pc);
   StrPtr[len-1]='\0';
   delete tmp;
   return pc-1;
}

String operator+(String s1, String s2) {
   String *STemp=new String(strlen(s1.StrPtr)+strlen(s2.StrPtr)+3);
   if (STemp!=NULL) sprintf(STemp->StrPtr,"%s%s",s1.StrPtr,s2.StrPtr);
   return *STemp;
}

String operator+(String s1, double d) {
   String *STemp=new String(strlen(s1.StrPtr)+350);
   if (STemp!=NULL) sprintf(STemp->StrPtr,"%s%f",s1.StrPtr,d);
   return *STemp;
}

String operator+(double d, String s1) {
   String *STemp=new String(strlen(s1.StrPtr)+350);
   if (STemp!=NULL) sprintf(STemp->StrPtr,"%f%s",d,s1.StrPtr);
   return *STemp;
}

String operator+(String s1, int i) {
   String *STemp=new String(strlen(s1.StrPtr)+40);
   if (STemp!=NULL) sprintf(STemp->StrPtr,"%s%i",s1.StrPtr,i);
   return *STemp;
}

String operator+(int i, String s1) {
   String *STemp=new String(strlen(s1.StrPtr)+40);
   if (STemp!=NULL) sprintf(STemp->StrPtr,"%i%s",i,s1.StrPtr);
   return *STemp;
}

String operator+(String s1, unsigned int u) {
   String *STemp=new String(strlen(s1.StrPtr)+40);
   if (STemp!=NULL) sprintf(STemp->StrPtr,"%s%u",s1.StrPtr,u);
   return *STemp;
}

String operator+(unsigned int u, String s1) {
   String *STemp=new String(strlen(s1.StrPtr)+40);
   if (STemp!=NULL) sprintf(STemp->StrPtr,"%u%s",u,s1.StrPtr);
   return *STemp;
}


String operator+(String s1, long long int D) {
   String *STemp=new String(strlen(s1.StrPtr)+40);
   if (STemp!=NULL) sprintf(STemp->StrPtr,"%s%LD",s1.StrPtr,D);
   return *STemp;
}

String operator+(long long int D, String s1) {
   String *STemp=new String(strlen(s1.StrPtr)+40);
   if (STemp!=NULL) sprintf(STemp->StrPtr,"%LD%s",D,s1.StrPtr);
   return *STemp;
}

String operator+(String s1, unsigned long long int U) {
   String *STemp=new String(strlen(s1.StrPtr)+40);
   if (STemp!=NULL) sprintf(STemp->StrPtr,"%s%LU",s1.StrPtr,U);
   return *STemp;
}

String operator+(unsigned long long int U, String s1) {
   String *STemp=new String(strlen(s1.StrPtr)+40);
   if (STemp!=NULL) sprintf(STemp->StrPtr,"%LU%s",U,s1.StrPtr);
   return *STemp;
}

String operator+(String s1, char c) {
   String *STemp=new String(strlen(s1.StrPtr)+3);
   if (STemp!=NULL) sprintf(STemp->StrPtr,"%s%c",s1.StrPtr,c);
   return *STemp;
}

String operator+(char c, String s1) {
   String *STemp=new String(strlen(s1.StrPtr)+3);
   if (STemp!=NULL) sprintf(STemp->StrPtr,"%c%s",c,s1.StrPtr);
   return *STemp;
}

String operator+(String s1, char *s) {
   String *STemp=new String(strlen(s1.StrPtr)+strlen(s)+3);
   if (STemp!=NULL) sprintf(STemp->StrPtr,"%s%s",s1.StrPtr,s);
   return *STemp;
}

String operator+(char *s, String s1) {
   String *STemp=new String(strlen(s1.StrPtr)+strlen(s)+3);
   if (STemp!=NULL) sprintf(STemp->StrPtr,"%s%s",s,s1.StrPtr);
   return *STemp;
}

String& String::operator+=(String s1) {
   char s[strlen(StrPtr)+strlen(s1.StrPtr)+4];
   sprintf(s,"%s%s",StrPtr,s1.StrPtr);
   strcpy(StrPtr,s);
   Chgd=1;
   if (ExtChgd) *ExtChgd=1;
   return *this;
}

String& String::operator+=(char * s) {
   char s2[strlen(StrPtr)+strlen(s)+4];
   sprintf(s2,"%s%s",StrPtr,s);
   strcpy(StrPtr,s2);
   Chgd=1;
   if (ExtChgd) *ExtChgd=1;
   return *this;
}

String& String::operator+=(double d) {
   char s[strlen(StrPtr)+350];
   sprintf(s,"%s%f",StrPtr,d);
   strcpy(StrPtr,s);
   Chgd=1;
   if (ExtChgd) *ExtChgd=1;
   return *this;
}

String& String::operator+=(int i) {
   char s[strlen(StrPtr)+40];
   sprintf(s,"%s%i",StrPtr,i);
   strcpy(StrPtr,s);
   Chgd=1;
   if (ExtChgd) *ExtChgd=1;
   return *this;
}

String& String::operator+=(unsigned int u) {
   char s[strlen(StrPtr)+40];
   sprintf(s,"%s%u",StrPtr,u);
   strcpy(StrPtr,s);
   Chgd=1;
   if (ExtChgd) *ExtChgd=1;
   return *this;
}

String& String::operator+=(long long int D) {
   char s[strlen(StrPtr)+40];
   sprintf(s,"%s%LD",StrPtr,D);
   strcpy(StrPtr,s);
   Chgd=1;
   if (ExtChgd) *ExtChgd=1;
   return *this;
}

String& String::operator+=(unsigned long long int U) {
   char s[strlen(StrPtr)+40];
   sprintf(s,"%s%LU",StrPtr,U);
   strcpy(StrPtr,s);
   Chgd=1;
   if (ExtChgd) *ExtChgd=1;
   return *this;
}

String& String::operator+=(char c) {
   char s[strlen(StrPtr)+3];
   sprintf(s,"%s%c",StrPtr,c);
   strcpy(StrPtr,s);
   Chgd=1;
   if (ExtChgd) *ExtChgd=1;
   return *this;
}

String& String::operator=(String s1) {
   sprintf(StrPtr,"%s",s1.StrPtr);
   Chgd=1;
   if (ExtChgd) *ExtChgd=1;
   return *this;
}

String& String::operator=(char *s) {
   sprintf(StrPtr,"%s",s);
   Chgd=1;
   if (ExtChgd) *ExtChgd=1;
   return *this;
}

String& String::operator=(double d) {
   sprintf(StrPtr,"%f",d);
   Chgd=1;
   if (ExtChgd) *ExtChgd=1;
   return *this;
}

String& String::operator=(int i) {
   sprintf(StrPtr,"%i",i);
   Chgd=1;
   if (ExtChgd) *ExtChgd=1;
   return *this;
}

String& String::operator=(unsigned int u) {
   sprintf(StrPtr,"%u",u);
   Chgd=1;
   if (ExtChgd) *ExtChgd=1;
   return *this;
}

String& String::operator=(long long int D) {
   sprintf(StrPtr,"%LD",D);
   Chgd=1;
   if (ExtChgd) *ExtChgd=1;
   return *this;
}

String& String::operator=(unsigned long long int U) {
   sprintf(StrPtr,"%LU",U);
   Chgd=1;
   if (ExtChgd) *ExtChgd=1;
   return *this;
}

String& String::operator=(char c) {
   sprintf(StrPtr,"%c",c);
   Chgd=1;
   if (ExtChgd) *ExtChgd=1;
   return *this;
}

bool  String::operator > (String &S) {
  return (strcmp(StrPtr, S.StrPtr)>0);
}

bool  String::operator < (String &S) {
  return (strcmp(StrPtr, S.StrPtr)<0);
}

bool  String::operator <= (String &S) {
  return (strcmp(StrPtr, S.StrPtr)<=0);
}

bool  String::operator >= (String &S) {
  return (strcmp(StrPtr, S.StrPtr)>=0);
}

bool  String::operator != (String &S) {
  return (strcmp(StrPtr, S.StrPtr)!=0);
}

bool  String::operator == (String &S) {
  return (strcmp(StrPtr, S.StrPtr)==0);
}

///////////

bool  String::operator > (char *S) {
  return (strcmp(StrPtr, S)>0);
}

bool  String::operator < (char *S) {
  return (strcmp(StrPtr, S)<0);
}

bool  String::operator <= (char *S) {
  return (strcmp(StrPtr, S)<=0);
}

bool  String::operator >= (char *S) {
  return (strcmp(StrPtr, S)>=0);
}

bool  String::operator != (char *S) {
  return (strcmp(StrPtr, S)!=0);
}

bool  String::operator == (char *S) {
  return (strcmp(StrPtr, S)==0);
}

// 1.02

int String::FindChar(char c, int Pos) {
  int len;
  if ((len=strlen(StrPtr))<=Pos) return -1;
  for (int i=Pos;i<len;i++)
    if (c==StrPtr[i]) return i; // found
  return -1; // not found ! :(
}


String* String::SubString(int Pos, int length) {
  int ln=Length(),
      lncpy; // length of copy
  char *substr = NULL;
  String *SRet = NULL;

  if (Pos>=ln || length==0 || ln==0) lncpy=0;
  else {
    if (Pos+length<=ln) lncpy=length;
    else lncpy=ln-Pos;
  }
  if (lncpy>0) {
     substr = new char[lncpy+1];
     memcpy((void*)substr, (void*)&StrPtr[Pos], lncpy);
     substr[lncpy]='\0';
     SRet = new String(substr);
     delete substr;
  }
  else
     SRet = new String("");

  return SRet;
}

String* String::SubString(int Pos, char COpen, char CClose) {
  int ln=Length(),
    nbOpen=0, // nb open car found
    nbClose=0, // nb close car found
    posFirstCOpen;

  // invalid pos
  if (Pos>=ln) return new String("");
  // search
  for (int i=Pos;i<ln;i++) {
    if (StrPtr[i]==COpen) {
      nbOpen++;
      if (nbOpen==1) posFirstCOpen=i;
    }
    else
      if (StrPtr[i]==CClose && nbOpen>0) { nbClose++; nbOpen--; }
    // substring found
    if (nbOpen==0 && nbClose>0)
      return SubString(posFirstCOpen+1,i-(posFirstCOpen+1));
  }
  // else no open close substring found
  return new String("");
}

ListString* String::Split(char delim) {
 int cpos = 0;
 int lngth = Length();
 int findpos = 0;

 ListString *newLStr= new ListString();
 if (newLStr==NULL) return NULL; // no mem
 if (lngth<=0) {
   newLStr->Add("");
   return newLStr; // empty str
 }
 // get frags
 for (cpos = 0; cpos < lngth;)
 {
    findpos = FindChar(delim, cpos);
    if(findpos != -1) {
       newLStr->Add(SubString(cpos, findpos-cpos));
       cpos = findpos + 1;
    }
    else {
       newLStr->Add(SubString(cpos, lngth-cpos));
       break;
    }
 }
 return newLStr;
}

ListString* String::Split(char *StrDelim) {

}

ListString* String::SplitPos(int Pos,char delim) {
 char sep[2];
 sep[0]=delim; sep[1]=0;
 char **strp;
 char *frag,*pdeb;

 ListString *newLStr= new ListString();
 if (newLStr==NULL) return NULL; // no mem
 if (Length()<=0) return newLStr; // empty str
 // add first fragment before Pos
 if(Pos>0)
   newLStr->Add(SubString(0,Pos));
 // short String ?
 if (Pos>=Length()) return newLStr;
 // set strp to Pos
 pdeb=&StrPtr[Pos];
 strp=&pdeb;
 // get frags
 while ((frag=strsep(strp,sep))!=NULL) {
   newLStr->Add(frag);
 }
 return newLStr;
}

ListString* String::SplitPos(int Pos,char *StrSep) {
}

void String::DelSpacesTabs() {
}

void String::Del13_10() {
  int lngth = Length();

  for(int i=lngth-1;i>=0;i--)
  {
    if(StrPtr[i] != 0) {
      if(StrPtr[i] == 13) StrPtr[i] = 0;
      else
        if(StrPtr[i] == 10) StrPtr[i] = 0;
      else
        break;
    }
  }
}

