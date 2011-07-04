
#include "cli_hook.h"
#include "client.h"
#include "cli_session.h"

MM_NAMESPACE_BEGIN



bool_t Cli_Init(const cliInit_t *init)
{
		Com_UNUSED(init);
		Com_printf(L"On Cli_Init\r\n");
		return true;
}

bool_t Cli_UnInit()
{
		Com_printf(L"On Cli_UnInit\r\n");
		return true;
}




/************************************************************************************************************************************/



static void	client_io_thread_func(void *data);
static bool_t hook_dispatch(const nmMsg_t *msg, void *ctx);

static cmMutex_t		__g_ss_mtx;
static ss_t				*__g_ss_set[NM_POS_MAX];
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

		Com_InitMutex(&__g_ss_mtx);
		Com_memset(__g_ss_set, 0, sizeof(__g_ss_set));
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



		for(i = 0; i < NM_POS_MAX; ++i)
		{
				if(__g_ss_set[i] != NULL)
				{
						Hook_Cli_UnRegisterHandler((nmPosition_t)i);
						SS_CloseSession(__g_ss_set[i]);
						__g_ss_set[i] = NULL;
				}
		}

		if(!Hook_Cli_Stop())
		{
				Com_ASSERT(false);
				return false;
		}
		

		Com_memset(__g_ss_set, 0, sizeof(__g_ss_set));
		Com_UnInitMutex(&__g_ss_mtx);

		return true;
}





bool_t	Cli_InsertServer(nmPosition_t pos, const wchar_t *srv_ip, uint_16_t port)
{
		bool_t has_srv;
		ss_t *ss;
		Com_ASSERT(srv_ip != NULL);


		if(!Cli_IsStarted())
		{
				Com_ASSERT(false);
				return false;
		}

		Com_LockMutex(&__g_ss_mtx);
		has_srv = __g_ss_set[pos] != NULL;
		Com_UnLockMutex(&__g_ss_mtx);

		if(has_srv)
		{
				Com_error(COM_ERR_WARNING, L"Can't insert multi server to single position\r\n");
				return false;
		}


		ss = SS_ConnectSession(pos, srv_ip, port);

		if(ss == NULL)
		{
				return false;
		}

		Com_LockMutex(&__g_ss_mtx);
		__g_ss_set[pos] = ss;

		if(!Hook_Cli_RegisterHandler(pos, (void*)ss, hook_dispatch))
		{
				Com_error(COM_ERR_FATAL, L"Internal Error : Hook_Cli_RegisterDispatch Failed\r\n");
		}
		Com_UnLockMutex(&__g_ss_mtx);
		return true;
}



bool_t	Cli_RemoveServer(nmPosition_t pos)
{
		ss_t *ss;
		Com_ASSERT(pos < NM_POS_MAX);

		if(!Cli_IsStarted())
		{
				Com_ASSERT(false);
				return false;
		}

		

		Com_LockMutex(&__g_ss_mtx);

		
		if(!Hook_Cli_UnRegisterHandler(pos))
		{
				Com_error(COM_ERR_FATAL, L"Internal Error : Hook_Cli_UnRegisterHandler Failed\r\n");
		}
		
		ss = __g_ss_set[pos];
		__g_ss_set[pos] = NULL;

		
		Com_UnLockMutex(&__g_ss_mtx);

		if(ss)
		{
				if(SS_IsActive(ss))
				{
						if(!Hook_Cli_ControlReturn())
						{
								Com_error(COM_ERR_FATAL, L"Internal Error : Hook_Cli_ControlReturn Failed\r\n");
						}
				}
				SS_CloseSession(ss);
				ss = NULL;
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
				
				Com_LockMutex(&__g_ss_mtx);
				for(i = 0; i < NM_POS_MAX; ++i)
				{
						ss_t *ss = __g_ss_set[i];
						if(!ss)
						{
								continue;
						}

						FD_SET(ss->sockfd, &rd_set);
						rd_cnt++;
						
						if(SS_HasDataToSend(ss))
						{
								FD_SET(ss->sockfd, &wd_set);
								wd_cnt++;
						}
				}
				Com_UnLockMutex(&__g_ss_mtx);


				if(rd_cnt == 0 && wd_cnt == 0 && ex_cnt == 0)
				{
						FD_SET(garbage_fd, &rd_set);
						rd_cnt++;
				}

				prd = rd_cnt > 0 ? &rd_set : NULL;
				pwd = wd_cnt > 0 ? &wd_set : NULL;
				pex = ex_cnt > 0 ? &ex_set : NULL;

				tv.tv_sec = 0;
				tv.tv_usec = NM_BLOCKING_TIMEOUT * 1000;
				
				sel_ret = select(0, prd, pwd, pex, &tv);

				if(sel_ret == SOCKET_ERROR)
				{
						Com_error(COM_ERR_FATAL, L"Internal error : select failed (%d)\r\n", WSAGetLastError());
						/*continue;*/
				}
				
				if(Com_GetTime_Milliseconds() - time_mark >= NM_TIMER_TICK)
				{
						Com_LockMutex(&__g_ss_mtx);
						for(i = 0; i < NM_POS_MAX; ++i)
						{
								ss_t *ss = __g_ss_set[i];
								if(!ss)
								{
										continue;
								}
								
								if(!SS_OnTimer(ss))
								{
										//Com_printf("Timeout Cli_RemoveServer Cli_OnTimer \r\n");
										Cli_RemoveServer(ss->for_position);
								}
						}
						Com_UnLockMutex(&__g_ss_mtx);
						time_mark = Com_GetTime_Milliseconds();
				}
				
				if(sel_ret == 0)
				{
						continue;
				}else
				{
						size_t k;
						Com_LockMutex(&__g_ss_mtx);

						for(k = 0; k < NM_POS_MAX; ++k)
						{
								ss_t *ss = __g_ss_set[k];

								if(ss)
								{
										if(FD_ISSET(ss->sockfd, &wd_set))
										{
												if(!SS_SendData(ss))
												{
														Cli_RemoveServer(ss->for_position);
												}
										}
								}
						}

						for(k = 0; k < NM_POS_MAX; ++k)
						{
								ss_t *ss = __g_ss_set[k];

								if(ss)
								{
										if(FD_ISSET(ss->sockfd, &rd_set))
										{
												if(!SS_RecvData(ss))
												{
														Cli_RemoveServer(ss->for_position);
												}
										}
								}

						}

						
						Com_UnLockMutex(&__g_ss_mtx);
				}
		}
		
		closesocket(garbage_fd);
		garbage_fd = INVALID_SOCKET;
}







static bool_t hook_dispatch(const nmMsg_t *msg, void *ctx)
{
		Com_ASSERT(ctx != NULL && msg != NULL);
		//Com_printf(L"On hook_dispatch\r\n");

		switch(msg->t)
		{
		case NM_MSG_KEEPALIVE:
		case NM_MSG_HANDSHAKE:
		case NM_MSG_HANDSHAKE_REPLY:
		case NM_MSG_LEAVE:
		default:
				Com_ASSERT(false);/*不可达*/
				return false;
				break;
		case NM_MSG_ENTER:
				return SS_SendEnterMsg((ss_t*)ctx, msg);
		case NM_MSG_MOUSE:
				return SS_SendMouseMsg((ss_t*)ctx, msg);
		case NM_MSG_KEYBOARD:
				return SS_SendKeyboardMsg((ss_t*)ctx, msg);
		}
}






MM_NAMESPACE_END
