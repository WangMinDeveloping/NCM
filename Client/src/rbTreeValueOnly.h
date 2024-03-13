#pragma once
#include"rbNodeValueOnly.h"
#include <stddef.h>
template<class TKey,class TValue>
class CRedBlackTree
{
	private:
		int rightRotate(CRedBlackTreeNode<TKey,TValue> *pCenterNode);
		int leftRotate(CRedBlackTreeNode<TKey,TValue> *pCenterNode);
		void balanceInsert(CRedBlackTreeNode<TKey,TValue> *p);
		void balanceDelete(CRedBlackTreeNode<TKey,TValue> *p);
		CRedBlackTreeNode<TKey,TValue>* seek(TKey key,CRedBlackTreeNode<TKey,TValue> **pInsertPosition=0);
		static void deleteAll(CRedBlackTreeNode<TKey,TValue> *pNode);//删除红黑树
	public:
		CRedBlackTreeNode<TKey,TValue>  *pRootNode;
		CRedBlackTree(){pRootNode=0;}
		~CRedBlackTree();//删除所有结点
		int insert(TKey key,TValue value);//插入函数,如果插入的值重复
		TValue deleteNode(TKey key);//删除某键的索引
		TValue find(TKey key);//查找键对应的值
};

#include <cassert>
template <class TKey, class TValue>
/*********************************************************************
  右旋函数
  参数:旋转的中心结点
  返回值:-1 参数无效;1 传入的参数为叶子结点;2传进来的结点无左子结点;0 成功
 ***********************************************************************/
int CRedBlackTree<TKey, TValue>::rightRotate(CRedBlackTreeNode<TKey, TValue> *pCenterNode)
{
	CRedBlackTreeNode<TKey, TValue> *pLeftNode; // 中心结点的左子节点
	if (pCenterNode == 0)
		return -1; // 参数无效
	if (pCenterNode->pLeft == 0 && pCenterNode->pRight == 0)
		return 1; // 传进来的结点为叶子结点
	if (pCenterNode->pLeft->pLeft == 0)
		return 2; // 左子结点为叶子结点
	pLeftNode = pCenterNode->pLeft;
	pCenterNode->pLeft = pLeftNode->pRight;	   // 中心结点的左孩子的右孩子赋值给中心结点的左孩子
	pLeftNode->pRight->pFather = pCenterNode;  // 将中心结点赋值为中心结点的左孩子的右孩子的父亲
	pLeftNode->pFather = pCenterNode->pFather; // 将中心结点的父亲赋值为中心结点的左孩子的父亲
	if (pCenterNode->pFather == 0)
		pRootNode = pLeftNode; // 情况1：如果中心结点为根结点，右旋后重新设置根结点
	else if (pCenterNode == pCenterNode->pFather->pRight)
		pCenterNode->pFather->pRight = pLeftNode; // 情况2：如果中心结点是右孩子，更改中心结点父亲的右孩子为中心结点的左孩子
	else
		pCenterNode->pFather->pLeft = pLeftNode; // 情况3：如果中心结点是左孩子，更改中心结点父亲的左孩子为中心结点的左孩子
	pLeftNode->pRight = pCenterNode;			 // 将中心结点修正为其左孩子的右孩子
	pCenterNode->pFather = pLeftNode;			 // 将中心结点的父结点修正为其左孩子
	return 0;
}

template <class TKey, class TValue>
/***********************************************************************
  左旋函数
  参数:旋转的中心结点
  返回值:-1 参数无效；1 传入的参数为叶子结点；2传进来的结点无左子结点
 ***********************************************************************/
