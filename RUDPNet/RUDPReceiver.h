#ifndef _RUDP_RECEIVER_H_
#define _RUDP_RECEIVER_H_

#include "Thread.h"


//! ���漱��
class CRUDPNet;



/*! Network�� data�� recv ��
*  
*  @Version  2007. 5. 22
*
*  ��ȣ��
*/


class CRUDPReceiver : public IThread
{
private:
	CRUDPNet*	m_pNet;	

public:
	CRUDPReceiver();
	~CRUDPReceiver();

	unsigned int Work( void* pParam );
};


#endif
