// WebpDropConverterDlg.cpp : 実装ファイル
//

#include "pch.h"
#include "framework.h"
#include "PropertyParameter.h"
#include "DialogFmtWebp.h"
#include "DialogFmtOther.h"
#include "ConvertThread.h"
#include "DialogDropped.h"

#include "WebpDropConverter.h"
#include "WebpDropConverterDlg.h"

#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma warning( disable : 26454 6387 )

#define WM_USER_THREAD ( WM_USER + 1 )

#pragma region AboutDialog
// アプリケーションのバージョン情報に使われる CAboutDlg ダイアログ
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV サポート

// 実装
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx( IDD_ABOUTBOX )
{}

void CAboutDlg::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
}

BEGIN_MESSAGE_MAP( CAboutDlg, CDialogEx )
END_MESSAGE_MAP()
#pragma endregion

// CWebpDropConverterDlg ダイアログ
CWebpDropConverterDlg::CWebpDropConverterDlg( CWnd* pParent )
	: CDialogEx( IDD_WEBPDROPCONVERTER_DIALOG, pParent )
{
	m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );

	// IniFile
	appIni = new XmlIni( L"Application.xml" );
	pEncParam = new PropertyParameter();
	FmtWebpDlg.pEncParam = pEncParam;
	FmtOthrDlg.pEncParam = pEncParam;
	DroppedDlg.pEncParam = pEncParam;
}

CWebpDropConverterDlg::~CWebpDropConverterDlg()
{
	delete pEncParam;
	delete appIni;
}

void CWebpDropConverterDlg::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_TAB1, tab );
}

BEGIN_MESSAGE_MAP( CWebpDropConverterDlg, CDialogEx )
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY( TCN_SELCHANGE, IDC_TAB1, &CWebpDropConverterDlg::OnTcnSelchangeTab1 )
	ON_MESSAGE( (UINT)MessageType::MM_PROGRESS, OnGetProgressMessage )
	ON_MESSAGE( (UINT)MessageType::MM_STRING, OnGetStringMessage )
	ON_WM_DROPFILES()
	ON_WM_MOVE()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

// CWebpDropConverterDlg メッセージ ハンドラー
BOOL
CWebpDropConverterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// "バージョン情報..." メニューをシステム メニューに追加します。

	// IDM_ABOUTBOX は、システム コマンドの範囲内になければなりません。
	ASSERT( ( IDM_ABOUTBOX & 0xFFF0 ) == IDM_ABOUTBOX );
	ASSERT( IDM_ABOUTBOX < 0xF000 );

	CMenu* pSysMenu = GetSystemMenu( FALSE );
	if( pSysMenu != nullptr )
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString( IDS_ABOUTBOX );
		ASSERT( bNameValid );
		if( !strAboutMenu.IsEmpty() )
		{
			pSysMenu->AppendMenu( MF_SEPARATOR );
			pSysMenu->AppendMenu( MF_STRING, IDM_ABOUTBOX, strAboutMenu );
		}
	}

	// このダイアログのアイコンを設定します。アプリケーションのメイン ウィンドウがダイアログでない場合、
	//  Framework は、この設定を自動的に行います。
	DragAcceptFiles( TRUE );

	SetIcon( m_hIcon, TRUE );		// 大きいアイコンの設定
	SetIcon( m_hIcon, FALSE );		// 小さいアイコンの設定

	// タブコントロールを追加
	tab.InsertItem( 0, _T( "  Drop" ) );
	tab.InsertItem( 1, _T( "  DropWebp" ) );
	tab.InsertItem( 2, _T( "  DropOther" ) );

	// タブのプロパティシート
	FmtWebpDlg.Create( IDD_FMTWEBP, &tab );
	FmtOthrDlg.Create( IDD_FMTOTHER, &tab );
	DroppedDlg.Create( IDD_PROP_Dropped, &tab );
	

	FmtWebpDlg.GetWindowRect( &FmtWebpDlg.OrigRect );
	FmtOthrDlg.GetWindowRect( &FmtOthrDlg.OrigRect );
	DroppedDlg.GetWindowRect( &DroppedDlg.OrigRect );

	// シートのズレを補正する
	CRect rect;
	tab.GetClientRect( &rect );
	rect.left += 2;
	rect.right -= 4;
	rect.top += 20;
	rect.bottom -= 4;

	tab.GetWindowRect( rect );
	tab.AdjustRect( FALSE, rect );
	tab.ScreenToClient( rect );

	FmtWebpDlg.MoveWindow( rect );
	FmtOthrDlg.MoveWindow( rect );
	DroppedDlg.MoveWindow( rect );

	// タブ 0 をカレントにし、ダイアログ １ を表示する
	tab.SetCurSel( 0 );
	DroppedDlg.ShowWindow( SW_SHOW );
	FmtWebpDlg.ShowWindow( SW_HIDE );
	FmtOthrDlg.ShowWindow( SW_HIDE );

	RECT dlgRect = DroppedDlg.OrigRect;
	auto width = dlgRect.right - dlgRect.left + 10;
	auto height = dlgRect.bottom - dlgRect.top + 60;

	auto x = appIni->GetIniInt( "WindowRegion", "x", 100 );
	auto y = appIni->GetIniInt( "WindowRegion", "y", 100 );

	this->SetWindowPos( NULL, x, y, width, height, SWP_NOZORDER );

	return TRUE;
}

