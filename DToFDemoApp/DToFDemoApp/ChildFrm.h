
// ChildFrm.h: CChildFrame 類別的介面
//

#pragma once

class CChildFrame : public CMDIChildWndEx
{
	DECLARE_DYNCREATE(CChildFrame)
public:
	CChildFrame() noexcept;

// 屬性
protected:
	CSplitterWndEx m_wndSplitter;
public:

// 作業
public:

// 覆寫
	public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 程式碼實作
public:
	virtual ~CChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// 產生的訊息對應函式
protected:
	DECLARE_MESSAGE_MAP()
};
