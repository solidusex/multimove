#ifndef __MMCORE_SERVER_H__
#define __MMCORE_SERVER_H__

#include "../Common/common.h"
#include "../NetMsg/netmsg.h"
#pragma comment(lib, "Ws2_32.lib")

MM_NAMESPACE_BEGIN




typedef enum
{
		SRV_NOTIFY_ON_STARTUP,
		SRV_NOTIFY_ON_LOGIN,
		SRV_NOTIFY_ON_LOGOFF,
		SRV_NOTIFY_ON_ENTER,
		SRV_NOTIFY_ON_LEAVE,
		SRV_NOTIFY_ON_CLIPBOARD_CHANGED,
}srvNotifyType_t;


typedef struct __server_notify_tag
{
		srvNotifyType_t	t;
		
		const wchar_t	*ip;
		uint_16_t		port;
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









MM_NAMESPACE_END


#endif
