
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
	DDX_Control(pDX, IDC_EDIT2, m_maxEditControl);
	DDX_Control(pDX, IDC_EDIT3, m_pointXEditControl);
	DDX_Control(pDX, IDC_EDIT4, m_pointYEditControl);
	DDX_Control(pDX, IDC_EDIT5, m_DataEditControl);
	DDX_Control(pDX, IDC_EDIT6, m_RegEditControl);
	DDX_Control(pDX, IDC_LIST2, m_deviceListBox); // 綁定控件變數
	DDX_Control(pDX, IDC_SLIDER1, m_sliderThreshold);
	DDX_Control(pDX, IDC_THRESHOLDTEXT, m_thresholdText);
}

BEGIN_MESSAGE_MAP(CDToFDemoAppDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_PREBTN, &CDToFDemoAppDlg::OnBnClickedPreview)
	ON_BN_CLICKED(IDCANCEL, &CDToFDemoAppDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_EDITBTN, &CDToFDemoAppDlg::OnBnClickedButtonSetText)
	ON_BN_CLICKED(IDC_MAXBTN, &CDToFDemoAppDlg::OnBnClickedSetMaxValue)
	ON_BN_CLICKED(IDC_POINTBTN, &CDToFDemoAppDlg::OnBnClickedSetPointXY)
	ON_BN_CLICKED(IDC_SPEEDBTN, &CDToFDemoAppDlg::OnBnClickedSpeedUp)
	ON_BN_CLICKED(IDC_TRANSFERBTN, &CDToFDemoAppDlg::OnStnClickedTransfer)

	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CDToFDemoAppDlg::OnHScroll)
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
	directShowCamera = new DirectShowCamera[2];

	int screenWidth = 1920;//GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = 1080;// GetSystemMetrics(SM_CYSCREEN);
	int width = screenWidth * 0.8;
	int height = screenHeight * 0.8;
	int posX = (screenWidth - width) / 2;
	int posY = (screenHeight - height) / 2;
	MoveWindow(posX, posY, width, height);

	GetDlgItem(IDC_PIC)->SetWindowPos(GetParent(),  10,               10,                width * 0.3, height * 0.3, SWP_SHOWWINDOW);
	GetDlgItem(IDC_PIC1)->SetWindowPos(GetParent(), width * 0.3 + 11, 10,                width * 0.3, height * 0.3, SWP_SHOWWINDOW);
	GetDlgItem(IDC_PIC2)->SetWindowPos(GetParent(), 10,               height * 0.3 + 11, width * 0.3, height * 0.3, SWP_SHOWWINDOW);
	GetDlgItem(IDC_PIC3)->SetWindowPos(GetParent(), width * 0.3 + 11, height * 0.3 + 11, width * 0.3, height * 0.3, SWP_SHOWWINDOW);

	CRect rect;
	GetDlgItem(IDC_LIST2)->GetWindowRect(&rect);
	ScreenToClient(&rect); // 把螢幕座標轉換為父視窗的座標
	GetDlgItem(IDC_LIST2)->SetWindowPos(GetParent(), width * 0.84, 10, rect.Width(), rect.Height(), SWP_SHOWWINDOW);

	GetDlgItem(IDC_EDIT1)->GetWindowRect(&rect);
	ScreenToClient(&rect); // 把螢幕座標轉換為父視窗的座標
	GetDlgItem(IDC_EDIT1)->SetWindowPos(GetParent(), width * 0.92, height * 0.7, rect.Width(), rect.Height(), SWP_SHOWWINDOW);

	GetDlgItem(IDC_EDIT2)->GetWindowRect(&rect);
	ScreenToClient(&rect); // 把螢幕座標轉換為父視窗的座標
	GetDlgItem(IDC_EDIT2)->SetWindowPos(GetParent(), width * 0.92, height * 0.6, rect.Width(), rect.Height(), SWP_SHOWWINDOW);

	GetDlgItem(IDC_EDIT3)->GetWindowRect(&rect);
	ScreenToClient(&rect); // 把螢幕座標轉換為父視窗的座標
	GetDlgItem(IDC_EDIT3)->SetWindowPos(GetParent(), width * 0.92 + 10, height * 0.52, 25, rect.Height(), SWP_SHOWWINDOW);

	GetDlgItem(IDC_EDIT4)->GetWindowRect(&rect);
	ScreenToClient(&rect); // 把螢幕座標轉換為父視窗的座標
	GetDlgItem(IDC_EDIT4)->SetWindowPos(GetParent(), width * 0.92 + 50, height * 0.52, 25, rect.Height(), SWP_SHOWWINDOW);

	GetDlgItem(IDC_EDIT5)->GetWindowRect(&rect);
	ScreenToClient(&rect); // 把螢幕座標轉換為父視窗的座標
	GetDlgItem(IDC_EDIT5)->SetWindowPos(GetParent(), width * 0.92, height * 0.3, rect.Width(), rect.Height(), SWP_SHOWWINDOW);

	GetDlgItem(IDC_EDIT6)->GetWindowRect(&rect);
	ScreenToClient(&rect); // 把螢幕座標轉換為父視窗的座標
	GetDlgItem(IDC_EDIT6)->SetWindowPos(GetParent(), width * 0.92, height * 0.2, rect.Width(), rect.Height(), SWP_SHOWWINDOW);

	GetDlgItem(IDC_EDITBTN)->GetWindowRect(&rect);
	ScreenToClient(&rect); // 把螢幕座標轉換為父視窗的座標
	GetDlgItem(IDC_EDITBTN)->SetWindowPos(GetParent(), width * 0.92, height * 0.75, rect.Width(), rect.Height(), SWP_SHOWWINDOW);

	GetDlgItem(IDC_MAXBTN)->GetWindowRect(&rect);
	ScreenToClient(&rect); // 把螢幕座標轉換為父視窗的座標
	GetDlgItem(IDC_MAXBTN)->SetWindowPos(GetParent(), width * 0.92, height * 0.65, rect.Width(), rect.Height(), SWP_SHOWWINDOW);

	GetDlgItem(IDC_POINTBTN)->GetWindowRect(&rect);
	ScreenToClient(&rect); // 把螢幕座標轉換為父視窗的座標
	GetDlgItem(IDC_POINTBTN)->SetWindowPos(GetParent(), width * 0.92, height * 0.55, rect.Width(), rect.Height(), SWP_SHOWWINDOW);

	GetDlgItem(IDCANCEL)->GetWindowRect(&rect);
	ScreenToClient(&rect); // 把螢幕座標轉換為父視窗的座標
	GetDlgItem(IDCANCEL)->SetWindowPos(GetParent(), width * 0.92, height * 0.8, rect.Width(), rect.Height(), SWP_SHOWWINDOW);

	GetDlgItem(IDC_PREBTN)->GetWindowRect(&rect);
	ScreenToClient(&rect); // 把螢幕座標轉換為父視窗的座標
	GetDlgItem(IDC_PREBTN)->SetWindowPos(GetParent(), width * 0.84, height * 0.8, rect.Width(), rect.Height(), SWP_SHOWWINDOW);

	GetDlgItem(IDC_SLIDER1)->SetWindowPos(GetParent(), 10, height * 0.84, width * 0.8, 30, SWP_SHOWWINDOW);
	GetDlgItem(IDC_THRESHOLDTEXT)->SetWindowPos(GetParent(), width * 0.8 + 10, height * 0.84 + 5, 30, 30, SWP_SHOWWINDOW);
	GetDlgItem(IDC_XTEXT)->SetWindowPos(GetParent(), width * 0.92, height * 0.52 + 5, 10, 20, SWP_SHOWWINDOW);
	GetDlgItem(IDC_YTEXT)->SetWindowPos(GetParent(), width * 0.92 + 40, height * 0.52 + 5, 10, 20, SWP_SHOWWINDOW);
	GetDlgItem(IDC_REGTEXT)->SetWindowPos(GetParent(), width * 0.9, height * 0.2 + 5, rect.Width(), rect.Height(), SWP_SHOWWINDOW);
	GetDlgItem(IDC_DATATEXT)->SetWindowPos(GetParent(), width * 0.9, height * 0.3 + 5, rect.Width(), rect.Height(), SWP_SHOWWINDOW);

	GetDlgItem(IDC_SPEEDBTN)->GetWindowRect(&rect);
	ScreenToClient(&rect); // 把螢幕座標轉換為父視窗的座標
	GetDlgItem(IDC_SPEEDBTN)->SetWindowPos(GetParent(), width * 0.92, height * 0.45, rect.Width(), rect.Height(), SWP_SHOWWINDOW);

	GetDlgItem(IDC_SPEEDBTN2)->GetWindowRect(&rect);
	ScreenToClient(&rect); // 把螢幕座標轉換為父視窗的座標
	GetDlgItem(IDC_SPEEDBTN2)->SetWindowPos(GetParent(), width * 0.92, height * 0.4, rect.Width(), rect.Height(), SWP_SHOWWINDOW);

	m_sliderThreshold.SetRange(0, 7500);
	m_sliderThreshold.SetTicFreq(500);
	m_sliderThreshold.SetPos(defaultThreshold);

	UpdateSliderValue(m_sliderThreshold.GetPos());

	GetClientRect(m_initClientRect);
	UINT controlIDs[] = {
		IDC_PIC, IDC_PIC1, IDC_PIC2, IDC_PIC3,
		IDC_PREBTN, IDCANCEL, IDC_EDIT1, IDC_EDIT2, IDC_EDIT3, IDC_EDIT4, IDC_EDIT5, IDC_EDIT6, IDC_EDITBTN, IDC_MAXBTN, IDC_POINTBTN,
		IDC_LIST2, IDC_SLIDER1, IDC_THRESHOLDTEXT, IDC_XTEXT, IDC_YTEXT, IDC_REGTEXT, IDC_DATATEXT, IDC_SPEEDBTN, IDC_SPEEDBTN2
	};
	for (UINT id : controlIDs) {
		GetDlgItem(id)->GetWindowRect(rect);
		ScreenToClient(&rect);
		m_controls[id] = rect;
	}

	pLTSubView.top = m_controls[IDC_PIC].top;
	pLTSubView.left = m_controls[IDC_PIC].left;
	pLTSubView.bottom = m_controls[IDC_PIC].bottom;
	pLTSubView.right = m_controls[IDC_PIC].right;

	/*pRTSubView.top = m_controls[IDC_PIC2].top;
	pRTSubView.left = m_controls[IDC_PIC2].left;
	pRTSubView.bottom = m_controls[IDC_PIC2].bottom;
	pRTSubView.right = m_controls[IDC_PIC2].right;*/

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

