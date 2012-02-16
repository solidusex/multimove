
// MultiMoveClientDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CMultiMoveClientDlg dialog
class CMultiMoveClientDlg : public CDialogEx
{
private:
		NOTIFYICONDATA			m_nid;
		BOOL					m_is_hide;
		iniObject_t				*m_cfg;				

private:
		BOOL	init_systray();
		void	uninit_systray();

		BOOL	is_hide()const;
		void	show_dlg();
		void	hide_dlg();

		void	init_config();
		void	uninit_config();
		void	save_config();

		void	init_dlg_items();


		void	init_client();
		void	uninit_cliet();
		
public:
		afx_msg void OnHideDlg();
		afx_msg void OnShowDlg();
		afx_msg void OnAbout();

		LRESULT OnShowTask(WPARAM wParam, LPARAM lParam);
		LRESULT OnLogMsg(WPARAM wParam, LPARAM lParam);
		LRESULT OnNotifyMsg(WPARAM wParam, LPARAM lParam);

		afx_msg void OnNcDestroy();
		afx_msg void OnBnClickedButtonQuit();
		afx_msg void OnBnClickedOk();
		afx_msg void OnClose();
		afx_msg void OnBnClickedButtonUpConnect();
		afx_msg void OnBnClickedButtonDownConnect();
		afx_msg void OnBnClickedButtonLeftConnect();
		afx_msg void OnBnClickedButtonRightConnect();
// Construction
public:
	CMultiMoveClientDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MULTIMOVECLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support




// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

public:
		void AppendLog(const CString &log);


	DECLARE_MESSAGE_MAP()
	CEdit m_log;
	CEdit m_up_ip;
	CEdit m_up_port;
	CEdit m_down_ip;
	CEdit m_down_port;
	CEdit m_left_ip;
	CEdit m_left_port;
	CEdit m_right_ip;
	CEdit m_right_port;
	
	BOOL	m_up_is_connected;
	CButton m_up_connect;

	BOOL	m_down_is_connected;
	CButton m_down_connect;

	BOOL	m_left_is_connected;
	CButton m_left_connect;

	BOOL	m_right_is_connected;
	CButton m_right_connect;
	
};
