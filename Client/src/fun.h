#pragma once
#include"header.h"
#include"rbTreeValueOnly.h"
#include <stdio.h>
extern unsigned int uErrorNumber;//操作的错误码
typedef unsigned int (*PackageHandle)(struct sIpData *pIPData);//操作的错误码
extern int clientsocket;

extern struct sockaddr_ll *sll;                                 //原始套接字地址结构
extern struct ifreq *req;                                       //网络接口地址

extern struct SSocketSeq *pSocketSeq;
extern CRedBlackTree<unsigned int,struct SResendPackage *> *pFilePackage;
extern void *pSelfSocketAddress;//保存自己的ip地址
extern unsigned short wSourcePort;
extern unsigned short wDstPort;
extern PackageHandle packageHandle[];
extern  unsigned long long ullUserID;
extern  unsigned char aucPassword[];


void packageHandleInit();
void  ens33Destroy();
void regroupRegisterAndLogin(unsigned char *pSendBuffer,int *pLen,unsigned int uOperationCode,unsigned long long ullUserID,const char *pPassword);//组登录注册包`
void regropFilePackage(unsigned char *pFileBytes,int iFileBytes);//组文件包

unsigned int loginAndRegisterHandle(struct sIpData *pIPData);
unsigned int sackHandle(struct sIpData *pIPData);
unsigned int ackHandle(struct sIpData *pIPData);
void heartHopBack(unsigned char *pRecvbuffer,int iLen);

