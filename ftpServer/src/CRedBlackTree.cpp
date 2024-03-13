/***********************************************************************
 * Module:  CRedBlackTree.cpp
 * Author:  crisisseer
 * Modified: 2021Äê7ÔÂ1ÈÕ 17:18:17
 * Purpose: Implementation of the class CRedBlackTree
 ***********************************************************************/

#include "CRedBlackTree.h"
#include<stdio.h>

CRedBlackTree::CRedBlackTree()
{
	pRoot=NULL;
	NodeCount=0;
}

CRedBlackTree::~CRedBlackTree()
{
	SRedBlackTreeNode* useForSave=pRoot;
	SRedBlackTreeNode* useForSaveFather=NULL;
	int iCount=0;
	while(1)
	{
		iCount=0;
		if(useForSave==NULL)//是否有数据
			return;
		while(useForSave->pLeftChlidNode!=NULL)//是否有左孩子
		{
			iCount=1;
			useForSaveFather=useForSave;
			useForSave=useForSave->pLeftChlidNode;
		}
		if(useForSaveFather!=NULL&&useForSaveFather->pRightChlidNode!=NULL&&iCount!=0)
		{
			if(useForSave->pRightChlidNode!=NULL)//打印当前节点的右孩子
			{
				delete useForSave->pKey;
				delete useForSave;
			}
			delete useForSave->pKey;
			delete useForSave;
			useForSave=useForSaveFather->pRightChlidNode;
		}
		else
		{
			if(useForSave->pRightChlidNode!=NULL)
			{
				delete useForSave->pRightChlidNode->pKey;
				delete useForSave->pRightChlidNode;
			}
			delete useForSave->pKey;
			delete useForSave;
			useForSave=useForSaveFather;
			while(useForSave!=NULL)
			{
				delete useForSave->pKey;
				delete useForSave;
				useForSaveFather=useForSave->pFatherNode;
				if(useForSaveFather!=NULL)
				{
					if(useForSaveFather->pRightChlidNode==useForSave)//如果一直为右孩子那么一直上移动
					{
						useForSave=useForSaveFather;
					}
					else
					{
						/*if(useForSaveFather->pRightChlidNode!=NULL)//如中间为左，就进入另一个右子树
						{*/
							useForSave=useForSaveFather->pRightChlidNode;
							break;
						/*}
						else//没有右子树继续上移动
						{
							useForSave=useForSaveFather;
						}*/
					}
				}
				else//夫节点为空表示到更节点
				{
					return;
				}
			}
		}
		
	} 
}

inline void CRedBlackTree::NewNodeCreate(SRedBlackTreeNode* saveAddNode,SRedBlackTreeNode* useForSave,CData* pKey,void* pValue)
{
	saveAddNode->pKey=pKey;
	saveAddNode->pFatherNode=useForSave;
	saveAddNode->pLeftChlidNode=NULL;
	saveAddNode->pRightChlidNode=NULL;
	saveAddNode->iColor=1;
	saveAddNode->oRecordList.addoneupNode(pValue);
	saveAddNode->sSelfDoublyNode=pDoublyListRoot.addoneupNode(saveAddNode);//节点进入双向链表并保存自己的地址
}

