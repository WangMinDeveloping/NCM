/***********************************************************************
 * Module:  SRedBlackTreeNode.h
 * Author:  crisisseer
 * Modified: 2021Äê7ÔÂ1ÈÕ 19:08:07
 * Purpose: Declaration of the class SRedBlackTreeNode
 ***********************************************************************/

#if !defined(__SRedBlackTreeNode_h)
#define __SRedBlackTreeNode_h

#include "CData.h"
#include "CInformationTable.h"

class SRedBlackTreeNode
{
public:
   SRedBlackTreeNode* pFatherNode;
   SRedBlackTreeNode* pLeftChlidNode;
   SRedBlackTreeNode* pRightChlidNode;
   CData* pKey;
   int iColor;
   CInformationTable oRecordList;
   SDoublyListNode* sSelfDoublyNode;

protected:
private:

};

#endif
