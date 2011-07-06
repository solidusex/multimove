
#include "srv_wndsrv.h"
#include "srv_session.h"


MM_NAMESPACE_BEGIN



srvSession_t*	SS_OnClientSession(SOCKET cli_fd, const struct sockaddr_in *addr)
{
		srvSession_t	*ss;
		Com_ASSERT(cli_fd != INVALID_SOCKET && addr != NULL);
		
		ss = Com_NEW0(srvSession_t);
		ss->fd = cli_fd;

		if(inet_ntoa(addr->sin_addr) != NULL)
		{
				Com_swprintf(ss->ip, 256, L"%S", inet_ntoa(addr->sin_addr));
		}

		
		ss->port = ntohs(addr->sin_port);

		ss->out_buf = Com_CreateBuffer(1024);
		Com_InitMutex(&ss->out_lock);
		ss->last_out_stamp = Com_GetTime_Milliseconds();

		ss->in_buf = Com_CreateBuffer(1024);
		Com_InitMutex(&ss->in_lock);
		ss->last_in_stamp = Com_GetTime_Milliseconds();

		ss->in_state = NM_RECV_WAIT_HEADER;
		ss->remain_len = NM_PACKAGE_HEADER_LENGTH;
		
		ss->pos = NM_POS_MAX;
		Com_socket_nonblocking(ss->fd, true);
		
		ss->is_entered = false;
		ss->is_handshaked = false;


		Com_printf(L"Client Login %s:%d\r\n", ss->ip, ss->port);

		return ss;
}


void			SS_CloseClientSession(srvSession_t *ss)
{

		Com_ASSERT(ss != NULL);

		Com_printf(L"Client Logoff %s:%d\r\n", ss->ip, ss->port);
				
		Com_DestroyBuffer(ss->in_buf);
		ss->in_buf = NULL;
		Com_UnInitMutex(&ss->in_lock);

		Com_DestroyBuffer(ss->out_buf);
		ss->out_buf = NULL;
		Com_UnInitMutex(&ss->out_lock);
		Com_DEL(ss);
		ss = NULL;

}


bool_t			SS_IsEntered(const srvSession_t *ss)
{
		Com_ASSERT(ss != NULL);
		return ss->is_entered;
}


bool_t			SS_SendKeepAlive(srvSession_t *ss)
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

bool_t			SS_SendHandShakeReply(srvSession_t *ss)
{
		nmMsg_t msg;
		Com_ASSERT(ss != NULL);

		Com_memset(&msg, 0, sizeof(msg));
		msg.t = NM_MSG_HANDSHAKE_REPLY;
		

		Com_LockMutex(&ss->out_lock);
		
		NM_MsgToBuffer(&msg, ss->out_buf);

		Com_UnLockMutex(&ss->out_lock);
		return true;
}

bool_t			SS_SendMouseLeave(srvSession_t *ss, const nmMsg_t *msg)
{
		Com_ASSERT(ss != NULL);
		Com_ASSERT(msg->t == NM_MSG_LEAVE);
		Com_LockMutex(&ss->out_lock);

		NM_MsgToBuffer(msg, ss->out_buf);
		
		Com_UnLockMutex(&ss->out_lock);

		ss->is_entered = false;
		return true;
}


bool_t			SS_SendClipDataMsg(srvSession_t *ss, const nmMsg_t *msg)
{
		Com_ASSERT(ss != NULL && msg && msg->t == NM_MSG_CLIPDATA);

		
		Com_LockMutex(&ss->out_lock);
		
		NM_MsgToBuffer(msg, ss->out_buf);

		Com_UnLockMutex(&ss->out_lock);

		return true;
}


bool_t			SS_HasDataToSend(srvSession_t *ss)
{
		bool_t	has_data;
		Com_ASSERT(ss != NULL);
		Com_LockMutex(&ss->out_lock);

		has_data = Com_GetBufferAvailable(ss->out_buf) > 0 ? true : false;
		
		Com_UnLockMutex(&ss->out_lock);
		return has_data;
}


