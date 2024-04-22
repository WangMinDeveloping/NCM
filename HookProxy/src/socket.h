#pragma once
#define PACKAGE_TYPE_INITIAL_VALUE 0
#define SOCKET_TOTAL 65535
#define SERVER_TOTAL 2
#define _DEBUG
//包类型
enum EPackageType
{
	ServerSelfIntroduction=PACKAGE_TYPE_INITIAL_VALUE,
	Finish,
	FINACK,
	HeartHop,
	ToLogin,
	ToFTP,
	LoginSuccess,
	ToUser
};
enum EOperationCode
{
        Login,
        Register,
        AskFTPUpLoadFile,
        AskFTPDownLoadFile,
        FTPFileOver,
        SAck,
        Ack,
        Push //push 上传的文件包
};


typedef  struct  SSocket
{
	unsigned  long long ullUserID; //功能服务器发包给用户时，找到socket指针 核查用户ID，避免多用户重复使用socket指针
	unsigned int uArmIPAddress;//用户的地址,大端
	unsigned short wArmPort;//用户的端口,大端
	void *pHeartHopSelfNodeAddress;//检测用户在线的链表中自己的节点地址
	//Int iSocketState;//标识全连接，半连接，分手状态
	unsigned long long ullOnlineTime;//记录在线时间，用于检测用户是否在线，每收到一次用户的包更新一次
}SSocket;


#pragma pack(1)
struct SSocketAndID
{
	SSocket  *pDestinationSocket;
	unsigned long long ullUserID;
};
struct SUserIPAndPort
{
	unsigned int uIPAddress;
	unsigned short wPort;
	unsigned char aucPassword[16];
};


/*为了各种包类型统一处理，心跳包组包所需结构体如下，以空间换时间*/
struct STCPHeader
{
	unsigned short wSrcPort;//源端口
	unsigned short wDstPort;//目的端口
	unsigned short wEnumPackType;//枚举包
	unsigned short wAckFirstPack;//是否为ack后的第一个包
	unsigned int uSeqNumber;//序列号
	unsigned int uAckNumber;//确认好
};
struct SUserProtocol
{
	unsigned short wParametersLength;//参数长度
	unsigned short wCheckWord;//校验字
	unsigned int uOperationCode;
	struct SSocketAndID stSocketID;
};
struct SIPData
{
	struct STCPHeader stTCPHeader;
	struct SUserProtocol stUserProtocolHeader;
};
struct SLoginParameter
{
	struct SIPData stIPData;
	struct SUserIPAndPort stUserIPPort;
};



#pragma pack()


/*extern unsigned char aucArmMac[6];
  extern unsigned char aucSourceMac[6];//本机MAC
  extern unsigned int uSourceIP;
  extern unsigned short int wSourcePort;
  extern SSocket* (*pstServerSocket)[];
  extern void *pSocketMemoryPool;
  extern void *pAllSocketList;*/
