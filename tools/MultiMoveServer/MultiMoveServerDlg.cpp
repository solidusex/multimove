
// MultiMoveServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MultiMoveServer.h"
#include "MultiMoveServerDlg.h"
#include "afxdialogex.h"
#include "AboutDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static void	__stdcall print_func(const wchar_t *msg, void *ctx)
{
		OutputDebugString(msg);
		
		/*
		CMultiMoveServerDlg		*dlg = (CMultiMoveServerDlg*)ctx;

		CString *log = new CString(msg);
		dlg->PostMessage(WM_LOGMSG, (WPARAM)log);
		*/

		UNUSED(ctx);
				
}

static void	__stdcall error_func(int_t level, const wchar_t *msg, void *ctx)
{
		OutputDebugString(msg);

		CMultiMoveServerDlg		*dlg = (CMultiMoveServerDlg*)ctx;

		CString *log = new CString(msg);
		dlg->PostMessage(WM_LOGMSG, (WPARAM)log);

}



static void on_notify_func(void *ctx, const srvNotify_t	*notify)
{

		CMultiMoveServerDlg		*dlg = (CMultiMoveServerDlg*)ctx;
		CString tmp;

		ASSERT(notify != NULL);

		if(notify)
		{
				srvNotify_t *pnf = new srvNotify_t;
				memcpy(pnf, notify, sizeof(*notify));
				if(dlg)
				{
						dlg->PostMessage(WM_NOTIFYMSG, (WPARAM)pnf);
				}
		}


}





/******************************************************Utility******************************************************/

#if defined(_UNICODE)
		#define lsplitpath		_wsplitpath
#else
		#define lsplitpath		_splitpath
#endif


CString UTIL_GetModulePath()
{
		CString path;
		static TCHAR buf[MAX_PATH * 2];
		VERIFY(::GetModuleFileName(GetModuleHandle(NULL), buf, MAX_PATH * 2) > 0);
		
		static TCHAR d[5], dir[MAX_PATH];

		lsplitpath(buf, d, dir,NULL,NULL);

		path.Append(d);
		path.Append(dir);
		return path;
}


/************************************************************************************************************/
// CMultiMoveServerDlg dialog

CString CMultiMoveServerDlg::get_dlg_ip()const
{
		struct in_addr addr;
		m_srv_ip.GetAddress(addr.S_un.S_addr);
		
		addr.S_un.S_addr = COM_BYTEFLIP_U32(addr.S_un.S_addr);

		char *ip = inet_ntoa(addr);

		wchar_t wip[128];

		size_t wn = Com_str_to_wcs(COM_CP_ACP, ip, strlen(ip), wip, 128);

		if(wn > 0)
		{
				wip[wn] = 0;
		}else
		{
				wcscpy(wip, L"0.0.0.0");
		}

		return CString(wip);
}


void CMultiMoveServerDlg::get_dlg_port_range(DWORD &beg, DWORD &end)const
{
		CString tmp;
		
		m_srv_port_start.GetWindowText(tmp);
		beg = _wtoi64(tmp);

		m_srv_port_end.GetWindowText(tmp);
		end = _wtoi64(tmp);

		if(end < beg)
		{
				end = beg;
		}
}


#define MM_SRV_CONFIG	TEXT("server_config.ini")

#define MM_SRV_CONFIG_SEC	TEXT("Server")
#define MM_SRV_CONFIG_IP	TEXT("IP")
#define MM_SRV_CONFIG_PORT_START	TEXT("Port Start")
#define MM_SRV_CONFIG_PORT_END	TEXT("Port End")

void CMultiMoveServerDlg::init_config_files()
{
		
		bool_t is_ok;
		CString path = UTIL_GetModulePath();

		path += MM_SRV_CONFIG;

		cmString_t *out = Com_CreateString();

		is_ok = Com_LoadBomTextFile((const wchar_t*)path, NULL, out);

		if(is_ok)
		{
				Ini_ClearObject(m_cfg);
				is_ok = Ini_LoadObjectFromString(m_cfg, Com_GetStrString(out));
		}

		Com_DestroyString(out);
		out = NULL;
}

void	CMultiMoveServerDlg::save_config()
{
		
		CString ip = this->get_dlg_ip();

		Ini_SetString(m_cfg, MM_SRV_CONFIG_SEC, MM_SRV_CONFIG_IP, (const wchar_t*)ip, NULL);
		
		CString tmp;
		DWORD beg,end;

		get_dlg_port_range(beg, end);

		Ini_SetInt(m_cfg, MM_SRV_CONFIG_SEC, MM_SRV_CONFIG_PORT_START, beg, NULL);
		Ini_SetInt(m_cfg, MM_SRV_CONFIG_SEC, MM_SRV_CONFIG_PORT_END, end, NULL);

////////////////////////////////////////////////////////////////

		cmString_t *out = Com_CreateString();

		Ini_SaveObjectToString(m_cfg, out);

		CString path = UTIL_GetModulePath();
		path += MM_SRV_CONFIG;

		Com_SaveBomTextFile((const wchar_t*)path, COM_TXT_BOM_UTF_8, Com_GetStrString(out));

		if(out)
		{
				Com_DestroyString(out);
				out = NULL;
		}
		
}




