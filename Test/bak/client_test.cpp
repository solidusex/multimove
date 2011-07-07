


#include "test.h"


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>





void Client_Test()
{
		//hook_display_test();

		if(!Cli_Start())
		{
				Com_ASSERT(false);
		}
		
		printf("Cli_Start\r\n");
		getchar();


		if(!Cli_Stop())
		{
				Com_ASSERT(false);
		}

		printf("Cli_Stop\r\n");

}
