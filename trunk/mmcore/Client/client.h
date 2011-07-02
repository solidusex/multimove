#ifndef __MMCORE_CLIENT_H__
#define __MMCORE_CLIENT_H__

#include "../Common/common.h"
#include "../NetMsg/netmsg.h"

MM_NAMESPACE_BEGIN


struct __client_init_tag;
typedef struct __client_init_tag	cliInit_t;

bool_t Cli_Init(const cliInit_t *init);
bool_t Cli_UnInit();





bool_t	Cli_Start();
bool_t	Cli_Stop();


typedef enum
{
		CLI_LEFT_SRV = 0x00,
		CLI_RIGHT_SRV,

		CLI_DIR_MAX
}cliServerDir_t;




bool_t	Cli_InsertServer(cliServerDir_t dir, const wchar_t *srv_ip, uint_16_t port);
bool_t	Cli_RemoveServer(cliServerDir_t dir);







MM_NAMESPACE_END


#endif
