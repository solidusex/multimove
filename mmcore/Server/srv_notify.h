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