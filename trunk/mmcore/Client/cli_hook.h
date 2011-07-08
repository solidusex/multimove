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