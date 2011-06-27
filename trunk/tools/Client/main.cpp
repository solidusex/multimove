

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <assert.h>


#include "mmcore.h"
#pragma comment(lib, "mmcore.lib")




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




int main()
{

		mmInit_t init;
		init.ctx = NULL;
		init.on_error = default_error;
		init.on_print = default_print;
		init.is_client = true;

		setlocale(LC_ALL,NULL);

		MM_Init(&init);
		
		Cli_Start();

		//Cli_InsertServer(cliDirection_t

		getchar();
		Cli_Stop();



		MM_UnInit();

#if defined(_DEBUG)
		_CrtDumpMemoryLeaks();
#endif
		

		printf("done\r\n");
		getchar();
		return 0;
}