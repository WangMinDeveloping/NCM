#ifndef sUserSock_h
#define sUserSock_h
#include"CRedBlackTree.h"
#include<signal.h>
#include<time.h>
class CUserSock
{
	public:
		unsigned short wSendWindowSize;//不使用
		unsigned short wRecWindowSize;//不使用
		void* sSocket;//网关的socket地址
		unsigned long long ullUserId;//用户id
		unsigned int uRecvWinLeftSeq;//期待的小包序列号
		unsigned int uRecvWinRightSeq;//收到包有序的最大的值
		unsigned int uAckCount;//回发给用户的ack，保存的用户的seq
		unsigned int uSeqCount;//回发给用户的seq，保存的用户的ack
		unsigned int uTimer;//发送ack的间隔时间
		unsigned long long uLastTimer;//上一次发送ack的系统时间
		timer_t sTimerId;//time结构体
		struct sigevent evp;//time结构体的设置
		struct itimerspec tick;//定时函数循环时间设置
		int iLock;//这个用户的btstl锁
		CRedBlackTree pRecvPackTree;//管理包和包排序
		CUserSock();
		static void AckSackSendFunction(union sigval v);//ack或sack发送函数
	protected:
	private:
};
#endif
