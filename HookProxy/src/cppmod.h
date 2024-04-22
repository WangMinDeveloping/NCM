#ifndef CPP_MOD_H
#define CPP_MOD_H
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

    extern int DoublyListDelete(void *pList, void *pMember);
    extern void *DoublyListInsert(void *pList, void *pMember);
    extern void DoublyListDestory(void *pList);
    extern void *DoublyListInit(void);
    extern void *DoublyListIterator(void *pList);
    extern void __cxa_pure_virtual(void);

#ifdef __cplusplus
}
#endif
#endif
