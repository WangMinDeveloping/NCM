#ifndef _DOUBLYLIST_H
#define _DOUBLYLIST_H
#include"doublyListNode.h"
template<class TNodeType>
class CDoublyList
{
	private:
		SDoublyListNode<TNodeType> *pHeadNode;
		SDoublyListNode<TNodeType> *pIterator;
	public:
		CDoublyList();
		~CDoublyList();//删除所有结点
		SDoublyListNode<TNodeType> * insert(TNodeType value);
		int deleteNode(SDoublyListNode<TNodeType> *pNode);
		TNodeType iterator();
};
template<class TNodeType>
/*生成一个头节点*/
CDoublyList<TNodeType>::CDoublyList()
{
	pHeadNode=new SDoublyListNode<TNodeType>();
	pHeadNode->value=0;
	pIterator=pHeadNode;
}
	template<class TNodeType>
CDoublyList<TNodeType>::~CDoublyList()
{
	SDoublyListNode<TNodeType> *pSeekNode=pHeadNode;
	SDoublyListNode<TNodeType> *pNode;

	while(pSeekNode!=0)
	{       
		pNode=pSeekNode->pNext;//保存删除的下一个结点
		delete pSeekNode;
		pSeekNode=pNode;
	}
	pHeadNode=0;


}
	template<class TNodeType>
TNodeType CDoublyList<TNodeType>::iterator()
{
	TNodeType retData;
	if(pIterator==pHeadNode)
		pIterator=pIterator->pNext;
	if(pIterator!=NULL)
	{
		retData=pIterator->value;
		pIterator=pIterator->pNext;
		return retData;
	}
	pIterator=pHeadNode;
	return 0;
}
/********************* 头插法**************************
  参数：数据指针
  返回值：大于0 该数据存入的链表结点地址；0 new分配空间失败
 ********************************************************/
	template<class TNodeType>
SDoublyListNode<TNodeType>* CDoublyList<TNodeType>::insert(TNodeType value)
{
	SDoublyListNode<TNodeType> *pNewNode=new SDoublyListNode<TNodeType>();
	if(!pNewNode) return 0;
	pNewNode->pNext=pHeadNode->pNext;
	pNewNode->value=value;
	pNewNode->pPrev=pHeadNode;
	pHeadNode->pNext=pNewNode;
	return pNewNode;
}
/*********************************************
  删除函数
  参数：链表结点指针
  返回值：0 成功  1 失败（参数无效）
 ***********************************************/
	template<class TNodeType>
int CDoublyList<TNodeType>::deleteNode(SDoublyListNode<TNodeType> *pNode)
{
	if(pNode==NULL) return 1;
	pNode->pPrev->pNext=pNode->pNext;
	if(pNode->pNext!=NULL)//删除的结点不是最后一个
		pNode->pNext->pPrev=pNode->pPrev;
	delete pNode;
	return 0;
}
#endif
