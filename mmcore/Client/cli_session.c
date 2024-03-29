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

#define OEMRESOURCE 1

#include "cli_hook.h"
#include "cli_wndsrv.h"
#include "cli_session.h"
#include "client.h"
#include "cli_notify.h"

MM_NAMESPACE_BEGIN


static const UINT __g_cursor_id[] = 
{
		OCR_APPSTARTING,
		OCR_NORMAL,
		OCR_CROSS,
		OCR_HAND,
		OCR_IBEAM,
		OCR_NO,
		OCR_SIZEALL,
		OCR_SIZENESW,
		OCR_SIZENS,
		OCR_SIZENWSE,
		OCR_SIZEWE,
		OCR_UP,
		OCR_WAIT,
};


#if(0)
static BOOL __hide_cursor()
{
		

		wchar_t buf[MAX_PATH * 2];
		wchar_t d[5], dir[MAX_PATH];
		HCURSOR new_cursor;

		if(!GetModuleFileNameW(GetModuleHandle(NULL), buf, MAX_PATH * 2))
		{
				return FALSE;
		}

		
		_wsplitpath(buf, d, dir, NULL, NULL);

		_snwprintf(buf, MAX_PATH * 2, L"%s%s%s", d, dir, L"blank_cursor.cur");

		new_cursor = LoadCursorFromFile(buf);

		if(new_cursor == NULL)
		{
				Com_error(COM_ERR_WARNING, L"Can not load blank cursor\r\n");
				return FALSE;
		}

		for(int i = 0; i < sizeof(__g_cursor_id)/sizeof(__g_cursor_id[0]); ++i)
		{
				SetSystemCursor(CopyCursor(new_cursor), __g_cursor_id[i]);
		}
		
		return TRUE;

}
#endif

static BOOL __hide_cursor()
{
		

		HCURSOR new_cursor = Cli_GetHideCursor();

		if(new_cursor == NULL)
		{
				Com_error(COM_ERR_WARNING, L"Can not hide cursor\r\n");
				return FALSE;
		}

		for(int i = 0; i < sizeof(__g_cursor_id)/sizeof(__g_cursor_id[0]); ++i)
		{
				SetSystemCursor(CopyCursor(new_cursor), __g_cursor_id[i]);
		}
		
		return TRUE;

}


static BOOL __show_cursor()
{
		return SystemParametersInfo(SPI_SETCURSORS,0,0,WM_SETTINGCHANGE | SPIF_UPDATEINIFILE );
}

/*****************************************************************************************************************/

cliSession_t*		SS_ConnectSession(nmPosition_t	pos, const wchar_t			*ip, uint_16_t		port)
{
		cliSession_t *ss = NULL;
		SOCKET fd = INVALID_SOCKET;
		struct sockaddr_in		addr;
		const uint_64_t timeout = SS_CONNECT_TO_SRV_TIMEOUT;
		BOOL no_delay = TRUE;
		Com_ASSERT(ip != NULL);

		if(!Com_GetIPByHostName_V4(ip, &addr))
		{
				return NULL;
		}

		addr.sin_port = htons((u_short)port);

		fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if(fd == INVALID_SOCKET)
		{
				Com_error(COM_ERR_WARNING,L"Create socket handle failed on connect to %s:%d\r\n", ip, port);
				return NULL;
		}
		
		if(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&no_delay, sizeof(BOOL)) != 0)
		{
				Com_error(COM_ERR_WARNING,L"Call setsockopt failed on connect to %s:%d\r\n", ip, port);
				closesocket(fd);
				return NULL;
		}

		if(Com_connect_timeout(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in), &timeout) != 0)
		{
				if(WSAGetLastError() == WSAETIMEDOUT)
				{
						Com_printf(L"Connect to %s:%d timeout \r\n", ip, port);
				}else
				{
						Com_printf(L"Connect to %s:%d failed : error code (%d) \r\n", ip, port, WSAGetLastError());
				}
				closesocket(fd);
				return NULL;
		}
		
		if(Com_socket_nonblocking(fd, true) != 0)
		{
				Com_printf(L"Connect to %s:%d failed : error code (%d) \r\n", ip, port, WSAGetLastError());
				closesocket(fd);
				return NULL;
		}

		ss = Com_NEW0(cliSession_t);
		ss->ip = Com_wcsdup(ip);
		ss->port = port;
		ss->for_position = pos;
		ss->sockfd = fd;
		ss->in_buf = Com_CreateBuffer(1024);
		Com_InitMutex(&ss->in_lock);
		ss->recv_state = NM_RECV_WAIT_HEADER;
		ss->remain_len = NM_PACKAGE_HEADER_LENGTH;
		ss->last_recv_stamp = Com_GetTime_Milliseconds();
		

		ss->out_buf = Com_CreateBuffer(1024);
		Com_InitMutex(&ss->out_lock);
		ss->last_send_stamp = Com_GetTime_Milliseconds();
		
		SS_SendHandShake(ss);
		ss->is_active = false;
		ss->is_handshaked = false;

		Cli_OnNotifyConnected(pos, ss->ip, ss->port);
		return ss;
}




