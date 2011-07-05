
#include "Client/client.h"
#pragma comment(lib, "Ws2_32.lib")









void __stdcall default_error(int_t level, const wchar_t* msg, void *ctx)
{
		assert(msg != NULL);
		printf("error msg == %ls", msg);
		
		if(level == COM_ERR_FATAL)
		{
				abort();
		}
}

void __stdcall default_print(const wchar_t *msg, void *ctx)
{
		assert(msg != NULL);
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

		Com_Init(&cm_init);
		Cli_Init(NULL);
		

		Cli_Start();
		
		while(true)
		{
				Cli_InsertServer(NM_POS_LEFT, L"193.168.19.179", 8412);
				getchar();
		}
		
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
