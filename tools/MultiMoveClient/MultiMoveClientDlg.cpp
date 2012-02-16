
// MultiMoveClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MultiMoveClient.h"
#include "MultiMoveClientDlg.h"
#include "afxdialogex.h"
#include "AboutDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif




static void	__stdcall __print_func(const wchar_t *msg, void *ctx)
{
		UNUSED(ctx);
		OutputDebugString(msg);
		
		/*
		CMultiMoveServerDlg		*dlg = (CMultiMoveServerDlg*)ctx;

		CString *log = new CString(msg);
		dlg->PostMessage(WM_LOGMSG, (WPARAM)log);
		*/

			
}

static void	__stdcall __error_func(int_t level, const wchar_t *msg, void *ctx)
{
		UNUSED(level);
		UNUSED(ctx);

		OutputDebugString(msg);

		CMultiMoveClientDlg		*dlg = (CMultiMoveClientDlg*)ctx;

		if(dlg)
		{
				CString *log = new CString(msg);
				dlg->PostMessage(WM_LOGMSG, (WPARAM)log);
		}

}


static void __client_notify(void *ctx, const cliNotify_t	*notify)
{
		CMultiMoveClientDlg		*dlg = (CMultiMoveClientDlg*)ctx;
		Com_ASSERT(notify != NULL);

		/*
		

		switch(notify->t)
		{
		case CLI_NOTIFY_ON_CONNECTED:
		{
				Com_printf(L"server %ls:%d connected\r\n",notify->on_connected.ip, notify->on_connected.port);
		}
				break;
		case CLI_NOTIFY_ON_DISCONNECTED:
		{
				Com_printf(L"server %ls:%d disconnected\r\n",notify->on_disconnected.ip, notify->on_disconnected.port);
		}
				break;
		case CLI_NOTIFY_ON_ACTIVE:
		{
				printf("server %ls:%d active\r\n",notify->on_active.ip, notify->on_active.port);
		}				
				break;
		case CLI_NOTIFY_ON_DEACTIVE:
		{
				Com_printf(L"server %ls:%d deactive\r\n",notify->on_deactive.ip, notify->on_deactive.port);
		}
				break;
		case CLI_NOTIFY_ON_CLIPDATA:
		{
				Com_printf(L"received server %ls:%d clipdata\r\n",notify->on_clipdata.ip, notify->on_clipdata.port);
		}
				break;
		default:
				Com_ASSERT(false);
				break;
		}
		*/
		

		
		
		if(dlg)
		{
				cliNotify_t *pnfy = new cliNotify_t;
				memcpy(pnfy, notify, sizeof(*notify));
				dlg->PostMessage(WM_NOTIFYMSG, (WPARAM)pnfy);
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






/******************************************************CMultiMoveClientDlg******************************************************/


#define MM_CLI_CONFIG	TEXT("client_config.ini")

#define MM_CLI_CONFIG_SEC		TEXT("Client")

#define MM_CLI_CONFIG_UP_IP		TEXT("UP IP")
#define MM_CLI_CONFIG_UP_PORT	TEXT("UP PORT")

#define MM_CLI_CONFIG_DOWN_IP	TEXT("DOWN IP")
#define MM_CLI_CONFIG_DOWN_PORT	TEXT("DOWN PORT")

#define MM_CLI_CONFIG_LEFT_IP	TEXT("LEFT IP")
#define MM_CLI_CONFIG_LEFT_PORT	TEXT("LEFT PORT")

#define MM_CLI_CONFIG_RIGHT_IP			TEXT("RIGHT IP")
#define MM_CLI_CONFIG_RIGHT_PORT		TEXT("RIGHT PORT")


BOOL	CMultiMoveClientDlg::init_systray()
{
		m_is_hide = FALSE;

		m_nid.cbSize  = (DWORD)sizeof(NOTIFYICONDATA);
		m_nid.hWnd    = this->m_hWnd;
		m_nid.uID     = IDR_MAINFRAME;
		m_nid.uFlags  = NIF_ICON | NIF_MESSAGE | NIF_TIP ;
		m_nid.uCallbackMessage = WM_SHOWTASK;									// 自定义的消息名称
		m_nid.hIcon   = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));
		lstrcpy(m_nid.szTip, TEXT("MultiMove Server"));							// 信息提示条为"MultiMove Client"
		return Shell_NotifyIcon(NIM_ADD, &m_nid);								// 在托盘区添加图标
}
		
