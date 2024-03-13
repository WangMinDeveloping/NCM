#pragma once
unsigned char* invertBytes(unsigned char *pSource,unsigned char *pDestination,int length);
unsigned short invertShort(unsigned short wData);
unsigned int invertInt(unsigned int uData);
unsigned short checkSum(unsigned short * addr, int count);
