// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once



class CMainDlg : public CDialogImpl<CMainDlg>,
				public CUpdateUI<CMainDlg>,
				public CMessageFilter,
				public CIdleHandler,
				public CTrayIconImpl<CMainDlg>
{
private:
		CString			m_listen_addr;
		UINT16			m_listen_port;

private:
		BOOL	load_config();
public:
	enum { IDD = IDD_MAINDLG };
public:
		CMainDlg();
		~CMainDlg();
	

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();
	void CloseDialog(int nVal);
	virtual void PrepareMenu(HMENU menu);
	BEGIN_UPDATE_UI_MAP(CMainDlg)

	END_UPDATE_UI_MAP()


	BEGIN_MSG_MAP(CMainDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_SIZE(OnSize)
		MSG_WM_CLOSE(OnClose)

		COMMAND_ID_HANDLER_EX(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_EXIT, OnExit)
		COMMAND_ID_HANDLER_EX(ID_MENU_SHOW, OnMenuShow)
		COMMAND_ID_HANDLER_EX(ID_MENU_START, OnMenuStart)
		COMMAND_ID_HANDLER_EX(ID_MENU_STOP, OnMenuStop)
		COMMAND_ID_HANDLER_EX(ID_MENU_OPTION, OnMenuOption)
		
		COMMAND_HANDLER_EX(IDC_BUTTON_HIDE, BN_CLICKED, OnBnClickedButtonHide)
		
		CHAIN_MSG_MAP(CTrayIconImpl<CMainDlg>)
	END_MSG_MAP()






// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(HWND wParam, LPARAM lParam);
	LRESULT OnClose();
	LRESULT OnSize(UINT nType, const CSize &size);

	LRESULT OnAppAbout(UINT uCode, int nCtrlID, HWND hwndCtrl);
	LRESULT OnMenuShow(UINT uCode, int nCtrlID, HWND hwndCtrl);
	LRESULT OnExit(UINT uCode, int nCtrlID, HWND hwndCtrl);
	LRESULT OnMenuStart(UINT uCode, int nCtrlID, HWND hwndCtrl);
	LRESULT OnMenuStop(UINT uCode, int nCtrlID, HWND hwndCtrl);
	LRESULT OnMenuOption(UINT uCode, int nCtrlID, HWND hwndCtrl);
	LRESULT OnBnClickedButtonHide(UINT code, int id, HWND hwnd);
	
};
