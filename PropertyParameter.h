#pragma once

#include "XmlIni.h"

struct PropertyParameter
{
	int Othr2WebpQuality = 80;
	char Webp2Format[ 16 ] = "PNG";
	char Othr2Format[ 16 ] = "PNG";
	int Webp2JpegQuality = 80;
	int Othr2JpegQuality = 80;

	XmlIni* ini = nullptr;

	PropertyParameter();
	~PropertyParameter();
	void GetFromIniFile();
	void WriteIniFile();
};