int CRedBlackTree<TKey, TValue>::leftRotate(CRedBlackTreeNode<TKey, TValue> *pCenterNode)
{
	CRedBlackTreeNode<TKey, TValue> *pRightNode; // 中心结点的右子节点
	if (pCenterNode == 0)
		return -1; // 参数无效
	if (pCenterNode->pLeft == 0 && pCenterNode->pRight == 0)
		return 1; // 传进来的结点为叶子结点
	if (pCenterNode->pRight->pLeft == 0)
		return 2; // 右子结点为叶子结点
	pRightNode = pCenterNode->pRight;
	pCenterNode->pRight = pRightNode->pLeft;	// 中心结点的右孩子的左孩子赋值给中心结点的右孩子
	pRightNode->pLeft->pFather = pCenterNode;	// 将中心结点赋值为中心结点的右孩子的左孩子的父亲
	pRightNode->pFather = pCenterNode->pFather; // 将中心结点的父亲赋值为中心结点的右孩子的父亲
	if (pCenterNode->pFather == 0)
		pRootNode = pRightNode; // 情况1：如果中心结点为根结点，右旋后重新设置根结点
	else if (pCenterNode == pCenterNode->pFather->pRight)
		pCenterNode->pFather->pRight = pRightNode; // 情况2：如果中心结点是右孩子，更改中心结点父亲的右孩子为中心结点的右孩子
	else
		pCenterNode->pFather->pLeft = pRightNode; // 情况3：如果中心结点是左孩子，更改中心结点父亲的左孩子为中心结点的右孩子
	pRightNode->pLeft = pCenterNode;			  // 将中心结点修正为其右孩子的左孩子
	pCenterNode->pFather = pRightNode;			  // 将中心结点的父结点修正为其右孩子
	return 0;
}
template <class TKey, class TValue>
/*******************************************************************
  查找函数
  参数：需要查找的键指针,该键值应该插入的位置的地址
  返回值：大于 0 目标键键的结点; 0 没有找到该键值;-1 根结点为0
 *******************************************************************/
CRedBlackTreeNode<TKey, TValue> *CRedBlackTree<TKey, TValue>::seek(TKey key, CRedBlackTreeNode<TKey, TValue> **pInsertPosition)
{
	CRedBlackTreeNode<TKey, TValue> *pSeekNode = pRootNode; // 将根结点的指针赋给pSeekNode
	if (pSeekNode == 0)
		return 0; // 根结点为0
	while (pSeekNode->pLeft != NULL)
	{
		if (pSeekNode->key == key) // 找到了该键
			return pSeekNode;
		if (key > pSeekNode->key) // 目标键值大于某结点键值
			pSeekNode = pSeekNode->pRight;
		else // 目标键值小于当前结点键值
			pSeekNode = pSeekNode->pLeft;
	}
	if (pInsertPosition != 0) // 如果需要知道目标键值的插入位置
		*pInsertPosition = pSeekNode;
	return 0; // 没有找到对应的键
}

template <class TKey, class TValue>
/*******************************************************************
  查找函数
  参数：需要查找的键值
  返回值：大于 0 目标键值的链表; 0 没有找到该键的值;
 *******************************************************************/
TValue CRedBlackTree<TKey, TValue>::find(TKey key)
{
	CRedBlackTreeNode<TKey, TValue> *pSeekNode = pRootNode; // 将根结点的指针赋给pSeekNode
	if (pSeekNode == NULL)
		return 0; // 根结点为0
	while (pSeekNode->pLeft != NULL)
	{
		if (pSeekNode->key == key) // 找到了该键
		{
			return pSeekNode->value;
		}
		if (key > pSeekNode->key) // 目标键值大于某结点键值
			pSeekNode = pSeekNode->pRight;
		else // 目标键值小于当前结点键值
			pSeekNode = pSeekNode->pLeft;
	}
	return 0; // 没有找到对应的键
}

template <class TKey, class TValue>
/*************************************************
  插入平衡树函数
  参数：新插入的结点
 *************************************************/
void CRedBlackTree<TKey, TValue>::balanceInsert(CRedBlackTreeNode<TKey, TValue> *pNewNode)
{
	assert(pNewNode != NULL);
	CRedBlackTreeNode<TKey, TValue> *pNode = pNewNode; // 用于判断是否平衡
	while (pNode != pRootNode && pNode->pFather->bColor == RED)
	{
		if (pNode->pFather->pFather->pLeft == pNode->pFather) // 如果父结点是祖父的左孩子
		{
			if (pNode->pFather->pLeft == pNode) // 当前结点是父结点的左孩子
			{
				pNode->bColor = BLACK;				  // 新插入的结点改为黑色
				rightRotate(pNode->pFather->pFather); // 右旋成功
				pNode = pNode->pFather;				  // 重新设置pNode
			}
			else // 当前结点为右孩子
			{
				pNewNode = pNode->pFather;
				leftRotate(pNode->pFather);
				pNode = pNewNode;
			}
		}
		else // 父亲结点是右孩子
		{
			if (pNode->pFather->pRight == pNode) // 当前结点是父结点的右孩子
			{
				pNode->bColor = BLACK; // 设置当前结点为黑色
				leftRotate(pNode->pFather->pFather);
				pNode = pNode->pFather;
			}
			else // 当前结点为左孩子
			{
				pNewNode = pNode->pFather;
				rightRotate(pNode->pFather);
				pNode = pNewNode;
			}
		}
	}
	pRootNode->bColor = BLACK;
}
template <class TKey, class TValue>
/********************************************
  递归方式删除红黑树
  参数：
  返回值：无
 ********************************************/