void	CMultiMoveClientDlg::uninit_systray()
{
		Shell_NotifyIcon(NIM_DELETE, &m_nid);								
}

BOOL	CMultiMoveClientDlg::is_hide()const
{
		return m_is_hide;
}


void	CMultiMoveClientDlg::show_dlg()
{
		if(m_is_hide)
		{
				m_is_hide = FALSE;
				this->ShowWindow(SW_SHOWNORMAL);
		}
}

void	CMultiMoveClientDlg::hide_dlg()
{
		if(!m_is_hide)
		{
				m_is_hide = TRUE;
				this->ShowWindow(SW_HIDE);
		}
}



void	CMultiMoveClientDlg::init_config()
{
		m_cfg = Ini_CreateObject();

		bool_t is_ok;
		CString path = UTIL_GetModulePath();
		path += MM_CLI_CONFIG;

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



void	CMultiMoveClientDlg::uninit_config()
{

		if(m_cfg)
		{
				Ini_DestroyObject(m_cfg);
				m_cfg = NULL;
		}

}




unsigned long inet_addr_wcs(const wchar_t *input)
{
		if(input == NULL)
		{
				return INADDR_NONE;
		}

		size_t l = wcslen(input) * 2+ 1;
		char *tmp = new char[l];

		size_t n = Com_wcs_to_str(COM_CP_ACP, input, wcslen(input), tmp, l - 1);

		tmp[n] = 0;
		
		unsigned long ret = inet_addr(tmp);
		delete tmp;
		tmp = NULL;
		return ret;
}

void	CMultiMoveClientDlg::save_config()
{
		CString ip_str;
		CString port_str;

		m_up_ip.GetWindowText(ip_str);
		m_up_port.GetWindowText(port_str);

		if(inet_addr_wcs(ip_str) == INADDR_NONE)
		{
				ip_str = TEXT("0.0.0.0");
		}

		Ini_SetString(m_cfg, MM_CLI_CONFIG_SEC, MM_CLI_CONFIG_UP_IP, (const wchar_t*)ip_str, NULL);
		Ini_SetString(m_cfg, MM_CLI_CONFIG_SEC, MM_CLI_CONFIG_UP_PORT, (const wchar_t*)port_str, NULL);


		m_down_ip.GetWindowText(ip_str);
		m_down_port.GetWindowText(port_str);
		
		if(inet_addr_wcs(ip_str) == INADDR_NONE)
		{
				ip_str = TEXT("0.0.0.0");
		}

		Ini_SetString(m_cfg, MM_CLI_CONFIG_SEC, MM_CLI_CONFIG_DOWN_IP, (const wchar_t*)ip_str, NULL);
		Ini_SetString(m_cfg, MM_CLI_CONFIG_SEC, MM_CLI_CONFIG_DOWN_PORT, (const wchar_t*)port_str, NULL);


		m_left_ip.GetWindowText(ip_str);
		m_left_port.GetWindowText(port_str);
		if(inet_addr_wcs(ip_str) == INADDR_NONE)
		{
				ip_str = TEXT("0.0.0.0");
		}

		Ini_SetString(m_cfg, MM_CLI_CONFIG_SEC, MM_CLI_CONFIG_LEFT_IP, (const wchar_t*)ip_str, NULL);
		Ini_SetString(m_cfg, MM_CLI_CONFIG_SEC, MM_CLI_CONFIG_LEFT_PORT, (const wchar_t*)port_str, NULL);

		m_right_ip.GetWindowText(ip_str);
		m_right_port.GetWindowText(port_str);
		if(inet_addr_wcs(ip_str) == INADDR_NONE)
		{
				ip_str = TEXT("0.0.0.0");
		}
		Ini_SetString(m_cfg, MM_CLI_CONFIG_SEC, MM_CLI_CONFIG_RIGHT_IP, (const wchar_t*)ip_str, NULL);
		Ini_SetString(m_cfg, MM_CLI_CONFIG_SEC, MM_CLI_CONFIG_RIGHT_PORT, (const wchar_t*)port_str, NULL);


/*****************************************************************************/
		cmString_t *out = Com_CreateString();

		Ini_SaveObjectToString(m_cfg, out);

		CString path = UTIL_GetModulePath();
		path += MM_CLI_CONFIG;

		Com_SaveBomTextFile((const wchar_t*)path, COM_TXT_BOM_UTF_8, Com_GetStrString(out));

		if(out)
		{
				Com_DestroyString(out);
				out = NULL;
		}
}




void	CMultiMoveClientDlg::init_dlg_items()
{
		
		const wchar_t *ip_str;
		uint_64_t port;
		CString port_str;
		

		ip_str = Ini_GetString(m_cfg, MM_CLI_CONFIG_SEC, MM_CLI_CONFIG_UP_IP);
		port   = Ini_GetUInt(m_cfg, MM_CLI_CONFIG_SEC, MM_CLI_CONFIG_UP_PORT, 0);

		port_str.Format(TEXT("%I64u"), port);

		if(ip_str == NULL || inet_addr_wcs(ip_str) == INADDR_NONE)
		{
				ip_str = TEXT("0.0.0.0");
		}
		
		m_up_ip.SetWindowText(ip_str);
		m_up_port.SetWindowText(port_str);

		/***************************************Down*****************************/

		ip_str = Ini_GetString(m_cfg, MM_CLI_CONFIG_SEC, MM_CLI_CONFIG_DOWN_IP);
		port   = Ini_GetUInt(m_cfg, MM_CLI_CONFIG_SEC, MM_CLI_CONFIG_DOWN_PORT, 0);

		port_str.Format(TEXT("%I64u"), port);

		if(ip_str == NULL || inet_addr_wcs(ip_str) == INADDR_NONE)
		{
				ip_str = TEXT("0.0.0.0");
		}

		m_down_ip.SetWindowText(ip_str);
		m_down_port.SetWindowText(port_str);

		/***************************************Left*****************************/


		ip_str = Ini_GetString(m_cfg, MM_CLI_CONFIG_SEC, MM_CLI_CONFIG_LEFT_IP);
		port   = Ini_GetUInt(m_cfg, MM_CLI_CONFIG_SEC, MM_CLI_CONFIG_LEFT_PORT, 0);

		port_str.Format(TEXT("%I64u"), port);

		if(ip_str == NULL || inet_addr_wcs(ip_str) == INADDR_NONE)
		{
				ip_str = TEXT("0.0.0.0");
		}

		m_left_ip.SetWindowText(ip_str);
		m_left_port.SetWindowText(port_str);

		/***************************************Right*****************************/

		ip_str = Ini_GetString(m_cfg, MM_CLI_CONFIG_SEC, MM_CLI_CONFIG_RIGHT_IP);
		port   = Ini_GetUInt(m_cfg, MM_CLI_CONFIG_SEC, MM_CLI_CONFIG_RIGHT_PORT, 0);

		port_str.Format(TEXT("%I64u"), port);

		if(ip_str == NULL || inet_addr_wcs(ip_str) == INADDR_NONE)
		{
				ip_str = TEXT("0.0.0.0");
		}
	
		m_right_ip.SetWindowText(ip_str);
		m_right_port.SetWindowText(port_str);

}
		

void	CMultiMoveClientDlg::init_client()
{
		ioCtx_t	ctx;
		ctx.on_error = __error_func;
		ctx.on_print = __print_func;
		ctx.ctx = (void*)this;

		cmInit_t cm_init;
		cm_init.io_ctx = ctx;

		cliInit_t init;
		init.ctx = (void*)this;
		init.hide_cursor =  ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_CURSOR_BLANK));
		init.on_notify = __client_notify;


		clientInit_t client_init;
		client_init.cm_init = cm_init;
		client_init.cli_init = init;
		
		if(!Client_Init(&client_init))
		{
				AfxAbort();
		}


		if(!Cli_Start())
		{
				AfxAbort();
		}

}


