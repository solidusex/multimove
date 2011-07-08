#include "cli_notify.h"


MM_NAMESPACE_BEGIN


bool_t	Cli_OnNotifyConnected(nmPosition_t	action_pos, const wchar_t *ip, uint_16_t port)
{
		cliNotify_t		notify;
		Com_ASSERT(ip != NULL);
		notify.t = CLI_NOTIFY_ON_CONNECTED;
		notify.on_connected.action_pos = action_pos;
		Com_wcscpy(notify.on_connected.ip, ip);
		notify.on_connected.port = port;
		
		return Cli_OnNotify(&notify);
}


bool_t	Cli_OnNotifyDisConnected(nmPosition_t	action_pos, const wchar_t *ip, uint_16_t port)
{
		cliNotify_t		notify;
		Com_ASSERT(ip != NULL);
		notify.t = CLI_NOTIFY_ON_DISCONNECTED;
		notify.on_disconnected.action_pos = action_pos;
		Com_wcscpy(notify.on_disconnected.ip, ip);
		notify.on_disconnected.port = port;
		
		return Cli_OnNotify(&notify);
}


bool_t	Cli_OnNotifyActive(nmPosition_t	action_pos, const wchar_t *ip, uint_16_t port)
{
		cliNotify_t		notify;
		Com_ASSERT(ip != NULL);
		notify.t = CLI_NOTIFY_ON_ACTIVE;
		notify.on_active.action_pos = action_pos;
		Com_wcscpy(notify.on_active.ip, ip);
		notify.on_active.port = port;
		
		return Cli_OnNotify(&notify);
}


bool_t	Cli_OnNotifyDeActive(nmPosition_t	action_pos, const wchar_t *ip, uint_16_t port)
{
		cliNotify_t		notify;
		Com_ASSERT(ip != NULL);
		notify.t = CLI_NOTIFY_ON_DEACTIVE;
		notify.on_deactive.action_pos = action_pos;
		Com_wcscpy(notify.on_deactive.ip, ip);
		notify.on_deactive.port = port;
		
		return Cli_OnNotify(&notify);
}


bool_t	Cli_OnNotifyClipData(nmPosition_t	action_pos, const wchar_t *ip, uint_16_t port)
{
		cliNotify_t		notify;
		Com_ASSERT(ip != NULL);
		notify.t = CLI_NOTIFY_ON_CLIPDATA;
		notify.on_clipdata.action_pos = action_pos;
		Com_wcscpy(notify.on_clipdata.ip, ip);
		notify.on_clipdata.port = port;
		
		return Cli_OnNotify(&notify);
}




MM_NAMESPACE_END
