#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
//定时器头文件
#include<sys/time.h>
#include<signal.h>
//socket头文件
//普通套接字
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/ip.h>
#include<netinet/in.h>
//原始套接字
#include<net/if.h>
#include<sys/ioctl.h>
#include<netinet/ether.h>
#include<netpacket/packet.h>
//线程头文件
#include<pthread.h>
//各类定义头文件
#include"scCariableip.h"//各个结构体的类型
#include"sUserSock.h"//用户对象
#include"SRedBlackTreeNode.h"//红黑树节点
#include"IpPack.h"//组包

#define uPackLen 1600//包长度
//线程池，变量红黑树,多线程队列
//#include"rbNodeValueOnly.h"
//#include"memorypool.h"

volatile unsigned long long ullTimer=0;//程序运行的计时器

int iSocketRaw=0;//原始套接字

//CMemoryPool cUserSendData(100,uPackLen);//100个包，每个包1064字节
//CMemoryPool cUserSocket(100,sizeof(CUserSock));//100个用户的对象
CUserSock cUser;//用户对象
unsigned char ucRecvData[uPackLen]={0};//接受的数组
unsigned char ucSendData[uPackLen]={0};//发送的数组
unsigned int uiUdpPort;//自己的端口号
unsigned int uiGetWayPort;//网关的端口
unsigned int uiSrcIpAddr[4];//自己的ip地址
unsigned int uiDstIpAddr[4];//网关的ip地址
void* psSocketAddr=NULL;//自己的网关地址
struct sockaddr_ll sllAddr;//原始套接在地址结构体
struct ifreq req;//网络接口地址
//CRedBlackTree<> cUserTable;//用户id建立的红黑树表
void TimerSaveFunction(int signo)
{
	ullTimer++;
}
void ZonePackFunction(struct sIpData* sGetData)//,CUserSock* cUser)//修改时打开，通过遍历红黑树后得到用户的结构体
{
	CData* pKey=new CData;
	pKey->Key.llSeqnumber=sGetData->TcpHead.uSeqNumber;
	pKey->Key.llAcknumber=sGetData->TcpHead.uSeqNumber;
	char* pData=new char[sGetData->UserHead.wParametersLength]();
	char* pSrc=(char*)(sGetData+1);
	memcpy(pData,pSrc,sGetData->UserHead.wParametersLength);
	 
	cUser.uAckCount=sGetData->TcpHead.uSeqNumber;
	cUser.uSeqCount=sGetData->TcpHead.uAckNumber;
	SRedBlackTreeNode* pTreeNode=cUser.pRecvPackTree.addNode(pKey,pData);
	if(pKey->Key.llSeqnumber==cUser.uRecvWinRightSeq)
	{
		cUser.uRecvWinRightSeq=pTreeNode->pKey->Key.llAcknumber+1;
		cUser.pRecvPackTree.iChick=0;
	}
	else
	{
		cUser.pRecvPackTree.iChick=1;
	}
	if(pTreeNode==NULL)
		delete pData;
	else
	{
		//printf("%d++++++++++++++++++-------------\n",sGetData->TcpHead.uSeqNumber);
		//cUser.pRecvPackTree.printfFunction();
	}
	printf("%d--------------------------\n",sGetData->TcpHead.uSeqNumber);
	//cUser.pRecvPackTree.printfFunction();
	//cUser.pRecvPackTree.pDoublyListRoot.resetiteratornode();
	//cUser.pRecvPackTree.pDoublyListRoot.iteratorprintf();
}

void UpLoadFileFunction(struct sIpData* sGetData)//,CUserSock* cUser)//修改时打开，通过遍历红黑树后得到用户的结构体
{
	//用户基本数据
	cUser.uAckCount=sGetData->TcpHead.uSeqNumber;
	cUser.uSeqCount=sGetData->TcpHead.uAckNumber;
	cUser.sSocket=sGetData->UserHead.pSocket;
	cUser.ullUserId=sGetData->UserHead.ullUserId;
	cUser.uRecvWinLeftSeq=1;
	//定时
	cUser.tick.it_value.tv_sec=3;
	cUser.tick.it_value.tv_nsec=0;
	cUser.tick.it_interval.tv_sec=0;
	cUser.tick.it_interval.tv_nsec=0;
	timer_settime(cUser.sTimerId,0,&cUser.tick,NULL);
	//回包组合
	/*cUserTable.insert(sGetData->ulUserId,&cUser);*///建立用户套节奏并放入红黑树，修改为改为内存池取
	sGetData->TcpHead.uSeqNumber=cUser.uSeqCount;
	sGetData->TcpHead.uAckNumber=cUser.uAckCount;
	sGetData->TcpHead.wEnumPackType=ToUser;
	sGetData->TcpHead.usLen=htons(sIpDataLen+4);
	//sGetData->UserHead.uUserOperation=Ack;
	sGetData->UserHead.wParametersLength=4;
	*(int*)(sGetData+1)=ECodeUpLoadFileAllow;
	(sGetData->UserHead).wCheckWord=0;
	(sGetData->UserHead).wCheckWord = myChecksum((unsigned short *)(&sGetData->TcpHead.uSeqNumber),sIpDataLen+sGetData->UserHead.wParametersLength-8);//计算校验
}

