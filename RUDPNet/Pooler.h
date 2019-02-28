#ifndef _POOLER_H_
#define _POOLER_H_

#include <deque>
#include <set>

using std::deque;
using std::set;



/*! Ư�� �ڷ����� ���ؼ� �޸𸮸� �̸� ������
*  
*  @Version  2007. 5. 23
*
*  ��ȣ��
*/


template<typename Object>
class  Pooler
{
public:
	typedef	Object		AObject;

private:
	unsigned long	m_dwIncreaseNum;

	deque<AObject*>	m_dequePooler;
	set<AObject*>	m_setRelease;


public:
	Pooler();
	virtual ~Pooler();

	//! �ʱ�ȭ
	bool   Initialize( unsigned long dwInitNum , unsigned long dwIncreaseNum);

	//! ����
	void    UnInitialize();	

	//! ��� , ��ȯ
	Object* Acquire();
	void    Release( AObject* pObject);


protected:
	//! Pooler�� ���� Object ����
	Object* CreateObject();

};


template<typename Object>
Pooler<Object>::Pooler():
m_dwIncreaseNum(0)
{

}


template<typename Object>
Pooler<Object>::~Pooler()
{
	//! UnInitialize�� ȣ�� �� �Ǿ��� ��
	UnInitialize();
}


//! ��ϵ� Factory object�� Pooler �Ҹ��ڿ��� delete ���ش�.
template<typename Object>
bool   
Pooler<Object>::Initialize( unsigned long dwInitNum , unsigned long dwIncreaseNum )
{	
	bool bResult = false;	

	if( m_dequePooler.empty() )
	{
		m_dwIncreaseNum =	dwIncreaseNum;		

		//! ��ü ����
		for(unsigned int nIndex = 0 ; nIndex < dwInitNum ; nIndex++)
		{
			AObject* pObject = CreateObject();	
			m_dequePooler.push_back(pObject);
		}

		bResult = true;
	}

	return bResult;
}

template<typename Object>
void    
Pooler<Object>::UnInitialize()
{
	//! Pool���� pObject ����
	if( !m_dequePooler.empty() )
	{
		deque<AObject*>::iterator where = m_dequePooler.begin();
		while( where != m_dequePooler.end() )
		{
			delete *where;
			where++;
		}

		m_dequePooler.clear();
	}

	//! Release ��Ͼ��� Object ����
	if( !m_setRelease.empty())
	{
		set<AObject*>::iterator where = m_setRelease.begin();
		while( where != m_setRelease.end() )
		{
			delete *where;
			where++;
		}

		m_setRelease.clear();
	}
}


//! Pooler�� ���� Object ����
template<typename Object>
Object*
Pooler<Object>::CreateObject()
{
	return new AObject;
}

template<typename Object>
Object*
Pooler<Object>::Acquire()
{   	
	AObject* pResult = NULL;

	//! �޸� ����
	if( m_dequePooler.empty())
	{	
		//! ��ü ����
		for(unsigned int nIndex = 0 ; nIndex < m_dwIncreaseNum ; nIndex++)
		{
			AObject* pObject = CreateObject();	
			m_dequePooler.push_back(pObject);
		}
	}	

	//! �޸� ���
	pResult = m_dequePooler.front();
	m_dequePooler.pop_front();

	//! Release ��Ͽ� ���
	m_setRelease.insert(pResult);

	return pResult;		
}


template<typename Object>
void
Pooler<Object>::Release( AObject* pObject)
{
	if( !m_setRelease.empty() )
	{
		//! Release ��Ͽ��� ����
		set<AObject*>::iterator where = m_setRelease.find(pObject);
		if( where != m_setRelease.end())
		{
			m_setRelease.erase(where);

			//! �޸� pooler�� �����ϱ�
			pObject->Reset();
			m_dequePooler.push_back(pObject);
		}		
	}	
}






#endif