bool_t			SS_OnSendData(srvSession_t *ss)
{
		bool_t	is_ok;
		const byte_t *p;
		size_t available;
		int wn;
		Com_ASSERT(ss != NULL);
		
		Com_LockMutex(&ss->out_lock);

		is_ok = true;

		available = Com_GetBufferAvailable(ss->out_buf);

		if(available <= 0)
		{
				is_ok = false;/*如果被挂进到select中，则一定有数据待发送*/
				goto END_POINT;
		}

		p = Com_GetBufferData(ss->out_buf);

		wn = send(ss->fd, (const char*)p, (int)available, 0);

		if(wn <= 0)
		{
				Com_printf(L"Send to server %s:%d failed\r\n", ss->ip,ss->port);
				is_ok = false;
				goto END_POINT;
		}else
		{
				Com_EraseBuffer(ss->out_buf, (size_t)wn);
		}


END_POINT:
		Com_UnLockMutex(&ss->out_lock);
		ss->last_out_stamp = Com_GetTime_Milliseconds();
		return is_ok;

}


bool_t			SS_OnRecvData(srvSession_t *ss)
{
		int rn;
		u_long available;
		byte_t *buf;
		bool_t is_ok;
		Com_ASSERT(ss != NULL);
		
		if(ioctlsocket(ss->fd, FIONREAD, &available) != 0 || available <= 0)
		{
				Com_error(COM_ERR_WARNING, L"Srv_RecvData : ioctlsocket error\r\n");
				return false;
		}

		if(available <= 0)
		{
				return false;
		}

		Com_LockMutex(&ss->in_lock);

		buf = Com_AllocBuffer(ss->in_buf, (int)available);
		rn = recv(ss->fd, (char*)buf, available, 0);

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

				switch(ss->in_state)
				{
				case NM_RECV_WAIT_HEADER:
				{
						uint_16_t package_len;

						if(ss->remain_len <= Com_GetBufferAvailable(ss->in_buf))
						{
								p = Com_GetBufferData(ss->in_buf);
								Com_memcpy((byte_t*)&package_len, p, sizeof(package_len));
								Com_EraseBuffer(ss->in_buf, sizeof(package_len));
								
								package_len = COM_NTOL_U16(package_len);

								if(package_len > 10 * COM_MB || package_len < 1)/*包过大或过小*/
								{
										Com_error(COM_ERR_WARNING, L"Invalid package size : %d\r\n", package_len);
										is_ok = false;
										goto END_POINT;
								}else
								{
										ss->in_state = NM_RECV_WAIT_PACKAGE;
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
								is_ok = SS_OnPackage(ss, p, ss->remain_len);
								Com_EraseBuffer(ss->in_buf, ss->remain_len);
								
								if(is_ok)
								{
										ss->in_state = NM_RECV_WAIT_HEADER;
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
						is_ok = false;
						break;
				}
		}

END_POINT:
		ss->last_in_stamp = Com_GetTime_Milliseconds();
		Com_UnLockMutex(&ss->in_lock);
		return is_ok;
}



bool_t			SS_OnPackage(srvSession_t		*ss, const byte_t *data, size_t len)
{
		nmMsg_t msg;
		bool_t is_ok;
		Com_ASSERT(ss != NULL && data != NULL && len > 0);
		
		if(!NM_ParseFromBuffer(data, len, &msg))
		{
				Com_error(COM_ERR_WARNING, L"Received bad package from %s:%d\r\n", ss->ip, ss->port);
				return false;
		}
		
		is_ok = true;

		switch(msg.t)
		{
		case NM_MSG_KEEPALIVE:
				Com_printf(L"Session from (%s:%d) received NM_MSG_KEEPALIVE\r\n", ss->ip, ss->port);
				is_ok = true;
				break;
		case NM_MSG_HANDSHAKE:
				if(ss->is_handshaked)
				{
						Com_error(COM_ERR_WARNING, L"Multi handshake is not allowed from (%s:%d)\r\n", ss->ip, ss->port);
						is_ok = false;
				}else
				{
						ss->is_handshaked = true;
						ss->pos = msg.handshake.srv_pos;
						SS_SendHandShakeReply(ss);
						is_ok = true;
				}
				break;
		case NM_MSG_ENTER:
		{
				int src_x_fullscreen, src_y_fullscreen;
				DWORD x_pos, y_pos;

				src_x_fullscreen = GetSystemMetrics(SM_CXSCREEN);
				src_y_fullscreen = GetSystemMetrics(SM_CYSCREEN);


				if(!ss->is_handshaked)
				{
						Com_error(COM_ERR_WARNING, L"Session from (%s:%d) received NM_MSG_ENTER request before handshake, connection terminated\r\n", ss->ip, ss->port);
						is_ok = false;
						goto END_POINT;
				}

				if(ss->is_entered)
				{
						Com_error(COM_ERR_WARNING, L"Session from (%s:%d) received multi NM_MSG_ENTER msg, connection terminated\r\n", ss->ip, ss->port);
						is_ok = false;
						goto END_POINT;
				}
				
				if(msg.enter.src_x_fullscreen == 0 || msg.enter.src_y_fullscreen == 0)
				{
						Com_error(COM_ERR_WARNING, L"Session from (%s:%d) received invalid resolution(%d x %d), connection terminated\r\n", ss->ip, ss->port, msg.enter.src_x_fullscreen, msg.enter.src_y_fullscreen);
						return false;
						goto END_POINT;
				}

				
				switch(ss->pos)
				{
				case NM_POS_LEFT:/*如果我在左面，则指针从右面出来*/
						x_pos = (src_x_fullscreen - 10) * 65535 / src_x_fullscreen;
						y_pos = msg.enter.y * 65535 / msg.enter.src_y_fullscreen;
						break;
				case NM_POS_RIGHT:/*如果我在右面，则指针从左面出来*/
						x_pos = 10 * 65535 / src_x_fullscreen;
						y_pos = msg.enter.y * 65535 / msg.enter.src_y_fullscreen;
						break;
				case NM_POS_UP:
						x_pos = msg.enter.x * 65535 / msg.enter.src_x_fullscreen;
						y_pos = (src_y_fullscreen - 10) * 65535 / src_y_fullscreen;
						break;
				case NM_POS_DOWN:
						x_pos = msg.enter.x * 65535 / msg.enter.src_x_fullscreen;
						y_pos = 10 * 65535 / src_y_fullscreen;
						break;
				default:
						x_pos = 0;
						y_pos = 0;
						Com_ASSERT(false);
						break;
				}
				
				mouse_event(MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE, x_pos, y_pos, 0, 0);
				ss->is_entered = true;
		}
				break;
		case NM_MSG_MOUSE:
		{
				
				if(!ss->is_handshaked)
				{
						Com_error(COM_ERR_WARNING, L"Session from (%s:%d) received NM_MSG_MOUSE request before NM_MSG_ENTER, connection terminated\r\n", ss->ip, ss->port);
						is_ok = false;
						goto END_POINT;
				}

				if(!ss->is_entered)
				{
						Com_error(COM_ERR_WARNING, L"Session from (%s:%d) received NM_MSG_MOUSE after msg NM_MSG_LEAVE, msg discard\r\n", ss->ip, ss->port);
						is_ok = true; 
						goto END_POINT;
				}


				if(msg.mouse.msg == WM_MOUSEMOVE)
				{

						bool_t need_send_leave = false;
						int src_x_fullscreen, src_y_fullscreen;
						POINT pt;
						src_x_fullscreen = GetSystemMetrics(SM_CXSCREEN);
						src_y_fullscreen = GetSystemMetrics(SM_CYSCREEN);
				
						GetCursorPos(&pt);
				
						if(ss->pos == NM_POS_LEFT && pt.x >= src_x_fullscreen - 1 && msg.mouse.x > 0)
						{
								Com_printf(L"Session (%s:%d) send mouse leave msg to client\r\n", ss->ip, ss->port);
								need_send_leave = true;
						}

						if(ss->pos == NM_POS_RIGHT && pt.x <= 0 && msg.mouse.x < 0)
						{
								Com_printf(L"Session (%s:%d) send mouse leave msg to client\r\n", ss->ip, ss->port);
								need_send_leave = true;
						}

						if(ss->pos == NM_POS_UP && pt.y >= src_y_fullscreen - 1 && msg.mouse.y > 0)
						{
								Com_printf(L"Session (%s:%d) send mouse leave msg to client\r\n", ss->ip, ss->port);
								need_send_leave = true;
						}

						if(ss->pos == NM_POS_DOWN && pt.y <= 0 && msg.mouse.y < 0)
						{
								Com_printf(L"Session (%s:%d) send mouse leave msg to client\r\n", ss->ip, ss->port);
								need_send_leave = true;
						}

						

						if(need_send_leave)
						{
								nmMsg_t leave_msg;
								leave_msg.t = NM_MSG_LEAVE;
								leave_msg.leave.src_x_fullscreen = src_x_fullscreen;
								leave_msg.leave.src_y_fullscreen = src_y_fullscreen;
								leave_msg.leave.x = pt.x;
								leave_msg.leave.y = pt.y;
								SS_SendMouseLeave(ss, &leave_msg);
						}
				}
				

				switch(msg.mouse.msg)
				{
				case WM_MOUSEMOVE:
				{
						mouse_event(MOUSEEVENTF_MOVE, (DWORD)msg.mouse.x, (DWORD)msg.mouse.y, 0, 0);
				}
						break;
				case WM_LBUTTONDOWN:
				{
						mouse_event(MOUSEEVENTF_LEFTDOWN, 0,0, 0, 0);
				}
						break;
				case WM_LBUTTONUP:
				{
						mouse_event(MOUSEEVENTF_LEFTUP, 0,0, 0, 0);
				}
						break;
				case WM_RBUTTONDOWN:
				{
						mouse_event(MOUSEEVENTF_RIGHTDOWN, 0,0, 0, 0);
				}
						break;
				case WM_RBUTTONUP:
				{
						mouse_event(MOUSEEVENTF_RIGHTUP, 0,0, 0, 0);
				}
						break;
				case WM_MOUSEWHEEL:
				{
						mouse_event(MOUSEEVENTF_WHEEL, 0,0, (DWORD)msg.mouse.data, 0);
				}
						break;
				case WM_MOUSEHWHEEL:
				{
						mouse_event(MOUSEEVENTF_HWHEEL, 0,0, (DWORD)msg.mouse.data, 0);
				}
						break;
				default:
						Com_printf(L"Msg (%d) has not been supported by this version\r\n", msg.mouse.msg);
						is_ok = true;
						break;
				}

				is_ok = true;
		}
				break;
		case NM_MSG_KEYBOARD:
		{
				if(!ss->is_handshaked)
				{
						Com_error(COM_ERR_WARNING, L"Session from (%s:%d) received NM_MSG_MOUSE request before handshake, connection terminated\r\n", ss->ip, ss->port);
						is_ok = false;
						goto END_POINT;
				}

				if(!ss->is_entered)
				{
						Com_error(COM_ERR_WARNING, L"Session from (%s:%d) received NM_MSG_KEYBOARD after msg NM_MSG_LEAVE, msg discard\r\n", ss->ip, ss->port);
						is_ok = true; 
						goto END_POINT;
				}
						
				keybd_event(msg.keyboard.vk, msg.keyboard.scan, msg.keyboard.is_keydown ? 0 : KEYEVENTF_KEYUP, 0);

				return true;
		}
				break;
		case NM_MSG_CLIPDATA:
				if(!ss->is_handshaked)
				{
						Com_error(COM_ERR_WARNING, L"Session from (%s:%d) received NM_MSG_CLIPDATA request before handshake, package discard\r\n", ss->ip, ss->port);
						is_ok = true;
						goto END_POINT;
				}
				
				WND_Srv_SetClipboardData(&msg);
				break;
		case NM_MSG_HANDSHAKE_REPLY:
		case NM_MSG_LEAVE:
		default:
				Com_error(COM_ERR_WARNING, L"Server received a bad msg type %d from (%s:%d), connection terminated\r\n", msg.t, ss->ip, ss->port);
				return false;
		}
END_POINT:
		return is_ok;
}



bool_t			SS_OnTimer(srvSession_t *ss)
{
		Com_ASSERT(ss != NULL);

		if(Com_GetTime_Milliseconds() - ss->last_in_stamp > NM_KEEPALIVE_TIMEOUT)
		{
				Com_printf(L"Session from (%s:%d) timeout\r\n", ss->ip, ss->port);
				return false;
		}
		

		if(Com_GetTime_Milliseconds() - ss->last_out_stamp > NM_KEEPALIVE_TIMEOUT - 1000)
		{
				Com_printf(L"Send KeepAlive to client (%s:%d)\r\n", ss->ip, ss->port);
				SS_SendKeepAlive(ss);
		}
		
		return true;

}


MM_NAMESPACE_END



