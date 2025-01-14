
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
	DDX_Control(pDX, IDC_PIC, m_picView);
	DDX_Control(pDX, IDC_EDIT1, m_editControl);
}

BEGIN_MESSAGE_MAP(CDToFDemoAppDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_PREBTN, &CDToFDemoAppDlg::OnBnClickedPreview)
	ON_BN_CLICKED(IDCANCEL, &CDToFDemoAppDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_EDITBTN, &CDToFDemoAppDlg::OnBnClickedButtonSetText)

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

	pLTSubView.top = 0;
	pLTSubView.left = 0;
	pLTSubView.bottom = 360;
	pLTSubView.right = 640;

	pRTSubView.top = 321;
	pRTSubView.left = 0;
	pRTSubView.bottom = 720;
	pRTSubView.right = 640;

	defaultCursor = GetCursor();

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
	PostQuitMessage(0); // 發送 WM_QUIT
}

void CDToFDemoAppDlg::OnBnClickedButtonSetText() {
	CString inputText;
	m_editControl.GetWindowTextW(inputText);
	if (!inputText.IsEmpty()) {
		int number = _ttoi(inputText);
		if (0 != number) {
			directShowCamera->writeFile(number);
		}
		else {
			directShowCamera->writeFile(defaultFileCount);
		}
	}
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
	POINT pt;
	GetCursorPos(&pt);
	::ScreenToClient(this->m_hWnd, &pt);

	if (cursorLTFlag && pt.x >= pLTSubView.left && pt.x <= pLTSubView.right && pt.y >= pLTSubView.top && pt.y <= pLTSubView.bottom) {
		if (point.x > startX)
			rotatX += 5;
		else if (point.x < startX)
			rotatX -= 5;

		if (point.y > startY)
			rotatY += 5;
		else if (point.y < startY)
			rotatY -= 5;

		startX = point.x;
		startY = point.y;

		RECT m_rect;
		GetDlgItem(IDC_PIC)->GetWindowRect(&m_rect);
		directShowCamera->setRotate(rotatX, rotatY, m_rect.right - m_rect.left, m_rect.bottom - m_rect.top);
	}
	else if (pt.x >= pRTSubView.left && pt.x <= pRTSubView.right && pt.y >= pRTSubView.top && pt.y <= pRTSubView.bottom) {
		if (GetCursor() != AfxGetApp()->LoadStandardCursor(IDC_CROSS)) {
			defaultCursor = SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS));
			cursorRTFlag = true;
		}
	}
	else {
		if (GetCursor() != defaultCursor)
			SetCursor(defaultCursor);

		cursorRTFlag = false;
	}

	CDialogEx::OnMouseMove(nFlags, point);
}

BOOL CDToFDemoAppDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) {
	if (cursorLTFlag)
		return TRUE;
	else if (cursorRTFlag)
		return TRUE;
	else
		return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}

void CDToFDemoAppDlg::OnLButtonDown(UINT nFlags, CPoint point) {
	POINT pt;
	GetCursorPos(&pt);
	::ScreenToClient(this->m_hWnd, &pt);

	if (pt.x >= pLTSubView.left && pt.x <= pLTSubView.right && pt.y >= pLTSubView.top && pt.y <= pLTSubView.bottom) {
		defaultCursor = SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
		cursorLTFlag = true;
		startX = point.x;
		startY = point.y;
	}
	else if (cursorRTFlag && pt.x >= pRTSubView.left && pt.x <= pRTSubView.right && pt.y >= pRTSubView.top && pt.y <= pRTSubView.bottom) {
		int index = -1;
		POINT* histpoints = directShowCamera->getPoints();
		index = Get2DPos(pt, histpoints);
		RECT m_rect;
		GetDlgItem(IDC_PIC1)->GetWindowRect(&m_rect);
		directShowCamera->setHistIndex(index, m_rect.right - m_rect.left, m_rect.bottom - m_rect.top);
	}
	else {
		SendMessage(WM_SYSCOMMAND, SC_MOVE | HTCAPTION);
	}

	//CDialogEx::OnLButtonDown(nFlags, point);
}

void CDToFDemoAppDlg::OnLButtonUp(UINT nFlags, CPoint point) {
	SetCursor(defaultCursor);
	cursorLTFlag = false;
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

bool CheckInRect(POINT srcpt, POINT limitpt) {
	int lx, ty;
	if ((limitpt.x - 5) < 0)
		lx = 0;
	else
		lx = limitpt.x - 5;

	if ((limitpt.y - 5) < 0)
		ty = 0;
	else
		ty = limitpt.y - 5;

	if (srcpt.x >= lx && srcpt.x <= limitpt.x + 5) {
		if (srcpt.y >= ty && srcpt.y <= limitpt.y + 5) {
			return true;
		}
	}

	return false;
}

int CDToFDemoAppDlg::Get2DPos(POINT srcpt, POINT* pt576) {
	int index = -1;
	for (int i = 0; i < DP_NUMBER; i++) {
		if (CheckInRect(srcpt, pt576[i])) {
			index = i;
			break;
		}
	}
	return index;
}