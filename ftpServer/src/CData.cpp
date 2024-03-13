/***********************************************************************
 * Module:  CAge.cpp
 * Author:  crisisseer
 * Modified: 2021Äê7ÔÂ2ÈÕ 8:39:30
 * Purpose: Implementation of the class CAge
 ***********************************************************************/
 #include"CData.h"
 #include<stdio.h>
 int CData::compare(CData* pBecompared)
{
	if(pBecompared->Key.llSeqnumber-1==Key.llAcknumber)//输入的区间和自己的相连,且比自己大
	{
		Key.llAcknumber=pBecompared->Key.llAcknumber;
		return 2;
	}
	else if(pBecompared->Key.llAcknumber+1==Key.llSeqnumber)//输入的区间和自己相连，且比自己小
	{
		Key.llSeqnumber=pBecompared->Key.llSeqnumber;
		return 1;
	}
	else if(pBecompared->Key.llSeqnumber>Key.llAcknumber)//输入的区间比自己大
	{
		return -2;
	}
	else if(pBecompared->Key.llAcknumber<Key.llSeqnumber)//输入的区间比自己小
	{
		return -1;
	}
	return 0;
}

 int CData::iterator(CData* pBecompared)
{
	if(pBecompared->Key.llSeqnumber>Key.llAcknumber)//输入的区间比自己大
	{
		return -2;
	}
	else if(pBecompared->Key.llSeqnumber<Key.llSeqnumber)//输入的区间比自己小
	{
		return -1;
	}
	return 0;
}
void CData::printfFunction()
{
	printf("keyLow=%d keyBig=%d\n",Key.llSeqnumber,Key.llAcknumber);
}
