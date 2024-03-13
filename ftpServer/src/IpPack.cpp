#include"IpPack.h"
#include"scCariableip.h"
#include<string.h>
#include<arpa/inet.h>
#include<stdio.h>
extern unsigned int uiUdpPort;
extern unsigned int uiGetWayPort;
extern void* psSocketAddr;
extern unsigned int uiSrcIpAddr[];
extern unsigned int uiDstIpAddr[];
unsigned char cData[34]={
	0x2c,0x61,0x04,0xfb,0xf6,0x17,//目的mac地址
	0x00,0x0c,0x29,0xba,0x03,0x64,//源mac地址
	0x08,0x00,//ip协议
	//ip头
	0x45,0x00,0x00,0x00,//一字节版本号，首部长度，一字节TOS，2字节长度
	0x00,0x00,0x00,0x00,//2字节标志位，2字节分片和片偏移
	0x80,0x11,0x00,0x00,//一字节ttl,一字节协议，2字节首部校验
	0xc0,0xa8,0x00,0xdf,//源ip
	0xc0,0xa8,0x00,0xde,//目的ip地址
};
void PutIpAddr()
{
	int i=0;
	int count=26;
	for(i=0;i<4;i++)
	{
		cData[count]=uiSrcIpAddr[i]&0xff;
		count++;
	}
	for(i=0;i<4;i++)
	{
		cData[count]=uiDstIpAddr[i]&0xff;
		count++;
	}
}

void XchangeIpPort(unsigned char* buf)
{
	int i=0;
	unsigned char c;
	for(i=0;i<6;i++)//mac
	{
		c=buf[i];
		buf[i]=buf[i+6];
		buf[i+6]=c;
	}
	for(i=26;i<30;i++)//ip
	{
		c=buf[i];
		buf[i]=buf[i+4];
		buf[i+4]=c;
	}
	//端口
	struct sIpData * pSIpData = (struct sIpData *)(buf+34);
	
	i=(pSIpData->TcpHead).wSrcPort;
	(pSIpData->TcpHead).wSrcPort = (pSIpData->TcpHead).wDstPort;
	(pSIpData->TcpHead).wDstPort = i;
}

void CopyMacIpHead(unsigned char* buf)
{
	//strncpy((char*)buf,(char*)cData,34);
	memcpy(buf,cData,34);
	/*int i=0;
	for(i=0;i<34;i++)
	{
		buf[i]=cData[i];
	}*/
}
void SelfIntroductionPackage(unsigned char * buf)//tcp头部和user头部
{

	struct sIpData * pSIpData = (struct sIpData *)buf;
	(pSIpData->TcpHead).wSrcPort = htons(uiUdpPort);//源端口
	(pSIpData->TcpHead).wDstPort = htons(uiGetWayPort);//目的端口
	(pSIpData->TcpHead).usLen=0;//udp包含头部长度
	(pSIpData->TcpHead).usCheck = 0;//校验
	(pSIpData->TcpHead).uSeqNumber = 1;//序列号
	(pSIpData->TcpHead).uAckNumber = 0;//确认号
	(pSIpData->TcpHead).wEnumPackType = ServerSelfIntroduction;//枚举包

	(pSIpData->UserHead).wParametersLength = 0;//数据部分长度
	(pSIpData->UserHead).wCheckWord = 0;//校验
	(pSIpData->UserHead).uUserOperation = Login;//操作码
	(pSIpData->UserHead).pSocket = psSocketAddr;//socket地址
	(pSIpData->UserHead).ullUserId = 1;//登录注册服务器

}

//TCP和IP校验函数	//第一个参数是首地址 第二个是字节数
unsigned short myChecksum(unsigned short * addr, int count)
{
    unsigned int sum = 0;

/*
    计算所有数据的16bit对之和
*/
    while( count > 1  )
    {
        /*  This is the inner loop */
        //printf("\n*(unsigned short*)addr=%#X\n",*(unsigned short*)addr);//checking
        sum += *(unsigned short*)addr++;
        //printf("sum = %#X\n",sum);
        count -= 2;
    }

/*
    如果数据长度为奇数，在该字节之后补一个字节(0),
   然后将其转换为16bit整数，加到上面计算的校验和中
*/
    if( count > 0 ) {
        char left_over[2] = {0};
        left_over[0] = *addr;
        sum += * (unsigned short*) left_over;
    }

/*
    将32bit数据压缩成16bit数据，即将进位加大校验和
　　的低字节上，直到没有进位为止。
*/
	while (sum>>16)
	{
		sum = (sum & 0xffff) + (sum >> 16);
	}
/*返回校验和的反码*/
//	printf("\nsum = %#X\n",sum);
//	printf("~sum = %#X\n",~sum);
   return ~sum;
}
