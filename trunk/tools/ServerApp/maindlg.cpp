#include "StdAfx.h"
#include "resource.h"
#include "trayiconimpl.h"
#include "MainDlg.h"


static void	__stdcall on_print_log(const wchar_t *msg, void *ctx)
{

}

static void	__stdcall on_error_log(int_t level, const wchar_t *msg, void *ctx)
{


}



static cmInit_t	__g_com_init = 
{
		{
				on_print_log,
				on_error_log,
				NULL
		}
};




////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMainDlg::CMainDlg()
{
		Com_Init(&__g_com_init);
}

CMainDlg::~CMainDlg()
{

		Com_UnInit();
}

/*****************************************************************************************************************************/
BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
		return IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle()
{
		return FALSE;
}
	


BOOL CMainDlg::load_config()
{
		wchar_t buf[MAX_PATH];
		wchar_t drv[3];

		::GetModuleFileNameW(::GetModuleHandleW(NULL), buf, MAX_PATH);
		//_wsplitpath(buf, drv, 

		return TRUE;
}

LRESULT CMainDlg::OnInitDialog(HWND wParam, LPARAM lParam)
	{
		// center the dialog on the screen
		CenterWindow();

		// set icons
		HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
		SetIcon(hIconSmall, FALSE);
		
		this->InstallIcon(_T("MultiMove Server"), hIconSmall, IDR_MENU_POPUP);
		
		UIEnable(ID_MENU_START, TRUE);
		UIEnable(ID_MENU_STOP, FALSE);
		//((CDialogImpl<CMainDlg>*)this)->ShowWindow(SW_HIDE);
		

		load_config();
		
		return TRUE;
	}

	LRESULT CMainDlg::OnAppAbout(UINT uCode, int nCtrlID, HWND hwndCtrl)
	{
		CSimpleDialog<IDD_ABOUTBOX, FALSE> dlg;
		dlg.DoModal();
		return 0;
	}
	
	void CMainDlg::CloseDialog(int nVal)
	{
		DestroyWindow();
		::PostQuitMessage(nVal);
	}


	LRESULT CMainDlg::OnExit(UINT uCode, int nCtrlID, HWND hwndCtrl)
	{
			CloseDialog(nCtrlID);
			return 0;
	}

	LRESULT CMainDlg::OnMenuStart(UINT uCode, int nCtrlID, HWND hwndCtrl)
	{
			// TODO: Add your command handler code here

			UIEnable(ID_MENU_START, FALSE);
			UIEnable(ID_MENU_STOP, TRUE);
			return 0;
	}

	LRESULT CMainDlg::OnMenuStop(UINT uCode, int nCtrlID, HWND hwndCtrl)
	{
			// TODO: Add your command handler code here

			UIEnable(ID_MENU_START, TRUE);
			UIEnable(ID_MENU_STOP, FALSE);
			return 0;
	}

	LRESULT CMainDlg::OnMenuOption(UINT uCode, int nCtrlID, HWND hwndCtrl)
	{
			// TODO: Add your command handler code here
			return 0;
	}

	
	LRESULT CMainDlg::OnMenuShow(UINT uCode, int nCtrlID, HWND hwndCtrl)
	{
			// TODO: Add your command handler code here
			if (!IsWindowVisible())
					ShowWindow(SW_SHOW);
			// Restore if window if minimized
			if (IsIconic())
					ShowWindow(SW_RESTORE);
			else
					BringWindowToTop();
			// Make this the active window
			::SetForegroundWindow(m_hWnd);
			this->RemoveIcon();
			return 0;
	}


		
	LRESULT CMainDlg::OnBnClickedButtonHide(UINT code, int id, HWND hwnd)
	{
			// TODO: Add your control notification handler code here
			HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
			this->InstallIcon(_T("MultiMove Server"), hIconSmall, IDR_MENU_POPUP);
			ShowWindow(SW_HIDE);
			return 0;
	}

	
	LRESULT CMainDlg::OnClose()
	{
			// TODO: Add your message handler code here and/or call default
			return OnBnClickedButtonHide(BN_CLICKED, IDC_BUTTON_HIDE,NULL);
	}
	

	LRESULT CMainDlg::OnSize(UINT nType, const CSize &size)
	{
			// TODO: Add your message handler code here and/or call default
				switch(nType)
				{
				case SIZE_MINIMIZED:
						return OnBnClickedButtonHide(BN_CLICKED, IDC_BUTTON_HIDE,NULL);
						break;
				case SIZE_MAXHIDE:
				case SIZE_MAXIMIZED:
				case SIZE_MAXSHOW:
				case SIZE_RESTORED:
				default:
						break;
				}
				return 0;
	}
		


	void CMainDlg::PrepareMenu(HMENU menu)
	{
			ATLASSERT(menu != NULL);
			CMenuHandle m(menu);
			m.EnableMenuItem(ID_MENU_START, MF_DISABLED);
			m.Detach();
	}