#include <cstdio>
#include "tcpIpFunc.h"

void IPHeadSetting(unsigned char *buf, unsigned short totalLen)
{
	struct IPHeader *stIPHeader = (struct IPHeader *)buf;
	/*IP头部组包*/
	stIPHeader->ucProtocol = 17;			// 254;254可能无法在网络上传播//设置IP包头部的上层协议
	stIPHeader->bitIPVersion = 4;			// IP版本为4
	stIPHeader->bitIPHeaderLen = 5;			// IP首部长度5
	stIPHeader->ucDifferentiatServices = 0; // IP区分服务为0
	stIPHeader->wLenTotal = totalLen;		// 总长度

	stIPHeader->wIdentification = 0; // 设置标识位
	stIPHeader->bitFlags = 2;		 // 没有分片
	stIPHeader->bitSliceOffset = 0;	 // IP片偏移为0
	stIPHeader->ucTTL = 64;			 // TTL linux下为64
	stIPHeader->uSourceIP = srcIp;	 // 本地IP
	stIPHeader->uArmIP = dstIp;		 // 目标IP
	stIPHeader->wHeaderCheckSum = 0; // 校验和初始值为0

	stIPHeader->wHeaderCheckSum = myChecksum((unsigned short int *)stIPHeader, sizeof(struct IPHeader)); // 计算IP校验和

	// 倒置
	invertFunction((void *)buf, 20); // 调用字节倒置函数
}

////////////////////组包函数////////////////////////////////
void PackageMake(unsigned char *buf, unsigned long long id, enum EPackageType packType) // tcp头部和user头部
{
	struct sIpData *pSIpData = (struct sIpData *)buf;
	(pSIpData->TcpHead).wSrcPort = invertWord(srcPort);					 // 源端口
	(pSIpData->TcpHead).wDstPort = invertWord(dstPort);					 // 目的端口
	(pSIpData->TcpHead).wUDPLength = invertWord(sizeof(struct sIpData)); // udp表头+数据长度
	(pSIpData->TcpHead).wUDPCheckWord = 0;								 // udp校验和
	(pSIpData->TcpHead).uSeqNumber = 0;									 // 序列号
	(pSIpData->TcpHead).uAckNumber = 0;									 // 确认号
	(pSIpData->TcpHead).wEnumPackType = packType;						 // 枚举包

	(pSIpData->UserHead).wParametersLength = 0;	 // 数据部分
	(pSIpData->UserHead).wCheckWord = 0;		 //
	(pSIpData->UserHead).uUserOperation = Login; // 操作码
	if (ServerSelfIntroduction == packType)
	{
		(pSIpData->UserHead).pSocket = 0;
	}
	else
	{
		(pSIpData->UserHead).pSocket = (void *)loginServerSocketAddr; // socket地址
	}
	(pSIpData->UserHead).ullUserId = id; // 登录注册服务器

	(pSIpData->UserHead).wCheckWord = myChecksum((unsigned short *)(&((pSIpData->TcpHead).uSeqNumber)), sizeof(struct sIpData) - 8);
}

