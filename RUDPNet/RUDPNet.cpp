#include "StdAfx.h"
#include "RUDPNet.h"
#include "Mmsystem.h"
#include "RUDPAddr.h"

#include "SessionManager.h"
#include "DeliveryManager.h"
#include "RUDPStruct.h"
#include "RUDPPacket.h"
#include "UHPSender.h"

#include "RUDPZlib.h"
#include "seedx.h"

#include <algorithm>

#include "assert_msg.h"


#define MAX_BUFFER_SIZE					1024


#define RUDP_UNRELIABLE					1
#define RUDP_RELIABLE					2
#define RUDP_CONFIRMATION				3
#define UHP_PUNCH						4



#define MAX_UHP_TIMEOUT					3000


#define	MINI_ENCRYPT_SIZE				16 //! ��ȣȭ�� �ʿ��� �ּ� ũ��





//! ���μ��� ����
void AppExit()
{
	int *p = NULL;
	*p = 30;
}





CRUDPNet::CRUDPNet(IRUDPListener* pListener , unsigned short usPort):
m_bActive(false),
m_pListener(NULL),
m_pUHPSender(NULL),
m_bUHPStart(false)
{
	WSADATA wsadata;
	WSAStartup( MAKEWORD(2,2) , &wsadata);

	CreateSocket(usPort);

	m_pListener = pListener;

	m_rEvent	=	WSACreateEvent();	

	m_strRead.reserve(MAX_BUFFER_SIZE);	

	m_bActive = true;

	//! seed user key ����
	BYTE byUserKey[16];
	memcpy(byUserKey , "1ac23bcf2fd" , sizeof(byUserKey) );

	CSeed::Seed(m_dwSeedKey , byUserKey);	
}

void
CRUDPNet::CreateSocket(unsigned short usPort)
{
	m_hSocket = WSASocket(PF_INET , SOCK_DGRAM, 0 , NULL , NULL, WSA_FLAG_OVERLAPPED);	

	//!  PPEȯ�濡���� public IP�� ��������� �����ؾ��Ѵ�. (INADDR_ANY�� ����ip���� ���� - �ܺο� ����� �ȵ�)
	ZeroMemory(&m_SockAddr , sizeof(SOCKADDR_IN) );
	m_SockAddr.sin_family		=	AF_INET;   
	m_SockAddr.sin_addr.s_addr	=	m_Adapter.GetInternetIP();
	m_SockAddr.sin_port			=	htons(usPort);	

	int nCheck = bind(m_hSocket,(SOCKADDR*)&m_SockAddr,sizeof(m_SockAddr));
	assert_msg( nCheck != SOCKET_ERROR , "���� bind ����");	
}

CRUDPNet::~CRUDPNet()
{
	WSACloseEvent(m_rEvent);		

	shutdown( m_hSocket , SD_SEND );
	closesocket(m_hSocket);
}

void
CRUDPNet::StopNet()
{
	m_csActive.Lock();
	m_bActive = false;
	m_csActive.UnLock();

	SetEvent(m_rEvent);	

	CDeliveryManager::GetInstance()->Stop();
	CSessionManager::GetInstance()->Stop();
}

void
CRUDPNet::WaitForRecvEvent()
{	
	RecvOL();

	while(1)
	{
		//! FD_READ �̺�Ʈ�� �� - Ÿ�����忡�� �̺�Ʈ ���پ��� ; ����ȭ �ʿ����
		WaitForSingleObject( m_rEvent , WSA_INFINITE);		

		if( !IsActive() )
			break;

		RecvData();		

		//! recv �ɱ�
		RecvOL();
	}			
}


//! �񵿱� Read �۾��� �Ѱ��� �ɾ���Ѵ�.( ���� ���� ���ÿ� �ɸ� �ȵ� )
void
CRUDPNet::RecvOL()
{	
	WSABUF rbuf;
	memset(&rbuf,0,sizeof(WSABUF));
	rbuf.len = MAX_BUFFER_SIZE;
	rbuf.buf = const_cast<char*>(m_strRead.c_str());

	memset(&m_rOL,0,sizeof(m_rOL));
	m_rOL.hEvent  =	m_rEvent;			

	ZeroMemory(&m_FromAddr , sizeof(SOCKADDR_IN));

	DWORD byte = 0 , flag = 0;
	int sock_size = sizeof(SOCKADDR_IN);

	bool bRecv;

	do	
	{
		bRecv = false;

		m_csSocket.Lock();
		if( -1 == WSARecvFrom( m_hSocket,
			&rbuf,
			1,
			&byte,
			&flag,
			(SOCKADDR*)&m_FromAddr,
			&sock_size,
			&m_rOL,
			NULL))
		{
			int error = WSAGetLastError();
			if( error != WSA_IO_PENDING )
			{
				bRecv = true;
			}	
		}		
		m_csSocket.UnLock();		

		if( bRecv)
			Sleep(10);

	}while(bRecv);
}

