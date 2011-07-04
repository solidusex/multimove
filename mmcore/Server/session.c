
#include "session.h"


MM_NAMESPACE_BEGIN




#if(0)







static srvClient_t*	CreateClient(SOCKET cli_fd, const struct sockaddr_in *addr)
{
		srvClient_t		*cli;
		
		Com_ASSERT(addr != NULL && cli_fd != INVALID_SOCKET);
		cli = Com_NEW0(srvClient_t);
		cli->fd = cli_fd;

		if(inet_ntoa(addr->sin_addr))
		{
				Com_swprintf(cli->ip, 256, L"%S", inet_ntoa(addr->sin_addr));
		}
		cli->port = ntohs(addr->sin_port);

		cli->out_buf = Com_CreateBuffer(1024);
		cli->last_out_stamp = Com_GetTime_Milliseconds();

		cli->in_buf = Com_CreateBuffer(1024);
		cli->last_in_stamp = Com_GetTime_Milliseconds();

		cli->in_state = SRV_RECV_WAIT_HEADER;
		cli->remain_len = 4;
		
		cli->pos = SRV_DIR_MAX;
		Com_socket_nonblocking(cli->fd, true);

		cli->is_handshake = false;
		return cli;
}

static void			DestroyClient(srvClient_t *cli)
{
		Com_ASSERT(cli != NULL);
		
		Com_DestroyBuffer(cli->in_buf);

		Com_DestroyBuffer(cli->out_buf);

		Com_DEL(cli);
		cli = NULL;

}



static bool_t		SendKeepAlive(srvClient_t *cli)
{
		uint_32_t package_len;
		uint_16_t package_type;

		Com_ASSERT(cli != NULL);

		package_len = sizeof(package_type);
		package_len = COM_LTON_32(package_len);
		package_type = COM_LTON_16(0);

		Com_InsertBuffer(cli->out_buf, (const byte_t*)&package_len, sizeof(package_len));
		Com_InsertBuffer(cli->out_buf, (const byte_t*)&package_type, sizeof(package_type));

		return false;
}

static bool_t		SendHandShake(srvClient_t *cli)
{
		uint_32_t package_len;
		uint_16_t package_type;

		Com_ASSERT(cli != NULL);

		package_len = sizeof(package_type);
		package_len = COM_LTON_32(package_len);
		package_type = COM_LTON_16(1);

		Com_InsertBuffer(cli->out_buf, (const byte_t*)&package_len, sizeof(package_len));
		Com_InsertBuffer(cli->out_buf, (const byte_t*)&package_type, sizeof(package_type));


		return false;
}

static bool_t		SendMouseLeave(srvClient_t *cli)
{
		uint_32_t package_len;
		uint_16_t package_type;

		Com_ASSERT(cli != NULL);

		package_len = sizeof(package_type);
		package_len = COM_LTON_32(package_len);
		package_type = COM_LTON_16(2);

		Com_InsertBuffer(cli->out_buf, (const byte_t*)&package_len, sizeof(package_len));
		Com_InsertBuffer(cli->out_buf, (const byte_t*)&package_type, sizeof(package_type));

		return false;
}









static bool_t		SendData(srvClient_t *cli)
{
		bool_t	is_ok;
		const byte_t *p;
		size_t available;
		int wn;
		Com_ASSERT(cli != NULL);
		
		is_ok = true;

		available = Com_GetBufferAvailable(cli->out_buf);

		if(available <= 0)
		{
				is_ok = false;/*如果被挂进到select中，则一定有数据待发送*/
				goto END_POINT;
		}

		p = Com_GetBufferData(cli->out_buf);

		wn = send(cli->fd, (const char*)p, (int)available, 0);

		if(wn <= 0)
		{
				is_ok = false;
				goto END_POINT;
		}else
		{
				Com_EraseBuffer(cli->out_buf, (size_t)wn);
		}


END_POINT:
		cli->last_out_stamp = Com_GetTime_Milliseconds();
		return is_ok;
}






static bool_t		RecvData(srvClient_t *cli)
{
		int rn;
		u_long available;
		byte_t *buf;
		bool_t is_ok;
		Com_ASSERT(cli != NULL);

		if(ioctlsocket(cli->fd, FIONREAD, &available) != 0 || available <= 0)
		{
				Com_error(COM_ERR_WARNING, L"Cli_RecvData : ioctlsocket error\r\n");
				return false;
		}


		buf = Com_AllocBuffer(cli->in_buf, (int)available);
		rn = recv(cli->fd, (char*)buf, available, 0);

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
				Com_ASSERT(cli->remain_len > 0);

				switch(cli->in_state)
				{
				case SRV_RECV_WAIT_HEADER:
				{
						uint_32_t package_len;

						if(cli->remain_len <= Com_GetBufferAvailable(cli->in_buf))
						{
								p = Com_GetBufferData(cli->in_buf);
								Com_memcpy((byte_t*)&package_len, p, sizeof(package_len));
								Com_EraseBuffer(cli->in_buf, sizeof(package_len));
								
								package_len = COM_NTOL_32(package_len);

								if(package_len > 1 * COM_KB || package_len < 2)/*包过大或过小*/
								{
										is_ok = false;
										goto END_POINT;
								}else
								{
										cli->in_state = SRV_RECV_WAIT_PACKAGE;
										cli->remain_len = package_len;
										goto RECHECK_POINT;
								}
						}
				}
						break;
				case SRV_RECV_WAIT_PACKAGE:
				{
						if(cli->remain_len <= Com_GetBufferAvailable(cli->in_buf))
						{
								p = Com_GetBufferData(cli->in_buf);
								is_ok = HandleRecvData(cli, p, cli->remain_len);
								Com_EraseBuffer(cli->in_buf, cli->remain_len);
								
								if(is_ok)
								{
										cli->in_state = SRV_RECV_WAIT_HEADER;
										cli->remain_len = 4;
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
		cli->last_in_stamp = Com_GetTime_Milliseconds();
		return is_ok;
}


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
		case 1: /*handshake 方向 LEFT == 0, RIGHT == 1 */
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

				uint_32_t msg;	/* 消息类型*/
				int_32_t x;		/* 鼠标x轴坐标*/
				int_32_t y;		/* 鼠标y轴坐标*/
				int_32_t data;	/*特殊数据，例如滚轮偏移*/

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
		default:/*坏包*/
				return false;
		}
}



#endif



MM_NAMESPACE_END



