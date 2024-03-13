#include"fun.h"
#include"header.h"
#include"invert.h"
#include"checksum.h"
#include <cstring>
 #include <net/if.h>// struct ifreq
 #include <sys/ioctl.h> // ioctl、SIOCGIFADDR
 #include <sys/socket.h> // socket
 #include <netinet/ether.h> // ETH_P_ALL
 #include <netpacket/packet.h> // struct sockaddr_ll


unsigned int uErrorNumber;//操作的错误码
int clientsocket;
struct SSocketSeq *pSocketSeq;
CRedBlackTree<unsigned int,struct SResendPackage *> *pFilePackage;
void *pSelfSocketAddress;//保存自己的ip地址

unsigned short wSourcePort=6666;
unsigned short wDstPort=8081;
unsigned int uSourceIP=0xc0A88180;//192.168.129.128
unsigned int uDstIP=0xc0A88181;///192.168.0.115
unsigned char aucSourceMac[6]={0x00,0x0c,0x29,0xe8,0x6c,0x29};
unsigned char aucDstMac[6]={0x00,0x0c,0x29,0xe0,0x13,0x65};

unsigned long long ullUserID;
unsigned char aucPassword[16];

PackageHandle packageHandle[Push]={0};

void regroupMacIP(struct SMACIPHeader *pMACIPHeader )
{
	memcpy(pMACIPHeader->stMACHeader.aucSourceMac,aucSourceMac,6);
	memcpy(pMACIPHeader->stMACHeader.aucArmMac,aucDstMac,6);
	pMACIPHeader->stMACHeader.wProtocol=0x0008;
	pMACIPHeader->stIPHeader.bitIPHeaderLen=5;
	pMACIPHeader->stIPHeader.bitIPVersion=4;
	pMACIPHeader->stIPHeader.ucDifferentiatServices=0;
	pMACIPHeader->stIPHeader.wIdentification=0;
	pMACIPHeader->stIPHeader.bitFlags=0;
	pMACIPHeader->stIPHeader.bitSliceOffset=0;
	pMACIPHeader->stIPHeader.ucProtocol=0xfe;
	pMACIPHeader->stIPHeader.ucTTL=0x80;
	pMACIPHeader->stIPHeader.wHeaderCheckSum=0;
	pMACIPHeader->stIPHeader.uSourceIP=invertInt(uSourceIP);
	pMACIPHeader->stIPHeader.uArmIP=invertInt(uDstIP);
}

void packageHandleInit()
{
	sll=new struct sockaddr_ll;
	req=new struct ifreq;
	packageHandle[Login]=loginAndRegisterHandle;
	packageHandle[Register]=loginAndRegisterHandle;
	packageHandle[SAck]=sackHandle;
	packageHandle[Ack]=ackHandle;

}
void ens33Destory()
{
	delete sll;
	delete req;
}


void regroupRegisterAndLogin(unsigned char *pSendBuffer,int *pLen,unsigned int uOperationCode,unsigned long long ullUserID,const char *pPassword)//组登录注册包
{
	struct sIpData *pstIPData=(struct sIpData *)(pSendBuffer+34);
	struct SMACIPHeader *pstMACIPHeader=(struct SMACIPHeader*)pSendBuffer;

	regroupMacIP(pstMACIPHeader);
	
	pstIPData->TcpHead.wSrcPort=wSourcePort;
	pstIPData->TcpHead.wDstPort=wDstPort;
	pstIPData->TcpHead. wEnumPackType= (enum EPackageType)uOperationCode;//服务器自我介绍

	pstIPData->UserHead.uUserOperation=(enum EOperationCode)uOperationCode;
	pstIPData->UserHead.ullUserId=ullUserID;
	if(uOperationCode==AskFTPUpLoadFile)
	{
		pstIPData->UserHead.wParametersLength=0;
		pstIPData->UserHead.pSocket=pSelfSocketAddress;//加上自己的地址

		pstMACIPHeader->stIPHeader.wLenTotal=invertShort(60);
		pstMACIPHeader->stIPHeader.wHeaderCheckSum=checkSum((unsigned short *)&pstMACIPHeader->stIPHeader,20);

		pstIPData->UserHead.wCheckWord=checkSum((unsigned short*)pstIPData,40);
		*pLen=74;
	}
	else
	{
		struct sLoginParameter *pstLoginParameter=(struct sLoginParameter *)(pSendBuffer+74);
		pstIPData->UserHead.wParametersLength=sizeof(struct sLoginParameter);
		memcpy(pstLoginParameter->uOperationParameter.ucPassword,pPassword,16);
		
		pstMACIPHeader->stIPHeader.wLenTotal=invertShort(60+sizeof(struct sLoginParameter));
		pstMACIPHeader->stIPHeader.wHeaderCheckSum=checkSum((unsigned short *)&pstMACIPHeader->stIPHeader,20);
		pstIPData->UserHead.wCheckWord=0;
		pstIPData->UserHead.wCheckWord=checkSum((unsigned short*)pstIPData,40+sizeof(struct sLoginParameter));
		*pLen=74+sizeof(struct sLoginParameter);
	}

}

