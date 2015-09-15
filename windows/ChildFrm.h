
// ChildFrm.h : interface of the CChildFrame class
//

#pragma once

class CChildFrame : public CMDIChildWndEx
{
	DECLARE_DYNCREATE(CChildFrame)
public:
	CChildFrame();

// Attributes
public:
    CSplitterWndEx m_wndSplitter;

// Operations
public:

// Overrides
	BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	~CChildFrame();
#ifdef _DEBUG
	void AssertValid() const;
	void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
    virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
};
