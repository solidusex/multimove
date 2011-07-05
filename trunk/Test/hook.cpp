
#define OEMRESOURCE

#include "test.h"


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Common/common.h"

#include <winuser.h>


DWORD g_main_tid = 0;
HHOOK g_kb_hook = 0;
HHOOK __g_mouse_hook = NULL;
bool CALLBACK con_handler (DWORD)
{
		PostThreadMessage (g_main_tid, WM_QUIT, 0, 0);
		return TRUE;
};

LRESULT CALLBACK kb_proc (int code, WPARAM w, LPARAM l)
{
	
				
		if(code < 0)
		{
				return CallNextHookEx (g_kb_hook, code, w, l);
		}


		/*
		PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)l;
		const char *info = NULL;
		if (w == WM_KEYDOWN)
				info = "key dn";
		else if (w == WM_KEYUP)
				info = "key up";
		// 本文转自 C Builder研究 - http://www.ccrun.com/article.asp?i=984&d=es23f6
		else if (w == WM_SYSKEYDOWN)
				info = "sys key dn";
		else if (w == WM_SYSKEYUP)
				info = "sys key up";
		printf ("%s - vkCode [%x], scanCode [%x]\n", info, p->vkCode, p->scanCode);
		// always call next hook
		return CallNextHookEx (g_kb_hook, code, w, l);
		*/

		return 1;
};


int prev_x = 0;
int prev_y = 0;

LRESULT CALLBACK mouse_proc(int code, WPARAM w, LPARAM l)
{

		int x_full_screen = GetSystemMetrics(SM_CXSCREEN);
		int y_full_screen = GetSystemMetrics(SM_CYSCREEN);
		MSLLHOOKSTRUCT *mouse_stu = (MSLLHOOKSTRUCT*)l;
		
		::ShowCursor(FALSE);

		if(code < 0)
		{
				return CallNextHookEx (g_kb_hook, code, w, l);
		}
		
		short mouseData = 0;
		const char *info = NULL;
		
		switch(w)
		{
		case WM_LBUTTONDOWN:
				info = "WM_LBUTTONDOWN";
				break;
		case WM_LBUTTONUP:
				info = "WM_LBUTTONUP";
				break;
		case WM_MOUSEMOVE:
				info = "WM_MOUSEMOVE";
				break;
		case WM_MOUSEWHEEL:
				info = "WM_MOUSEWHEEL";
				mouseData = (short)((mouse_stu->mouseData >> 16) & 0xffff);
				break;
		case WM_MOUSEHWHEEL:
				info = "WM_MOUSEHWHEEL";
				//delta = HIWORD(mouse_stu->mouseData);
				break;
		case WM_RBUTTONDOWN:
				info = "WM_RBUTTONDOWN";
				break;
		case WM_RBUTTONUP:
				info = "WM_RBUTTONUP";
				break;
		default:
				info = "UNKNOW";
				break;
		}

		if(info)
		{
				printf ("%s\r\n", info);
				if(w == WM_MOUSEWHEEL || w == WM_MOUSEHWHEEL)
				{
						printf("detal == %d\r\n", mouseData);
				}
		}
		return CallNextHookEx (g_kb_hook, code, w, l);
#if(0)
		if(w == WM_MOUSEMOVE)
		{
				static bool_t is_init = false;

				if(!is_init)
				{
						prev_x = mouse_stu->pt.x;
						prev_y = mouse_stu->pt.y;
						is_init = true;
				}

				if(mouse_stu->pt.x >= x_full_screen)
				{
						prev_x = 0;
						prev_y = mouse_stu->pt.y;
						mouse_event(MOUSEEVENTF_MOVE, -(x_full_screen), 0, 0, 0);
						return 1;
				}else  if(mouse_stu->pt.x <= 0)
				{
						prev_x = x_full_screen;
						prev_y = mouse_stu->pt.y;
						mouse_event(MOUSEEVENTF_MOVE, x_full_screen, 0, 0, 0);
						return 1;
				}else if(mouse_stu->pt.y >= y_full_screen)
				{
						prev_x = mouse_stu->pt.x;
						prev_y = 0;
						mouse_event(MOUSEEVENTF_MOVE, 0, -(y_full_screen), 0, 0);
						return 1;
				}else if(mouse_stu->pt.y <= 0)
				{
						prev_x = mouse_stu->pt.x;
						prev_y = y_full_screen;
						mouse_event(MOUSEEVENTF_MOVE, 0, y_full_screen, 0, 0);
						return 1;
				}
				
				else
				{
						int relative_x_move = mouse_stu->pt.x - prev_x;
						int relative_y_move = mouse_stu->pt.y - prev_y;
						printf("relative move (%d : %d)\r\n", relative_x_move, relative_y_move);
						prev_x = mouse_stu->pt.x;
						prev_y = mouse_stu->pt.y;
				}

				return CallNextHookEx (g_kb_hook, code, w, l);
		}else
		{
				return 1;
		}

#endif

}


