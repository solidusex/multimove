
#include "srv_hook.h"
#include "server.h"


MM_NAMESPACE_BEGIN


bool_t Srv_Init(const srvInit_t *init)
{
		Com_UNUSED(init);
		Hook_Srv_Init(NULL);
		return true;
}


bool_t Srv_UnInit()
{
		Hook_Srv_UnInit();
		return true;
}






/*************************************************************************************************/



/*

传输协议，以下不论Client->Server或者Server->Client，均为网络字节序

1. Client -> Server:
[0-4)字节 包长度
[4-6)字节 包类型 分为KeepAlive = 0, HandShake = 1, MouseEvent = 2, KeyboardEvent = 3

Content:

KeeyAlive:		Content length == 0 字节 
				包总长度为6字节


HandShake:		Content length == 1字节		
				[6-7)字节	direction	byte_t	方向 LEFT == 0, RIGHT == 1
				包总长度为7字节


MouseEvent:		Content length == 16字节		
		[6- 10)字节:  msg;				uint_32_t 消息类型
		[10- 14)字节: x;				int_32_t  鼠标x轴坐标
		[14-18)字节:  y;				int_32_t  鼠标y轴坐标
		[18-22)字节:  data;				int_32_t  特殊数据，例如滚轮偏移
		[22-23)字节:  mouse_enter		byte_t	  是否为鼠标首次进入Server桌面，是的话x,y为鼠标Client端坐标绝对值

		包总长度为22字节

KeyboardEvent:	Content length == 3字节			
		[6-7)字节： vk;				byte_t			虚拟键盘码
		[7-8)字节:	 scan;				byte_t			扫描码
		[8-9)字节:	 is_keydown			bool_t			是否为键被按下

		包总长度为9字节



2. Server -> Client:
[0-4)字节  包长度
[4-6)字节  包类型, 分为KeepAlive = 0, HandShake = 1, MouseLeave = 2

Content:

KeeyAlive:		Content length == 0字节

HandShake:		Content length == 0字节

MouseLeave:		Content length == 0字节

*/



typedef enum
{
		SRV_RECV_WAIT_HEADER,
		SRV_RECV_WAIT_PACKAGE
}srvRecvState_t;

#define KEEPALIVE_TIMEOUT				10 * 1000		/*10秒钟没有任何socket上的IO操作则认为该连接关闭*/


typedef struct __client_tag
{
		SOCKET			fd;
		wchar_t			ip[256];
		uint_16_t		port;

		cmBuffer_t		*out_buf;
		uint_64_t		last_out_stamp;

		cmBuffer_t		*in_buf;
		uint_64_t		last_in_stamp;

		srvRecvState_t	in_state;
		size_t			remain_len;		/*起始为4，下一次为本次header所指明的长度，不能超过1K字节，否则关闭连接*/

		
		srvDir_t		pos;
		bool_t			is_handshake;

}srvClient_t;






static srvClient_t*	CreateClient(SOCKET cli_fd, const struct sockaddr_in *addr);
static void			DestroyClient(srvClient_t *cli);


static bool_t		SendData(srvClient_t *cli);
static bool_t		SendKeepAlive(srvClient_t *cli);
static bool_t		SendHandShake(srvClient_t *cli);
static bool_t		SendMouseLeave(srvClient_t *cli);


static bool_t		RecvData(srvClient_t *cli);
static bool_t		HandleRecvData(srvClient_t *cli, const byte_t *data, size_t length);
static bool_t		OnTimer(srvClient_t *cli);





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

//KeepAlive = 0, HandShake = 1, MouseEvent = 2, KeyboardEvent = 3

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
						return true;
						break;
				case 1:
						cli->pos = SRV_RIGHT_SRV;
						cli->is_handshake = true;
						return true;
				default:
						return false;
				}
		}
				break;
		case 2:/*MouseEvent*/
		{

				uint_32_t msg;	/* 消息类型*/
				int_32_t x;		/* 鼠标x轴坐标*/
				int_32_t y;		/* 鼠标y轴坐标*/
				int_32_t data;	/*特殊数据，例如滚轮偏移*/
				byte_t	mouse_enter; /*是否为鼠标首次进入Server桌面，是的话x,y为鼠标Client端坐标绝对值*/

				if(!cli->is_handshake)
				{
						return false;
				}

				if(length < 17)
				{
						return false;
				}
				
				Com_memcpy(&msg, p, sizeof(msg));
				p += sizeof(msg);
				length -= sizeof(msg);
				msg = COM_NTOL_32(msg);

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

				Com_memcpy(&mouse_enter, p, sizeof(mouse_enter));
				p += sizeof(mouse_enter);
				length -= sizeof(mouse_enter);


				if(mouse_enter == 1)
				{

				}else
				{

				}

				
				return false;
		}
				break;
		case 3:/*KeyboardEvent*/
		{
				return false;
		}
				break;
		default:
				return false;
		}
}



