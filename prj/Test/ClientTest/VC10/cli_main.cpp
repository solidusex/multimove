

#include <stdio.h>
#include <locale.h>
#include <assert.h>

#include "resource.h"
#include "Client/client.h"
#pragma comment(lib, "Client.lib")



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

void client_notify(void *ctx, const cliNotify_t	*notify)
{

		Com_ASSERT(notify != NULL);

		switch(notify->t)
		{
		case CLI_NOTIFY_ON_CONNECTED:
		{
				printf("server %ls:%d connected\r\n",notify->on_connected.ip, notify->on_connected.port);
		}
				break;
		case CLI_NOTIFY_ON_DISCONNECTED:
		{
				printf("server %ls:%d disconnected\r\n",notify->on_disconnected.ip, notify->on_disconnected.port);
		}
				break;
		case CLI_NOTIFY_ON_ACTIVE:
		{
				printf("server %ls:%d active\r\n",notify->on_active.ip, notify->on_active.port);
		}				
				break;
		case CLI_NOTIFY_ON_DEACTIVE:
		{
				printf("server %ls:%d deactive\r\n",notify->on_deactive.ip, notify->on_deactive.port);
		}
				break;
		case CLI_NOTIFY_ON_CLIPDATA:
		{
				printf("received server %ls:%d clipdata\r\n",notify->on_clipdata.ip, notify->on_clipdata.port);
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

		cliInit_t init;
		init.ctx = NULL;
		init.hide_cursor = ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_CURSOR_BLANK));
		init.on_notify = client_notify;

		printf("current locale == %ls\r\n", setlocale(LC_ALL,NULL));

		Com_Init(&cm_init);
		Cli_Init(&init);

		Cli_Start();

		Cli_InsertServer(NM_POS_RIGHT, L"193.168.19.179", 8412);

		getchar();

		Cli_Stop();

		Cli_UnInit();
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



