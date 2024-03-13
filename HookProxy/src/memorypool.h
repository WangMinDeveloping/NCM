#pragma once
/*多线程内存池栈*/
class CMemoryPool
{
	public:
		CMemoryPool(int iCount, int iSize); //iCount内存块的数量,iSize代表每个内存块的大小
		~CMemoryPool();
		void* pop(void);//出栈一个内存块首地址，自行强转
		int push(void * pMember);//将不用内存块首地址地址入栈，
		void* getMinAddress();
		void* getMaxAddress();

	private:
		int iMaximum;
		int iStackTop;
		char *pMaxAddress;
		void* (*pStackArray)[];
		char* pMemory;


};