bool 
CRUDPNet::IsActive()
{
	m_csActive.Lock();
	bool bResult = m_bActive;
	m_csActive.UnLock();

	return bResult;
}

void
CRUDPNet::RecvData()
{
	WSAResetEvent(m_rEvent);

	DWORD	dwTransferBytes = 0 ,dwFlag = 0;

	m_csSocket.Lock();
	BOOL bCheck = WSAGetOverlappedResult( m_hSocket, &m_rOL, &dwTransferBytes, TRUE, &dwFlag);
	m_csSocket.UnLock();

	if( bCheck )
	{
		RUDPAddr  addr( m_FromAddr.sin_addr.s_addr , ntohs(m_FromAddr.sin_port) );

		if( dwTransferBytes > 0)
			_RecvData(addr , dwTransferBytes);
	}	
}

void
CRUDPNet::_RecvData(const RUDPAddr& addr , int dwTransferBytes)
{
	//! UHP ó��
	if( IsUHPStart() && sizeof(char) == dwTransferBytes)
	{
		assert_msg(NULL != m_pListener , "Listener�� NULL��");		

		m_csUHPAddr.Lock();
		vector<RUDPAddr>::iterator where = find( m_vUHPAddr.begin() , m_vUHPAddr.end() , addr);
		if( where != m_vUHPAddr.end() )
		{
			m_pListener->OnUHPSuccess(*where);

			m_vUHPAddr.erase(where);
		}
		m_csUHPAddr.UnLock();
	}

	//! �ּ� RUDP �ش� ũ�� �̻� data�� ó��
	else if( sizeof(RUDPHeader) == dwTransferBytes )
	{					
		CRUDPSession* pSession = CSessionManager::GetInstance()->GetSession( addr );		

		RUDPHeader* pHeader = (RUDPHeader*)(m_strRead.c_str());	
		assert_msg( RUDP_CONFIRMATION == pHeader->ID , "�߸��� RUDP �ش�" );

		if( RUDP_CONFIRMATION == pHeader->ID )
		{
			//! ���� UID�� ������ ť���� �� �� ��Ŷ �����Ѵ�.
			pSession->PopResendPacket(pHeader->UID);
		}		
	}	
	else if( MINI_ENCRYPT_SIZE <= dwTransferBytes )
	{
		//! ��Ʈ�� ���� - ���� ũ�� + �ٵ�
		unsigned short usSrcSize;
		memcpy( &usSrcSize , m_strRead.c_str() , sizeof(unsigned short) );
		
		string strData;
		strData.reserve(usSrcSize);

		//!  ��ȣȭ�ϱ�
		CSeed::Decrypt(m_strRead.c_str() + sizeof(unsigned short) , m_dwSeedKey);

		//!  ���� Ǯ��
		CRUDPZlib::UnCompress( (char*)strData.c_str() , usSrcSize, m_strRead.c_str() + sizeof(unsigned short) , dwTransferBytes - sizeof(unsigned short) );


		
		//! data ó��
		CRUDPSession* pSession = CSessionManager::GetInstance()->GetSession( addr );		

		RUDPHeader* pHeader = (RUDPHeader*)(strData.c_str());
		assert_msg( (usSrcSize - sizeof(RUDPHeader) ) == pHeader->usSize , "�߸��� RUDP Packet" );

		//! �������� ������ ��Ŷ�� ó��
		if( ( usSrcSize - sizeof(RUDPHeader) ) == pHeader->usSize	)
		{
			ProcessData(strData.c_str() , pSession , pHeader , usSrcSize);
		}		
	}
}

void
CRUDPNet::ProcessData(const char* pszBuffer , CRUDPSession* pSession , const RUDPHeader* pHeader , int usSrcSize)
{
	switch( pHeader->ID )
	{
	case RUDP_RELIABLE:
		{
			//! Ȯ�� protocol ������
			SendConfirmation(pSession->GetAddr() , pHeader);

			//! Reliable ��Ŷ�� ��� �̹� ó���� ��Ŷ�� �����Ѵ�.
			if( !pSession->IsPrevUID(pHeader->UID) ) 
			{
				DeliverData( pSession , pszBuffer ,  usSrcSize);
			}				
		}
		break;	

	case RUDP_UNRELIABLE:
		{
			DeliverData( pSession , pszBuffer ,  usSrcSize);
		}
		break;	

	default:
		assert(!"���ǵ��� ���� RDUP Header");
	}	
}

