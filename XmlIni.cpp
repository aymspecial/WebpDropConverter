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
	// �p�X�擾�o�b�t�@
	WCHAR lpDocumentPath[ MAX_PATH ];
	WCHAR lpAppFullPath[ MAX_PATH ];

	// '�h�L�������g'�̃p�X���擾
	SHGetSpecialFolderPath( NULL, lpDocumentPath, CSIDL_PERSONAL, 0 );

	// �J�n�v���O�����p�X�̎擾
	GetModuleFileName( NULL, lpAppFullPath, MAX_PATH );
	// �J�n�v���O�������̎擾
	auto lpAppFileName = PathFindFileName( lpAppFullPath );

	// ini�t�@�C���̃t�@�C���E�p�X��
	wcscat_s( lpDocumentPath, L"\\" );
	PathRemoveExtension( lpAppFileName );
	wcscat_s( lpDocumentPath, lpAppFileName ); // �V�����ۑ��p�X�����

	wcscpy_s( lpInifileFullPath, lpDocumentPath );
	wcscat_s( lpInifileFullPath, L"\\" );
	wcscat_s( lpInifileFullPath, inifileName );

	makeFullFolder( lpDocumentPath ); // �ۑ��t�H���_��������΍��

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

// CreateDirectoryEx �̎g�������킩��˂�
void
XmlIni::makeFullFolder( LPWSTR path )
{
	WCHAR lpBksPath[ MAX_PATH ]; // �Ō�� \\ ������p�X��
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
