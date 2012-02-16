
// MultiMoveClient.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif


#include "resource.h"		// main symbols


// CMultiMoveClientApp:
// See MultiMoveClient.cpp for the implementation of this class
//

class CMultiMoveClientApp : public CWinApp
{
public:
	CMultiMoveClientApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CMultiMoveClientApp theApp;