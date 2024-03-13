#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#include <errno.h>
#include <sys/time.h>

#include <net/if.h>		//struct ifreq
#include <sys/ioctl.h>		//ioctl,SIOCGIFADDR
#include <netinet/ether.h>	//ETH_P_ALL
#include <netpacket/packet.h>	//struct sockaddr_ll

#include <signal.h>
#include <time.h>

#include "LoginServer.h"
#include "analysis.h"
#include "structAndEnum.h"
#include "toolFunc.h"
#include "tcpIpFunc.h"
int sock_raw_fd;
//2.根据各种协议首部格式构建发送数据报
unsigned char buf[1500] = {0};//buf[1500] = {0};//buf[1514] = {0};
unsigned char send_msg[1500] = {//send_msg[1514] = {
	//--------------组MAC--------14------
	0x00,0x50,0x56,0xe2,0x85,0x24,	//dst_mac:00 50 56 e2 85 24//路由器
//	0x00, 0x0c, 0x29, 0x46, 0x90, 0x68,	//dst_mac:{0x00, 0x0c, 0x29, 0x46, 0x90, 0x68}对方主机
	0x00,0x0c,0x29,0xd2,0x39,0x33,	//src_mac:本机 00:0c:29:d2:39:33

	0x08,0x00};				//类型：0x0800 IP协议
unsigned int srcMacBigEndianPart1 = 0;
unsigned short srcMacBigEndianPart2 = 0;

volatile int introduceResendCount = 4;
volatile int exitFlag = 1;

//小端方式的IP值
unsigned int srcIp = 0xC0A82A88;//3232246408;//192.168.42.136 192*2^24 + 168*2^16 + 42*2^8 + 136//192*16777216 + 168*65536 + 42*256 + 136
unsigned int dstIp = 0x2d4dfedf;//du 网关//0xC0A82A8A;//3232246410;//192.168.42.138//192*16777216 + 168*65536 + 42*256 + 138
unsigned short srcPort = 8080;
unsigned short dstPort = 8081;//8081;//
volatile void * loginServerSocketAddr = 0;//网关中注册登陆注册服务器的socket地址

/////////////////////
volatile char introduceStartFlag = 1;//自我介绍包开关。0关1开
/////发送数据准备/////
struct sockaddr_ll sll;//原始套接字地址结构
struct ifreq req;//网络接口地址
int len = 0;

struct LoginServerSend * pSLSS = 0;

unsigned int changeTemp = 0;
unsigned short changeTemp2 = 0;
unsigned short packTotalLen = 34+sizeof(struct LoginServerSend);
unsigned short ipPackTotalLenBigEndian = 0;
unsigned short packTotalLenBigEndian = 0;
ssize_t recvLength = 0;
unsigned int checksumJudge = 0;
int TCPData_analysis_retNum = 0;//存储tcp分析函数的返回值

void introduceResendFunc(union sigval v);//自我介绍重发定时回调函数
void commonUpdata(unsigned char * buf);//公共部分修改函数
void LoginPackUpdata(unsigned char * buf);//登陆注册逻辑处理函数

int main(int argc, char *argv[])
{

	//记录大端mac对应的小端数值
	srcMacBigEndianPart1 = *((unsigned int *)(send_msg+6));
	srcMacBigEndianPart2 = *((unsigned short *)(send_msg+10));

	//定时器部分
	struct itimerspec it;
	timer_t introduceTimerid;
	struct sigevent evp;

/*	//占用端口
	int sock=socket(AF_INET,SOCK_DGRAM,0);
	struct sockaddr_in local;
	local.sin_family=AF_INET;//ipv4
	local.sin_port=srcPort;//主机字节序转换为网络字节序
	local.sin_addr.s_addr=inet_addr("192.168.42.136");//字符串转in_addr的函数
	if(bind(sock,(struct sockaddr*)&local,sizeof(local))<0) {perror("bind");return 3;}*/

////////////////////////////////////////////

	//创建通信用的原始套接字
	sock_raw_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));//ETH_P_IP
	if(-1 == sock_raw_fd){perror("socket");close(sock_raw_fd);exit(-1);}

	pSLSS = (struct LoginServerSend *)(buf+34);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//发送数据准备

	strncpy(req.ifr_name, "ens33", IFNAMSIZ);			//指定网卡名称
	if(-1 == ioctl(sock_raw_fd, SIOCGIFINDEX, &req))	//获取网络接口
	{perror("ioctl");close(sock_raw_fd);exit(-1);}

	//网卡设置混杂模式
	req.ifr_flags |= IFF_PROMISC;
	if(-1 == ioctl(sock_raw_fd, SIOCGIFINDEX, &req))	//网卡设置混杂模式
	{
		perror("ioctl");
		close(sock_raw_fd);
		exit(-1);
	}

	/*将网络接口赋值给原始套接字地址结构*/
	bzero(&sll, sizeof(sll));
	sll.sll_ifindex = req.ifr_ifindex;

	len = sendto(sock_raw_fd, send_msg, 14+20+8+len, 0 , (struct sockaddr *)&sll, sizeof(sll));
	if(len == -1){perror("sendto");}


