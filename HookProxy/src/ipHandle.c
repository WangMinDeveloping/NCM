#include "ipHandle.h"
#include "socket.h"
#include "invert.h" //包含各种字节倒置函数,校验和计算
#include "cppmod.h"
#include <linux/time.h>

unsigned char aucArmMac[6];
unsigned char aucSourceMac[6]; // 本机MAC
unsigned int uSourceIP;
unsigned short int wSourcePort;
SSocket *(*pstServerSocket)[];
void *pSocketMemoryPool;
void *pAllSocketList;

SSocket *synAckHandle(struct iphdr *pIPHeader, struct STransportLayerHeader *pSelfTCPHeader) { return 0; }

void createSocketMemoryPool()
{
	pstServerSocket = (SSocket * (*)[]) kmalloc(sizeof(SSocket *) * SERVER_TOTAL, GFP_KERNEL); // 服务器数组
	pAllSocketList = doublyListInit();														   // 心跳包链表
	pSocketMemoryPool = memoryStackInit(SOCKET_TOTAL, sizeof(SSocket));						   // socket内存池
}

void destorySocketMemoryPool()
{
	kfree(pstServerSocket);
	memoryStackDestory(pSocketMemoryPool);
	doublyListDestory(pAllSocketList);
}

// 校验socket地址是否正确，id是否对的上.
// 返回值 1用户ID不对，0 socket地址错误的 大于1 正确
int checkSocket(struct STransportLayerHeader *pSelfTCPHeader)
{
	// socket地址是否在申请的片空间范围内
	if (pSelfTCPHeader->pDestinationSocket >= (SSocket *)getMinAddress(pSocketMemoryPool) && pSelfTCPHeader->pDestinationSocket < (SSocket *)getMaxAddress(pSocketMemoryPool))
	{
		if (((unsigned long long)(pSelfTCPHeader->pDestinationSocket) % sizeof(SSocket)) == 0)
		{
			if (pSelfTCPHeader->pDestinationSocket->ullUserID == pSelfTCPHeader->ullUserID)
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
	struct udphdr *pUDPHeader = (struct udphdr *)skb_transport_header(pPackage);
	unsigned char *pMacHeader = pPackage->head + pPackage->mac_header;

	memcpy(pMacHeader, aucArmMac, 6);
	memcpy(pMacHeader + 6, aucSourceMac, 6);

	pIPHeader->daddr = pDestinationSocket->uArmIPAddress;
	pUDPHeader->dest = pDestinationSocket->wArmPort;

	/*计算校验和*/
	pIPHeader->check = 0;
	pUDPHeader->check = 0;
	pIPHeader->check = ip_fast_csum(pIPHeader, pIPHeader->ihl);

	/*计算UDP的校验和*/
	pPackage->csum = csum_partial(pUDPHeader, htons(pUDPHeader->len), 0); // skb->data指向传输层头
	pUDPHeader->check = csum_tcpudp_magic(pIPHeader->saddr, pIPHeader->daddr, htons(pUDPHeader->len), pIPHeader->protocol, pPackage->csum);

	if (0 > dev_queue_xmit(pPackage))
	{
		kfree_skb(pPackage);
		return 1; // 失败
	}
	else
	{
		return 0; // 成功
	}
}

/*服务器自我介绍包处理函数*/
SSocket *serverSelfIntroduction(struct iphdr *pIPHeader, struct STransportLayerHeader *pSelfTCPHeader)
{
	struct timeval ts;
	SSocket *pServerSocket = 0;
	struct udphdr *pUDPHeader = (struct udphdr *)((char *)pIPHeader + pIPHeader->ihl * 4);
	if (pSelfTCPHeader->ullUserID >= 0 && pSelfTCPHeader->ullUserID < SERVER_TOTAL) // 服务器ID正确
	{
		pServerSocket = (SSocket *)(pop_c(pSocketMemoryPool)); // 出栈一个socket空间
		if (pServerSocket != NULL)							   // 出栈成功
		{
			pServerSocket->ullUserID = pSelfTCPHeader->ullUserID;
			pServerSocket->uArmIPAddress = pIPHeader->saddr; // 大端
			pServerSocket->wArmPort = pUDPHeader->source;	 // 大端
			pServerSocket->pHeartHopSelfNodeAddress = 0;

			do_gettimeofday(&ts);
			pServerSocket->ullOnlineTime = ts.tv_sec;
			/*存储本机信息*/
			memcpy((void *)aucArmMac, (void *)((unsigned char *)pIPHeader - 8), 6);
			memcpy((void *)aucSourceMac, (void *)((unsigned char *)pIPHeader - 14), 6);
			uSourceIP = pIPHeader->daddr;								   // 大端
			wSourcePort = pUDPHeader->dest;								   // 大端
			(*pstServerSocket)[pSelfTCPHeader->ullUserID] = pServerSocket; // 放入数组
			pSelfTCPHeader->pDestinationSocket = pServerSocket;			   // 将服务器socket地址给服务器
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
SSocket *finHandle(struct iphdr *pIPHeader, struct STransportLayerHeader *pSelfTCPHeader)
{
	if (checkSocket(pSelfTCPHeader) > 1) // socket地址正确
		return pSelfTCPHeader->pDestinationSocket;
	return 0;
}

// 成功返回1
SSocket *finAckHandle(struct iphdr *pIPHeader, struct STransportLayerHeader *pSelfTCPHeader)
{
	if (checkSocket(pSelfTCPHeader) > 1) // socket地址正确
	{
		if (pSelfTCPHeader->pDestinationSocket->ullUserID >= SERVER_TOTAL) // 是用户的finACK
		{
			doublyListDelete(pAllSocketList, pSelfTCPHeader->pDestinationSocket->pHeartHopSelfNodeAddress);
		}
		else
		{
			(*pstServerSocket)[pSelfTCPHeader->pDestinationSocket->ullUserID] = 0; // 服务器下线将数组置0
		}
		pSelfTCPHeader->pDestinationSocket->ullUserID = -1;			   // 回栈的socket的ullUserID置0
		push_c(pSocketMemoryPool, pSelfTCPHeader->pDestinationSocket); // socket入栈，
	}
	return 0;
}

SSocket *heartHopHandle(struct iphdr *pIPHeader, struct STransportLayerHeader *pSelfTCPHeader)
{
	if (checkSocket(pSelfTCPHeader) > 1) // socket地址正确
	{
		struct timeval ts;
		do_gettimeofday(&ts);
		pSelfTCPHeader->pDestinationSocket->ullOnlineTime = ts.tv_sec;
	}
	return 0;
}
// 登录成功
SSocket *loginSuccess(struct iphdr *pIPHeader, struct STransportLayerHeader *pSelfTCPHeader)
{
	int iRet = checkSocket(pSelfTCPHeader);
	if (iRet > 1) // socket地址正确
		return pSelfTCPHeader->pDestinationSocket;
	return 0;
}

SSocket *toFTP(struct iphdr *pIPHeader, struct STransportLayerHeader *pSelfTCPHeader)
{
	if (checkSocket(pSelfTCPHeader) > 1) // socket地址正确
	{
		struct timeval ts;
		do_gettimeofday(&ts);
		pSelfTCPHeader->pDestinationSocket->ullOnlineTime = ts.tv_sec; // 更新用户在线时间
		return (*pstServerSocket)[1] > 0 ? (*pstServerSocket)[1] : 0;
	}
	return 0;
}

// 登录或注册
SSocket *toLogin(struct iphdr *pIPHeader, struct STransportLayerHeader *pSelfTCPHeader)
{
	void *pUserSocketListNode;
	struct udphdr *pUDPHeader = (struct udphdr *)((char *)pIPHeader + pIPHeader->ihl * 4);
	SSocket *pUserSocket = (SSocket *)(pop_c(pSocketMemoryPool)); // 出栈一个socket空间
	if (pUserSocket != NULL)
	{
		pUserSocketListNode = doublyListInsert(pAllSocketList, pUserSocket);
		if (pUserSocketListNode != NULL) // socket插入链表成功
		{
			struct timeval ts;
			do_gettimeofday(&ts);
			pUserSocket->pHeartHopSelfNodeAddress = pUserSocketListNode;
			pUserSocket->ullUserID = pSelfTCPHeader->ullUserID;
			pUserSocket->uArmIPAddress = pIPHeader->saddr; // 大端
			pUserSocket->wArmPort = pUDPHeader->source;	   // 大端
			pUserSocket->ullOnlineTime = ts.tv_sec;

			pSelfTCPHeader->pDestinationSocket = pUserSocket; // 将用户的socket发送给登录注册服务器

			pUserSocket->ullOnlineTime = ts.tv_sec; // 更新注册用户在线时间
			return (*pstServerSocket)[0] > 0 ? (*pstServerSocket)[0] : 0;
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

SSocket *toUser(struct iphdr *pIPHeader, struct STransportLayerHeader *pSelfTCPHeader)
{
	int iRet = checkSocket(pSelfTCPHeader);
	if (iRet > 1) // socket地址正确
		return pSelfTCPHeader->pDestinationSocket;
	return 0;
}

void sendHeartHop(SSocket *pDestinationSocket) // 后面写
{

	struct sk_buff *skb = NULL;
	int size = 62; // mac+ip+udp+selfTCP
	struct udphdr *udph;
	struct iphdr *iph;
	struct ethhdr *ethdr;
	struct STransportLayerHeader stTCPHeader; // 自己的TCP协议头部

	skb = dev_alloc_skb(size); // 申请size的空间，head，tail,data都指向空间起始位置
	skb->ip_summed = CHECKSUM_NONE;
	skb->protocol = htons(ETH_P_IP);
	skb->pkt_type = PACKET_OTHERHOST;
	skb->dev = dev_get_by_name(&init_net, "ens33"); // 设备名称
	skb->priority = 0;
	skb_reserve(skb, size); // 将data和tail往后偏移size

	/*组selfTCP部分*/
	stTCPHeader.pDestinationSocket = pDestinationSocket;
	stTCPHeader.ullUserID = pDestinationSocket->ullUserID;
	stTCPHeader.iPackageType = HeartHop;
	skb_push(skb, 20); // data往前移了20个字节
	memcpy(skb->data, &stTCPHeader, 20);

	/*组UDP部分*/
	skb_push(skb, 8); // data往前移了8个字节
	udph = (struct udphdr *)skb->data;

	udph->source = wSourcePort;
	udph->dest = pDestinationSocket->wArmPort;
	udph->len = htons(0x1c);
	udph->check = 0;
	skb->csum = csum_partial(udph, 28, 0); // skb->data指向传输层头
	udph->check = csum_tcpudp_magic(uSourceIP, pDestinationSocket->uArmIPAddress, 28, 17, skb->csum);

	/*组IP部分*/
	skb_push(skb, 20);
	iph = (struct iphdr *)skb->data;

	iph->version = 4;
	iph->ihl = sizeof(struct iphdr) >> 2; // 5
	iph->tos = 0;
	iph->tot_len = htons(0x30); /*TODO*/ /*报文长度*/
	iph->id = 1;
	iph->frag_off = 0;
	iph->ttl = 0x80;	  // 128
	iph->protocol = 0x11; // 17
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
	skb->len = 48;

	if (0 > dev_queue_xmit(skb))
	{
		kfree_skb(skb);
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
	while ((pSocket = (SSocket *)doublyListIterator(pAllSocketList)) != NULL)
	{
		if ((ts.tv_sec - pSocket->ullOnlineTime) > 130) // 超时未发送消息，关闭socket
		{
			doublyListDelete(pAllSocketList, pSocket->pHeartHopSelfNodeAddress); // 从结点删除改socket
			pSocket->ullUserID = -1;											 // 回栈的socket的ullUserID置-1
			push_c(pSocketMemoryPool, pSocket);									 // socket入栈，
		}
		else if ((ts.tv_sec - pSocket->ullOnlineTime) > 60)
			sendHeartHop(pSocket);
	}
	/*给服务器发心跳包*/
	for (i = 0; i < SERVER_TOTAL; i++)
	{
		if ((*pstServerSocket)[i] != NULL)
		{
			if ((ts.tv_sec - (*pstServerSocket)[i]->ullOnlineTime) > 130) // 超时未发送消息，关闭socket
			{
				(*pstServerSocket)[i]->ullUserID = -1;			  // 回栈的socket的ullUserID置-1
				push_c(pSocketMemoryPool, (*pstServerSocket)[i]); // socket入栈，
				(*pstServerSocket)[i] = 0;
			}
			else if ((ts.tv_sec - (*pstServerSocket)[i]->ullOnlineTime) > 60)
				sendHeartHop((*pstServerSocket)[i]);
		}
	}
	mod_timer(arg, jiffies + msecs_to_jiffies(1000 * 60)); // 循环定时一分钟
}
