#include "client_in.h"

MM_NAMESPACE_BEGIN









static cliSrv_t*	Cli_CreateServer(cliServerDir_t	dir, const wchar_t			*ip, uint_16_t		port)
{
		cliSrv_t *srv = NULL;
		SOCKET fd = INVALID_SOCKET;
		struct sockaddr_in		addr;
		const uint_64_t timeout = CLI_CONNECT_TO_SRV_TIMEOUT;
		Com_ASSERT(ip != NULL);

		if(!Com_GetIPByHostName_V4(ip, &addr))
		{
				return NULL;
		}

		addr.sin_port = htons((u_short)port);

		fd = socket(AF_INET, SOCK_STREAM, 0);

		if(fd == INVALID_SOCKET)
		{
				return NULL;
		}

		if(Com_connect_timeout(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in), &timeout) != 0)
		{
				closesocket(fd);
				return NULL;
		}
		
		Com_socket_nonblocking(fd, true);

		srv = Com_NEW0(cliSrv_t);
		srv->ip = Com_wcsdup(ip);
		srv->port = port;
		srv->dir = dir;
		srv->sockfd = fd;
		srv->in_buf = Com_CreateBuffer(1024);
		Com_InitMutex(&srv->in_lock);
		srv->recv_state = CLI_RECV_WAIT_HEADER;
		srv->remain_len = 4;
		srv->last_recv_tm = Com_GetTime_Milliseconds();
		

		srv->out_buf = Com_CreateBuffer(1024);
		Com_InitMutex(&srv->out_lock);
		srv->last_send_tm = Com_GetTime_Milliseconds();
		
		Cli_SendHandShake(srv, srv->dir);
		srv->is_active_side = false;
		return srv;
}



static void			Cli_DestroyServer(cliSrv_t *srv)
{
		Com_ASSERT(srv != NULL);
		
		Com_DEL(srv->ip);
		srv->ip = NULL;
		closesocket(srv->sockfd);
		srv->sockfd = INVALID_SOCKET;
		Com_DestroyBuffer(srv->in_buf);
		srv->in_buf = NULL;
		Com_UnInitMutex(&srv->in_lock);

		Com_DestroyBuffer(srv->out_buf);
		srv->out_buf = NULL;
		Com_UnInitMutex(&srv->out_lock);
		Com_DEL(srv);
		srv = NULL;
}



static bool_t		Cli_IsActiveSide(const cliSrv_t *srv)
{
		Com_ASSERT(srv != NULL);
		return srv->is_active_side;
}


static bool_t		Cli_HasDataToSend(cliSrv_t *srv)
{
		bool_t	has_data_to_send = false;
		Com_ASSERT(srv != NULL);
		Com_LockMutex(&srv->out_lock);
		has_data_to_send = Com_GetBufferAvailable(srv->out_buf) > 0 ? true : false;
		Com_UnLockMutex(&srv->out_lock);
		return has_data_to_send;
}



/*发送*/
static bool_t		Cli_SendKeepAlive(cliSrv_t *srv)
{
		uint_32_t package_len;
		uint_16_t package_type;

		Com_ASSERT(srv != NULL);

		package_len = sizeof(package_type);
		package_len = COM_LTON_32(package_len);
		
		package_type = COM_LTON_U16(0);

		Com_LockMutex(&srv->out_lock);
		Com_InsertBuffer(srv->out_buf, (const byte_t*)&package_len, sizeof(package_len));
		Com_InsertBuffer(srv->out_buf, (const byte_t*)&package_type, sizeof(package_type));

		Com_UnLockMutex(&srv->out_lock);
		return true;
}


static bool_t		Cli_SendHandShake(cliSrv_t *srv, cliServerDir_t dir)
{
		uint_32_t package_len;
		uint_16_t package_type;
		byte_t	 side;

		Com_ASSERT(srv != NULL);

		package_len = sizeof(package_type) + sizeof(side);
		package_len = COM_LTON_32(package_len);

		package_type = COM_LTON_16(1);
		
		switch(dir)
		{
		case CLI_LEFT_SRV:
				side = 0;
				break;
		case CLI_RIGHT_SRV:
				side = 1;
				break;
		default:
				Com_ASSERT(false);
				break;
		}

		Com_LockMutex(&srv->out_lock);

		Com_InsertBuffer(srv->out_buf, (const byte_t*)&package_len, sizeof(package_len));
		Com_InsertBuffer(srv->out_buf, (const byte_t*)&package_type, sizeof(package_type));
		Com_InsertBuffer(srv->out_buf, (const byte_t*)&side, sizeof(side));

		Com_UnLockMutex(&srv->out_lock);


		return true;
}


