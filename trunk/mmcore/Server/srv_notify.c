
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
				IP_ADAPTER_INFO *ptr;
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
				while(ptr)
				{
						Com_swprintf(notify.on_listen.bind_ip[i].ip, SRV_IP_LENGTH, L"%S", ptr->IpAddressList.IpAddress.String);
						notify.on_listen.bind_ip_cnt++;
						i++;
						ptr = ptr->Next;
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