void	CMultiMoveClientDlg::uninit_cliet()
{
		SystemParametersInfo(SPI_SETCURSORS,0,0,WM_SETTINGCHANGE | SPIF_UPDATEINIFILE );

		if(!Cli_Stop())
		{
				AfxAbort();
		}

		if(!Client_UnInit())
		{
				AfxAbort();
		}
}


// CMultiMoveClientDlg dialog




CMultiMoveClientDlg::CMultiMoveClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMultiMoveClientDlg::IDD, pParent)
{
		m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
		
		m_up_is_connected = FALSE;
		m_down_is_connected = FALSE;
		m_left_is_connected = FALSE;
		m_right_is_connected = FALSE;
}





void CMultiMoveClientDlg::DoDataExchange(CDataExchange* pDX)
{
		CDialogEx::DoDataExchange(pDX);
		DDX_Control(pDX, IDC_EDIT_LOG, m_log);
		DDX_Control(pDX, IDC_EDIT_UP_IP, m_up_ip);
		DDX_Control(pDX, IDC_EDIT_UP_PORT, m_up_port);
		DDX_Control(pDX, IDC_EDIT_DOWN_IP, m_down_ip);
		DDX_Control(pDX, IDC_EDIT_LEFT_IP, m_left_ip);
		DDX_Control(pDX, IDC_EDIT_RIGHT_IP, m_right_ip);
		DDX_Control(pDX, IDC_EDIT_DOWN_PORT, m_down_port);
		DDX_Control(pDX, IDC_EDIT_LEFT_PORT, m_left_port);
		DDX_Control(pDX, IDC_EDIT_RIGHT_PORT, m_right_port);
		DDX_Control(pDX, IDC_BUTTON_UP_CONNECT, m_up_connect);
		DDX_Control(pDX, IDC_BUTTON_DOWN_CONNECT, m_down_connect);
		DDX_Control(pDX, IDC_BUTTON_LEFT_CONNECT, m_left_connect);
		DDX_Control(pDX, IDC_BUTTON_RIGHT_CONNECT, m_right_connect);
}

