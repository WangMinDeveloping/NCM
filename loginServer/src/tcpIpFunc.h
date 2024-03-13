#include "structAndEnum.h"
#include "LoginServer.h"
#include "toolFunc.h"
#ifndef tcpIpFunc_h
#define tcpIpFunc_h
unsigned short myChecksum(unsigned short * addr, int count);//校验函数声明
void IPHeadSetting(unsigned char * buf,unsigned short totalLen);//ip头组装
void PackageMake(unsigned char * buf,unsigned long long id,enum EPackageType packType);//组包函数
void printLoginPack(unsigned char * buf,unsigned char witch);//witch:1是打印错误码，0是打印密码
#endif
