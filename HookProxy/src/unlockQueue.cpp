#include"unlockQueue.h"
#include"new.h"
Queue::Queue(int iQueueLen)
{
	pArray=(void* (*)[])new void*[iQueueLen]();
	for(int i=0;i<iQueueLen;i++)
		(*pArray)[i]=(void*)0;
	iMaximum=iQueueLen;
	llFront=0;
	llRear=0;
}
Queue::~Queue()
{
	delete []pArray;
	pArray=0;
}


/*获取队列长度*/
int Queue::getQueueLength()
{
	return iMaximum;
}

/*获取队列里成员个数*/
int Queue::getQueueMemberCount()
{
	return llRear-llFront;
}
/***********************************************************
  入队函数:
  输入：1、队列对象指针；2、入队变量指针
  返回值：0、成功；1、队满
 ***********************************************************/
int Queue::_enQueue(void *pMember)
{
	//正确版本
 	   int full=1;
           asm("\
           movl %4,%%ecx; /*最大队长*/\
           movslq %%ecx,%%rcx;/* ecx符号扩展为rcx*/ \
           mov %3,%%r9;/*需要入队的数据*/\
           movq %6,%%r10;/*队列数组首字节地址*/ \
    	   mov %2,%%rax;  /*队尾序号*/ \
   loopEn: mov %5,%%r15; /*队头序号*/ \
           sub %%r15,%%rax; /*当前队长*/ \
           cmp %%rcx,%%rax; /*判断当前队长是否等于最大队长*/ \
           jnz enQueue;\
	   mov $1,%%eax;\
	   jmp enQueueExit;\
  enQueue: add %%r15,%%rax; /*队尾序号恢复 cqto;队尾序号的符号位放入rdx*/ \
           mov %%rax,%%r14; /*队尾序号*/ \
           inc %%r14;/*寄存器队尾序号加a1*/ \
           lock cmpxchgq %%r14,%0;/*抢下标*/\
           jnz loopEn;/*没有抢到下标就循环，llRear取出来放在了rax里面*/\
           cqto;\
           idiv %%rcx;/*队尾下标对队长取余,余值在rdx*/ \
           lea (%%r10,%%rdx,8),%%r11;/*r10存储队尾下标元素的首字节地址*/\
addMember: mov $0,%%rax;\
	   lock cmpxchg %%r9,(%%r11);\
	   jnz addMember;\
	   mov $0,%%eax;\
enQueueExit:movl %%eax,%1;"\
           :"=m"(llRear),"=m"(full)\
           :"m"(llRear),"m"(pMember),"m"(iMaximum),"m"(llFront),"m"(pArray)\
           :"%rax","%rbx","%rdx","%rcx","%r9","%r11","%r10","%r14","%r15");
	return full;
	


/* asm("enQueueloop:");
 long long llOldRear=llRear;
 int iFull=1;
 long long llRealRear=llOldRear%iMaximum;
 if(llOldRear-llFront==iMaximum) return iFull;
 asm("\
                 mov %4,%%rax;\
                 mov %%rax,%%rbx;\
		 mov %5,%%rcx;\
		 add $1,%%rbx;\
                 lock cmpxchg %%rbx,%1;\
                 jnz enQueueloop;\
     addMember:  mov $0,%%rax;\
		 lock cmpxchg %%rcx,%0;\
		 jnz addMember;\
                 addl $-1,%3;"\
                 :"+m"((*pArray)[llRealRear]),"+m"(llRear),"=m"(iMemberCount),"=m"(iFull)\
                 :"m"(llOldRear),"m"(pMember)\
                 :"%rax","%rbx","%rcx");
 return iFull;*/
}
/********************************************************
  出队函数
  输入：1、队列对象指针；2、出队变量指针的地址
  返回值：大于1：出队指针；0；队空
 ********************************************************/

void*  Queue::deQueue()
{
	//最后版本
	void *pMember=0;
	 asm volatile("\
                        movl %3,%%ecx; /*最大队长*/\
                        movslq %%ecx,%%rcx;\
                        movq %5,%%r10;/*队列数组首字节地址*/ \
                        mov %4,%%rax;  /*队头序号*/ \
                loopDe: mov %2,%%r15; /*队尾序号*/ \
                        cmp %%r15,%%rax; /*判断队空*/ \
                        jnz deQueue; /*不空则出队*/ \
                        mov $0,%%rax;/*队列为空返回0*/ \
                        jmp DeQueueExit;\
                deQueue:mov %%rax,%%r14; /*队头序号*/ \
                        inc %%r14;/*寄存器队头序号加1*/ \
                        lock cmpxchg %%r14,%0;/*内存与寄存器的序号相等，则内存序号加1，不相等内存队头序号存入rax*/\
                        jnz loopDe;\
                        cqto;\
                        idiv %%rcx;/*队头下标对队长取余,余值在rdx*/ \
                        lea (%%r10,%%rdx,8),%%r11;/*r10存储队头下标元素的首字节地址*/\
			mov $0,%%rax;\
	      outMember:xchg (%%r11),%%rax;\
			test %%rax,%%rax;\
			jz outMember;\
     	    DeQueueExit:mov %%rax,%1;"\
                        /*sti; 开中断*/ \
                        :"+m"(llFront),"=m"(pMember)\
                        :"m"(llRear),"m"(iMaximum),"m"(llFront),"m"(pArray)\
                        :"%rax","%rdx","%rcx","%r12","%r10","%r11","%r14","%r15");
	 return pMember;


	/*void *pMember=(void *)1;
	asm("deQueueloop:");
	long long llOldFront=llFront;
	long long llRealFront=llOldFront%iMaximum;
	if(llOldFront==llRear) return pMember;
	asm("mov %4,%%rax;\
                mov %%rax,%%rbx;\
                add $1,%%rbx;\
                lock cmpxchg %%rbx,%2;\
                jnz deQueueloop;\
                mov $0,%%rax;\
     outMember: lock xchg %%rax,%0;\
                test %%rax,%%rax;\
                jz outMember;\
                mov %%rax,%1;"\
                :"+m"((*pArray)[llRealFront]),"=m"(pMember),"+m"(llFront),"=m"(iMemberCount)\
                :"m"(llOldFront)\
                :"%rax","%rbx");
	return pMember;*/


}
