#include "srv_hook.h"


MM_NAMESPACE_BEGIN



static bool_t			  __g_is_started = false;

static OnMouseEventFunc_t __g_mouse_handler = NULL;
static cmThread_t		  *__g_hook_thread = NULL;
static size_t			__g_hook_thread_id = 0;
static HHOOK			__g_mouse_hook = NULL;


#define WM_USER_QUIT_HOOK_THREAD_MSG_ID		0x12345

static void hook_thread_func(void *data);


bool_t	Hook_Srv_Start(OnMouseEventFunc_t func)
{
		cmEvent_t		*event;
		Com_ASSERT(func != NULL);

		if(Hook_Srv_IsStarted())
		{
				Com_ASSERT(false);
				return false;
		}


		__g_is_started = true;
		__g_mouse_handler = func;
		
		__g_mouse_hook = NULL;
		event = Com_CreateEvent(false);
		__g_hook_thread = Com_CreateThread(hook_thread_func, (void*)event, &__g_hook_thread_id);
		
		Com_WaitEvent(event);
		Com_DestroyEvent(event);
		event = NULL;

		return true;
}

bool_t	Hook_Srv_Stop()
{
		if(!Hook_Srv_IsStarted())
		{
				Com_ASSERT(false);
				return false;
		}
		
		PostThreadMessage((DWORD)__g_hook_thread_id, (UINT)WM_USER_QUIT_HOOK_THREAD_MSG_ID, (WPARAM)NULL, (LPARAM)NULL);

		Com_JoinThread(__g_hook_thread);
		Com_CloseThread(__g_hook_thread);
		__g_hook_thread = NULL;
		__g_hook_thread_id = 0;
		__g_mouse_handler = NULL;
		__g_mouse_hook = NULL;
		__g_is_started = false;
		return true;
		
}

bool_t	Hook_Srv_IsStarted()
{
		return __g_is_started;
}




static LRESULT CALLBACK mouse_hook_func(int code, WPARAM w, LPARAM l)
{

		
		
		if(code != HC_ACTION)
		{
				return CallNextHookEx(__g_mouse_hook, code, w, l);
		}else
		{
				if(__g_mouse_handler)
				{
						if(__g_mouse_handler((size_t)w, (const MSLLHOOKSTRUCT*)l))
						{
								return CallNextHookEx(__g_mouse_hook, code, w, l);
						}else
						{
								return 1;
						}
				}else
				{
						return CallNextHookEx(__g_mouse_hook, code, w, l);
				}
		}

}







static void hook_thread_func(void *data)
{
		MSG msg;
		cmEvent_t *completion_event = (cmEvent_t*)data;
		Com_ASSERT(completion_event != NULL);
		
		__g_mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)&mouse_hook_func,GetModuleHandle(NULL), 0);

		if(__g_mouse_hook == NULL)
		{
				Com_error(COM_ERR_FATAL, L"Hook mouse failed : error code (%d)\r\n", GetLastError());
				return;
		}
		
		Com_SetEvent(completion_event);


		while (GetMessage(&msg, NULL, 0, 0))
		{
				if(msg.message == WM_USER_QUIT_HOOK_THREAD_MSG_ID)
				{
						break;
				}
				TranslateMessage (&msg);
				DispatchMessage (&msg);
		};
		
		UnhookWindowsHookEx(__g_mouse_hook);
		__g_mouse_hook = NULL;
}


MM_NAMESPACE_END

