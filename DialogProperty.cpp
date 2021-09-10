// DialogProperty.cpp : 実装ファイル
//
#include "pch.h"
#include "WebpDropConverter.h"
#include "DialogProperty.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma warning( disable : 26454 )

#pragma region dialog
// DialogProperty ダイアログ
IMPLEMENT_DYNAMIC( DialogProperty, CDialogEx )

DialogProperty::DialogProperty( CWnd* pParent /*=nullptr*/ )
	: CDialogEx( IDD_PROPERTY, pParent ), iniFile( nullptr ), OrigRect{ 0 }
{}

DialogProperty::~DialogProperty()
{
	delete iniFile;
}

PropertyParameter
DialogProperty::GetEncodeParameter()
{
	return propParam;
}

void DialogProperty::FlushParameter()
{
	writeIniParamEnc();
}

void DialogProperty::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_WEBPQUALITYSLIDER, webpQualitySlider );
	DDX_Control( pDX, IDC_JPEGQUALITYSLIDER, jpegQualitySlider );
	DDX_Control( pDX, IDC_STATICWPVALUE, labelWebpQualityValue );
	DDX_Control( pDX, IDC_STATICJPVALUE, labelJpegQualityValue );
	DDX_Control( pDX, IDC_RADIOPNG, radioPNG );
	DDX_Control( pDX, IDC_RADIOJPG, radioJPG );
	DDX_Control( pDX, IDC_RADIOBMP, radioBMP );
	DDX_Control( pDX, IDC_RADIOTIF, radioTIF );
}
#pragma endregion

BEGIN_MESSAGE_MAP( DialogProperty, CDialogEx )
	ON_WM_PAINT()
	ON_NOTIFY( NM_RELEASEDCAPTURE, IDC_WEBPQUALITYSLIDER, &DialogProperty::OnReleasedWebpQualitySlider )
	ON_NOTIFY( NM_RELEASEDCAPTURE, IDC_JPEGQUALITYSLIDER, &DialogProperty::OnReleasedJpegqualityslider )
END_MESSAGE_MAP()

void
DialogProperty::OnPaint()
{
	CPaintDC dc( this ); // device context for painting
						 // 描画メッセージで CDialogEx::OnPaint() を呼び出さないでください。
}

BOOL
DialogProperty::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	iniFile = new XmlIni( L"Convert.xml" );

	webpQualitySlider.SetRange( 0, 100 );
	webpQualitySlider.SetTicFreq( 5 );
	jpegQualitySlider.SetRange( 0, 100 );
	jpegQualitySlider.SetTicFreq( 5 );

	getIniParamEnc();
	setCtrlsValue();

	return TRUE;
}

/// <summary>
/// 各コントロールへパラメータ値を書き込む
/// </summary>
void DialogProperty::setCtrlsValue()
{
	// WebpQuality
	webpQualitySlider.SetPos( propParam.webpQuality );
	jpegQualitySlider.SetPos( propParam.jpegQuality );
	wchar_t sValue[ 64 ];
	_itow_s( propParam.webpQuality, sValue, 10 );
	labelWebpQualityValue.SetWindowText( sValue );
	_itow_s( propParam.jpegQuality, sValue, 10 );
	labelJpegQualityValue.SetWindowText( sValue );

	radioPNG.SetCheck( BST_UNCHECKED );
	radioJPG.SetCheck( BST_UNCHECKED );
	radioBMP.SetCheck( BST_UNCHECKED );
	radioTIF.SetCheck( BST_UNCHECKED );

	if( strcmp( propParam.format, "PNG" ) == 0 )
		radioPNG.SetCheck( BST_CHECKED );
	if( strcmp( propParam.format, "JPG" ) == 0 )
		radioJPG.SetCheck( BST_CHECKED );
	if( strcmp( propParam.format, "BMP" ) == 0 )
		radioBMP.SetCheck( BST_CHECKED );
	if( strcmp( propParam.format, "TIF" ) == 0 )
		radioBMP.SetCheck( BST_CHECKED );
}

/// <summary>
/// 各コントロールからパラメータ値を取得
/// </summary>
void DialogProperty::getCtrlsValue()
{
	propParam.webpQuality = webpQualitySlider.GetPos();
	propParam.jpegQuality = jpegQualitySlider.GetPos();

	if( radioPNG.GetCheck() == BST_CHECKED )
		strcpy_s( propParam.format, "PNG" );
	else if( radioJPG.GetCheck() == BST_CHECKED )
		strcpy_s( propParam.format, "JPG" );
	else if( radioBMP.GetCheck() == BST_CHECKED )
		strcpy_s( propParam.format, "BMP" );
	else if( radioTIF.GetCheck() == BST_CHECKED )
		strcpy_s( propParam.format, "TIF" );
}

/// <summary>
/// Ini ファイルからパラメータ値を取得
/// </summary>
void DialogProperty::getIniParamEnc()
{
	propParam.webpQuality = iniFile->GetIniInt( "EncodeParameter", "WebpQuality", 80 );
	propParam.jpegQuality = iniFile->GetIniInt( "DecodeParameter", "JpegQuality", 80 );
	strcpy_s( propParam.format, iniFile->GetIniString( "DecodeParameter", "Encode", "PNG" ) );
}

/// <summary>
/// Ini ファイルに各パラメータを書き込む
/// </summary>
void DialogProperty::writeIniParamEnc()
{
	getCtrlsValue();
	iniFile->WriteIniInt( "EncodeParameter", "WebpQuality", propParam.webpQuality );
	iniFile->WriteIniInt( "DecodeParameter", "JpegQuality", propParam.jpegQuality );
	iniFile->WriteIniString( "DecodeParameter", "Encode", propParam.format );
	iniFile->WriteFlush();
}

/// <summary>
/// Quality Slider を離したときのコールバック
/// unresolved:何故か動かしてるときのコールバックが拾えない
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void
DialogProperty::OnReleasedWebpQualitySlider( NMHDR* pNMHDR, LRESULT* pResult )
{
	auto iPos = webpQualitySlider.GetPos();
	wchar_t sValue[ 16 ];
	_itow_s( iPos, sValue, 10 );
	labelWebpQualityValue.SetWindowTextW( sValue );

	*pResult = 0;
}


void
DialogProperty::OnReleasedJpegqualityslider( NMHDR* pNMHDR, LRESULT* pResult )
{
	auto iPos = jpegQualitySlider.GetPos();
	wchar_t sValue[ 16 ];
	_itow_s( iPos, sValue, 10 );
	labelJpegQualityValue.SetWindowTextW( sValue );

	*pResult = 0;
}
