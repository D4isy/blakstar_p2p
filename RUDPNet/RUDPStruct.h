#ifndef _RUDP_STRUCT_H_
#define _RUDP_STRUCT_H_




#pragma pack(1)




/*! RUDP ����� ���� ���������� ���̴� data �ش�
*  
*  @Version  2007. 5. 22
*
*  ��ȣ�� 
*/

struct RUDPHeader
{
	unsigned char	ID;				//! ��Ŷ ���� - Reliable , UnReliable��
	unsigned char   UID;			//! ��Ŷ ���� ���̵� - ��ȯ��
	unsigned short	usSize;

	void Reset(){ID=0L;UID=0;usSize=0;}
};








#pragma pack()


#endif

