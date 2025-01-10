﻿
// DToFDemoAppDlg.h: 標頭檔
//

#pragma once
#include "DirectShowCamera.h"

#define MYWM_NOTIFYICON (WM_USER + 2)

// CDToFDemoAppDlg 對話方塊
class CDToFDemoAppDlg : public CDialogEx
{
// 建構
public:
	CDToFDemoAppDlg(CWnd* pParent = nullptr);	// 標準建構函式

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DTOFDEMOAPP_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支援
	BOOL TrayMessage(DWORD dwMessage);

// 程式碼實作
protected:
	HICON m_hIcon;
	DirectShowCamera* directShowCamera;

	// 產生的訊息對應函式
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CStatic picView;
	afx_msg void OnBnClickedPreview();
	afx_msg void OnBnClickedCancel();

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

private:
	const int subViewWidth = 640, subViewHeight = 360;
	int rotatx = 40, rotaty = 50;
	void SetSubView();
	void DisplaySubView();
	void LTSubView();
	void RTSubView();
	void LBSubView();
	void RBSubView();

};
