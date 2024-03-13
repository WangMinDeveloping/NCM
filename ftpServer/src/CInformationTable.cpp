/***********************************************************************
 * Module:  CStudenTable.cpp
 * Author:  crisisseer
 * Modified: 2021Äê7ÔÂ1ÈÕ 18:20:00
 * Purpose: Implementation of the class CStudenTable
 ***********************************************************************/
#include<stdio.h>
#include<new>
#include "CInformationTable.h"
CInformationTable::CInformationTable()
{
	pHeadDoublyListNode=new SDoublyListNode;
	pHeadDoublyListNode->pNextListNode=NULL;
	pHeadDoublyListNode->pFrontListNode=NULL;
	
	pIteratorNode=NULL;
	pLastNode=pHeadDoublyListNode;
}

CInformationTable::~CInformationTable()
{
	SDoublyListNode* p=NULL;
	resetiteratornode();
	p=iteratordata();
	while(p!=NULL)
	{
		delete p;
		p=iteratordata();
	}
	delete pHeadDoublyListNode;
	pHeadDoublyListNode=NULL;
}

void CInformationTable::operator=(CInformationTable& pValue)
{
	SDoublyListNode* p=NULL;
	p=pHeadDoublyListNode;
	pHeadDoublyListNode=pValue.pHeadDoublyListNode;
	pValue.pHeadDoublyListNode=p;
	
	p=pIteratorNode;
	pIteratorNode=pValue.pIteratorNode;
	pValue.pIteratorNode=p;
	
	p=pLastNode;
	pLastNode=pValue.pLastNode;
	pValue.pLastNode=p;
}

SDoublyListNode* CInformationTable::addonelowNode(void* pValue)//尾插一个
{
   SDoublyListNode* addnode=new SDoublyListNode;
   addnode->pStudent=pValue;
	
   addnode->pNextListNode=pHeadDoublyListNode->pNextListNode;
   pHeadDoublyListNode->pNextListNode->pFrontListNode=addnode;
   
   pHeadDoublyListNode->pNextListNode=addnode;
   addnode->pFrontListNode=pHeadDoublyListNode;
   return addnode;
}

SDoublyListNode* CInformationTable::addoneupNode(void* pValue)//头插一个
{
   SDoublyListNode* addnode=new SDoublyListNode;
   addnode->pStudent=pValue;
   
   addnode->pNextListNode=NULL;
   pLastNode->pNextListNode=addnode;
   addnode->pFrontListNode=pLastNode;
   iteratorLastNode();
   return addnode;
}

void CInformationTable::addmorelowNode(CInformationTable* pValue)//尾插多个
{
	
	pValue->pLastNode->pNextListNode=pHeadDoublyListNode->pNextListNode;
	pHeadDoublyListNode->pNextListNode->pFrontListNode=pValue->pLastNode;
	
	pHeadDoublyListNode->pNextListNode=pValue->pHeadDoublyListNode->pNextListNode;
	pValue->pHeadDoublyListNode->pNextListNode->pFrontListNode=pHeadDoublyListNode;
}

void CInformationTable::addmoreupNode(CInformationTable* pValue)//头插多个
{
   pLastNode->pNextListNode=pValue->pHeadDoublyListNode->pNextListNode;
   pValue->pHeadDoublyListNode->pNextListNode->pFrontListNode=pLastNode;
   iteratorLastNode();
}

void CInformationTable::iteratorprintf(void)//遍历打印
{
  	while(pIteratorNode!=NULL)
  	{
  		//printf("%p %p\n",pIteratorNode,pHeadDoublyListNode);
  		pIteratorNode=pIteratorNode->pNextListNode;
  		if(pIteratorNode==NULL)
  			break;
  		printf("%p\n",pIteratorNode->pStudent);
  	}
  	printf("\n");
}

SDoublyListNode* CInformationTable::iteratordata(void)//遍历数据
{
	if(pIteratorNode!=NULL)
		pIteratorNode=pIteratorNode->pNextListNode;
  	return pIteratorNode;
}

void CInformationTable::resetiteratornode(void)//重置遍历
{
	pIteratorNode=pHeadDoublyListNode;
}

SDoublyListNode* CInformationTable::iteratorLastNode(void)//移动尾指针
{
	while(pLastNode->pNextListNode!=NULL)
	{
		pLastNode=pLastNode->pNextListNode;

	}
	return pLastNode;
}

int CInformationTable::deleteNode(SDoublyListNode* pNode)//删除单个节点
{
   pNode->pFrontListNode->pNextListNode=pNode->pNextListNode;
   if(pNode->pNextListNode!=NULL)
   	pNode->pNextListNode->pFrontListNode=pNode->pFrontListNode;
   delete pNode;
   return 0;
}
