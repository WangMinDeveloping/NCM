#ifndef IPPACK_H
#define IPPACK_H
extern unsigned char cData[];

void CopyMacIpHead(unsigned char* buf);
void SelfIntroductionPackage(unsigned char * buf);//tcp头部和user头部
void PutIpAddr();
unsigned short myChecksum(unsigned short * addr, int count);
void XchangeIpPort(unsigned char* buf);
#endif
