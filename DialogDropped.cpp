// DialogDropped.cpp : 実装ファイル
//

#include "pch.h"

#include "DialogProperty.h"
#include "ConvertThread.h"
#include "WebpDropConverter.h"
#include "DialogDropped.h"
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
}

BEGIN_MESSAGE_MAP( DialogDropped, CDialogEx )
	ON_WM_DROPFILES()
	ON_BN_CLICKED( IDC_CONVERTSTOP, &DialogDropped::OnBnClickedStopconvert )
END_MESSAGE_MAP()

// DialogDropped メッセージ ハンドラー
BOOL
DialogDropped::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	DragAcceptFiles();
	ProgBar.EnableWindow( FALSE );
	ButtonProgressStop.EnableWindow( FALSE );

	return TRUE;
}

void DialogDropped::OnDropFiles( HDROP hDropInfo )
{
	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。
	CDialogEx::OnDropFiles( hDropInfo );
}

void DialogDropped::OnBnClickedStopconvert()
{
	MyMessage mess;
	mess.Type = MessageType::MM_QUIT;

	ConvertWorker->PushMessage( mess );
}
