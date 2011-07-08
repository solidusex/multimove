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