////////////////////////////////////////
void printLoginPack(unsigned char *buf, unsigned char witch) // witch:1是打印错误码，0是打印密码
{
	struct LoginServerSend *pSLSS = (struct LoginServerSend *)buf;

	printf("wSrcPort = %u\n", invertWord(pSLSS->sIPD.TcpHead.wSrcPort));
	printf("wDstPort = %u\n", invertWord(pSLSS->sIPD.TcpHead.wDstPort));
	printf("wUDPLength = %u\n", invertWord(pSLSS->sIPD.TcpHead.wUDPLength));
	printf("wUDPCheckWord = %#X\n", invertWord(pSLSS->sIPD.TcpHead.wUDPCheckWord));

	printf("uSeqNumber = %u\n", pSLSS->sIPD.TcpHead.uSeqNumber);
	printf("uAckNumber = %u\n", pSLSS->sIPD.TcpHead.uAckNumber);
	switch (pSLSS->sIPD.TcpHead.wEnumPackType)
	{
	case ServerSelfIntroduction:
		printf("ServerSelfIntroduction:");
		break;
	case Finish:
		printf("Finish:");
		break;
	case FINACK:
		printf("FINACK:");
		break;
	case HeartHop:
		printf("HeartHop:");
		break;
	case ToLogin:
		printf("ToLogin:");
		break;
	case ToFtp:
		printf("ToFtp:");
		break;
	case LoginSuccess:
		printf("LoginSuccess:");
		break;
	case ToUser:
		printf("ToUser:");
		break;
	}
	printf(" wEnumPackType = %u\n", pSLSS->sIPD.TcpHead.wEnumPackType);

	printf("wParametersLength = %u\n", pSLSS->sIPD.UserHead.wParametersLength);
	printf("wCheckWord = %x\n", pSLSS->sIPD.UserHead.wCheckWord);
	switch (pSLSS->sIPD.UserHead.uUserOperation)
	{
	case Login:
		printf("Login:");
		break;
	case Register:
		printf("Register:");
		break;
	case AskFTPUpLoadFile:
		printf("AskFTPUpLoadFile:");
		break;
	case AskFTPDownLoadFile:
		printf("AskFTPDownLoadFile:");
		break;
	case FTPFileOver:
		printf("FTPFileOver:");
		break;
	case SAck:
		printf("SAck:");
		break;
	case Ack:
		printf("Ack:");
		break;
	case Push:
		printf("Push:");
		break;
	}
	printf(" uUserOperation = %u\n", pSLSS->sIPD.UserHead.uUserOperation);
	printf("pSocket = %p\n", pSLSS->sIPD.UserHead.pSocket);
	printf("ullUserId = %llu\n", pSLSS->sIPD.UserHead.ullUserId);

	printf("uIpAddress = %x\n", pSLSS->sLP.Ip.uIpAddress);
	printf("wIpPort = %u\n", pSLSS->sLP.Ip.wIpPort);

	if (1 == witch)
	{
		switch (pSLSS->sLP.uErrorCode)
		{
		case ECodeLoginFail:
			printf("ECodeLoginFail:");
			break;
		case ECodeLoginSuccess:
			printf("ECodeLoginSuccess:");
			break;
		case ECodeRegisterFail:
			printf("ECodeRegisterFail:");
			break;
		case ECodeRegisterSuccess:
			printf("ECodeRegisterSuccess:");
			break;

		case ECodeUpLoadFileAllow:
			printf("ECodeUpLoadFileAllow:");
			break;
		case ECodeUpLoadFileNotAllow:
			printf("ECodeUpLoadFileNotAllow:");
			break;
		case ECodeDownLoadFileAllow:
			printf("ECodeDownLoadFileAllow:");
			break;
		case ECodeDownLoadFileNotAllow:
			printf("ECodeDownLoadFileNotAllow:");
			break;
		case ECodeUpLoadFileFail:
			printf("ECodeUpLoadFileFail:");
			break;
		case ECodeDownLoadFileFail:
			printf("ECodeDownLoadFileFail:");
			break;
		}
		printf(" uErrorCode = %u\n", pSLSS->sLP.uErrorCode);
	}
	else if (0 == witch)
	{
		printf("password = ");
		for (unsigned int i = sizeof(struct sIpData) + 6; i < sizeof(struct sIpData) + 6 + 16; i++)
		{
			printf("%#X ", buf[i]);
		}
		printf("\n");
	}
	else
	{
		printf("其他类型数据。\n");
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////工具函数////////////////////////////////
// 校验函数
// TCP和IP校验函数	//第一个参数是首地址 第二个是字节数
unsigned short myChecksum(unsigned short *addr, int count)
{
	unsigned int sum = 0;

	/*
		计算所有数据的16bit对之和
	*/
	while (count > 1)
	{
		/*  This is the inner loop */
		//		printf("\n*(unsigned short*)addr=%#X\n",*(unsigned short*)addr);//checking
		sum += *(unsigned short *)addr++;
		//		printf("sum = %#X\n",sum);
		count -= 2;
	}

	/*
		如果数据长度为奇数，在该字节之后补一个字节(0),
	   然后将其转换为16bit整数，加到上面计算的校验和中
	*/
	if (count > 0)
	{
		char left_over[2] = {0};
		left_over[0] = *addr;
		sum += *(unsigned short *)left_over;
	}

	/*
		将32bit数据压缩成16bit数据，即将进位加大校验和
	　　的低字节上，直到没有进位为止。
	*/
	while (sum >> 16)
	{
		sum = (sum & 0xffff) + (sum >> 16);
	}
	/*返回校验和的反码*/
	//	printf("\nsum = %#X\n",sum);
	//	printf("~sum = %#X\n",~sum);
	return ~sum;
}
