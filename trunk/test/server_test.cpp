


#include "test.h"


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mmcore.h"




void Server_Test()
{
		//hook_display_test();

		if(!Srv_Start(NULL, 8412))
		{
				Com_ASSERT(false);
		}
		
		printf("Srv_Start\r\n");
		getchar();


		if(!Srv_Stop())
		{
				Com_ASSERT(false);
		}

		printf("Srv_Stop\r\n");

}
