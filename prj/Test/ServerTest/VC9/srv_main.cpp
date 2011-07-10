

#include <stdio.h>
#include <locale.h>
#include <assert.h>


#include "Server/server.h"
#pragma comment(lib, "Server.lib")



void __stdcall default_error(int_t level, const wchar_t* msg, void *ctx)
{
		//printf("%ls", msg);
		assert(msg != NULL);
		//::MessageBoxA(NULL, msg, "default_error", 0);

		printf("error msg == %ls", msg);
}

void __stdcall default_print(const wchar_t *msg, void *ctx)
{
		assert(msg != NULL);
		//::MessageBoxA(NULL, msg, "default_printf", 0);
		printf("%ls", msg);
}


void on_srv_notify(void *ctx, const srvNotify_t	*notify)
{
		switch(notify->t)
		{
		case SRV_NOTIFY_ON_LISTEN:
		{
				printf("on listen:\r\n");
				for(size_t i = 0; i < notify->on_listen.bind_ip_cnt; ++i)
				{
						printf("%ls:%d\r\n", notify->on_listen.bind_ip[i].ip, notify->on_listen.listen_port);
				}
				printf("\r\n");
		}
				break;
		case SRV_NOTIFY_ON_LOGIN:
		{
				printf("%ls:%d Login\r\n", notify->on_login.remote_ip, notify->on_login.remote_port);
		}
				break;
		case SRV_NOTIFY_ON_LOGOFF:
		{
				printf("%ls:%d LogOff\r\n", notify->on_logoff.remote_ip, notify->on_logoff.remote_port);
		}
				break;
		case SRV_NOTIFY_ON_ENTER:
		{
				printf("%ls:%d Enter\r\n", notify->on_enter.remote_ip, notify->on_enter.remote_port);
		}
				break;
		case SRV_NOTIFY_ON_LEAVE:
		{
				printf("%ls:%d Leave\r\n", notify->on_leave.remote_ip, notify->on_leave.remote_port);
		}
				break;
		case SRV_NOTIFY_ON_CLIPDATA:
		{
				printf("%ls:%d Received clipboard data\r\n", notify->on_recv_clipdata.remote_ip, notify->on_recv_clipdata.remote_port);
		}
				break;
		default:
				Com_ASSERT(false);
				break;
		}
}

int main(int argc, char **argv)
{
		ioCtx_t	ctx;
		ctx.on_error = default_error;
		ctx.on_print = default_print;
		ctx.ctx = NULL;

		cmInit_t cm_init;
		cm_init.io_ctx = ctx;
		
		srvInit_t		init;
		init.ctx = NULL;
		init.on_notify = on_srv_notify;
		printf("current locale == %ls\r\n", setlocale(LC_ALL,NULL));

		Com_Init(&cm_init);
		Srv_Init(&init);

		//Srv_Start(NULL, 8412);

		if(!Srv_Start(NULL,1068, 8500))
		{
				abort();
		}


		getchar();

		Srv_Stop();

		Srv_UnInit();
		Com_UnInit();

#if (WINVER >= 0x401)
		#if defined(DEBUG_FLAG)
				_CrtDumpMemoryLeaks();
		#endif
#endif
		
		printf("done\r\n");
		

		getchar();
		return 0;

}



