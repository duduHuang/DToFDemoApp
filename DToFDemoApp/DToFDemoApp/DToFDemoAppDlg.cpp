
// DToFDemoAppDlg.cpp: 實作檔案
//

#include "pch.h"
#include "framework.h"
#include "DToFDemoApp.h"
#include "DToFDemoAppDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 對 App About 使用 CAboutDlg 對話方塊

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

// 程式碼實作
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CDToFDemoAppDlg 對話方塊



CDToFDemoAppDlg::CDToFDemoAppDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DTOFDEMOAPP_DIALOG, pParent), directShowCamera(nullptr)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDToFDemoAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PIC, picView);
}

BEGIN_MESSAGE_MAP(CDToFDemoAppDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_PREBTN, &CDToFDemoAppDlg::OnBnClickedPreview)
	ON_BN_CLICKED(IDCANCEL, &CDToFDemoAppDlg::OnBnClickedCancel)

	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CDToFDemoAppDlg 訊息處理常式

BOOL CDToFDemoAppDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 將 [關於...] 功能表加入系統功能表。

	// IDM_ABOUTBOX 必須在系統命令範圍之中。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 設定此對話方塊的圖示。當應用程式的主視窗不是對話方塊時，
	// 框架會自動從事此作業
	SetIcon(m_hIcon, TRUE);			// 設定大圖示
	SetIcon(m_hIcon, FALSE);		// 設定小圖示

	// TODO: 在此加入額外的初始設定
	directShowCamera = new DirectShowCamera();

	GetDlgItem(IDC_PIC)->SetWindowPos(GetParent(), 10, 10, 1280, 720, SWP_SHOWWINDOW);

	return TRUE;  // 傳回 TRUE，除非您對控制項設定焦點
}

void CDToFDemoAppDlg::OnDestroy() {
	if (nullptr != directShowCamera) {
		delete directShowCamera;
		directShowCamera = nullptr;
	}
	CDialogEx::OnDestroy();
}

void CDToFDemoAppDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果將最小化按鈕加入您的對話方塊，您需要下列的程式碼，
// 以便繪製圖示。對於使用文件/檢視模式的 MFC 應用程式，
// 框架會自動完成此作業。

void CDToFDemoAppDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 繪製的裝置內容

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 將圖示置中於用戶端矩形
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 描繪圖示
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 當使用者拖曳最小化視窗時，
// 系統呼叫這個功能取得游標顯示。
HCURSOR CDToFDemoAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDToFDemoAppDlg::OnBnClickedPreview() {
	const char* targetDeviceName = "CX3-UVC";
	std::vector<std::string> list;
	int deviceCount = directShowCamera->listDevices(list);
	SetSubView();
	DisplaySubView();
	directShowCamera->openCamera(targetDeviceName);
	directShowCamera->prepareCamera();
	directShowCamera->run();
	MSG msg;
	while (directShowCamera->isPreview) {
		if (GetMessage(&msg, nullptr, 0, 0) > 0) {
			if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN) {
				// 如果按下 Enter 鍵，退出消息循環
				directShowCamera->isPreview = false;
				PostQuitMessage(0); // 發送 WM_QUIT
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			// 如果 GetMessage 返回 0，表示收到 WM_QUIT
			directShowCamera->isPreview = false;
		}
	}
}

void CDToFDemoAppDlg::OnBnClickedCancel() {
	directShowCamera->stop();
	CDialogEx::OnCancel();
}

BOOL CDToFDemoAppDlg::TrayMessage(DWORD dwMessage) {
	CString sTip(_T("POLYVISION HR"));
	NOTIFYICONDATA tnd;
	tnd.cbSize = sizeof(NOTIFYICONDATA);
	tnd.hWnd = m_hWnd;
	tnd.uID = IDR_MAINFRAME;
	tnd.uFlags = NIF_MESSAGE | NIF_ICON;
	tnd.uCallbackMessage = MYWM_NOTIFYICON;
	tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	VERIFY(tnd.hIcon = LoadIcon(AfxGetInstanceHandle(),
		MAKEINTRESOURCE(IDI_16ICON)));
	lstrcpyn(tnd.szTip, (LPCTSTR)sTip, sizeof(tnd.szTip));
	return Shell_NotifyIcon(dwMessage, &tnd);
}

void CDToFDemoAppDlg::OnTimer(UINT_PTR nIDEvent) {
	CDialogEx::OnTimer(nIDEvent);
}

void CDToFDemoAppDlg::OnMouseMove(UINT nFlags, CPoint point) {
	CDialogEx::OnMouseMove(nFlags, point);
}

BOOL CDToFDemoAppDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) {
	return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}

void CDToFDemoAppDlg::OnLButtonDown(UINT nFlags, CPoint point) {
	CDialogEx::OnLButtonDown(nFlags, point);
}

