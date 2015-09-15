
// DmTraceViewerView.cpp : implementation of the CDmTraceViewerView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "DmTraceViewer.h"
#endif

#include "DmTraceViewerDoc.h"
#include "DmTraceViewerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int nGridWidth		= 1000;
int nGridHeight		= 32;
int nGridMargin		= 5;
int nLabelWidth		= 200;
int nLabelHeight	= 100;

// CDmTraceViewerView

IMPLEMENT_DYNCREATE(CDmTraceViewerView, CScrollView)

BEGIN_MESSAGE_MAP(CDmTraceViewerView, CScrollView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
    ON_WM_SIZE()
END_MESSAGE_MAP()

// CDmTraceViewerView construction/destruction

CDmTraceViewerView::CDmTraceViewerView()
{
    mTimescale = new CDmTimescaleView(this);
    mTimeLine = new CDmTimeLineView(this);
    mThreadLabel = new CDmThreadLabelView(this);
}

CDmTraceViewerView::~CDmTraceViewerView()
{
    delete mTimescale;
    delete mTimeLine;
    delete mThreadLabel;
}

BOOL CDmTraceViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

// CDmTraceViewerView drawing

void CDmTraceViewerView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
    CView::OnPrepareDC(pDC, pInfo);
}

void CDmTraceViewerView::OnDraw(CDC* pDC)
{
	CDmTraceViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CPaintDC* paintDC = reinterpret_cast<CPaintDC*>(pDC);
    if (paintDC != nullptr) {
        CRect rcInvalid = paintDC->m_ps.rcPaint;
        pDC->FillSolidRect(&rcInvalid, RGB(0xF0, 0xF0, 0xF0));
        CPoint ptScroll = GetScrollPosition();
        draw(pDC);
    }
}

void CDmTraceViewerView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;

    CDmTraceViewerDoc* pReader = GetDocument();

    setData(pReader);

    CRect rcClient;
    GetClientRect(rcClient);

    int nRows = pReader->getThreads()->size();
    int nHeight = nRows * 32;

    TickScaler& scaleInfo = getScaleInfo();
    scaleInfo.setMinVal(pReader->getMinTime());
    scaleInfo.setMaxVal(pReader->getMaxTime());
    scaleInfo.setNumPixels(rcClient.Width());
    scaleInfo.computeTicks(false);
    computeVisibleRows(rcClient.Height());

    int nWidth = scaleInfo.getMaxVal() - scaleInfo.getMinVal();

    sizeTotal.cx = rcClient.Width();//nLabelWidth + nGridMargin * 2 + nWidth;
	sizeTotal.cy = nLabelHeight + nGridMargin * 2 + nHeight;
	SetScrollSizes(MM_TEXT, sizeTotal);
}

void CDmTraceViewerView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CDmTraceViewerView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CDmTraceViewerView diagnostics

#ifdef _DEBUG
void CDmTraceViewerView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CDmTraceViewerView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CDmTraceViewerDoc* CDmTraceViewerView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDmTraceViewerDoc)));
	return (CDmTraceViewerDoc*)m_pDocument;
}
#endif //_DEBUG

void CDmTraceViewerView::DrawGrid(CDC* pDC, DmTraceModel* pReader, DmTraceControl* pTimeLineview)
{
}

void CDmTraceViewerView::DrawThread(CDC* pDC, ThreadData* thread, int nBaseLine)
{
    StripList& stripList = thread->getStripList();
    for (auto _si = 0; _si < stripList.size(); _si++) {
        Strip* strip = stripList.get(_si);
        pDC->FillSolidRect(strip->mX + nLabelWidth, strip->mY + nLabelHeight, strip->mWidth, strip->mHeight, strip->mColor);
    }
}

void CDmTraceViewerView::OnSize(UINT nType, int cx, int cy)
{
    CScrollView::OnSize(nType, cx, cy);
    Android::Rectangle rcClient(0, 0, cx, cy);
    setBoundary(&rcClient);
}

void CDmTraceViewerView::draw(void* context)
{
    if (mDmTraceData == nullptr) {
        return;
    }

    CDC* pDC = reinterpret_cast<CDC*>(context);
    CPaintDC* paintDC = dynamic_cast<CPaintDC*>(pDC);

    if (paintDC != nullptr) {
        CRect rcUpdate = paintDC->m_ps.rcPaint;
        paintDC->FillSolidRect(&rcUpdate, RGB(0xF0, 0xF0, 0xF0));
        CPoint ptScroll = GetScrollPosition();

        Android::Rectangle rcTimeLine, rcTimeScale, rcThreadLabel;
        mTimeLine->getBoundary(&rcTimeLine);
        mTimescale->getBoundary(&rcTimeScale);
        mThreadLabel->getBoundary(&rcThreadLabel);

        if (doesRectIntersect(&rcUpdate, rcTimeLine)) {
            mTimeLine->draw(context);
        }

        if (doesRectIntersect(&rcUpdate, rcTimeScale)) {
            mTimescale->draw(context);
        }

        if (doesRectIntersect(&rcUpdate, rcThreadLabel)) {
            mThreadLabel->draw(context);
        }
    }
}