//心跳包处理函数gg
void heartHopBack(unsigned char *pRecvbuffer,int iLen)
{
	int i=0;
	unsigned char tmp;
	for(i=0;i<6;i++)//mac
	{
		tmp=pRecvbuffer[i];
		pRecvbuffer[i]=pRecvbuffer[i+6];
		pRecvbuffer[i+6]=tmp;
	}
	for(i=26;i<30;i++)//ip
	{
		tmp=pRecvbuffer[i];
		pRecvbuffer[i]=pRecvbuffer[i+4];
		pRecvbuffer[i+4]=tmp;
	}
	//端口
	struct sIpData * pSIpData = (struct sIpData *)(pRecvbuffer+34);
	
	i=(pSIpData->TcpHead).wSrcPort;
	(pSIpData->TcpHead).wSrcPort = (pSIpData->TcpHead).wDstPort;
	(pSIpData->TcpHead).wDstPort = i;
	sendto(clientsocket,pRecvbuffer,iLen, 0 , (struct sockaddr *)sll, sizeof(struct sockaddr_ll));//发送心跳包

}



void regropFilePackage(unsigned char *pFileBytes,int iFileBytes)//组文件包
{
	struct SResendPackage *pResendPackage=new struct SResendPackage;
	unsigned char *pSendBuffer=new unsigned char[1500];

	
	//psendBUffer填写macip
	struct SMACIPHeader *pstMACIPHeader=(struct SMACIPHeader *)pSendBuffer;
	regroupMacIP(pstMACIPHeader);
	pstMACIPHeader->stIPHeader.wLenTotal=invertShort(iFileBytes+40+20);
	pstMACIPHeader->stIPHeader.wHeaderCheckSum=checkSum((unsigned short *)&pstMACIPHeader->stIPHeader,20);

	struct sIpData* pSIpData=(struct sIpData*)(pSendBuffer+34);
	(pSIpData->TcpHead).wSrcPort = wSourcePort;//源端口
	(pSIpData->TcpHead).wDstPort = wDstPort;//目的端口
	(pSIpData->TcpHead).wEnumPackType = ToFtp;//枚举包
	(pSIpData->TcpHead).wAckFirstPack = 0;//是否为ack后的第一个包
	
	pSocketSeq->uMaxRecvAck=pSocketSeq->uNextSendSeq;
	pSocketSeq->uNextReceiveSeq=pSocketSeq->uNextSendAck;//暂时不用

	(pSIpData->TcpHead).uSeqNumber = pSocketSeq->uNextSendSeq++;//序列号
	(pSIpData->TcpHead).uAckNumber = pSocketSeq->uNextSendAck++;//确认号

	(pSIpData->UserHead).wParametersLength = iFileBytes;//数据部分
	(pSIpData->UserHead).uUserOperation = Push;//操作码
	(pSIpData->UserHead).pSocket = pSelfSocketAddress;//socket地址
	(pSIpData->UserHead).ullUserId =ullUserID;//FTP服务器

	memcpy(pSendBuffer+74,pFileBytes,iFileBytes);
	
	(pSIpData->UserHead).wCheckWord =checkSum((unsigned short *)pSIpData,sizeof(struct sIpData)+iFileBytes);//数据校验

	
	pResendPackage->pPackage=pSendBuffer;
	pResendPackage->iPackageLen=74+iFileBytes;
	if(pFilePackage->insert(pSocketSeq->uMaxRecvAck,pResendPackage)!=0)
		printf("链表空间已用完\n");
	if(pSocketSeq->uSendPackageCount!=0)//可发送再发送
	{      
		 if(pSocketSeq->uSendPackageCount==pSocketSeq->uMaxSendPackageCount)
			(pSIpData->TcpHead).wAckFirstPack = 1;//是否为ack后的第一个包
		sendto(clientsocket,pSendBuffer,pResendPackage->iPackageLen=74+iFileBytes, 0 , (struct sockaddr *)sll, sizeof(struct sockaddr_ll));//发送上传文件的包
		pSocketSeq->uSendPackageCount--;
	}

}

