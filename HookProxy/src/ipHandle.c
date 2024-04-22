#include "ipHandle.h"
#include "socket.h"
#include "invert.h" //包含各种字节倒置函数,校验和计算
#include "cppmod.h"
#include <linux/time.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>

unsigned char aucArmMac[6];
unsigned char aucSourceMac[6]; // 本机MAC
unsigned int uSourceIP;
unsigned short int wSourcePort;
SSocket *astServerSocket[SERVER_TOTAL] = {0};
void *pSocketMemoryPool;
void *pAllSocketList;
int iServerSockLock = 0;
packageHandle aPackageHandle[ToUser + 1]; // 定义一个函数指针数组，大小是ToUser大小

void gatewayInit()
{
	aPackageHandle[ServerSelfIntroduction] = serverSelfIntroduction;
	aPackageHandle[Finish] = finHandle;
	aPackageHandle[FINACK] = finAckHandle;
	aPackageHandle[HeartHop] = heartHopHandle;
	aPackageHandle[ToLogin] = toLogin;
	aPackageHandle[ToFTP] = toFTP;
	aPackageHandle[LoginSuccess] = loginSuccess;
	aPackageHandle[ToUser] = toUser;

	//	pstServerSocket=(SSocket* (*)[])kmalloc(sizeof(SSocket*)*SERVER_TOTAL, GFP_KERNEL);//服务器数组
	pAllSocketList = DoublyListInit();									// 心跳包链表
	pSocketMemoryPool = memoryStackInit(SOCKET_TOTAL, sizeof(SSocket)); // socket内存池
}

void gatewayDestroy()
{
	//	kfree(pstServerSocket);
	memoryStackDestory(pSocketMemoryPool);
	DoublyListDestory(pAllSocketList);
}

// 校验socket地址是否正确，id是否对的上.
// 返回值 1用户ID不对，0 socket地址错误的 大于1 正确
int checkSocket(struct SSocketAndID *pstSocketID)
{
	// socket地址是否在申请的片空间范围内
	if (pstSocketID->pDestinationSocket >= (SSocket *)getMinAddress(pSocketMemoryPool) && pstSocketID->pDestinationSocket < (SSocket *)getMaxAddress(pSocketMemoryPool))
	{
		if (((unsigned long long)(pstSocketID->pDestinationSocket) % sizeof(SSocket)) == 0)
		{
			if (pstSocketID->pDestinationSocket->ullUserID == pstSocketID->ullUserID)
				return 2;
			else
			{
#ifdef _DEBUG
				printk("checkSocket ID对不上，可能用户已下线");
#endif
				return 1;
			}
		}
	}
#ifdef _DEBUG
	printk("check Socket地址不正确\n");
#endif
	return 0;
}
// 返回值 0成功 1失败
int regroupPackage(struct sk_buff *pPackage, SSocket *pDestinationSocket) // 重组数据包
{
	struct iphdr *pIPHeader = ip_hdr(pPackage);
	struct udphdr *pUDPHeader = (struct udphdr *)skb_transport_header(pPackage); // 借用udp头部的源端口和目的端口结构体
	unsigned char *pMacHeader = pPackage->head + pPackage->mac_header;
	memcpy(pMacHeader, aucArmMac, 6);
	memcpy(pMacHeader + 6, aucSourceMac, 6);

	pIPHeader->daddr = pDestinationSocket->uArmIPAddress;
	pUDPHeader->dest = pDestinationSocket->wArmPort; // 小端

	pIPHeader->saddr = uSourceIP;
	pUDPHeader->source = wSourcePort; // 小端
#ifdef _DEBUG
	/*printk(" pIPHeader->daddr=%u\n",htonl(pIPHeader->daddr));
	  printk(" pUDPHeader->dest=%d\n",htons(pUDPHeader->dest));
	  printk(" pIPHeader->saddr=%u\n",htonl(pIPHeader->saddr));
	  printk(" pUDPHeader->source=%d\n",htons(pUDPHeader->source));*/

#endif

	/*计算校验和*/
	pIPHeader->check = 0;
	pIPHeader->check = ip_fast_csum(pIPHeader, pIPHeader->ihl);

	sendPackage(pPackage);

	/*if(0>dev_queue_xmit(pPackage))
	  {
#ifdef _DEBUG
printk("package send fail\n");
#endif
}
else
{
#ifdef _DEBUG
printk("package send success\n");
#endif
}*/
	return 0;
}
void sendPackage(struct sk_buff *pPackage) // 后面写
{

	struct sk_buff *skb = NULL;
	struct iphdr *pIPHeader = (struct iphdr *)ip_hdr(pPackage);
	int size = htons(pIPHeader->tot_len) + 14;

	skb = dev_alloc_skb(size); // 申请size的空间，head，tail,data都指向空间起始位置
	skb->ip_summed = CHECKSUM_NONE;
	skb->protocol = htons(ETH_P_IP);
	skb->pkt_type = PACKET_OTHERHOST;
	skb->dev = dev_get_by_name(&init_net, "ens33"); // 设备名称
	skb->priority = 0;
	skb_reserve(skb, size); // 将data和tail往后偏移size

	skb_push(skb, size);									  // data往前移了size个字节
	memcpy(skb->data, (unsigned char *)pIPHeader - 14, size); // copy mac package

	skb->mac_header = skb->data - skb->head;
	skb->network_header = skb->mac_header + 14;
	skb->transport_header = skb->network_header + 20;
	skb->len = size;

	if (0 > dev_queue_xmit(skb))
	{
#ifdef _DEBUG
		printk("package send fail\n");
#endif
	}
	else
	{
#ifdef _DEBUG
		printk("package send success\n");
#endif
	}
	kfree(pPackage);
}

