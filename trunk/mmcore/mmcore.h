#ifndef __NS_UTILITY_H__
#define __NS_UTILITY_H__


#include "./common/common.h"
#include "./Client/client.h"
#include "./Server/server.h"

#pragma comment(lib, "Ws2_32.lib")
/*#pragma comment(lib, "Mpr.lib")*/
/*#pragma comment(lib, "Iphlpapi.lib")*/
/*#pragma comment(lib, "shlwapi.lib")*/

MM_NAMESPACE_BEGIN








typedef struct	__netscan_init_tag		
{
		Com_print_func_t		on_print;
		Com_error_func_t		on_error;
		void					*ctx;
		bool_t					is_client;
}nsInit_t;


bool_t	MM_Init(const nsInit_t *init);
bool_t	MM_UnInit();





MM_NAMESPACE_END




#endif
