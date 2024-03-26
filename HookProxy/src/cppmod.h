//==========================================
// cppmod.h, exported C interface from C++ code
#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

    extern void *queueInit(int data);
    extern void queueDestory(void *pQueue);
    extern void *deQueue_c(void *pQueue);
    extern int enQueue_c(void *pQueue, void *pMember);

    extern void *memoryStackInit(int iCount, int iSize);
    extern void memoryStackDestory(void *pMemoryPool);
    extern void *pop_c(void *pMemoryPool);
    extern int push_c(void *pMemoryPool, void *pMember);
    extern void *getMinAddress(void *pMemoryPool);
    extern void *getMaxAddress(void *pMemoryPool);

    extern int doublyListDelete(void *pList, void *pMember);
    extern void *doublyListInsert(void *pList, void *pMember);
    extern void doublyListDestory(void *pList);
    extern void *doublyListInit(void);
    extern void *doublyListIterator(void *pList);

#ifdef __cplusplus
}
#endif
