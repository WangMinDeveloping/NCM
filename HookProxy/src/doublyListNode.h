#pragma once
template<typename TDataType>
struct SDoublyListNode
{
	SDoublyListNode<TDataType> *pNext;
	SDoublyListNode<TDataType> *pPrev;
	TDataType value;
};
