#ifndef scCarlable_H
#define scCarlable_H

enum EPackageType : short
{
	ServerSelfIntroduction,
	Finish,
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
struct sTcpHead//18
{
	unsigned short wSrcPort;//源端口
	unsigned short wDstPort;//目的端口
	unsigned short wUDPLength;//udp长度，udp表头+udp数据段的总长度
	unsigned short wUDPCheckWord;//udp的校验和
	unsigned int uSeqNumber;//序列号
	unsigned int uAckNumber;//确认好
	enum EPackageType wEnumPackType;//枚举包
};


struct sUserProtocol//24
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
////////////////////////////////////////////////
struct LoginServerRecv
{
	struct sIpData sIPD;
	struct sUserLoginParameter sULP;
};

struct LoginServerSend
{
	struct sIpData sIPD;
	struct sLonginParameter sLP;
};
////////////////////////////////////////////////
//用户发给ftp,现不使用
struct sUserFtpParameter
{
	char cFileName[10];
};

struct IPHeader	//这是倒拼
{
	unsigned int uArmIP;
	unsigned int uSourceIP;
	unsigned short int wHeaderCheckSum;
	unsigned char ucProtocol;
	unsigned char ucTTL;
	unsigned int bitSliceOffset :13;
	unsigned int bitFlags :3;
	unsigned short int wIdentification;
	unsigned short int wLenTotal;
	unsigned char ucDifferentiatServices;
	unsigned int bitIPHeaderLen :4;
	unsigned int bitIPVersion :4;
};
//用户上传文件直接强转后续char
//用户登录和注册网关不建立socket，只该端口，ip，当登陆服务器回LoginSuccess时，建立用户socket
#pragma pack()

#endif