SRedBlackTreeNode* CRedBlackTree::addNode(CData* pKey,void* pValue)
{
	SRedBlackTreeNode* useForSave=pRoot;
	SRedBlackTreeNode* saveAddNode=NULL;

	if(useForSave!=NULL)
	{
		while(1)
		{
			switch (useForSave->pKey->compare(pKey))
			{
				case 2:
					saveAddNode=useForSave;
					saveAddNode->oRecordList.addoneupNode(pValue);
					if(useForSave->pRightChlidNode!=NULL)
						useForSave=useForSave->pRightChlidNode;
					else
						return saveAddNode;
					while(useForSave->pLeftChlidNode!=NULL)
					{
						useForSave=useForSave->pLeftChlidNode;
					}
					if(saveAddNode->pKey->compare(useForSave->pKey)==2)//节点合并
					{
						saveAddNode->oRecordList.addmoreupNode(&useForSave->oRecordList);
						if(useForSave->pRightChlidNode!=NULL)
						{
							CData *p=NULL;
							p=useForSave->pKey;
							useForSave->pKey=useForSave->pRightChlidNode->pKey;
							useForSave->pRightChlidNode->pKey=p;
							useForSave->oRecordList=useForSave->pRightChlidNode->oRecordList;
							useForSave=useForSave->pRightChlidNode;
						}
						pDoublyListRoot.deleteNode(useForSave->sSelfDoublyNode);
						deleteNode(useForSave);
						NodeCount--;
					}
					return saveAddNode;
					break;
				case 1:
					saveAddNode=useForSave;
					saveAddNode->oRecordList.addonelowNode(pValue);
					if(useForSave->pLeftChlidNode!=NULL)
						useForSave=useForSave->pLeftChlidNode;
					else
						return saveAddNode;
					while(useForSave->pRightChlidNode!=NULL)
					{
						useForSave=useForSave->pRightChlidNode;
					}
					if(saveAddNode->pKey->compare(useForSave->pKey)==1)//节点合并
					{
						saveAddNode->oRecordList.addmorelowNode(&useForSave->oRecordList);
						if(useForSave->pLeftChlidNode!=NULL)
						{
							CData *p=NULL;
							p=useForSave->pKey;
							useForSave->pKey=useForSave->pLeftChlidNode->pKey;
							useForSave->pLeftChlidNode->pKey=p;
							useForSave->oRecordList=useForSave->pLeftChlidNode->oRecordList;
							useForSave=useForSave->pLeftChlidNode;
						}
						pDoublyListRoot.deleteNode(useForSave->sSelfDoublyNode);
						deleteNode(useForSave);
						NodeCount--;
					}
					return saveAddNode;
					break;
				case -1:
					if(useForSave->pLeftChlidNode!=NULL)
					{
						useForSave=useForSave->pLeftChlidNode;
						break;	
					}
					else
					{
						saveAddNode=new SRedBlackTreeNode;
						NewNodeCreate(saveAddNode,useForSave,pKey,pValue);
						useForSave->pLeftChlidNode=saveAddNode;
						NodeCount++;
						break;
					}
				case -2:
					if(useForSave->pRightChlidNode!=NULL)
					{
						useForSave=useForSave->pRightChlidNode;
						break;	
					}
					else
					{
						saveAddNode=new SRedBlackTreeNode;
						NewNodeCreate(saveAddNode,useForSave,pKey,pValue);
						useForSave->pRightChlidNode=saveAddNode;

						NodeCount++;
						break;
					}
				case 0:
					//printf("0000000000 %d\n",pKey->Key.llSeqnumber);
					delete pKey;
					return NULL;
			}
			if(saveAddNode!=NULL)
			{
				break;
			}
		}
	}
	else
	{
		saveAddNode=new SRedBlackTreeNode;
		NewNodeCreate(saveAddNode,useForSave,pKey,pValue);
		pRoot=saveAddNode;
		NodeCount++;
	}
	if(saveAddNode->pFatherNode==NULL)
	{
		return saveAddNode;
	}
	else if(saveAddNode->pFatherNode->iColor)
	{
		balanceAddNode(saveAddNode);
	}
	return saveAddNode;
}

int CRedBlackTree::iterator(CData* pKey)
{
	SRedBlackTreeNode* useForSave=pRoot;
	if(useForSave!=NULL)
	{
		printf("start2\n");
		while(1)
		{
			switch (useForSave->pKey->iterator(pKey))
			{
				case -1:
					if(useForSave->pLeftChlidNode!=NULL)
					{
						useForSave=useForSave->pLeftChlidNode;
						break;	
					}
					else
					{
						return 1;
						break;
					}
				case -2:
					if(useForSave->pRightChlidNode!=NULL)
					{
						useForSave=useForSave->pRightChlidNode;
						break;	
					}
					else
					{
						return 2;
						break;
					}
				case 0:
					return 0;
					break;
			}
		}
	}
	else
	{
		return -2;
	}
}

