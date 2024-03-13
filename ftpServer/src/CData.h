/***********************************************************************
 * Module:  CAge.h
 * Author:  crisisseer
 * Modified: 2021Äê7ÔÂ2ÈÕ 8:39:30
 * Purpose: Declaration of the class CAge
 ***********************************************************************/

#if !defined(__CData_h)
#define __CData_h
struct key
{
	unsigned int llSeqnumber;
	unsigned int llAcknumber;
};
class CData
{
public:
	struct key Key;
	int compare(CData* pBecompared);
	int iterator(CData* pBecompared);
	void printfFunction();
protected:
private:

};

#endif
