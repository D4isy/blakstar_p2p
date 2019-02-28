// testRUDP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winsock2.h>


//! �޸� ����
#define DLL_VLD_LIB			"../../lib/vldmtdll.lib"
#define MT_VLD_LIB			"../../lib/vldmt.lib"
#define VLD_LIB				"../../lib/vld.lib"

#include "vld.h"



//! RUDP header
#include "RudpNetManager.h"
#include "RUDPListener.h"
#include "RUDPAddr.h"
#include "RUDPPacket.h"


//! util
#include "PacketMacro.h"
#include "assert_msg.h"



using std::cout;
using std::endl;





//! ��Ŷ ����

#pragma pack(1)

struct TEST
{
	int a;
	int b;
	char szBuffer[1024];
};

#pragma pack()

RUDP_DATA(1 , RUDP_TEST , TEST)





int g_Test = 0;



//! Recv �̺�Ʈ - ��Ŷ ó���ϱ�

class MyListener : public IRUDPListener
{
public:
	void OnReceive(const RUDPAddr& addr , const RUDPPacket* pPacket)
	{
		TEST data;

		memcpy(&data , pPacket->PacketData.c_str() , sizeof(TEST));

	//	TEST data;
	//	CPacketData<RUDP_TEST>::GetDataFromStream(pPacket , data);
		cout << "addr : " << addr.GetPort() << " : " << pPacket->m_uPacketID << " : " << data.a << " : " << data.b << " : " << data.szBuffer << endl;

		assert_msg( g_Test == data.a , "��Ŷ ������ ���� ����");

		g_Test++;
	}
};




int Ask(const char* pszMsg)
{
	char szNum[64];
	int nNum = 0;

	cout << pszMsg << endl;	
	gets(szNum);

	return atoi(szNum);
}







#define			TEST_ALONE //! TEST_ALONE , TEST_OTHER



#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

int _tmain(int argc, _TCHAR* argv[])
{		
#ifdef _DEBUG
	int tmpDbgFlag;

	tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);   
	tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;

	_CrtSetDbgFlag(tmpDbgFlag);		

#endif	





#ifdef TEST_ALONE

	//! RUDPNet Start~!!
	MyListener listener;
	CRudpNetManager::GetInstance()->Startup(&listener);

	
	//! ���� display
	SOCKADDR_IN sock_addr;
	ZeroMemory(&sock_addr , sizeof(SOCKADDR_IN));
	sock_addr.sin_addr.s_addr = CRudpNetManager::GetInstance()->GetIP();
	char* pszIP = inet_ntoa( sock_addr.sin_addr );

	cout << "Operating : " << pszIP << " : " << CRudpNetManager::GetInstance()->GetPort()  << endl;



	//! Reliable ��Ŷ test
	char szNum[64];
	int nNum = 0;

	while(1)
	{
		cout << "push any Num to be sending packets" << endl;
		gets(szNum);

		g_Test = 0;

		int nNum = atoi(szNum);

		if( 0 == nNum)
			break;
		else
		{
			RUDPAddr addr1("192.168.0.5" , DEFAULT_PORT);			

			for( int i = 0 ; i < nNum ; ++i )
			{
				//! ��Ŷ ����
				RUDPPacket packet;
				packet.m_uPacketID = 10;

				TEST test;
				test.a = i;
				test.b = 20;
				strncpy( test.szBuffer , "abcdefghijklmn" , sizeof(test.szBuffer) );

				CPacketData<RUDP_TEST>::SetDataIntoStream(&packet , test);

				//! 1. UnReliable Send
				CRudpNetManager::GetInstance()->SendPacket(addr1 , &packet , false);

				//! 2. Reliable Send
				//CRudpNetManager::GetInstance()->SendPacket(addr1 , &packet , true);		
			}		
		}		
	}

#endif




#ifdef TEST_OTHER

	int nMyPort		=	Ask("�۵��� My ��Ʈ : ");
	int nYourPort	=	Ask("��Ŷ�� ���� Your ��Ʈ : ");	


	//! RUDPNet Start~!!
	MyListener listener;
	CRudpNetManager::GetInstance()->Startup(&listener , nMyPort);



	//! ���� display
	SOCKADDR_IN sock_addr;
	ZeroMemory(&sock_addr , sizeof(SOCKADDR_IN));
	sock_addr.sin_addr.s_addr = CRudpNetManager::GetInstance()->GetIP();
	char* pszIP = inet_ntoa( sock_addr.sin_addr );

	cout << "Operating : " << pszIP << " : " << CRudpNetManager::GetInstance()->GetPort()  << endl;



	//! Reliable ��Ŷ test
	char szNum[64];
	int nNum = 0;

	while(1)
	{
		cout << "push any Num to be sending packets" << endl;
		gets(szNum);	

		g_Test = 0;

		int nNum = atoi(szNum);

		if( 0 == nNum)
			break;

		if( nNum > 1)
		{
			RUDPAddr addr1("10.255.253.160" , nYourPort);			

			for( int i = 0 ; i < nNum ; ++i )
			{
				//! ��Ŷ ����
				RUDPPacket packet;
				packet.m_uPacketID = 10;

				TEST test;
				test.a = i;
				test.b = 20;
				strncpy( test.szBuffer , "abcdefghijklmn" , sizeof(test.szBuffer) );

				CPacketData<RUDP_TEST>::SetDataIntoStream(&packet , test);

				CRudpNetManager::GetInstance()->SendPacket(addr1 , &packet , true);					
			}		
		}		
	}

#endif


	//! RDUP ����
	CRudpNetManager::GetInstance()->Cleanup();	
	

	return 0;
}