/////////////////////自我介绍定时重发///////////////////////
    if(introduceStartFlag)
    {//自我介绍重发开启开关。

	memset(&evp, 0, sizeof(struct sigevent));       //清零初始化

	evp.sigev_value.sival_ptr = &introduceTimerid;                //也是标识定时器的，回调函数可以获得
	evp.sigev_notify = SIGEV_THREAD;                //线程通知的方式，派驻新线程
	evp.sigev_notify_function = introduceResendFunc;       //线程函数地址


/*
函数：
int timer_create(clockid_t clockid, struct sigevent * sevp,  timer_t * timerid);
	创建一个POSIX标准的进程定时器
参数：
     @clockid 可选系统系统的宏，比如 CLOCK_REALTIME
     @sevp 环境值，结构体struct sigevent变量的地址
     @timerid 定时器标识符，结构体timer_t变量的地址
     link with -lrt.
返回值：
0 - 成功；-1 - 失败，errno被设置。
*/
	//定时器
	if (timer_create(CLOCK_REALTIME, &evp, &introduceTimerid) == -1)
	{perror("fail to timer_create");exit(-1);}

    /* 第一次间隔it.it_value这么长,以后每次都是it.it_interval这么长,就是说it.it_value变0的时候会>装载it.it_interval的值 */
	//struct itimerspec it;
	it.it_interval.tv_sec = 1;  // 回调函数执行频率为1s运行1次
	it.it_interval.tv_nsec = 0;
	it.it_value.tv_sec = 3;     // 倒计时3秒开始调用回调函数
	it.it_value.tv_nsec = 0;

////发送自我介绍包
	//包长度
	len = 34+sizeof(struct sIpData);
	//组ip头
	IPHeadSetting((unsigned char * )(send_msg+14),len-14);
	//组协议头
	PackageMake((unsigned char * )(send_msg+34),0,ServerSelfIntroduction);

	len = sendto(sock_raw_fd, send_msg,len,0,(struct sockaddr *)&sll,sizeof(sll));
	if(len == -1){printf("165行\n");perror("sendto");}


/*
timer_settime 参数：
	参数1：timerid 定时器标识
	参数2：flags 0标识相对时间，1标识绝对时间
	参数3：new_value 定时器的新初始值和间隔，如下面的it
	参数4：old_value 取值通常为0或NULL，若不为NULL，则返回定时器前一个值
*/

	if (timer_settime(introduceTimerid, 0, &it, NULL) == -1)
	{perror("fail to timer_settime");exit(-1);}
/////如果接收到了包就开始/////
	len = recvfrom(sock_raw_fd, buf, sizeof(buf), 0, NULL, NULL);
	while(introduceResendCount)
	{
		if(len>0)
		{
			if( srcPort == invertWord(*((unsigned short *)(buf+36))) )
			{
				unsigned int checksumJudge = 0;
				//ip校验和 和 用户层校验和在2字节以内是0
				checksumJudge = myChecksum((unsigned short *)(buf+14),20) + myChecksum((unsigned short *)(buf+34+8),42+pSLSS->sIPD.UserHead.wParametersLength-8);
				if(!checksumJudge&&0x11==buf[23])//协议17
				{
					introduceResendCount = 0;//取消自我介绍重发。
					exitFlag = 0;
					//记录登陆服务器自己的socket地址
					loginServerSocketAddr = ((struct sUserProtocol *)(buf+34+sizeof(struct sTcpHead)))->pSocket;
					printf("网关收到自我介绍。\n");
				}
			}
		}
	}

	//如果未受到网关的回应则退出。
	if(exitFlag){printf("未收到网关的回应。\n");close(sock_raw_fd);exit(-1);}//整合网关时要打开。

	memset(buf,0,sizeof(buf));

    }
