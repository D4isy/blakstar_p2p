#ifndef _UHP_SENDER_H_
#define _UHP_SENDER_H_

#include "Thread.h"


//! ���漱��




/*! UHP�� ��Ŷ�� ��� Ŭ���̾�Ʈ���� �����ð� ���� send�ϴ� ������
*  
*  @Version  2007. 5. 31
*
*  ��ȣ��
*/

class CUHPSender : public IThread
{
private:	

public:
	CUHPSender();
	~CUHPSender();

	unsigned int Work( void* pParam );

private:

};


#endif