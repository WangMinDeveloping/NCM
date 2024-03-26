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

void createSocketMemoryPool(void);
void destorySocketMemoryPool(void);

SSocket *serverSelfIntroduction(struct iphdr *pIPHeader, struct STransportLayerHeader *pSelfTCPHeader);
SSocket *finHandle(struct iphdr *pIPHeader, struct STransportLayerHeader *pSelfTCPHeader);
SSocket *synAckHandle(struct iphdr *pIPHeader, struct STransportLayerHeader *pSelfTCPHeader); // 暂时空着
SSocket *finAckHandle(struct iphdr *pIPHeader, struct STransportLayerHeader *pSelfTCPHeader);

SSocket *heartHopHandle(struct iphdr *pIPHeader, struct STransportLayerHeader *pSelfTCPHeader);
SSocket *toLogin(struct iphdr *pIPHeader, struct STransportLayerHeader *pSelfTCPHeader);
SSocket *toFTP(struct iphdr *pIPHeader, struct STransportLayerHeader *pSelfTCPHeader);
SSocket *loginSuccess(struct iphdr *pIPHeader, struct STransportLayerHeader *pSelfTCPHeader);
SSocket *toUser(struct iphdr *pIPHeader, struct STransportLayerHeader *pSelfTCPHeader);

int checkSocket(struct STransportLayerHeader *pSelfTCPHeader);             // 校验socket地址是否正确，id是否对的上，， 返回值 1正确，0 错误的
int regroupPackage(struct sk_buff *pPackage, SSocket *pDestinationSocket); // 重组数据包

// 组心跳包的函数。
void sendHeartHop(SSocket *pDestinationSocket); // 后面写
void OnlineTest(struct timer_list *arg);