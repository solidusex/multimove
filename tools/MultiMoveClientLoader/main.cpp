

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>



#define MUTEX_NAME		L"2ED7BA57-3923-432A-B725-A1CAC76231C7"

BOOL IsSameProcessRunning()
{
		HANDLE   mtx   =   CreateMutexW(0,   TRUE,   MUTEX_NAME); 

		if(GetLastError()   ==   ERROR_ALREADY_EXISTS)   
		{ 
				ReleaseMutex(mtx); 
				CloseHandle(mtx); 
				return TRUE;
		}else
		{
				return FALSE;
		}
}

void UTIL_GetModulePath(wchar_t path[4096])
{
		static TCHAR buf[MAX_PATH * 2];
		::GetModuleFileName(GetModuleHandle(NULL), buf, MAX_PATH * 2);
		
		static TCHAR d[5], dir[MAX_PATH];

		_wsplitpath(buf, d, dir,NULL,NULL);

		path[0] = L'\0';
		wcscat(path, d);
		wcscat(path, dir);
}



int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR    lpCmdLine, int       nCmdShow)
{

		if(IsSameProcessRunning())
		{
				return -1;
		}

		wchar_t path[4096];
		UTIL_GetModulePath(path);

		wcscat(path, L"MultiMoveClient.exe");


		PROCESS_INFORMATION     pi; 
		ZeroMemory(&pi,sizeof(PROCESS_INFORMATION)); 
		STARTUPINFO   si; 
		ZeroMemory(&si,sizeof(STARTUPINFO)); 
		si.cb=sizeof(STARTUPINFO); 
		si.wShowWindow=SW_SHOW; 
		si.dwFlags=STARTF_USESHOWWINDOW; 




		BOOL   fRet=::CreateProcessW(path, 
				NULL,
				NULL, 
				NULL, 
				FALSE, 
				NORMAL_PRIORITY_CLASS|CREATE_NEW_CONSOLE, 
				NULL, 
				NULL, 
				&si, 
				&pi); 

		if(fRet)
		{
				::WaitForSingleObject(pi.hProcess, INFINITE);
				//::MessageBoxW(NULL, L"AAA", L"AAA", 0);
				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);

				pi.hThread = INVALID_HANDLE_VALUE;
				pi.hProcess = INVALID_HANDLE_VALUE;
				SystemParametersInfo(SPI_SETCURSORS,0,0,WM_SETTINGCHANGE | SPIF_UPDATEINIFILE );
		} 



		return 0;
}