//! Ȯ�� protocol ������
void
CRUDPNet::SendConfirmation(const RUDPAddr& addr , const RUDPHeader* pHeader)
{
	//! �ش� ����
	RUDPHeader header;
	header.ID		=	RUDP_CONFIRMATION;
	header.UID		=	pHeader->UID;
	header.usSize	=	0;

	SOCKADDR_IN sockaddr;
	ZeroMemory(&sockaddr , sizeof(SOCKADDR_IN));
	sockaddr.sin_family			=	AF_INET;	    	
	sockaddr.sin_addr.s_addr	=	addr.GetIP();
	sockaddr.sin_port			=	htons(addr.GetPort());	

	m_csSocket.Lock();
	sendto(m_hSocket , (const char*)&header , sizeof(RUDPHeader) , 0 , (SOCKADDR*)&sockaddr , sizeof(SOCKADDR_IN) );
	m_csSocket.UnLock();
}

void
CRUDPNet::DeliverData( CRUDPSession* pSession , const char* pszBuffer , int ret )
{
	pSession->SaveData(pszBuffer , ret);

	//! deliver�� ����
	CDeliveryManager::GetInstance()->PushDelivery(pSession);	
}

void
CRUDPNet::SendPacket(const RUDPAddr& addr , RUDPPacket* pPacket , bool bReliable)
{
	CRUDPSession* pSession = CSessionManager::GetInstance()->GetSession(addr);

	RUDPHeader* pHeader = pSession->AcquireHeader();	

	//! �ش� ����	
	if( bReliable )
		pHeader->ID		=	RUDP_RELIABLE;
	else
		pHeader->ID		=	RUDP_UNRELIABLE;

	pHeader->UID	=	pSession->AddUID();
	pHeader->usSize	=	pPacket->GetPacketSize();	

	if( bReliable )
	{
		//! ������ ť�� ����		
		pSession->PushResendPacket(pPacket , pHeader);					
	}
	else
	{
		_SendPacket(addr, pPacket , pHeader);
	}
}


//! Send�� �ѹ��� �ϳ����� �����ϴ�. (��Ƽ ������ safe)
void
CRUDPNet::_SendPacket(const RUDPAddr& addr , RUDPPacket* pPacket , RUDPHeader* pHeader)
{	
	//! ���� ��Ŷ Stream
	string strSend;		
	strSend.reserve(MAX_BUFFER_SIZE);

	strSend.append((const char*)pHeader , sizeof(RUDPHeader));

	pPacket->MakeStream(strSend);	

	unsigned short usSrcSize = strSend.size();
	string strDummy;

	//! �����ϱ�
	int nBytes = Compress(strDummy , strSend);
	if( 0 < nBytes )
	{
		//! ��ȣȭ�ϱ�
		Encrypt(nBytes , usSrcSize , strDummy , strSend);

		//! �񵿱� send ȣ��
		AsyncSendto( addr , strSend );	
	}	
}

int
CRUDPNet::Compress(string& strDummy , string& strSend)
{
	int nDummySize = 0;

	if( 64 > strSend.size() )
		nDummySize = 64;
	else
		nDummySize = strSend.size();		

	strDummy.reserve( nDummySize );	

	int nBytes = CRUDPZlib::Compress((char*)strDummy.c_str() , nDummySize , strSend.c_str() , strSend.size() );
	assert_msg(nBytes > 0 , "���� ����");
	
	if( nBytes > 0 )
	{		
		
	}
	else
	{
		//! �α� �����
		AppExit();
	}

	return nBytes;
}

void
CRUDPNet::Encrypt(int nBytes , unsigned short usSrcSize , string& strDummy , string& stream)
{
	if( MINI_ENCRYPT_SIZE > nBytes)
	{
		nBytes = MINI_ENCRYPT_SIZE;
	}

	//! 2. ��ȣȭ�ϱ�
	CSeed::Encrypt( m_dwSeedKey , strDummy.c_str() , nBytes );

	stream.clear();

	//! 3 ��Ʈ�� ���� - ���� ũ�� + �ٵ�
	stream.append((const char*)&usSrcSize , sizeof(unsigned short));

	stream.append(strDummy.c_str() , nBytes);	
}

