#ifndef __MMCORE_SERVER_H__
#define __MMCORE_SERVER_H__

#include "../Common/common.h"
#include "../NetMsg/netmsg.h"
#pragma comment(lib, "Ws2_32.lib")

MM_NAMESPACE_BEGIN


struct __server_init_tag;
typedef struct __server_init_tag	srvInit_t;

bool_t Srv_Init(const srvInit_t *init);
bool_t Srv_UnInit();


bool_t	Srv_Start(const wchar_t *bind_ip, uint_16_t port);
bool_t	Srv_Stop();
bool_t	Srv_IsStarted();









MM_NAMESPACE_END


#endif