void CMultiMoveServerDlg::init_dlg_items()
{
		unsigned long addr = 0;

		char ip[128];

		const wchar_t *wip = Ini_GetString(m_cfg, MM_SRV_CONFIG_SEC, MM_SRV_CONFIG_IP);
		if(wip == NULL)
		{
				wip = L"0.0.0.0";
		}

		size_t wn = Com_wcs_to_str(COM_CP_ACP, wip, wcslen(wip), ip, 128);

		if(wn > 0)
		{
				ip[wn] = 0;
		}

		addr = inet_addr(ip);

		m_srv_ip.SetAddress(COM_BYTEFLIP_U32((DWORD)addr));


		DWORD beg = Ini_GetUInt(m_cfg, MM_SRV_CONFIG_SEC, MM_SRV_CONFIG_PORT_START, 0);
		DWORD end = Ini_GetUInt(m_cfg, MM_SRV_CONFIG_SEC, MM_SRV_CONFIG_PORT_END, 65535);

		if(beg > end)
		{
				beg = end;
		}

		CString tmp;
		tmp.Format(TEXT("%d"), beg);
		m_srv_port_start.SetWindowText(tmp);

		tmp.Format(TEXT("%d"), end);
		m_srv_port_end.SetWindowText(tmp);


		m_srv_start.EnableWindow(TRUE);
		m_srv_stop.EnableWindow(FALSE);

		this->OnClientConnected(TEXT("No client connected."));
}



CMultiMoveServerDlg::CMultiMoveServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMultiMoveServerDlg::IDD, pParent)
	, m_is_hide(FALSE)
	, m_cfg(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_cfg = Ini_CreateObject();
}



void CMultiMoveServerDlg::OnNcDestroy()
{
		
		// TODO: Add your message handler code here

	
		if(!Server_UnInit())
		{
				this->MessageBox(TEXT("server core failed to uninitialize"));
				AfxAbort();
		}

		Shell_NotifyIcon(NIM_DELETE, &m_nid);

		if(m_cfg)
		{
				Ini_DestroyObject(m_cfg);
				m_cfg = NULL;
		}

		CDialogEx::OnNcDestroy();
}


void CMultiMoveServerDlg::DoDataExchange(CDataExchange* pDX)
{
		CDialogEx::DoDataExchange(pDX);
		DDX_Control(pDX, IDC_IPADDRESS_SRVIP, m_srv_ip);
		DDX_Control(pDX, IDC_EDIT_SRVPORT_BEG, m_srv_port_start);
		DDX_Control(pDX, IDC_EDIT_SRVPORT_END, m_srv_port_end);
		DDX_Control(pDX, IDC_BUTTON_SRVSTART, m_srv_start);
		DDX_Control(pDX, IDC_BUTTON_SRVSTOP, m_srv_stop);
		DDX_Control(pDX, IDC_EDIT_LOG, m_log);

		DDX_Control(pDX, IDC_EDIT_CLIADDR, m_client_addr);
}

BEGIN_MESSAGE_MAP(CMultiMoveServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CMultiMoveServerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CMultiMoveServerDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_QUIT, &CMultiMoveServerDlg::OnBnClickedButtonQuit)

	ON_BN_CLICKED(IDC_CUS_SHOW_DIALOG, &CMultiMoveServerDlg::OnShowDlg)
	ON_BN_CLICKED(IDC_CUS_HIDE_DIALOG, &CMultiMoveServerDlg::OnHideDlg)
	
	ON_BN_CLICKED(IDC_CUS_SHOW_ABOUT, &CMultiMoveServerDlg::OnAbout)

	ON_MESSAGE(WM_SHOWTASK,OnShowTask)
	ON_MESSAGE(WM_LOGMSG,OnLogMsg)
	ON_MESSAGE(WM_NOTIFYMSG, OnNotifyMsg)
	
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_NCDESTROY()
	ON_BN_CLICKED(IDC_BUTTON_SRVSTART, &CMultiMoveServerDlg::OnBnClickedButtonSrvstart)
	ON_BN_CLICKED(IDC_BUTTON_SRVSTOP, &CMultiMoveServerDlg::OnBnClickedButtonSrvstop)
END_MESSAGE_MAP()


// CMultiMoveServerDlg message handlers

