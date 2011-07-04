
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





static SOCKET			__g_srv_sockfd = INVALID_SOCKET;
static cmThread_t		*__g_working_thread = NULL;
static bool_t			__g_is_started = false;

static cmMutex_t		__g_ss_lock;
static srvSession_t		*__g_ss = NULL;

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
				Com_printf(L"Server bind any address\r\n");
		}else
		{
				if(!Com_GetIPByHostName_V4(bind_ip, &addr))
				{
						Com_error(COM_ERR_WARNING, L"Can't get ip host information from %s\r\n", bind_ip);
						return false;
				}
		}

		addr.sin_port = htons(port);
		
		fd = socket(AF_INET, SOCK_STREAM, 0);
		
		if(fd == INVALID_SOCKET)
		{
				Com_error(COM_ERR_WARNING, L"Server can not allocate a new socket handle : error code = %d\r\n", WSAGetLastError());
				return false;
		}

		if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0 || listen(fd, 15) != 0)
		{
				closesocket(fd);
				fd = INVALID_SOCKET;
				Com_error(COM_ERR_WARNING, L"Server can not bind socket to address %s:%d : error code = %d\r\n", bind_ip == NULL ?  L"Any" : bind_ip, port, WSAGetLastError());
				return false;
		}


		Com_InitMutex(&__g_ss_lock);
		__g_ss = NULL;

		
		if(!Hook_Srv_Start(mouse_event_handler))
		{
				Com_UnInitMutex(&__g_ss_lock);
				closesocket(fd);
				fd = INVALID_SOCKET;
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

		Com_UnInitMutex(&__g_ss_lock);
		__g_ss = NULL;
		
		return true;
}



bool_t	Srv_IsStarted()
{
		return __g_is_started;
}



static bool_t	handle_client_control(SOCKET cli_fd, const struct sockaddr_in *addr);