void
CRUDPNet::AsyncSendto(const RUDPAddr& addr , string& strSend)
{
	SOCKADDR_IN sockaddr;
	ZeroMemory(&sockaddr , sizeof(SOCKADDR_IN));
	sockaddr.sin_family			=	AF_INET;	    	
	sockaddr.sin_addr.s_addr	=	addr.GetIP();
	sockaddr.sin_port			=	htons(addr.GetPort());	

	WSABUF sbuf;
	memset(&sbuf,0,sizeof(WSABUF));
	sbuf.len = strSend.size();
	sbuf.buf = (char*)strSend.c_str();

	OVERLAPPED sOL;
	memset(&sOL,0,sizeof(OVERLAPPED));	

	DWORD byte = 0 , flag = 0;

	m_csSocket.Lock();
	if( -1 == WSASendTo( m_hSocket,
		&sbuf,
		1,
		&byte,
		flag,
		(SOCKADDR*)&sockaddr,
		sizeof(SOCKADDR_IN),
		&sOL,
		NULL))
	{
		if( WSAGetLastError() != WSA_IO_PENDING )
		{

		}			
	}
	m_csSocket.UnLock();	
}

void 
CRUDPNet::OnReceive( const RUDPAddr& addr , const RUDPPacket* pPacket)
{
	m_pListener->OnReceive(addr , pPacket);
}

bool 
CRUDPNet::StartUHP(vector<RUDPAddr>& v_addr)
{
	bool bStart = false;

	if( NULL == m_pUHPSender)
	{
		//! UHP ��� �ּ� ����
		m_csUHPAddr.Lock();	
		assert_msg( m_vUHPAddr.empty() , "UHPaddr�� �ٸ� �ּҰ� ����");
		m_vUHPAddr = v_addr;		
		m_csUHPAddr.UnLock();
		
		//! UHP������ ����
		m_pUHPSender = new CThread<CUHPSender>;
		m_pUHPSender->run(this);	

		Sleep(10);

		bStart = true;
	}	

	return bStart;
}

void 
CRUDPNet::SendUHP()
{		
	DWORD dwUHPTimeout	=	timeGetTime() + MAX_UHP_TIMEOUT;

	m_csUHPAddr.Lock();
	vector<RUDPAddr> addr = m_vUHPAddr;
	m_csUHPAddr.UnLock();

	//! UHP ���� - addr.empty ��츦 ����
	m_csUHPStart.Lock();
	m_bUHPStart = true;
	m_csUHPStart.UnLock();


	//! UHP timeout - MAX_UHP_TIMEOUT
	while( timeGetTime() < dwUHPTimeout )
	{
		//! UHP send
		vector<RUDPAddr>::iterator v_where = addr.begin();
		while( v_where != addr.end() )
		{
			PunchUHP(*v_where);

			++v_where;
		}

		Sleep(30);			
	}	

	assert_msg(NULL != m_pListener , "Listener�� NULL��");

	//! timeout �̺�Ʈ
	m_csUHPAddr.Lock();
	vector<RUDPAddr>::iterator where = m_vUHPAddr.begin();
	while( where != m_vUHPAddr.end() )
	{
		m_pListener->OnUHPTimeout(*where);		

		where = m_vUHPAddr.erase(where);		
	}
	m_csUHPAddr.UnLock();
}

void
CRUDPNet::EndUHP()
{
	m_csUHPStart.Lock();
	m_bUHPStart = false;
	m_csUHPStart.UnLock();

	//! �޸� ����
	delete m_pUHPSender;
	m_pUHPSender = NULL;	
}

void
CRUDPNet::PunchUHP(const RUDPAddr& addr)
{	
	SOCKADDR_IN sockaddr;
	ZeroMemory(&sockaddr , sizeof(SOCKADDR_IN));
	sockaddr.sin_family			=	AF_INET;	    	
	sockaddr.sin_addr.s_addr	=	addr.GetIP();
	sockaddr.sin_port			=	htons(addr.GetPort());	

	char chUHP = UHP_PUNCH;

	m_csSocket.Lock();
	sendto(m_hSocket , (const char*)&chUHP , sizeof(char) , 0 , (SOCKADDR*)&sockaddr , sizeof(SOCKADDR_IN) );
	m_csSocket.UnLock();

#ifdef _DEBUG
	char szBuffer[64];
	sprintf(szBuffer , "PunchUHP : %d : %d\n" , addr.GetIP() , addr.GetPort());
	OutputDebugString(szBuffer);
#endif

}

bool
CRUDPNet::IsUHPStart()
{
	m_csUHPStart.Lock();
	bool bStart = m_bUHPStart;
	m_csUHPStart.UnLock();

	return bStart;
}

//! ���� �ּҴ� �ٲ��� ���� - ����ȭ ���ʿ�
unsigned long
CRUDPNet::GetIP()
{
	return m_SockAddr.sin_addr.s_addr;	
}

unsigned short
CRUDPNet::GetPort()
{
	return ntohs(m_SockAddr.sin_port);
}