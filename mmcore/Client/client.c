
#include "cli_hook.h"
#include "client.h"
#include "client_in.h"

MM_NAMESPACE_BEGIN



bool_t Cli_Init(const cliInit_t *init)
{
		Com_UNUSED(init);
		return true;
}

bool_t Cli_UnInit()
{
		return true;
}




/************************************************************************************************************************************/
#if(0)


static void	client_io_thread_func(void *data);
static bool_t hook_dispatch(const hkCliDispatchEntryParam_t *parameter);

static cmMutex_t		__g_srv_mtx;
static cliSrv_t			*__g_srv_set[CLI_DIR_MAX];
static cmThread_t		*__g_working_thread = NULL;
static bool_t			__g_is_started = false;




bool_t	Cli_IsStarted()
{
		return __g_is_started;
}


bool_t	Cli_Start()
{
		if(Cli_IsStarted())
		{
				Com_ASSERT(false);
				return false;
		}

		if(!Hook_Cli_Start())
		{
				Com_ASSERT(false);
				return false;
		}

		Com_InitMutex(&__g_srv_mtx);
		Com_memset(__g_srv_set, 0, sizeof(__g_srv_set));
		__g_is_started = true;
		__g_working_thread = Com_CreateThread(client_io_thread_func, NULL, NULL);
		return true;
}




bool_t	Cli_Stop()
{
		size_t i;
		if(!Cli_IsStarted())
		{
				Com_ASSERT(false);
				return false;
		}

		__g_is_started = false;
		Com_JoinThread(__g_working_thread);
		Com_CloseThread(__g_working_thread);
		__g_working_thread = NULL;



		for(i = 0; i < CLI_DIR_MAX; ++i)
		{
				if(__g_srv_set[i] != NULL)
				{
						Hook_Cli_UnRegisterDispatch((hkCliDirection_t)i);
						Cli_DestroyServer(__g_srv_set[i]);
						__g_srv_set[i] = NULL;
				}
		}

		if(!Hook_Cli_Stop())
		{
				Com_ASSERT(false);
				return false;
		}
		

		Com_memset(__g_srv_set, 0, sizeof(__g_srv_set));
		Com_UnInitMutex(&__g_srv_mtx);

		return true;
}




bool_t	Cli_InsertServer(cliServerDir_t dir, const wchar_t *srv_ip, uint_16_t port)
{
		bool_t has_srv;
		cliSrv_t *srv;
		Com_ASSERT(srv_ip != NULL);


		if(!Cli_IsStarted())
		{
				Com_ASSERT(false);
				return false;
		}

		Com_LockMutex(&__g_srv_mtx);
		has_srv = __g_srv_set[dir] != NULL;
		Com_UnLockMutex(&__g_srv_mtx);

		if(has_srv)
		{
				return false;
		}


		srv = Cli_CreateServer(dir, srv_ip, port);

		if(srv == NULL)
		{
				return false;
		}

		Com_LockMutex(&__g_srv_mtx);
		__g_srv_set[dir] = srv;

		if(!Hook_Cli_RegisterDispatch((hkCliDirection_t)dir, (void*)srv, hook_dispatch))
		{
				Com_error(COM_ERR_FATAL, L"Internal Error : Hook_Cli_RegisterDispatch Failed\r\n");
		}
		Com_UnLockMutex(&__g_srv_mtx);
		return true;
}



bool_t	Cli_RemoveServer(cliServerDir_t dir)
{
		cliSrv_t *srv;
		Com_ASSERT(dir < CLI_DIR_MAX);

		Com_printf(L"On Cli_RemoveServer\r\n");
		
		if(!Cli_IsStarted())
		{
				Com_ASSERT(false);
				return false;
		}

		

		Com_LockMutex(&__g_srv_mtx);

		
		if(!Hook_Cli_UnRegisterDispatch((hkCliDirection_t)dir))
		{
				Com_error(COM_ERR_FATAL, L"Internal Error : Hook_Cli_UnRegisterDispatch Failed\r\n");
		}
		
		srv = __g_srv_set[dir];
		__g_srv_set[dir] = NULL;

		
		Com_UnLockMutex(&__g_srv_mtx);

		if(srv)
		{
				if(Cli_IsActiveSide(srv))
				{
						if(!Hook_Cli_ControlReturn())
						{
								Com_error(COM_ERR_FATAL, L"Internal Error : Hook_Cli_ControlReturn Failed\r\n");
						}
				}
				Cli_DestroyServer(srv);
				srv = NULL;
		}
		return true;
}