/*****************************************************************************************************************/





static SOCKET			__g_srv_sockfd = INVALID_SOCKET;
static cmThread_t		*__g_working_thread = NULL;
static bool_t			__g_is_started = false;

static cmMutex_t		__g_client_lock;
static srvClient_t		*__g_client = NULL;

static bool_t	mouse_event_handler(size_t msg_id, const MSLLHOOKSTRUCT *mouse_stu);
static void		server_io_thread_func(void *data);


bool_t	Srv_Start(const wchar_t *bind_ip, uint_16_t port)
{
		struct sockaddr_in addr;
		SOCKET fd;
		Com_memset(&addr, 0, sizeof(addr));

		if(bind_ip == NULL)
		{
				addr.sin_family = AF_INET;
				addr.sin_addr.s_addr = INADDR_ANY;
		}else
		{
				if(!Com_GetIPByHostName_V4(bind_ip, &addr))
				{
						return false;
				}
		}

		addr.sin_port = htons(port);
		
		fd = socket(AF_INET, SOCK_STREAM, 0);
		
		if(fd == INVALID_SOCKET)
		{
				return false;
		}

		if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0 || listen(fd, 15) != 0/* || Com_socket_nonblocking(fd, true) != 0*/)
		{
				closesocket(fd);
				return false;
		}


		Com_InitMutex(&__g_client_lock);
		__g_client = NULL;

		
		if(!Hook_Srv_Start(mouse_event_handler))
		{
				closesocket(fd);
				return false;
		}
		
		
		__g_srv_sockfd = fd;
		__g_is_started = true;
		__g_working_thread = Com_CreateThread(server_io_thread_func, NULL, NULL);
		return true;
}




bool_t	Srv_Stop()
{
		if(!Srv_IsStarted())
		{
				Com_ASSERT(false);
				return false;
		}

		
		Hook_Srv_Stop();

		__g_is_started = false;
		Com_JoinThread(__g_working_thread);
		Com_CloseThread(__g_working_thread);
		__g_working_thread = NULL;
		
		closesocket(__g_srv_sockfd);
		__g_srv_sockfd = INVALID_SOCKET;

		Com_UnInitMutex(&__g_client_lock);
		__g_client = NULL;
		
		return true;
}



bool_t	Srv_IsStarted()
{
		return __g_is_started;
}



#define RECHECK_STATE_TIMEOUT	1 * 1000
#define TIMER_RESULTION			3 * 1000

static bool_t	handle_client_control(SOCKET cli_fd, const struct sockaddr_in *addr);



static void	server_io_thread_func(void *data)
{
		SOCKET cli_fd;
		struct sockaddr_in addr;
		int addr_len;
		const uint_64_t timeout = RECHECK_STATE_TIMEOUT;
		Com_UNUSED(data);
		
		while(Srv_IsStarted())
		{
				addr_len = 0;
				Com_memset(&addr, 0, sizeof(addr));
				cli_fd = Com_accpet_timeout(__g_srv_sockfd, (struct sockaddr*)&addr, &addr_len, &timeout);

				if(cli_fd == INVALID_SOCKET)
				{
						if(WSAGetLastError() != WSAETIMEDOUT)
						{
								Com_error(COM_ERR_FATAL, L"Server internal error Com_accpet_timeout : == %d\r\n", WSAGetLastError());
						}else
						{
								continue;
						}
				}else
				{
						handle_client_control(cli_fd, &addr);
						closesocket(cli_fd);
						cli_fd = INVALID_SOCKET;
				}
		}
}







