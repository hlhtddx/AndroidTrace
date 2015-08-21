
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
int nGridHeight		= 40;
int nGridMargin		= 5;
int nLabelWidth		= 200;
int nLabelHeight	= 100;

// CDmTraceViewerView

IMPLEMENT_DYNCREATE(CDmTraceViewerView, CScrollView)

BEGIN_MESSAGE_MAP(CDmTraceViewerView, CScrollView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
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

//	SetScrollSizes(MM_HIMETRIC, CSize(1000, 100 + 100 * GetDocument()->m_pTraceFile->m_dataKeys->numThreads));

//	CClientDC dc(this);
	CRect rcInvalid;
	GetUpdateRect(rcInvalid);
	pDC->FillSolidRect(&rcInvalid, RGB(0xF0, 0xF0, 0xF0));
	//DrawGrid(pDC);
}

void CDmTraceViewerView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;

	if (GetDocument() && GetDocument()->m_pTimeLineView) {
		sizeTotal.cx = nLabelWidth + nGridMargin * 2 + nGridWidth;
		sizeTotal.cy = nLabelHeight + nGridMargin * 2 + nGridHeight * 2;
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


// CDmTraceViewerView message handlers
//
//void CDmTraceViewerView::DrawGrid(CDC* pDC)
//{
//	if (!GetDocument() || !GetDocument()->m_pTraceFile) {
//		TRACE("Document is not opened!\n");
//		return;
//	}
//	Android::TraceFile* pTraceFile = GetDocument()->m_pTraceFile;
//	int nThreads = pTraceFile->m_dataKeys->numThreads;
//
//	CBrush brDark(RGB(0xd0, 0xd0, 0xff));
//	pDC->SetBkMode(TRANSPARENT);
//
//	int cx = nLabelWidth + nGridMargin * 2 + nGridWidth;
//	CRect rcThread = CRect(0, nLabelHeight + nGridMargin,
//		cx, nLabelHeight + nGridMargin + nGridHeight);
//
//	for (int nThread = 0; nThread < nThreads; nThread++) {
//		if ((nThread & 0x1) == 0) {
//			pDC->FillRect(rcThread, &brDark);
//		}
//		rcThread.top += nGridHeight;
//		rcThread.bottom += nGridHeight;
//
//		DrawThread(pDC, pTraceFile, nThread);
//	}
//
//	uint64_t nTicks = pTraceFile->m_lastTime - pTraceFile->m_startTime;
//	uint64_t nTicksPerPixel = nTicks / nGridWidth;
//	uint64_t nCurrentPixel = 0;
//	uint64_t nCurrentTick = 0;
//	uint64_t nNextPixel = 1;
//	uint64_t nNextTick = nTicksPerPixel;
//	COLORREF colors[] = { RGB(90, 90, 255),
//		RGB(0, 240, 0), RGB(255, 0, 0), RGB(0, 255, 255),
//		RGB(255, 80, 255), RGB(200, 200, 0), RGB(40, 0, 200),
//		RGB(150, 255, 150), RGB(150, 0, 0), RGB(30, 150, 150),
//		RGB(200, 200, 255), RGB(0, 120, 0), RGB(255, 150, 150),
//		RGB(140, 80, 140), RGB(150, 100, 50), RGB(70, 70, 70) };
//
//	int nColorIndex = 0;
//	for (Android::CallEntryList::const_iterator ci = pTraceFile->m_callEntries.begin(); ci != pTraceFile->m_callEntries.end(); ci++) {
//		//if current method is enough to fill a pixel, draw current method
//		const Android::CallEntry* callEntry = *(ci);
//		Android::ThreadEntry* thread = callEntry->thread;
//		int top = nLabelHeight + nGridMargin + thread->index * nGridHeight + nGridMargin;
//		int cy = nGridHeight - nGridMargin * 2;
//		int left = nLabelWidth + nGridMargin;
//
//		if (callEntry->startTime - pTraceFile->m_startTime < nCurrentTick) {
//			continue;
//		}
//		else if (!callEntry->hasChildren) {
//			if ((callEntry->endTime - pTraceFile->m_startTime) >= nNextTick) {
//				nNextPixel = (callEntry->endTime - pTraceFile->m_startTime) / nTicksPerPixel;
//				nNextTick = nNextPixel * nTicksPerPixel;
//
//				nColorIndex = (++nColorIndex) % 16;
//				pDC->FillSolidRect(left + nCurrentPixel, top, nNextPixel - nCurrentPixel, cy, colors[nColorIndex]);
//				nCurrentPixel = nNextPixel;
//				nNextPixel = nCurrentPixel + 1;
//				nCurrentTick = nNextTick;
//				nNextTick = nCurrentTick + nTicksPerPixel;
//			}
//		}
//		else if ((callEntry->endTime - pTraceFile->m_startTime) <= nNextTick) {
//			nColorIndex = (++nColorIndex) % 16;
//			pDC->FillSolidRect(left + nCurrentPixel, top, nNextPixel - nCurrentPixel, cy, colors[nColorIndex]);
//			nCurrentPixel = nNextPixel;
//			nNextPixel = nCurrentPixel + 1;
//			nCurrentTick = nNextTick;
//			nNextTick = nCurrentTick + nTicksPerPixel;
//		}
//	}
//
//	pDC->MoveTo(nLabelWidth, 0);
//	pDC->LineTo(nLabelWidth, nLabelHeight + nGridMargin * 2 + nGridHeight * nThreads);
//
//}
//
//void CDmTraceViewerView::DrawThread(CDC* pDC, Android::TraceFile* pTraceFile, int nThread)
//{
//	Android::ThreadEntry& threadEntry = pTraceFile->m_dataKeys->threads[nThread];
//	pDC->SetTextAlign(TA_RIGHT | TA_BOTTOM);
//	CString strThreadLabel;
//	CA2T szThreadName(threadEntry.threadName);
//	strThreadLabel.Format(_T("[%d] %s"), threadEntry.threadId, szThreadName.m_psz);
//	pDC->TextOutW(nLabelWidth - nGridMargin, nLabelHeight + nGridMargin + (nThread + 1) * nGridHeight - 12, strThreadLabel);
//}
