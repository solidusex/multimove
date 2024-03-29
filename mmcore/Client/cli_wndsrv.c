/*
Copyright (C) 2011 by Solidus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.


*/
#include "cli_wndsrv.h"

MM_NAMESPACE_BEGIN


#define MULTI_MOVE_WND_CLASS	TEXT("__MultiMoveWindowClass_Client__")

#define WM_SET_CLIPBOARD_MSG	WM_USER + 1001


/******************************************************************/


typedef struct __set_clip_board_tag
{
		nmPosition_t			pos;
		nmClipDataType_t		t;
		byte_t					*data;
		size_t					length;
}setClipBoardPack_t;

static setClipBoardPack_t*	CreateSetClipBoardPack(nmPosition_t			pos, nmClipDataType_t	t,const byte_t	*data, size_t length)
{
		setClipBoardPack_t		*pack;
		Com_ASSERT(data != NULL && length != NULL);
		pack = Com_NEW0(setClipBoardPack_t);
		pack->pos = pos;
		pack->t = t;
		pack->data = Com_NEWARR(byte_t, length);
		Com_memcpy(pack->data, data, length);
		pack->length = length;
		return pack;
}

static void			DestroySetClipBoardPack(setClipBoardPack_t *pack)
{
		Com_ASSERT(pack != NULL);

		if(pack->data)
		{
				Com_DEL(pack->data);
		}
		Com_DEL(pack);
}



/******************************************************************/
static struct {
		void					*ctx;
		wndMsgHander_t			handler;
}__g_entry[NM_POS_MAX] = 
{
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL}
};

static HWND __g_wnd = NULL;
static HWND __g_next_clipboard = NULL;
static cmThread_t *__g_worker_thread = NULL;
static cmMutex_t  __g_lock;


static cmAsyncQueue_t *__g_clipdata_queue;

static void window_thread(void *data);


bool_t	WND_Cli_Start()
{
		cmEvent_t *event;
		if(WND_Cli_IsStarted())
		{
				return false;
		}


		__g_wnd = NULL;
		__g_next_clipboard = NULL;
		__g_worker_thread = NULL;
		Com_InitMutex(&__g_lock);

		event = Com_CreateEvent(false);
		__g_worker_thread = Com_CreateThread(window_thread, (void*)event, NULL);

		Com_WaitEvent(event);
		Com_DestroyEvent(event);
		event = NULL;
		
		return true;
}



bool_t	WND_Cli_Stop()
{
		if(!WND_Cli_IsStarted())
		{
				return false;
		}

		SendMessage(__g_wnd, WM_CLOSE, NULL, NULL);

		Com_JoinThread(__g_worker_thread);
		Com_CloseThread(__g_worker_thread);
		
		__g_worker_thread = NULL;
		__g_next_clipboard = NULL;
		__g_wnd = NULL;
		Com_UnInitMutex(&__g_lock);
		return true;
}



bool_t	WND_Cli_IsStarted()
{
		if(__g_wnd != NULL)
		{
				return true;
		}else
		{
				return false;
		}
		
}


bool_t	WND_Cli_RegisterHandler(nmPosition_t	pos, void	*ctx, wndMsgHander_t on_msg)
{
		bool_t ret;
		Com_ASSERT(pos < NM_POS_MAX && ctx != NULL && on_msg != NULL);

		if(!WND_Cli_IsStarted())
		{
				return false;
		}


		Com_LockMutex(&__g_lock);
		
		if(__g_entry[pos].handler != NULL)
		{
				ret = false;
		}else
		{
				__g_entry[pos].handler = on_msg;
				__g_entry[pos].ctx = ctx;
				ret = true;
		}
		Com_UnLockMutex(&__g_lock);
		
		return true;
}

bool_t	WND_Cli_UnRegisterHandler(nmPosition_t	pos)
{
		Com_ASSERT(pos < NM_POS_MAX);
		
		if(!WND_Cli_IsStarted())
		{
				return false;
		}

		Com_LockMutex(&__g_lock);
		__g_entry[pos].handler = NULL;
		__g_entry[pos].ctx = NULL;
		Com_UnLockMutex(&__g_lock);
		return true;
}





static bool_t	__set_clipboard_data(const setClipBoardPack_t *pack)
{
		wchar_t *utf16;
		size_t l;
		HGLOBAL hglb;
		bool_t is_ok;

		is_ok = true;
		utf16 = NULL;
		l = 0;
		hglb = NULL;


		if(!WND_Cli_IsStarted())
		{
				return false;
		}
		
		if(pack->t != NM_CLIP_TEXT)
		{
				Com_error(COM_ERR_WARNING, L"Invalid Clipboard data type %d\r\n", pack->t);
				is_ok = false;
				goto END_POINT;
		}
		

		
		if(!OpenClipboard(__g_wnd))
		{
				Com_error(COM_ERR_WARNING, L"OpenClipboard failed error code : %d\r\n", GetLastError());
				is_ok = false;
				goto END_POINT;
		}

		
		if(!EmptyClipboard())
		{
				Com_error(COM_ERR_WARNING, L"EmptyClipboard failed error code %d\r\n", GetLastError());
				is_ok = false;
				goto END_POINT;
		}
		


		utf16 = Com_str_convto_wcs(COM_CP_UTF8, (const char*)pack->data, pack->length);
		if(utf16 == NULL)
		{
				Com_error(COM_ERR_WARNING, L"Invlaid clipboard data\r\n");
				is_ok = false;
				goto END_POINT;
		}
		
		l = Com_wcslen(utf16);

		if(l == 0)
		{
				Com_DEL(utf16);
				is_ok = true;
				goto END_POINT;
		}

		hglb = GlobalAlloc(GMEM_DDESHARE, (l +1) * sizeof(wchar_t));
		if (hglb != NULL) 
		{
				wchar_t *dest = (wchar_t*) GlobalLock(hglb);
				Com_wcscpy(dest, utf16);
				GlobalUnlock(hglb);
				SetClipboardData(CF_UNICODETEXT, hglb);
		}

END_POINT:
		if(utf16)
		{
				Com_DEL(utf16);
				utf16 = NULL;
		}
		CloseClipboard();


		return is_ok;
}


