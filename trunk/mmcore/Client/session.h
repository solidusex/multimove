#ifndef __MMCORE_CLIENT_SESSION_H__
#define __MMCORE_CLIENT_SESSION_H__

#include "client.h"


MM_NAMESPACE_BEGIN



typedef enum
{
		SS_RECV_WAIT_HEADER,
		SS_RECV_WAIT_PACKAGE
}ssRecvState_t;


typedef struct __session_tag
{
		nmPosition_t	for_position;
		wchar_t			*ip;
		uint_16_t		port;
		bool_t			is_active;
		bool_t			is_handshaked;
		
		SOCKET			sockfd;
		
		cmBuffer_t		*in_buf;
		cmMutex_t		in_lock;
		ssRecvState_t	recv_state;		/*CLI_RECV_WAIT_HEADER ����£�remain_len��ʼֵΪ2, CLI_RECV_WAIT_PACKAGE��ʼֵΪheader���õ��ð�ͷ*/		
		size_t			remain_len;
		uint_64_t		last_recv_stamp;
		
		cmBuffer_t		*out_buf;
		cmMutex_t		out_lock;
		uint_64_t		last_send_stamp;
}ss_t;



/*������Ϣ*/

#define		SS_CONNECT_TO_SRV_TIMEOUT		3 * 1000	/*���Ӷ��ٺ������ʾ��ʱʧ��*/

ss_t*		SS_ConnectSession(nmPosition_t	dir, const wchar_t			*ip, uint_16_t		port); /*����������SS_CONNECT_TO_SRV_TIMEOUT����*/
void		SS_CloseSession(ss_t *ss);
bool_t		SS_IsActive(const ss_t *ss);

bool_t		SS_HasDataToSend(ss_t *ss);		/*out_buf�Ƿ��������*/
bool_t		SS_RecvData(ss_t *ss);		/*��sockfd�Է�������ʽ���նԶ����ݣ�����in_buf����,�����к�������*/
bool_t		SS_SendData(ss_t *ss);		/*��out_buf�����÷�������ʽ���ͳ�ȥ*/
bool_t		SS_OnTimer(ss_t *ss); /*������Ϊ��Ҫ������������false���__g_srv_setɾ����*/



/*����*/
bool_t		SS_SendKeepAlive(ss_t *ss);
bool_t		SS_SendHandShake(ss_t *ss);
bool_t		SS_SendEnterMsg(ss_t *ss, const nmMsg_t *msg);
bool_t		SS_SendMouseMsg(ss_t *ss, const nmMsg_t *msg);
bool_t		SS_SendKeyboardMsg(ss_t *ss, const nmMsg_t *msg);

/*����*/
bool_t		SS_HandleRecvBuffer(ss_t *s, const byte_t *data, size_t length);




MM_NAMESPACE_END


#endif
