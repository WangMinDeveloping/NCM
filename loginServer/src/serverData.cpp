#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "database.h"
#include "structAndEnum.h"
#define TM_L 4 // 定义表示时间的字符长度
sqlite3 *db = 0;

const char *dbfile = "data.db"; // 定义数据库名称
int res;						// 返回值，错误码
char *errmsg = NULL;			// 用于存储返回的错误信息的指针
char **result;					// 用于保存查询结果
int row, col;					// 保存行列
const char *sqlcmd;				// 存sql语句的变量
// char * sqlcmd;//存sql语句的变量
char sqlcmdArr[2048] = {0}; // 存sql语句的变量

// char * packBuf = 0;
volatile int insertNum = 0;
volatile int queryNum = 0;

// 打印结果
int insertDisplay(void *para, int ncolumn, char **columnvalue, char *columnname[]) // 回调函数，一般用于显示
// 像查找函数，如果查到2条，就会被调用2次。
{
	// int i;
	insertNum++;
	// printf("insertNum = %d\n",insertNum);
	printf("total column is %d\n", ncolumn); // 列数
	/*for(i = 0; i < ncolumn; i++)
	{
		printf("%s \n",columnname[i]);
		printf("%s \n",columnvalue[i]);
		printf("%s   %s\n",columnname[i],columnvalue[i]);
	}
	printf("===============================\n");*/
	return 0;
}
int queryDisplay(void *para, int ncolumn, char **columnvalue, char *columnname[]) // 回调函数，一般用于显示
// 像查找函数，如果查到2条，就会被调用2次。
{
	// int i;
	queryNum++;
	// printf("queryNum = %d\n",queryNum);
	printf("total column is %d\n", ncolumn); // 列数
	/*for(i = 0; i < ncolumn; i++)
	{
		printf("%s \n",columnname[i]);
		printf("%s \n",columnvalue[i]);
		printf("%s   %s\n",columnname[i],columnvalue[i]);
	}
	printf("===============================\n");*/
	return 0;
}

// 注册成功：3	注册失败：2	登陆成功：1	登陆失败：0
int dataControl(char *packData, int control)
{
	int retNum = -2;
	////////打开数据库////////
	res = open_database(dbfile);
	if (res != 0)
	{
		printf("打开失败：%s", sqlite3_errmsg(db));
		return -1;
	}
	else
	{
		puts("打开成功");
	}

	////////创建表////////
	// 若无表则创建
	sqlcmd = "CREATE TABLE IF NOT EXISTS userList(\
				id long long PRIMARY KEY,\
				password varchar(255));";

	res = sqlite3_exec(db, sqlcmd, NULL, NULL, &errmsg);
	if (res != SQLITE_OK)
	{
		printf("创建执行失败，代码：%d-%s\n", res, errmsg);
	}

	//////////////执行逻辑/////////////
	if (0 == control) // 登陆
	{
		retNum = queryModel(packData, 24);
		if (0 == retNum) // 0是找到 1是未找到
		{
			retNum = 1; // 找到 登陆成功
		}
		else
		{
			retNum = 0; // 未找到 登陆失败
		}
	}

	// function();
	if (1 == control) // 注册
	{
		retNum = insertQueryModel(packData, 8);
		if (1 == retNum) // 1表示未找到，所以可以注册。
		{
			retNum = insertModel(packData, 24);
			if (0 == retNum)
				retNum = 3; // 注册成功
			else
				retNum = 2; // 注册失败
		}
		else if (0 == retNum) // 0表示找到了对应用户，所以注册失败。
		{
			retNum = 2; // 注册失败
		}
		else
		{
			// insertQueryModel()出问题了。
		}
	}
	////// /////关闭数据库////// /////
	res = close_database();
	if (res != 0)
	{
		printf("数据库关闭失败:%s", sqlite3_errmsg(db));
		return -1;
	}
	else
	{
		puts("数据库已关闭");
	}

	return retNum;
}

