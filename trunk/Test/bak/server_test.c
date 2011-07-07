#include "test.h"


#if(0)
#include "Server/server.h"

#include "Server/server.h"
#pragma comment(lib, "Server.lib")

void Server_Test()
{
		Srv_Init(NULL);
		
		Srv_Start(NULL, 8412);
		
		
		getchar();
		Srv_Stop();

		Srv_UnInit();
}

#endif

