#pragma once
#include <linux/netfilter.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/udp.h>
#include <linux/ip.h>
#include <linux/vmalloc.h>
#include <linux/tcp.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include <linux/timekeeping.h>
#include <linux/time.h>
#include <linux/kernel.h>
#include "socket.h"
typedef SSocket *(*packageHandle)(struct sk_buff *, struct SSocketAndID *); // 声明一个函数指针
extern packageHandle aPackageHandle[];                                      // 定义一个函数指针数组，大小是ToUser大小

void gatewayInit(void);
void gatewayDestroy(void);

SSocket *serverSelfIntroduction(struct sk_buff *pPackage, struct SSocketAndID *pstSocketID);
SSocket *finHandle(struct sk_buff *pPackage, struct SSocketAndID *pstSocketID);
SSocket *finAckHandle(struct sk_buff *pPackage, struct SSocketAndID *pstSocketID);

SSocket *heartHopHandle(struct sk_buff *pPackage, struct SSocketAndID *pstSocketID);
SSocket *toLogin(struct sk_buff *pPackage, struct SSocketAndID *pstSocketID);
SSocket *toFTP(struct sk_buff *pPackage, struct SSocketAndID *pstSocketID);
SSocket *loginSuccess(struct sk_buff *pPackage, struct SSocketAndID *pstSocketID);
SSocket *toUser(struct sk_buff *pPackage, struct SSocketAndID *pstSocketID);

int checkSocket(struct SSocketAndID *pstSocketID);                         // 校验socket地址是否正确，id是否对的上，， 返回值 1正确，0 错误的
int regroupPackage(struct sk_buff *pPackage, SSocket *pDestinationSocket); // 重组数据包
void sendPackage(struct sk_buff *pPackage);

// 组心跳包的函数。
void sendHeartHop(SSocket *pDestinationSocket); // 后面写
void OnlineTest(struct timer_list *arg);