/*服务器自我介绍包处理函数*/
SSocket *serverSelfIntroduction(struct sk_buff *pPackage, struct SSocketAndID *pstSocketID)
{
	struct iphdr *pIPHeader = (struct iphdr *)ip_hdr(pPackage);
	SSocket *pServerSocket = 0;
	struct timeval ts;
	struct SIPData *pstIPData = (struct SIPData *)((char *)pIPHeader + pIPHeader->ihl * 4);
	do_gettimeofday(&ts);
	if (pstSocketID->ullUserID >= 0 && pstSocketID->ullUserID < SERVER_TOTAL) // 服务器ID正确
	{
		pServerSocket = (SSocket *)(pop_c(pSocketMemoryPool)); // 出栈一个socket空间
		if (pServerSocket != NULL)							   // 出栈成功
		{
			pServerSocket->ullUserID = pstSocketID->ullUserID;
			pServerSocket->uArmIPAddress = pIPHeader->saddr;		   // 大端
			pServerSocket->wArmPort = pstIPData->stTCPHeader.wSrcPort; // 小端
			pServerSocket->pHeartHopSelfNodeAddress = 0;
			pServerSocket->ullOnlineTime = ts.tv_sec;
			/*存储本机信息*/
			memcpy((void *)aucArmMac, (void *)((unsigned char *)pIPHeader - 8), 6);
			memcpy((void *)aucSourceMac, (void *)((unsigned char *)pIPHeader - 14), 6);
			uSourceIP = pIPHeader->daddr;				   // 大端
			wSourcePort = pstIPData->stTCPHeader.wDstPort; // 小端

			asm("loop1:lock btsl $0,%0;jc loop1;" : "+m"(iServerSockLock)); // 上锁
			if (astServerSocket[pstSocketID->ullUserID] != NULL)
				if (push_c(pSocketMemoryPool, astServerSocket[pstSocketID->ullUserID]) != 0) // 如果以前服务器已发过selfIntroduction，将其回栈
				{
#ifdef _DEBUG
					printk("serverSelfIntroduction 栈满\n");
#endif
				}
			astServerSocket[pstSocketID->ullUserID] = pServerSocket; // 放入数组
			iServerSockLock = 0;									 // 解锁

			pstSocketID->pDestinationSocket = pServerSocket; // 将服务器socket地址给服务器
			pstIPData->stUserProtocolHeader.wCheckWord = 0;
			pstIPData->stUserProtocolHeader.wCheckWord = checkSum((unsigned short *)&pstIPData->stTCPHeader.wEnumPackType, sizeof(struct SIPData) - 4);
#ifdef _DEBUG
			printk("server success\n");
#endif
			return pServerSocket;
		}
#ifdef _DEBUG
		else
			printk("serverSelfIntroduction socket 栈空，无法接收用户的连接了\n");
#endif
	}
#ifdef _DEBUG
	else
		printk("serverIntriduanction serverID不正确，包丢掉\n");
#endif
	return 0;
}
// 返回值 0失败 非0成功
SSocket *finHandle(struct sk_buff *pPackage, struct SSocketAndID *pstSocketID)
{
	if (checkSocket(pstSocketID) > 1) // socket地址正确
		return pstSocketID->pDestinationSocket;
	return 0;
}

