#ifndef _RUDP_DELIVER_H_
#define _RUDP_DELIVER_H_


#include "Thread.h"


//! ���漱��
class CRUDPNet;
class CRUDPSession;



/*! Network recv�� ����Ÿ�� ó���ϴ� ������
*  
*  @Version  2007. 5. 22
*
*  ��ȣ��
*/

class CRUDPDeliver : public IThread
{
private:
	CRUDPNet*	m_pNet;		

public:
	CRUDPDeliver();
	~CRUDPDeliver();

	unsigned int Work( void* pParam );

private:
	void OnProcess( CRUDPSession* pSession );
};


#endif