// 視窗大小改變時，等比例調整元件大小與位置
void CDToFDemoAppDlg::OnSize(UINT nType, int cx, int cy) {
	CDialogEx::OnSize(nType, cx, cy);

	if (0 == m_initClientRect.Width() || 0 == m_initClientRect.Height()) return;

	CRect newClientRect;
	GetClientRect(&newClientRect);
	float scaleX = (float)newClientRect.Width() / m_initClientRect.Width();
	float scaleY = (float)newClientRect.Height() / m_initClientRect.Height();

	for (const auto& [id, initRect] : m_controls) {
		CRect rect;
		rect.left = (int)(initRect.left * scaleX);
		rect.top = (int)(initRect.top * scaleY);
		rect.right = (int)(initRect.right * scaleX);
		rect.bottom = (int)(initRect.bottom * scaleY);

		GetDlgItem(id)->MoveWindow(&rect);
	}
	pLTSubView.top = 0;
	pLTSubView.left = 0;
	pLTSubView.bottom = m_controls[IDC_PIC].bottom;
	pLTSubView.right = m_controls[IDC_PIC].right;

	/*pRTSubView.top = m_controls[IDC_PIC2].top;
	pRTSubView.left = m_controls[IDC_PIC2].left;
	pRTSubView.bottom = m_controls[IDC_PIC2].bottom;
	pRTSubView.right = m_controls[IDC_PIC2].right;*/

	directShowCamera->setSubViewWH(m_controls[IDC_PIC].Width(), m_controls[IDC_PIC].Height());
}