BEGIN_MESSAGE_MAP(CMultiMoveClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_BN_CLICKED(IDC_CUS_SHOW_DIALOG, &OnShowDlg)
	ON_BN_CLICKED(IDC_CUS_HIDE_DIALOG, &OnHideDlg)
	
	ON_BN_CLICKED(IDC_CUS_SHOW_ABOUT, &OnAbout)

	ON_MESSAGE(WM_SHOWTASK,OnShowTask)
	ON_MESSAGE(WM_LOGMSG,OnLogMsg)
	ON_MESSAGE(WM_NOTIFYMSG,OnNotifyMsg)
	
	ON_WM_NCDESTROY()
	ON_BN_CLICKED(IDC_BUTTON_QUIT, &CMultiMoveClientDlg::OnBnClickedButtonQuit)
	ON_BN_CLICKED(IDOK, &CMultiMoveClientDlg::OnBnClickedOk)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_UP_CONNECT, &CMultiMoveClientDlg::OnBnClickedButtonUpConnect)
	ON_BN_CLICKED(IDC_BUTTON_DOWN_CONNECT, &CMultiMoveClientDlg::OnBnClickedButtonDownConnect)
	ON_BN_CLICKED(IDC_BUTTON_LEFT_CONNECT, &CMultiMoveClientDlg::OnBnClickedButtonLeftConnect)
	ON_BN_CLICKED(IDC_BUTTON_RIGHT_CONNECT, &CMultiMoveClientDlg::OnBnClickedButtonRightConnect)
END_MESSAGE_MAP()


// CMultiMoveClientDlg message handlers


BOOL CMultiMoveClientDlg::OnInitDialog()
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

	//ShowWindow(SW_SHOWNORMAL);

	// TODO: Add extra initialization here
		init_systray();
		init_config();
		init_dlg_items();
		init_client();


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMultiMoveClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMultiMoveClientDlg::OnPaint()
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
HCURSOR CMultiMoveClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}




void CMultiMoveClientDlg::OnShowDlg()
{
		this->show_dlg();
}

void CMultiMoveClientDlg::OnHideDlg()
{
		this->hide_dlg();
		
}

void CMultiMoveClientDlg::OnAbout()
{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
}



LRESULT CMultiMoveClientDlg::OnShowTask(WPARAM wParam, LPARAM lParam)
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
				UNUSED(hmenu);
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