bool_t	WND_Cli_SetClipboardData(nmPosition_t	pos, const nmMsg_t *msg)
{
		setClipBoardPack_t *pack;
		Com_ASSERT(msg != NULL && msg->clip_data.data != NULL && msg->clip_data.length > 0);

		if(!WND_Cli_IsStarted())
		{
				return false;
		}

		pack = CreateSetClipBoardPack(pos, msg->clip_data.data_type, msg->clip_data.data, msg->clip_data.length);
		
		if(!PostMessage(__g_wnd, WM_SET_CLIPBOARD_MSG, (WPARAM)pack, NULL))
		{
				Com_error(COM_ERR_WARNING, L"WND_Cli_SetClipboardData failed error code : %d\r\n", GetLastError());
				DestroySetClipBoardPack(pack);
				pack = NULL;
				return false;
		}
		return true;
}



/*********************************************************************************************************/

static LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg,WPARAM wParam,LPARAM lParam);


static void window_thread(void *data)
{
		WNDCLASSEX wcex;
		MSG msg;
		cmEvent_t *event;
		HWND hwnd = NULL;
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


		hwnd = CreateWindow(
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

		if (hwnd == NULL)
		{
				
				Com_error(COM_ERR_FATAL, L"Create helper window failed\r\n");
				return;
		}
		
		Com_ASSERT(__g_wnd != NULL);

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

		int i;
		nmMsg_t msg;
		HGLOBAL hglb = NULL;
		
		char *utf8;
		const wchar_t* str;
		
		str = NULL;
		utf8 = NULL;


		if (!OpenClipboard(__g_wnd))
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
		
		Com_printf(L"On Clipboard Data:\r\n\r\n%s\r\n\r\n", str);

		utf8 = Com_wcs_convto_str(COM_CP_UTF8, str, Com_wcslen(str));
		
		
		

		
		

		if(utf8 == NULL)
		{
				Com_error(COM_ERR_WARNING, L"invalid clipboard data %ls\r\n",str);

		}else
		{
				Com_memset(&msg, 0, sizeof(msg));
				msg.t = NM_MSG_CLIPDATA;
				msg.clip_data.data_type = NM_CLIP_TEXT;
				msg.clip_data.data = (const byte_t*)utf8;
				msg.clip_data.length = Com_strlen(utf8);
				
				Com_LockMutex(&__g_lock);

				for(i = 0; i < NM_POS_MAX; ++i)
				{
						if(__g_entry[i].handler)
						{
								__g_entry[i].handler(&msg, __g_entry[i].ctx);
						}
				}

				Com_UnLockMutex(&__g_lock);
		}
		
		str = NULL;
		
		GlobalUnlock(hglb);
		CloseClipboard(); 

		if(utf8)
		{
				Com_DEL(utf8);
				utf8 = NULL;
		}

		return true;
}





static LRESULT CALLBACK window_proc(HWND hwnd,  UINT uMsg,  WPARAM wParam, LPARAM lParam)
{

		static bool_t is_in_setclipboard_mode = false;

		switch (uMsg)
		{
		case WM_DESTROY:
				if(!ChangeClipboardChain(hwnd, __g_next_clipboard))
				{
						Com_ASSERT(false);
				}
				__g_next_clipboard = NULL;
				PostQuitMessage(0);
				break;
		case WM_DRAWCLIPBOARD:
				if(is_in_setclipboard_mode)
				{
						is_in_setclipboard_mode = false;
				}else
				{
						__on_clipboard_changed();
						SendMessage(__g_next_clipboard, WM_DRAWCLIPBOARD , 0,0);
				}
				
				break;
		case WM_CREATE:
				__g_wnd = hwnd;
				__g_next_clipboard = SetClipboardViewer(hwnd);
				break;
		case WM_SET_CLIPBOARD_MSG:
		{
				int i;
				const setClipBoardPack_t *pack;
				nmMsg_t msg;

				Com_ASSERT(wParam != NULL);
				Com_printf(L"On WM_SET_CLIPBOARD_MSG\r\n");
				
				pack = (const setClipBoardPack_t*)wParam;

				Com_memset(&msg, 0, sizeof(msg));
				msg.t = NM_MSG_CLIPDATA;
				msg.clip_data.data_type = NM_CLIP_TEXT;
				msg.clip_data.data = pack->data;
				msg.clip_data.length = pack->length;


				is_in_setclipboard_mode = true;
				__set_clipboard_data(pack);


				Com_LockMutex(&__g_lock);

				for(i = 0; i < NM_POS_MAX; ++i)
				{
						if(i != (int)pack->pos && __g_entry[i].handler)
						{
								__g_entry[i].handler(&msg, __g_entry[i].ctx);
						}
				}

				Com_UnLockMutex(&__g_lock);



				DestroySetClipBoardPack((setClipBoardPack_t*)wParam);
		}
				break;
		default:
				return DefWindowProc(hwnd,uMsg,wParam,lParam);
		}

		return 0;
}

/***************************************************************************************************/






MM_NAMESPACE_END