void* PackManageFunction()//多线程recfrom会出现问题，并其该服务器没有使用多线程队列
{
	//void* pUserTableNode=NULL;
	struct sIpData* sGetData=NULL;
	int len=34+sIpDataLen;
	printf("123\n");
	while(1)
	{
		memset(ucRecvData, 0, uPackLen);//清空存留消息
		memset(ucSendData, 0, uPackLen);//清空存留消息
		//printf("123\n");
		recvfrom(iSocketRaw,ucRecvData,uPackLen,0,NULL,NULL);
		//sendto(iSocketRaw,ucSendData,len,0,(struct sockaddr*)&sllAddr,sizeof(sllAddr));
		sGetData=(struct sIpData*)(ucRecvData+34);
		if((sGetData->TcpHead).wDstPort != htons(uiUdpPort))//判断接受到的端口是否等于自己的端口
		{
			continue;
		}
		if(sGetData->TcpHead.wEnumPackType==HeartHop)//心跳包
		{
			printf("heartHop!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			len=34+sIpDataLen;
			XchangeIpPort(ucRecvData);
			memcpy(ucSendData,ucRecvData,len);
			sendto(iSocketRaw,ucSendData,len,0,(struct sockaddr*)&sllAddr,sizeof(sllAddr));
			continue;
		}
		if(myChecksum((unsigned short *)(&(sGetData->TcpHead).uSeqNumber),34+sGetData->UserHead.wParametersLength))
		{
			printf("len************%d ",sGetData->UserHead.wParametersLength);
			printf("%02x ",sGetData->UserHead.wCheckWord);
			printf("%d\n",myChecksum((unsigned short *)(&(sGetData->TcpHead).uSeqNumber),34+sGetData->UserHead.wParametersLength));
			printf("check\n");
			continue;
		}
		//pUserTableNode=cUserTable.find(sGetData->ulUserId);//查找是否有用户，后面cUser改为查找到的节点
		asm("ack:lock btsl $1,%0;\
		jc ack;"\
		:"+m"(cUser.iLock));
		//printf("getdata\n");
		if(sGetData->TcpHead.uSeqNumber==cUser.uRecvWinLeftSeq)
		{
			printf("1--------------------------------------\n");
			cUser.uTimer=(ullTimer-cUser.uLastTimer+cUser.uTimer+1)/2;
			while(cUser.uTimer>=10)
			{
				cUser.uTimer=cUser.uTimer/10;
			}
			printf("timer:%u\n",cUser.uTimer);
			cUser.uTimer=cUser.uTimer%3;
			cUser.uTimer+=1;
			cUser.uLastTimer=ullTimer;
			cUser.tick.it_value.tv_sec=0;
			cUser.tick.it_value.tv_nsec=1;
			cUser.tick.it_interval.tv_sec=0;
			cUser.tick.it_interval.tv_nsec=0;//100000000*cUser.uTimer;
			timer_settime(cUser.sTimerId,0,&cUser.tick,NULL);
		}
		if(sGetData->UserHead.uUserOperation==Push)
		{
			printf("2--------------------------------------\n");
			ZonePackFunction(sGetData);
		}
		else if(sGetData->UserHead.uUserOperation==AskFTPUpLoadFile)
		{
			//printf("3-----------------------\n");
			//if(pUserTableNode==NULL)
				UpLoadFileFunction(sGetData);
			len=34+sIpDataLen+4;
			memcpy(ucSendData,ucRecvData,len);
			XchangeIpPort(ucSendData);
			cUser.uLastTimer=ullTimer;
			*(unsigned short*)(ucSendData+24)=0;
			*(unsigned short*)(ucSendData+16)=htons(20+sIpDataLen+4);//ip包长度
			*(unsigned short*)(ucSendData+24)=myChecksum((unsigned short*)(ucSendData+14),20);//ip头校验
			sendto(iSocketRaw,ucSendData,len,0,(struct sockaddr*)&sllAddr,sizeof(sllAddr));
		}
		else if(sGetData->UserHead.uUserOperation==FTPFileOver)
		{
			timer_delete(cUser.sTimerId);
			cUser.pRecvPackTree.printfFunction();
			printf("end\n");
		}
		asm("lock btrl $1,%0;"\
		:"+m"(cUser.iLock));
	}
	return 0;
}

int main(int argc,char* argv[])
{
	if(argc<6)
	{
		printf("please input computer srcport , dstport , srcip address , dst ip address and thread number!\n");
		return -1;
	}
	printf("%d\n",sIpDataLen);
	//unsigned char ucSendRecvData[uPackLen]={0};//此线程发包和收包数组
	int i=0;
	int len=0;//发包的长度
	unsigned long long Timer=0;//时间中间变量
	uiUdpPort=atoi(argv[1]);//源端口
	uiGetWayPort=atoi(argv[2]);//目的端口
	sscanf(argv[3],"%u.%u.%u.%u",&uiSrcIpAddr[0],&uiSrcIpAddr[1],&uiSrcIpAddr[2],&uiSrcIpAddr[3]);//源ip
	sscanf(argv[4],"%u.%u.%u.%u",&uiDstIpAddr[0],&uiDstIpAddr[1],&uiDstIpAddr[2],&uiDstIpAddr[3]);//目的ip
	PutIpAddr();
	
	//网关套节字建立
	int iOccupyPort=socket(AF_INET,SOCK_DGRAM,0);//占用端口套接字
	struct sockaddr_in sUdpRecvAddr;
	sUdpRecvAddr.sin_family=AF_INET;
	sUdpRecvAddr.sin_port=htons(uiUdpPort);
	sUdpRecvAddr.sin_addr.s_addr=inet_addr(argv[3]);
	if(bind(iOccupyPort,(struct sockaddr*)&sUdpRecvAddr,sizeof(sUdpRecvAddr))<0)
	{
		perror("bind error");
		close(iOccupyPort);
		exit(0);
	}
	
	iSocketRaw=socket(PF_PACKET,SOCK_RAW,htons(ETH_P_IP));//原始套接字
	if(iSocketRaw<0)
	{
		perror("socket");
		exit(0);
	}
	strncpy(req.ifr_name,"ens33",IFNAMSIZ);//指定网卡名称
	if(-1==ioctl(iSocketRaw,SIOCGIFINDEX,&req))
	{
		perror("ioctl\n");
		close(iSocketRaw);
		exit(0);
	}
	bzero(&sllAddr,sizeof(sllAddr));
	sllAddr.sll_ifindex=req.ifr_ifindex;
	
	//程序运行记时
	signal(SIGALRM,TimerSaveFunction);
	struct itimerval tim;
	tim.it_value.tv_sec=0;
	tim.it_value.tv_usec=1;
	tim.it_interval.tv_sec=0;
	tim.it_interval.tv_usec=10000;
	setitimer(ITIMER_REAL,&tim,NULL); 
	
	//网关注册
	CopyMacIpHead(ucSendData);//mac,ip头
	SelfIntroductionPackage(ucSendData+34);//tcp头部和user头部
	(((struct sIpData *)(ucSendData+34))->TcpHead).usLen=htons(sIpDataLen);
	(((struct sIpData *)(ucSendData+34))->UserHead).wCheckWord = myChecksum((unsigned short *)(&(((struct sIpData *)(ucSendData+34))->TcpHead).uSeqNumber),sIpDataLen-8);//计算校验
	*(unsigned short*)(ucSendData+16)=htons((20+sIpDataLen)&0xffff);//ip包长度
	*(unsigned short*)(ucSendData+24)=myChecksum((unsigned short*)(ucSendData+14),20);//ip头校验
	len=34+sIpDataLen;
	printf("ret=%d\n",myChecksum((unsigned short *)(&(((struct sIpData *)(ucSendData+34))->TcpHead).uSeqNumber),sIpDataLen-8));
	//给网关发包
	sendto(iSocketRaw,ucSendData,len,0,(struct sockaddr*)&sllAddr,sizeof(sllAddr));
	Timer=ullTimer;
	loop:recvfrom(iSocketRaw,ucRecvData,uPackLen,0,NULL,NULL);
	if((((struct sIpData*)(ucRecvData+34))->TcpHead).wDstPort != htons(uiUdpPort))//判断接受到的端口是否等于自己的端口
	{
		printf("recfrom outtime\n");
		if(ullTimer-Timer>300000)
		{
			i++;
			if(i==4)
			{
				printf("connect link!\n");
				exit(0);
			}
			sendto(iSocketRaw,ucSendData,len,0,(struct sockaddr*)&sllAddr,sizeof(sllAddr));
			Timer=ullTimer;
		}
		goto loop;
	}

	psSocketAddr=(((struct sIpData*)(ucRecvData+34))->UserHead).pSocket;//socket地址*/
	//包处理线程建立
	/*pthread_t thread[uThreadCount];
	for(i=0;i<uThreadCount;i++)
	{
		pthread_create(&thread[i],NULL,PackManageFunction,NULL);
	}*/
	//sleep(1);
	PackManageFunction();
	close(iOccupyPort);
	close(iSocketRaw);
	return 0;
}
