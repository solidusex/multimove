#ifndef __MMCORE_NOTIFY_CLIENT_H__
#define __MMCORE_NOTIFY_CLIENT_H__

#include "client.h"


MM_NAMESPACE_BEGIN

bool_t	Cli_OnNotifyConnected(nmPosition_t	action_pos, const wchar_t *ip, uint_16_t port);
bool_t	Cli_OnNotifyDisConnected(nmPosition_t	action_pos, const wchar_t *ip, uint_16_t port);
bool_t	Cli_OnNotifyActive(nmPosition_t	action_pos, const wchar_t *ip, uint_16_t port);
bool_t	Cli_OnNotifyDeActive(nmPosition_t	action_pos, const wchar_t *ip, uint_16_t port);
bool_t	Cli_OnNotifyClipData(nmPosition_t	action_pos, const wchar_t *ip, uint_16_t port);



MM_NAMESPACE_END


#endif