static bool_t	handle_client_control(SOCKET client_fd, const struct sockaddr_in *addr)
{

		bool_t is_ok;
		uint_64_t		time_mark = TIMER_RESULTION;
		Com_ASSERT(client_fd != INVALID_SOCKET && addr != NULL);

		Com_LockMutex(&__g_client_lock);
		__g_client = CreateClient(client_fd, addr);
		Com_UnLockMutex(&__g_client_lock);

		
		is_ok = true;

		while(Srv_IsStarted())
		{
				int ret;
				struct timeval tv;
				struct fd_set rd_set, wd_set, ex_set;
				struct fd_set *prd, *pwd, *pex;
				size_t rd_cnt, wd_cnt, ex_cnt;
				
						
				Com_LockMutex(&__g_client_lock);

				FD_ZERO(&rd_set);
				FD_ZERO(&wd_set);
				FD_ZERO(&ex_set);
				rd_cnt = 0;
				wd_cnt = 0;
				ex_cnt = 0;

				tv.tv_sec = 0;
				tv.tv_usec = RECHECK_STATE_TIMEOUT * 1000;
				
				FD_SET(__g_client->fd, &rd_set);
				rd_cnt += 1;
				
				if(Com_GetBufferAvailable(__g_client->out_buf) > 0)
				{
						FD_SET(__g_client->fd, &wd_set);
						wd_cnt += 1;
				}
				
				prd = rd_cnt > 0 ? &rd_set : NULL;
				pwd = wd_cnt > 0 ? &wd_set : NULL;
				pex = ex_cnt > 0 ? &ex_set : NULL;

				Com_UnLockMutex(&__g_client_lock);
				


				ret = select(0, prd, pwd, pex, &tv);


				Com_LockMutex(&__g_client_lock);

				if(ret < 0)
				{
						Com_UnLockMutex(&__g_client_lock);
						is_ok = false;
						Com_error(COM_ERR_WARNING, L"handle_client_control : select (%d)\r\n", WSAGetLastError());
						goto END_POINT;
				}
				

				if(Com_GetTime_Milliseconds() - time_mark >= TIMER_RESULTION)
				{
						if(!OnTimer(__g_client))
						{
								Com_UnLockMutex(&__g_client_lock);
								is_ok = false;
								goto END_POINT;
						}else
						{
								Com_UnLockMutex(&__g_client_lock);
								time_mark = Com_GetTime_Milliseconds();
						}
				}

					
				if(ret == 0)
				{
						Com_UnLockMutex(&__g_client_lock);
						continue;
				}else
				{
						if(FD_ISSET(__g_client->fd, &rd_set))
						{
								if(!RecvData(__g_client))
								{
										Com_UnLockMutex(&__g_client_lock);
										is_ok = false;
										goto END_POINT;
								}
						}

						if(FD_ISSET(__g_client->fd, &wd_set))
						{
								if(!SendData(__g_client))
								{
										Com_UnLockMutex(&__g_client_lock);
										is_ok = false;
										goto END_POINT;
								}
						}

						Com_UnLockMutex(&__g_client_lock);
				}
		}
		
END_POINT:

		Com_LockMutex(&__g_client_lock);
		DestroyClient(__g_client);
		__g_client = NULL;
		Com_UnLockMutex(&__g_client_lock);



		return false;
}





static bool_t mouse_event_handler(size_t msg_id, const MSLLHOOKSTRUCT *mouse_stu)
{
		bool_t is_ok;

		int x_full_screen, y_full_screen;
		

		Com_ASSERT(mouse_stu != NULL);
		
		is_ok = true;
		x_full_screen = GetSystemMetrics(SM_CXSCREEN);
		y_full_screen = GetSystemMetrics(SM_CYSCREEN);


		Com_LockMutex(&__g_client_lock);

		if(__g_client == NULL)
		{
				Com_UnLockMutex(&__g_client_lock);
				return true;
		}

		switch(msg_id)
		{
		case WM_MOUSEMOVE:
		{
				if(mouse_stu->pt.x <= 0 && __g_client->pos == SRV_RIGHT_SRV)
				{
						Com_printf(L"SendMouseLeave On : (%d:%d)\r\n", mouse_stu->pt.x, mouse_stu->pt.y);
						SendMouseLeave(__g_client);
						is_ok = true;
				}else if(mouse_stu->pt.x >= x_full_screen && __g_client->pos == SRV_LEFT_SRV)
				{
						Com_printf(L"SendMouseLeave On : (%d:%d)\r\n", mouse_stu->pt.x, mouse_stu->pt.y);
						SendMouseLeave(__g_client);
						is_ok = true;
				}else
				{
						is_ok = true;
				}
		}
				break;
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		default:
				is_ok = true;
				break;
		}
		Com_UnLockMutex(&__g_client_lock);
		return is_ok;
}



MM_NAMESPACE_END
