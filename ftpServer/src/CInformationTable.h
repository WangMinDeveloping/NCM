/***********************************************************************
 * Module:  CStudenTable.h
 * Author:  crisisseer
 * Modified: 2021Äê7ÔÂ1ÈÕ 18:20:00
 * Purpose: Declaration of the class CStudenTable
 ***********************************************************************/

#if !defined(__CInformationTable_h)
#define __CInformationTable_h

#include "SDoublyListNode.h"

class CInformationTable
{
public:
   CInformationTable();
   ~CInformationTable();

   SDoublyListNode* addonelowNode(void* pValue);//头插一个
   SDoublyListNode* addoneupNode(void* pValue);//尾插一个
   
   void addmorelowNode(CInformationTable* pValue);//头插很多
   void addmoreupNode(CInformationTable* pValue);//尾插很多
   
   int deleteNode(SDoublyListNode* pNode);//删除节点
   
   void resetiteratornode(void);//重置遍历指针到前
   void iteratorprintf(void);//遍历打印
   SDoublyListNode* iteratordata(void);//遍历数据
   
   
   void operator=(CInformationTable& pValue);

protected:
private:
	SDoublyListNode* iteratorLastNode(void);//移动尾指针
	SDoublyListNode* pHeadDoublyListNode;//头节点
   	SDoublyListNode* pIteratorNode;//遍历使用节点
  	SDoublyListNode* pLastNode;//尾节点

};

#endif