// 成功返回1
SSocket *finAckHandle(struct sk_buff *pPackage, struct SSocketAndID *pstSocketID)
{
	if (checkSocket(pstSocketID) > 1) // socket地址正确
	{
		if (pstSocketID->pDestinationSocket->ullUserID >= SERVER_TOTAL) // 是用户的finACK
		{
			DoublyListDelete(pAllSocketList, pstSocketID->pDestinationSocket->pHeartHopSelfNodeAddress);
		}
		else
		{
			asm("loop2:lock btsl $0,%0;jc loop2;" : "+m"(iServerSockLock));	 // 上锁
			astServerSocket[pstSocketID->pDestinationSocket->ullUserID] = 0; // 服务器下线将数组置0
			iServerSockLock = 0;											 // 解锁
		}
		pstSocketID->pDestinationSocket->ullUserID = -1;			// 回栈的socket的ullUserID置0
		push_c(pSocketMemoryPool, pstSocketID->pDestinationSocket); // socket入栈，
	}
	return 0;
}

SSocket *heartHopHandle(struct sk_buff *pPackage, struct SSocketAndID *pstSocketID)
{
	struct timeval ts;
	if (checkSocket(pstSocketID) > 1) // socket地址正确
	{
#ifdef _DEBUG
		printk("recv hearthop\n");
#endif
		do_gettimeofday(&ts);
		pstSocketID->pDestinationSocket->ullOnlineTime = ts.tv_sec;
	}
	return 0;
}
// 登录或注册
SSocket *toLogin(struct sk_buff *pPackage, struct SSocketAndID *pstSocketID)
{
	struct iphdr *pIPHeader = (struct iphdr *)ip_hdr(pPackage);
	struct udphdr *pUDPHeader = (struct udphdr *)((char *)pIPHeader + pIPHeader->ihl * 4);
	struct SLoginParameter *pstLoginParameter = (struct SLoginParameter *)((unsigned char *)pIPHeader + pIPHeader->ihl * 4); // 找到之间填的用户的ip和端口
	pstLoginParameter->stUserIPPort.uIPAddress = pIPHeader->saddr;															 // 保存用户的信息，发给登录注册服务器
	pstLoginParameter->stUserIPPort.wPort = pUDPHeader->source;
	pstLoginParameter->stIPData.stUserProtocolHeader.wCheckWord = 0;
	pstLoginParameter->stIPData.stUserProtocolHeader.wCheckWord = checkSum((unsigned short *)&pstLoginParameter->stIPData.stTCPHeader.wEnumPackType, sizeof(struct SLoginParameter) - 4);
#ifdef _DEBUG
	printk("recv toLogin\n");
#endif
	return astServerSocket[0] > 0 ? astServerSocket[0] : 0;
}

SSocket *toFTP(struct sk_buff *pPackage, struct SSocketAndID *pstSocketID)
{
	struct timeval ts;
	do_gettimeofday(&ts);
	if (checkSocket(pstSocketID) > 1) // socket地址正确
	{
		pstSocketID->pDestinationSocket->ullOnlineTime = ts.tv_sec; // 更新用户在线时间
#ifdef _DEBUG
		printk("recv toFTP success\n");
#endif
		return astServerSocket[1] > 0 ? astServerSocket[1] : 0;
	}
	return 0;
}

