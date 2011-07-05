
#include "cli_hook.h"





typedef enum 
{
		HK_STATE_STOP			= 0x00,
		HK_STATE_NORMAL,
		HK_STATE_REMOTE
}hkCliState_t;



struct {
		void					*ctx;
		hkMsgHander_t			handler;
}__g_entry[NM_POS_MAX] = 
{
		{ NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL}
};



static HHOOK	__g_mouse_hook = NULL;
static HHOOK	__g_keyboard_hook = NULL;
static size_t	__g_hook_thread_id = 0;
static hkCliState_t __g_state = HK_STATE_STOP;
static nmPosition_t __g_curr_pos = NM_POS_MAX;


static cmMutex_t		__g_lock;
static cmThread_t		*__g_thread = NULL;

static POINT			__g_prev_pt;




#define WM_USER_QUIT_HOOK_THREAD_MSG_ID		0x12345

static void	hook_thread_func(void *data);




bool_t	Hook_Cli_Start()
{

		cmEvent_t *event;
		if(Hook_Cli_IsStarted())
		{
				return false;
		}
		Com_InitMutex(&__g_lock);
		__g_thread = NULL;
		Com_memset(&__g_entry, 0, sizeof(__g_entry));
		__g_mouse_hook = NULL;
		__g_keyboard_hook = NULL;
		__g_hook_thread_id = 0;
		__g_state = HK_STATE_STOP;
		__g_curr_pos = NM_POS_MAX;


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
		__g_curr_pos = NM_POS_MAX;
		Com_UnInitMutex(&__g_lock);
		
		return true;
}


bool_t	Hook_Cli_IsStarted()
{
		return __g_state != HK_STATE_STOP ? true : false;
}





bool_t	Hook_Cli_RegisterHandler(nmPosition_t	pos, void	*ctx, hkMsgHander_t	on_msg)
{
		Com_ASSERT(pos < NM_POS_MAX && on_msg != NULL);
		
		Com_ASSERT(Hook_Cli_IsStarted());
		
		if(!Hook_Cli_IsStarted())
		{
				return false;
		}

		if(__g_entry[pos].handler != NULL)
		{
				return false;
		}
		
		Com_LockMutex(&__g_lock);
		__g_entry[pos].handler = on_msg;
		__g_entry[pos].ctx = ctx;
		Com_UnLockMutex(&__g_lock);
		return true;
}


bool_t	Hook_Cli_UnRegisterHandler(nmPosition_t	pos)
{
		
		Com_ASSERT(pos < NM_POS_MAX);
		Com_ASSERT(Hook_Cli_IsStarted());
		
		if(!Hook_Cli_IsStarted())
		{
				return false;
		}

		if(__g_entry[pos].handler == NULL)
		{
				return false;
		}

		Com_LockMutex(&__g_lock);
		__g_entry[pos].handler = NULL;
		__g_entry[pos].ctx = NULL;
		Com_UnLockMutex(&__g_lock);
		return true;
}





