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

#include "srv_notify.h"

#include <Iphlpapi.h>
#pragma comment(lib, "Iphlpapi.lib")


MM_NAMESPACE_BEGIN


bool_t	Srv_NotifyOnListen(const wchar_t *bind_ip, uint_16_t port)
{
		srvNotify_t		notify;
		Com_memset(&notify, 0, sizeof(notify));
		notify.t = SRV_NOTIFY_ON_LISTEN;
		notify.on_listen.listen_port = port;

		if(bind_ip != NULL)
		{
				Com_wcscpy(notify.on_listen.bind_ip[0].ip, bind_ip);
				notify.on_listen.bind_ip_cnt = 1;
		}else
		{
				ULONG buf_size;
				IP_ADAPTER_INFO *ptr, *curr;
				size_t i;
				buf_size = 0;
				ptr = NULL;

				if(GetAdaptersInfo(NULL,&buf_size) != ERROR_BUFFER_OVERFLOW)
				{
						goto END_POINT;
				}

				ptr = (IP_ADAPTER_INFO*)Com_NEWARR0(byte_t, buf_size);

				if(GetAdaptersInfo(ptr,&buf_size) != ERROR_SUCCESS)
				{
						goto END_POINT;
				}
				
				i = 0;

				curr = ptr;
				while(curr)
				{
						Com_swprintf(notify.on_listen.bind_ip[i].ip, SRV_IP_LENGTH, L"%S", curr->IpAddressList.IpAddress.String);
						notify.on_listen.bind_ip_cnt++;
						i++;
						curr = curr->Next;
				}

END_POINT:
				if(ptr != NULL)
				{
						Com_DEL(ptr);
						ptr = NULL;
				}
		}

		
		Srv_OnNotify(&notify);		

		return true;
}







bool_t	Srv_NotifyOnLogin(const wchar_t *remote_ip, uint_16_t port)
{
		srvNotify_t		notify;
		Com_ASSERT(remote_ip != NULL);
		Com_memset(&notify, 0, sizeof(notify));
		notify.t = SRV_NOTIFY_ON_LOGIN;
		Com_wcscpy(notify.on_login.remote_ip, remote_ip);
		notify.on_logoff.remote_port = port;
		return Srv_OnNotify(&notify);
}


bool_t	Srv_NotifyOnLogoff(const wchar_t *remote_ip, uint_16_t port)
{
		srvNotify_t		notify;
		Com_ASSERT(remote_ip != NULL);
		Com_memset(&notify, 0, sizeof(notify));
		notify.t = SRV_NOTIFY_ON_LOGIN;
		Com_wcscpy(notify.on_logoff.remote_ip, remote_ip);
		notify.on_logoff.remote_port = port;
		return Srv_OnNotify(&notify);

}

bool_t	Srv_NotifyOnEnter(const wchar_t *remote_ip, uint_16_t port)
{
		srvNotify_t		notify;
		Com_ASSERT(remote_ip != NULL);
		Com_memset(&notify, 0, sizeof(notify));
		notify.t = SRV_NOTIFY_ON_ENTER;
		Com_wcscpy(notify.on_enter.remote_ip, remote_ip);
		notify.on_enter.remote_port = port;
		return Srv_OnNotify(&notify);

}

bool_t	Srv_NotifyOnLeave(const wchar_t *remote_ip, uint_16_t port)
{
		srvNotify_t		notify;
		Com_ASSERT(remote_ip != NULL);
		Com_memset(&notify, 0, sizeof(notify));
		notify.t = SRV_NOTIFY_ON_LEAVE;
		Com_wcscpy(notify.on_leave.remote_ip, remote_ip);
		notify.on_leave.remote_port = port;
		return 	Srv_OnNotify(&notify);

}

bool_t	Srv_NotifyOnClipData(const wchar_t *remote_ip, uint_16_t port)
{
		srvNotify_t		notify;
		Com_ASSERT(remote_ip != NULL);
		Com_memset(&notify, 0, sizeof(notify));
		notify.t = SRV_NOTIFY_ON_CLIPDATA;
		Com_wcscpy(notify.on_recv_clipdata.remote_ip, remote_ip);
		notify.on_recv_clipdata.remote_port = port;
		return Srv_OnNotify(&notify);

}

MM_NAMESPACE_END