static bool_t		Cli_SendEnterMsg(cliSrv_t *srv, const hkEnterEvent_t     *enter)
{
		uint_32_t package_len;
		uint_16_t package_type;
		
		uint_32_t		src_x;
		uint_32_t		src_y;

		int_32_t		x;		/*鼠标x轴坐标*/
		int_32_t		y;		/*鼠标y轴坐标*/

		Com_ASSERT(srv != NULL && enter != NULL);

		package_len = sizeof(package_type) + sizeof(src_x) + sizeof(src_y) + sizeof(x) + sizeof(y);
		package_len = COM_LTON_U32(package_len);
		package_type = COM_LTON_U16(2);

		src_x = COM_LTON_U32(enter->src_x_fullscreen);
		src_y = COM_LTON_U32(enter->src_y_fullscreen);
		x = COM_LTON_32(enter->x);
		y = COM_LTON_32(enter->y);

		
		Com_LockMutex(&srv->out_lock);



		Com_InsertBuffer(srv->out_buf, (const byte_t*)&package_len, sizeof(package_len));
		Com_InsertBuffer(srv->out_buf, (const byte_t*)&package_type, sizeof(package_type));

		Com_InsertBuffer(srv->out_buf, (const byte_t*)&src_x, sizeof(src_x));
		Com_InsertBuffer(srv->out_buf, (const byte_t*)&src_y, sizeof(src_y));
		Com_InsertBuffer(srv->out_buf, (const byte_t*)&x, sizeof(x));
		Com_InsertBuffer(srv->out_buf, (const byte_t*)&y, sizeof(y));

		Com_UnLockMutex(&srv->out_lock);

		srv->is_active_side = true;

		return true;
}

static bool_t		Cli_SendMouseMsg(cliSrv_t *srv, const hkMouseEvent_t *mouse_msg)
{
		uint_32_t package_len;
		uint_16_t package_type;
		
		uint_32_t		msg;	/* 消息类型*/
		int_32_t		x;		/*鼠标x轴坐标*/
		int_32_t		y;		/*鼠标y轴坐标*/
		uint_32_t		data;	/*特殊数据，例如滚轮偏移*/


		Com_ASSERT(srv != NULL);

		package_len = sizeof(package_type) + sizeof(msg) + sizeof(x) + sizeof(y) + sizeof(data);
		package_len = COM_LTON_U32(package_len);
		package_type = COM_LTON_U16(3);
		msg = COM_LTON_U32(mouse_msg->msg);
		x = COM_LTON_32(mouse_msg->x);
		y = COM_LTON_32(mouse_msg->y);
		data = COM_LTON_32(mouse_msg->data);

		Com_LockMutex(&srv->out_lock);


		Com_InsertBuffer(srv->out_buf, (const byte_t*)&package_len, sizeof(package_len));
		Com_InsertBuffer(srv->out_buf, (const byte_t*)&package_type, sizeof(package_type));

		Com_InsertBuffer(srv->out_buf, (const byte_t*)&msg, sizeof(msg));
		Com_InsertBuffer(srv->out_buf, (const byte_t*)&x, sizeof(x));
		Com_InsertBuffer(srv->out_buf, (const byte_t*)&y, sizeof(y));
		Com_InsertBuffer(srv->out_buf, (const byte_t*)&data, sizeof(data));

		Com_UnLockMutex(&srv->out_lock);

		return true;
}


static bool_t		Cli_SendKeyboardMsg(cliSrv_t *srv, const hkKeyboardEvent_t *keyboard)
{
		/*
		[6-7)字节： vk;				byte_t			
		[7-8)字节:	 scan;				byte_t			
		[8-9)字节:	 is_keydown			bool_t			
		*/


		uint_32_t package_len;
		uint_16_t package_type;
		
		byte_t			vk;		/* 虚拟键盘码*/
		byte_t			scan;	/*扫描码*/
		byte_t			is_keydown; /*是否为键被按下*/

		Com_ASSERT(srv != NULL);

		package_len = sizeof(package_type) + sizeof(vk) + sizeof(scan) + sizeof(is_keydown);
		package_len = COM_LTON_U32(package_len);
		package_type = COM_LTON_U16(4);
		vk = keyboard->vk;
		scan = keyboard->scan;
		is_keydown = keyboard->is_keydown;
		
		
		Com_LockMutex(&srv->out_lock);

		Com_InsertBuffer(srv->out_buf, (const byte_t*)&package_len, sizeof(package_len));
		Com_InsertBuffer(srv->out_buf, (const byte_t*)&package_type, sizeof(package_type));

		Com_InsertBuffer(srv->out_buf, (const byte_t*)&vk, sizeof(vk));
		Com_InsertBuffer(srv->out_buf, (const byte_t*)&scan, sizeof(scan));
		Com_InsertBuffer(srv->out_buf, (const byte_t*)&is_keydown, sizeof(is_keydown));
		

		Com_UnLockMutex(&srv->out_lock);

		return true;
}


