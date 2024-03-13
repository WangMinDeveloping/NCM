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
#include "cppmod.h"
#include "ipHandle.h"

/*一入多出队列*/
int *pQueue=0;
unsigned int uQueueLen=1024;

struct timer_list timer; /* 定义定时器 */

//钩子声明
// Define some structs that will be used
static struct nf_hook_ops nfho_in;//钩子函数结构体
struct iphdr *pIPHeader;//skb的ip头部：第一次得到，用于判断
struct udphdr *pUDPHeader;
struct task_struct *practice_deQueue_p[10] = {};//线程返回值
volatile int iExitFlag=1;
//函数指针数组初始化
typedef SSocket* (*packageHandle)(struct iphdr*,struct STransportLayerHeader *);//声明一个函数指针
packageHandle aPackageHandle[ToUSER+1];//定义一个函数指针数组，大小是ToUser大小

/*出队线程*/
int threadDeQueue(void *pThreadID)
{
	struct sk_buff *pPackage;
	 struct iphdr *pIPHeader;
	struct STransportLayerHeader *pSelfTCPHeader;
	SSocket *pDestinationSocket;
	int iSendSuccess;
	while(iExitFlag)
	{
		pPackage=(struct sk_buff*)deQueue_c(pQueue);//出队一个数据包
		if(pPackage!=NULL)//出队成功，非空
		{
			  pIPHeader=ip_hdr(pPackage);
			  pSelfTCPHeader=(struct STransportLayerHeader *)((char *)pIPHeader+pIPHeader->ihl*4+8);
			  if(pSelfTCPHeader->iPackageType>=PACKAGE_TYPE_INITIAL_VALUE&&pSelfTCPHeader->iPackageType<ToUSER)//校验包类型是否正确
			  {
				pDestinationSocket=aPackageHandle[pSelfTCPHeader->iPackageType](pIPHeader,pSelfTCPHeader);
      			        if(pDestinationSocket>(SSocket *)1)//返回的目的socket正确
				{
					iSendSuccess=regroupPackage(pPackage,pDestinationSocket);
#ifdef _DEBUG
					if(iSendSuccess==1)
						printk("发送数据包失败\n");
#endif
				}	
			 }
#ifdef _DEBUG
		         else
				printk("包类型不符合协议\n");
#endif
		}
	}
	return 0;
}


unsigned int my_hook_func_in(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{

	// 这里是ip头
	pIPHeader = (struct iphdr *)skb_network_header(skb);
	if(pIPHeader->protocol==17) //UDP协议
	{
		pUDPHeader = (struct udphdr *)skb_transport_header(skb);
		if(pUDPHeader->dest==htons(8081))//指定端口
		{
			if(enQueue_c(pQueue,(void *)skb)==0)
				return  NF_STOLEN;//接管数据包
			else
				return NF_DROP;//队满。丢了
		}
	}
	return NF_ACCEPT;
}

//模块初始化
static int __init char_test_init(void) //初始化模块
{
	int i,rc;
	int iThreadNumber[10];
	pQueue=queueInit(uQueueLen);
	 for( i=0;i<10;i++)
	         iThreadNumber[i]=i;
	//钩子,到本电脑初始化
	// Init the struct nfho
	nfho_in.hook = my_hook_func_in;
	nfho_in.hooknum = NF_INET_LOCAL_IN;
	nfho_in.pf = PF_INET;
	nfho_in.priority =  NF_IP_PRI_FIRST;
	// Register the hook function by the struct nfho
	// This function has already update t2o nf_register_net_hook from nf_register_hook
	nf_register_hook(&nfho_in);

	createSocketMemoryPool();//创建socket内存池，链表等
	
	aPackageHandle[ServerSelfIntroduction]=serverSelfIntroduction;
	aPackageHandle[FINISH]=finHandle;
	aPackageHandle[SYNACK]=synAckHandle;
	aPackageHandle[FINACK]=finAckHandle;
	aPackageHandle[ToLogin]=toLogin;
	aPackageHandle[ToFTP]=toFTP;
	aPackageHandle[LoginSuccess]= loginSuccess;
	aPackageHandle[ToUSER]=toUser;

	 /* 创建线程pthread，负责出队*/
         for( i=0;i<1;i++)
         {
                 practice_deQueue_p[i] = kthread_run(threadDeQueue,(void*)&iThreadNumber[i],"kthread_run");
                 if (IS_ERR(practice_deQueue_p[i]))
                 {
                         rc = PTR_ERR(practice_deQueue_p[i]);
                         printk(" error %d create thread_name thread", rc);
                 }
#ifdef BUG
                 BUG_ON(!practice_deQueue_p[i]);
#endif
         }
	timer.function = OnlineTest; /* 设置定时处理函数 */
	timer.expires=jiffies + msecs_to_jiffies(1000*10);/* 超时时间 2 分钟 */
	add_timer(&timer); /* 启动定时器 */
	printk(KERN_INFO "hook in ok\n");
	return 0;
}
module_init(char_test_init);

//模块卸载函数
static void __exit char_test_exit(void)
{
	//钩子
	// Unregister the hook function
	// This function has also already update to nf_unregister_net_hook from nf_unregister_hook
	 queueDestory((void*)pQueue);
	destorySocketMemoryPool();//销毁socket内存池，链表等
	nf_unregister_hook(&nfho_in);
	del_timer(&timer);
	iExitFlag=0;
	msleep(30);
}
module_exit(char_test_exit);

MODULE_AUTHOR("Barry Song <baohua@kernel.org>");
MODULE_LICENSE("GPL v2");
