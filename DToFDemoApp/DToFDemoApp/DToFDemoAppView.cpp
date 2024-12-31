﻿
// DToFDemoAppView.cpp: CDToFDemoAppView 類別的實作
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以定義在實作預覽、縮圖和搜尋篩選條件處理常式的
// ATL 專案中，並允許與該專案共用文件程式碼。
#ifndef SHARED_HANDLERS
#include "DToFDemoApp.h"
#endif

#include "DToFDemoAppDoc.h"
#include "DToFDemoAppView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDToFDemoAppView

IMPLEMENT_DYNCREATE(CDToFDemoAppView, CView)

BEGIN_MESSAGE_MAP(CDToFDemoAppView, CView)
	// 標準列印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CDToFDemoAppView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CDToFDemoAppView 建構/解構

CDToFDemoAppView::CDToFDemoAppView() noexcept
{
	// TODO: 在此加入建構程式碼

}

CDToFDemoAppView::~CDToFDemoAppView()
{
}

BOOL CDToFDemoAppView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此經由修改 CREATESTRUCT cs 
	// 達到修改視窗類別或樣式的目的

	return CView::PreCreateWindow(cs);
}

// CDToFDemoAppView 繪圖

void CDToFDemoAppView::OnDraw(CDC* /*pDC*/)
{
	CDToFDemoAppDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此加入原生資料的描繪程式碼
}


// CDToFDemoAppView 列印


void CDToFDemoAppView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CDToFDemoAppView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 預設的準備列印程式碼
	return DoPreparePrinting(pInfo);
}

void CDToFDemoAppView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 加入列印前額外的初始設定
}

void CDToFDemoAppView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 加入列印後的清除程式碼
}

void CDToFDemoAppView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CDToFDemoAppView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CDToFDemoAppView 診斷

#ifdef _DEBUG
void CDToFDemoAppView::AssertValid() const
{
	CView::AssertValid();
}

void CDToFDemoAppView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CDToFDemoAppDoc* CDToFDemoAppView::GetDocument() const // 內嵌非偵錯版本
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDToFDemoAppDoc)));
	return (CDToFDemoAppDoc*)m_pDocument;
}
#endif //_DEBUG


// CDToFDemoAppView 訊息處理常式
