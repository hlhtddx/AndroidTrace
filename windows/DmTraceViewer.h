
// DmTraceViewer.h : main header file for the DmTraceViewer application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CDmTraceViewerApp:
// See DmTraceViewer.cpp for the implementation of this class
//

class CDmTraceViewerApp : public CWinAppEx
{
public:
	CDmTraceViewerApp();


// Overrides
public:
	BOOL InitInstance();
	int ExitInstance();

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	void PreLoadState();
	void LoadCustomState();
	void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CDmTraceViewerApp theApp;