void CDmTraceViewerView::CDmTimeLineView::draw(void* context)
{
    CDC* pDC = reinterpret_cast<CDC*>(context);
    CBrush brDark(RGB(0xd0, 0xd0, 0xff));

    int cx = mBoundRect.size.cx;
    CRect rcThread = CRect(mBoundRect.topleft.x, mBoundRect.topleft.y,
        mBoundRect.topleft.x + mBoundRect.size.cx, mBoundRect.topleft.y + mParent->rowYSpace);

    ThreadPtrList* sortedThreads = mParent->getThreads();
    for (int nThread = mParent->mStartRow; nThread < mParent->mEndRow; nThread++) {
        if ((nThread & 0x1) == 0) {
            pDC->FillRect(rcThread, &brDark);
        }
        rcThread.top += mParent->rowYSpace;
        rcThread.bottom += mParent->rowYSpace;

        ThreadData* thread = sortedThreads->at(nThread);
        StripList& stripList = thread->getStripList();

        for (auto _si = 0; _si < stripList.size(); _si++) {
            Strip* strip = stripList.get(_si);
            pDC->FillSolidRect(strip->mX + mBoundRect.topleft.x, strip->mY + mBoundRect.topleft.y, strip->mWidth, strip->mHeight, strip->mColor);
        }
    }
}

void CDmTraceViewerView::CDmTimescaleView::draw(void* context)
{

}

void CDmTraceViewerView::CDmThreadLabelView::draw(void* context)
{
    CDC* pDC = reinterpret_cast<CDC*>(context);
    CBrush brDark(RGB(0xd0, 0xd0, 0xff));
    pDC->SetBkMode(TRANSPARENT);
    pDC->SetTextAlign(TA_RIGHT | TA_TOP);

    int cx = mBoundRect.size.cx - labelMarginX;
    CRect rcThread = CRect(mBoundRect.topleft.x, mBoundRect.topleft.y,
        mBoundRect.topleft.x + mBoundRect.size.cx, mBoundRect.topleft.y + mParent->rowYSpace);

    ThreadPtrList* sortedThreads = mParent->getThreads();
    for (int nThread = mParent->mStartRow; nThread < mParent->mEndRow; nThread++) {
        if ((nThread & 0x1) == 0) {
            pDC->FillRect(rcThread, &brDark);
        }

        ThreadData* thread = sortedThreads->at(nThread);
        CString strThreadLabel;
        CA2T szThreadName(thread->getName());
        pDC->TextOutW(cx, rcThread.top + mParent->rowYMarginHalf, szThreadName.m_psz);

        rcThread.top += mParent->rowYSpace;
        rcThread.bottom += mParent->rowYSpace;
    }
}

void CDmTraceViewerView::redraw()
{
    Invalidate(NULL);
}

void CDmTraceViewerView::CDmTimeLineView::redraw()
{
    Android::Rectangle rcBound;
    getBoundary(&rcBound);

    CRect rcToRedraw(rcBound.topleft.x, rcBound.topleft.y, rcBound.topleft.x + rcBound.size.cx, rcBound.topleft.y + rcBound.size.cy);
    CDmTraceViewerView* pView = reinterpret_cast<CDmTraceViewerView*>(mParent);
    pView->InvalidateRect(&rcToRedraw, FALSE);
}

void CDmTraceViewerView::CDmTimescaleView::redraw()
{
    Android::Rectangle rcBound;
    getBoundary(&rcBound);

    CRect rcToRedraw(rcBound.topleft.x, rcBound.topleft.y, rcBound.topleft.x + rcBound.size.cx, rcBound.topleft.y + rcBound.size.cy);
    CDmTraceViewerView* pView = reinterpret_cast<CDmTraceViewerView*>(mParent);
    pView->InvalidateRect(&rcToRedraw, FALSE);
}

void CDmTraceViewerView::CDmThreadLabelView::redraw()
{
    Android::Rectangle rcBound;
    getBoundary(&rcBound);

    CRect rcToRedraw(rcBound.topleft.x, rcBound.topleft.y, rcBound.topleft.x + rcBound.size.cx, rcBound.topleft.y + rcBound.size.cy);
    CDmTraceViewerView* pView = reinterpret_cast<CDmTraceViewerView*>(mParent);
    pView->InvalidateRect(&rcToRedraw, FALSE);
}

