
// DmTraceViewerView.h : interface of the CDmTraceViewerView class
//

#pragma once

#include "DmTraceViewerDoc.h"

class CDmTraceViewerView
    : public CScrollView
    , public DmTraceControl
{
protected: // create from serialization only
	CDmTraceViewerView();
	DECLARE_DYNCREATE(CDmTraceViewerView)
// Types

    class CDmTimeLineView : public TimeLineView
    {
    public:
        CDmTimeLineView(DmTraceControl* pParent)
            : TimeLineView(pParent)
        {}
        virtual void draw(void* context);
        virtual void redraw();
    };

    class CDmTimescaleView : public TimeScaleView
    {
    public:
        CDmTimescaleView(DmTraceControl* pParent)
            : TimeScaleView(pParent)
        {}
        virtual void draw(void* context);
        virtual void redraw();
    };

    class CDmThreadLabelView : public ThreadLabelView
    {
    public:
        CDmThreadLabelView(DmTraceControl* pParent)
            : ThreadLabelView(pParent)
        {}
        virtual void draw(void* context);
        virtual void redraw();
    };

// Attributes
public:
	CDmTraceViewerDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	void OnDraw(CDC* pDC);  // overridden to draw this view
	BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void draw(void* context);
    virtual void redraw();

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
    afx_msg void OnSize(UINT nType, int cx, int cy);
    virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
    void DrawGrid(CDC* pDC, DmTraceModel* pReader, DmTraceControl* pTimeLineview);
    void DrawThread(CDC* pDC, ThreadData* pThread, int nBaseLine);
public:
};

#ifndef _DEBUG  // debug version in DmTraceViewerView.cpp
inline CDmTraceViewerDoc* CDmTraceViewerView::GetDocument() const
   { return reinterpret_cast<CDmTraceViewerDoc*>(m_pDocument); }
#endif

inline bool doesRectIntersect(LPRECT rect1, Android::Rectangle& rect2)
{
    if (rect1->left > rect2.topleft.x + rect2.size.cx ||
        rect1->right < rect2.topleft.x ||
        rect1->top > rect2.topleft.y + rect2.size.cy ||
        rect1->bottom < rect2.topleft.y)
        return false;
    return true;
}
