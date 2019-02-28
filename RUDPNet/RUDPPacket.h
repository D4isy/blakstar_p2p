#ifndef _RUDP_PACKET_H_
#define _RUDP_PACKET_H_



//! stl
#include <string>
using std::string;






/*! P2P�� ���Ǵ� ��Ŷ Ŭ����
*  
*  @Version  2007. 5.23
*
*  ��ȣ�� 
*/

class RUDPPacket
{
public:
	unsigned int m_uPacketID;	

	//! ��Ŷ data
	string	 PacketData;

public:
	RUDPPacket();
	RUDPPacket(const unsigned int uPacketID);	

	void Reset();	

	void operator=(const RUDPPacket& packet);	

	void ProcessStream( const char* pszBuffer , int ret );

	void MakeStream(string& strData);

	int  GetPacketSize();
};


#endif