/////////////////////开始实际收发/////////////////////////
	ipPackTotalLenBigEndian = invertWord(packTotalLen-14);
	packTotalLenBigEndian = invertWord(packTotalLen);//(packTotalLen>>8)+(packTotalLen<<8);

	while(1)
	{

		recvLength = recvfrom(sock_raw_fd, buf, sizeof(buf), 0, NULL, NULL);

		if(recvLength>0&&(srcPort == invertWord(pSLSS->sIPD.TcpHead.wDstPort)))
		{
			//ip校验和 和 用户层校验和在2字节以内是0
			checksumJudge = myChecksum((unsigned short *)(buf+14),20) + myChecksum((unsigned short *)(buf+34+8),42+pSLSS->sIPD.UserHead.wParametersLength-8);
			if(0x11==buf[23])//协议17
			{
				commonUpdata(buf);//处理公共部分的函数
				
				if(HeartHop == pSLSS->sIPD.TcpHead.wEnumPackType)
				{
					//心跳包的处理//长度 用户层校验和不变
					//len = 34+sizeof(struct sIpData);
					printf("心跳包\n");
					//添加ip校验
					*((unsigned short *)(buf+24)) = myChecksum((unsigned short *)(buf+14),20);
					sendto(sock_raw_fd,buf,recvLength,0,(struct sockaddr *)&sll,sizeof(sll));
				}

				//ip校验和 和 用户层校验和都为0时
				if(!checksumJudge)
				{
					printf("收到包 长度 = %lu \n",recvLength);

					if(ToLogin == pSLSS->sIPD.TcpHead.wEnumPackType)
					{
						printLoginPack(buf+34,0);//·要改

						LoginPackUpdata(buf);//登陆注册逻辑处理函数

						printf("发送前\n");
						printf("协议 = %u\n",(unsigned char)buf[23]);
						printf("ip校验 = %#X\n",myChecksum((unsigned short *)(buf+14),20));
						printf( "tcp校验 = %#X\n",myChecksum( (unsigned short *)(buf+34+8),40+sizeof(struct sLonginParameter)-8 ) );
						printf("ipLen %#X\n",*((unsigned short *)(buf+16)));
						printf("sendLen = %lu\n",34+sizeof(struct LoginServerSend));
						printf("=========================================\n=========================================\n");
						printLoginPack(buf+34,1);
						printf("=========================================\n=========================================\n");

						sendto(sock_raw_fd,buf,34+sizeof(struct LoginServerSend),0,(struct sockaddr *)&sll,sizeof(sll));

						printf("发送成功\n");
					}else if(Finish == pSLSS->sIPD.TcpHead.wEnumPackType)
					{
						PackageMake((unsigned char * )(buf+34),0,FINACK);
						//添加ip校验
						*((unsigned short *)(buf+24)) = myChecksum((unsigned short *)(buf+14),20);
						sendto(sock_raw_fd,buf,recvLength,0,(struct sockaddr *)&sll,sizeof(sll));
					}
				}else
				{
					printf("校验和错误包start=========\n");
					printf("toTalSum = %x\n",checksumJudge);
					checksumJudge = myChecksum((unsigned short *)(buf+14),20);
					printf("IPCheckSum = %x\n",checksumJudge);
					checksumJudge = myChecksum((unsigned short *)(buf+34+8),sizeof(struct sIpData)+sizeof(struct sUserLoginParameter)-8);
					printf("TCPCheckSum = %x\n",checksumJudge);
					printf("校验和错误包=========end\n");

					memset(buf,0,sizeof(buf));
				}
			}
		}
		memset(buf,0,sizeof(buf));
	}

	close(sock_raw_fd);
	while(1);
	printf("登陆服务器关闭。\n");
	return 0;
}

