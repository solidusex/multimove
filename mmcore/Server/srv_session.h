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

#ifndef __MMCORE_SERVER_SESSION_H__
#define __MMCORE_SERVER_SESSION_H__

#include "server.h"


MM_NAMESPACE_BEGIN



typedef struct __server_session_tag
{
		
		wchar_t			ip[256];
		uint_16_t		port;
		
		nmPosition_t	pos;
		bool_t			is_handshaked;
		bool_t			is_entered;
		SOCKET			fd;						

		cmBuffer_t		*in_buf;
		cmMutex_t		in_lock;
		uint_64_t		last_in_stamp;		/*�����ʱ���*/
		nmRecvState_t	in_state;				
		size_t			remain_len;			/*��ʼΪNM_PACKAGE_HEADER_LENGTH����һ��Ϊ����header��ָ���ĳ��ȣ����ܳ���1K�ֽڣ�����ر�����*/

		cmBuffer_t		*out_buf;
		cmMutex_t		out_lock;
		uint_64_t		last_out_stamp;		/*������ʱ���*/
}srvSession_t;




srvSession_t*	SS_OnClientSession(SOCKET cli_fd, const struct sockaddr_in *addr);
void			SS_CloseClientSession(srvSession_t *ss);

bool_t			SS_SendKeepAlive(srvSession_t *ss);
bool_t			SS_SendHandShakeReply(srvSession_t *ss);
bool_t			SS_SendMouseLeave(srvSession_t *ss, const nmMsg_t *msg);
bool_t			SS_SendClipDataMsg(srvSession_t *ss, const nmMsg_t *msg);

bool_t			SS_OnPackage(srvSession_t		*ss, const byte_t *data, size_t len);

bool_t			SS_HasDataToSend(srvSession_t *ss);		/*out_buf�Ƿ��������*/

bool_t			SS_OnSendData(srvSession_t *ss);

bool_t			SS_OnRecvData(srvSession_t *ss);

bool_t			SS_OnTimer(srvSession_t *ss);

bool_t			SS_IsEntered(const srvSession_t *ss);



MM_NAMESPACE_END


#endif
