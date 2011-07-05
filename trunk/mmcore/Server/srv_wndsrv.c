#include "srv_wndsrv.h"

MM_NAMESPACE_BEGIN


#define MULTI_MOVE_WND_CLASS	TEXT("__MultiMoveWindowClass_Server__")




static HWND __g_wnd = NULL;
static HWND __g_next_clipboard = NULL;
static cmThread_t *__g_worker_thread = NULL;
static wndMsgHander_t	__g_handler = NULL;

static void window_thread(void *data);


bool_t	WND_Srv_Start(wndMsgHander_t handler)
{
		cmEvent_t *event;
		Com_ASSERT(handler != NULL);


		if(WND_Srv_IsStarted())
		{
				return false;
		}
		

		__g_wnd = NULL;
		__g_next_clipboard = NULL;
		__g_worker_thread = NULL;
		__g_handler = handler;

		event = Com_CreateEvent(false);
		__g_worker_thread = Com_CreateThread(window_thread, (void*)event, NULL);

		Com_WaitEvent(event);
		Com_DestroyEvent(event);
		event = NULL;
		
		return true;
}



bool_t	WND_Srv_Stop()
{
		if(WND_Srv_IsStarted())
		{
				return false;
		}

		SendMessage(__g_wnd, WM_CLOSE, NULL, NULL);

		Com_JoinThread(__g_worker_thread);
		Com_CloseThread(__g_worker_thread);
		
		__g_worker_thread = NULL;
		__g_next_clipboard = NULL;
		__g_wnd = NULL;
		__g_handler = NULL;
		
		return true;
}



bool_t	WND_Srv_IsStarted()
{
		return __g_wnd != NULL ? true : false;
}







static LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg,WPARAM wParam,LPARAM lParam);


static void window_thread(void *data)
{
		WNDCLASSEX wcex;
		MSG msg;
		cmEvent_t *event;
		Com_ASSERT(data != NULL);

		event = (cmEvent_t*)data;


		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= window_proc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= GetModuleHandle(NULL);;
		wcex.hIcon			= NULL;
		wcex.hCursor		= NULL;
		wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wcex.lpszMenuName	= NULL;
		wcex.lpszClassName	= MULTI_MOVE_WND_CLASS;
		wcex.hIconSm		= NULL;

		RegisterClassEx(&wcex);


		__g_wnd = CreateWindow(
								MULTI_MOVE_WND_CLASS, 
								MULTI_MOVE_WND_CLASS, 
								WS_OVERLAPPEDWINDOW, 
								CW_USEDEFAULT, 
								0, 
								CW_USEDEFAULT, 
								0, 
								NULL, 
								NULL, 
								GetModuleHandle(NULL), 
								NULL
								);

		if (!__g_wnd)
		{
				
				Com_error(COM_ERR_FATAL, L"Create helper window failed\r\n");
				return;
		}

		ShowWindow(__g_wnd, SW_HIDE);
		UpdateWindow(__g_wnd);

		Com_SetEvent(event);

		while (GetMessage(&msg,NULL,0,0))
		{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
		}

}



static bool	__on_clipboard_changed()
{
		size_t i,j;
		nmMsg_t msg;
		HGLOBAL hglb = NULL;
		
		char *utf8;
		const wchar_t* str;
		wchar_t *unixcontents, *contents;
		
		str = NULL;
		utf8 = NULL;
		unixcontents = NULL;
		contents = NULL;

		if (!OpenClipboard(NULL))
		{
				return false;
		}

		
		hglb = GetClipboardData(CF_UNICODETEXT);
		if(hglb == NULL)
		{
				CloseClipboard();
				return false;
		}
		

		str = (const wchar_t*) GlobalLock(hglb);
		contents = Com_NEWARR0(wchar_t, Com_wcslen(str) + 1);
		unixcontents = Com_NEWARR0(wchar_t, Com_wcslen(str) + 1);

		wcscpy(contents,str);
		str = NULL;
		GlobalUnlock(hglb);
		CloseClipboard(); 

		// Translate to Unix-format lines before sending
		
		for (i = 0,j= 0; contents[i] != TEXT('\0'); i++)
		{
				if (contents[i] != TEXT('\x0d')) 
				{
						unixcontents[j++] = contents[i];
				}
		}
		unixcontents[j] = TEXT('\0');

		Com_printf(L"On Clipboard Data:\r\n\r\n%s\r\n\r\n", unixcontents);

		Com_memset(&msg, 0, sizeof(msg));
		msg.t = NM_MSG_CLIPDATA;
		msg.clip_data.data_type = NM_CLIP_TEXT;
		utf8 = Com_wcs_convto_str(COM_CP_UTF8, unixcontents, Com_wcslen(unixcontents));

		if(utf8 == NULL)
		{
				Com_error(COM_ERR_WARNING, L"invalid clipboard data %ls\r\n",unixcontents);

		}else
		{
				msg.clip_data.data = (const byte_t*)utf8;
				msg.clip_data.length = Com_strlen(utf8);
				
				if(__g_handler)
				{
						__g_handler(&msg);
				}
		}

		if(contents)
		{
				Com_DEL(contents);
				unixcontents = NULL;
		}

		if(unixcontents)
		{
				Com_DEL(unixcontents);
				unixcontents = NULL;
		}

		if(utf8)
		{
				Com_DEL(utf8);
				utf8 = NULL;
		}

		return true;
}





static LRESULT CALLBACK window_proc(HWND hwnd,  UINT uMsg,  WPARAM wParam, LPARAM lParam)
{
		switch (uMsg)
		{
		case WM_DESTROY:
				if(!ChangeClipboardChain(hwnd, __g_next_clipboard))
				{
						assert(false);
				}
				__g_next_clipboard = NULL;
				PostQuitMessage(0);
				break;
		case WM_DRAWCLIPBOARD:
				__on_clipboard_changed();
				SendMessage(__g_next_clipboard, WM_DRAWCLIPBOARD , 0,0);
				break;
		case WM_CREATE:
				__g_next_clipboard = SetClipboardViewer(hwnd);
				break;
		default:
				return DefWindowProc(hwnd,uMsg,wParam,lParam);
		}

		return 0;
}





MM_NAMESPACE_END



