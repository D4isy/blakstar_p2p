
#ifndef _PACKET_MACRO_H_
#define _PACKET_MACRO_H_

#include <vector>
using std::vector;





/*! ��Ŷ �ش��� ��üȭ��ų template Ŭ����
*  
*  @Version  2005. 6. 3
*
*  hoblue
*/

template<int nPacketNum> 
class CPacketData;









/*! ��Ŷ �ش��� ��Ŷ �ٵ� �ִ� data���� �����Ų macro
*  
*  @Version  2005. 6. 3
*
*  hoblue
*/


//! 1. ������Ÿ���� ���� ��Ŷ
#define PACKET_NO_DATA(PACKET_NUM, PACKET_DESCRIPTER)\
	const UINT16 PACKET_DESCRIPTER = PACKET_NUM;\




//! 2. ����Ÿ Ÿ���� �ִ� ��Ŷ
#define PACKET_DATA(PACKET_NUM , PACKET_DESCRIPTER , DATATYPE)\
	const int PACKET_DESCRIPTER = PACKET_NUM;\
	template<>\
class CPacketData<PACKET_DESCRIPTER>\
{	\
public:	   \
	static void SetDataIntoStream(IPacket* pPacket , const DATATYPE& data)\
	{\
		pPacket->PacketData.append((const char*)&data , sizeof(DATATYPE));\
	}\
	\
	static void GetDataFromStream(const IPacket* pPacket , DATATYPE& data)\
	{\
		memcpy(&data , pPacket->PacketData.c_str() , sizeof(DATATYPE));\
	}\
};\



//! 3. ��Ŷ �ٵ� ��Ģ���� data�� �������� ��Ŷ
#define PACKET_MULTI_DATA(PACKET_NUM , PACKET_DESCRIPTER , ITERATOR_DATATYPE)\
	const int PACKET_DESCRIPTER = PACKET_NUM;\
	template<>\
class CPacketData<PACKET_DESCRIPTER>\
{\
public:	   \
	static void SetMultiDataIntoStream(IPacket* pPacket , const vector<ITERATOR_DATATYPE*>& v_data)	\
	{\
		int nDataCount = static_cast<int>(v_data.size());\
		pPacket->PacketData.append((const char*)&nDataCount , sizeof(int));\
		\
		ITERATOR_DATATYPE* pData = NULL;\
		vector<ITERATOR_DATATYPE*>::const_iterator where = v_data.begin();\
		\
		while(where != v_data.end())\
		{\
			pData = *where;\
			pPacket->PacketData.append((const char*)pData , sizeof(ITERATOR_DATATYPE));\
			where++;\
		}\
	}\
	\
	static void GetMultiDataFromStream(const IPacket* pPacket , vector<ITERATOR_DATATYPE>& v_data)\
	{\
		int nvDataOffset	=	sizeof(int);\
		int nIterator		=	sizeof(ITERATOR_DATATYPE);\
		int nTotalNum = 0;\
		memcpy( &nTotalNum , pPacket->PacketData.c_str() , sizeof(int));\
		\
		ITERATOR_DATATYPE DummyData;\
		for( int i = 0 ; i < nTotalNum ; ++i)\
		{\
			memcpy(&DummyData , (pPacket->PacketData.c_str() + nvDataOffset + (i * nIterator)) , sizeof(ITERATOR_DATATYPE));\
			v_data.push_back(DummyData);			\
		}\
	}\
};\




//! 4. [��Ŷ �ٵ� = �ұ�Ģ data + ��Ģ���� data] �� �������� ��Ŷ
#define PACKET_COMPLICATED_DATA(PACKET_NUM , PACKET_DESCRIPTER , PRE_DATATYPE , ITERATOR_DATATYPE)\
	const int PACKET_DESCRIPTER = PACKET_NUM;\
	template<>\
class CPacketData<PACKET_DESCRIPTER>\
{\
public:	   \
	static void SetMultiDataIntoStream(IPacket* pPacket , const PRE_DATATYPE& preData , const vector<ITERATOR_DATATYPE*>& v_data)	\
	{\
		pPacket->PacketData.append((const char*)&preData , sizeof(PRE_DATATYPE));\
		\
		int nDataCount = static_cast<int>(v_data.size());\
		pPacket->PacketData.append((const char*)&nDataCount , sizeof(int));\
		\
		ITERATOR_DATATYPE* pData = NULL;\
		vector<ITERATOR_DATATYPE*>::const_iterator where = v_data.begin();\
		while(where != v_data.end())\
		{\
			pData = *where;\
			pPacket->PacketData.append((const char*)pData , sizeof(ITERATOR_DATATYPE));\
		where++;\
		}\
	}\
	\
	static void GetMultiDataFromStream(const IPacket* pPacket , PRE_DATATYPE& preData , vector<ITERATOR_DATATYPE>& v_data)\
	{\
		memcpy(&preData , pPacket->PacketData.c_str() , sizeof(PRE_DATATYPE));\
	\
		int nDataOffset		=	sizeof(PRE_DATATYPE) +  sizeof(int);\
		int nIterator		=	sizeof(ITERATOR_DATATYPE);\
	\
		int nTotalNum = 0;\
		memcpy( &nTotalNum , pPacket->PacketData.c_str() + sizeof(PRE_DATATYPE) , sizeof(int));\
	\
		if( nTotalNum > 0)\
		{			\
			ITERATOR_DATATYPE DummyData;\
			for( int i = 0 ; i < nTotalNum ; ++i)\
			{\
				memcpy( &DummyData , (pPacket->PacketData.c_str() + nDataOffset + (i * nIterator) ) , sizeof(ITERATOR_DATATYPE));\
			\
				v_data.push_back(DummyData);			\
			}\
		}\
	}\
	\
};\