LRESULT CMultiMoveClientDlg::OnNotifyMsg(WPARAM wParam, LPARAM lParam)
{
		cliNotify_t *notify = (cliNotify_t*)wParam;
		UNUSED(lParam);
		assert(wParam != NULL);

		CString  msg;

		switch(notify->t)
		{
		case CLI_NOTIFY_ON_CONNECTED:
		{
				msg.Format(L"server %ls:%d connected\r\n",notify->on_connected.ip, notify->on_connected.port);
		}
				break;
		case CLI_NOTIFY_ON_DISCONNECTED:
		{
				msg.Format(L"server %ls:%d disconnected\r\n",notify->on_disconnected.ip, notify->on_disconnected.port);
				
				switch(notify->on_disconnected.action_pos)
				{
				case NM_POS_UP:
						if(m_up_is_connected)
						{
								OnBnClickedButtonUpConnect();
						}
						break;
				case NM_POS_DOWN:
						if(m_down_is_connected)
						{
								OnBnClickedButtonDownConnect();
						}
						break;
				case NM_POS_LEFT:
						if(m_left_is_connected)
						{
								OnBnClickedButtonLeftConnect();
						}
						break;
				case NM_POS_RIGHT:
						if(m_right_is_connected)
						{
								OnBnClickedButtonRightConnect();
						}
						break;
				default:
						break;
				}

		}
				break;
		case CLI_NOTIFY_ON_ACTIVE:
		{
				msg.Format(L"server %ls:%d active\r\n",notify->on_active.ip, notify->on_active.port);
		}				
				break;
		case CLI_NOTIFY_ON_DEACTIVE:
		{
				msg.Format(L"server %ls:%d deactive\r\n",notify->on_deactive.ip, notify->on_deactive.port);
		}
				break;
		case CLI_NOTIFY_ON_CLIPDATA:
		{
				msg.Format(L"received server %ls:%d clipdata\r\n",notify->on_clipdata.ip, notify->on_clipdata.port);
		}
				break;
		default:
				Com_ASSERT(false);
				break;
		}

		this->AppendLog(msg);

		delete notify;
		notify = NULL;

		return 0;
}

LRESULT CMultiMoveClientDlg::OnLogMsg(WPARAM wParam, LPARAM lParam)
{
		UNUSED(lParam);
		CString *msg = (CString*)wParam;
		if(msg)
		{
				OutputDebugString(*msg);
				this->AppendLog(*msg);
				delete msg;
				msg = NULL;
		}
		return 0;
}



void CMultiMoveClientDlg::AppendLog(const CString &log)
{
		OutputDebugString(log);
		int nLen = m_log.GetWindowTextLength ();
		m_log.SetSel (nLen,nLen);
		m_log.ReplaceSel (log);
}


void CMultiMoveClientDlg::OnNcDestroy()
{
		
		
		
		uninit_cliet();
		uninit_config();
		uninit_systray();
		
		CDialogEx::OnNcDestroy();
}


void CMultiMoveClientDlg::OnBnClickedButtonQuit()
{
		// TODO: Add your control notification handler code here
		save_config();

		if(m_up_is_connected)
		{
				OnBnClickedButtonUpConnect();
		}

		if(m_down_is_connected)
		{
				OnBnClickedButtonDownConnect();
		}

		if(m_left_is_connected)
		{
				OnBnClickedButtonLeftConnect();
		}

		if(m_right_is_connected)
		{
				OnBnClickedButtonRightConnect();
		}


		CDialogEx::OnOK();
}


void CMultiMoveClientDlg::OnBnClickedOk()
{
		// TODO: Add your control notification handler code here
		this->OnHideDlg();
		//CDialogEx::OnOK();
}


void CMultiMoveClientDlg::OnClose()
{
		// TODO: Add your message handler code here and/or call default
		this->OnHideDlg();
		//CDialogEx::OnClose();
}