void CRedBlackTree::deleteNode(SRedBlackTreeNode* pKey)
{
	//printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
	//printf("%p ",pKey);
	//printf("%p color=%d ",pKey,pKey->iColor);
	//pKey->pKey->printfFunction();
	//printf("%p",pKey->pFatherNode);
	//printf("%p color=%d ",pKey->pFatherNode,pKey->pFatherNode->iColor);
	//pKey->pFatherNode->pKey->printfFunction();
	if(!pKey->iColor)
	{
		balanceDelete(pKey);
	}
	else
	{
		if(pKey->pFatherNode->pLeftChlidNode==pKey)
		{
			pKey->pFatherNode->pLeftChlidNode=NULL;
		}
		else
		{
			pKey->pFatherNode->pRightChlidNode=NULL;
		}
		delete pKey->pKey;
		delete pKey;
	}
	//printfFunction();
}

//左旋函数
//输入 旋转的中心节点
//返回值 无
void CRedBlackTree::leftRotating(SRedBlackTreeNode* rotatingNode)
{
	SRedBlackTreeNode* saveFather=rotatingNode->pFatherNode;
	SRedBlackTreeNode* saveChlid=rotatingNode->pLeftChlidNode;
	//和父交换
	rotatingNode->pFatherNode=saveFather->pFatherNode;

	//更改祖父的孩子
	if(saveFather->pFatherNode==NULL)
	{
		pRoot=rotatingNode;
	}
	else if(saveFather->pFatherNode->pLeftChlidNode==saveFather)
	{
		saveFather->pFatherNode->pLeftChlidNode=rotatingNode;
	}
	else
	{
		saveFather->pFatherNode->pRightChlidNode=rotatingNode;
	}

	saveFather->pFatherNode=rotatingNode;
	rotatingNode->pLeftChlidNode=saveFather;
	
	//给予孩子
	saveFather->pRightChlidNode=saveChlid;
	if(saveChlid!=NULL)
	{
		saveChlid->pFatherNode=saveFather;
	}
	
}

//右旋函数
//输入 旋转的中心节点
//返回值 无
void CRedBlackTree::rightRotating(SRedBlackTreeNode* rotatingNode)
{
	SRedBlackTreeNode* saveFather=rotatingNode->pFatherNode;
	SRedBlackTreeNode* saveChlid=rotatingNode->pRightChlidNode;
	//和父交换
	rotatingNode->pFatherNode=saveFather->pFatherNode;
	
	//更改祖父的孩子
	if(saveFather->pFatherNode==NULL)
	{
		pRoot=rotatingNode;
	}
	else if(saveFather->pFatherNode->pLeftChlidNode==saveFather)
	{
		saveFather->pFatherNode->pLeftChlidNode=rotatingNode;
	}
	else
	{
		saveFather->pFatherNode->pRightChlidNode=rotatingNode;
	}

	saveFather->pFatherNode=rotatingNode;
	rotatingNode->pRightChlidNode=saveFather;
	
	//给予孩子
	saveFather->pLeftChlidNode=saveChlid;
	if(saveChlid!=NULL)
	{
		saveChlid->pFatherNode=saveFather;
	}
}

