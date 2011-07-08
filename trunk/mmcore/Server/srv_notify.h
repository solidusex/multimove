#ifndef __MMCORE_NOTIFY_SERVER_H__
#define __MMCORE_NOTIFY_SERVER_H__

#include "server.h"

MM_NAMESPACE_BEGIN



/**********************************************************************************************************************/


bool_t	Srv_NotifyOnListen(const wchar_t *bind_ip, uint_16_t port);
bool_t	Srv_NotifyOnLogin(const wchar_t *remote_ip, uint_16_t port);
bool_t	Srv_NotifyOnLogoff(const wchar_t *remote_ip, uint_16_t port);

bool_t	Srv_NotifyOnEnter(const wchar_t *remote_ip, uint_16_t port);
bool_t	Srv_NotifyOnLeave(const wchar_t *remote_ip, uint_16_t port);
bool_t	Srv_NotifyOnClipData(const wchar_t *remote_ip, uint_16_t port);


MM_NAMESPACE_END


#endif