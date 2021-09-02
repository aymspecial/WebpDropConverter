#include "pch.h"
#include "XmlIni.h"

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#pragma warning( disable : 6387 6054 )

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

XmlIni::XmlIni( LPWSTR inifileName )
{
	// https://docs.microsoft.com/ja-jp/windows/win32/shell/shlwapi-path
	// パス取得バッファ
	WCHAR lpDocumentPath[ MAX_PATH ];
	WCHAR lpAppFullPath[ MAX_PATH ];

	// 'ドキュメント'のパスを取得
	SHGetSpecialFolderPath( NULL, lpDocumentPath, CSIDL_PERSONAL, 0 );

	// 開始プログラムパスの取得
	GetModuleFileName( NULL, lpAppFullPath, MAX_PATH );
	// 開始プログラム名の取得
	auto lpAppFileName = PathFindFileName( lpAppFullPath );

	// iniファイルのファイル・パス名
	wcscat_s( lpDocumentPath, L"\\" );
	PathRemoveExtension( lpAppFileName );
	wcscat_s( lpDocumentPath, lpAppFileName ); // 新しい保存パスを作る

	wcscpy_s( lpInifileFullPath, lpDocumentPath );
	wcscat_s( lpInifileFullPath, L"\\" );
	wcscat_s( lpInifileFullPath, inifileName );

	makeFullFolder( lpDocumentPath ); // 保存フォルダが無ければ作る

	char lpSjis[ MAX_PATH ];
	uTF8ToShiftJis( lpSjis, lpInifileFullPath );

	doc = new TiXmlDocument( lpSjis );
	bool loadOkay = doc->LoadFile();
	if( loadOkay == false )
	{
		auto decl = new TiXmlDeclaration( "1.0", "", "" );
		doc->LinkEndChild( decl );
	}
}

XmlIni::~XmlIni()
{
	delete doc;
}

// CreateDirectoryEx の使い方がわかんねえ
void
XmlIni::makeFullFolder( LPWSTR path )
{
	WCHAR lpBksPath[ MAX_PATH ]; // 最後に \\ があるパス名
	WCHAR lpIterPath[ MAX_PATH ];

	wcsncpy_s( lpBksPath, MAX_PATH, path, wcslen( path ) );
	wcsncat_s( lpBksPath, MAX_PATH, L"\\", 1 );

	for( size_t i = 0; i < wcslen( lpBksPath ); i++ )
	{
		if( lpBksPath[ i ] == '\\' )
		{
			wcsncpy_s( lpIterPath, lpBksPath, i );
			if( !PathFileExists( lpIterPath ) )
			{
				auto bRet = CreateDirectory( lpIterPath, NULL );
			}
		}
	}
}

void
XmlIni::uTF8ToShiftJis( LPSTR bufShiftJis, LPWSTR bufUTF8 )
{
	int ret = WideCharToMultiByte( CP_ACP, 0, bufUTF8, -1, bufShiftJis, MAX_PATH, NULL, NULL );
}

LPSTR
XmlIni::GetIniString( LPSTR section, LPSTR key, LPSTR defaultValue )
{
	auto sectionElement = doc->FirstChild( section );

	if( sectionElement == nullptr )
		goto NoValue;

	auto valueElement = sectionElement->FirstChildElement( key );

	if( valueElement == nullptr )
		goto NoValue;

	auto lpRet = valueElement->GetText();

	if( lpRet == nullptr )
		goto NoValue;

	return (LPSTR)lpRet;

NoValue:
	return defaultValue;
}

int
XmlIni::GetIniInt( LPSTR section, LPSTR key, int defaultValue )
{
	auto value = GetIniString( section, key, "" );

	if( strlen( value ) == 0 )
		return( defaultValue );

	return atoi( value );
}

void
XmlIni::WriteFlush()
{
	char savepath[ MAX_PATH ];
	uTF8ToShiftJis( savepath, lpInifileFullPath );
	doc->SaveFile( savepath );
}

void
XmlIni::WriteIniString( LPSTR section, LPSTR key, LPSTR _value )
{
	auto sectionElement = doc->FirstChild( section );
	if( sectionElement == nullptr )
	{
		sectionElement = new TiXmlElement( section );
		doc->LinkEndChild( sectionElement );
	}

	auto keyElement = sectionElement->FirstChildElement( key );
	if( keyElement == nullptr )
	{
		keyElement = new TiXmlElement( key );
		auto text = new TiXmlText( _value );
		keyElement->InsertEndChild( *text );
		sectionElement->InsertEndChild( *keyElement );
		delete keyElement;
		delete text;
	}
	else
	{
		keyElement->Clear();
		auto text = new TiXmlText( _value );
		keyElement->InsertEndChild( *text );
		delete text;
	}
}

void
XmlIni::WriteIniInt( LPSTR section, LPSTR key, int _value )
{
	char sValue[ 64 ];
	_itoa_s( _value, sValue, 10 );

	WriteIniString( section, key, sValue );
}
