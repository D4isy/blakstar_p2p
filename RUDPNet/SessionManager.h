#ifndef _SESSION_MANAGER_H_
#define _SESSION_MANAGER_H_

#include <winsock2.h>

#include "Pooler.h"
#include "RUDPAddr.h"
#include "RUDPSession.h"

#include "Lock.h"


//! stl
#include <map>
using std::map;


class CRUDPNet;




/*! �ܺ� Ŭ���̾�Ʈ�� ������ �߻�ȭ�� CRUDPSession�� �����ϴ� Ŭ����
*  
*  @Version  2007. 5. 23
*
*  ��ȣ��
*/

class CSessionManager
{	
	typedef map< RUDPAddr , CRUDPSession* >		SessionMap;

private:
	SessionMap				m_mapSession;
	Pooler<CRUDPSession>	m_plSession;

	WSAEVENT	m_Event;

	CLock		m_csSession;
	CLock		m_csPooler;
	CLock		m_csEvent;

public:
	static CSessionManager* GetInstance()
	{
		static CSessionManager instance;
		return &instance;
	}

	~CSessionManager();

	CRUDPSession*	GetSession(const RUDPAddr& addr);	

	void	Erase(CRUDPSession* pSession);
	void	Check();
	bool	Resend(CRUDPNet* pNet);

	WSAEVENT	GetEvent();
	void		ResetEvent();

	void		Stop();
	void		SetSignal();


private:
	CSessionManager();
};


#endif