
#include "srv_hook.h"
#include "server.h"
#include "srv_session.h"

MM_NAMESPACE_BEGIN


bool_t Srv_Init(const srvInit_t *init)
{
		Com_UNUSED(init);
		
		return true;
}


bool_t Srv_UnInit()
{
		return true;
}






/*****************************************************************************************************************/


#if(0)


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

				Com_ASSERT(__g_client != NULL);

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
#endif



MM_NAMESPACE_END
