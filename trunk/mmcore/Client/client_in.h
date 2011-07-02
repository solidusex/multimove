#ifndef __MMCORE_CLIENT_H__
#define __MMCORE_CLIENT_H__

#include "client.h"


MM_NAMESPACE_BEGIN



typedef enum
{
		CLI_RECV_WAIT_HEADER,
		CLI_RECV_WAIT_PACKAGE
}cliRecvState_t;


typedef struct __client_server_tag
{
		nmPosition_t	pos;
		wchar_t			*ip;
		uint_16_t		port;
		bool_t			is_active_side;

		SOCKET			sockfd;
		
		cmBuffer_t		*in_buf;
		/*CLI_RECV_WAIT_HEADER ����£�remain_len��ʼֵΪ4, CLI_RECV_WAIT_PACKAGE��ʼֵΪheader���õ��ð�ͷ*/		
		cliRecvState_t	recv_state;
		size_t			remain_len;
		uint_64_t		last_recv_tm;
		
		uint_64_t		last_send_tm;
}cliSrv_t;



/*������Ϣ*/

#define CLI_CONNECT_TO_SRV_TIMEOUT		3000

static cliSrv_t*	Cli_CreateServer(nmPosition_t	dir, const wchar_t			*ip, uint_16_t		port); /*����������������*/
static void			Cli_DestroyServer(cliSrv_t *srv);
static bool_t		Cli_IsActiveSide(const cliSrv_t *srv);

static bool_t		Cli_HasDataToSend(cliSrv_t *srv);/*out_buf�Ƿ��������*/

static bool_t		Cli_RecvData(cliSrv_t *srv);/*��sockfd�Է�������ʽ���նԶ����ݣ�����in_buf����,���ﵽһ�������������Cli_HandleRecvBuffer*/
static bool_t		Cli_SendData(cliSrv_t *srv);/*��out_buf�����÷�������ʽ���ͳ�ȥ*/
static bool_t		Cli_OnTimer(cliSrv_t *srv); /*������Ϊ��Ҫ������������false���__g_srv_setɾ����*/

/*����*/
static bool_t		Cli_SendKeepAlive(cliSrv_t *srv);
static bool_t		Cli_SendHandShake(cliSrv_t *srv, cliServerDir_t dir);
static bool_t		Cli_SendMouseMsg(cliSrv_t *srv, const hkMouseEvent_t *mouse_msg);
static bool_t		Cli_SendKeyboardMsg(cliSrv_t *srv, const hkKeyboardEvent_t *keyboard);
static bool_t		Cli_SendEnterMsg(cliSrv_t *srv, const hkEnterEvent_t     *enter);
/*����*/
static bool_t		Cli_HandleRecvBuffer(cliSrv_t *srv, const byte_t *data, size_t length);




MM_NAMESPACE_END


#endif