// 注册之前的查询，查询是否数据库中已有该用户//0找到 1未找到
// int insertQueryModel(const char * dataProcessBuf,int size)
int insertQueryModel(char *dataProcessBuf, int size)
{
	struct sUserProtocol *pSUP = (struct sUserProtocol *)(dataProcessBuf + sizeof(struct sTcpHead));
	int i, j;
	// SELECT * FROM userList where id = 1111;
	const char *queryPart1 = "SELECT * FROM userList where id = ";
	const char *queryPart2 = ";";

	int queryPart1Len = strlen(queryPart1);
	int queryPart2Len = strlen(queryPart2);

	int queryCount = queryNum;

	////// /////查询语句////// /////
	unsigned long long account = pSUP->ullUserId;
	int accountLen = 0;
	char accountArr[30] = {0}; // 如果是unsigned long long 是0到1 777 777 777 777 777 777 777
	sprintf(accountArr, "%lld", account);
	accountLen = strlen(accountArr); // 获取转换成字符串后的账号长度。

	// part1
	for (i = 0; i < queryPart1Len; i++)
	{
		sqlcmdArr[i] = queryPart1[i];
	}

	// 拷贝账号部分
	for (i=0, j = 0; j < accountLen; i++, j++)
	{
		sqlcmdArr[i] = accountArr[j];
	}

	// part2
	for (i=0, j = 0; j < queryPart2Len; i++, j++)
	{
		sqlcmdArr[i] = queryPart2[j];
	}

	sqlcmdArr[i] = 0;
	printf("sqlcmdArrLen = %ld\n", strlen(sqlcmdArr));
	printf("%s\n", sqlcmdArr);

	////////////////////////////////
	res = sqlite3_exec(db, sqlcmdArr, queryDisplay, NULL, &errmsg); // 填dispaly表示执行完之后调用回调函数display
	if (res != SQLITE_OK)
	{
		printf("p170 查询失败，失败代码：%d-%s\n", res, errmsg);
		return 2;
	}
	if (!(queryNum = (queryNum - queryCount)))
	{
		printf("未找到。\n");
		return 1;
	}
	else
	{
		printf("找到%d项记录。\n", queryNum);
	}

	return 0;
}

// 插入//成功0//失败1
// int insertModel(const char * dataProcessBuf,int size)
int insertModel(char *dataProcessBuf, int size)
{
	////// /////插入语句////// /////
	struct sUserProtocol *pSUP = (struct sUserProtocol *)(dataProcessBuf + sizeof(struct sTcpHead));
	unsigned int i, j;
	const char *insertPart1 = "INSERT INTO userList VALUES (";
	const char *insertPart2 = ");";
	const char comma = ',';
	const char singleQuote = '\'';
	size_t insertPart1Len = strlen(insertPart1);
	size_t insertPart2Len = strlen(insertPart2);
	size_t passwordLength = strlen(dataProcessBuf + sizeof(struct LoginServerRecv) - 16);
	unsigned long long account = pSUP->ullUserId;
	int accountLen = 0;
	char accountArr[30] = {0}; // 如果是unsigned long long 是0到1 777 777 777 777 777 777 777
	sprintf(accountArr, "%lld", account);
	accountLen = strlen(accountArr) + 1;
	accountArr[accountLen - 1] = comma;
	//	printf("accountLen = %d\n",accountLen);
	//	printf("accountArr = %s\n",accountArr);

	// 如果长度超过16就截断。
	if (passwordLength > 16)
	{
		passwordLength = 16;
	}

	// part1
	for (i = 0; i < insertPart1Len; i++)
	{
		sqlcmdArr[i] = insertPart1[i];
	}

	// 要把账号数字转换成字符串//一个字节不一定是小于9的数字
	for (i = 0, j = 0; j < (unsigned int)accountLen; i++, j++)
	{
		sqlcmdArr[i] = accountArr[j];
	}

	// 字符串的单引号
	sqlcmdArr[i] = singleQuote;
	i++;
	// 拷贝密码部分
	for (i = 0, j = (sizeof(struct LoginServerRecv) - 16); j < (sizeof(struct LoginServerRecv) - 16 + passwordLength); i++, j++)
	{
		sqlcmdArr[i] = dataProcessBuf[j];
		// printf("%#X ",dataProcessBuf[j]);
	}
	// printf("----------End\n");
	// sqlcmdArr[i] = 0;

	// 字符串的单引号
	sqlcmdArr[i] = singleQuote;
	i++;

	// part2
	for (i = 0, j = 0; j < insertPart2Len; i++, j++)
	{
		sqlcmdArr[i] = insertPart2[j];
	}
	sqlcmdArr[i] = 0;
	printf("sqlcmdArrLen = %ld\n", strlen(sqlcmdArr));
	printf("%s\n", sqlcmdArr);

	// sqlcmd = ;
	// sqlcmd="INSERT INTO userList VALUES (1,0,12345,'20180712120000',30,0.36,0.36);";
	// res=sqlite3_exec(db,sqlcmd,NULL,NULL,&errmsg);//插入数据
	res = sqlite3_exec(db, sqlcmdArr, NULL, NULL, &errmsg); // 插入数据
	if (res != SQLITE_OK)
	{
		printf("执行错误代码：%d-%s\n", res, errmsg);
		return 1;
	}

	return 0;
}