/*接收*/
static bool_t		Cli_HandleRecvBuffer(cliSrv_t *srv, const byte_t *data, size_t length)
{
		uint_16_t package_type;
		const byte_t *p;
		Com_ASSERT(srv != NULL && data != NULL && length >= 2);

		if(length < 2)
		{
				return false;
		}

		p = data;

		Com_memcpy((byte_t*)&package_type, p, 2);
		p += 2;
		
		package_type = COM_NTOL_16(package_type);

		switch(package_type)
		{
		case 0: /*keepalive*/
				Com_printf(L"On KeepAlive\r\n");
				return true;
		case 1: /*handshake reply*/
				Com_printf(L"On handshake reply\r\n");
				return true;
		case 2:/*mouse leave*/
				if(srv->is_active_side)
				{

						srv->is_active_side = false;
						Hook_Cli_ControlReturn();
				}
				return true;
		default:
				return false;
		}
}



static bool_t		Cli_RecvData(cliSrv_t *srv)
{
		int rn;
		u_long available;
		byte_t *buf;
		bool_t is_ok;
		Com_ASSERT(srv != NULL);

		if(ioctlsocket(srv->sockfd, FIONREAD, &available) != 0/* || available <= 0*/)
		{
				Com_error(COM_ERR_WARNING, L"Cli_RecvData : ioctlsocket error : %d\r\n", WSAGetLastError());
				return false;
		}

		Com_LockMutex(&srv->in_lock);
		

		buf = Com_AllocBuffer(srv->in_buf, (int)available);
		rn = recv(srv->sockfd, (char*)buf, available, 0);

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
				Com_ASSERT(srv->remain_len > 0);

				switch(srv->recv_state)
				{
				case CLI_RECV_WAIT_HEADER:
				{
						uint_32_t package_len;

						if(srv->remain_len <= Com_GetBufferAvailable(srv->in_buf))
						{
								p = Com_GetBufferData(srv->in_buf);
								Com_memcpy((byte_t*)&package_len, p, sizeof(package_len));
								Com_EraseBuffer(srv->in_buf, sizeof(package_len));


								package_len = COM_NTOL_32(package_len);

								if(package_len > 1 * COM_KB || package_len < 2)/*包过大或过小*/
								{
										is_ok = false;
										goto END_POINT;
								}else
								{
										srv->recv_state = CLI_RECV_WAIT_PACKAGE;
										srv->remain_len = package_len;
										goto RECHECK_POINT;
								}
						}
				}
						break;
				case CLI_RECV_WAIT_PACKAGE:
				{
						if(srv->remain_len <= Com_GetBufferAvailable(srv->in_buf))
						{
								p = Com_GetBufferData(srv->in_buf);
								is_ok = Cli_HandleRecvBuffer(srv, p, srv->remain_len);
								Com_EraseBuffer(srv->in_buf, srv->remain_len);
								
								if(is_ok)
								{
										srv->recv_state = CLI_RECV_WAIT_HEADER;
										srv->remain_len = 4;
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
		srv->last_recv_tm = Com_GetTime_Milliseconds();
		Com_UnLockMutex(&srv->in_lock);
		return is_ok;
		
}


static bool_t		Cli_SendData(cliSrv_t *srv)
{
		bool_t	is_ok;
		const byte_t *p;
		size_t available;
		int wn;
		Com_ASSERT(srv != NULL);
		Com_LockMutex(&srv->in_lock);


		is_ok = true;

		available = Com_GetBufferAvailable(srv->out_buf);

		if(available <= 0)
		{
				is_ok = false;/*如果被挂进到select中，则一定有数据待发送*/
				goto END_POINT;
		}

		p = Com_GetBufferData(srv->out_buf);

		wn = send(srv->sockfd, (const char*)p, (int)available, 0);

		if(wn <= 0)
		{
				Com_printf(L"send failed\r\n");
				is_ok = false;
				goto END_POINT;
		}else
		{
				Com_EraseBuffer(srv->out_buf, wn);
		}


END_POINT:
		srv->last_send_tm = Com_GetTime_Milliseconds();
		Com_UnLockMutex(&srv->in_lock);
		return is_ok;
}


static bool_t		Cli_OnTimer(cliSrv_t *srv)
{
		Com_ASSERT(srv != NULL);

		if(Com_GetTime_Milliseconds() - srv->last_recv_tm > KEEPALIVE_TIMEOUT)
		{
				Com_printf(L"recv timeout\r\n");
				return false;
		}
		

		if(Com_GetTime_Milliseconds() - srv->last_send_tm > KEEPALIVE_TIMEOUT - 5000)
		{
				Com_printf(L"Timeout Send KeepAlive\r\n");
				Cli_SendKeepAlive(srv);
		}
		
		return true;
}



MM_NAMESPACE_END