//! 1. ������Ÿ���� ���� ��Ŷ
#define RUDP_NO_DATA(PACKET_NUM, PACKET_DESCRIPTER)\
	const unsigned short PACKET_DESCRIPTER = PACKET_NUM;\




//! 2. ����Ÿ Ÿ���� �ִ� ��Ŷ
#define RUDP_DATA(PACKET_NUM , PACKET_DESCRIPTER , DATATYPE)\
	const int PACKET_DESCRIPTER = PACKET_NUM;\
	template<>\
class CPacketData<PACKET_DESCRIPTER>\
{	\
public:	   \
	static void SetDataIntoStream(RUDPPacket* pPacket , const DATATYPE& data)\
	{\
		pPacket->PacketData.append((const char*)&data , sizeof(DATATYPE));\
	}\
	\
	static void GetDataFromStream(const RUDPPacket* pPacket , DATATYPE& data)\
	{\
		memcpy(&data , pPacket->PacketData.c_str() , sizeof(DATATYPE));\
	}\
};\



//! 3. ��Ŷ �ٵ� ��Ģ���� data�� �������� ��Ŷ
#define RUDP_MULTI_DATA(PACKET_NUM , PACKET_DESCRIPTER , ITERATOR_DATATYPE)\
	const int PACKET_DESCRIPTER = PACKET_NUM;\
	template<>\
class CPacketData<PACKET_DESCRIPTER>\
{\
public:	   \
	static void SetMultiDataIntoStream(RUDPPacket* pPacket , const vector<ITERATOR_DATATYPE*>& v_data)	\
	{\
		int nDataCount = static_cast<int>(v_data.size());\
		pPacket->PacketData.append((const char*)&nDataCount , sizeof(int));\
	\
		ITERATOR_DATATYPE* pData = NULL;\
		vector<ITERATOR_DATATYPE*>::const_iterator where = v_data.begin();\
		\
		while(where != v_data.end())\
		{\
			pData = *where;\
			pPacket->PacketData.append((const char*)pData , sizeof(ITERATOR_DATATYPE));\
			where++;\
		}\
	}\
	\
	static void GetMultiDataFromStream(const RUDPPacket* pPacket , vector<ITERATOR_DATATYPE>& v_data)\
	{\
		int nvDataOffset	=	sizeof(int);\
		int nIterator		=	sizeof(ITERATOR_DATATYPE);\
		int nTotalNum = 0;\
		memcpy( &nTotalNum , pPacket->PacketData.c_str() , sizeof(int));\
	\
		ITERATOR_DATATYPE DummyData;\
		for( int i = 0 ; i < nTotalNum ; ++i)\
		{\
			memcpy(&DummyData , (pPacket->PacketData.c_str() + nvDataOffset + (i * nIterator)) , sizeof(ITERATOR_DATATYPE));\
			v_data.push_back(DummyData);			\
		}\
	}\
};\




//! 4. [��Ŷ �ٵ� = �ұ�Ģ data + ��Ģ���� data] �� �������� ��Ŷ
#define RUDP_COMPLICATED_DATA(PACKET_NUM , PACKET_DESCRIPTER , PRE_DATATYPE , ITERATOR_DATATYPE)\
	const int PACKET_DESCRIPTER = PACKET_NUM;\
	template<>\
class CPacketData<PACKET_DESCRIPTER>\
{\
public:	   \
	static void SetMultiDataIntoStream(RUDPPacket* pPacket , const PRE_DATATYPE& preData , const vector<ITERATOR_DATATYPE*>& v_data)	\
	{\
		pPacket->PacketData.append((const char*)&preData , sizeof(PRE_DATATYPE));\
	\
		int nDataCount = static_cast<int>(v_data.size());\
		pPacket->PacketData.append((const char*)&nDataCount , sizeof(int));\
	\
		ITERATOR_DATATYPE* pData = NULL;\
		vector<ITERATOR_DATATYPE*>::const_iterator where = v_data.begin();\
		while(where != v_data.end())\
		{\
			pData = *where;\
			pPacket->PacketData.append((const char*)pData , sizeof(ITERATOR_DATATYPE));\
			where++;\
		}\
	}\
	\
	static void GetMultiDataFromStream(const RUDPPacket* pPacket , PRE_DATATYPE& preData , vector<ITERATOR_DATATYPE>& v_data)\
	{\
		memcpy(&preData , pPacket->PacketData.c_str() , sizeof(PRE_DATATYPE));\
	\
		int nDataOffset		=	sizeof(PRE_DATATYPE) +  sizeof(int);\
		int nIterator		=	sizeof(ITERATOR_DATATYPE);\
	\
		int nTotalNum = 0;\
		memcpy( &nTotalNum , pPacket->PacketData.c_str() + sizeof(PRE_DATATYPE) , sizeof(int));\
	\
		if( nTotalNum > 0)\
		{			\
			ITERATOR_DATATYPE DummyData;\
			for( int i = 0 ; i < nTotalNum ; ++i)\
			{\
				memcpy( &DummyData , (pPacket->PacketData.c_str() + nDataOffset + (i * nIterator) ) , sizeof(ITERATOR_DATATYPE));\
			\
				v_data.push_back(DummyData);			\
			}\
		}\
	}\
	\
};\


#endif