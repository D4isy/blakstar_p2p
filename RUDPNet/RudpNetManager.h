#ifndef _RUDP_NET_MANAGER_H_
#define _RUDP_NET_MANAGER_H_


//! stl
#include <vector>
using std::vector;



//! ���� ����
class CRUDPReceiver;
class CRUDPDeliver;
class CRUDPNet;
class IRUDPListener;
class RUDPPacket;
class RUDPAddr;
class CRUDPResender;

template <typename OperationClass> class CThread;




#define DEFAULT_PORT					9190





/*! RUDP�� facade Ŭ���� - �ܺο��� ����� RUDP�� �������̽��� �����ϴ� Ŭ����
*  
*  @Version  2007. 5. 22
*
*  ��ȣ��
*/

class CRudpNetManager
{
private:
	CRUDPNet*	m_pRUDPNet;

	CThread<CRUDPReceiver>*	m_pReceiver;
	CThread<CRUDPDeliver>*	m_pDeliver;	
	CThread<CRUDPResender>* m_pResender;

public:
	static CRudpNetManager* GetInstance();	
	~CRudpNetManager();

	void Startup(IRUDPListener* pListener , unsigned short usPort = DEFAULT_PORT);
	void Cleanup();

	void SendPacket(const RUDPAddr& addr , RUDPPacket* pPacket , bool bReliable = false);

	bool StartUHP(vector<RUDPAddr>& v_addr);

	unsigned long  GetIP();
	unsigned short GetPort();

public:
	CRudpNetManager();

private:

	void StartThread();
	
};

#endif