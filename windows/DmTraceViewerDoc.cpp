// DmTraceViewerDoc.cpp : implementation of the CDmTraceViewerDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "DmTraceViewer.h"
#endif

#include "DmTraceViewerDoc.h"

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
	: m_pTraceReader(NULL)
	, m_pTimeLineView(NULL)
{
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

    try {
        m_pTraceReader = new Android::DmTraceData(fileName, false);
        m_pTimeLineView = new Android::DmTraceControl(m_pTraceReader);
    }
    catch (GeneralException& e) {
        TRACE("Got a exception(%s)\n", e.getDescription());
        DeleteContents();
        return FALSE;
    }

	return TRUE;
}

void CDmTraceViewerDoc::DeleteContents()
{
	if (m_pTimeLineView)
	{
		delete m_pTimeLineView;
		m_pTimeLineView = NULL;
	}

	if (m_pTraceReader)
	{
		delete m_pTraceReader;
		m_pTraceReader = NULL;
	}

	CDocument::DeleteContents();
}
