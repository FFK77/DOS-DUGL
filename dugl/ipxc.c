#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/movedata.h>
#include <dpmi.h>
#include <go32.h>
#include "DUGL.h"
#include "intrdugl.h"

unsigned short IPXMaxPacketSize=0;
IPXAddress MyIPXAddress;
IPXNode IPXAllNodes ={ { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };
IPXNetwork IPXSameNetwork= { { 0, 0, 0, 0 } };

int  DetectIPX() {
   __dpmi_regs r;
   bzero(&r,sizeof(__dpmi_regs));
   r.d.eax = 0x7a00;
   __dpmi_int(0x2f, &r);
   return (r.h.al==0xff);
}

unsigned short GetIPXMaxPacketSize() {
   __dpmi_regs r;
   bzero(&r,sizeof(__dpmi_regs));
   r.d.ebx = 0x1a;
   __dpmi_int(0x7a, &r);
   return r.x.ax;
}

void GetIPXInterNetworkAddress(IPXAddress *Address) {
   __dpmi_regs r;
   bzero(&r,sizeof(__dpmi_regs));
   r.d.ebx = 0x9;
   r.x.es = (__tb>>4) ;
   r.d.esi = __tb & 0xf;
   __dpmi_int(0x7a, &r);
   dosmemget(__tb,sizeof(IPXAddress),Address);
   ReverseBuffBytes(&Address->Network,sizeof(IPXNetwork));
   ReverseBuffBytes(&Address->Node,sizeof(IPXNode));
}

int  IPXOpenSocket(IPXSocket *Socket) {
   __dpmi_regs r;
   bzero(&r,sizeof(__dpmi_regs));
   r.d.ebx = 0x0;
   r.d.eax = 0; // open until close or terminate
   r.x.dx = Socket->Socket;
   ReverseBuffBytes(&r.x.dx,sizeof(IPXSocket));
   __dpmi_int(0x7a, &r);
   if (r.h.al!=0) return 0;
   ReverseBuffBytes(&r.x.dx,sizeof(IPXSocket));
   Socket->Socket=r.x.dx;
   return 1;
}

void IPXCloseSocket(IPXSocket *Socket) {
   __dpmi_regs r;
   bzero(&r,sizeof(__dpmi_regs));
   r.d.ebx = 0x1;
   r.x.dx = Socket->Socket;
   ReverseBuffBytes(&r.x.dx,sizeof(IPXSocket));
   __dpmi_int(0x7a, &r);
}

void IPXRelinquishControl() {
   __dpmi_regs r;
   bzero(&r,sizeof(__dpmi_regs));
   r.d.ebx = 0xa;
   __dpmi_int(0x7a, &r);
}

int  InitIPX() {
   if (DetectCPUID()) {
     if (!DetectMMX()) return 0;
   }
   else
     return 0;
   if (DetectIPX()) {
     IPXMaxPacketSize=GetIPXMaxPacketSize();
     GetIPXInterNetworkAddress(&MyIPXAddress);
     return 1;
   }
   return 0;
}

unsigned short GetIPXMaxDataSize() {
   return IPXMaxPacketSize-sizeof(RmIPXPacket);
}

int CreateECB(IPXECB **ECB,unsigned short MaxDataSize) {
   int seg;
   RmIPXECB RmECB;
   if ((!IPXMaxPacketSize) || (MaxDataSize>GetIPXMaxDataSize())) return 0;
   if ((*ECB=(IPXECB *)malloc(sizeof(IPXECB)))==NULL) return 0;
   seg=__dpmi_allocate_dos_memory(((sizeof(RmIPXECB)+MaxDataSize+15)>>4),
   				  &(*ECB)->ECBSel);
   if (seg==-1) { free(*ECB); return 0; }
   (*ECB)->Sign='BCEV';
   (*ECB)->MaxSize=MaxDataSize;
   (*ECB)->OffRmECB=0;
   (*ECB)->SegRmECB=seg;
   bzero(&RmECB,sizeof(RmIPXECB));
   RmECB.FragmentCount=1;
   RmECB.AddressSeg=seg;
   RmECB.AddressOff=sizeof(RmIPXECB)-sizeof(RmIPXPacket);
   dosmemput(&RmECB,sizeof(RmIPXECB),(*ECB)->SegRmECB*16);
   return 1;
}

void DestroyECB(IPXECB *ECB) {
   if (ECB->Sign=='BCEV') {
     __dpmi_free_dos_memory(ECB->ECBSel);
     bzero(ECB,sizeof(IPXECB));
     free(ECB);
   }
}

int  GetECB(IPXECB *ECB) {
   RmIPXECB RmECB;
   if (ECB->Sign!='BCEV') return 0;
   dosmemget(ECB->SegRmECB*16,sizeof(RmIPXECB),&RmECB);
   ECB->InUse=RmECB.InUse;
   ECB->CompletitionCode=RmECB.CompletitionCode;
   ECB->Socket=RmECB.Socket; ReverseBuffBytes(&ECB->Socket,sizeof(IPXSocket));
   ECB->ImmediateAddress=RmECB.ImmediateAddress; ReverseBuffBytes(&ECB->ImmediateAddress,sizeof(IPXNode));

   ECB->Packet.Checksum=RmECB.RmPacket.Checksum; ReverseBuffBytes(&ECB->Packet.Checksum,sizeof(unsigned short));
   ECB->Packet.Length=RmECB.RmPacket.Length; ReverseBuffBytes(&ECB->Packet.Length,sizeof(unsigned short));
   ECB->Packet.TransportControl=RmECB.RmPacket.TransportControl;
   ECB->Packet.Type=RmECB.RmPacket.Type;
   ECB->Packet.DNetwork=RmECB.RmPacket.DNetwork; ReverseBuffBytes(&ECB->Packet.DNetwork,sizeof(IPXNetwork));
   ECB->Packet.DNode=RmECB.RmPacket.DNode; ReverseBuffBytes(&ECB->Packet.DNode,sizeof(IPXNode));
   ECB->Packet.DSocket=RmECB.RmPacket.DSocket; ReverseBuffBytes(&ECB->Packet.DSocket,sizeof(IPXSocket));
   ECB->Packet.SNetwork=RmECB.RmPacket.SNetwork; ReverseBuffBytes(&ECB->Packet.SNetwork,sizeof(IPXNetwork));
   ECB->Packet.SNode=RmECB.RmPacket.SNode; ReverseBuffBytes(&ECB->Packet.SNode,sizeof(IPXNode));
   ECB->Packet.SSocket=RmECB.RmPacket.SSocket; ReverseBuffBytes(&ECB->Packet.SSocket,sizeof(IPXSocket));
   return 1;
}

unsigned short GetECBData(IPXECB *ECB,void *Data) {
   unsigned short sizeret;
   if (ECB->Sign!='BCEV' || ECB->InUse || ECB->CompletitionCode ||
       ECB->Packet.Length<=sizeof(RmIPXPacket)) return 0;
   sizeret=ECB->Packet.Length-sizeof(RmIPXPacket);
   dosmemget(ECB->SegRmECB*16+sizeof(RmIPXECB),sizeret,Data);
   return sizeret;
}

int IPXSendPacket(IPXECB *ECB,IPXNetwork *Network,IPXNode *Node,IPXSocket *Socket,
                  unsigned char Type,void *Data,unsigned short SizeData) {
   RmIPXECB RmECB;
   __dpmi_regs r;

   if (ECB->Sign!='BCEV' || SizeData>ECB->MaxSize) return 0;
   dosmemget(ECB->SegRmECB*16,sizeof(RmIPXECB),&RmECB);
   bzero(&RmECB.RmPacket,sizeof(RmIPXPacket));
   RmECB.RmPacket.Type=Type;
   RmECB.RmPacket.DNetwork=*Network; ReverseBuffBytes(&RmECB.RmPacket.DNetwork,sizeof(IPXNetwork));
   RmECB.RmPacket.DNode=*Node; ReverseBuffBytes(&RmECB.RmPacket.DNode,sizeof(IPXNode));
   RmECB.RmPacket.DSocket=*Socket; ReverseBuffBytes(&RmECB.RmPacket.DSocket,sizeof(IPXSocket));
   RmECB.ESROff=RmECB.ESRSeg=0;
   RmECB.Socket=*Socket; ReverseBuffBytes(&RmECB.Socket,sizeof(IPXSocket));
   RmECB.ImmediateAddress=*Node; ReverseBuffBytes(&RmECB.ImmediateAddress,sizeof(IPXNode));
   RmECB.Size=sizeof(RmIPXPacket)+SizeData;
   dosmemput(&RmECB,sizeof(RmIPXECB),ECB->SegRmECB*16);
   dosmemput(Data,SizeData,ECB->SegRmECB*16+sizeof(RmIPXECB));
   bzero(&r,sizeof(__dpmi_regs));
   r.d.ebx = 0x3;
   r.x.es = ECB->SegRmECB;
   r.d.edi = ECB->OffRmECB;
   __dpmi_int(0x7a, &r);
//   if (GetECB(ECB)) {
//     if ((!ECB->InUse) && ECB->CompletitionCode) return 0;
//   }
//   else return 0;
   return 1;
}

int  IPXListenPacket(IPXECB *ECB,IPXSocket *Socket) {
   RmIPXECB RmECB;
   __dpmi_regs r;
   if (ECB->Sign!='BCEV') return 0;
   dosmemget(ECB->SegRmECB*16,sizeof(RmIPXECB),&RmECB);
   bzero(&RmECB.RmPacket,sizeof(RmIPXPacket));
   RmECB.ESROff=RmECB.ESRSeg=0;
   RmECB.Socket=*Socket; ReverseBuffBytes(&RmECB.Socket,sizeof(IPXSocket));
   RmECB.Size=ECB->MaxSize+sizeof(RmIPXPacket);
   dosmemput(&RmECB,sizeof(RmIPXECB),ECB->SegRmECB*16);
   bzero(&r,sizeof(__dpmi_regs));
   r.d.ebx = 0x4;
   r.x.es = ECB->SegRmECB;
   r.d.edi = ECB->OffRmECB;
   __dpmi_int(0x7a, &r);
   return 1;
}

int  IPXCancelECB(IPXECB *ECB) {
   __dpmi_regs r;
   if (ECB->Sign!='BCEV') return 0;
   bzero(&r,sizeof(__dpmi_regs));
   r.d.ebx = 0x6;
   r.x.es = ECB->SegRmECB;
   r.d.edi = ECB->OffRmECB;
   __dpmi_int(0x7a, &r);
   if (r.h.al!=0) return 0;
   return 1;
}

