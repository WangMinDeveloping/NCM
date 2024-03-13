#include<stdio.h>
#include "structAndEnum.h"
#include "database.h"
//#include "serverData.c"
#include "analysis.h"

int TCPData_analysis(void* pBuf,int size)
{
	struct LoginServerSend * pSLSS = (struct LoginServerSend *)pBuf;

	int retNum = 0;

	enum EOperationCode controlNum = ((pSLSS->sIPD).UserHead).uUserOperation;
	//这里可以优化if
	//retNum	注册成功：3	注册失败：2	登陆成功：1	登陆失败：0
	if(controlNum == Login)
	{
		printf("开始登录\n");
		retNum = dataControl((char *)pBuf,controlNum);
	}else if(controlNum == Register)
	{
		printf("开始注册\n");
		retNum = dataControl((char *)pBuf,controlNum);
	}

	//可以直接retNum = dataControl((char *)pBuf,sLSS->ucEPackageControl);
	//retNum//这里ErrorCode
	printf("analysis.c p28 :eCoed = %d\n",retNum);

	return retNum;
}
