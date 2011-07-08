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

#ifndef __MMCORE_SERVER_H__
#define __MMCORE_SERVER_H__

#include "../Common/common.h"
#include "../NetMsg/netmsg.h"
#pragma comment(lib, "Ws2_32.lib")

MM_NAMESPACE_BEGIN




typedef enum
{
		SRV_NOTIFY_ON_LISTEN,
		SRV_NOTIFY_ON_LOGIN,
		SRV_NOTIFY_ON_LOGOFF,
		SRV_NOTIFY_ON_ENTER,
		SRV_NOTIFY_ON_LEAVE,
		SRV_NOTIFY_ON_CLIPDATA,
}srvNotifyType_t;


#define SRV_MAX_NETWORK_INTERFACE_SUPPORT		20
#define SRV_IP_LENGTH							32
#define SRV_ADAPTER_DESC_LENGTH					256
typedef struct __server_notify_tag
{
		srvNotifyType_t	t;
		
		union{
				struct{
						struct{
								wchar_t	ip[SRV_IP_LENGTH];
						}bind_ip[SRV_MAX_NETWORK_INTERFACE_SUPPORT];
						size_t			bind_ip_cnt;
						uint_16_t		listen_port;
				}on_listen;
				
				struct{
						wchar_t			remote_ip[SRV_IP_LENGTH];
						uint_16_t		remote_port;
				}on_login;


				struct{
						wchar_t			remote_ip[SRV_IP_LENGTH];
						uint_16_t		remote_port;
				}on_logoff;


				struct {
						wchar_t			remote_ip[SRV_IP_LENGTH];
						uint_16_t		remote_port;
				}on_enter;

				struct {
						wchar_t			remote_ip[SRV_IP_LENGTH];
						uint_16_t		remote_port;
				}on_leave;

				struct {
						wchar_t			remote_ip[SRV_IP_LENGTH];
						uint_16_t		remote_port;
				}on_recv_clipdata;
		};

}srvNotify_t;


typedef void (*srvNotifyFunc_t)(void *ctx, const srvNotify_t	*notify);


typedef struct __server_init_tag	
{
		void			*ctx;
		srvNotifyFunc_t	on_notify;
}srvInit_t;


bool_t Srv_Init(const srvInit_t *init);
bool_t Srv_UnInit();


bool_t	Srv_Start(const wchar_t *bind_ip, uint_16_t port);
bool_t	Srv_Stop();
bool_t	Srv_IsStarted();


/************************************Internal****************************/



bool_t	Srv_OnNotify(const srvNotify_t *notify);


MM_NAMESPACE_END


#endif