void			SS_CloseSession(cliSession_t *ss)
{
		Com_ASSERT(ss != NULL);
		
		Cli_OnNotifyDisConnected(ss->for_position, ss->ip, ss->port);

		if(SS_IsActive(ss))
		{
				__show_cursor();
		}

		Com_DEL(ss->ip);
		ss->ip = NULL;
		closesocket(ss->sockfd);
		ss->sockfd = INVALID_SOCKET;
		Com_DestroyBuffer(ss->in_buf);
		ss->in_buf = NULL;
		Com_UnInitMutex(&ss->in_lock);

		Com_DestroyBuffer(ss->out_buf);
		ss->out_buf = NULL;
		Com_UnInitMutex(&ss->out_lock);
		Com_DEL(ss);
		ss = NULL;

		
}



bool_t		SS_IsActive(const cliSession_t *ss)
{
		Com_ASSERT(ss != NULL);
		return ss->is_active;

}

bool_t		SS_IsHandshaked(const cliSession_t *ss)
{
		Com_ASSERT(ss != NULL);
		return ss->is_handshaked;
}

bool_t		SS_HasDataToSend(cliSession_t *ss)		/*out_buf是否存在数据*/
{
		bool_t has_data_to_send;
		Com_ASSERT(ss != NULL);

		Com_LockMutex(&ss->out_lock);
		has_data_to_send = Com_GetBufferAvailable(ss->out_buf) > 0 ? true : false;
		Com_UnLockMutex(&ss->out_lock);
		return has_data_to_send;
}


bool_t		SS_SendData(cliSession_t *ss)		/*将out_buf数据用非阻塞方式发送出去*/
{
		bool_t	is_ok;
		const byte_t *p;
		size_t available;
		int wn;
		Com_ASSERT(ss != NULL);

		Com_LockMutex(&ss->in_lock);

		is_ok = true;

		available = Com_MIN(Com_GetBufferAvailable(ss->out_buf), 4 * COM_KB);

		if(available <= 0)
		{
				is_ok = false;/*如果被挂进到select中，则一定有数据待发送*/
				goto END_POINT;
		}

		p = Com_GetBufferData(ss->out_buf);

		wn = send(ss->sockfd, (const char*)p, (int)available, 0);

		if(wn <= 0)
		{
				Com_printf(L"Send data to %s:%d failed\r\n", ss->ip, ss->port);
				is_ok = false;
				goto END_POINT;
		}else
		{
				Com_EraseBuffer(ss->out_buf, wn);
		}


END_POINT:
		ss->last_send_stamp = Com_GetTime_Milliseconds();
		Com_UnLockMutex(&ss->in_lock);
		return is_ok;

}




bool_t		SS_RecvData(cliSession_t *ss)		/*从sockfd以非阻塞方式接收对端数据，放入in_buf数据,并进行后续处理*/
{
		int rn;
		u_long available;
		byte_t *buf;
		bool_t is_ok;
		
		Com_ASSERT(ss != NULL);

		if(ioctlsocket(ss->sockfd, FIONREAD, &available) != 0/* || available <= 0*/)
		{
				//Com_error(COM_ERR_WARNING, L"SS_RecvData : ioctlsocket error : %d\r\n", WSAGetLastError());
				return false;
		}

		if(available <= 0)
		{
				return false;
		}

		Com_LockMutex(&ss->in_lock);
		

		buf = Com_AllocBuffer(ss->in_buf, (int)available);
		rn = recv(ss->sockfd, (char*)buf, available, 0);

		

		if(rn != (int)available)
		{
				is_ok = false;
		}else
		{
				is_ok = true;
		}

		if(is_ok)
		{
				const byte_t *p;
RECHECK_POINT:
				Com_ASSERT(ss->remain_len > 0);

				switch(ss->recv_state)
				{
				case NM_RECV_WAIT_HEADER:
				{
						uint_32_t package_len;

						if(ss->remain_len <= Com_GetBufferAvailable(ss->in_buf))
						{
								p = Com_GetBufferData(ss->in_buf);
								Com_memcpy((byte_t*)&package_len, p, sizeof(package_len));
								Com_EraseBuffer(ss->in_buf, sizeof(package_len));
								package_len = COM_NTOL_U32(package_len);

								if(package_len > 10 * COM_MB || package_len < 1)/*包过大或过小*/
								{
										is_ok = false;
										goto END_POINT;
								}else
								{
										ss->recv_state = NM_RECV_WAIT_PACKAGE;
										ss->remain_len = package_len;
										goto RECHECK_POINT;
								}
						}
				}
						break;
				case NM_RECV_WAIT_PACKAGE:
				{
						if(ss->remain_len <= Com_GetBufferAvailable(ss->in_buf))
						{
								p = Com_GetBufferData(ss->in_buf);
								is_ok = SS_HandleRecvBuffer(ss, p, ss->remain_len);
								Com_EraseBuffer(ss->in_buf, ss->remain_len);
								
								if(is_ok)
								{
										ss->recv_state = NM_RECV_WAIT_HEADER;
										ss->remain_len = NM_PACKAGE_HEADER_LENGTH;
										goto RECHECK_POINT;
								}else
								{
										goto END_POINT;
								}
						}
				}
						break;
				default:
						Com_ASSERT(false);/*不可达*/
						break;
				}
		}

END_POINT:
		ss->last_recv_stamp = Com_GetTime_Milliseconds();
		Com_UnLockMutex(&ss->in_lock);
		return is_ok;
}



