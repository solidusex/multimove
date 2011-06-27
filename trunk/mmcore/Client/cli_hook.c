
#include "cli_hook.h"




typedef enum 
{
		HK_STATE_STOP			= 0x00,
		HK_STATE_NORMAL,
		HK_STATE_REMOTE

}hkCliState_t;



struct {
		void					*ctx;
		hkCliDispatchFunc_t		dispatch;
}__g_entry[HK_DIR_MAX] = 
{
		{ NULL, NULL},
		{NULL, NULL}
};


static HHOOK	__g_mouse_hook = NULL;
static HHOOK	__g_keyboard_hook = NULL;
static size_t	__g_hook_thread_id = 0;
static hkCliState_t __g_state = HK_STATE_STOP;
static hkCliDirection_t __g_curr_dir = HK_DIR_MAX;
static cmSpinLock_t		__g_lock;
static cmThread_t		*__g_thread = NULL;

static POINT			__g_last_pt;



bool_t	Hook_Cli_Init(const hkCliInit_t *init)
{
		Com_UNUSED(init);

		Com_InitSpinLock(&__g_lock);
		__g_thread = NULL;

		Com_memset(&__g_entry, 0, sizeof(__g_entry));

		__g_mouse_hook = NULL;
		__g_keyboard_hook = NULL;
		__g_hook_thread_id = 0;
		__g_state = HK_STATE_STOP;
		__g_curr_dir = HK_DIR_MAX;
		
		
		return true;
}


bool_t	Hook_Cli_UnInit()
{
		Com_UnInitSpinLock(&__g_lock);
		__g_thread = NULL;
		
		Com_memset(&__g_entry, 0, sizeof(__g_entry));
		__g_mouse_hook = NULL;
		__g_keyboard_hook = NULL;
		__g_hook_thread_id = 0;
		__g_state = HK_STATE_STOP;
		__g_curr_dir = HK_DIR_MAX;
		return true;
}



#define WM_USER_QUIT_HOOK_THREAD_MSG_ID		0x12345

static void	hook_thread_func(void *data);




bool_t	Hook_Cli_Start()
{
		cmEvent_t *event;
		if(Hook_Cli_IsStarted())
		{
				return false;
		}
		
		__g_thread = NULL;
		Com_memset(&__g_entry, 0, sizeof(__g_entry));
		__g_mouse_hook = NULL;
		__g_keyboard_hook = NULL;
		__g_hook_thread_id = 0;
		__g_state = HK_STATE_STOP;
		__g_curr_dir = HK_DIR_MAX;


		event = Com_CreateEvent(false);
		__g_thread = Com_CreateThread(hook_thread_func, (void*)event, &__g_hook_thread_id);

		if(__g_thread == NULL)
		{
				Com_error(COM_ERR_FATAL, L"%ls\r\n", L"Hook_Cli_Start : Create Thread failed" );
				return false; //disable warning
		}

		Com_WaitEvent(event);

		Com_DestroyEvent(event);
		event = NULL;
		return true;
}





bool_t	Hook_Cli_Stop()
{
		if(!Hook_Cli_IsStarted())
		{
				return false;
		}
		
		Com_ASSERT(__g_thread != NULL);

		PostThreadMessage((DWORD)__g_hook_thread_id, (UINT)WM_USER_QUIT_HOOK_THREAD_MSG_ID, (WPARAM)NULL, (LPARAM)NULL);

		Com_JoinThread(__g_thread);
		Com_CloseThread(__g_thread);
		
		__g_thread = NULL;
		Com_memset(&__g_entry, 0, sizeof(__g_entry));
		__g_mouse_hook = NULL;
		__g_keyboard_hook = NULL;
		__g_hook_thread_id = 0;
		__g_state = HK_STATE_STOP;
		__g_curr_dir = HK_DIR_MAX;
		
		return true;
}

bool_t	Hook_Cli_IsStarted()
{
		hkCliState_t	s;
		Com_LockSpinLock(&__g_lock);
		s = __g_state;
		Com_UnLockSpinLock(&__g_lock);
		return s != HK_STATE_STOP ? true : false;
}







static LRESULT CALLBACK keyboard_hook_func(int code, WPARAM w, LPARAM l);
static LRESULT CALLBACK mouse_hook_func(int code, WPARAM w, LPARAM l);



