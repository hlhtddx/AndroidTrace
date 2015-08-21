
// DmTraceViewerView.h : interface of the CDmTraceViewerView class
//

#pragma once


class CDmTraceViewerView : public CScrollView
{
protected: // create from serialization only
	CDmTraceViewerView();
	DECLARE_DYNCREATE(CDmTraceViewerView)

// Attributes
public:
	CDmTraceViewerDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	void OnDraw(CDC* pDC);  // overridden to draw this view
	BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	~CDmTraceViewerView();
#ifdef _DEBUG
	void AssertValid() const;
	void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
	//void DrawGrid(CDC* pDC);
public:
//	void DrawThread(CDC* pDC, Android::TraceFile* pTraceFile, int nThread);
};

#ifndef _DEBUG  // debug version in DmTraceViewerView.cpp
inline CDmTraceViewerDoc* CDmTraceViewerView::GetDocument() const
   { return reinterpret_cast<CDmTraceViewerDoc*>(m_pDocument); }
#endif

