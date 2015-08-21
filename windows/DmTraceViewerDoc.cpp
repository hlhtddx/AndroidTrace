// DmTraceViewerDoc.cpp : implementation of the CDmTraceViewerDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "DmTraceViewer.h"
#endif

#include "DmTraceViewerDoc.h"
#include "Android/DmTraceReader.hpp"
#include "Android/TimeLineView.hpp"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDmTraceViewerDoc

IMPLEMENT_DYNCREATE(CDmTraceViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CDmTraceViewerDoc, CDocument)
END_MESSAGE_MAP()


// CDmTraceViewerDoc construction/destruction

CDmTraceViewerDoc::CDmTraceViewerDoc()
	: m_pTraceFile(NULL)
{
	// TODO: add one-time construction code here

}

CDmTraceViewerDoc::~CDmTraceViewerDoc()
{
}

BOOL CDmTraceViewerDoc::OnNewDocument()
{
	//It is not possible to create a new trace file manually
	return FALSE;
}


#ifdef SHARED_HANDLERS

// Support for thumbnails
void CDmTraceViewerDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CDmTraceViewerDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CDmTraceViewerDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CDmTraceViewerDoc diagnostics

#ifdef _DEBUG
void CDmTraceViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDmTraceViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CDmTraceViewerDoc commands


void CDmTraceViewerDoc::OnCloseDocument()
{
	CDocument::OnCloseDocument();
}

BOOL CDmTraceViewerDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	DeleteContents();
	CT2A fileName(lpszPathName);

	Android::DmTraceReader* reader = new Android::DmTraceReader(fileName, false);
	Android::TimeLineView* view = new Android::TimeLineView(reader);
	delete reader;

	//m_pTraceFile = new Android::TraceFile;
	//if (!m_pTraceFile->parseDataKeys(fileName)) {
	//	delete m_pTraceFile;
	//	m_pTraceFile = NULL;
	//}
	return TRUE;
}

void CDmTraceViewerDoc::DeleteContents()
{
	if (m_pTraceFile)
	{
		delete m_pTraceFile;
		m_pTraceFile = NULL;
	}

	CDocument::DeleteContents();
}
