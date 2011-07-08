/*
Copyright (C) 2011 by Solidus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

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
		nmRecvState_t	recv_state;		/*CLI_RECV_WAIT_HEADER 情况下，remain_len初始值为2, CLI_RECV_WAIT_PACKAGE初始值为header所得到得包头*/		
		size_t			remain_len;
		uint_64_t		last_recv_stamp;
		
		cmBuffer_t		*out_buf;
		cmMutex_t		out_lock;
		uint_64_t		last_send_stamp;
}cliSession_t;



/*基本信息*/

#define		SS_CONNECT_TO_SRV_TIMEOUT		3 * 1000	/*连接多少毫秒后提示超时失败*/

cliSession_t*	SS_ConnectSession(nmPosition_t	dir, const wchar_t			*ip, uint_16_t		port); /*会阻塞至多SS_CONNECT_TO_SRV_TIMEOUT秒钟*/
void			SS_CloseSession(cliSession_t *ss);
bool_t			SS_IsActive(const cliSession_t *ss);
bool_t			SS_IsHandshaked(const cliSession_t *ss);

bool_t			SS_HasDataToSend(cliSession_t *ss);		/*out_buf是否存在数据*/
bool_t			SS_RecvData(cliSession_t *ss);		/*从sockfd以非阻塞方式接收对端数据，放入in_buf数据,并进行后续处理*/
bool_t			SS_SendData(cliSession_t *ss);		/*将out_buf数据用非阻塞方式发送出去*/
bool_t			SS_OnTimer(cliSession_t *ss); /*返回真为需要继续处理，返回false则从__g_srv_set删除掉*/



/*发送*/
bool_t			SS_SendKeepAlive(cliSession_t *ss);
bool_t			SS_SendHandShake(cliSession_t *ss);
bool_t			SS_SendEnterMsg(cliSession_t *ss, const nmMsg_t *msg);
bool_t			SS_SendMouseMsg(cliSession_t *ss, const nmMsg_t *msg);
bool_t			SS_SendKeyboardMsg(cliSession_t *ss, const nmMsg_t *msg);
bool_t			SS_SendClipDataMsg(cliSession_t *ss, const nmMsg_t *msg);

/*接收*/
bool_t			SS_HandleRecvBuffer(cliSession_t *s, const byte_t *data, size_t length);




MM_NAMESPACE_END


#endif