// 當使用者拖曳最小化視窗時，
// 系統呼叫這個功能取得游標顯示。
HCURSOR CDToFDemoAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDToFDemoAppDlg::OnBnClickedPreview() {
	const std::string dToFDeviceName = "CX3-UVC", rgbDeviceName = "USB Camera";
	const int testX = 500, testY = 500;
	std::vector<std::string> list;
	int deviceCount = directShowCamera->listDevices(list);
	for (int i = 0; i < deviceCount; ++i) {
		std::string str(list.at(i));
		m_deviceListBox.InsertString(i, CString(str.c_str()));
	}
	m_deviceListBox.UpdateData(TRUE);
	SetSubView();
	//DisplaySubView();
	directShowCamera->openCamera(dToFDeviceName);
	directShowCamera->prepareCamera();
	directShowCamera->run();
	/*directShowCamera[1].openCamera(rgbDeviceName);
	directShowCamera[1].prepareCamera();
	directShowCamera[1].run();*/
	MSG msg;
	MoveMouseTo(testX, testY);
	while (directShowCamera->isPreview) {
		if (GetMessage(&msg, nullptr, 0, 0) > 0) {
			if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN) {
				// 如果按下 Enter 鍵，退出消息循環
				directShowCamera->isPreview = false;
				//directShowCamera[1].isPreview = false;
				PostQuitMessage(0); // 發送 WM_QUIT
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			// 如果 GetMessage 返回 0，表示收到 WM_QUIT
			directShowCamera->isPreview = false;
			//directShowCamera[1].isPreview = false;
		}
	}
}

