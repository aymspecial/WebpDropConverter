#pragma once

#include <tinyxml.h>
#pragma comment(lib, "tinyxml.lib")

class XmlIni
{
private:
	XmlIni() {}; // à¯êîñ≥ÇµÇ≈ÇÕégÇ¶Ç»Ç¢
	WCHAR lpInifileFullPath[ MAX_PATH ] = L"";
	TiXmlDocument *doc;

	void makeFullFolder( LPWSTR path );

public:
	XmlIni( LPWSTR inifileName );
	~XmlIni();

	// Write Value
	void WriteIniString( LPSTR section, LPSTR key, LPSTR _value );
	void WriteIniInt( LPSTR section, LPSTR key, int _value );

	// Get Value
	LPSTR	GetIniString( LPSTR section, LPSTR key, LPSTR defaultValue );
	int		GetIniInt( LPSTR section, LPSTR key, int defaultValue );

	void WriteFlush();

	void uTF8ToShiftJis( LPSTR bufShiftJis, LPWSTR bufUTF8 );
};