static void	client_io_thread_func(void *data)
{
		SOCKET garbage_fd;
		fd_set rd_set, wd_set, ex_set;
		size_t rd_cnt, wd_cnt, ex_cnt;
		struct timeval tv;
		uint_64_t	time_mark = Com_GetTime_Milliseconds();



		Com_UNUSED(data);

/********************************************************************/
		garbage_fd = socket(AF_INET, SOCK_STREAM, 0);
		
		if(garbage_fd == INVALID_SOCKET)
		{
				Com_error(COM_ERR_FATAL, L"Client Internal Error : error code == %d\r\n", WSAGetLastError());
				return; /*避免警告*/
		}

		

		while(__g_is_started)
		{
				int sel_ret = 0;
				size_t i;
				struct fd_set *prd, *pwd, *pex;

				FD_ZERO(&rd_set);
				FD_ZERO(&wd_set);
				FD_ZERO(&ex_set);
				rd_cnt = 0;
				wd_cnt = 0; 
				ex_cnt = 0;
				
				
				/*printf("dump\r\n");*/
				
				Com_LockMutex(&__g_srv_mtx);
				for(i = 0; i < CLI_DIR_MAX; ++i)
				{
						cliSrv_t *srv = __g_srv_set[i];
						if(!srv)
						{
								continue;
						}

						FD_SET(srv->sockfd, &rd_set);
						rd_cnt++;
						
						if(Cli_HasDataToSend(srv))
						{
								FD_SET(srv->sockfd, &wd_set);
								wd_cnt++;
						}
				}
				Com_UnLockMutex(&__g_srv_mtx);


				if(rd_cnt == 0 && wd_cnt == 0 && ex_cnt == 0)
				{
						FD_SET(garbage_fd, &rd_set);
						rd_cnt++;
				}

				prd = rd_cnt > 0 ? &rd_set : NULL;
				pwd = wd_cnt > 0 ? &wd_set : NULL;
				pex = ex_cnt > 0 ? &ex_set : NULL;

				tv.tv_sec = 0;
				tv.tv_usec = TICKCOUNT_RESOLUTION * 1000;
				
				sel_ret = select(0, prd, pwd, pex, &tv);

				if(sel_ret == SOCKET_ERROR)
				{
						Com_error(COM_ERR_WARNING, L"Internal error : select failed (%d)\r\n", WSAGetLastError());
						continue;
				}
				
				if(Com_GetTime_Milliseconds() - time_mark >= TIMER_RESULTION)
				{
						Com_LockMutex(&__g_srv_mtx);
						for(i = 0; i < CLI_DIR_MAX; ++i)
						{
								cliSrv_t *srv = __g_srv_set[i];
								if(!srv)
								{
										continue;
								}
								
								if(!Cli_OnTimer(srv))
								{
										//Com_printf("Timeout Cli_RemoveServer Cli_OnTimer \r\n");
										Cli_RemoveServer(srv->dir);
								}
						}
						Com_UnLockMutex(&__g_srv_mtx);
						time_mark = Com_GetTime_Milliseconds();
				}
				
				if(sel_ret == 0)
				{
						continue;
				}else
				{
						size_t k;
						Com_LockMutex(&__g_srv_mtx);
						
						for(i = 0; i < wd_set.fd_count; ++i)
						{
								for(k = 0; k < CLI_DIR_MAX; ++k)
								{
										cliSrv_t *srv = __g_srv_set[k];
										if(!srv)
										{
												continue;
										}

										if(srv->sockfd == wd_set.fd_array[i])
										{
												if(!Cli_SendData(srv))
												{
														Cli_RemoveServer(srv->dir);
												}
										}
								}
						}



						for(i = 0; i < rd_set.fd_count; ++i)
						{
								if(rd_set.fd_array[i] == garbage_fd)
								{
										Com_error(COM_ERR_FATAL, L"Internal Error : garbage_fd error\r\n");
										continue;
								}

								for(k = 0; k < CLI_DIR_MAX; ++k)
								{
										cliSrv_t *srv = __g_srv_set[k];
										if(!srv)
										{
												continue;
										}

										if(srv->sockfd == rd_set.fd_array[i])
										{
												if(!Cli_RecvData(srv))
												{
														Cli_RemoveServer(srv->dir);
												}
										}
								}
						}



						
						for(i = 0; i < ex_set.fd_count; ++i)
						{
								Com_ASSERT(false);/*不可能执行到此*/
						}
						
						Com_UnLockMutex(&__g_srv_mtx);
				}
		}
		
		closesocket(garbage_fd);
		garbage_fd = INVALID_SOCKET;
}





static bool_t hook_dispatch(const hkCliDispatchEntryParam_t *parameter)
{
		Com_ASSERT(parameter != NULL && parameter->ctx != NULL);
		
		switch(parameter->event)
		{
		case HK_EVENT_ENTER:
				Com_printf(L"enter %s\r\n", ((cliSrv_t*)parameter->ctx)->dir == CLI_LEFT_SRV ? L"Left Computer" : L"Right Computer");
				return Cli_SendEnterMsg((cliSrv_t*)parameter->ctx, &parameter->enter_evt);
				break;
		case HK_EVENT_MOUSE:
				Com_printf(L"On HK_EVENT_MOUSE (msg == %d, x : %d, y : %d, data : %d)\r\n", parameter->mouse_evt.msg, parameter->mouse_evt.x, parameter->mouse_evt.y, parameter->mouse_evt.data);
				return Cli_SendMouseMsg((cliSrv_t*)parameter->ctx, &parameter->mouse_evt);
				break;
		case HK_EVENT_KEYBOARD:
				return Cli_SendKeyboardMsg((cliSrv_t*)parameter->ctx, &parameter->keyboard_evt);
				break;
		default:
				Com_ASSERT(false);/*不可达*/
				return false;
				break;
		}


}

#endif






MM_NAMESPACE_END
