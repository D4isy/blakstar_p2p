#ifndef _NET_ADAPTER_H_ 
#define	_NET_ADAPTER_H_


//! STL
#include <map>
using std::map;




/*! ��Ʈ��ũ �������̽��� �߻�ȭ�� Ŭ����
*  
*  @Version  2006. 7. 14
*
*  ��ȣ��
*/


class CNetAdapter
{
protected:

public:
	CNetAdapter();
	~CNetAdapter();	
	
	unsigned long	GetInternetIP();

protected:
	unsigned long	GetIP(int dwIndex);
	void			GetAdapterInfo();	
};


#endif