void CDToFDemoAppDlg::OnBnClickedCancel() {
	directShowCamera->stop();
	//directShowCamera[1].stop();
	PostQuitMessage(0); // 發送 WM_QUIT
}

void CDToFDemoAppDlg::OnBnClickedButtonSetText() {
	CString inputText;
	m_editControl.GetWindowTextW(inputText);
	if (!inputText.IsEmpty()) {
		int number = _ttoi(inputText);
		0 != number ? directShowCamera->writeFile(number) : directShowCamera->writeFile(defaultFileCount);
	}
}

void CDToFDemoAppDlg::OnBnClickedSetMaxValue() {
	CString inputText;
	
	m_maxEditControl.GetWindowTextW(inputText);
	if (!inputText.IsEmpty()) {
		int number = _ttoi(inputText);
		number = defaultThreshold < number ? number : defaultThreshold;
		directShowCamera->setMaxValue(number);
	}
}

void CDToFDemoAppDlg::OnBnClickedSetPointXY() {
	RECT m_rect;
	CString inputTextX, inputTextY;
	m_pointXEditControl.GetWindowTextW(inputTextX);
	m_pointYEditControl.GetWindowTextW(inputTextY);
	int x = (!inputTextX.IsEmpty()) ? _ttoi(inputTextX) : -1;
	int y = (!inputTextY.IsEmpty()) ? _ttoi(inputTextY) : -1;
	int index = (0 < x && 0 < y) ? index = x + y * 24 : -1;
	GetDlgItem(IDC_PIC1)->GetWindowRect(&m_rect);
	directShowCamera->setHistIndex(index, m_rect.right - m_rect.left, m_rect.bottom - m_rect.top);
}

