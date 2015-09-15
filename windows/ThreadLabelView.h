#pragma once

#include "DmTraceViewerDoc.h"

class CThreadLabelView
    : public CView
    , public ThreadLabelView
{
	DECLARE_DYNCREATE(CThreadLabelView)

protected:
	CThreadLabelView();           // 动态创建所使用的受保护的构造函数
	virtual ~CThreadLabelView();

public:
	virtual void OnDraw(CDC* pDC);      // 重写以绘制该视图
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
    virtual void draw(void* context) {}
    virtual void redraw() {}

    // Attributes
public:
    CDmTraceViewerDoc* GetDocument() const;

protected:
	DECLARE_MESSAGE_MAP()
};


