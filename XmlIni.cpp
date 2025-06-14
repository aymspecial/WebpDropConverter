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

using namespace tinyxml2;

// UTF8 から SJIS へ変換
void
UTF8ToShiftJis( LPSTR bufShiftJis, LPCWSTR bufUTF8 )
{
	int ret = ::WideCharToMultiByte( CP_ACP, 0, bufUTF8, -1, bufShiftJis, MAX_PATH, NULL, NULL );
}

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
	UTF8ToShiftJis( lpSjis, lpInifileFullPath );

	doc = new tinyxml2::XMLDocument( lpSjis );
	bool loadOkay = doc->LoadFile( lpSjis );
	if( loadOkay == false )
	{
		//auto decl = new XMLDeclaration( doc );
		//doc->LinkEndChild( decl );
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

LPSTR
XmlIni::GetIniString( LPSTR section, LPSTR key, LPSTR defaultValue )
{
	auto sectionElement = doc->FirstChildElement( section );

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
XmlIni::ClearDoc()
{
	doc->Clear();
}

void
XmlIni::WriteFlush()
{
	char savepath[ MAX_PATH ];
	UTF8ToShiftJis( savepath, lpInifileFullPath );
	doc->SaveFile( savepath );
}

void
XmlIni::SetIniString( LPSTR section, LPSTR key, LPSTR _value )
{
	auto sectionElement = doc->FirstChildElement( section );
	if( sectionElement == nullptr )
	{
		sectionElement = doc->NewElement( section );
		doc->LinkEndChild( sectionElement );
	}

	auto keyElement = sectionElement->FirstChildElement( key );
	if( keyElement == nullptr )
	{
		keyElement = doc->NewElement( key );
		sectionElement->InsertEndChild( keyElement );
	}

	keyElement->SetText( _value );
}

void
XmlIni::SetIniString( LPSTR section, LPSTR key, LPCWSTR _value )
{
	CHAR cValue[ MAX_PATH ];
	UTF8ToShiftJis( cValue, _value );
	this->SetIniString( section, key, cValue );
}

void
XmlIni::SetIniInt( LPSTR section, LPSTR key, int _value )
{
	char sValue[ 64 ];
	_itoa_s( _value, sValue, 10 );

	this->SetIniString( section, key, sValue );
}