// 登录成功
SSocket *loginSuccess(struct sk_buff *pPackage, struct SSocketAndID *pstSocketID)
{
	struct iphdr *pIPHeader = (struct iphdr *)ip_hdr(pPackage);
	void *pUserSocketListNode;
	struct timeval ts;
	SSocket *pUserSocket = (SSocket *)(pop_c(pSocketMemoryPool));															 // 出栈一个socket空间
	struct SLoginParameter *pstLoginParameter = (struct SLoginParameter *)((unsigned char *)pIPHeader + pIPHeader->ihl * 4); // 找到之间填的用户的ip和端口
	do_gettimeofday(&ts);
	if (pUserSocket != NULL)
	{
		pUserSocketListNode = DoublyListInsert(pAllSocketList, pUserSocket);
		if (pUserSocketListNode != NULL) // socket插入链表成功
		{

			pUserSocket->pHeartHopSelfNodeAddress = pUserSocketListNode;
			pUserSocket->ullUserID = pstSocketID->ullUserID;
			pUserSocket->uArmIPAddress = pstLoginParameter->stUserIPPort.uIPAddress; // 大端
			pUserSocket->wArmPort = pstLoginParameter->stUserIPPort.wPort;			 // 大端
			pUserSocket->ullOnlineTime = ts.tv_sec;

			pstSocketID->pDestinationSocket = pUserSocket; // 将用户的socket发送给用户

			pstLoginParameter->stIPData.stUserProtocolHeader.wCheckWord = 0;
			pstLoginParameter->stIPData.stUserProtocolHeader.wCheckWord = checkSum((unsigned short *)&pstLoginParameter->stIPData.stTCPHeader.wEnumPackType, sizeof(struct SLoginParameter) - 4);

			asm("loop3:lock btsl $0,%0;jc loop3;" : "+m"(iServerSockLock)); // 上锁
			if ((astServerSocket)[0] != NULL)
				astServerSocket[0]->ullOnlineTime = ts.tv_sec; // 更新注册登录服务器在线时间
#ifdef _DEBUG
			else
				printk("login server is not online\n");
			printk("接收到login success\n");
#endif
			iServerSockLock = 0; // 解锁
			return pUserSocket;
		}
		else
		{
			push_c(pSocketMemoryPool, pUserSocket); // 刚出栈socket入栈，返回，空间不足
#ifdef _DEBUG
			printk("login succss 插入链表动态空间已耗尽\n");
#endif
		}
	}
#ifdef _DEBUG
	else
		printk("serverSelfIntroduction socket 栈空，无法接收用户的连接了\n");
#endif
	return 0;
}

SSocket *toUser(struct sk_buff *pPackage, struct SSocketAndID *pstSocketID)
{
	struct iphdr *pIPHeader = (struct iphdr *)ip_hdr(pPackage);
	struct SLoginParameter *pLoginParameter = (struct SLoginParameter *)((unsigned char *)pIPHeader + pIPHeader->ihl * 4);
	unsigned int uOperationCode = pLoginParameter->stIPData.stUserProtocolHeader.uOperationCode;
	int iRet = checkSocket(pstSocketID);
#ifdef _DEBUG
	printk("recv toUser\n");
#endif
	if (iRet > 1) // socket地址正确
		return pstSocketID->pDestinationSocket;
	else // login fail/register fail/register sucess
	{
		if (uOperationCode == Login || uOperationCode == Register)
		{
			memcpy(pIPHeader - 14, aucArmMac, 6);
			memcpy(pIPHeader - 8, aucSourceMac, 6);

			pIPHeader->daddr = pLoginParameter->stUserIPPort.uIPAddress;
			pLoginParameter->stIPData.stTCPHeader.wDstPort = pLoginParameter->stUserIPPort.wPort; // 小端

			pIPHeader->saddr = uSourceIP;
			pLoginParameter->stIPData.stTCPHeader.wSrcPort = wSourcePort; // 小端

			pIPHeader->check = 0;
			pIPHeader->check = ip_fast_csum(pIPHeader, pIPHeader->ihl);

			sendPackage(pPackage);
		}
	}
	return 0;
}

