#ifndef __MMCORE_HOOK_SERVER_H__
#define __MMCORE_HOOK_SERVER_H__

#include "../Common/common.h"

MM_NAMESPACE_BEGIN


struct __hook_server_init_tag;
typedef struct __hook_server_init_tag	hkSrvInit_t;

bool_t Hook_Srv_Init(const hkSrvInit_t *init);
bool_t Hook_Srv_UnInit();



typedef bool_t (*OnMouseEventFunc_t)(size_t msg_id, const MSLLHOOKSTRUCT *mouse_stu);


bool_t	Hook_Srv_Start(OnMouseEventFunc_t func);
bool_t	Hook_Srv_Stop();
bool_t	Hook_Srv_IsStarted();









MM_NAMESPACE_END


#endif
