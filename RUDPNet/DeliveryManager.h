#ifndef _DELIVERY_MANAGER_H_
#define _DELIVERY_MANAGER_H_

#include <winsock2.h>
#include "Lock.h"


//! stl
#include <deque>
using std::deque;



class CRUDPSession;



/*! network���� recv�� data�� �ٸ� ������� �������ִ� ������ �����
*  
*  @Version  2007. 5. 23
*
*  ��ȣ��
*/

class CDeliveryManager
{
private:
	deque<CRUDPSession*>	m_dequeDeliver;
	WSAEVENT	m_Event;

	CLock		m_csDeliver;
	CLock		m_csEvent;

public:
	static CDeliveryManager* GetInstance()
	{
		static CDeliveryManager instance;
		return &instance;
	}

	~CDeliveryManager();

	bool IsEmpty();

	void PushDelivery(CRUDPSession* pSession);	
	void PopDelivery();

	CRUDPSession* front();

	WSAEVENT	GetEvent();
	void		ResetEvent();

	void		Stop();
	

private:
	CDeliveryManager();
};



#endif