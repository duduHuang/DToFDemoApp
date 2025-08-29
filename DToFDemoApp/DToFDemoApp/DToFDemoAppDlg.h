
// DToFDemoAppDlg.h: 標頭檔
//

#pragma once
#include <cmath>
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
	virtual HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnDpiChanged(WPARAM wParam, LPARAM lParam);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CStatic m_picView;
	afx_msg void OnBnClickedPreview();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonSetText();
	afx_msg void OnBnClickedSetMaxValue();
	afx_msg void OnBnClickedSetPointXY();
	afx_msg void OnBnClickedSpeedUp();
	afx_msg void OnBtnClickedTransfer();
	afx_msg void OnBtnClickedSaveSD();

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnHScroll(NMHDR* pNMHDR, LRESULT* pResult);

private:
	const int defaultFileCount = 30, defaultThreshold = 1000;
	UINT m_uOriginalDpi = 96; // 初始 DPI (預設 96)
	CListBox m_infoListBox;
	CSliderCtrl m_sliderThreshold;
	CEdit m_editControl, m_maxEditControl, m_pointXEditControl, m_pointYEditControl, m_RegEditControl, m_DataEditControl;
	CStatic m_thresholdText;
	HCURSOR defaultCursor;
	int startX = 0, startY = 0, rotatX = 40, rotatY = 50;
	CRect pLTSubView, pRTSubView;
	bool cursorLTFlag = false, cursorRTFlag = false;
	CRect m_initClientRect;         // 記錄視窗初始大小
	std::map<UINT, CRect> m_controls;
	LARGE_INTEGER m_frequency;
	LARGE_INTEGER m_lastTime;
	int m_frameCount = 0;
	double m_fps = 0.0, m_RMSE = 0.0;
	uint8_t fWVersion[32];
	std::string statusMsg = "Disconnect";

	void SetSubView();
	void DisplaySubView(int width, int height);
	int Get2DPos(POINT srcpt, POINT* pt576);
	void UpdateSliderValue(int value);
	void MoveMouseTo(int x, int y);
	void ReLayoutUI(UINT newDpi);
	double GetRMSE();
	void AdjustControlsForDPI();
};
