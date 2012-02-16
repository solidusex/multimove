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

#include "srv_wndsrv.h"
#include "server.h"
#include "srv_session.h"
#include "srv_notify.h"


MM_NAMESPACE_BEGIN

static srvInit_t		__g_srv_init = 
{
		NULL,
		NULL
};


bool_t Srv_Init(const srvInit_t *init)
{
		Com_ASSERT(init != NULL);

		__g_srv_init = *init;
		
		return true;
}


bool_t Srv_UnInit()
{
		Com_memset(&__g_srv_init, 0, sizeof(__g_srv_init));
		return true;
}


bool_t	Srv_OnNotify(const srvNotify_t *notify)
{
		Com_ASSERT(notify != NULL);

		if(__g_srv_init.on_notify != NULL)
		{
				__g_srv_init.on_notify(__g_srv_init.ctx, notify);
				return true;
		}

		return false;
}




/*****************************************************************************************************************/


static bool_t	__on_window_notify(const nmMsg_t *msg);



static SOCKET			__g_srv_sockfd = INVALID_SOCKET;
static cmThread_t		*__g_working_thread = NULL;
static bool_t			__g_is_started = false;

static cmMutex_t		__g_ss_lock;
static srvSession_t		*__g_ss = NULL;

/*
static bool_t	mouse_event_handler(size_t msg_id, const MSLLHOOKSTRUCT *mouse_stu);
*/

static void		server_io_thread_func(void *data);





bool_t	Srv_Start(const wchar_t *bind_ip, uint_16_t port_beg, uint_16_t port_end)
{

		struct sockaddr_in addr;
		SOCKET fd;
		uint_16_t	port;
		bool_t bind_ok;
		int last_error;
		Com_ASSERT(port_beg <= port_end);

		last_error = 0; 
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
		
		fd = socket(AF_INET, SOCK_STREAM, 0);
		
		if(fd == INVALID_SOCKET)
		{
				Com_error(COM_ERR_WARNING, L"Server can not allocate a new socket handle : error code = %d\r\n", WSAGetLastError());
				return false;
		}

		bind_ok = false;

		
		for(port = port_beg; !bind_ok && port <= port_end; ++port)
		{
				addr.sin_port = htons(port);
		
				if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0)
				{
						last_error = WSAGetLastError();
						if(last_error != WSAEADDRINUSE)
						{
								break;
						}
				}else
				{
						bind_ok = true;
						break;
				}
		}

		
		if(!bind_ok)
		{
				closesocket(fd);
				fd = INVALID_SOCKET;
				Com_error(COM_ERR_WARNING, L"Server can not bind socket to address %s:%d : error code = %d\r\n", bind_ip == NULL ?  L"Any" : bind_ip, port, last_error);
				return false;
		}

		if(listen(fd, 15) != 0)
		{
				closesocket(fd);
				fd = INVALID_SOCKET;
				Com_error(COM_ERR_WARNING, L"Server can not listen in address %s:%d : error code = %d\r\n", bind_ip == NULL ?  L"Any" : bind_ip, port, WSAGetLastError());
		}
		
		/*notify for ui or console*/
		Srv_NotifyOnListen(bind_ip, port);


		Com_InitMutex(&__g_ss_lock);
		__g_ss = NULL;

		
		if(!WND_Srv_Start(__on_window_notify))
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

		
		WND_Srv_Stop();
		

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
						shutdown(cli_fd, SD_BOTH);
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


bool_t	__on_window_notify(const nmMsg_t *msg)
{
		bool_t ret;
		Com_ASSERT(msg != NULL);
		
		ret = true;

		Com_LockMutex(&__g_ss_lock);
		if(__g_ss != NULL)
		{
				switch(msg->t)
				{
				case NM_MSG_CLIPDATA:
						if(SS_IsEntered(__g_ss))
						{
								ret = SS_SendClipDataMsg(__g_ss, msg);
						}else
						{
								ret = true;
						}
						break;
				default:
						Com_ASSERT(false);
						ret = false;
						break;
				}
		}
		
		Com_UnLockMutex(&__g_ss_lock);

		return ret;


}


bool_t	Server_Init(const serverInit_t *init)
{
		if(!Com_Init(&init->cm_init))
		{
				return false;
		}

		if(!Srv_Init(&init->srv_init))
		{
				Com_UnInit();
				return false;
		}

		return true;

}

bool_t	Server_UnInit()
{
		
		if(!Srv_UnInit())
		{
				return false;
		}

		if(!Com_UnInit())
		{
				return false;
		}
		return true;
}


MM_NAMESPACE_END