void CRedBlackTree<TKey, TValue>::deleteAll(CRedBlackTreeNode<TKey, TValue> *pNode)
{
	if (pNode != 0)
	{
		deleteAll(pNode->pLeft);
		deleteAll(pNode->pRight);
		delete pNode; // 删除结点
	}
}

template <class TKey, class TValue>
/******************************
  析构函数
 ******************************/
CRedBlackTree<TKey, TValue>::~CRedBlackTree()
{
	if (pRootNode)
	{
		deleteAll(pRootNode);
		pRootNode = 0;
	}
}

template <class TKey, class TValue>
/*************************************************
  删除平衡函数
  参数：被删除点的子结点
  返回值：无
 *************************************************/
void CRedBlackTree<TKey, TValue>::balanceDelete(CRedBlackTreeNode<TKey, TValue> *p)
{
	CRedBlackTreeNode<TKey, TValue> *pBotherNode;
	CRedBlackTreeNode<TKey, TValue> *pAdjustNode = p;
	while (pAdjustNode != pRootNode && pAdjustNode->bColor == BLACK)
	{
		if (pAdjustNode->pFather->pRight == pAdjustNode) // 当前调整结点是右结点
		{
			pBotherNode = pAdjustNode->pFather->pLeft;
			if (pBotherNode->bColor == RED) // 当前结点的兄弟结点为红色
			{
				pBotherNode->bColor = BLACK;
				pBotherNode->pFather->bColor = RED;
				rightRotate(pBotherNode->pFather);
			}
			else // 当前结点的兄弟结点是黑色
			{
				if (pBotherNode->pLeft->bColor == BLACK && pBotherNode->pRight->bColor == BLACK)
				{
					pBotherNode->bColor = RED;
					pAdjustNode = pAdjustNode->pFather;
				}
				else if (pBotherNode->pLeft->bColor == BLACK && pBotherNode->pRight->bColor == RED)
				{
					pBotherNode->pRight->bColor = BLACK;
					pBotherNode->bColor = RED;
					leftRotate(pBotherNode);
					pBotherNode = pAdjustNode->pFather->pLeft;
				}
				else
				{
					pBotherNode->bColor = pBotherNode->pFather->bColor;
					pBotherNode->pFather->bColor = BLACK;
					pBotherNode->pLeft->bColor = BLACK;
					rightRotate(pBotherNode->pFather);
					pAdjustNode = pRootNode; // 退出循坏
				}
			}
		}
		else // 当前调整结点是左结点
		{
			pBotherNode = pAdjustNode->pFather->pRight;
			if (pBotherNode->bColor == RED) // 当前结点的兄弟结点为红色
			{
				pBotherNode->bColor = BLACK;
				pBotherNode->pFather->bColor = RED;
				leftRotate(pBotherNode->pFather);
				pBotherNode = pAdjustNode->pFather->pRight; // 重新设置兄弟结点
			}
			else // 当前结点的兄弟结点是黑色
			{
				if (pBotherNode->pLeft->bColor == BLACK && pBotherNode->pRight->bColor == BLACK)
				{
					pBotherNode->bColor = RED;
					pAdjustNode = pAdjustNode->pFather;
				}
				else if (pBotherNode->pLeft->bColor == RED && pBotherNode->pRight->bColor == BLACK)
				{
					pBotherNode->pLeft->bColor = BLACK;
					pBotherNode->bColor = BLACK;
					rightRotate(pBotherNode);
					pBotherNode = pAdjustNode->pFather->pRight;
				}
				else
				{
					pBotherNode->bColor = pBotherNode->pFather->bColor;
					pBotherNode->pFather->bColor = BLACK;
					pBotherNode->pRight->bColor = BLACK;
					leftRotate(pBotherNode->pFather);
					pAdjustNode = pRootNode; // 退出循坏
				}
			}
		}
	}
	pAdjustNode->bColor = BLACK;
}
template <class TKey, class TValue>
/************************************************
  插入函数：
  参数：1、键的地址 ；2、值的地址
  返回值：0 成功；1 new分配空间失败
 **********************************************/