void CDToFDemoAppDlg::OnLButtonUp(UINT nFlags, CPoint point) {
	//SetCursor(defaultcursor);
	//cursorflag = false;
	CDialogEx::OnLButtonUp(nFlags, point);
}

void CDToFDemoAppDlg::SetSubView() {
	directShowCamera->setSubViewWH(subViewWidth, subViewHeight);
	directShowCamera->setCDC(
		GetDlgItem(IDC_PIC)->GetDC(),
		GetDlgItem(IDC_PIC1)->GetDC(),
		GetDlgItem(IDC_PIC2)->GetDC(),
		GetDlgItem(IDC_PIC3)->GetDC()
	);
}

void CDToFDemoAppDlg::DisplaySubView() {
	GetDlgItem(IDC_PIC)->SetWindowPos(
		GetParent(),
		0, 0,
		subViewWidth, subViewHeight,
		SWP_SHOWWINDOW);
	GetDlgItem(IDC_PIC1)->SetWindowPos(
		GetParent(),
		subViewWidth + 1, 0,
		subViewWidth, subViewHeight,
		SWP_SHOWWINDOW);
	GetDlgItem(IDC_PIC2)->SetWindowPos(
		GetParent(),
		0, subViewHeight + 1,
		subViewWidth, subViewHeight,
		SWP_SHOWWINDOW);
	GetDlgItem(IDC_PIC3)->SetWindowPos(
		GetParent(),
		subViewWidth + 1, subViewHeight + 1,
		subViewWidth, subViewHeight,
		SWP_SHOWWINDOW);
}

void CDToFDemoAppDlg::LTSubView() {
	CRect dcrect(0, 0, subViewWidth, subViewHeight);
	CDC memDC;
	CDC* pDC = GetDlgItem(IDC_PIC)->GetDC();
	memDC.CreateCompatibleDC(pDC);

	CBitmap Membitmap;
	Membitmap.CreateCompatibleBitmap(pDC, dcrect.Width(), dcrect.Height());
	CBitmap* pBmpOld = memDC.SelectObject(&Membitmap);

	cv::Mat pic(subViewWidth, subViewHeight, CV_8UC3);
	directShowCamera->Cloud3D(subViewWidth, subViewHeight, pic.data, rotatx, rotaty);

	BITMAPINFO bitInfo;
	bitInfo.bmiHeader.biBitCount = SPOT_NUMBER;
	bitInfo.bmiHeader.biWidth = subViewWidth;
	bitInfo.bmiHeader.biHeight = subViewHeight;
	bitInfo.bmiHeader.biPlanes = 1;
	bitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitInfo.bmiHeader.biCompression = BI_RGB;
	bitInfo.bmiHeader.biClrImportant = 0;
	bitInfo.bmiHeader.biClrUsed = 0;
	bitInfo.bmiHeader.biSizeImage = 0;
	bitInfo.bmiHeader.biXPelsPerMeter = 0;
	bitInfo.bmiHeader.biYPelsPerMeter = 0;

	::StretchDIBits(memDC.GetSafeHdc(), 0, 0,
		subViewWidth, subViewHeight, 0, 0,
		subViewWidth, subViewHeight,
		pic.data, &bitInfo, DIB_RGB_COLORS, SRCCOPY
	);

	pDC->BitBlt(dcrect.left, dcrect.top, dcrect.Width(), dcrect.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pBmpOld);

	Membitmap.DeleteObject();
	memDC.DeleteDC();
	GetDlgItem(IDC_PIC)->ReleaseDC(pDC);
}

void CDToFDemoAppDlg::RTSubView() {
	CRect dcrect(0, 0, subViewWidth, subViewHeight);
	CDC memDC;
	CDC* pDC = GetDlgItem(IDC_PIC1)->GetDC();
	memDC.CreateCompatibleDC(pDC);
	CBitmap Membitmap;
	Membitmap.CreateCompatibleBitmap(pDC, dcrect.Width(), dcrect.Height());
	CBitmap* pBmpOld = memDC.SelectObject(&Membitmap);
	cv::Mat pic(subViewWidth, subViewHeight, CV_8UC3);

	directShowCamera->Histgram(subViewWidth, subViewHeight, pic.data, -1);

	BITMAPINFO bitInfo;
	bitInfo.bmiHeader.biBitCount = SPOT_NUMBER;
	bitInfo.bmiHeader.biWidth = subViewWidth;
	bitInfo.bmiHeader.biHeight = subViewHeight;
	bitInfo.bmiHeader.biPlanes = 1;
	bitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitInfo.bmiHeader.biCompression = BI_RGB;
	bitInfo.bmiHeader.biClrImportant = 0;
	bitInfo.bmiHeader.biClrUsed = 0;
	bitInfo.bmiHeader.biSizeImage = 0;
	bitInfo.bmiHeader.biXPelsPerMeter = 0;
	bitInfo.bmiHeader.biYPelsPerMeter = 0;

	::StretchDIBits(memDC.GetSafeHdc(), 0, 0,
		subViewWidth, subViewHeight, 0, 0,
		subViewWidth, subViewHeight,
		pic.data, &bitInfo, DIB_RGB_COLORS, SRCCOPY
	);

	pDC->BitBlt(dcrect.left, dcrect.top, dcrect.Width(), dcrect.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pBmpOld);

	Membitmap.DeleteObject();
	memDC.DeleteDC();
	GetDlgItem(IDC_PIC1)->ReleaseDC(pDC);
}

