#ifndef __MMCORE_HOOK_CLIENT_H__
#define __MMCORE_HOOK_CLIENT_H__

#include "../Common/common.h"
#include "../NetMsg/netmsg.h"

MM_NAMESPACE_BEGIN


typedef bool_t	(*hkMsgHander_t)(const nmMsg_t *msg, void *ctx);


bool_t	Hook_Cli_Start();
bool_t	Hook_Cli_Stop();
bool_t	Hook_Cli_IsStarted();

bool_t	Hook_Cli_RegisterHandler(nmPosition_t	pos, void	*ctx, hkMsgHander_t	on_msg);
bool_t	Hook_Cli_UnRegisterHandler(nmPosition_t	pos);

bool_t	Hook_Cli_ControlReturn();










MM_NAMESPACE_END


#endif