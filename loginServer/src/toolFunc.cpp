#include"toolFunc.h"
//2字节倒置
void invertWord(unsigned char *vData)
{
	unsigned char temp = vData[0];
	vData[0] = vData[1];
	vData[1] = temp;
}
unsigned short invertWord(unsigned short Data){return ((Data<<8)+(Data>>8));}

//倒置函数	//倒置地址	//数据长度
void invertFunction(void * vData,int length)
{
	unsigned char * pData = (unsigned char *)vData;
	int front = 0;
	int rear = length - 1;
	int temp;
	while(front < rear)
	{
		temp = pData[front];
		pData[front] = pData[rear];
		pData[rear] = temp;

		front++;
		rear--;
	}

}

