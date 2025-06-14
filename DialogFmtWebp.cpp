// DialogProperty.cpp : 実装ファイル
//
#include "pch.h"

#include "PropertyParameter.h"
#include "WebpDropConverter.h"
#include "DialogFmtWebp.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma warning( disable : 26454 )

#pragma region dialog
// DialogProperty ダイアログ
IMPLEMENT_DYNAMIC( DialogFmtWebp, CDialogEx )

DialogFmtWebp::DialogFmtWebp( CWnd* pParent, PropertyParameter* _pEncParam )
	: CDialogEx( IDD_FMTWEBP, pParent ), pEncParam( _pEncParam ), OrigRect{ 0 }
{}

DialogFmtWebp::~DialogFmtWebp()
{}


void DialogFmtWebp::FlushParameter()
{
	writeIniParamEnc();
}

void DialogFmtWebp::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_JPEGQUALITYSLIDER, jpegQualitySlider );
	DDX_Control( pDX, IDC_STATICJPVALUE, labelJpegQualityValue );
	DDX_Control( pDX, IDC_RADIOPNG, radioPNG );
	DDX_Control( pDX, IDC_RADIOJPG, radioJPG );
	DDX_Control( pDX, IDC_RADIOBMP, radioBMP );
	DDX_Control( pDX, IDC_RADIOTIF, radioTIF );
}
#pragma endregion

BEGIN_MESSAGE_MAP( DialogFmtWebp, CDialogEx )
	ON_WM_PAINT()
	ON_NOTIFY( NM_RELEASEDCAPTURE, IDC_JPEGQUALITYSLIDER, &DialogFmtWebp::OnReleasedJpegqualitySlider )
END_MESSAGE_MAP()

BOOL
DialogFmtWebp::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	jpegQualitySlider.SetRange( 0, 100 );
	jpegQualitySlider.SetTicFreq( 5 );

	getIniParamEnc();
	setCtrlsValue();

	return TRUE;
}

/// <summary>
/// 各コントロールへパラメータ値を書き込む
/// </summary>
void DialogFmtWebp::setCtrlsValue()
{
	jpegQualitySlider.SetPos( pEncParam->Webp2JpegQuality );
	wchar_t sValue[ 64 ];
	_itow_s( pEncParam->Webp2JpegQuality, sValue, 10 );
	labelJpegQualityValue.SetWindowText( sValue );

	radioPNG.SetCheck( BST_UNCHECKED );
	radioJPG.SetCheck( BST_UNCHECKED );
	radioBMP.SetCheck( BST_UNCHECKED );
	radioTIF.SetCheck( BST_UNCHECKED );

	if( strcmp( pEncParam->Webp2Format, "PNG" ) == 0 )
		radioPNG.SetCheck( BST_CHECKED );
	if( strcmp( pEncParam->Webp2Format, "JPG" ) == 0 )
		radioJPG.SetCheck( BST_CHECKED );
	if( strcmp( pEncParam->Webp2Format, "BMP" ) == 0 )
		radioBMP.SetCheck( BST_CHECKED );
	if( strcmp( pEncParam->Webp2Format, "TIF" ) == 0 )
		radioBMP.SetCheck( BST_CHECKED );
}

/// <summary>
/// 各コントロールからパラメータ値を取得
/// </summary>
void DialogFmtWebp::getCtrlsValue()
{
	pEncParam->Webp2JpegQuality = jpegQualitySlider.GetPos();

	if( radioPNG.GetCheck() == BST_CHECKED )
		strcpy_s( pEncParam->Webp2Format, "PNG" );
	else if( radioJPG.GetCheck() == BST_CHECKED )
		strcpy_s( pEncParam->Webp2Format, "JPG" );
	else if( radioBMP.GetCheck() == BST_CHECKED )
		strcpy_s( pEncParam->Webp2Format, "BMP" );
	else if( radioTIF.GetCheck() == BST_CHECKED )
		strcpy_s( pEncParam->Webp2Format, "TIF" );
}

/// <summary>
/// Ini ファイルからパラメータ値を取得
/// </summary>
void DialogFmtWebp::getIniParamEnc()
{
	pEncParam->GetFromIniFile();
}

/// <summary>
/// Ini ファイルに各パラメータを書き込む
/// </summary>
void DialogFmtWebp::writeIniParamEnc()
{
	getCtrlsValue();
	pEncParam->WriteIniFile();
}

void
DialogFmtWebp::OnReleasedJpegqualitySlider( NMHDR* pNMHDR, LRESULT* pResult )
{
	auto iPos = jpegQualitySlider.GetPos();
	wchar_t sValue[ 16 ];
	_itow_s( iPos, sValue, 10 );
	labelJpegQualityValue.SetWindowTextW( sValue );

	*pResult = 0;
}
