
// MultiMoveServerDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"




// CMultiMoveServerDlg dialog
class CMultiMoveServerDlg : public CDialogEx
{
private:
		NOTIFYICONDATA			m_nid;
		BOOL					m_is_hide;
		iniObject_t				*m_cfg;				
		// Construction
public:
		CMultiMoveServerDlg(CWnd* pParent = NULL);	// standard constructor

		// Dialog Data
		enum { IDD = IDD_MULTIMOVESERVER_DIALOG };

protected:
		virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


		// Implementation
private:
		void	init_dlg_items();
		
		BOOL	init_systray();
		void	init_config_files();

		void	save_config();

		CString get_dlg_ip()const;
		void	get_dlg_port_range(DWORD &beg, DWORD &end)const;

		BOOL	is_hide()const;
		void	show_dlg();
		void	hide_dlg();
public:
		LRESULT OnShowTask(WPARAM wParam, LPARAM lParam);
		LRESULT OnLogMsg(WPARAM wParam, LPARAM lParam);
		LRESULT OnNotifyMsg(WPARAM wParam, LPARAM lParam);

		afx_msg void OnHideDlg();
		afx_msg void OnShowDlg();
		afx_msg void OnAbout();

public:
		void	AppendLog(const CString &str);
		void	OnClientConnected(const CString &addr_info);

protected:
		HICON m_hIcon;

		// Generated message map functions
		virtual BOOL OnInitDialog();
		afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
		afx_msg void OnPaint();
		afx_msg HCURSOR OnQueryDragIcon();
		DECLARE_MESSAGE_MAP()
public:
		afx_msg void OnBnClickedOk();
		afx_msg void OnBnClickedCancel();
		afx_msg void OnBnClickedButtonQuit();
		
		afx_msg void OnDestroy();
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
		afx_msg void OnNcDestroy();
		virtual BOOL PreTranslateMessage(MSG* pMsg);

		afx_msg void OnBnClickedButtonSrvstart();
		afx_msg void OnBnClickedButtonSrvstop();

		CIPAddressCtrl m_srv_ip;
		CEdit m_srv_port_start;
		CEdit m_srv_port_end;
		CButton m_srv_start;
		CButton m_srv_stop;
		CEdit m_log;
		CEdit m_client_addr;
};


