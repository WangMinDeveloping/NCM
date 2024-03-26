#pragma once
class Queue
{
	public:
		Queue(int iQueueLen);
		~Queue();
		int getQueueLength();
		int getQueueMemberCount();
		int _enQueue(void *pMember);
		void* deQueue();
	private:
		long long llFront;
		long long  llRear;
		int iMaximum; //原来是整型
		void *(*pArray)[];

};

