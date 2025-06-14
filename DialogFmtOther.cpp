// DialogFmtOther : 実装ファイル
//
#include "pch.h"
#include "PropertyParameter.h"
#include "WebpDropConverter.h"
#include "DialogFmtOther.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma warning( disable : 26454 )

#pragma region dialog
// DialogProperty ダイアログ
IMPLEMENT_DYNAMIC( DialogFmtOther, CDialogEx )

DialogFmtOther::DialogFmtOther( CWnd* pParent  )
	: CDialogEx( IDD_FMTOTHER, pParent ), OrigRect{ 0 }
{}

DialogFmtOther::~DialogFmtOther()
{}

void DialogFmtOther::FlushParameter()
{
	writeIniParamEnc();
}

void DialogFmtOther::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_OTHR_WEBPQL, webpQualitySlider );
	DDX_Control( pDX, IDC_OTHR_JPGQL, jpegQualitySlider );
	DDX_Control( pDX, IDC_STATICWPVALUE, labelWebpQualityValue );
	DDX_Control( pDX, IDC_STATICJPVALUE, labelJpegQualityValue );
	DDX_Control( pDX, IDC_RB_OTHR2WEBP, radioWbp );
	DDX_Control( pDX, IDC_RB_OTHR2PNG, radioPNG );
	DDX_Control( pDX, IDC_RB_OTHR2JPG, radioJPG );
	DDX_Control( pDX, IDC_RB_OTHR2BMP, radioBMP );
}
#pragma endregion

BEGIN_MESSAGE_MAP( DialogFmtOther, CDialogEx )
	ON_WM_PAINT()
	ON_NOTIFY( NM_RELEASEDCAPTURE, IDC_OTHR_WEBPQL, &DialogFmtOther::OnReleasedWebpQualitySlider )
	ON_NOTIFY( NM_RELEASEDCAPTURE, IDC_OTHR_JPGQL, &DialogFmtOther::OnReleasedJpegqualitySlider )
END_MESSAGE_MAP()

BOOL
DialogFmtOther::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	jpegQualitySlider.SetRange( 0, 100 );
	jpegQualitySlider.SetTicFreq( 5 );
	webpQualitySlider.SetRange( 0, 100 );
	webpQualitySlider.SetTicFreq( 5 );

	getIniParamEnc();
	setCtrlsValue();

	return TRUE;
}

/// <summary>
/// 各コントロールへパラメータ値を書き込む
/// </summary>
void DialogFmtOther::setCtrlsValue()
{
	// WebpQuality
	webpQualitySlider.SetPos( pEncParam->Othr2WebpQuality );
	jpegQualitySlider.SetPos( pEncParam->Othr2JpegQuality );
	wchar_t sValue[ 64 ];
	_itow_s( pEncParam->Othr2WebpQuality, sValue, 10 );
	labelWebpQualityValue.SetWindowText( sValue );
	_itow_s( pEncParam->Othr2JpegQuality, sValue, 10 );
	labelJpegQualityValue.SetWindowText( sValue );

	radioPNG.SetCheck( BST_UNCHECKED );
	radioJPG.SetCheck( BST_UNCHECKED );
	radioBMP.SetCheck( BST_UNCHECKED );

	if( strcmp( pEncParam->Othr2Format, "WEBP" ) == 0 )
		radioWbp.SetCheck( BST_CHECKED );
	else if( strcmp( pEncParam->Othr2Format, "PNG" ) == 0 )
		radioPNG.SetCheck( BST_CHECKED );
	else if( strcmp( pEncParam->Othr2Format, "JPG" ) == 0 )
		radioJPG.SetCheck( BST_CHECKED );
	else if( strcmp( pEncParam->Othr2Format, "BMP" ) == 0 )
		radioBMP.SetCheck( BST_CHECKED );
	else if( strcmp( pEncParam->Othr2Format, "TIF" ) == 0 )
		radioBMP.SetCheck( BST_CHECKED );
}

/// <summary>
/// 各コントロールからパラメータ値を取得
/// </summary>
void DialogFmtOther::getCtrlsValue()
{
	pEncParam->Othr2JpegQuality = jpegQualitySlider.GetPos();
	pEncParam->Othr2WebpQuality = webpQualitySlider.GetPos();

	if( radioWbp.GetCheck() == BST_CHECKED )
		strcpy_s( pEncParam->Othr2Format, "WEBP" );
	else if( radioPNG.GetCheck() == BST_CHECKED )
		strcpy_s( pEncParam->Othr2Format, "PNG" );
	else if( radioJPG.GetCheck() == BST_CHECKED )
		strcpy_s( pEncParam->Othr2Format, "JPG" );
	else if( radioBMP.GetCheck() == BST_CHECKED )
		strcpy_s( pEncParam->Othr2Format, "BMP" );
}

/// <summary>
/// Ini ファイルからパラメータ値を取得
/// </summary>
void DialogFmtOther::getIniParamEnc()
{
	pEncParam->GetFromIniFile();
}

/// <summary>
/// Ini ファイルに各パラメータを書き込む
/// </summary>
void DialogFmtOther::writeIniParamEnc()
{
	getCtrlsValue();
	pEncParam->WriteIniFile();

}

void DialogFmtOther::OnReleasedWebpQualitySlider( NMHDR* pNMHDR, LRESULT* pResult )
{
	auto iPos = webpQualitySlider.GetPos();
	wchar_t sValue[ 16 ];
	_itow_s( iPos, sValue, 10 );
	labelWebpQualityValue.SetWindowTextW( sValue );

	*pResult = 0;
}

void
DialogFmtOther::OnReleasedJpegqualitySlider( NMHDR* pNMHDR, LRESULT* pResult )
{
	auto iPos = jpegQualitySlider.GetPos();
	wchar_t sValue[ 16 ];
	_itow_s( iPos, sValue, 10 );
	labelJpegQualityValue.SetWindowTextW( sValue );

	*pResult = 0;
}
