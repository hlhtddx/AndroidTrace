// ThreadLabelView.cpp : 实现文件
//

#include "stdafx.h"
#include "DmTraceViewer.h"
#include "ThreadLabelView.h"


// CThreadLabelView

IMPLEMENT_DYNCREATE(CThreadLabelView, CView)

CThreadLabelView::CThreadLabelView()
    : ThreadLabelView(NULL)
{

}

CThreadLabelView::~CThreadLabelView()
{
}

BEGIN_MESSAGE_MAP(CThreadLabelView, CView)
END_MESSAGE_MAP()

#ifdef _DEBUG
CDmTraceViewerDoc* CThreadLabelView::GetDocument() const // non-debug version is inline
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDmTraceViewerDoc)));
    return (CDmTraceViewerDoc*)m_pDocument;
}
#endif

void CThreadLabelView::OnDraw(CDC* pDC)
{
    CBrush brDark(RGB(0xd0, 0xd0, 0xff));
    pDC->SetBkMode(TRANSPARENT);
    pDC->SetTextAlign(TA_RIGHT | TA_TOP);

    int cx = mBoundRect.size.cx - labelMarginX;
    CRect rcThread = CRect(mBoundRect.topleft.x, mBoundRect.topleft.y,
        mBoundRect.topleft.x + mBoundRect.size.cx, mBoundRect.topleft.y + mParent->rowYSpace);

    CDmTraceViewerDoc* pDoc = GetDocument();

    ThreadPtrList* sortedThreads = pDoc->getThreads();
    for (int nThread = pDoc->mStartRow; nThread < pDoc->mEndRow; nThread++) {
        if ((nThread & 0x1) == 0) {
            pDC->FillRect(rcThread, &brDark);
        }

        ThreadData* thread = sortedThreads->at(nThread);
        CString strThreadLabel;
        CA2T szThreadName(thread->getName());
        pDC->TextOutW(cx, rcThread.top + pDoc->rowYMarginHalf, szThreadName.m_psz);

        rcThread.top += pDoc->rowYSpace;
        rcThread.bottom += pDoc->rowYSpace;
    }
}


// CThreadLabelView 诊断

#ifdef _DEBUG
void CThreadLabelView::AssertValid() const
{
	CView::AssertValid();
}

#ifndef _WIN32_WCE
void CThreadLabelView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif
#endif //_DEBUG


// CThreadLabelView 消息处理程序
