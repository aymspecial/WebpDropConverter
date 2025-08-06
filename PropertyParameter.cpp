#include "pch.h"
#include "PropertyParameter.h"

PropertyParameter::PropertyParameter()
{
	ini = new XmlIni( L"Convert.xml" );
	GetFromIniFile();
}

PropertyParameter::~PropertyParameter()
{
	delete ini;
}

void PropertyParameter::GetFromIniFile()
{
	Webp2JpegQuality = ini->GetIniInt( "WebpFormt", "JpegQuality", 80 );
	strcpy_s( Webp2Format, ini->GetIniString( "WebpFormt", "Encode", "PNG" ) );
	Othr2WebpQuality = ini->GetIniInt( "OtherFormat", "WebpQuality", 80 );
	Othr2JpegQuality = ini->GetIniInt( "OtherFormat", "JpegQuality", 80 );
	strcpy_s( Othr2Format, ini->GetIniString( "OtherFormat", "Encode", "PNG" ) );
	bTrashSource = ini->GetIniInt( "Common", "TrashSource", 0 );
}

void
PropertyParameter::WriteIniFile()
{
	ini->SetIniInt( "WebpFormt", "JpegQuality", Webp2JpegQuality );
	ini->SetIniString( "WebpFormt", "Encode", Webp2Format );
	ini->SetIniInt( "OtherFormat", "WebpQuality", Othr2WebpQuality );
	ini->SetIniInt( "OtherFormat", "JpegQuality", Othr2JpegQuality );
	ini->SetIniString( "OtherFormat", "Encode", Othr2Format );
	ini->SetIniInt( "Common", "TrashSource", bTrashSource ? 1 : 0 );

	ini->WriteFlush();
}