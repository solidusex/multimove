

#include <stdio.h>
#include <locale.h>
#include <assert.h>







#include "test.h"



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
		nsInit_t init;
		init.ctx = NULL;
		init.on_error = default_error;
		init.on_print = default_print;
		init.is_client = true;


		printf("current locale == %ls\r\n", setlocale(LC_ALL,NULL));

		MM_Init(&init);
		
		//Common_Test();
		//Hook_Test();
		//Client_Test();
		Server_Test();

		MM_UnInit();

#if (WINVER >= 0x401)
		#if defined(DEBUG_FLAG)
				_CrtDumpMemoryLeaks();
		#endif
#endif
		
		printf("done\r\n");
		

		getchar();
		return 0;

}



