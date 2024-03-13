#include"memorypool.h"
#include"new.h"
CMemoryPool::CMemoryPool(int iCount,int iSize)
{
	pMemory=new char[iSize*iCount];
	pStackArray=(void* (*)[])new void*[iCount]();
	for(iStackTop=0;iStackTop<iCount;iStackTop++) //栈顶下标代表栈中的元素个数
		(*pStackArray)[iStackTop]=(void*)(pMemory+iStackTop*iSize);//将分配好的空间按每Size大小的块内存的地址入栈
	iMaximum=iCount;
	pMaxAddress=pMemory+(iMaximum-1)*iSize;
}
CMemoryPool::~CMemoryPool()
{
	delete []pMemory;
	delete []pStackArray;
	pStackArray=0;
	pMemory=0;
}
void* CMemoryPool::getMinAddress()
{
	return (void *)pMemory;
}

void* CMemoryPool::getMaxAddress()
{
	return (void *)pMaxAddress;
}

/***********************************************************
  入栈函数:
  输入：1.需要入栈的指针
  返回值：0、成功；1、栈满
 ***********************************************************/
int CMemoryPool::push(void *pMember)
{


	//正确版本
 	   int full=1;
           asm("\
           movl %4,%%ecx; /*最大栈长*/\
           mov %3,%%r9;/*需要入栈的数据*/\
           movq %5,%%r10;/*队列数组首字节地址*/ \
    	   mov %2,%%eax;  /*栈顶下标*/ \
 loopPush: cmp %%ecx,%%eax; /*判断当前栈中个数是否小于最大栈容纳个数*/ \
           jb push;/*当前栈中个数小于最大栈容纳个数跳转*/\
	   mov $1,%%eax;\
	   jmp pushExit;\
     push: mov %%eax,%%r14d; /*栈顶下标*/ \
           inc %%r14d;/*栈顶下标加一*/ \
           lock cmpxchgl %%r14d,%0;/*抢下标*/\
           jnz loopPush;/*没有抢到栈顶就循环，iStackTop取出来放在了eax里面*/\
	   cltd;\
           lea (%%r10,%%rax,8),%%r11;/*r10存储队尾下标元素的首字节地址*/\
addMember: mov $0,%%rax;\
	   lock cmpxchg %%r9,(%%r11);\
	   jnz addMember;\
	   mov $0,%%eax;\
  pushExit:movl %%eax,%1;"\
           :"=m"(iStackTop),"=m"(full)\
           :"m"(iStackTop),"m"(pMember),"m"(iMaximum),"m"(pStackArray)\
           :"%rax","%rcx","rdx","%r9","%r11","%r10","%r14");
	return full;
	

}
/********************************************************
  出栈函数
  输入：无
  返回值：大于1：出队指针；0；队空
 ********************************************************/

void* CMemoryPool::pop()
{
	//最后版本
	void *pMember=0;
	 asm volatile("\
                        movl %3,%%ecx; /*最大栈长*/\
                        movq %4,%%r10;/*队列数组首字节地址*/ \
                        mov %2,%%eax;  /*栈顶下标*/ \
               loopPop: cmp $0,%%eax; /*判断栈空*/ \
                        jg pop; /*不空则出栈*/ \
                        mov $0,%%rax;/*栈为空返回1*/ \
                        jmp popExit;\
                    pop:mov %%eax,%%r14d; /*栈顶下标*/ \
                        dec %%r14d;/*栈顶减1*/ \
                        lock cmpxchgl %%r14d,%0;/*内存与寄存器的序号相等，则内存栈顶减1，不相等内存栈顶存入eax*/\
                        jnz loopPop;\
			movslq %%r14d,%%r14;\
                        lea (%%r10,%%r14,8),%%r11;/*r10存储栈顶下标元素的首字节地址*/\
			mov $0,%%rax;\
	      outMember:xchg (%%r11),%%rax;\
			test %%rax,%%rax;\
			jz outMember;\
     	         popExit:mov %%rax,%1;"\
                        /*sti; 开中断*/ \
                        :"+m"(iStackTop),"=m"(pMember)\
                        :"m"(iStackTop),"m"(iMaximum),"m"(pStackArray)\
                        :"%rax","%rcx","%r10","%r11","%r14");
	 return pMember;
}
