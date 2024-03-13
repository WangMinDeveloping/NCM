//==========================================
//  cppmod.cpp, finally, my C++ code
#include "new.h"
#include "cppmod.h"
#include "memorypool.h"
#include"unlockQueue.h"
#include"doublyList.h"
#include"socket.h"

extern "C" void* queueInit(int data) 
{
	Queue *pQueue = new Queue(data);
	if(pQueue)
		return (void*)pQueue;
	return 0;
}
extern "C" void queueDestory(void*pQueue)
{
	if(pQueue)
		delete (Queue*)pQueue;
}
extern "C" int enQueue_c(void*pQueue,void *pMember) 
{
	return ((Queue *)pQueue)->_enQueue(pMember);
}
extern "C" void*  deQueue_c(void*pQueue) 
{
	return ((Queue *)pQueue)->deQueue();
}

extern "C" void* memoryStackInit(int iCount,int iSize) 
{
	CMemoryPool *pMemoryPool = new CMemoryPool(iCount,iSize);
	if(pMemoryPool!=NULL)
		return (void*)pMemoryPool;
	return 0;
}
extern "C" void memoryStackDestory(void*pMemoryStack)
{
	if(pMemoryStack!=NULL)
		delete (CMemoryPool*)pMemoryStack;
}
extern "C" int push_c(void* pMemoryStack,void *pMember) 
{
	return ((CMemoryPool *)pMemoryStack)->push(pMember);
}
extern "C" void*  pop_c(void*pMemoryStack) 
{
	return ((CMemoryPool *)pMemoryStack)->pop();
}
extern "C" void* getMinAddress(void* pMemoryPool)
{
	return ((CMemoryPool *)pMemoryPool)->getMinAddress();
}
extern "C" void* getMaxAddress(void* pMemoryPool)
{
	return ((CMemoryPool *)pMemoryPool)->getMaxAddress();
}

extern "C" void* doublyListInit()
{
	CDoublyList<SSocket*> *pAllSocket = new CDoublyList<SSocket*>();
	if(pAllSocket!=NULL)
		return (void*)pAllSocket;
	return 0;
}

extern "C" void doublyListDestory(void*pList)
{
	if(pList!=NULL)
		delete (CDoublyList<SSocket*> *)pList;
}

extern "C" void* doublyListInsert(void* pList,void *pMember) 
{
	return (void *)((CDoublyList<SSocket*> *)pList)->insert((SSocket*)pMember);
}


extern "C" int doublyListDelete(void* pList,void *pMember) 
{
	return ((CDoublyList<SSocket*> *)pList)->deleteNode((SDoublyListNode<SSocket*> *)pMember);
}
extern "C" void* doublyListIterator(void* pList)
{
	return (void*)((CDoublyList<SSocket*> *)pList)->iterator();
}