void CDToFDemoAppDlg::OnBnClickedSpeedUp() {
	const CString cstr[] = { _T("DefaultSpeed"), _T("SpeedUp") };
	CString cs;
	GetDlgItem(IDC_SPEEDBTN)->GetWindowText(cs);
	if (cs == cstr[0]) {
		GetDlgItem(IDC_SPEEDBTN)->SetWindowText(cstr[1]);
		directShowCamera->setSpeedUp();
	}
	else {
		GetDlgItem(IDC_SPEEDBTN)->SetWindowText(cstr[0]);
		directShowCamera->setDefaultSpeed();
	}
}

void CDToFDemoAppDlg::OnStnClickedTransfer() {
	CString inputText, inText;

	m_RegEditControl.GetWindowTextW(inputText);
	m_DataEditControl.GetWindowTextW(inText);
	if (!inputText.IsEmpty() && !inText.IsEmpty()) {
		wchar_t* endPtr = nullptr;
		uint16_t reg = (uint16_t)wcstol(inputText, &endPtr, 16);
		uint8_t data = (uint8_t)wcstol(inText, &endPtr, 16);
		directShowCamera->sendCx3Command(reg, data);
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
	/*else if (cursorRTFlag && pt.x >= pRTSubView.left && pt.x <= pRTSubView.right && pt.y >= pRTSubView.top && pt.y <= pRTSubView.bottom) {
		int index = -1;
		POINT* histpoints = directShowCamera->getPoints();
		index = Get2DPos(pt, histpoints);
		RECT m_rect;
		GetDlgItem(IDC_PIC1)->GetWindowRect(&m_rect);
		directShowCamera->setHistIndex(index, m_rect.right - m_rect.left, m_rect.bottom - m_rect.top);
	}*/
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

void CDToFDemoAppDlg::OnHScroll(NMHDR* pNMHDR, LRESULT* pResult) {
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此加入控制項告知處理常式程式碼
	UpdateSliderValue(m_sliderThreshold.GetPos());
	*pResult = 0;
}

void CDToFDemoAppDlg::SetSubView() {
	directShowCamera->setSubViewWH(m_controls[IDC_PIC].Width(), m_controls[IDC_PIC].Height());
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
		m_controls[IDC_PIC].Width(), m_controls[IDC_PIC].Height(),
		SWP_SHOWWINDOW);
	GetDlgItem(IDC_PIC1)->SetWindowPos(
		GetParent(),
		m_controls[IDC_PIC1].Width() + 1, 0,
		m_controls[IDC_PIC1].Width(), m_controls[IDC_PIC1].Height(),
		SWP_SHOWWINDOW);
	GetDlgItem(IDC_PIC2)->SetWindowPos(
		GetParent(),
		0, m_controls[IDC_PIC2].Height() + 1,
		m_controls[IDC_PIC2].Width(), m_controls[IDC_PIC2].Height(),
		SWP_SHOWWINDOW);
	GetDlgItem(IDC_PIC3)->SetWindowPos(
		GetParent(),
		m_controls[IDC_PIC3].Width() + 1, m_controls[IDC_PIC3].Height() + 1,
		m_controls[IDC_PIC3].Width(), m_controls[IDC_PIC3].Height(),
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

void CDToFDemoAppDlg::UpdateSliderValue(int value) {
	CString strValue;
	strValue.Format(_T("%d"), value);
	m_thresholdText.SetWindowText(strValue);
	directShowCamera->setFilterThreshold(value);
}

void CDToFDemoAppDlg::MoveMouseTo(int x, int y) {
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// 轉換為絕對座標（0~65535）
	int dx = (x * 65535) / screenWidth;
	int dy = (y * 65535) / screenHeight;

	INPUT input = { 0 };
	input.type = INPUT_MOUSE;
	input.mi.dx = dx;
	input.mi.dy = dy;
	input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;

	SendInput(1, &input, sizeof(INPUT));
}