BOOL CMultiMoveServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	
	init_config_files();

	init_dlg_items();

	if(!init_systray())
	{
			this->MessageBox(TEXT("systray failed to initialize"));
			return FALSE;
	}


	
	serverInit_t init;
	init.cm_init.io_ctx.on_print = print_func;
	init.cm_init.io_ctx.on_error = error_func;
	init.cm_init.io_ctx.ctx = (void*)this;

	init.srv_init.on_notify = on_notify_func;
	init.srv_init.ctx = (void*)this;

	if(!Server_Init(&init))
	{
			this->MessageBox(TEXT("server core failed to initialize"));
			return FALSE;
	}


	

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMultiMoveServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMultiMoveServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMultiMoveServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMultiMoveServerDlg::OnBnClickedOk()
{
		// TODO: Add your control notification handler code here
		//CDialogEx::OnOK();


		this->hide_dlg();
}


void CMultiMoveServerDlg::OnBnClickedCancel()
{
		// TODO: Add your control notification handler code here
		//CDialogEx::OnCancel();

		this->hide_dlg();
}


void CMultiMoveServerDlg::OnBnClickedButtonQuit()
{
		// TODO: Add your control notification handler code here

		OnBnClickedButtonSrvstop();
		save_config();
		CDialogEx::OnOK();
}



void CMultiMoveServerDlg::OnSize(UINT nType, int cx, int cy)
{


		CDialogEx::OnSize(nType, cx, cy);

		// TODO: Add your message handler code here
}


void CMultiMoveServerDlg::OnShowDlg()
{
		this->show_dlg();
}

void CMultiMoveServerDlg::OnHideDlg()
{
		this->hide_dlg();
		
}

void CMultiMoveServerDlg::OnAbout()
{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
}


LRESULT CMultiMoveServerDlg::OnShowTask(WPARAM wParam, LPARAM lParam)
{
		if(wParam != IDR_MAINFRAME)
		{
				return 1;
		}

		switch(lParam)
		{
		case WM_RBUTTONUP:                                //右键起来时弹出菜单
		{
				LPPOINT lpoint = new tagPOINT;
				::GetCursorPos(lpoint);                   //得到鼠标位置
				CMenu menu;
				menu.CreatePopupMenu();                   //声明一个弹出式菜单


				
				menu.AppendMenu(MF_STRING, IDC_CUS_SHOW_ABOUT, TEXT("About"));
				menu.AppendMenu(MF_SEPARATOR);

				menu.AppendMenu(MF_STRING, IDC_BUTTON_SRVSTART, TEXT("Start"));
				menu.AppendMenu(MF_STRING, IDC_BUTTON_SRVSTOP, TEXT("Stop"));
				menu.AppendMenu(MF_SEPARATOR);

				if(is_hide())
				{
						menu.AppendMenu(MF_STRING, IDC_CUS_SHOW_DIALOG, TEXT("Show"));
				}else
				{
						menu.AppendMenu(MF_STRING, IDC_CUS_HIDE_DIALOG, TEXT("Hide"));
				}

				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING, IDC_BUTTON_QUIT, TEXT("Quit"));


				menu.TrackPopupMenu(TPM_LEFTALIGN, lpoint->x ,lpoint->y, this);

				
				
				HMENU hmenu = menu.Detach();
				menu.DestroyMenu();
				delete lpoint;
		}
				break;
		case WM_LBUTTONDBLCLK:                                 // 双击左键的处理
		{
				this->show_dlg();
		}
				break;
		}
		return 0;
}




LRESULT CMultiMoveServerDlg::OnLogMsg(WPARAM wParam, LPARAM lParam)
{
		CString *msg = (CString*)wParam;
		if(msg)
		{
				this->AppendLog(*msg);
				delete msg;
				msg = NULL;
		}
		return 0;
}

void CMultiMoveServerDlg::OnDestroy()
{
		CDialogEx::OnDestroy();

		// TODO: Add your message handler code here
		
}


void CMultiMoveServerDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
		CDialogEx::OnSizing(fwSide, pRect);

		// TODO: Add your message handler code here
}



/***************************************************************************/
BOOL	CMultiMoveServerDlg::init_systray()
{
		m_nid.cbSize  = (DWORD)sizeof(NOTIFYICONDATA);
		m_nid.hWnd    = this->m_hWnd;
		m_nid.uID     = IDR_MAINFRAME;
		m_nid.uFlags  = NIF_ICON | NIF_MESSAGE | NIF_TIP ;
		m_nid.uCallbackMessage = WM_SHOWTASK;             // 自定义的消息名称
		m_nid.hIcon   = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));
		lstrcpy(m_nid.szTip, TEXT("MultiMove Server"));                // 信息提示条为"MultiMove Server"
		return Shell_NotifyIcon(NIM_ADD, &m_nid);                // 在托盘区添加图标
}


