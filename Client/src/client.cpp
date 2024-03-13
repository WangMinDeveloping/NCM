#include<stdio.h>
#include <sys/socket.h>
#include <netinet/ether.h>
#include<sys/stat.h>
#include<errno.h>
#include<netdb.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<string.h>
#include <sys/select.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<bits/select.h>
#include <sys/ioctl.h>
#include<linux/sockios.h>
#include <time.h>  
#include <sys/time.h>  
#include <stdlib.h>  
#include <signal.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <net/if.h>// struct ifreq
#include <sys/ioctl.h> // ioctl、SIOCGIFADDR
#include <sys/socket.h> // socket
#include <netinet/ether.h> // ETH_P_ALL
#include <netpacket/packet.h> // struct sockaddr_ll

#include "invert.h"
#include"header.h"
#include"fun.h"
#include"checksum.h"
using namespace std;
struct sockaddr_ll *sll;  
struct ifreq *req;        

static struct itimerval oldtv;  

unsigned char reSendBuffer[1500]={0xff};//重发包
unsigned int uResendPackageLen=0;

//定时函数
int recvFunction();
int registerAndLogin(enum EOperationCode uOpearation);
void uploadFile();


void signal_resend(int i)
{	
	int len;
	if(reSendBuffer[0]!=0xff)
	{
		len = sendto(clientsocket,reSendBuffer,uResendPackageLen, 0 , (struct sockaddr *)sll, sizeof(struct sockaddr_ll));
		if(len == -1)
		{
			printf("resendto fail:\n");

		}
		else printf("resend success\n");
	}
}

int main()
{

	//struct timeval timeout={0,0}; 
	struct itimerval itv;  
	itv.it_interval.tv_sec = 30;  
	itv.it_interval.tv_usec = 0;  
	itv.it_value.tv_sec = 30;  
	itv.it_value.tv_usec = 0;  
	int flag;

	packageHandleInit();//初始化函数处理数组
	if((clientsocket=socket(PF_PACKET,SOCK_RAW,htons(ETH_P_IP)))<0)//创建原始套接字
	{  
		perror("socket");
		return 1;
	}

	strncpy(req->ifr_name, "ens33", IFNAMSIZ);			//指定网卡名称
	if(-1 == ioctl(clientsocket, SIOCGIFINDEX, req))	//获取网络接口
	{
		perror("ioctl");
		close(clientsocket);
		exit(-1);
	}

	/*将网络接口赋值给原始套接字地址结构*/
	bzero(sll, sizeof(struct sockaddr_ll));
	sll->sll_ifindex = req->ifr_ifindex;


	setitimer(ITIMER_REAL, &itv, &oldtv); 
	signal(SIGALRM,signal_resend);
	while(1)
	{
		printf("*****************1.register********2.login*******3.uploadFile*******4.quit*********5.loginServer*******6.FTPserver*****7.User**************\n");
		scanf("%d",&flag);
		switch(flag)
		{
			case 1:
				registerAndLogin(Register);
				break;
			case 2:
				registerAndLogin(Login);
				break;
			case 3:
				uploadFile();
				break;
			case 4:
				close(clientsocket);
				break;
			case 5:
				registerAndLogin((enum EOperationCode)ServerSelfIntroduction);
				break;
			case 6:
				registerAndLogin((enum EOperationCode)ServerSelfIntroduction);
				break;
			case 7:
				registerAndLogin((enum EOperationCode)ToUser);
				break;
			default:
				break;
		}
	}
	ens33Destroy();//删除网卡结构体空间
	return 0;

}

