/***********************************************************************
 * Module:  CRedBlackTree.h
 * Author:  crisisseer
 * Modified: 2021Äê7ÔÂ1ÈÕ 17:18:17
 * Purpose: Declaration of the class CRedBlackTree
 ***********************************************************************/

#if !defined(__CRedBlackTree_h)
#define __CRedBlackTree_h

#include "SRedBlackTreeNode.h"
#include "CInformationTable.h"
#include "CData.h"
class CRedBlackTree
{
public:
   CRedBlackTree();
   virtual ~CRedBlackTree();
   SRedBlackTreeNode* addNode(CData* pKey,void* pValue);
   int iterator(CData* pKey);
   void deleteNode(SRedBlackTreeNode* pKey);

   void printfFunction();
   CInformationTable pDoublyListRoot;//双向链表，为sack进行节点的遍历
   int NodeCount;//判断节点的个树是否为一
   int iChick;//当只有一个节点时判断是否有问题

protected:
private:
   void rightRotating(SRedBlackTreeNode* rotatingNode);
   void leftRotating(SRedBlackTreeNode* rotatingNode);
   void balanceDelete(SRedBlackTreeNode* deleteNode);
   void balanceAddNode(SRedBlackTreeNode* addnode);
   inline void NewNodeCreate(SRedBlackTreeNode* saveAddNode,SRedBlackTreeNode* useForSave,CData* pKey,void* pValue);
   
   SRedBlackTreeNode* pRoot;


};

#endif
