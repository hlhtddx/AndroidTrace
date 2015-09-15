
// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"
#include "DmTraceViewer.h"

#include "ChildFrm.h"
#include "ThreadLabelView.h"
#include "DmTraceViewerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)
END_MESSAGE_MAP()

// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	// TODO: add member initialization code here
}

CChildFrame::~CChildFrame()
{
}


BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs
	if( !CMDIChildWndEx::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWndEx::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWndEx::Dump(dc);
}
#endif //_DEBUG

// CChildFrame message handlers


BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
    // create splitter window
    if (!m_wndSplitter.CreateStatic(this, 1, 2))
        return FALSE;

    if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CThreadLabelView), CSize(100, 100), pContext) ||
        !m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CDmTraceViewerView), CSize(100, 100), pContext))
    {
        m_wndSplitter.DestroyWindow();
        return FALSE;
    }
    return TRUE;
}
