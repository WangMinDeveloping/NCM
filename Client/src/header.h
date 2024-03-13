#pragma once
//原始套接字
//后期功能：一个包里面有多个用户单包，用户没有收到ack不发下一个
//目前：一个包一个用户单包，用户没有收到ack不发下一个
//连包现占包位置。
//包头结构体
enum EErrorCode
{
	ECodeLoginFail,
	ECodeLoginSuccess,
	ECodeRegisterFail,
	ECodeRegisterSuccess,
	ECodeUpLoadFileAllow,
	ECodeUpLoadFileNotAllow,
	ECodeDownLoadFileAllow,
	ECodeDownLoadFileNotAllow,
	ECodeUpLoadFileFail,
	ECodeDownLoadFileFail
};

enum EPackageType : short
{
	ServerSelfIntroduction,
	Finsh,
	FINACK,
	HeartHop,
	ToLogin,
	ToFtp,
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
#pragma pack(1)
struct sTcpHead
{
	unsigned short wSrcPort;//源端口
	unsigned short wDstPort;//目的端口
	enum EPackageType wEnumPackType;//枚举包
	unsigned short wAckFirstPack;//是否为ack后的第一个包
	unsigned int uSeqNumber;//序列号
	unsigned int uAckNumber;//确认好
};
struct sUserProtocol
{
	unsigned short wParametersLength;//参数长度
	unsigned short wCheckWord;//校验字
	enum EOperationCode uUserOperation;//操作码
	void* pSocket;//socket地址
	unsigned long long ullUserId;//用户id
};
struct SMacHeader
{
	unsigned char aucArmMac[6];
	unsigned char aucSourceMac[6];
	unsigned short wProtocol;
};

struct sIpData
{
	struct sTcpHead TcpHead;
	struct sUserProtocol UserHead;
};
//数据部分结构体
//FTP发送给用户
struct sFTPSendAck
{
	unsigned int uMinAckNumber;
	unsigned int uMaxAckNumber;
};
struct sFTPSendError
{
	enum EErrorCode uiError;
};

//登陆结构体
struct sIpInformation
{
	unsigned int uIpAddress;
	unsigned short wIpPort;
};

union loginParameter
{
	enum EErrorCode uErrorCode;
	unsigned char ucPassword[16];//用户发的密码
};
//登陆服务器回发
struct sLoginParameter
{ 
	struct sIpInformation Ip;
	union loginParameter uOperationParameter;
};
//用户上传文件直接强转后续char
//用户登录和注册网关不建立socket，只该端口，ip，当登陆服务器回LoginSuccess时，建立用户socket
struct SSocketSeq
{
	unsigned int uNextSendSeq;
	unsigned int uNextSendAck;
	unsigned int uSendPackageCount;//当前允许发送的最大数量
	unsigned int uMaxSendPackageCount;//最大可以一次性发送的包数量
	unsigned int uNextReceiveSeq;//push包查看
	unsigned int uNextRecvAck;//ack包 查看
	unsigned int uMaxRecvAck;//sack 查看

};
struct SResendPackage  //链表结点，便于重发
{
	unsigned char *pPackage;
	int iPackageLen;
};
struct SIPHeader
{
unsigned int bitIPHeaderLen :4;
unsigned int bitIPVersion :4;
unsigned char ucDifferentiatServices;
unsigned short int wLenTotal;
unsigned short int wIdentification;
unsigned int bitFlags :3;
unsigned int bitSliceOffset :13;
unsigned char ucTTL;
unsigned char ucProtocol;
unsigned short int wHeaderCheckSum;
unsigned int uSourceIP;
unsigned int uArmIP;
};


struct SMACHeader
{
        unsigned char aucArmMac[6];
        unsigned char  aucSourceMac[6];
        unsigned short int wProtocol;
};

struct SMACIPHeader
{
	struct SMACHeader stMACHeader;
	struct SIPHeader stIPHeader;
	struct sTcpHead stTCPHeader;
	struct sUserProtocol stUserHeader;
	
};

#pragma pack()
