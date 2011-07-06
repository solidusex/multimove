#ifndef __MMCORE_CLIENT_H__
#define __MMCORE_CLIENT_H__

#include "../Common/common.h"
#include "../NetMsg/netmsg.h"
#pragma comment(lib, "Ws2_32.lib")

MM_NAMESPACE_BEGIN



typedef enum
{
		CLI_NOTIFY_ON_CONNECTED,
		CLI_NOTIFY_ON_DISCONNECTED,
		CLI_NOTIFY_ON_ACTIVE
}cliNotifyType_t;


typedef struct __client_notify_tag
{
		cliNotifyType_t	t;

		nmPosition_t	action_pos;
		
		const wchar_t	*ip;
		uint_16_t		port;

}cliNotify_t;

typedef void (*cliNotifyFunc_t)(void *ctx, const cliNotify_t	*notify);


typedef struct __client_init_tag
{
		void			*ctx;
		cliNotifyFunc_t	on_notify;
		HCURSOR			hide_cursor;
}cliInit_t;


bool_t Cli_Init(const cliInit_t *init);
bool_t Cli_UnInit();





bool_t	Cli_Start();
bool_t	Cli_Stop();



bool_t	Cli_InsertServer(nmPosition_t pos, const wchar_t *srv_ip, uint_16_t port);
bool_t	Cli_RemoveServer(nmPosition_t pos);


/****************************************Internal***************************/

HCURSOR	Cli_GetHideCursor();



MM_NAMESPACE_END


#endif