static void	hook_thread_func(void *data)
{
		MSG msg;
		cmEvent_t *completion_event = (cmEvent_t*)data;

		Com_ASSERT(completion_event != NULL);
		Com_ASSERT(__g_hook_thread_id == GetCurrentThreadId());

		__g_keyboard_hook = SetWindowsHookEx (WH_KEYBOARD_LL, (HOOKPROC)&keyboard_hook_func, GetModuleHandle(NULL), 0);
		
		if(__g_keyboard_hook == NULL)
		{
				Com_error(COM_ERR_FATAL, L"Hook keyboard failed : error code (%d)\r\n", GetLastError());
		}

		__g_mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)&mouse_hook_func,GetModuleHandle(NULL), 0);

		if(__g_mouse_hook == NULL)
		{
				Com_error(COM_ERR_FATAL, L"Hook mouse failed : error code (%d)\r\n", GetLastError());
		}

		__g_state = HK_STATE_NORMAL;
		
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
		
		UnhookWindowsHookEx(__g_keyboard_hook);
		UnhookWindowsHookEx(__g_mouse_hook);
		
}





static LRESULT CALLBACK mouse_hook_func(int code, WPARAM w, LPARAM l)
{
		hkCliState_t s;
		int x_full_screen, y_full_screen;
		MSLLHOOKSTRUCT *mouse_stu;

		if(code != HC_ACTION)
		{
				return CallNextHookEx(__g_mouse_hook, code, w, l);
		}

		x_full_screen = GetSystemMetrics(SM_CXSCREEN);
		y_full_screen = GetSystemMetrics(SM_CYSCREEN);

		Com_LockSpinLock(&__g_lock);
		s = __g_state;
		Com_UnLockSpinLock(&__g_lock);
		
		mouse_stu = (MSLLHOOKSTRUCT*)l;
		
		switch(s)
		{
		case HK_STATE_NORMAL:
		{
				switch(w)
				{
				case WM_MOUSEMOVE:
				{
						
						

						if(mouse_stu->pt.x <= 0)
						{
								Com_printf(L"(%d:%d)\r\n", mouse_stu->pt.x, mouse_stu->pt.y);

								Com_LockSpinLock(&__g_lock);
								
								if(__g_entry[HK_DIR_LEFT].dispatch != NULL)
								{
										hkCliDispatchEntryParam_t param;

										param.ctx = __g_entry[HK_DIR_LEFT].ctx;
										param.event = HK_EVENT_ENTER;
										Com_memset(&param.enter_evt, 0, sizeof(param.enter_evt));
										param.enter_evt.src_x_fullscreen = x_full_screen;
										param.enter_evt.src_y_fullscreen = y_full_screen;
										param.enter_evt.x = mouse_stu->pt.x;
										param.enter_evt.y = mouse_stu->pt.y;

										__g_entry[HK_DIR_LEFT].dispatch(&param);
										__g_state = HK_STATE_REMOTE;
										__g_curr_dir = HK_DIR_LEFT;
										__g_last_pt = mouse_stu->pt;
								}
								Com_UnLockSpinLock(&__g_lock);
								return 1;/*开始锁定鼠标*/

						}else if(mouse_stu->pt.x >= x_full_screen)
						{
								Com_printf(L"(%d:%d)\r\n", mouse_stu->pt.x, mouse_stu->pt.y);

								Com_LockSpinLock(&__g_lock);

								if(__g_entry[HK_DIR_RIGHT].dispatch != NULL)
								{
										hkCliDispatchEntryParam_t param;

										param.ctx = __g_entry[HK_DIR_RIGHT].ctx;
										param.event = HK_EVENT_ENTER;
										Com_memset(&param.enter_evt, 0, sizeof(param.enter_evt));
										param.enter_evt.src_x_fullscreen = x_full_screen;
										param.enter_evt.src_y_fullscreen = y_full_screen;
										param.enter_evt.x = mouse_stu->pt.x;
										param.enter_evt.y = mouse_stu->pt.y;

										__g_entry[HK_DIR_RIGHT].dispatch(&param);
										__g_state = HK_STATE_REMOTE;
										__g_curr_dir = HK_DIR_RIGHT;
										__g_last_pt = mouse_stu->pt;

								}
								Com_UnLockSpinLock(&__g_lock);
								return 1;/*开始锁定鼠标*/
						}else
						{
								return CallNextHookEx(__g_mouse_hook, code, w, l);
						}
				}
						break;
				case WM_LBUTTONDOWN:
				case WM_LBUTTONUP:
				case WM_MOUSEWHEEL:
				case WM_MOUSEHWHEEL:
				case WM_RBUTTONDOWN:
				case WM_RBUTTONUP:
				default:
						return CallNextHookEx(__g_mouse_hook, code, w, l);
						break;
				}
		}
				break;
		case HK_STATE_REMOTE:
		{
				Com_LockSpinLock(&__g_lock);

				if(__g_entry[__g_curr_dir].dispatch != NULL)
				{
						hkCliDispatchEntryParam_t param;

						param.ctx = __g_entry[__g_curr_dir].ctx;
						param.event = HK_EVENT_MOUSE;
						
						param.mouse_evt.x = mouse_stu->pt.x - __g_last_pt.x;
						param.mouse_evt.y = mouse_stu->pt.y - __g_last_pt.y;
						param.mouse_evt.msg = (uint_32_t)w;
						param.mouse_evt.data = mouse_stu->mouseData;
						__g_entry[__g_curr_dir].dispatch(&param);
				}
				Com_UnLockSpinLock(&__g_lock);

				return 1;
		}
				break;
		default:
				Com_error(COM_ERR_FATAL, L"%d is not valid state\r\n", s);
				return CallNextHookEx(__g_keyboard_hook, code, w, l); //disable warning
				break;
		}
		/*return CallNextHookEx(__g_mouse_hook, code, w, l);*/
}





