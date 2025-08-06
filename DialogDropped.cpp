// DialogDropped.cpp : 実装ファイル
//

#include "pch.h"

#include "PropertyParameter.h"
#include "DialogFmtWebp.h"
#include "DialogFmtOther.h"
#include "DialogDropped.h"
#include "ConvertThread.h"
#include "WebpDropConverter.h"
#include "WebpDropConverterDlg.h"
#include "afxdialogex.h"


// DialogDropped ダイアログ

IMPLEMENT_DYNAMIC( DialogDropped, CDialogEx )

DialogDropped::DialogDropped( CWnd* pParent /*=nullptr*/ )
	: CDialogEx( IDD_PROP_Dropped, pParent ), ConvertWorker( nullptr ), OrigRect{ 0 }
{}

DialogDropped::~DialogDropped()
{}


void DialogDropped::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_PROGRESS1, ProgBar );
	DDX_Control( pDX, IDC_CONVERTSTOP, ButtonProgressStop );
	DDX_Control( pDX, IDC_THREADSTRING, ThreadLabel );
	DDX_Check( pDX, IDC_TRASHSOURCE, bTrashSource );
	DDX_Control( pDX, IDC_TRASHSOURCE, TrashSourceBtn );
}

BEGIN_MESSAGE_MAP( DialogDropped, CDialogEx )
	ON_WM_DROPFILES()
	ON_BN_CLICKED( IDC_CONVERTSTOP, &DialogDropped::OnBnClickedStopConvert )
	ON_BN_CLICKED( IDC_TRASHSOURCE, &DialogDropped::OnBnClickedTrashsource )
	ON_WM_DROPFILES()
END_MESSAGE_MAP()

// DialogDropped メッセージ ハンドラー
BOOL
DialogDropped::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	DragAcceptFiles( TRUE );
	ProgBar.EnableWindow( FALSE );
	ButtonProgressStop.EnableWindow( FALSE );
	TrashSourceBtn.EnableWindow( TRUE );

	bTrashSource = pEncParam->bTrashSource;
	UpdateData( FALSE );

	return TRUE;
}

void DialogDropped::OnBnClickedStopConvert()
{
	MyMessage mess;
	mess.Type = MessageType::MM_QUIT;

	ConvertWorker->PushMessage( mess );
}

void
DialogDropped::EnableControls( bool bWorking )
{
	if( bWorking )
	{
		ProgBar.EnableWindow( TRUE );
		ButtonProgressStop.EnableWindow( TRUE );
		TrashSourceBtn.EnableWindow( TRUE );
	}
	else
	{
		ProgBar.EnableWindow( FALSE );
		ButtonProgressStop.EnableWindow( FALSE );
		TrashSourceBtn.EnableWindow( TRUE );

		ThreadLabel.SetWindowTextW( L"Drop Image File( s ) Here." );
	}
}
void DialogDropped::OnBnClickedTrashsource()
{
	// 元ファイルをゴミ箱へ　チェックボックスが押された
	UpdateData( TRUE );
	pEncParam->bTrashSource = bTrashSource;
	pEncParam->WriteIniFile();
}

void DialogDropped::OnDropFiles( HDROP hDropInfo )
{
	// ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。
	CWnd* pMainWnd = AfxGetMainWnd();
	if( pMainWnd && ::IsWindow( pMainWnd->GetSafeHwnd() ) )
	{
		::SendMessage( pMainWnd->GetSafeHwnd(), WM_DROPFILES, (WPARAM)hDropInfo, 0 );
	}

	CDialogEx::OnDropFiles( hDropInfo );
}