void CMultiMoveClientDlg::OnBnClickedButtonUpConnect()
{
		// TODO: Add your control notification handler code here

		if(!m_up_is_connected)
		{
				CString ip_str, port_str;

				m_up_ip.GetWindowText(ip_str);
				m_up_port.GetWindowText(port_str);

				if(ip_str.Compare(TEXT("0.0.0.0")) == 0 || inet_addr_wcs(ip_str) == INADDR_NONE || _wtoi(port_str) == 0)
				{
						CString msg;
						msg.Format(TEXT("Invalid address : '%s : %s'"), ip_str, port_str);
						this->MessageBox(msg);
						return;
				}


				if(!Cli_InsertServer(NM_POS_UP, ip_str, _wtoi(port_str)))
				{
						CString msg;
						msg.Format(TEXT("failed to connect : '%s : %s'"), ip_str, port_str);
						this->MessageBox(msg);
						return;
				}

				m_up_is_connected = TRUE;
				m_up_connect.SetWindowText(TEXT("Disconnect"));
		}else
		{
				m_up_is_connected = FALSE;
				m_up_connect.SetWindowText(TEXT("Connect"));
				Cli_RemoveServer(NM_POS_UP);
		}

}


void CMultiMoveClientDlg::OnBnClickedButtonDownConnect()
{
		if(!m_down_is_connected)
		{
				CString ip_str, port_str;

				m_down_ip.GetWindowText(ip_str);
				m_down_port.GetWindowText(port_str);

				if(ip_str.Compare(TEXT("0.0.0.0")) == 0 || inet_addr_wcs(ip_str) == INADDR_NONE || _wtoi(port_str) == 0)
				{
						CString msg;
						msg.Format(TEXT("Invalid address : '%s : %s'"), ip_str, port_str);
						this->MessageBox(msg);
						return;
				}


				if(!Cli_InsertServer(NM_POS_DOWN, ip_str, _wtoi(port_str)))
				{
						CString msg;
						msg.Format(TEXT("failed to connect : '%s : %s'"), ip_str, port_str);
						this->MessageBox(msg);
						return;
				}

				m_down_is_connected = TRUE;
				m_down_connect.SetWindowText(TEXT("Disconnect"));
		}else
		{
				m_down_is_connected = FALSE;
				m_down_connect.SetWindowText(TEXT("Connect"));
				Cli_RemoveServer(NM_POS_DOWN);
		}

}


void CMultiMoveClientDlg::OnBnClickedButtonLeftConnect()
{
		if(!m_left_is_connected)
		{
				CString ip_str, port_str;

				m_left_ip.GetWindowText(ip_str);
				m_left_port.GetWindowText(port_str);

				if(ip_str.Compare(TEXT("0.0.0.0")) == 0 || inet_addr_wcs(ip_str) == INADDR_NONE || _wtoi(port_str) == 0)
				{
						CString msg;
						msg.Format(TEXT("Invalid address : '%s : %s'"), ip_str, port_str);
						this->MessageBox(msg);
						return;
				}


				if(!Cli_InsertServer(NM_POS_LEFT, ip_str, _wtoi(port_str)))
				{
						CString msg;
						msg.Format(TEXT("failed to connect : '%s : %s'"), ip_str, port_str);
						this->MessageBox(msg);
						return;
				}

				m_left_is_connected = TRUE;
				m_left_connect.SetWindowText(TEXT("Disconnect"));
		}else
		{
				m_left_is_connected = FALSE;
				m_left_connect.SetWindowText(TEXT("Connect"));
				Cli_RemoveServer(NM_POS_LEFT);
		}
}


void CMultiMoveClientDlg::OnBnClickedButtonRightConnect()
{
		if(!m_right_is_connected)
		{
				CString ip_str, port_str;

				m_right_ip.GetWindowText(ip_str);
				m_right_port.GetWindowText(port_str);

				if(ip_str.Compare(TEXT("0.0.0.0")) == 0 || inet_addr_wcs(ip_str) == INADDR_NONE || _wtoi(port_str) == 0)
				{
						CString msg;
						msg.Format(TEXT("Invalid address : '%s : %s'"), ip_str, port_str);
						this->MessageBox(msg);
						return;
				}


				if(!Cli_InsertServer(NM_POS_RIGHT, ip_str, _wtoi(port_str)))
				{
						CString msg;
						msg.Format(TEXT("failed to connect : '%s : %s'"), ip_str, port_str);
						this->MessageBox(msg);
						return;
				}

				m_right_is_connected = TRUE;
				m_right_connect.SetWindowText(TEXT("Disconnect"));
		}else
		{
				m_right_is_connected = FALSE;
				m_right_connect.SetWindowText(TEXT("Connect"));
				Cli_RemoveServer(NM_POS_RIGHT);
		}
}
