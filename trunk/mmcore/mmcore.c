



#include "mmcore.h"




MM_NAMESPACE_BEGIN




static bool_t	__g_is_client_mode = true;


bool_t	MM_Init(const nsInit_t *init)
{
		cmInit_t cm_init;
		Com_ASSERT(init != NULL);
		cm_init.io_ctx.ctx = init->ctx;
		cm_init.io_ctx.on_error = init->on_error;
		cm_init.io_ctx.on_print = init->on_print;
		__g_is_client_mode = init->is_client;
		if(!Com_Init(&cm_init))
		{
				return false;
		}


		if(__g_is_client_mode)
		{
				Cli_Init(NULL);
		}else
		{
				Srv_Init(NULL);
		}

		return true;
}


bool_t	MM_UnInit()
{

		if(__g_is_client_mode)
		{
				Cli_UnInit();
		}else
		{
				Srv_UnInit();
		}

		if(!Com_UnInit())
		{
				return false;
		}
		

		return true;
}


MM_NAMESPACE_END
