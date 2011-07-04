#ifndef __MMCORE_SERVER_SESSION_H__
#define __MMCORE_SERVER_SESSION_H__

#include "server.h"


MM_NAMESPACE_BEGIN


typedef enum
{
		SRV_RECV_WAIT_HEADER,
		SRV_RECV_WAIT_PACKAGE
}srvRecvState_t;


typedef struct __client_tag
{
		
		wchar_t			ip[256];
		uint_16_t		port;

		SOCKET			fd;

		


		cmBuffer_t		*in_buf;
		uint_64_t		last_in_stamp;
		srvRecvState_t	in_state;
		size_t			remain_len;		/*起始为NM_PACKAGE_HEADER_LENGTH，下一次为本次header所指明的长度，不能超过1K字节，否则关闭连接*/

		nmPosition_t	pos;
		bool_t			is_handshaked;

		cmBuffer_t		*out_buf;
		uint_64_t		last_out_stamp;
		
		

}srvClient_t;






static srvClient_t*	CreateClient(SOCKET cli_fd, const struct sockaddr_in *addr);
static void			DestroyClient(srvClient_t *cli);


static bool_t		SendData(srvClient_t *cli);
static bool_t		SendKeepAlive(srvClient_t *cli);
static bool_t		SendHandShake(srvClient_t *cli);
static bool_t		SendMouseLeave(srvClient_t *cli);


static bool_t		RecvData(srvClient_t *cli);
static bool_t		HandleRecvData(srvClient_t *cli, const byte_t *data, size_t length);
static bool_t		OnTimer(srvClient_t *cli);





MM_NAMESPACE_END


#endif