// 登陆//找到0，未找到1
// int queryModel(const char * dataProcessBuf,int size)
int queryModel(char *dataProcessBuf, int size)
{
	struct sUserProtocol *pSUP = (struct sUserProtocol *)(dataProcessBuf + sizeof(struct sTcpHead));
	size_t i, j;
	// SELECT * FROM userList where id = 1111 and password = 'AAAAAAAABBBBBBB1';
	const char *queryPart1 = "SELECT * FROM userList where id = ";
	const char *queryPart2 = " and password = '";
	const char *queryPart3 = "';";

	size_t queryPart1Len = strlen(queryPart1);
	size_t queryPart2Len = strlen(queryPart2);
	size_t queryPart3Len = strlen(queryPart3);
	size_t passwordLength = strlen(dataProcessBuf + sizeof(struct LoginServerRecv) - 16);
	int queryCount = queryNum; // 查询回调函数被调用次数

	////// /////查询语句////// /////
	unsigned long long account = pSUP->ullUserId;
	size_t accountLen = 0;
	char accountArr[30] = {0}; // 如果是unsigned long long 是0到1 777 777 777 777 777 777 777
	sprintf(accountArr, "%lld", account);
	accountLen = strlen(accountArr); // 获取转换成字符串后的账号长度。

	// 如果长度超过16就截断。
	if (passwordLength > 16)
	{
		passwordLength = 16;
	}

	// part1
	for (i = 0; i < queryPart1Len; i++)
	{
		sqlcmdArr[i] = queryPart1[i];
	}

	// 拷贝账号部分
	for (i = 0, j = 0; j < accountLen; i++, j++)
	{
		sqlcmdArr[i] = accountArr[j];
	}

	// part2
	for (i = 0, j = 0; j < queryPart2Len; i++, j++)
	{
		sqlcmdArr[i] = queryPart2[j];
	}

	// 拷贝密码部分
	for (i = 0, j = (sizeof(struct LoginServerRecv) - 16); j < (sizeof(struct LoginServerRecv) - 16 + passwordLength); i++, j++)
	{
		sqlcmdArr[i] = dataProcessBuf[j];
	}
	// sqlcmdArr[i] = 0;

	// part3
	for (i = 0, j = 0; j < queryPart3Len; i++, j++)
	{
		sqlcmdArr[i] = queryPart3[j];
	}

	sqlcmdArr[i] = 0;
	printf("sqlcmdArrLen = %ld\n", strlen(sqlcmdArr));
	printf("%s\n", sqlcmdArr);

	////////////////////////////////
	res = sqlite3_exec(db, sqlcmdArr, queryDisplay, NULL, &errmsg); // 填dispaly表示执行完之后调用回调函数display
	if (res != SQLITE_OK)
	{
		printf("p312 查询失败，失败代码：%d-%s\n", res, errmsg);
		return 2;
	}
	if (!(queryNum = (queryNum - queryCount)))
	{
		printf("未找到。\n");
		return 1;
	}
	else
	{
		printf("找到%d项记录。\n", queryNum);
	}

	return 0;
}

/*int queryModel(const char * dataProcessBuf,int size)
{
	int queryCount = queryNum;
	////// /////查询语句////// /////
	sqlcmd = "SELECT * FROM userList";
	res= sqlite3_exec(db,sqlcmd,queryDisplay,NULL,&errmsg);//填dispaly表示执行完之后调用回调函数display
	if(res!=SQLITE_OK)
	{
		printf("查询失败，失败代码：%d-%s\n",res,errmsg);
	}
	if(!( queryNum = (queryNum-queryCount)))
	{
		printf("未找到。\n");
	}else
	{
		printf("找到%d项记录。\n",queryNum);
	}

	//else {//puts("查询成功");}
	return 0;
}*/
int updataModel()
{
	////// /////修改语句////// /////
	/*	sqlcmd = "UPDATE userList set telnum = 678910 where id=1;" \
			  "SELECT * from userList";
		res=sqlite3_exec(db,sqlcmd,display,NULL,&errmsg);//更改数据
		if(res!=SQLITE_OK)
		{
			printf("修改失败，失败代码：%d-%s\n",res,errmsg);
		}
		//else {//puts("修改成功");}
	*/
	return 0;
}

int function()
{

	////// /////插入语句////// /////
	sqlcmd = "INSERT INTO userList VALUES (1,0,12345,'20180712120000',30,0.36,0.36);";
	res = sqlite3_exec(db, sqlcmd, NULL, NULL, &errmsg); // 插入数据
	if (res != SQLITE_OK)
	{
		printf("执行错误代码：%d-%s\n", res, errmsg);
	}
	// else {//puts("添加成功");//成功不打印不然多余信息会很多。}

	////// /////查询语句////// /////
	sqlcmd = "SELECT * FROM userList";
	res = sqlite3_exec(db, sqlcmd, queryDisplay, NULL, &errmsg); // 填dispaly表示执行完之后调用回调函数display
	if (res != SQLITE_OK)
	{
		printf("查询失败，失败代码：%d-%s\n", res, errmsg);
	}
	// else {//puts("查询成功");}

	////// /////修改语句////// /////
	sqlcmd = "UPDATE userList set telnum = 678910 where id=1;"
			 "SELECT * from userList";
	res = sqlite3_exec(db, sqlcmd, NULL, NULL, &errmsg); // 更改数据
	if (res != SQLITE_OK)
	{
		printf("修改失败，失败代码：%d-%s\n", res, errmsg);
	}
	// else {//puts("修改成功");}

	return 0;
}

int open_database(const char *dbfile)
{
	int result;
	result = sqlite3_open(dbfile, &db);
	return result;
}
int close_database()
{
	int result;
	result = sqlite3_close(db);
	return result;
}
