#ifndef __MMCORE_CLIENT_SESSION_H__
#define __MMCORE_CLIENT_SESSION_H__

#include "client.h"


MM_NAMESPACE_BEGIN





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
		nmRecvState_t	recv_state;		/*CLI_RECV_WAIT_HEADER ����£�remain_len��ʼֵΪ2, CLI_RECV_WAIT_PACKAGE��ʼֵΪheader���õ��ð�ͷ*/		
		size_t			remain_len;
		uint_64_t		last_recv_stamp;
		
		cmBuffer_t		*out_buf;
		cmMutex_t		out_lock;
		uint_64_t		last_send_stamp;
}cliSession_t;



/*������Ϣ*/

#define		SS_CONNECT_TO_SRV_TIMEOUT		3 * 1000	/*���Ӷ��ٺ������ʾ��ʱʧ��*/

cliSession_t*	SS_ConnectSession(nmPosition_t	dir, const wchar_t			*ip, uint_16_t		port); /*����������SS_CONNECT_TO_SRV_TIMEOUT����*/
void			SS_CloseSession(cliSession_t *ss);
bool_t			SS_IsActive(const cliSession_t *ss);
bool_t			SS_IsHandshaked(const cliSession_t *ss);

bool_t			SS_HasDataToSend(cliSession_t *ss);		/*out_buf�Ƿ��������*/
bool_t			SS_RecvData(cliSession_t *ss);		/*��sockfd�Է�������ʽ���նԶ����ݣ�����in_buf����,�����к�������*/
bool_t			SS_SendData(cliSession_t *ss);		/*��out_buf�����÷�������ʽ���ͳ�ȥ*/
bool_t			SS_OnTimer(cliSession_t *ss); /*������Ϊ��Ҫ������������false���__g_srv_setɾ����*/



/*����*/
bool_t			SS_SendKeepAlive(cliSession_t *ss);
bool_t			SS_SendHandShake(cliSession_t *ss);
bool_t			SS_SendEnterMsg(cliSession_t *ss, const nmMsg_t *msg);
bool_t			SS_SendMouseMsg(cliSession_t *ss, const nmMsg_t *msg);
bool_t			SS_SendKeyboardMsg(cliSession_t *ss, const nmMsg_t *msg);
bool_t			SS_SendClipDataMsg(cliSession_t *ss, const nmMsg_t *msg);

/*����*/
bool_t			SS_HandleRecvBuffer(cliSession_t *s, const byte_t *data, size_t length);




MM_NAMESPACE_END


#endif
