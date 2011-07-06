

#include "test.h"

#if(1)
#include "Client/client.h"
#pragma comment(lib, "Client.lib")


void Client_Test()
{
		cliInit_t init;
		init.ctx = NULL;
		init.hide_cursor = LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(101));
		init.on_notify = NULL;

		Cli_Init(&init);
		
		Cli_Start();
		
		//Cli_InsertServer(NM_POS_LEFT, L"192.168.1.4", 8412);
		Cli_InsertServer(NM_POS_RIGHT, L"192.168.1.124", 8412);
		
		getchar();

		Cli_Stop();

		Cli_UnInit();
}

#endif