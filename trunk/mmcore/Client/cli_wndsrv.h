#ifndef __MMCORE_WINDOW_SERVICE_CLIENT_H__
#define __MMCORE_WINDOW_SERVICE_CLIENT_H__

#include "../Common/common.h"
#include "../NetMsg/netmsg.h"

MM_NAMESPACE_BEGIN


typedef bool_t	(*wndMsgHander_t)(const nmMsg_t *msg, void *ctx);



bool_t	WND_Cli_Start();
bool_t	WND_Cli_Stop();
bool_t	WND_Cli_IsStarted();

bool_t	WND_Cli_RegisterHandler(nmPosition_t	pos, void	*ctx, wndMsgHander_t on_msg);
bool_t	WND_Cli_UnRegisterHandler(nmPosition_t	pos);

/*******************************************************/
bool_t	WND_Cli_SetClipboardData(const nmMsg_t *msg);








MM_NAMESPACE_END


#endif