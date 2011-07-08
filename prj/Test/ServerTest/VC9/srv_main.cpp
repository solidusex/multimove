

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
		init.on_notify = NULL;
		printf("current locale == %ls\r\n", setlocale(LC_ALL,NULL));

		Com_Init(&cm_init);
		Srv_Init(&init);

		Srv_Start(NULL, 8412);


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