void CRedBlackTree::balanceDelete(SRedBlackTreeNode* deleteNode)
{
	SRedBlackTreeNode* saveFatherNode=deleteNode->pFatherNode;
	//printfFunction();
	int firstDelete=1;
	SRedBlackTreeNode* saveBotherNode=NULL;
loop:	if(saveFatherNode->pRightChlidNode==deleteNode)//兄弟节点为左孩子
	{
		if(firstDelete)
		{
			delete deleteNode->pKey;
			delete deleteNode;
			saveFatherNode->pRightChlidNode=NULL;
			firstDelete=0;
		}
		saveBotherNode=saveFatherNode->pLeftChlidNode;
		if(saveBotherNode==NULL)
		{
			printf("error1\n");
			printfFunction();
			printf("%p\n",saveFatherNode);
			while(1);
		}
		if(saveBotherNode->iColor)//判断兄弟是不是红色
		{
			rightRotating(saveBotherNode);
			saveBotherNode->iColor=0;
			saveFatherNode->iColor=1;
			goto loop;
		}
		else
		{
			if(saveBotherNode->pRightChlidNode!=NULL&&saveBotherNode->pRightChlidNode->iColor)//兄弟有近孩子为红
			{
				leftRotating(saveBotherNode->pRightChlidNode);//改变为远孩子
				saveBotherNode->iColor=1;
				saveBotherNode=saveBotherNode->pFatherNode;
				saveBotherNode->iColor=0;
			}
			if(saveBotherNode->pLeftChlidNode!=NULL&&saveBotherNode->pLeftChlidNode->iColor)//兄弟有远孩子为红

			{
				saveBotherNode->iColor=saveFatherNode->iColor;
				saveFatherNode->iColor=0;
				saveBotherNode->pLeftChlidNode->iColor=0;
				rightRotating(saveBotherNode);//让孩子变为父亲颜色，父亲变为黑，兄弟旋转后变为父亲，平很
			}
			else//没有孩子
			{
				if(saveFatherNode->iColor)//父亲为红
				{
					saveFatherNode->iColor=0;
					saveBotherNode->iColor=1;
				}
				else//无法解决，向上转移问题
				{
					saveBotherNode->iColor=1;
					//saveBotherNode->pKey->printfFunction();
					deleteNode=saveFatherNode;
					saveFatherNode=saveFatherNode->pFatherNode;
					if(saveFatherNode!=NULL)
						goto loop;
				}
			}
		}
	}
	else
	{
		if(firstDelete)
		{
			delete deleteNode->pKey;
			delete deleteNode;
			saveFatherNode->pLeftChlidNode=NULL;
			firstDelete=0;
		}
		saveBotherNode=saveFatherNode->pRightChlidNode;
		if(saveBotherNode==NULL)
		{
			printf("error2\n");
			printfFunction();
			printf("%p\n",saveFatherNode);
			while(1);
		}
		if(saveBotherNode->iColor)//判断兄弟是不是红色
		{
			leftRotating(saveBotherNode);
			saveBotherNode->iColor=0;
			saveFatherNode->iColor=1;
			goto loop;
		}
		else
		{
			if(saveBotherNode->pLeftChlidNode!=NULL&&saveBotherNode->pLeftChlidNode->iColor)//兄弟有近孩子为红
			{
				rightRotating(saveBotherNode->pLeftChlidNode);//改变为远孩子
				saveBotherNode->iColor=1;
				saveBotherNode=saveBotherNode->pFatherNode;
				saveBotherNode->iColor=0;
			}
			if(saveBotherNode->pRightChlidNode!=NULL&&saveBotherNode->pRightChlidNode->iColor)//兄弟有远孩子为红
			{
				saveBotherNode->iColor=saveFatherNode->iColor;
				saveFatherNode->iColor=0;
				saveBotherNode->pRightChlidNode->iColor=0;
				leftRotating(saveBotherNode);//让孩子变为父亲颜色，父亲变为黑，兄弟旋转后变为父亲，平很
			}
			else//没有孩子
			{
				if(saveFatherNode->iColor)//父亲为红
				{
					saveFatherNode->iColor=0;
					saveBotherNode->iColor=1;
				}
				else//无法解决，向上转移问题
				{
					saveBotherNode->iColor=1;
					//saveBotherNode->pKey->printfFunction();
					deleteNode=saveFatherNode;
					saveFatherNode=saveFatherNode->pFatherNode;
					if(saveFatherNode!=NULL)
						goto loop;
				}
			}
		}
	}
}