bool_t		SS_OnTimer(cliSession_t *ss) /*返回真为需要继续处理，返回false则从__g_srv_set删除掉*/
{
		Com_ASSERT(ss != NULL);

		if(Com_GetTime_Milliseconds() - ss->last_recv_stamp > NM_KEEPALIVE_TIMEOUT)
		{
				Com_printf(L"Session (%s:%d) timeout\r\n", ss->ip, ss->port);
				return false;
		}
		

		if(Com_GetTime_Milliseconds() - ss->last_send_stamp > (NM_KEEPALIVE_TIMEOUT - 2000))
		{
				Com_printf(L"Session (%s:%d) Send KeepAlive\r\n", ss->ip, ss->port);
				SS_SendKeepAlive(ss);
		}
		
		return true;
}

bool_t		SS_SendKeepAlive(cliSession_t *ss)
{
		nmMsg_t msg;
		Com_ASSERT(ss != NULL);

		Com_memset(&msg, 0, sizeof(msg));
		msg.t = NM_MSG_KEEPALIVE;

		Com_LockMutex(&ss->out_lock);
		
		NM_MsgToBuffer(&msg, ss->out_buf);

		Com_UnLockMutex(&ss->out_lock);
		return true;
}


bool_t		SS_SendHandShake(cliSession_t *ss)
{
		nmMsg_t msg;
		Com_ASSERT(ss != NULL);

		Com_memset(&msg, 0, sizeof(msg));
		msg.t = NM_MSG_HANDSHAKE;
		msg.handshake.srv_pos = ss->for_position;

		Com_LockMutex(&ss->out_lock);
		
		NM_MsgToBuffer(&msg, ss->out_buf);

		Com_UnLockMutex(&ss->out_lock);
		return true;
}


bool_t		SS_SendEnterMsg(cliSession_t *ss, const nmMsg_t *msg)
{
		Com_ASSERT(ss != NULL && msg && msg->t == NM_MSG_ENTER);
		Com_LockMutex(&ss->out_lock);
		ss->is_active = true;
		NM_MsgToBuffer(msg, ss->out_buf);

		Com_UnLockMutex(&ss->out_lock);

		__hide_cursor();
		Cli_OnNotifyActive(ss->for_position, ss->ip, ss->port);

		return true;
}

bool_t		SS_SendMouseMsg(cliSession_t *ss, const nmMsg_t *msg)
{
		Com_ASSERT(ss != NULL && msg && msg->t == NM_MSG_MOUSE);
		Com_LockMutex(&ss->out_lock);
		
		NM_MsgToBuffer(msg, ss->out_buf);

		Com_UnLockMutex(&ss->out_lock);
		return true;
}


bool_t		SS_SendKeyboardMsg(cliSession_t *ss, const nmMsg_t *msg)
{

		Com_ASSERT(ss != NULL && msg && msg->t == NM_MSG_KEYBOARD);
		Com_LockMutex(&ss->out_lock);
		
		NM_MsgToBuffer(msg, ss->out_buf);

		Com_UnLockMutex(&ss->out_lock);

		return true;
}


