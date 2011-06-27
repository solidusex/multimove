
#include "cli_hook.h"
#include "client.h"


MM_NAMESPACE_BEGIN



bool_t Cli_Init(const cliInit_t *init)
{
		Com_UNUSED(init);
		Hook_Cli_Init(NULL);
		return true;
}

bool_t Cli_UnInit()
{
		Hook_Cli_UnInit();
		return true;
}




/*


����Э�飬���²���Client->Server����Server->Client����Ϊ�����ֽ���

1. Client -> Server:
[0-4)�ֽ� ������
[4-6)�ֽ� ������ ��ΪKeepAlive = 0, HandShake = 1, MouseEnter = 2, MouseEvent = 3, KeyboardEvent = 4

Content:

KeeyAlive:		Content length == 0 �ֽ� 
				���ܳ���Ϊ6�ֽ�


HandShake:		Content length == 1�ֽ�		
				[6-7)�ֽ�	direction	byte_t	���� LEFT == 0, RIGHT == 1
				���ܳ���Ϊ7�ֽ�

MouseEnter:		Content length == 16�ֽ�
				[6-10)�ֽ� : src_x_fullscreen	:  uint_32_t; Դ��Ļ���
				[10-14)�ֽ� : src_x_fullscreen	:  uint_32_t; Դ��Ļ�߶�
				[14-18)�ֽڣ� x				:  int_32_t ; ���x������
				[18-22)�ֽڣ� y				:  int_32_t ; ���y������

MouseEvent:		Content length == 16�ֽ�		
		[6- 10)�ֽ�:  msg;				uint_32_t ��Ϣ����
		[10- 14)�ֽ�: x;				int_32_t  ���x������
		[14-18)�ֽ�:  y;				int_32_t  ���y������
		[18-22)�ֽ�:  data;				int_32_t  �������ݣ��������ƫ��


KeyboardEvent:	Content length == 3�ֽ�			
		[6-7)�ֽڣ� vk;				byte_t			���������
		[7-8)�ֽ�:	 scan;				byte_t			ɨ����
		[8-9)�ֽ�:	 is_keydown			bool_t			�Ƿ�Ϊ��������

		���ܳ���Ϊ9�ֽ�



2. Server -> Client:
[0-4)�ֽ�  ������
[4-6)�ֽ�  ������, ��ΪKeepAlive = 0, HandShake = 1, MouseLeave = 2

Content:

KeeyAlive:		Content length == 0�ֽ�

HandShake:		Content length == 0�ֽ�

MouseLeave:		Content length == 0�ֽ�




*/



typedef enum
{
		CLI_RECV_WAIT_HEADER,
		CLI_RECV_WAIT_PACKAGE
}cliRecvState_t;

#define KEEPALIVE_TIMEOUT				10 * 1000		/*10����û���κ�socket�ϵ�IO��������Ϊ�����ӹر�*/
#define TICKCOUNT_RESOLUTION			20				/*20����*/
#define TIMER_RESULTION					3  * 1000		/*3����һ��*/

typedef struct __client_server_tag
{
		cliServerDir_t	dir;
		wchar_t			*ip;
		uint_16_t		port;
		bool_t			is_active_side;

		SOCKET			sockfd;
		
		cmBuffer_t		*in_buf;
		cmMutex_t		in_lock;
		/*CLI_RECV_WAIT_HEADER ����£�remain_len��ʼֵΪ4, CLI_RECV_WAIT_PACKAGE��ʼֵΪheader���õ��ð�ͷ*/		
		cliRecvState_t	recv_state;		
		size_t			remain_len;
		uint_64_t		last_recv_tm;

		

		cmBuffer_t		*out_buf;
		cmMutex_t		out_lock;
		uint_64_t		last_send_tm;
}cliSrv_t;



/*������Ϣ*/

#define CLI_CONNECT_TO_SRV_TIMEOUT		3000

static cliSrv_t*	Cli_CreateServer(cliServerDir_t	dir, const wchar_t			*ip, uint_16_t		port); /*����������������*/
static void			Cli_DestroyServer(cliSrv_t *srv);
static bool_t		Cli_IsActiveSide(const cliSrv_t *srv);

