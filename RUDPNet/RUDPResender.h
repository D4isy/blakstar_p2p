#ifndef _RUDP_RESENDER_H_
#define _RUDP_RESENDER_H_

#include "Thread.h"


//! ���漱��
class CRUDPNet;
class CRUDPSession;



/*! Reliable ����Ÿ�� �ٽ� ���������ִ� ������
*  
*  @Version  2007. 5. 28
*
*  ��ȣ��
*/

class CRUDPResender : public IThread
{
private:
	CRUDPNet*	m_pNet;		

public:
	CRUDPResender();
	~CRUDPResender();

	unsigned int Work( void* pParam );

private:

};


#endif