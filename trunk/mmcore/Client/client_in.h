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
		/*CLI_RECV_WAIT_HEADER 情况下，remain_len初始值为4, CLI_RECV_WAIT_PACKAGE初始值为header所得到得包头*/		
		cliRecvState_t	recv_state;
		size_t			remain_len;
		uint_64_t		last_recv_tm;
		
		uint_64_t		last_send_tm;
}cliSrv_t;



/*基本信息*/

#define CLI_CONNECT_TO_SRV_TIMEOUT		3000

static cliSrv_t*	Cli_CreateServer(nmPosition_t	dir, const wchar_t			*ip, uint_16_t		port); /*会阻塞至多三秒钟*/
static void			Cli_DestroyServer(cliSrv_t *srv);
static bool_t		Cli_IsActiveSide(const cliSrv_t *srv);

static bool_t		Cli_HasDataToSend(cliSrv_t *srv);/*out_buf是否存在数据*/

static bool_t		Cli_RecvData(cliSrv_t *srv);/*从sockfd以非阻塞方式接收对端数据，放入in_buf数据,当达到一定条件，会调用Cli_HandleRecvBuffer*/
static bool_t		Cli_SendData(cliSrv_t *srv);/*将out_buf数据用非阻塞方式发送出去*/
static bool_t		Cli_OnTimer(cliSrv_t *srv); /*返回真为需要继续处理，返回false则从__g_srv_set删除掉*/

/*发送*/
static bool_t		Cli_SendKeepAlive(cliSrv_t *srv);
static bool_t		Cli_SendHandShake(cliSrv_t *srv, cliServerDir_t dir);
static bool_t		Cli_SendMouseMsg(cliSrv_t *srv, const hkMouseEvent_t *mouse_msg);
static bool_t		Cli_SendKeyboardMsg(cliSrv_t *srv, const hkKeyboardEvent_t *keyboard);
static bool_t		Cli_SendEnterMsg(cliSrv_t *srv, const hkEnterEvent_t     *enter);
/*接收*/
static bool_t		Cli_HandleRecvBuffer(cliSrv_t *srv, const byte_t *data, size_t length);




MM_NAMESPACE_END


#endif