unsigned int loginAndRegisterHandle(struct sIpData *pIPData)      
{
	if(pIPData->TcpHead.wEnumPackType==LoginSuccess)
		pSelfSocketAddress=pIPData->UserHead.pSocket;
	uErrorNumber=((struct sLoginParameter*)((unsigned char *)pIPData+40))->uOperationParameter.uErrorCode;
	return pIPData->UserHead.uUserOperation;//返回对应操作码

}
unsigned int sackHandle(struct sIpData *pIPData)
{
	int groups=(pIPData->UserHead.wParametersLength-24)/8;
	int temp=groups;//用于判断发送的第一包
	struct sFTPSendAck *pReceiveACK=(struct sFTPSendAck*)((unsigned char *)pIPData+40);
	struct SResendPackage *pstResendPackage;
	unsigned int uMinACK=pSocketSeq->uNextRecvAck;
	pSocketSeq->uMaxSendPackageCount--;//收到ACK减少最大发送包数量
	while(groups!=0)
	{
		unsigned int uMaxACK=pReceiveACK->uMinAckNumber;
		for(unsigned int i=uMinACK;i<uMaxACK;i++)
		{
			pstResendPackage=pFilePackage->find(i);
			if(pstResendPackage!=NULL)
			{
				if(temp==groups&&i==uMinACK)
				{
					struct SMACIPHeader *pPackage=(struct SMACIPHeader *)(pstResendPackage->pPackage);
					pPackage->stTCPHeader.wAckFirstPack=1;//收到ACK后的第一个包
				}
				sendto(clientsocket,pstResendPackage->pPackage,pstResendPackage->iPackageLen,0, (struct sockaddr *)sll, sizeof(struct sockaddr_ll));//发送注册包
				
			}
		}
		groups--;
		uMinACK=pReceiveACK->uMaxAckNumber;
		pReceiveACK=pReceiveACK+1;//下一个区间
	}
	return SAck;
}
unsigned int ackHandle(struct sIpData *pIPData)
{
	struct SResendPackage *pstResendPackage;
	if(pIPData->TcpHead.uAckNumber>=pSocketSeq->uNextRecvAck)
	{
		pSocketSeq->uNextRecvAck=pIPData->TcpHead.uAckNumber+1;
		pSocketSeq->uMaxSendPackageCount=(pSocketSeq->uMaxSendPackageCount)+1;//按滑动窗口指数增大
		pSocketSeq->uSendPackageCount=pSocketSeq->uMaxSendPackageCount;
		for(unsigned int i=pSocketSeq->uNextRecvAck;i<=pIPData->TcpHead.uAckNumber;i++)//删除收到的包
		{	
			if(pFilePackage!=NULL)//红黑树不为空
			{
				pstResendPackage=pFilePackage->deleteNode(i);//该包被确认，从红黑树中删除
				if(pstResendPackage!=NULL)//查找到了这个包
				{
					delete pstResendPackage->pPackage;
					delete pstResendPackage;
				}
			}

		}
		for(unsigned int i=pIPData->TcpHead.uAckNumber+1;i<pIPData->TcpHead.uAckNumber+pSocketSeq->uSendPackageCount;i++)
		{
			pstResendPackage=pFilePackage->find(i);
			if(pstResendPackage!=NULL)
			{
				if(i==pIPData->TcpHead.uAckNumber+1)
				{
					struct SMACIPHeader *pPackage=(struct SMACIPHeader *)(pstResendPackage->pPackage);
					pPackage->stTCPHeader.wAckFirstPack=1;//收到ACK后的第一个包
				}
				pSocketSeq->uSendPackageCount--;
				sendto(clientsocket,pstResendPackage->pPackage,pstResendPackage->iPackageLen,0, (struct sockaddr *)sll, sizeof(struct sockaddr_ll));//发送注册包
			}
		}
	
	}
	if(pIPData->TcpHead.uAckNumber==pSocketSeq->uMaxRecvAck)
	{
		delete pFilePackage;
		pFilePackage=0;
		return FTPFileOver;
	}
	return Ack;
}