static bool_t		Cli_HasDataToSend(cliSrv_t *srv);/*out_buf�Ƿ��������*/

static bool_t		Cli_RecvData(cliSrv_t *srv);/*��sockfd�Է�������ʽ���նԶ����ݣ�����in_buf����,���ﵽһ�������������Cli_HandleRecvBuffer*/
static bool_t		Cli_SendData(cliSrv_t *srv);/*��out_buf�����÷�������ʽ���ͳ�ȥ*/
static bool_t		Cli_OnTimer(cliSrv_t *srv); /*������Ϊ��Ҫ������������false���__g_srv_setɾ����*/

/*����*/
static bool_t		Cli_SendKeepAlive(cliSrv_t *srv);
static bool_t		Cli_SendHandShake(cliSrv_t *srv, cliServerDir_t dir);
static bool_t		Cli_SendMouseMsg(cliSrv_t *srv, const hkMouseEvent_t *mouse_msg);
static bool_t		Cli_SendKeyboardMsg(cliSrv_t *srv, const hkKeyboardEvent_t *keyboard);
static bool_t		Cli_SendEnterMsg(cliSrv_t *srv, const hkEnterEvent_t     *enter);
/*����*/
static bool_t		Cli_HandleRecvBuffer(cliSrv_t *srv, const byte_t *data, size_t length);





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



/*����*/
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

		int_32_t		x;		/*���x������*/
		int_32_t		y;		/*���y������*/

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
		
		uint_32_t		msg;	/* ��Ϣ����*/
		int_32_t		x;		/*���x������*/
		int_32_t		y;		/*���y������*/
		uint_32_t		data;	/*�������ݣ��������ƫ��*/


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
		[6-7)�ֽڣ� vk;				byte_t			
		[7-8)�ֽ�:	 scan;				byte_t			
		[8-9)�ֽ�:	 is_keydown			bool_t			
		*/


		uint_32_t package_len;
		uint_16_t package_type;
		
		byte_t			vk;		/* ���������*/
		byte_t			scan;	/*ɨ����*/
		byte_t			is_keydown; /*�Ƿ�Ϊ��������*/

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


/*����*/
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
				return true;
		case 1: /*handshake reply*/
				return true;
		case 2:/*mouse leave*/
				Com_ASSERT(srv->is_active_side);
				srv->is_active_side = false;
				Hook_Cli_ControlReturn();
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

		if(ioctlsocket(srv->sockfd, FIONREAD, &available) != 0 || available <= 0)
		{
				Com_error(COM_ERR_WARNING, L"Cli_RecvData : ioctlsocket error\r\n");
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

								if(package_len > 1 * COM_KB || package_len < 2)/*��������С*/
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
						Com_ASSERT(false);/*���ɴ�*/
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
				is_ok = false;/*������ҽ���select�У���һ�������ݴ�����*/
				goto END_POINT;
		}

		p = Com_GetBufferData(srv->out_buf);

		wn = send(srv->sockfd, (const char*)p, (int)available, 0);

		if(wn <= 0)
		{
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
				return false;
		}
		

		if(Com_GetTime_Milliseconds() - srv->last_send_tm > KEEPALIVE_TIMEOUT - 5000)
		{
				Cli_SendKeepAlive(srv);
		}
		
		return true;
}



/************************************************************************************************************************************/



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
				return; /*���⾯��*/
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
								Com_ASSERT(false);/*������ִ�е���*/
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
				return Cli_SendEnterMsg((cliSrv_t*)parameter->ctx, &parameter->enter_evt);
				break;
		case HK_EVENT_MOUSE:
				return Cli_SendMouseMsg((cliSrv_t*)parameter->ctx, &parameter->mouse_evt);
				break;
		case HK_EVENT_KEYBOARD:
				return Cli_SendKeyboardMsg((cliSrv_t*)parameter->ctx, &parameter->keyboard_evt);
				break;
		default:
				Com_ASSERT(false);/*���ɴ�*/
				return false;
				break;
		}


}








MM_NAMESPACE_END
