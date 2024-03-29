
// MultiMoveClient.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "MultiMoveClient.h"
#include "MultiMoveClientDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMultiMoveClientApp

BEGIN_MESSAGE_MAP(CMultiMoveClientApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CMultiMoveClientApp construction

CMultiMoveClientApp::CMultiMoveClientApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CMultiMoveClientApp object

CMultiMoveClientApp theApp;


// CMultiMoveClientApp initialization

#define MUTEX_NAME		L"A038B7FF-735C-44F2-BA06-F23EEE541B3D"

static BOOL IsSameProcessRunning()
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



BOOL CMultiMoveClientApp::InitInstance()
{

		if(IsSameProcessRunning())
		{
				return FALSE;
		}


	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CMultiMoveClientDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