void uploadFile()
{
	char fileName[100];
	int fd;
	int iFileBytes;//文件操作返回值
	unsigned char aucSendFileBytes[1024]={};
	//int iTotalLen;//包总长，包括MAC.IP部分
	printf("请输入文件名\n");
	scanf("%s",fileName);
	fd=open(fileName,O_CREAT,O_RDWR);
	if(fd<0)
		printf("文件打开错误\n");

	/*regroupRegisterAndLogin(aucSendFileBytes,&iTotalLen,AskFTPUpLoadFile,ullUserID,0);//请求上传文件
	memcpy(reSendBuffer,aucSendFileBytes,iTotalLen);//用作重发
	uResendPackageLen=iTotalLen;
	sendto(clientsocket,aucSendFileBytes,iTotalLen, 0 , (struct sockaddr *)sll, sizeof(struct sockaddr_ll));//发送上传文件的包
	while(1)
	{
		if(Ack==recvFunction())
		{
			if(uErrorNumber==ECodeUpLoadFileAllow)
				reSendBuffer[0]=0xff;//取消重发
			break;
		}
	}*/
	pSocketSeq=new struct SSocketSeq;//创建socket，保证文件传输顺序

	pSocketSeq->uNextSendSeq=1;
	pSocketSeq->uNextSendAck=1;
	pSocketSeq->uMaxSendPackageCount=2;//最开始设置一次性可以发两个包，收到ack一直增大
	pSocketSeq->uSendPackageCount=2;//当前可以发送的数据包数量

	pSocketSeq->uNextRecvAck=1;

	pFilePackage=new CRedBlackTree<unsigned int,struct SResendPackage *>();//链表存储文件数据包，便于重发
	while(1)
	{
		iFileBytes=read(fd,aucSendFileBytes,sizeof(aucSendFileBytes));
		if(iFileBytes<0) printf("读文件错误\n");
		regropFilePackage(aucSendFileBytes,iFileBytes);//组文件包，记得该函数需要将包放入红黑树 并且发送
		if(iFileBytes!=sizeof(aucSendFileBytes))//文件读完了
			break;
	}

	while(1)
	{
		if(FTPFileOver==recvFunction())
		{
			printf("文件传输结束\n");
			break;
		}

	}
}

int registerAndLogin(enum EOperationCode uOperation)
{	
	unsigned int iType;//标识处理包的类型
	unsigned char sendbuffer[1500]={0};
	int iLen;
	printf("请输入账号\n");
	scanf("%lld",&ullUserID);
	printf("请输入密码\n");
	scanf("%s",aucPassword);

	regroupRegisterAndLogin(sendbuffer,&iLen,uOperation,ullUserID,(const char *)aucPassword);
	memcpy(reSendBuffer,sendbuffer,iLen);//用作重发
	uResendPackageLen=iLen;
	sendto(clientsocket,sendbuffer,iLen, 0 , (struct sockaddr *)sll, sizeof(struct sockaddr_ll));//发送注册包
	while(1)
	{
		iType=recvFunction();
		if(uOperation==iType)//返回当前函数的包处理
		{
			if(uOperation==Login)
			{
				if(uErrorNumber== ECodeLoginSuccess)
					printf("登录成功\n");
				if(uErrorNumber== ECodeLoginFail)
					printf("登录失败\n");
			}
			else if(uOperation==Register)
			{
				if(uErrorNumber== ECodeRegisterSuccess)
					printf("注册成功\n");
				if(uErrorNumber== ECodeRegisterFail)
					printf("注册失败\n");
			}
			reSendBuffer[0]=0xff;//取消重发
			break;
		}
	}
	return 0;
}


/*返回值 enum EOperationCode*/
int recvFunction()
{
	unsigned int iType;//标识处理包的类型
	unsigned char recvbuffer[1500];
	while(1)
	{
		int len = recvfrom(clientsocket, recvbuffer, sizeof(recvbuffer), 0, NULL, NULL);
		if(recvbuffer[23]==0xfe)//我的协议254
		{
			struct sIpData *pstIPData=(struct sIpData *)(recvbuffer+34);
			unsigned short wCheckSum=checkSum((unsigned short *)pstIPData,40+pstIPData->UserHead.wParametersLength);//计算校验和
			if(wCheckSum==0)
			{
				if(pstIPData->TcpHead.wEnumPackType!=HeartHop)
				{
					if(pstIPData->UserHead.uUserOperation>=Login&&pstIPData->UserHead.uUserOperation<=Push)
					{
						if(packageHandle[pstIPData->UserHead.uUserOperation]!=NULL)
						{
							iType=packageHandle[pstIPData->UserHead.uUserOperation](pstIPData);
							return iType;
						}
					}
				}
				else heartHopBack(recvbuffer,len);
			}
		}

	}

}



