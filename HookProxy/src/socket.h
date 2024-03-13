#pragma once
#define PACKAGE_TYPE_INITIAL_VALUE 0
#define SOCKET_TOTAL 65535
#define SERVER_TOTAL 2
#define _DEBUG
// 包类型
enum EProtocol
{
	ServerSelfIntroduction = PACKAGE_TYPE_INITIAL_VALUE, // 服务器自我介绍
	FINISH,												 // 分手包，回一个FIN+ACK，半分手状态1
	SYNACK,												 // 半连接变全连接，客户端回，暂时没有函数，姑且认为不必要
	FINACK,												 // socket入栈
	HeartHop,
	ToLogin,
	ToFTP,
	LoginSuccess, // 创建socket，半连接状态
	ToUSER
};

typedef struct SSocket
{
	unsigned long long ullUserID;	// 功能服务器发包给用户时，找到socket指针 核查用户ID，避免多用户重复使用socket指针
	unsigned int uArmIPAddress;		// 用户的地址,大端
	unsigned short wArmPort;		// 用户的端口,大端
	void *pHeartHopSelfNodeAddress; // 检测用户在线的链表中自己的节点地址
	
	// Int iSocketState;//标识全连接，半连接，分手状态
	unsigned long long ullOnlineTime; // 记录在线时间，用于检测用户是否在线，每收到一次用户的包更新一次
} SSocket;

#pragma pack(1)
struct STransportLayerHeader
{
	SSocket *pDestinationSocket;
	unsigned long long ullUserID;
	int iPackageType;
};

// 用于UDP校验,UDP伪首部
struct SUDPFakeHeader
{
	unsigned int uSourceIP;
	unsigned int uArmIP;
	unsigned char ucZero;
	unsigned char ucProtocol;
	unsigned short int wLen;
};
#pragma pack()

/*extern unsigned char aucArmMac[6];
extern unsigned char aucSourceMac[6];//本机MAC
extern unsigned int uSourceIP;
extern unsigned short int wSourcePort;
extern SSocket* (*pstServerSocket)[];
extern void *pSocketMemoryPool;
extern void *pAllSocketList;*/
