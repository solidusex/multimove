#ifndef __MMCORE_HOOK_CLIENT_H__
#define __MMCORE_HOOK_CLIENT_H__

#include "../Common/common.h"

MM_NAMESPACE_BEGIN



struct __hook_client_init_tag;
typedef struct __hook_client_init_tag	hkCliInit_t;


bool_t	Hook_Cli_Init(const hkCliInit_t *init);
bool_t	Hook_Cli_UnInit();




typedef enum
{
		HK_DIR_LEFT = 0x00,
		HK_DIR_RIGHT,


		HK_DIR_MAX,
}hkCliDirection_t;


typedef enum
{
		HK_EVENT_MOUSE,
		HK_EVENT_KEYBOARD
}hkCliEvent_t;


typedef struct __mouse_event_tag
{
		uint_32_t	msg;
		int_32_t	x;
		int_32_t	y;
		int_32_t	data; /*¹öÂÖÆ«ÒÆ*/
		bool_t		is_first_msg;
}hkMouseEvent_t;		


typedef struct __keyboard_event_tag
{
		byte_t			vk;
		byte_t			scan;
		bool_t			is_keydown;
}hkKeyboardEvent_t;

typedef struct __hook_client_dispatch_entry_param_tag
{
		hkCliEvent_t			event;
		void					*ctx;
		
		union{
				hkMouseEvent_t		mouse_evt;
				hkKeyboardEvent_t	keyboard_evt;
		};
}hkCliDispatchEntryParam_t;


typedef bool_t	(*hkCliDispatchFunc_t)(const hkCliDispatchEntryParam_t *parameter);


bool_t	Hook_Cli_Start();
bool_t	Hook_Cli_Stop();
bool_t	Hook_Cli_IsStarted();

bool_t	Hook_Cli_RegisterDispatch(hkCliDirection_t		dir, void	*ctx, hkCliDispatchFunc_t		dispatch);
bool_t	Hook_Cli_UnRegisterDispatch(hkCliDirection_t		dir);

bool_t	Hook_Cli_ControlReturn();










MM_NAMESPACE_END


#endif