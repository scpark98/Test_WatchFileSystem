
// Test_WatchFileSystemDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "Test_WatchFileSystem.h"
#include "Test_WatchFileSystemDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
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


// CTestWatchFileSystemDlg 대화 상자



CTestWatchFileSystemDlg::CTestWatchFileSystemDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TEST_WATCHFILESYSTEM_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestWatchFileSystemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTestWatchFileSystemDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DROPFILES()
	ON_REGISTERED_MESSAGE(Message_CDirectoryChangeWatcher, &CTestWatchFileSystemDlg::on_message_CDirectoryChangeWatcher)
	ON_BN_CLICKED(IDOK, &CTestWatchFileSystemDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CTestWatchFileSystemDlg 메시지 처리기

BOOL CTestWatchFileSystemDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
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

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	DragAcceptFiles();
	m_dir_watcher.WatchDirectory(_T("D:\\temp"), m_hWnd);
	m_dir_watcher.WatchDirectory(_T("D:\\temp1"), m_hWnd);
	m_dir_watcher.WatchDirectory(_T("D:\\test"), m_hWnd, false);

	TCHAR ch = '1';
	ch = _totlower(ch);
	//m_watch_fs.init(m_hWnd);

	//m_watch_fs.add(_T("D:\\temp"));
	//m_watch_fs.add(_T("D:\\temp1"));
	//m_watch_fs.add(_T("D:\\test"));

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CTestWatchFileSystemDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CTestWatchFileSystemDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CTestWatchFileSystemDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTestWatchFileSystemDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	int count = DragQueryFile(hDropInfo, 0xffffffff, 0, 0);

	TCHAR path[MAX_PATH] = { 0, };

	for (int i = 0; i < count; i++)
	{
		DragQueryFile(hDropInfo, i, path, MAX_PATH);
		if (PathFileExists(path))
		{
			if (PathIsDirectory(path))
			{
				//m_watch_fs.add(path);

			}
			else
			{

			}
		}
	}

	CDialogEx::OnDropFiles(hDropInfo);
}

LRESULT CTestWatchFileSystemDlg::on_message_CDirectoryChangeWatcher(WPARAM wParam, LPARAM lParam)
{
	//FILE_ACTION_ADDED(1), FILE_ACTION_REMOVED(2), FILE_ACTION_RENAMED_OLD_NAME(4)
	CDirectoryChangeWatcherMessage* msg = (CDirectoryChangeWatcherMessage*)wParam;
	TRACE(_T("action = %d, filename0 = %s, filename1 = %s\n"), msg->action, msg->filename0, msg->filename1);
	return 0;
}


void CTestWatchFileSystemDlg::OnBnClickedOk()
{
	m_dir_watcher.UnwatchAllDirectories();
	//m_dir_watcher.UnwatchDirectory(_T("D:\\temp"));
	//m_dir_watcher.UnwatchDirectory(_T("D:\\temp1"));
	//m_dir_watcher.WatchDirectory(_T("D:\\test"));
}