int CRedBlackTree<TKey, TValue>::insert(TKey key, TValue value)
{
	CRedBlackTreeNode<TKey, TValue> *pNewNode = 0;
	// TValue oldValue;
	CRedBlackTreeNode<TKey, TValue> *pExistNode; // 如果插入的键经查找已存在，存放该键值的红黑树结点
	pExistNode = this->seek(key, &pNewNode);
	if (pExistNode == 0) // 对应键值不存在
	{
		if (pRootNode == 0) ////插入的键的是根结点
		{
			pNewNode = new CRedBlackTreeNode<TKey, TValue>();
			if (!pNewNode)
				return 1;

			pNewNode->key = key;
			pNewNode->value = value;
			pNewNode->pLeft = new CRedBlackTreeNode<TKey, TValue>(pNewNode); // 叶子结点
			if (!pNewNode->pLeft)
				return 1;

			pNewNode->pRight = new CRedBlackTreeNode<TKey, TValue>(pNewNode); // 叶子结点
			if (!pNewNode->pRight)
				return 1;

			pRootNode = pNewNode; // 设置根结点
			return 0;
		}
		else
		{
			pNewNode->key = key;
			pNewNode->value = value;
			pNewNode->bColor = RED;

			pNewNode->pLeft = new CRedBlackTreeNode<TKey, TValue>(pNewNode); // 叶子结点
			if (!pNewNode->pLeft)
				return 1;													  // 空间分配失败
			pNewNode->pRight = new CRedBlackTreeNode<TKey, TValue>(pNewNode); // 叶子结点
			if (!pNewNode->pRight)
				return 1;

			if (pNewNode->pFather->bColor == RED)
				this->balanceInsert(pNewNode);
			return 0;
		}
	}
	else // 插入的键已存在
	{
		//	oldValue=pExistNode->value;
		pExistNode->value = value;
		//	return oldValue;
		return 0;
	}
}
template <class TKey, class TValue>
/************************************************
  删除函数：
  参数：1、键
  返回值：非0 值；0 删除键不存在
 **********************************************/
TValue CRedBlackTree<TKey, TValue>::deleteNode(TKey key)
{
	CRedBlackTreeNode<TKey, TValue> *pInheritedNode; // 继承被删除结点位置的结点
	CRedBlackTreeNode<TKey, TValue> *pDeleteNode;	 // 被删除的结点
	CRedBlackTreeNode<TKey, TValue> *pDeleteKeyNode; // 需要被删除键值的结点
	pDeleteKeyNode = this->seek(key);
	TValue retValue;
	if (pDeleteKeyNode == 0)
		return 0; // 没有找到需要删除的结点
	else		  // 找到了需要删除的键值结点
	{
		/*没有指定删除哪一个值*/
		if (pDeleteKeyNode->pLeft->pLeft == 0 || pDeleteKeyNode->pRight->pLeft == 0) // 被删除结点有一个空子节点或两个空结点
			pDeleteNode = pDeleteKeyNode;											 // 被删除结点就等于需要被删除键值结点
		else
		{
			pDeleteNode = pDeleteKeyNode->pLeft;
			// 找到左边的最大值
			while (pDeleteNode->pRight->pLeft != 0)
				pDeleteNode = pDeleteNode->pRight;
		}

		if (pDeleteNode->pLeft->pLeft != 0) // 被删除结点左子结点不为空
		{
			pInheritedNode = pDeleteNode->pLeft; // 就把左子结点给继承结点
			delete pDeleteNode->pRight;
		}
		else // 被删除结点左子节点为空
		{
			pInheritedNode = pDeleteNode->pRight; // 否则把右子结点给继承结点，右子节点可能为叶子结点
			delete pDeleteNode->pLeft;
		}

		pInheritedNode->pFather = pDeleteNode->pFather; // 将继承结点的父结点修正为被删除的父结点

		if (pDeleteNode->pFather == 0) // 删除点是根结点
		{
			if (pInheritedNode->key == 0) // 并且子继结点为空
			{
				pRootNode = 0;		   // 更新根结点
				delete pInheritedNode; // 删除该叶子结点
				pInheritedNode = 0;
			}
			else
				pRootNode = pInheritedNode;
		}
		else // 被删除点不是根结点
		{
			if (pDeleteNode->pFather->pRight == pDeleteNode)   // 被删除结点是右孩子
				pDeleteNode->pFather->pRight = pInheritedNode; // 继承结点为右孩子
			else
				pDeleteNode->pFather->pLeft = pInheritedNode; // 继承结点为左孩子
		}

		retValue = pDeleteKeyNode->value;
		pDeleteKeyNode->key = pDeleteNode->key;
		pDeleteKeyNode->value = pDeleteNode->value;

		if (pDeleteNode->bColor == BLACK && pRootNode != NULL) // 被删除的结点为黑色
			this->balanceDelete(pInheritedNode);			   // 调用删除平衡调节函数
		delete pDeleteNode;									   // 删除红黑树结点
	}
	return retValue; // 删除成功
}