void	CMultiMoveServerDlg::show_dlg()
{
		if(m_is_hide)
		{
				m_is_hide = FALSE;
				this->ShowWindow(SW_SHOWNORMAL);
		}
}

void	CMultiMoveServerDlg::hide_dlg()
{
		if(!m_is_hide)
		{
				m_is_hide = TRUE;
				this->ShowWindow(SW_HIDE);
		}
}

BOOL	CMultiMoveServerDlg::is_hide()const
{
		return m_is_hide;
}






BOOL CMultiMoveServerDlg::PreTranslateMessage(MSG* pMsg)
{
		// TODO: Add your specialized code here and/or call the base class

		

		return   CDialog::PreTranslateMessage(pMsg); 
} 





void CMultiMoveServerDlg::OnBnClickedButtonSrvstart()
{
		// TODO: Add your control notification handler code here

		if(Srv_IsStarted())
		{
				return;
		}
		m_log.SetWindowText(TEXT(""));
		CString ip = this->get_dlg_ip();
		DWORD beg, end;

		this->get_dlg_port_range(beg, end);

		const wchar_t *inip = NULL;

		if(ip.Compare(TEXT("0.0.0.0")) == 0)
		{
				inip = NULL;
		}else
		{
				inip = (const wchar_t*)ip;
		}


		if(Srv_Start((const wchar_t*)inip, beg, end))
		{
				m_srv_start.EnableWindow(FALSE);
				m_srv_stop.EnableWindow(TRUE);

				m_srv_ip.EnableWindow(FALSE);
				m_srv_port_start.EnableWindow(FALSE);
				m_srv_port_end.EnableWindow(FALSE);
		}else
		{

		}
		

}


void CMultiMoveServerDlg::OnBnClickedButtonSrvstop()
{
		// TODO: Add your control notification handler code here
		if(!Srv_IsStarted())
		{
				return;
		}

		if(Srv_Stop())
		{
				m_srv_start.EnableWindow(TRUE);
				m_srv_stop.EnableWindow(FALSE);

				m_srv_ip.EnableWindow(TRUE);
				m_srv_port_start.EnableWindow(TRUE);
				m_srv_port_end.EnableWindow(TRUE);
		}
}



void	CMultiMoveServerDlg::AppendLog(const CString &str)
{
		int nLen = m_log.GetWindowTextLength ();
		m_log.SetSel (nLen,nLen);
		m_log.ReplaceSel (str);

}
		
void	CMultiMoveServerDlg::OnClientConnected(const CString &addr_info)
{
		m_client_addr.SetWindowText(addr_info);
}


LRESULT CMultiMoveServerDlg::OnNotifyMsg(WPARAM wParam, LPARAM lParam)
{
		srvNotify_t *notify = (srvNotify_t*)wParam;
		ASSERT(wParam != NULL);

		CString tmp;
		
		switch(notify->t)
		{
		case SRV_NOTIFY_ON_LISTEN:
		{
				
				tmp.Append(TEXT("on listen:\r\n"));

				for(size_t i = 0; i < notify->on_listen.bind_ip_cnt; ++i)
				{
						tmp.AppendFormat(L"%ls:%d\r\n", notify->on_listen.bind_ip[i].ip, notify->on_listen.listen_port);
				}
				tmp.AppendFormat(L"\r\n");
		}
				break;
		case SRV_NOTIFY_ON_LOGIN:
		{
				tmp.AppendFormat(L"%ls:%d Login\r\n", notify->on_login.remote_ip, notify->on_login.remote_port);

				CString addr_info;

				addr_info.Format(TEXT("%s : %d"), notify->on_login.remote_ip, notify->on_login.remote_port);
				this->OnClientConnected(addr_info);
		}
				break;
		case SRV_NOTIFY_ON_LOGOFF:
		{
				tmp.AppendFormat(L"%ls:%d LogOff\r\n", notify->on_logoff.remote_ip, notify->on_logoff.remote_port);
				this->OnClientConnected(TEXT("No client connected."));
		}
				break;
		case SRV_NOTIFY_ON_ENTER:
		{
				tmp.AppendFormat(L"%ls:%d Enter\r\n", notify->on_enter.remote_ip, notify->on_enter.remote_port);
		}
				break;
		case SRV_NOTIFY_ON_LEAVE:
		{
				tmp.AppendFormat(L"%ls:%d Leave\r\n", notify->on_leave.remote_ip, notify->on_leave.remote_port);
		}
				break;
		case SRV_NOTIFY_ON_CLIPDATA:
		{
				tmp.AppendFormat(L"%ls:%d Received clipboard data\r\n", notify->on_recv_clipdata.remote_ip, notify->on_recv_clipdata.remote_port);
		}
				break;
		default:
				Com_ASSERT(false);
				break;
		}

		this->AppendLog(tmp);

		delete notify;
		notify = NULL;

		return 0;
}