bool_t			SS_SendClipDataMsg(cliSession_t *ss, const nmMsg_t *msg)
{
		Com_ASSERT(ss != NULL && msg && msg->t == NM_MSG_CLIPDATA);

		
		Com_LockMutex(&ss->out_lock);
		
		NM_MsgToBuffer(msg, ss->out_buf);

		Com_UnLockMutex(&ss->out_lock);

		return true;

}




bool_t		SS_HandleRecvBuffer(cliSession_t *ss, const byte_t *data, size_t length)
{
		bool_t is_ok;
		nmMsg_t msg;
		
		Com_ASSERT(ss != NULL && data != NULL);
		
		if(!NM_ParseFromBuffer(data, length, &msg))
		{
				Com_error(COM_ERR_WARNING, L"Session (%s:%d) received a bad net package, connection terminated\r\n", ss->ip, ss->port);
				return false;
		}

		is_ok = true;

		switch(msg.t)
		{
		case NM_MSG_KEEPALIVE:
				Com_printf(L"Session (%s:%d) received NM_MSG_KEEPALIVE\r\n", ss->ip, ss->port);
				is_ok = true;
				break;
		case NM_MSG_HANDSHAKE_REPLY:
				ss->is_handshaked = true;
				is_ok = true;
				break;
		case NM_MSG_LEAVE:
		{
				__show_cursor();
				int x_full_screen, y_full_screen;
				DWORD mouse_pos_x, mouse_pos_y;
				x_full_screen = GetSystemMetrics(SM_CXSCREEN);
				y_full_screen = GetSystemMetrics(SM_CYSCREEN);

				if(!ss->is_handshaked)
				{
						Com_error(COM_ERR_WARNING, L"Session (%s:%d) received NM_MSG_LEAVE request before handshake, connection terminated\r\n", ss->ip, ss->port);
						is_ok = false;
						goto END_POINT;
				}

				if(!ss->is_active)
				{
						Com_error(COM_ERR_WARNING, L"Session (%s:%d) received NM_MSG_LEAVE request in non-active mode, connection terminated\r\n", ss->ip, ss->port);
						goto END_POINT;
				}
				

				switch(ss->for_position)
				{
				case NM_POS_LEFT: /*server在左侧，则从左侧回来，将鼠标设置到左侧*/
						mouse_pos_x = 2 * 65535 / x_full_screen;
						mouse_pos_y = msg.leave.y * 65535 / msg.leave.src_y_fullscreen;
						break;
				case NM_POS_RIGHT:
						mouse_pos_x = (x_full_screen - 2) * 65535 / x_full_screen;
						mouse_pos_y = msg.leave.y * 65535 / msg.leave.src_y_fullscreen;
						break;
				case NM_POS_UP:/*server在上面，则从上面回来，将鼠标设置到上面*/
						mouse_pos_x = msg.leave.x * 65535 / msg.leave.src_x_fullscreen;
						mouse_pos_y = 2 * 65535 / y_full_screen;
						break;
				case NM_POS_DOWN:
						mouse_pos_x = msg.leave.x * 65535 / msg.leave.src_x_fullscreen;
						mouse_pos_y = (y_full_screen - 2) * 65535 / y_full_screen;
						break;
				default:
						Com_error(COM_ERR_WARNING, L"Session (%s:%d) received invalid NM_MSG_LEAVE, connection terminated\r\n", ss->ip, ss->port);
						is_ok = false;
						goto END_POINT;
						break;

				}
				mouse_event(MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE, mouse_pos_x, mouse_pos_y, 0, 0);
				Hook_Cli_ControlReturn();
				Cli_OnNotifyDeActive(ss->for_position, ss->ip, ss->port);

		}
				break;
		case NM_MSG_CLIPDATA:
		{
				Com_printf(L"Session (%s:%d) received NM_MSG_CLIPDATA\r\n", ss->ip, ss->port);
				
				if(!ss->is_handshaked)
				{
						Com_error(COM_ERR_WARNING, L"Session (%s:%d) received NM_MSG_LEAVE request before handshake, package discard\r\n", ss->ip, ss->port);
						is_ok = true;
						goto END_POINT;
				}
				
				WND_Cli_SetClipboardData(ss->for_position, &msg);
				Cli_OnNotifyClipData(ss->for_position, ss->ip, ss->port);

		}
				break;
		case NM_MSG_MOUSE:
		case NM_MSG_KEYBOARD:
		case NM_MSG_HANDSHAKE:
		case NM_MSG_ENTER:
		default:
				Com_error(COM_ERR_WARNING, L"Session (%s:%d) receive a bad msg type, connection terminated\r\n", ss->ip, ss->port);
				is_ok = false;
				goto END_POINT;
		}

END_POINT:
		return is_ok;
}


MM_NAMESPACE_END