void sendHeartHop(SSocket *pDestinationSocket) // 后面写
{

	struct sk_buff *skb = NULL;
	int iIPDataLen = sizeof(struct SIPData);
	int size = sizeof(struct ethhdr) + sizeof(struct iphdr) + iIPDataLen; // mac+ip+udp+selfTCP
	struct iphdr *iph;
	struct ethhdr *ethdr;
	struct SIPData stIPData;

	skb = dev_alloc_skb(size); // 申请size的空间，head，tail,data都指向空间起始位置
	skb->ip_summed = CHECKSUM_NONE;
	skb->protocol = htons(ETH_P_IP);
	skb->pkt_type = PACKET_OTHERHOST;
	skb->dev = dev_get_by_name(&init_net, "ens33"); // 设备名称
	skb->priority = 0;
	skb_reserve(skb, size); // 将data和tail往后偏移size

	/*组IP数据部分*/
	stIPData.stTCPHeader.wSrcPort = wSourcePort;
	stIPData.stTCPHeader.wDstPort = pDestinationSocket->wArmPort;
	stIPData.stTCPHeader.wEnumPackType = HeartHop;
	stIPData.stUserProtocolHeader.stSocketID.pDestinationSocket = pDestinationSocket;
	stIPData.stUserProtocolHeader.stSocketID.ullUserID = pDestinationSocket->ullUserID;

#ifdef _DEBUF
	// printk("Heartop=%d\n",HeartHop);
#endif
	skb_push(skb, iIPDataLen); // data往前移了40个字节
	memcpy(skb->data, &stIPData, iIPDataLen);

	/*组IP部分*/
	skb_push(skb, 20);
	iph = (struct iphdr *)skb->data;

	iph->version = 4;
	iph->ihl = sizeof(struct iphdr) >> 2; // 5
	iph->tos = 0;
	iph->tot_len = htons(0x3c); /*TODO*/ /*报文长度*/
	iph->id = 1;
	iph->frag_off = 0;
	iph->ttl = 0x80;	  // 128
	iph->protocol = 0xfe; // 17
	iph->saddr = uSourceIP;
	iph->daddr = pDestinationSocket->uArmIPAddress;
	iph->check = 0; /*TODO*/
	iph->check = ip_fast_csum(iph, iph->ihl);

	/*组MAC部分*/
	skb_push(skb, sizeof(struct ethhdr));
	ethdr = (struct ethhdr *)skb->data;
	memcpy(ethdr->h_dest, aucArmMac, 6);
	memcpy(ethdr->h_source, aucSourceMac, 6);
	ethdr->h_proto = htons(0x0800);

	skb->mac_header = skb->data - skb->head;
	skb->network_header = skb->mac_header + 14;
	skb->transport_header = skb->network_header + 20;
	skb->len = size;

	if (0 > dev_queue_xmit(skb))
	{
#ifdef _DEBUG
		printk("心跳包send out fail\n");
#endif
	}
	else
	{
#ifdef _DEBUG
		printk("心跳包 sendd out success\n");
#endif
	}
}

void OnlineTest(struct timer_list *arg)
{
	SSocket *pSocket;
	int i = 0;
	struct timeval ts;
	do_gettimeofday(&ts);
	/*给用户发心跳包*/
	pSocket = (SSocket *)DoublyListIterator(pAllSocketList);
	while (pSocket != NULL)
	{
		if ((ts.tv_sec - pSocket->ullOnlineTime) > 130) // 超时未发送消息，关闭socket
		{
			DoublyListDelete(pAllSocketList, pSocket->pHeartHopSelfNodeAddress); // 从结点删除改socket
			pSocket->ullUserID = -1;											 // 回栈的socket的ullUserID置-1
			push_c(pSocketMemoryPool, pSocket);									 // socket入栈，
		}
		else if ((ts.tv_sec - (pSocket->ullOnlineTime)) > 60)
		{
			sendHeartHop(pSocket);
#ifdef _DEBUG
			printk("(pSocket->ullOnlineTime=%llu", pSocket->ullOnlineTime);
			printk("(pSocket->ullOnlineTime=%lu", ts.tv_sec);
			printk("client 心跳包发送成功\n");
#endif
		}
		pSocket = (SSocket *)DoublyListIterator(pAllSocketList);
	}
	printk("hearthop exit i=%d\n", i);
	/*给服务器发心跳包*/
	for (i = 0; i < SERVER_TOTAL; i++)
	{
		pSocket = astServerSocket[i];
		if (pSocket != NULL)
		{
			if ((ts.tv_sec - (pSocket->ullOnlineTime)) > 130) // 超时未发送消息，关闭socket
			{
				pSocket->ullUserID = -1;																										// 回栈的socket的ullUserID置-1
				push_c(pSocketMemoryPool, pSocket);																								// socket入栈
				asm volatile("movq %1,%%rax;movq $0,%%rbx;lock cmpxchgq %%rbx,%0;" : "+m"(astServerSocket[i]) : "m"(pSocket) : "%rax", "%rbx"); // 将astServerSocket[i]置0  如同(*pstServerSocket)[i]=0，目的是为了减少锁的使用
			}
			else if ((ts.tv_sec - pSocket->ullOnlineTime) > 60)
			{
				sendHeartHop(pSocket);
#ifdef _DEBUG
				printk("server心跳包发送成功\n");
#endif
			}
		}
	}
	mod_timer(arg, jiffies + msecs_to_jiffies(10000)); // 循环定时一分钟
}