bool_t	Hook_Cli_ControlReturn()
{
		Com_ASSERT(Hook_Cli_IsStarted());

		if(!Hook_Cli_IsStarted())
		{
				return false;
		}

		Com_LockMutex(&__g_lock);
		__g_state = HK_STATE_NORMAL;
		Com_UnLockMutex(&__g_lock);

		return true;
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




static LRESULT on_normal_mouse_action(int code, WPARAM w, LPARAM l)
{
		int x_full_screen, y_full_screen;
		MSLLHOOKSTRUCT *mouse_stu;

		
		x_full_screen = GetSystemMetrics(SM_CXSCREEN);
		y_full_screen = GetSystemMetrics(SM_CYSCREEN);
		
		mouse_stu = (MSLLHOOKSTRUCT*)l;

		//Com_printf(L"On on_normal_mouse_action\r\n");
		Com_ASSERT(mouse_stu != NULL);
		
		switch(w)
		{
		case WM_MOUSEMOVE:
		{
				nmMsg_t	msg;
				msg.t = NM_MSG_ENTER;
				msg.enter.src_x_fullscreen = x_full_screen;
				msg.enter.src_y_fullscreen = y_full_screen;
				msg.enter.x = mouse_stu->pt.x;
				msg.enter.y = mouse_stu->pt.y;
				if(mouse_stu->pt.x < 0)
				{
						Com_printf(L"Shift trigger point on : (%d:%d) to left\r\n", mouse_stu->pt.x, mouse_stu->pt.y);
						
						Com_LockMutex(&__g_lock);

						if(__g_entry[NM_POS_LEFT].handler != NULL)
						{
								if(__g_entry[NM_POS_LEFT].handler(&msg, __g_entry[NM_POS_LEFT].ctx))
								{
										__g_state    = HK_STATE_REMOTE;
										__g_curr_pos = NM_POS_LEFT;
										__g_prev_pt = mouse_stu->pt;
										Com_printf(L"Shift to left side\r\n");
								}
						}
						Com_UnLockMutex(&__g_lock);

						return CallNextHookEx(__g_mouse_hook, code, w, l);
				
				}else if(mouse_stu->pt.x > x_full_screen)
				{
						Com_printf(L"Shift trigger point on : (%d:%d) to right\r\n", mouse_stu->pt.x, mouse_stu->pt.y);
						
						Com_LockMutex(&__g_lock);

						if(__g_entry[NM_POS_RIGHT].handler != NULL)
						{
								if(__g_entry[NM_POS_RIGHT].handler(&msg, __g_entry[NM_POS_RIGHT].ctx))
								{
										__g_state    = HK_STATE_REMOTE;
										__g_curr_pos = NM_POS_RIGHT;
										__g_prev_pt = mouse_stu->pt;
										Com_printf(L"Shift to right side\r\n");
								}
						}
						Com_UnLockMutex(&__g_lock);

						return CallNextHookEx(__g_mouse_hook, code, w, l);
				}else if(mouse_stu->pt.y < 0)
				{
						Com_printf(L"Shift trigger point on : (%d:%d) to up\r\n", mouse_stu->pt.x, mouse_stu->pt.y);
						
						Com_LockMutex(&__g_lock);

						if(__g_entry[NM_POS_UP].handler != NULL)
						{
								if(__g_entry[NM_POS_UP].handler(&msg, __g_entry[NM_POS_UP].ctx))
								{
										__g_state    = HK_STATE_REMOTE;
										__g_curr_pos = NM_POS_UP;
										__g_prev_pt = mouse_stu->pt;
								}
								
								Com_printf(L"Shift to up side\r\n");
						}
						Com_UnLockMutex(&__g_lock);

						return CallNextHookEx(__g_mouse_hook, code, w, l);

				}else if(mouse_stu->pt.y > y_full_screen)
				{
						Com_printf(L"Shift trigger point on : (%d:%d) to down\r\n", mouse_stu->pt.x, mouse_stu->pt.y);
						
						Com_LockMutex(&__g_lock);

						if(__g_entry[NM_POS_DOWN].handler != NULL)
						{
								if(__g_entry[NM_POS_DOWN].handler(&msg, __g_entry[NM_POS_DOWN].ctx))
								{
										__g_state    = HK_STATE_REMOTE;
										__g_curr_pos = NM_POS_DOWN;
										__g_prev_pt = mouse_stu->pt;
										
										Com_printf(L"Shift to up side\r\n");
								}

						}
						Com_UnLockMutex(&__g_lock);

						return CallNextHookEx(__g_mouse_hook, code, w, l);

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


static LRESULT on_remote_mouse_action(int code, WPARAM w, LPARAM l)
{

		int x_full_screen, y_full_screen;
		MSLLHOOKSTRUCT *mouse_stu;
		
		int		relative_x, relative_y;
		bool_t need_send_msg, need_call_next;
		short mouse_data = 0;

		//Com_printf(L"On on_remote_mouse_action\r\n");
		x_full_screen = GetSystemMetrics(SM_CXSCREEN);
		y_full_screen = GetSystemMetrics(SM_CYSCREEN);
		
		mouse_stu = (MSLLHOOKSTRUCT*)l;
		
		Com_ASSERT(mouse_stu != NULL);
		
		need_send_msg = true;
		need_call_next = true;
		relative_x = 0;
		relative_y = 0;


		Com_LockMutex(&__g_lock);
		
		switch(w)
		{
		case WM_MOUSEMOVE:
		{
				
				if(mouse_stu->pt.x >= x_full_screen)
				{
						__g_prev_pt.x = 0;
						__g_prev_pt.y = mouse_stu->pt.y;
						mouse_event(MOUSEEVENTF_MOVE, -x_full_screen, 0, 0, 0);
						need_send_msg = false;
						need_call_next = false;
				}else  if(mouse_stu->pt.x <= 0)
				{
						__g_prev_pt.x = x_full_screen;
						__g_prev_pt.y = mouse_stu->pt.y;
						mouse_event(MOUSEEVENTF_MOVE, x_full_screen, 0, 0, 0);
						need_send_msg = false;
						need_call_next = false;
				}else if(mouse_stu->pt.y >= y_full_screen)
				{
						__g_prev_pt.x = mouse_stu->pt.x;
						__g_prev_pt.y = 0;
						mouse_event(MOUSEEVENTF_MOVE, 0, -y_full_screen, 0, 0);
						need_send_msg = false;
						need_call_next = false;
				}else if(mouse_stu->pt.y <= 0)
				{
						__g_prev_pt.x = mouse_stu->pt.x;
						__g_prev_pt.y = y_full_screen;
						mouse_event(MOUSEEVENTF_MOVE, 0, y_full_screen, 0, 0);
						need_send_msg = false;
						need_call_next = false;
				}else
				{
						relative_x = mouse_stu->pt.x - __g_prev_pt.x;
						relative_y = mouse_stu->pt.y - __g_prev_pt.y;
						
						__g_prev_pt.x = mouse_stu->pt.x;
						__g_prev_pt.y = mouse_stu->pt.y;

						need_send_msg = true;
						need_call_next = true;


						//Com_printf(L"Remote mode relative move : (%d : %d)\r\n", relative_x, relative_y);

						
				}


		}
				break;
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEWHEEL:
				mouse_data = (short)((mouse_stu->mouseData >> 16) & 0xffff);
		case WM_MOUSEHWHEEL:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
				need_send_msg = true;
				need_call_next = false;
				break;
		default:
				need_send_msg = false;
				need_call_next = false;
				break;
		}

		Com_UnLockMutex(&__g_lock);


		if(need_send_msg)
		{
		
				if(__g_entry[__g_curr_pos].handler != NULL)
				{
						nmMsg_t	msg;
						msg.t = NM_MSG_MOUSE;
						msg.mouse.x = relative_x;
						msg.mouse.y = relative_y;
						msg.mouse.data = mouse_data;
						msg.mouse.msg = (uint_32_t)w;

						__g_entry[__g_curr_pos].handler(&msg, __g_entry[__g_curr_pos].ctx);
				}
		}

		return need_call_next ? CallNextHookEx(__g_mouse_hook, code, w, l) : 1;
}


static LRESULT CALLBACK mouse_hook_func(int code, WPARAM w, LPARAM l)
{

		hkCliState_t s;
		if(code != HC_ACTION)
		{
				return CallNextHookEx(__g_mouse_hook, code, w, l);
		}

		Com_LockMutex(&__g_lock);
		s = __g_state;
		Com_UnLockMutex(&__g_lock);
		

		
		switch(s)
		{
		case HK_STATE_NORMAL:
		{
				return on_normal_mouse_action(code, w, l);
		}
				break;
		case HK_STATE_REMOTE:
		{
				return on_remote_mouse_action(code, w, l);
		}
				break;
		default:
				Com_error(COM_ERR_FATAL, L"%d is not valid state\r\n", s);
				return CallNextHookEx(__g_mouse_hook, code, w, l); //disable warning
				break;
		}
}




static LRESULT CALLBACK keyboard_hook_func(int code, WPARAM w, LPARAM l)
{
		hkCliState_t s;
		
		const KBDLLHOOKSTRUCT *kb_stu;
		if(code != HC_ACTION)
		{
				return CallNextHookEx(__g_keyboard_hook, code, w, l);
		}
		
		Com_LockMutex(&__g_lock);
		s = __g_state;
		Com_UnLockMutex(&__g_lock);
		
		kb_stu = (const KBDLLHOOKSTRUCT*)l;

		
		switch(s)
		{
		case HK_STATE_NORMAL:
		{
				return CallNextHookEx(__g_keyboard_hook, code, w, l);
		}
				break;
		case HK_STATE_REMOTE:
		{
				bool_t	is_key_down = true;
				bool_t	need_send_msg = true;
				BYTE vk,scan;
				switch(w)
				{
				case WM_KEYDOWN:
				case WM_SYSKEYDOWN:
						is_key_down = true;
						need_send_msg = true;
						vk = kb_stu->vkCode;
						scan = kb_stu->scanCode;
						break;
				case WM_KEYUP:
				case WM_SYSKEYUP:
						is_key_down = false;
						need_send_msg = true;
						vk = kb_stu->vkCode;
						scan = kb_stu->scanCode;
						break;
				default:
						is_key_down = false;
						need_send_msg = false;
						vk = 0;
						scan = 0;
						break;
				}
				
				if(need_send_msg)
				{
						Com_LockMutex(&__g_lock);

						if(__g_entry[__g_curr_pos].handler != NULL)
						{
								nmMsg_t	msg;
								msg.t = NM_MSG_KEYBOARD;
								msg.keyboard.is_keydown = is_key_down;
								msg.keyboard.vk = vk;
								msg.keyboard.scan = scan;
								__g_entry[__g_curr_pos].handler(&msg, __g_entry[__g_curr_pos].ctx);
						}

						Com_UnLockMutex(&__g_lock);

				}

				//return need_call_next ? CallNextHookEx(__g_keyboard_hook, code, w, l) : 1;
				return 1;
		}
				break;
		default:
				Com_error(COM_ERR_FATAL, L"%d is not valid state\r\n", s);
				return CallNextHookEx(__g_keyboard_hook, code, w, l); //disable warning
				break;
		}

}



