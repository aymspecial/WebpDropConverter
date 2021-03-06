#include "pch.h"
#include "webp_mod.h"

#include "DialogProperty.h"
#include "ConvertThread.h"

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

// コンストラクタ
ConvertThread::ConvertThread( CWnd* _w, int _messageID, std::vector<std::wstring> _droppedFiles, PropertyParameter _encParam ) :
	messageID( _messageID ), parentWindow( _w ), endFlag( false ), propParam( _encParam )
{
	droppedFiles = _droppedFiles;
}

// デストラクタ
ConvertThread::~ConvertThread()
{}

// デバッグ用のメッセージを CWnd* に送る 
void
ConvertThread::stringMessage( const wchar_t* mess )
{
	parentWindow->PostMessage( (UINT)MessageType::MM_STRING, ( WPARAM )nullptr, (LPARAM)mess );
}

void ConvertThread::progressMessage( int it )
{
	parentWindow->PostMessage( (UINT)MessageType::MM_PROGRESS, ( WPARAM )nullptr, (LPARAM)it );
}

void
ConvertThread::PushMessage( enum MessageType _type, void* _data )
{
	MyMessage msg;
	msg.Type = _type;
	msg.Data = _data;

	auto wa = (wchar_t*)_data;

	this->PushMessage( msg );
}

void
ConvertThread::PushMessage( MyMessage msg )
{
	// メッセージキューにメッセージを格納
	{
		std::lock_guard<std::mutex> lock( mtx );
		messageQueue.push( msg );
		is_ready = true;
	}

	// スレッドレジュームの通知
	cv.notify_one();
	//	cv.notify_all();
}

void
ConvertThread::GetMessage( MyMessage* msg )
{
	size_t nMessage;
	{
		// メッセージが何個あるか確認し
		std::lock_guard<std::mutex> lock( mtx );
		nMessage = messageQueue.size();
	}

	if( 0 < nMessage )
	{
		// メッセージがあれば先頭を取り出して先頭を削除する
		std::lock_guard<std::mutex> lock( mtx );
		auto currentMessage = messageQueue.front();
		*msg = currentMessage;
		messageQueue.pop();
	}

	// 次に cv.wait で止める
	is_ready = false;
}

// メインループ
void
ConvertThread::operator()()
{
	endFlag = false;
	SetPriorityClass( GetCurrentProcess(), IDLE_PRIORITY_CLASS );

	MyMessage msg;
	std::wstring wstr;

	// メインループ
	for( int i = 0; endFlag != true && i < droppedFiles.size(); i++ )
	{
		this->GetMessage( &msg );

		switch( msg.Type )
		{
			case MessageType::MM_QUIT:
				endFlag = true;
				continue;

			case MessageType::MM_STRING:
				wstr = L"Message:";
				wstr += (wchar_t*)msg.Data;
				stringMessage( wstr.c_str() );
				continue;

			default:
				break;
		}

		// テキストメッセージの作成
		wchar_t progText[ MAX_PATH ];
		swprintf_s( progText, MAX_PATH, L"%d/%d  \n%s", (int)i, (int)droppedFiles.size(),
					droppedFiles[ i ].c_str() + fileNamePos( droppedFiles[ i ].c_str() ) );
		stringMessage( progText );

		convertFile( droppedFiles[ i ].c_str() );
		progressMessage( i );
	}

	// スレッドループ終了時は -1 を送る
	Sleep( 2000 );
	progressMessage( -1 );
}

int
ConvertThread::fileNamePos( const wchar_t* fullPath )
{
	auto length = wcslen( fullPath );

	for( int i = (int)length - 1; 0 < i; i-- )
	{
		if( fullPath[ i ] == L'\\' )
			return( i + 1 );
	}

	return( 0 );
}

void
ConvertThread::uTF8ToShiftJis( LPSTR bufShiftJis, LPWSTR bufUTF8 )
{
	int ret = WideCharToMultiByte( CP_ACP, 0, bufUTF8, -1, bufShiftJis, MAX_PATH, NULL, NULL );
}

#pragma warning( disable : 6011 6386 )

void
ConvertThread::convertFile( const wchar_t* _sourceFileFullPath )
{
	char sourceFileFullPath[ MAX_PATH ];
	uTF8ToShiftJis( sourceFileFullPath, (LPWSTR)_sourceFileFullPath );
	const char* extention = PathFindExtensionA( sourceFileFullPath );

	char outputFileName[ MAX_PATH ];
	strcpy_s( outputFileName, sourceFileFullPath );

	if( strcmp( extention, ".png" ) == 0 || strcmp( extention, ".jpg" ) == 0 )
	{
		::PathRenameExtensionA( outputFileName, ".webp" );

		// 引数の作成
		std::vector<std::string> vArgs;
		vArgs.push_back( "main" );  // 別になんでもよい

		vArgs.push_back( "-q" );	// 圧縮 quality
		char sQuality[ 64 ] = { 0 };
		_itoa_s( propParam.webpQuality, sQuality, 10 );
		vArgs.push_back( sQuality );

		vArgs.push_back( (LPSTR)sourceFileFullPath ); // 入力ファイル
		vArgs.push_back( "-o" );
		vArgs.push_back( (LPSTR)outputFileName );	// 出力ファイル

		int argc = (int)vArgs.size();
		auto argv = (char**)malloc( sizeof( char* ) * argc );

		for( int i = 0; i < argc; i++ )
		{
			argv[ i ] = (char*)vArgs[ i ].c_str();
		}

		cwebp_main( argc, argv );
	}
	else if( strcmp( extention, ".webp" ) == 0 )
	{
		// 引数の作成
		std::vector<std::string> vArgs;
		vArgs.push_back( "main" );  // 別になんでもよい

		vArgs.push_back( sourceFileFullPath ); // 入力ファイル

		if( strcmp( propParam.format, "JPG" ) == 0 )
		{
			vArgs.push_back( "-jpeg" );	// 出力フォーマット
			::PathRenameExtensionA( outputFileName, ".jpg" );

			vArgs.push_back( "-q" );	// 圧縮 quality
			char sQuality[ 64 ] = { 0 };
			_itoa_s( propParam.jpegQuality, sQuality, 10 );
			vArgs.push_back( sQuality );
		}
		else if( strcmp( propParam.format, "TIF" ) == 0 )
		{
			vArgs.push_back( "-tiff" );	// 出力フォーマット
			::PathRenameExtensionA( outputFileName, ".tiff" );
		}
		else if( strcmp( propParam.format, "BMP" ) == 0 )
		{
			vArgs.push_back( "-bmp" );	// 出力フォーマット
			::PathRenameExtensionA( outputFileName, ".bmp" );
		}
		else
		{
			::PathRenameExtensionA( outputFileName, ".png" );
		}

		vArgs.push_back( "-o" );
		vArgs.push_back( (LPSTR)outputFileName );	// 出力ファイル

		int argc = (int)vArgs.size();
		auto argv = (char**)malloc( sizeof( char* ) * argc );

		for( int i = 0; i < argc; i++ )
		{
			argv[ i ] = (char*)vArgs[ i ].c_str();
		}

		dwebp_main( argc, argv );
	}
	else
	{
		char message[ 64 ] = { 0 };
		sprintf_s( message, 64, "Format(Extention) '%s' is not Supported.", extention );
		MessageBoxA( NULL, message, "Error", MB_ICONERROR );
	}
}