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
	


LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
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
		return TRUE;
	}

	LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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


	LRESULT CMainDlg::OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
			CloseDialog(wID);
			return 0;
	}

	LRESULT CMainDlg::OnMenuStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
			// TODO: Add your command handler code here

			UIEnable(ID_MENU_START, FALSE);
			UIEnable(ID_MENU_STOP, TRUE);
			return 0;
	}

	LRESULT CMainDlg::OnMenuStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
			// TODO: Add your command handler code here

			UIEnable(ID_MENU_START, TRUE);
			UIEnable(ID_MENU_STOP, FALSE);
			return 0;
	}

	LRESULT CMainDlg::OnMenuOption(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
			// TODO: Add your command handler code here

			return 0;
	}

	
	LRESULT CMainDlg::OnMenuShow(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
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