void hook_test1()
{

		g_main_tid = GetCurrentThreadId ();
		SetConsoleCtrlHandler ((PHANDLER_ROUTINE)&con_handler, TRUE);
		//g_kb_hook = SetWindowsHookEx (WH_KEYBOARD_LL, (HOOKPROC)&kb_proc,GetModuleHandle(NULL), 0);

		g_kb_hook = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)&mouse_proc,GetModuleHandle(NULL), 0);
		
		if (g_kb_hook == NULL)
		{
				fprintf (stderr, "SetWindowsHookEx failed with error %d\n", ::GetLastError ());
				abort();
		};
		// 消息循环是必须的，想知道原因可以查msdn
		MSG msg;
		while (GetMessage (&msg, NULL, 0, 0))
		{
				TranslateMessage (&msg);
				DispatchMessage (&msg);
		};
		UnhookWindowsHookEx (g_kb_hook);
}








/************************************************************************************************************************/

LRESULT CALLBACK mouse_proc_display(int code, WPARAM w, LPARAM l)
{

		int x_full_screen = GetSystemMetrics(SM_CXSCREEN);
		int y_full_screen = GetSystemMetrics(SM_CYSCREEN);
		MSLLHOOKSTRUCT *mouse_stu = (MSLLHOOKSTRUCT*)l;
		
		//::ShowCursor(FALSE);

		if(code < 0)
		{
				return CallNextHookEx (g_kb_hook, code, w, l);
		}

		if(w == WM_MOUSEMOVE)
		{
				printf("Mouse Move (%d : %d)\r\n", mouse_stu->pt.x, mouse_stu->pt.y);
		}
		return CallNextHookEx (g_kb_hook, code, w, l);
}


void hook_test_input_thread(void *data)
{
		cmEvent_t *ent = (cmEvent_t*)data;
		assert(ent != NULL);

		g_kb_hook = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)&mouse_proc_display,GetModuleHandle(NULL), 0);
		
		if (g_kb_hook == NULL)
		{
				fprintf (stderr, "SetWindowsHookEx failed with error %d\n", ::GetLastError ());
				abort();
		};
		// 消息循环是必须的，想知道原因可以查msdn

		Com_SetEvent(ent);

		MSG msg;
		while (GetMessage (&msg, NULL, 0, 0))
		{
				TranslateMessage (&msg);
				DispatchMessage (&msg);
		};
		UnhookWindowsHookEx (g_kb_hook);
}



static void test_input()
{

		bool_t is_neg;
		
		is_neg = false;
		while(true)
		{
#if(0)
				INPUT ipt;
				memset(&ipt, 0, sizeof(ipt));
				
				ipt.type = INPUT_MOUSE;
				ipt.mi.dwFlags = MOUSEEVENTF_MOVE;
				ipt.mi.dwExtraInfo = GetMessageExtraInfo();
				ipt.mi.dx = is_neg ? -1 : 1;
				ipt.mi.dy = 0;

				for(int i = 0; i < 400; ++i)
				{
						SendInput(1, &ipt, sizeof(ipt));
						::Sleep(10);
				}
#endif
				for(int i = 0; i < 400; ++i)
				{
						mouse_event(MOUSEEVENTF_MOVE, is_neg ? -1 : 1, 0, 0, /*GetMessageExtraInfo()*/0);
						::Sleep(10);
				}

				
				::Sleep(10);
				is_neg = !is_neg;
		}

}




void hook_and_sendinput_test()
{
		cmEvent_t *event = Com_CreateEvent(false);

		cmThread_t *thd = Com_CreateThread(hook_test_input_thread, (void*)event, NULL);
		
		Com_WaitEvent(event);

		Com_DestroyEvent(event);
		event = NULL;
		
		test_input();
	
		getchar();
}






void Hook_Test()
{
		
		//hook_and_sendinput_test();

		//hook_display_test();

		hook_test1();

}
