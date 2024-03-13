#ifndef _DOUBLYLISTNODE_H
#define _DOUBLYLISTNODE_H
template<typename TDataType>
struct SDoublyListNode
{
	SDoublyListNode<TDataType> *pNext;
	SDoublyListNode<TDataType> *pPrev;
	TDataType value;
};
#endif
