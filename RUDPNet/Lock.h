#ifndef _LOCK_H_
#define _LOCK_H_

#include "windows.h"



/*! CRITICAL_SECTION ����ȭ ��ü��  proxy�ϴ� Ŭ����
*  
*  @Version  2005. 6. 10
*
*  hoblue
*/

class CLock
{
protected:
	CRITICAL_SECTION m_insCS;
	BOOL	bLock;

public:
	CLock();
	virtual ~CLock();

	const CLock & operator = (const CLock & lock);

	void Lock();
	void UnLock();

private:	
	BOOL IsLock() const;
};

#endif
