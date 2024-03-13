unsigned char* invertBytes(unsigned char *pSource,unsigned char *pDestination,int length)
{
	int front = 0;
	int rear = length - 1;
	while(length--)
	{
		pDestination[front] = pSource[rear];
		front++;
		rear--;
	}

	return pDestination;
}

unsigned int doubleSum(unsigned short * addr, int count)
{
	unsigned int sum = 0;

	/* 计算所有数据的16bit对之和*/
	while( count > 1  )
	{
		/*  This is the inner loop */

		sum += *(unsigned short*)addr++;
		count -= 2;
	}
	return sum;
}

unsigned short invertShort(unsigned short wData)
{
	unsigned char *pc=(unsigned char *)&wData;
	unsigned char ucTemp;
	ucTemp=pc[0];
	pc[0]=pc[1];
	pc[1]=ucTemp;
	return wData;
}
unsigned int invertInt(unsigned int uData)
{
	unsigned char *pc=(unsigned char *)&uData;
	unsigned char ucTemp;
	int i=0;
	int j=3;
	while(i<j)
	{
		ucTemp=pc[i];
		pc[i]=pc[j];
		pc[j]=ucTemp;
		i++;
		j--;
	}
	return uData;
}
