
// DmTraceViewerDoc.h : interface of the CDmTraceViewerDoc class
//

#pragma once

#include "DmTraceReader.hpp"
#include "TimeLineView.hpp"

class CDmTraceViewerDoc : public CDocument
{
protected: // create from serialization only
	CDmTraceViewerDoc();
	DECLARE_DYNCREATE(CDmTraceViewerDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	BOOL OnNewDocument();
//	void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	void InitializeSearchContent();
	void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	~CDmTraceViewerDoc();
#ifdef _DEBUG
	void AssertValid() const;
	void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	void OnCloseDocument();
	BOOL OnOpenDocument(LPCTSTR lpszPathName);
	void DeleteContents();
	Android::DmTraceReader* m_pTraceReader;
	Android::TimeLineView* m_pTimeLineView;
};