////////自我介绍重发定时回调函数////////
void introduceResendFunc(union sigval v)
{
	len = 14+20+sizeof(struct sIpData);
	if(0 != introduceResendCount)
	{
		printf("调用发送函数! %p\n", v.sival_ptr);
		len = sendto(sock_raw_fd, send_msg, len, 0 , (struct sockaddr *)&sll, sizeof(sll));
		if(len == -1){printf("227行\n");perror("sendto");}
	}

	if(0 == introduceResendCount)
	{
		printf("关闭定时器重发。\n");
		timer_delete( *((timer_t *)(v.sival_ptr)) );
	}
	introduceResendCount--;//被调用一次就减一

}
//修改公共部分
void commonUpdata(unsigned char * buf)
{
	//把包中的源mac地址写到目的mac地址的位置上
	*((unsigned int *)(buf+0)) = *((unsigned int *)(buf+6));
	*((unsigned short *)(buf+4)) = *((unsigned short *)(buf+10));
	//把自己mac地址写到源mac地址的位置上
	*((unsigned int *)(buf+6)) = srcMacBigEndianPart1;// = *((unsigned int *)(send_msg+6));
	*((unsigned short *)(buf+10)) = srcMacBigEndianPart2;// = *((unsigned short *)(send_msg+10));
	//换ip
	changeTemp = *((unsigned int *)(buf+26));
	*((unsigned int *)(buf+26)) = *((unsigned int *)(buf+30));
	*((unsigned int *)(buf+30)) = changeTemp;

	//换port
	changeTemp2 = pSLSS->sIPD.TcpHead.wSrcPort;
	pSLSS->sIPD.TcpHead.wSrcPort = pSLSS->sIPD.TcpHead.wDstPort;
	pSLSS->sIPD.TcpHead.wDstPort = changeTemp2;

	//设置服务类型
	buf[15] = 0;
	//重置ttl
	buf[22] = 64;//这个重置，否则路由器转发次数可能会不够。然后心跳包和分手包可能要重算校验和
	//16位标识
	*((unsigned short *)(buf+18)) = 0;
	//3位标志和13位偏移
	*((unsigned short *)(buf+20)) = 0x40;
	//udp部分和ip部分的校验位置 置0
	*((unsigned short *)(buf+24)) = 0;
	pSLSS->sIPD.TcpHead.wUDPCheckWord = 0;
}
void LoginPackUpdata(unsigned char * buf)
{
	TCPData_analysis_retNum = (enum EErrorCode)(TCPData_analysis((buf+34),sizeof(buf)));

	pSLSS->sLP.uErrorCode = (enum EErrorCode)TCPData_analysis_retNum;//这个是回复错误码

	/*LoginSuccess,5//创建socket，半连接状态 登陆成功1 时填
	  ToUSER	6//注册成功3 注册失败2 登陆失败0 时天*/
	if(1 == TCPData_analysis_retNum){pSLSS->sIPD.TcpHead.wEnumPackType = LoginSuccess;
	}else{pSLSS->sIPD.TcpHead.wEnumPackType = ToUser;}

	//改用户层长度
	pSLSS->sIPD.UserHead.wParametersLength = sizeof(struct sLonginParameter);

	//填udp长度
	//大端方式 sizeof(struct sIpData)+sizeof(struct sLonginParameter)
	pSLSS->sIPD.TcpHead.wUDPLength = invertWord(sizeof(struct sIpData)+sizeof(struct sLonginParameter)) ;

	//改用户层校验和
	pSLSS->sIPD.UserHead.wCheckWord = 0;
	pSLSS->sIPD.UserHead.wCheckWord = myChecksum((unsigned short *)(buf+34+8),sizeof(struct sIpData)+sizeof(struct sLonginParameter)-8);

	//改ip头中的总长度
	*((unsigned short *)(buf+16)) = ipPackTotalLenBigEndian;

	//改ip头中的校验和
	//*((unsigned short *)(buf+24)) = 0;//在分支之前已经置0
	*((unsigned short *)(buf+24)) = myChecksum((unsigned short *)(buf+14),20);


}
