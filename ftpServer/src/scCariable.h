#ifndef scCarlable_H
#define scCarlable_H
#pragma pack(1)
struct sPack
{
	void* sSocket;//socket地址
	unsigned long long ullUserId;//用户id
	unsigned int ucEnumNumber;//枚举包类型
	unsigned int uiHeadLen;//头部长度
	unsigned int uiSeqNumber;//序列号
	unsigned int uiAckNumber;//确认号
	unsigned char uiOperationNumber;//操作玛
	unsigned char ucPackType;//是否为发送后的第一个ack
	unsigned int uiZonePackNumber;//分配包序列号
	unsigned short usSendWindSize;//发送滑动窗口
};
#pragma pack()

enum EPackageType
{
	ServerSelfIntroduction,
	Finsh,
	Synack,
	FINACK,
	HeartHop,
	ToLogin,
	
	ToFtp,
	
	LoginSuccess,
	
	ToUser
};

enum EProtocol
{
	Login=9,
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
	LoginFail=17,
	RegisterFail,
	RegisterSuccess,
	UpLoadFileAllow,
	UpLoadFileNotAllow,
	DownLoadFileAllow,
	DownLoadFileNotAllow,
	UpLoadFileFail,
	DownLoadFileFail
};
#endif
