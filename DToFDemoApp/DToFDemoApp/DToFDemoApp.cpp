﻿
// DToFDemoApp.cpp: 定義應用程式的類別表現方式。
//

#include "pch.h"
#include "framework.h"
#include "DToFDemoApp.h"
#include "DToFDemoAppDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDToFDemoAppApp

BEGIN_MESSAGE_MAP(CDToFDemoAppApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CDToFDemoAppApp 建構

CDToFDemoAppApp::CDToFDemoAppApp()
{
	// 支援重新啟動管理員
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 在此加入建構程式碼，
	// 將所有重要的初始設定加入 InitInstance 中
}


// 唯一一個 CDToFDemoAppApp 物件

CDToFDemoAppApp theApp;


// CDToFDemoAppApp 初始化

BOOL CDToFDemoAppApp::InitInstance()
{
	//若應用程式有以下情況時，則 Windows XP 上必須要有 InitCommonControlsEx():
	// 來啟動視覺化樣式，在 Windows XP 上，則需要 InitCommonControls()。
	// 否則任何視窗的建立都將失敗。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 設定要包含所有您想要用於應用程式中的
	// 通用控制項類別。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	mutexhandle = CreateMutex(NULL, TRUE, _T("D10F5CDB-2757-4E82-B219-BF53639A7D87"));
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		exit(0);

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}


	AfxEnableControlContainer();

	// 建立殼層管理員，以防對話方塊包含
	// 任何殼層樹狀檢視或殼層清單檢視控制項。
	CShellManager *pShellManager = new CShellManager;

	// 啟動 [Windows 原生] 視覺化管理員可啟用 MFC 控制項中的主題
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// 標準初始設定
	// 如果您不使用這些功能並且想減少
	// 最後完成的可執行檔大小，您可以
	// 從下列程式碼移除不需要的初始化常式，
	// 變更儲存設定值的登錄機碼
	// TODO: 您應該適度修改此字串
	// (例如，公司名稱或組織名稱)
	SetRegistryKey(_T("本機 AppWizard 所產生的應用程式"));

	CDToFDemoAppDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置於使用 [Preview] 來停止使用對話方塊時
		// 處理的程式碼
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置於使用 [Cancel] 來停止使用對話方塊時
		// 處理的程式碼
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "警告: 對話方塊建立失敗，因此，應用程式意外終止。\n");
		TRACE(traceAppMsg, 0, "警告: 如果您要在對話方塊上使用 MFC 控制項，則無法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
	}

	// 刪除上面所建立的殼層管理員。
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// 因為已經關閉對話方塊，傳回 FALSE，所以我們會結束應用程式，
	// 而非提示開始應用程式的訊息。
	return FALSE;
}