static void	server_io_thread_func(void *data)
{
		SOCKET cli_fd;
		struct sockaddr_in addr;
		int addr_len;
		const uint_64_t timeout = NM_BLOCKING_TIMEOUT;
		Com_UNUSED(data);
		
		while(Srv_IsStarted())
		{
				addr_len = sizeof(addr);
				Com_memset(&addr, 0, sizeof(addr));
				cli_fd = Com_accpet_timeout(__g_srv_sockfd, (struct sockaddr*)&addr, &addr_len, &timeout);

				if(cli_fd == INVALID_SOCKET)
				{
						if(WSAGetLastError() != WSAETIMEDOUT)
						{
								Com_error(COM_ERR_FATAL, L"Server accept failed : error code = %d\r\n", WSAGetLastError());
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
		uint_64_t		time_mark = Com_GetTime_Milliseconds();
		Com_ASSERT(client_fd != INVALID_SOCKET && addr != NULL);
		
		Com_ASSERT(__g_ss == NULL);

		Com_LockMutex(&__g_ss_lock);
		__g_ss = SS_OnClientSession(client_fd, addr);
		Com_UnLockMutex(&__g_ss_lock);

		
		is_ok = true;

		while(is_ok && Srv_IsStarted())
		{
				int ret;
				struct timeval tv;
				struct fd_set rd_set, wd_set, ex_set;
				struct fd_set *prd, *pwd, *pex;
				size_t rd_cnt, wd_cnt, ex_cnt;
				

				Com_LockMutex(&__g_ss_lock);

				FD_ZERO(&rd_set);
				FD_ZERO(&wd_set);
				FD_ZERO(&ex_set);
				rd_cnt = 0;
				wd_cnt = 0;
				ex_cnt = 0;

				tv.tv_sec = 0;
				tv.tv_usec = NM_BLOCKING_TIMEOUT * 1000;
				
				FD_SET(__g_ss->fd, &rd_set);
				rd_cnt += 1;
				
				if(SS_HasDataToSend(__g_ss))
				{
						FD_SET(__g_ss->fd, &wd_set);
						wd_cnt += 1;
				}
				
				prd = rd_cnt > 0 ? &rd_set : NULL;
				pwd = wd_cnt > 0 ? &wd_set : NULL;
				pex = ex_cnt > 0 ? &ex_set : NULL;

				
				Com_UnLockMutex(&__g_ss_lock);
				


				ret = select(0, prd, pwd, pex, &tv);



/***********************************Handle Block******************************/
				Com_LockMutex(&__g_ss_lock);

				Com_ASSERT(__g_ss != NULL);

				if(ret < 0)
				{
						is_ok = false;
						Com_error(COM_ERR_WARNING, L"select has error : %d\r\n", WSAGetLastError());
						goto HANDLE_END_POINT;
				}
				

				if(Com_GetTime_Milliseconds() - time_mark >= NM_TIMER_TICK)
				{
						if(!SS_OnTimer(__g_ss))
						{
								is_ok = false;
								goto HANDLE_END_POINT;
						}else
						{
								time_mark = Com_GetTime_Milliseconds();
						}
				}

				if(ret == 0)
				{
						/*Timeout*/
						goto HANDLE_END_POINT;
				}else
				{
						if(FD_ISSET(__g_ss->fd, &rd_set))
						{
								if(!SS_OnRecvData(__g_ss))
								{
										is_ok = false;
										goto HANDLE_END_POINT;
								}
						}

						if(FD_ISSET(__g_ss->fd, &wd_set))
						{
								if(!SS_OnSendData(__g_ss))
								{
										is_ok = false;
										goto HANDLE_END_POINT;
								}
						}
				}
HANDLE_END_POINT:
				Com_UnLockMutex(&__g_ss_lock);
		}
		


		Com_LockMutex(&__g_ss_lock);
		SS_CloseClientSession(__g_ss);
		__g_ss = NULL;
		Com_UnLockMutex(&__g_ss_lock);
		
		return is_ok;
}







static bool_t mouse_event_handler(size_t msg_id, const MSLLHOOKSTRUCT *mouse_stu)
{
		bool_t is_ok;

		int x_full_screen, y_full_screen;
		

		Com_ASSERT(mouse_stu != NULL);

		if(msg_id == WM_MOUSEMOVE)
		{
				Com_printf(L"On MouseMove (%d:%d)\r\n", mouse_stu->pt.x, mouse_stu->pt.y);
		}
		
		is_ok = true;
		x_full_screen = GetSystemMetrics(SM_CXSCREEN);
		y_full_screen = GetSystemMetrics(SM_CYSCREEN);

		
		Com_LockMutex(&__g_ss_lock);

		if(__g_ss != NULL)
		{
				switch(msg_id)
				{
				case WM_MOUSEMOVE:
				{
						if(mouse_stu->pt.x <= 0 && __g_ss->pos == NM_POS_RIGHT)
						{
								
								if(SS_IsEntered(__g_ss))
								{
										nmMsg_t msg;
										msg.t = NM_MSG_LEAVE;
										msg.leave.src_x_fullscreen = x_full_screen;
										msg.leave.src_y_fullscreen = y_full_screen;
										msg.leave.x = mouse_stu->pt.x;
										msg.leave.y = mouse_stu->pt.y;

										Com_printf(L"Send mouse leave to session (%s:%d) on point (%d:%d)", __g_ss->ip, __g_ss->port, mouse_stu->pt.x, mouse_stu->pt.y);
										SS_SendMouseLeave(__g_ss, &msg);
								}
								is_ok = true;
						}else if(mouse_stu->pt.x >= x_full_screen && __g_ss->pos == NM_POS_LEFT)
						{
								if(SS_IsEntered(__g_ss))
								{
										nmMsg_t msg;
										msg.t = NM_MSG_LEAVE;
										msg.leave.src_x_fullscreen = x_full_screen;
										msg.leave.src_y_fullscreen = y_full_screen;
										msg.leave.x = mouse_stu->pt.x;
										msg.leave.y = mouse_stu->pt.y;

										Com_printf(L"Send mouse leave to session (%s:%d) on point (%d:%d)", __g_ss->ip, __g_ss->port, mouse_stu->pt.x, mouse_stu->pt.y);
										SS_SendMouseLeave(__g_ss, &msg);
								}
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
		}
		Com_UnLockMutex(&__g_ss_lock);
		return is_ok;
}


MM_NAMESPACE_END
