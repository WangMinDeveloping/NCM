#pragma once
#define RED 0
#define BLACK 1
template<class KeyType,class ValueType>
class CRedBlackTreeNode
{
        public:
                CRedBlackTreeNode<KeyType,ValueType> *pLeft;
                CRedBlackTreeNode<KeyType,ValueType> *pRight;
                CRedBlackTreeNode<KeyType,ValueType> *pFather;
                KeyType key;
                bool bColor;
                ValueType value;//键重复时的链表
                CRedBlackTreeNode(CRedBlackTreeNode<KeyType,ValueType> *pFatherNode=0)
                {
                        pLeft=0;
                        pRight=0;
                        pFather=pFatherNode;
                        key=0;
                        bColor=BLACK;
                        value=0;//不支持键重复

                }
                ~CRedBlackTreeNode(){};
};
