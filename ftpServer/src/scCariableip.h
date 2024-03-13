#ifndef scCarlable_H
#define scCarlable_H
extern unsigned int sIpDataLen;
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
	Push
};

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
#pragma pack(1)
//原始套接字
//后期功能：一个包里面有多个用户单包，用户没有收到ack不发下一个

//目前：一个包一个用户单包，用户没有收到ack不发下一个
//连包现占包位置。
//包头结构体
struct sTcpHead
{
	unsigned short wSrcPort;//源端口
	unsigned short wDstPort;//目的端口
	unsigned short usLen;//ip包长度
	unsigned short usCheck;//校验为
	unsigned int uSeqNumber;//序列号
	unsigned int uAckNumber;//确认号
	enum EPackageType wEnumPackType;//包类型
};


struct sUserProtocol
{
	unsigned short wParametersLength;//参数长度
	unsigned short wCheckWord;//校验字
	enum EOperationCode uUserOperation;//操作码
	void* pSocket;//socket地址
	unsigned long long ullUserId;//用户id
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
	unsigned int uiSeqnumber;
	unsigned int uiAcknumber;
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

//登陆服务器回发
struct sLonginParameter
{	
	struct sIpInformation Ip;
	enum EErrorCode uErrorCode;
};

//用户给登陆服务器
struct sUserLoginParameter
{
	struct sIpInformation Ip;
	unsigned char ucPassword[16];
};
//用户发给ftp,现不使用
struct sUserFtpParamter
{
	char cFileName[10]; 
};
//用户上传文件直接强转后续char
//用户登录和注册网关不建立socket，只该端口，ip，当登陆服务器回LoginSuccess时，建立用户socket
#pragma pack()
#endif
