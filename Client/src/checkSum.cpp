#include"checksum.h"
unsigned short checkSum(unsigned short * addr, int count)
{
	unsigned int sum = 0;

/*
    计算所有数据的16bit对之和
*/
	while(count > 1)
	{
		/*  This is the inner loop */
//		printf("\n*(unsigned short*)addr=%#X\n",*(unsigned short*)addr);//checking
		sum += *(unsigned short*)addr++;
//		printf("sum = %#X\n",sum);
		count -= 2;
	}

/*
    如果数据长度为奇数，在该字节之后补一个字节(0),
   然后将其转换为16bit整数，加到上面计算的校验和中
*/
	if(count > 0)
	{
		char left_over[2] = {0};
		left_over[0] = *addr;
		sum += *(unsigned short*) left_over;
	}

/*
    将32bit数据压缩成16bit数据，即将进位加大校验和
　　的低字节上，直到没有进位为止。
*/
	while (sum>>16)
	{
		sum = (sum & 0xffff) + (sum >> 16);
	}
/*返回校验和的反码*/
//	printf("\nsum = %#X\n",sum);
//	printf("~sum = %#X\n",~sum);
   return ~sum;
}
