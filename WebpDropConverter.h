
// WebpDropConverter.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'pch.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CWebpDropConverterApp:
// このクラスの実装については、WebpDropConverter.cpp を参照してください
//

class CWebpDropConverterApp : public CWinApp
{
public:
	CWebpDropConverterApp();

// オーバーライド
public:
	virtual BOOL InitInstance();

// 実装

	DECLARE_MESSAGE_MAP()
};

extern CWebpDropConverterApp theApp;
