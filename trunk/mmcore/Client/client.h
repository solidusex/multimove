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
		CLI_NOTIFY_ON_ACTIVE,
		CLI_NOTIFY_ON_DEACTIVE,
		CLI_NOTIFY_ON_CLIPDATA
}cliNotifyType_t;


#define CLI_IP_LENGTH							32

typedef struct __client_notify_tag
{
		cliNotifyType_t	t;

		union{
				struct {
						nmPosition_t	action_pos;
						wchar_t			ip[CLI_IP_LENGTH];
						uint_16_t		port;
				}on_connected;

				struct {
						nmPosition_t	action_pos;
						wchar_t			ip[CLI_IP_LENGTH];
						uint_16_t		port;
				}on_disconnected;

				struct {
						nmPosition_t	action_pos;
						wchar_t			ip[CLI_IP_LENGTH];
						uint_16_t		port;
				}on_active;


				struct {
						nmPosition_t	action_pos;
						wchar_t			ip[CLI_IP_LENGTH];
						uint_16_t		port;
				}on_deactive;

				struct {
						nmPosition_t	action_pos;
						wchar_t			ip[CLI_IP_LENGTH];
						uint_16_t		port;
				}on_clipdata;
		};
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
bool_t	Cli_OnNotify(const cliNotify_t *notfiy);


MM_NAMESPACE_END


#endif
