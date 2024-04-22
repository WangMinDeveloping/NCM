#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include<linux/init.h>
#include <linux/cdev.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/udp.h>
#include <linux/ip.h>
#include <linux/vmalloc.h>
#include <linux/moduleparam.h>
#include <linux/kmod.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/inet.h>
#include <linux/timer.h>
#include <linux/tcp.h>
#include <linux/time.h>
#include <linux/ktime.h>
#include "cppmod.h"
#include "ipHandle.h"

#define DequeueThreadNum (1)
#define QueueLen  (1024)

/*一入多出队列*/
void *pQueue = 0;
struct timer_list timer; // 定义定时器 

// 钩子声明
static struct nf_hook_ops nfho_in; // 钩子函数结构体
volatile int iExitFlag = 1; //线程退出标志

//数据包处理函数数组
typedef SSocket *(*packageHandle)(struct iphdr *, struct STransportLayerHeader *); // 声明一个函数指针
packageHandle aPackageHandle[ToUSER];// 定义一个函数指针数组，ToUSER是最大值，因为用户不会发出这种类型的数据，只有后台应用服务器会发

/*出队线程*/
/*出队线程*/
int threadDeQueue(void *pThreadID)
{
	struct sk_buff *pPackage;
	struct iphdr *pIPHeader;
	struct udphdr *pUDPHeader;
	struct SSocketAndID *pstSocketID;
	unsigned short wPackageType;
	SSocket *pDestinationSocket;
	int iSendSuccess;
	while (iExitFlag)
	{
		pPackage = (struct sk_buff *)deQueue_c(pQueue); // 出队一个数据包
		if (pPackage != NULL)							// 出队成功，非空
		{
			pIPHeader = ip_hdr(pPackage);
			pUDPHeader = udp_hdr(pPackage);
			wPackageType = pUDPHeader->len; // 跳过端口，找到包类型

			if (wPackageType >= PACKAGE_TYPE_INITIAL_VALUE && wPackageType <= ToUser) // 校验包类型是否正确
			{
				pstSocketID = (struct SSocketAndID *)((char *)pIPHeader + pIPHeader->ihl * 4 + 24); // 偏移找到socket首地址
				pDestinationSocket = aPackageHandle[wPackageType](pPackage, pstSocketID);
				if (pDestinationSocket > (SSocket *)1) // 返回的目的socket正确
				{
					iSendSuccess = regroupPackage(pPackage, pDestinationSocket);
					if (iSendSuccess == 1)
						printk("发送数据包失败\n");
				}
			}
			else
			{
				printk("包类型不符合协议\n");
			}
		}
		else // 队空
		{
			msleep(0);
		}
	}
	return 0;
}


unsigned int my_hook_func_in(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
	// 这里是ip头
	struct iphdr *pIPHeader = NULL;		   // skb的ip头部：第一次得到，用于判断
	struct udphdr *pUDPHeader = NULL;
	pIPHeader = (struct iphdr *)skb_network_header(skb);
	if (pIPHeader->protocol == 17) // UDP协议
	{
		pUDPHeader = (struct udphdr *)skb_transport_header(skb);
		if (pUDPHeader->dest == htons(8081)) // 指定端口
		{
			if (enQueue_c(pQueue, (void *)skb) == 0)
				return NF_STOLEN; // 接管数据包
			else
				return NF_DROP; // 队满，丢了
		}
	}
	return NF_ACCEPT; //允许通过
}

// 模块初始化
static int __init GatewayInit(void) // 初始化模块
{
	int i, rc;
	int iThreadNumber[DequeueThreadNum];
	struct task_struct *practice_deQueue_p[DequeueThreadNum] = {}; // 线程返回值
	
	pQueue = queueInit(QueueLen);

	// 钩子
	nfho_in.hook = my_hook_func_in;
	nfho_in.hooknum = NF_INET_LOCAL_IN;
	nfho_in.pf = PF_INET;
	nfho_in.priority = NF_IP_PRI_FIRST;
	nf_register_hook(&nfho_in);// 不同内核版本用不同版本 nf_register_net_hook  nf_register_hook

	createSocketMemoryPool(); // 创建socket内存池，链表等

	aPackageHandle[ServerSelfIntroduction] = serverSelfIntroduction;
	aPackageHandle[FINISH] = finHandle;
	aPackageHandle[SYNACK] = synAckHandle;
	aPackageHandle[FINACK] = finAckHandle;
	aPackageHandle[ToLogin] = toLogin;
	aPackageHandle[ToFTP] = toFTP;
	aPackageHandle[LoginSuccess] = loginSuccess;
	aPackageHandle[ToUSER] = toUser;
	
	/* 创建线程pthread，负责出队*/
	for (i = 0; i < DequeueThreadNum; i++)
		iThreadNumber[i] = i;
	for (i = 0; i < DequeueThreadNum; i++)
	{
		practice_deQueue_p[i] = kthread_run(threadDeQueue, (void *)&iThreadNumber[i], "kthread_run");
		if (IS_ERR(practice_deQueue_p[i]))
		{
			rc = PTR_ERR(practice_deQueue_p[i]);
			printk(" error %d create thread_name thread", rc);
		}
	}

	timer.function = OnlineTest;						   /* 设置定时处理函数 */
	timer.expires = jiffies + msecs_to_jiffies(1000 * 10); /* 超时时间 2 分钟 */
	add_timer(&timer);									   /* 启动定时器 */
	printk(KERN_INFO "hook in ok\n");
	return 0;
}

// 模块卸载函数
static void __exit GatewayDestroy(void)
{
	nf_unregister_hook(&nfho_in); // 不同内核版本用不同版本 nf_unregister_net_hook f nf_unregister_hook
	
	queueDestory((void *)pQueue);
	
	destorySocketMemoryPool(); // 销毁socket内存池，链表等
	
	del_timer(&timer);
	
	iExitFlag = 0;

	msleep(30);
}

module_init(GatewayInit);
module_exit(GatewayDestroy);

MODULE_AUTHOR("Gateway <64bit@kernel.org>");
MODULE_LICENSE("GPL v2");
