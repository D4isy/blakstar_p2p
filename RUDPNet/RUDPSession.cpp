#include "StdAfx.h"
#include "RUDPSession.h"

#include "SessionManager.h"
#include "RUDPNet.h"
#include "assert_msg.h"


#define MAX_RESPONSE_COUNT		300



CRUDPSession::CRUDPSession()
{	
	m_plPacket.Initialize(10,10);
	m_plHeader.Initialize(10,10);

	Reset();
}

CRUDPSession::~CRUDPSession()
{
}

void
CRUDPSession::Reset()
{
	m_ulRef		=	0;
	m_uResponse	=	0;
	m_chUID		=	0;
	m_chPrevUID	=	0;

	deque<RUDPPacket*>::iterator recv_where = m_dequeRecv.begin();
	while( recv_where != m_dequeRecv.end() )
	{
		m_plPacket.Release(*recv_where);
		++recv_where;
	}

	m_dequeRecv.clear();	

	m_dequeResend.clear();

	m_addr.Reset();
}


void
CRUDPSession::AddRef()
{
	m_csRef.Lock();
	++m_ulRef;
	m_csRef.UnLock();
}

void
CRUDPSession::RelRef()
{
	m_csRef.Lock();
	unsigned long uRef = --m_ulRef;
	m_csRef.UnLock();

	if ( 0 == uRef )
	{	
		//! ���� �޸� pooler���� �ݳ�
		CSessionManager::GetInstance()->Erase(this);		
	}		
}

//! Data -> CRUDPPacket���� ��ȯ
void 
CRUDPSession::SaveData(const char* pszBuffer , int ret)
{
	m_csResponse.Lock();
	m_uResponse = 0;
	m_csResponse.UnLock();

	//! RUDPPacket ����	
	RUDPPacket* pPacket = AcquirePacket();

	pPacket->ProcessStream( pszBuffer + sizeof(RUDPHeader) , ret - sizeof(RUDPHeader) );

	//! ���
	m_csRecvQueue.Lock();
	m_dequeRecv.push_back(pPacket);
	m_csRecvQueue.UnLock();	
}

RUDPPacket*
CRUDPSession::GetPacket()
{	
	RUDPPacket* pPacket = NULL;

	m_csRecvQueue.Lock();

	if( !m_dequeRecv.empty() )
		pPacket = m_dequeRecv.front();

	m_csRecvQueue.UnLock();
	
	return pPacket;
}

void
CRUDPSession::PopPacket()
{
	//! ť���� �����
	m_csRecvQueue.Lock();
	RUDPPacket* pPacket = m_dequeRecv.front();
	m_dequeRecv.pop_front();
	m_csRecvQueue.UnLock();

	//! �޸� �ݳ�
	ReleasePacket(pPacket);	
}

//! ������ �� �ѹ��� �ʱ�ȭ - ����ȭ ���ʿ�
void 
CRUDPSession::SetAddr(const RUDPAddr& addr)
{	
	m_addr = addr;
}

const RUDPAddr&
CRUDPSession::GetAddr()
{
	return m_addr;
}

bool	
CRUDPSession::IsExpired()
{
	bool bExpired = false;

	m_csResponse.Lock();

	if( MAX_RESPONSE_COUNT == m_uResponse )
	{
		bExpired = true;
	}
	else
	{
		++m_uResponse;
	}

	assert_msg( m_uResponse <= MAX_RESPONSE_COUNT , "Expired ��� ����");

	m_csResponse.UnLock();

	return bExpired;
}

void
CRUDPSession::PushResendPacket(RUDPPacket* pPacket , RUDPHeader* pHeader)
{		
	//! RUDPPacket ����
	RUDPPacket* pData = AcquirePacket();
	*pData = *pPacket;	

	RUDPNotConfirmation NotConfirm( pData , pHeader );

	//! ������ ť�� ����
	m_csResendQueue.Lock();
	m_dequeResend.push_back(NotConfirm);
	m_csResendQueue.UnLock();

	CSessionManager::GetInstance()->SetSignal();
}

void
CRUDPSession::PopResendPacket(const unsigned char UID)
{
	//! ������ ť���� ����
	m_csResendQueue.Lock();

	if( !m_dequeResend.empty() )
	{
		RUDPNotConfirmation& data = m_dequeResend.front();	
		if( UID == data.m_pHeader->UID)
		{
			ReleaseHeader(data.m_pHeader);

			ReleasePacket(data.m_pPacket);	

			m_dequeResend.pop_front();		
		}
	}

	m_csResendQueue.UnLock();	
}

RUDPPacket*
CRUDPSession::AcquirePacket()
{
	m_csPooler.Lock();
	RUDPPacket* pPacket = m_plPacket.Acquire();	
	m_csPooler.UnLock();

	return pPacket;
}

void
CRUDPSession::ReleasePacket(RUDPPacket* pPacket)
{
	m_csPooler.Lock();
	m_plPacket.Release(pPacket);	
	m_csPooler.UnLock();
}

RUDPHeader* 
CRUDPSession::AcquireHeader()
{
	m_csHeader.Lock();
	RUDPHeader* pHeader = m_plHeader.Acquire();
	m_csHeader.UnLock();

	return pHeader;
}

void
CRUDPSession::ReleaseHeader(RUDPHeader* pHeader)
{
	m_csHeader.Lock();
	m_plHeader.Release(pHeader);	
	m_csHeader.UnLock();
}

bool 
CRUDPSession::Resend(CRUDPNet* pNet)
{
	bool bResend = false;

	m_csResendQueue.Lock();

	if( !m_dequeResend.empty() )
	{
		RUDPNotConfirmation& data =  m_dequeResend.front();
		pNet->_SendPacket( GetAddr() , data.m_pPacket , data.m_pHeader );

		bResend = true;
	}

	m_csResendQueue.UnLock();

	return bResend;
}

//! �� �����忡���� ���ٵ� - ����ȭ ���ʿ�
unsigned char
CRUDPSession::AddUID()
{
	m_csUID.Lock();
	unsigned char UID = ++m_chUID;
	m_csUID.UnLock();

	return UID;
}

//! �� �����忡���� ���ٵ� - ����ȭ ���ʿ�
bool 
CRUDPSession::IsPrevUID(unsigned char UID)
{
	bool bResult = false;

	if( m_chPrevUID == UID )
		bResult = true;
	else
	{
		m_chPrevUID = UID;
	}

	return bResult;
}
