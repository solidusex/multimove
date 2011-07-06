#ifndef __MMCORE_WINDOW_SERVICE_SERVER_H__
#define __MMCORE_WINDOW_SERVICE_SERVER_H__

#include "../Common/common.h"
#include "../NetMsg/netmsg.h"

MM_NAMESPACE_BEGIN


typedef bool_t	(*wndMsgHander_t)(const nmMsg_t *msg);



bool_t	WND_Srv_Start(wndMsgHander_t handler);
bool_t	WND_Srv_Stop();
bool_t	WND_Srv_IsStarted();

bool_t	WND_Srv_SetClipboardData(const nmMsg_t *msg);

MM_NAMESPACE_END


#endif