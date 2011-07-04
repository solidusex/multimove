
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
		return ss;
}


void			SS_CloseClientSession(srvSession_t *ss)
{

		Com_ASSERT(ss != NULL);
				
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
		Com_ASSERT(ss != NULL);
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
				is_ok = false;/*������ҽ���select�У���һ�������ݴ�����*/
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

								if(package_len > 1 * COM_KB || package_len < 1)/*��������С*/
								{
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
						Com_ASSERT(false);/*���ɴ�*/
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
						return true;
				}
				break;
		case NM_MSG_ENTER:
		{
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
				
				mouse_event(MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE, 1, (DWORD)(msg.enter.y * 65535 / msg.enter.src_y_fullscreen), 0, 0);
				ss->is_entered = true;
		}
				break;
		case NM_MSG_MOUSE:
		{

				if(!ss->is_handshaked)
				{
						Com_error(COM_ERR_WARNING, L"Session from (%s:%d) received NM_MSG_MOUSE request before handshake, connection terminated\r\n", ss->ip, ss->port);
						is_ok = false;
						goto END_POINT;
				}

				if(ss->is_entered)
				{
						Com_error(COM_ERR_WARNING, L"Session from (%s:%d) received multi NM_MSG_MOUSE msg, connection terminated\r\n", ss->ip, ss->port);
						is_ok = false;
						goto END_POINT;
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
						mouse_event(MOUSEEVENTF_WHEEL, 0,0, (DWORD)data, 0);
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

				if(ss->is_entered)
				{
						Com_error(COM_ERR_WARNING, L"Session from (%s:%d) received multi NM_MSG_MOUSE msg, connection terminated\r\n", ss->ip, ss->port);
						is_ok = false;
						goto END_POINT;
				}

						
				keybd_event(msg.keyboard.vk, msg.keyboard.scan, msg.keyboard.is_keydown ? 0 : KEYEVENTF_KEYUP, 0);

				return true;
		}
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

#if(0)



static bool_t		OnTimer(srvClient_t *cli)
{
		Com_ASSERT(cli != NULL);

		if(Com_GetTime_Milliseconds() - cli->last_in_stamp > KEEPALIVE_TIMEOUT)
		{
				return false;
		}
		

		if(Com_GetTime_Milliseconds() - cli->last_out_stamp > KEEPALIVE_TIMEOUT - 500)
		{
				SendKeepAlive(cli);
		}

		return true;
}


/*
		KeepAlive = 0, HandShake = 1, 
		MouseEnter = 2, MouseEvent = 3, 
		KeyboardEvent = 4
*/


static bool_t		HandleRecvData(srvClient_t *cli, const byte_t *data, size_t length)
{
		uint_16_t package_type;
		const byte_t *p;
		Com_ASSERT(cli != NULL && data != NULL && length >= 2);

		if(length < sizeof(package_type))
		{
				return false;
		}

		p = data;

		Com_memcpy((byte_t*)&package_type, p, sizeof(package_type));
		p += sizeof(package_type);
		length -= sizeof(package_type);

		package_type = COM_NTOL_16(package_type);
		
		
		switch(package_type)
		{
		case 0: /*keepalive*/
				return true;
		case 1: /*handshake ���� LEFT == 0, RIGHT == 1 */
		{
				if(length < 1)
				{
						return false;
				}

				if(cli->is_handshake)
				{
						return false;
				}
				
				switch(*p)
				{
				case 0:
						cli->pos = SRV_LEFT_SRV;
						cli->is_handshake = true;
						SendHandShake(cli);
						return true;
						break;
				case 1:
						cli->pos = SRV_RIGHT_SRV;
						cli->is_handshake = true;
						SendHandShake(cli);
						return true;
				default:
						return false;
				}

				
		}
				break;
		case 2:/*MouseEnter*/
		{
				uint_32_t src_x_fullscreen;
				uint_32_t src_y_fullscreen;
				int_32_t  x;
				int_32_t  y;
				
				if(!cli->is_handshake)
				{
						return false;
				}
				
				Com_memcpy(&src_x_fullscreen, p, sizeof(src_x_fullscreen));
				p += sizeof(src_x_fullscreen);
				length -= sizeof(src_x_fullscreen);
				src_x_fullscreen = COM_NTOL_U32(src_x_fullscreen);

				Com_memcpy(&src_y_fullscreen, p, sizeof(src_y_fullscreen));
				p += sizeof(src_y_fullscreen);
				length -= sizeof(src_y_fullscreen);
				src_x_fullscreen = COM_NTOL_U32(src_y_fullscreen);


				Com_memcpy(&x, p, sizeof(x));
				p += sizeof(x);
				length -= sizeof(x);
				y = COM_NTOL_32(x);

				Com_memcpy(&y, p, sizeof(y));
				p += sizeof(y);
				length -= sizeof(y);
				y = COM_NTOL_32(y);
				
				if(src_x_fullscreen == 0 || src_y_fullscreen == 0)
				{
						return false;
				}
				

				mouse_event(MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE, 1, (DWORD)( y * 65535 / src_y_fullscreen), 0, 0);
				

				return true;
		}
				break;
		case 3:/*MouseEvent*/
		{

				uint_32_t msg;	/* ��Ϣ����*/
				int_32_t x;		/* ���x������*/
				int_32_t y;		/* ���y������*/
				int_32_t data;	/*�������ݣ��������ƫ��*/

				if(!cli->is_handshake)
				{
						return false;
				}

				if(length < 16)
				{
						return false;
				}
				
				Com_memcpy(&msg, p, sizeof(msg));
				p += sizeof(msg);
				length -= sizeof(msg);
				msg = COM_NTOL_U32(msg);

				Com_memcpy(&x, p, sizeof(x));
				p += sizeof(x);
				length -= sizeof(x);
				x = COM_NTOL_32(x);

				Com_memcpy(&y, p, sizeof(y));
				p += sizeof(y);
				length -= sizeof(y);
				y = COM_NTOL_32(y);

				Com_memcpy(&data, p, sizeof(data));
				p += sizeof(data);
				length -= sizeof(data);
				data = COM_NTOL_32(data);

				/***************************************************/
				switch(msg)
				{
				case WM_MOUSEMOVE:
				{
						mouse_event(MOUSEEVENTF_MOVE, (DWORD)x, (DWORD)y, 0, 0);
				}
						return true;
				case WM_LBUTTONDOWN:
				{
						mouse_event(MOUSEEVENTF_LEFTDOWN, 0,0, 0, 0);
				}
						return true;
				case WM_LBUTTONUP:
				{
						mouse_event(MOUSEEVENTF_LEFTUP, 0,0, 0, 0);
				}
						return true;
				case WM_RBUTTONDOWN:
				{
						mouse_event(MOUSEEVENTF_RIGHTDOWN, 0,0, 0, 0);
				}
						return true;
				case WM_RBUTTONUP:
				{
						mouse_event(MOUSEEVENTF_RIGHTUP, 0,0, 0, 0);

				}
						return true;
				case WM_MOUSEWHEEL:
				{
						mouse_event(MOUSEEVENTF_WHEEL, 0,0, (DWORD)data, 0);
				}
						return true;
				case WM_MOUSEHWHEEL:
				{
						mouse_event(MOUSEEVENTF_HWHEEL, 0,0, (DWORD)data, 0);
				}
						return true;
				default:
						return false;
						break;
				}
		}
				break;
		case 4:/*KeyboardEvent*/
		{
				byte_t vk, scan, is_keydown;

				if(!cli->is_handshake)
				{
						return false;
				}

				if(length < 3)
				{
						return false;
				}

				vk = *p++;
				scan = *p++;
				is_keydown = *p++;

				keybd_event(vk, scan, is_keydown ? 0 : KEYEVENTF_KEYUP, 0);

				return true;
		}
				break;
		default:/*����*/
				return false;
		}
}



#endif



MM_NAMESPACE_END