void CDToFDemoAppDlg::LBSubView() {
	CRect dcrect(0, 0, subViewWidth, subViewHeight);
	CDC memDC;
	CDC* pDC = GetDlgItem(IDC_PIC2)->GetDC();
	memDC.CreateCompatibleDC(pDC);
	CBitmap Membitmap;
	Membitmap.CreateCompatibleBitmap(pDC, dcrect.Width(), dcrect.Height());
	CBitmap* pBmpOld = memDC.SelectObject(&Membitmap);
	cv::Mat pic(subViewWidth, subViewHeight, CV_8UC3);

	directShowCamera->Cloud2D(subViewWidth, subViewHeight, pic.data);

	BITMAPINFO bitInfo;
	bitInfo.bmiHeader.biBitCount = SPOT_NUMBER;
	bitInfo.bmiHeader.biWidth = subViewWidth;
	bitInfo.bmiHeader.biHeight = subViewHeight;
	bitInfo.bmiHeader.biPlanes = 1;
	bitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitInfo.bmiHeader.biCompression = BI_RGB;
	bitInfo.bmiHeader.biClrImportant = 0;
	bitInfo.bmiHeader.biClrUsed = 0;
	bitInfo.bmiHeader.biSizeImage = 0;
	bitInfo.bmiHeader.biXPelsPerMeter = 0;
	bitInfo.bmiHeader.biYPelsPerMeter = 0;

	::StretchDIBits(memDC.GetSafeHdc(), 0, 0,
		subViewWidth, subViewHeight, 0, 0,
		subViewWidth, subViewHeight,
		pic.data, &bitInfo, DIB_RGB_COLORS, SRCCOPY
	);

	pDC->BitBlt(dcrect.left, dcrect.top, dcrect.Width(), dcrect.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pBmpOld);

	Membitmap.DeleteObject();
	memDC.DeleteDC();
	GetDlgItem(IDC_PIC2)->ReleaseDC(pDC);
}

void CDToFDemoAppDlg::RBSubView() {
	CRect dcrect(0, 0, subViewWidth, subViewHeight);
	CDC memDC;
	CDC* pDC = GetDlgItem(IDC_PIC3)->GetDC();
	memDC.CreateCompatibleDC(pDC);
	CBitmap Membitmap;
	Membitmap.CreateCompatibleBitmap(pDC, dcrect.Width(), dcrect.Height());
	CBitmap* pBmpOld = memDC.SelectObject(&Membitmap);
	cv::Mat pic(subViewWidth, subViewHeight, CV_8UC3);

	directShowCamera->Filter2D(subViewWidth, subViewHeight, pic.data, 1000, 0);

	BITMAPINFO bitInfo;
	bitInfo.bmiHeader.biBitCount = SPOT_NUMBER;
	bitInfo.bmiHeader.biWidth = subViewWidth;
	bitInfo.bmiHeader.biHeight = subViewHeight;
	bitInfo.bmiHeader.biPlanes = 1;
	bitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitInfo.bmiHeader.biCompression = BI_RGB;
	bitInfo.bmiHeader.biClrImportant = 0;
	bitInfo.bmiHeader.biClrUsed = 0;
	bitInfo.bmiHeader.biSizeImage = 0;
	bitInfo.bmiHeader.biXPelsPerMeter = 0;
	bitInfo.bmiHeader.biYPelsPerMeter = 0;

	::StretchDIBits(memDC.GetSafeHdc(), 0, 0,
		subViewWidth, subViewHeight, 0, 0,
		subViewWidth, subViewHeight,
		pic.data, &bitInfo, DIB_RGB_COLORS, SRCCOPY
	);

	pDC->BitBlt(dcrect.left, dcrect.top, dcrect.Width(), dcrect.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pBmpOld);

	Membitmap.DeleteObject();
	memDC.DeleteDC();
	GetDlgItem(IDC_PIC3)->ReleaseDC(pDC);
}
