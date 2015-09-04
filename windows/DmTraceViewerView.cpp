
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
	// TODO: add construction code here

}

CDmTraceViewerView::~CDmTraceViewerView()
{
}

BOOL CDmTraceViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

// CDmTraceViewerView drawing

void CDmTraceViewerView::OnDraw(CDC* pDC)
{
	CDmTraceViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

//	CClientDC dc(this);
	CRect rcInvalid;
	GetUpdateRect(rcInvalid);
    pDC->LPtoDP(&rcInvalid);
	pDC->FillSolidRect(&rcInvalid, RGB(0xF0, 0xF0, 0xF0));
    CPoint ptScroll = GetScrollPosition();

    if (pDoc == nullptr || pDoc->m_pTimeLineView == nullptr) {
        return;
    }

    DmTraceData* pReader = GetDocument()->m_pTraceReader;
    DmTraceControl* pTimeLineview = GetDocument()->m_pTimeLineView;

    int nRows = pReader->getThreads()->size();
    int nHeight = nRows * 32;
    DrawGrid(pDC, pReader, pTimeLineview);
}

void CDmTraceViewerView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;

	if (GetDocument() && GetDocument()->m_pTimeLineView) {
        DmTraceData* pReader = GetDocument()->m_pTraceReader;
        DmTraceControl* pTimeLineview = GetDocument()->m_pTimeLineView;

        CRect rcClient;
        GetClientRect(rcClient);

        int nRows = pReader->getThreads()->size();
        int nHeight = nRows * 32;

        TickScaler& scaleInfo = pTimeLineview->getScaleInfo();
        scaleInfo.setMinVal(pReader->getMinTime());
        scaleInfo.setMaxVal(pReader->getMaxTime());
        scaleInfo.setNumPixels(rcClient.Width());
        scaleInfo.computeTicks(false);
        pTimeLineview->computeVisibleRows(rcClient.Height());

        int nWidth = scaleInfo.getMaxVal() - scaleInfo.getMinVal();

        sizeTotal.cx = rcClient.Width();//nLabelWidth + nGridMargin * 2 + nWidth;
		sizeTotal.cy = nLabelHeight + nGridMargin * 2 + nHeight;
		SetScrollSizes(MM_TEXT, sizeTotal);
    }
	else {
		sizeTotal.cx = sizeTotal.cy = 1;
		SetScrollSizes(MM_TEXT, sizeTotal);
	}
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

void CDmTraceViewerView::DrawGrid(CDC* pDC, DmTraceData* pReader, DmTraceControl* pTimeLineview)
{
	int nThreads = pReader->getThreads()->size();

	CBrush brDark(RGB(0xd0, 0xd0, 0xff));
	pDC->SetBkMode(TRANSPARENT);

	int cx = nLabelWidth + nGridMargin * 2 + nGridWidth;
	CRect rcThread = CRect(0, nLabelHeight + nGridMargin,
		cx, nLabelHeight + nGridMargin + nGridHeight);

	for (int nThread = 0; nThread < nThreads; nThread++) {
		if ((nThread & 0x1) == 0) {
			pDC->FillRect(rcThread, &brDark);
		}
		rcThread.top += nGridHeight;
		rcThread.bottom += nGridHeight;

        ThreadData* thread = (*pReader->getThreads())[nThread];
		DrawThread(pDC, thread, nThread * nGridHeight);
	}

    TickScaler& scaler = pTimeLineview->getScaleInfo();
	uint64_t nTicks = scaler.getPixelsPerTick();
	uint64_t nTicksPerPixel = nTicks / nGridWidth;
	uint64_t nCurrentPixel = 0;
	uint64_t nCurrentTick = 0;
	uint64_t nNextPixel = 1;
	uint64_t nNextTick = nTicksPerPixel;

	pDC->MoveTo(nLabelWidth, 0);
	pDC->LineTo(nLabelWidth, nLabelHeight + nGridMargin * 2 + nGridHeight * nThreads);

}

void CDmTraceViewerView::DrawThread(CDC* pDC, ThreadData* thread, int nBaseLine)
{
	pDC->SetTextAlign(TA_RIGHT | TA_BOTTOM);
	CString strThreadLabel;
	CA2T szThreadName(thread->getName());
	pDC->TextOutW(nLabelWidth - nGridMargin, nLabelHeight + nGridMargin + nBaseLine - 12, szThreadName.m_psz);
    StripList& stripList = thread->getStripList();
    for (auto _si = 0; _si < stripList.size(); _si++) {
        Strip* strip = stripList.get(_si);
        pDC->FillSolidRect(strip->mX + nLabelWidth, strip->mY + nLabelHeight, strip->mWidth, strip->mHeight, strip->mColor);
    }
}


void CDmTraceViewerView::OnSize(UINT nType, int cx, int cy)
{
    CScrollView::OnSize(nType, cx, cy);

    if (GetDocument() == nullptr || GetDocument()->m_pTimeLineView == nullptr) {
        return;
    }

    DmTraceData* pReader = GetDocument()->m_pTraceReader;
    DmTraceControl* pTimeLineview = GetDocument()->m_pTimeLineView;

    int nRows = pReader->getThreads()->size();
    int nHeight = nRows * 32;

    TickScaler& scaleInfo = pTimeLineview->getScaleInfo();
    scaleInfo.setMinVal(pReader->getMinTime());
    scaleInfo.setMaxVal(pReader->getMaxTime());
    scaleInfo.setNumPixels(cx - nLabelWidth);
    scaleInfo.computeTicks(true);
    pTimeLineview->computeVisibleRows(cy - nLabelHeight);
    pTimeLineview->computeStrips();
}
