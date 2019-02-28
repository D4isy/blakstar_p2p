#include "StdAfx.h"
#include "SessionManager.h"
#include "RUDPNet.h"




CSessionManager::CSessionManager()
{
	m_plSession.Initialize(10,10);

	m_Event = WSACreateEvent();
}

CSessionManager::~CSessionManager()
{
	WSACloseEvent(m_Event);
}

CRUDPSession* 
CSessionManager::GetSession(const RUDPAddr& addr)
{
	CRUDPSession* pSession = NULL;

	m_csSession.Lock();

	//! ������ �ִ� �� �˻�
	SessionMap::iterator where = m_mapSession.find(addr);
	if( where != m_mapSession.end() )
	{
		pSession = where->second;
	}
	
	//! ������ ���� �����Ѵ�.
	else
	{
		m_csPooler.Lock();
		pSession = m_plSession.Acquire();
		m_csPooler.UnLock();

		pSession->SetAddr(addr);

		pSession->AddRef();

		m_mapSession.insert( SessionMap::value_type(addr , pSession) );
	}

	m_csSession.UnLock();

	return pSession;
}

void	
CSessionManager::Erase(CRUDPSession* pSession)
{
	m_csPooler.Lock();
	m_plSession.Release(pSession);
	m_csPooler.UnLock();	
}

void	
CSessionManager::Check()
{
	m_csSession.Lock();

	SessionMap::iterator where = m_mapSession.begin();
	while( where != m_mapSession.end() )
	{
		CRUDPSession* pSession = where->second;

		if( pSession->IsExpired() )
		{
			//! ��Ͽ��� ����
			where = m_mapSession.erase(where);			

			//! ���� ī��Ʈ ����
			pSession->RelRef();
		}
		else
		{
			++where;
		}
	}

	m_csSession.UnLock();
}

bool
CSessionManager::Resend(CRUDPNet* pNet)
{
	bool bResend = false;

	m_csSession.Lock();
	
	SessionMap::iterator where = m_mapSession.begin();
	while( where != m_mapSession.end() )
	{
		CRUDPSession* pSession = where->second;

		//! �� ������ ù ���� resend ��Ŷ�� ������.
		if( pSession->Resend(pNet) )
		{
			bResend = true;
		}

		++where;
	}

	m_csSession.UnLock();	

	return bResend;
}

WSAEVENT	
CSessionManager::GetEvent()
{
	m_csEvent.Lock();
	WSAEVENT event = m_Event;
	m_csEvent.UnLock();

	return event;
}

void
CSessionManager::ResetEvent()
{
	m_csEvent.Lock();
	WSAResetEvent(m_Event);
	m_csEvent.UnLock();
}

void	
CSessionManager::Stop()
{
	SetSignal();
}

void
CSessionManager::SetSignal()
{
	m_csEvent.Lock();
	SetEvent(m_Event);
	m_csEvent.UnLock();
}