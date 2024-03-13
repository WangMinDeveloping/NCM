#include"sUserSock.h"
#include"scCariableip.h"
#include"IpPack.h"
#include <time.h>
#include<string.h>
#include<stdio.h>
//原始套接字
#include<sys/socket.h>
#include<net/if.h>
#include<sys/ioctl.h>
#include<netinet/ether.h>
#include<netpacket/packet.h>
#include<arpa/inet.h>
extern struct sockaddr_ll sllAddr;//原始套接在地址结构体
extern int iSocketRaw;//原始套接字
extern unsigned int sIpDataLen;//自己协议长度
CUserSock::CUserSock()
{
	uTimer=0;
	uLastTimer=0;
	uRecvWinLeftSeq=-1;
	uRecvWinRightSeq=1;
	pRecvPackTree.iChick=1;
	memset(&evp,0,sizeof(struct sigevent));
	evp.sigev_value.sival_ptr=this;
	evp.sigev_notify=SIGEV_THREAD;
	evp.sigev_notify_function=AckSackSendFunction;
	timer_create(CLOCK_REALTIME,&evp,&sTimerId);
}

void CUserSock::AckSackSendFunction(union sigval v)
{
	int i=0;
	CUserSock* data=(CUserSock*)v.sival_ptr;
	asm("ack1:lock btsl $1,%0;\
		jc ack1;"\
		:"+m"(data->iLock));
	unsigned char ucSendData[1514]={0};
	loop:memset(ucSendData,00,1514);
	data->uRecvWinLeftSeq=data->uRecvWinRightSeq;
	CopyMacIpHead(ucSendData);//mac,ip头
	SelfIntroductionPackage(ucSendData+34);//tcp头部和user头部
	
	struct sIpData * pSIpData = (struct sIpData *)(ucSendData+34);
	(pSIpData->TcpHead).wEnumPackType = ToUser;//枚举包

	(pSIpData->TcpHead).uSeqNumber = data->uSeqCount;//序列号
	(pSIpData->TcpHead).uAckNumber = data->uAckCount;//确认号

	(pSIpData->UserHead).pSocket = data->sSocket;//socket地址
	(pSIpData->UserHead).ullUserId = data->ullUserId;//用户id
	//printf("sdfgasdg\n");
	int count=34+sIpDataLen;
	if(data->pRecvPackTree.NodeCount!=1&&data->pRecvPackTree.iChick!=0)
	{
		data->pRecvPackTree.pDoublyListRoot.resetiteratornode();//重置遍历节点的位置
		for(i=0;i<4;i++)
		{
			SDoublyListNode* pDoublyData=data->pRecvPackTree.pDoublyListRoot.iteratordata();
			if(pDoublyData==NULL)
				break;
			SRedBlackTreeNode* pCData=(SRedBlackTreeNode*)(pDoublyData->pStudent);
			//printf("}}}}}}}}}}}}%d %d}}}}}}}}}}}}}}}}}\n",pCData->pKey->Key.llSeqnumber,pCData->pKey->Key.llAcknumber);
			*(unsigned int*)(ucSendData+count)=pCData->pKey->Key.llSeqnumber;
			count+=4;
			*(unsigned int*)(ucSendData+count)=pCData->pKey->Key.llAcknumber;
			count+=4;
		}
		//printf("5\n");
		(pSIpData->UserHead).uUserOperation = SAck;//操作码
	}
	else
	{
		//printf("6\n");
		//printf("|||||||||||||||||||%d|||||||||||||||\n",data->uAckCount);
		(pSIpData->UserHead).uUserOperation = Ack;//操作码
	}
	(pSIpData->UserHead).wParametersLength = i*8;//数据部分
	(pSIpData->TcpHead).usLen=htons(sIpDataLen+i*8);
	(pSIpData->UserHead).wCheckWord = myChecksum((unsigned short*)(&((pSIpData->TcpHead).uSeqNumber)),count-38);//校验
	*(unsigned short*)(ucSendData+16)=htons(count-14);//ip包数据长度
	*(unsigned short*)(ucSendData+24)=myChecksum((unsigned short*)(ucSendData+14),20);//ip头校验
	i=sendto(iSocketRaw,ucSendData,count,0,(struct sockaddr*)&sllAddr,sizeof(sllAddr));
	if(i==-1)
		printf("error\n");
	goto loop;
	asm("lock btrl $1,%0;"\
		:"+m"(data->iLock));	
}