void CWebpDropConverterDlg::OnSysCommand( UINT nID, LPARAM lParam )
{
	if( ( nID & 0xFFF0 ) == IDM_ABOUTBOX )
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand( nID, lParam );
	}
}

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。
void CWebpDropConverterDlg::OnPaint()
{
	if( IsIconic() )
	{
		CPaintDC dc( this ); // 描画のデバイス コンテキスト

		SendMessage( WM_ICONERASEBKGND, reinterpret_cast<WPARAM>( dc.GetSafeHdc() ), 0 );

		// クライアントの四角形領域内の中央
		int cxIcon = GetSystemMetrics( SM_CXICON );
		int cyIcon = GetSystemMetrics( SM_CYICON );
		CRect rect;
		GetClientRect( &rect );
		int x = ( rect.Width() - cxIcon + 1 ) / 2;
		int y = ( rect.Height() - cyIcon + 1 ) / 2;

		// アイコンの描画
		dc.DrawIcon( x, y, m_hIcon );
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ユーザーが最小化したウィンドウをドラッグしているときに表示するカーソルを取得するために、
//  システムがこの関数を呼び出します。
HCURSOR CWebpDropConverterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>( m_hIcon );
}

void CWebpDropConverterDlg::OnTcnSelchangeTab1( NMHDR* pNMHDR, LRESULT* pResult )
{
	FmtWebpDlg.FlushParameter();
	FmtOthrDlg.FlushParameter();

	switch( tab.GetCurSel() )
	{
		case 0:
		{
			DroppedDlg.ShowWindow( SW_SHOW );
			FmtWebpDlg.ShowWindow( SW_HIDE );
			FmtOthrDlg.ShowWindow( SW_HIDE );

			RECT dlgRect;
			dlgRect = DroppedDlg.OrigRect;
			auto width = dlgRect.right - dlgRect.left + 10;
			auto height = dlgRect.bottom - dlgRect.top + 60;
			this->SetWindowPos( NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER );
			DragAcceptFiles( TRUE );
			break;
		}
		case 1:
		{
			DroppedDlg.ShowWindow( SW_HIDE );
			FmtWebpDlg.ShowWindow( SW_SHOW );
			FmtOthrDlg.ShowWindow( SW_HIDE );

			RECT dlgRect;
			dlgRect = FmtWebpDlg.OrigRect;
			auto width = dlgRect.right - dlgRect.left + 10;
			auto height = dlgRect.bottom - dlgRect.top + 70;
			this->SetWindowPos( NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER );
			DragAcceptFiles( TRUE );  // プロパティシートは Drag を受け付けないので苦肉の策
			// Drop のダイアログは不要かも（泣
			break;
		}
		case 2:
		{
			DroppedDlg.ShowWindow( SW_HIDE );
			FmtWebpDlg.ShowWindow( SW_HIDE );
			FmtOthrDlg.ShowWindow( SW_SHOW );

			RECT dlgRect;
			dlgRect = FmtOthrDlg.OrigRect;
			auto width = dlgRect.right - dlgRect.left + 10;
			auto height = dlgRect.bottom - dlgRect.top + 70;
			this->SetWindowPos( NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER );
			DragAcceptFiles( TRUE );
			break;
		}

		default:
			;
	}

	*pResult = 0;
}

void
CWebpDropConverterDlg::uTF8ToShiftJis( LPSTR bufShiftJis, LPWSTR bufUTF8 )
{
	int ret = WideCharToMultiByte( CP_ACP, 0, bufUTF8, -1, bufShiftJis, MAX_PATH, NULL, NULL );
}

LRESULT
CWebpDropConverterDlg::OnGetProgressMessage( WPARAM wParam, LPARAM lParam )
{
	auto m = (int)lParam + 1;
	DroppedDlg.ProgBar.SetPos( m );

	if( lParam == -1 )  // 終了時はスレッドから -1 が渡ってくる
	{
		DroppedDlg.EnableControls( false );

		th->join();
		delete worker;
		delete th;
	}

	return TRUE;
}

LRESULT
CWebpDropConverterDlg::OnGetStringMessage( WPARAM wParam, LPARAM lParam )
{
	auto m = (LPCWSTR)lParam;

	DroppedDlg.ThreadLabel.SetWindowTextW( m );

	return TRUE;
}

void 
CWebpDropConverterDlg::OnDropFiles( HDROP hDropInfo )
{
	droppedFiles.clear();

	UINT uiCount = DragQueryFile( hDropInfo, -1, NULL, 0 );

	for( UINT i = 0; i < uiCount; i++ )
	{
		//ファイル名の長さを取得
		UINT uiLen = DragQueryFile( hDropInfo, i, NULL, 0 );

		//ファイル名を取得
		wchar_t fileName[ MAX_PATH ];
		DragQueryFile( hDropInfo, i, fileName, MAX_PATH );

		std::wstring fileNameString( fileName );
		droppedFiles.push_back( fileNameString );
	}

	DroppedDlg.ProgBar.SetRange( 0, uiCount );

	worker = new ConvertThread( this, WM_USER_THREAD, droppedFiles, pEncParam );
	th = new std::thread( std::ref( *worker ) );

	DroppedDlg.EnableControls( true );
	DroppedDlg.ConvertWorker = worker;

	CDialogEx::OnDropFiles( hDropInfo );
}

void CWebpDropConverterDlg::OnMove( int x, int y )
{
	CDialogEx::OnMove( x, y );

	if( ! IsWindowVisible() )  // 初期化の時は x, y がデタラメな数値で呼ばれる
		return;

	appIni->SetIniInt( "WindowRegion", "x", x );
	appIni->SetIniInt( "WindowRegion", "y", y );
	appIni->WriteFlush();
}

void CWebpDropConverterDlg::OnClose()
{
	FmtWebpDlg.FlushParameter();
	FmtOthrDlg.FlushParameter();

	CDialogEx::OnClose();
}
