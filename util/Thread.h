#ifndef _TEMPLATE_THREAD_
#define _TEMPLATE_THREAD_

//! �ʿ��ش�
#include "windows.h"
#include "process.h"

#include <vector>

using std::vector;



/*! ������� ������ִ� Ŭ����
*  
*  @Version  2005. 5. 12
*
*  Template���ڷ� ���� Ŭ������ ������ function���� ���������ش�.
*/


template <typename OperationClass>
class CThread
{
protected:
	HANDLE			m_hThread;
	vector<void*>	v;	

public:
	CThread():m_hThread(INVALID_HANDLE_VALUE)
	{}

	virtual ~CThread(){}

	//! ������ �Լ� ���Ḧ ��ٸ�
	void join(BOOL bWaitAll = TRUE , DWORD dwMillisec = INFINITE){
		if( m_hThread != INVALID_HANDLE_VALUE )
			WaitForSingleObject(m_hThread , dwMillisec);
	}


	//! ������ ����
	void run(void* pParam)
	{
		if( m_hThread == INVALID_HANDLE_VALUE)
		{
			unsigned int nFlag = 0 , nThreadId = 0;

			v.push_back(pParam);
			v.push_back((void*)this);

			m_hThread = (HANDLE)_beginthreadex(NULL , 0 , CThread::ThreadAddress, (void*)&v,
				nFlag , &nThreadId);		
		}		
	}	

	//! util
	HANDLE GetThreadHandle(){return m_hThread;}


private:

	//! ���ø� ���� Ŭ������ ����Լ��� ȣ��
	static unsigned int __stdcall ThreadAddress(void* pData){		
		OperationClass	op;
		IThread *thread = (IThread*)&op;

		vector<void*>* pV = (vector<void*>*)pData;

		//! ���� ����
		void* pParam	=	(*pV)[0];
		CThread<OperationClass>* pThread	=	(CThread<OperationClass>*)(*pV)[1];					

		thread->Work( pParam );

		return 0;
	}


	
};





/*! ������ �������̽� - 

1.�����忡�� ���� Ŭ������ �̰��� ��ӹ޾ƾ��Ѵ�.

2. void work�� �������̵��Ѵ�
*/


class IThread
{	
public:	
	virtual unsigned int Work( void* pParam ) = 0;	
};



#endif