void CRedBlackTree::balanceAddNode(SRedBlackTreeNode* addnode)
{
		SRedBlackTreeNode* useForSaveFather=addnode->pFatherNode;
		while(useForSaveFather->iColor&&addnode->iColor)
		{
			if(useForSaveFather->pLeftChlidNode==addnode&&useForSaveFather->pFatherNode->pRightChlidNode==useForSaveFather)
			{
				rightRotating(addnode);
				useForSaveFather->iColor=0;
				//printfFunction();
				leftRotating(addnode);
				useForSaveFather=addnode->pFatherNode;
				if(useForSaveFather==NULL)
					break;
			}
			else if(useForSaveFather->pRightChlidNode==addnode&&useForSaveFather->pFatherNode->pLeftChlidNode==useForSaveFather)
			{
				leftRotating(addnode);
				useForSaveFather->iColor=0;
				rightRotating(addnode);
				//printfFunction();
				useForSaveFather=addnode->pFatherNode;
				if(useForSaveFather==NULL)
					break;
			}
			else
			{
				addnode->iColor=0;
				if(useForSaveFather->pFatherNode->pLeftChlidNode==useForSaveFather)
				{
					rightRotating(useForSaveFather);
				}
				else
				{
					leftRotating(useForSaveFather);
				}
				addnode=useForSaveFather;
				useForSaveFather=addnode->pFatherNode;
				if(useForSaveFather==NULL)
					break;
			}
		}
	pRoot->iColor=0;
	//printfFunction();
}

void CRedBlackTree::printfFunction()
{
	SRedBlackTreeNode* useForSave=pRoot;
	SRedBlackTreeNode* useForSaveFather=NULL;
	int iCount=0;
	printf("?????%p\n",pRoot);
	while(1)
	{
		iCount=0;
		if(useForSave==NULL)//是否有数据
		{
			printf("------------------------------------------------------\n");
			return;
		}
		while(useForSave->pLeftChlidNode!=NULL)//是否有左孩子
		{
			iCount=1;
			useForSaveFather=useForSave;
			useForSave=useForSave->pLeftChlidNode;
		}
		//printf("%d\n",iCount);
		if(useForSaveFather!=NULL&&useForSaveFather->pRightChlidNode!=NULL&&iCount!=0)//如进入了最后的左，且右兄弟不为空，打印后变为右兄弟
		{
			if(useForSave->pRightChlidNode!=NULL)//打印当前节点的右孩子
			{
				printf("%p color=%d ",useForSave->pRightChlidNode,useForSave->pRightChlidNode->iColor);
				useForSave->pRightChlidNode->pKey->printfFunction();
			}
			printf("%p color=%d ",useForSave,useForSave->iColor);
			useForSave->pKey->printfFunction();
			useForSave=useForSaveFather->pRightChlidNode;
		}
		else
		{
			if(useForSave->pRightChlidNode!=NULL)//打印当前节点的右孩子
			{
				printf("%p color=%d ",useForSave->pRightChlidNode,useForSave->pRightChlidNode->iColor);
				useForSave->pRightChlidNode->pKey->printfFunction();
			}
			printf("%p color=%d ",useForSave,useForSave->iColor);//打印当前节点
			useForSave->pKey->printfFunction();
			useForSave=useForSaveFather;//节点上移
			while(useForSave!=NULL)//从父亲节点开始上移动
			{
				printf("%p color=%d ",useForSave,useForSave->iColor);
				useForSave->pKey->printfFunction();
				useForSaveFather=useForSave->pFatherNode;
				if(useForSaveFather!=NULL)
				{
					if(useForSaveFather->pRightChlidNode==useForSave)//如一至为右，一直上移动
					{
						useForSave=useForSaveFather;
					}
					else
					{
						/*if(useForSaveFather->pRightChlidNode!=NULL)//如中间为左，就进入另一个右子树
						{*/
							useForSave=useForSaveFather->pRightChlidNode;
							break;
						/*}
						else//没有右子树继续上移动
						{
							useForSave=useForSaveFather;
						}*/
					}
				}
				else//父亲为空，直接退出
				{
					printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
					return;
				}
			}
		}
		
	}
}