static LRESULT CALLBACK keyboard_hook_func(int code, WPARAM w, LPARAM l)
{
		hkCliState_t s;

		if(code != HC_ACTION)
		{
				return CallNextHookEx(__g_keyboard_hook, code, w, l);
		}
		
		Com_LockSpinLock(&__g_lock);
		s = __g_state;
		Com_UnLockSpinLock(&__g_lock);
		
		
		
		switch(s)
		{
		case HK_STATE_NORMAL:
		{
				return CallNextHookEx(__g_keyboard_hook, code, w, l);
		}
				break;
		case HK_STATE_REMOTE:
		{
				return CallNextHookEx(__g_keyboard_hook, code, w, l);
				/*return 1;*/
		}
				break;
		default:
				Com_error(COM_ERR_FATAL, L"%d is not valid state\r\n", s);
				return CallNextHookEx(__g_keyboard_hook, code, w, l); //disable warning
				break;
		}

}






bool_t	Hook_Cli_RegisterDispatch(hkCliDirection_t		dir, void	*ctx, hkCliDispatchFunc_t		dispatch)
{
		Com_ASSERT(dir < HK_DIR_MAX && dispatch != NULL);
		
		Com_ASSERT(Hook_Cli_IsStarted());
		
		if(!Hook_Cli_IsStarted())
		{
				return false;
		}

		if(__g_entry[dir].dispatch != NULL)
		{
				return false;
		}
		
		Com_LockSpinLock(&__g_lock);
		__g_entry[dir].dispatch = dispatch;
		__g_entry[dir].ctx = ctx;
		Com_UnLockSpinLock(&__g_lock);
		return true;
}

bool_t	Hook_Cli_UnRegisterDispatch(hkCliDirection_t		dir)
{
		
		Com_ASSERT(dir < HK_DIR_MAX);
		Com_ASSERT(Hook_Cli_IsStarted());
		
		if(!Hook_Cli_IsStarted())
		{
				return false;
		}

		if(__g_entry[dir].dispatch == NULL)
		{
				return false;
		}

		Com_LockSpinLock(&__g_lock);
		__g_entry[dir].dispatch = NULL;
		__g_entry[dir].ctx = NULL;
		Com_UnLockSpinLock(&__g_lock);
		return true;
}




bool_t	Hook_Cli_ControlReturn()
{
		Com_ASSERT(Hook_Cli_IsStarted());

		if(!Hook_Cli_IsStarted())
		{
				return false;
		}

		Com_LockSpinLock(&__g_lock);
		__g_state = HK_STATE_NORMAL;
		Com_UnLockSpinLock(&__g_lock);

		return true;
}
