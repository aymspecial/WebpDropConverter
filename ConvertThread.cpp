#include "pch.h"
#include "webp_mod.h"

#include "PropertyParameter.h"
#include "DialogFmtWebp.h"
#include "ConvertThread.h"

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

// �R���X�g���N�^
ConvertThread::ConvertThread( CWnd* _w, int _messageID, std::vector<std::wstring> _droppedFiles, PropertyParameter* _pEncParam ) :
	messageID( _messageID ), parentWindow( _w ), endFlag( false ), pEncParam( _pEncParam )
{
	droppedFiles = _droppedFiles;
}

// �f�X�g���N�^
ConvertThread::~ConvertThread()
{}

// �f�o�b�O�p�̃��b�Z�[�W�� CWnd* �ɑ��� 
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
	// ���b�Z�[�W�L���[�Ƀ��b�Z�[�W���i�[
	{
		std::lock_guard<std::mutex> lock( mtx );
		messageQueue.push( msg );
		is_ready = true;
	}

	// �X���b�h���W���[���̒ʒm
	cv.notify_one();
	//	cv.notify_all();
}

void
ConvertThread::GetMessage( MyMessage* msg )
{
	size_t nMessage;
	{
		// ���b�Z�[�W�������邩�m�F��
		std::lock_guard<std::mutex> lock( mtx );
		nMessage = messageQueue.size();
	}

	if( 0 < nMessage )
	{
		// ���b�Z�[�W������ΐ擪�����o���Đ擪���폜����
		std::lock_guard<std::mutex> lock( mtx );
		auto currentMessage = messageQueue.front();
		*msg = currentMessage;
		messageQueue.pop();
	}

	// ���� cv.wait �Ŏ~�߂�
	is_ready = false;
}

// ���C�����[�v
void
ConvertThread::operator()()
{
	endFlag = false;
	SetPriorityClass( GetCurrentProcess(), IDLE_PRIORITY_CLASS );

	MyMessage msg;
	std::wstring wstr;

	// ���C�����[�v
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

		// �e�L�X�g���b�Z�[�W�̍쐬
		wchar_t progText[ MAX_PATH ];
		swprintf_s( progText, MAX_PATH, L"%d/%d  \n%s", (int)i, (int)droppedFiles.size(),
					droppedFiles[ i ].c_str() + fileNamePos( droppedFiles[ i ].c_str() ) );
		stringMessage( progText );

		convertFile( droppedFiles[ i ].c_str() );
		progressMessage( i );
	}

	// �X���b�h���[�v�I������ -1 �𑗂�
	Sleep( 1000 );
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


void MoveToRecycleBin( const wchar_t* filePath )
{
	// �t�@�C���p�X�̒������擾

	
	size_t len = wcsnlen_s( filePath, MAX_PATH );
	LPWSTR p = (LPWSTR)malloc( ( len + 2 ) * sizeof( WCHAR ) );
	wcscpy_s( p, len + 2, filePath );
	p[ len + 1 ] = 0;

	SHFILEOPSTRUCT fileOp = { 0 };
	fileOp.wFunc = FO_DELETE; // �폜����
	fileOp.pFrom = p;  // �t�@�C���p�X
	fileOp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;                     // �v���O���X��\��;

	auto rc = SHFileOperation( &fileOp ); // ���s
	// �����������H
	if( !rc )
	{
		;
	}
	else
	{
		TRACE( "rc:%d\n", rc );
	}
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

	int return_value = 1;

	if( !strcmp( extention, ".png" ) || !strcmp( extention, ".jpg" ) || !strcmp( extention, ".bmp" ) )
	{
		if( !strcmp( pEncParam->Othr2Format, "WEBP" ) )
		{
			::PathRenameExtensionA( outputFileName, ".webp" );

			// �����̍쐬
			std::vector<std::string> vArgs;
			vArgs.push_back( "main" );  // �ʂɂȂ�ł��悢

			vArgs.push_back( "-q" );	// ���k quality
			char sQuality[ 64 ] = { 0 };
			_itoa_s( pEncParam->Othr2WebpQuality, sQuality, 10 );
			vArgs.push_back( sQuality );

			vArgs.push_back( (LPSTR)sourceFileFullPath ); // ���̓t�@�C��
			vArgs.push_back( "-o" );
			vArgs.push_back( (LPSTR)outputFileName );	// �o�̓t�@�C��

			int argc = (int)vArgs.size();
			auto argv = (char**)malloc( sizeof( char* ) * argc );

			for( int i = 0; i < argc; i++ )
			{
				argv[ i ] = (char*)vArgs[ i ].c_str();
			}
			cwebp_main( argc, argv );

			free( argv );
		}
		else
		{
			if( !strcmp( pEncParam->Othr2Format, "PNG" ) )
			{
				::PathRenameExtensionA( outputFileName, ".png" );
			}
			else if( !strcmp( pEncParam->Othr2Format, "JPG" ) )
			{
				::PathRenameExtensionA( outputFileName, ".jpg" );
			}
			else if( !strcmp( pEncParam->Othr2Format, "BMP" ) )
			{
				::PathRenameExtensionA( outputFileName, ".bmp" );
			}

			// �����̍쐬
			std::vector<std::string> vArgs;
			vArgs.push_back( "main" );  // �ʂɂȂ�ł��悢

			vArgs.push_back( sourceFileFullPath ); // ���̓t�@�C��

			if( strcmp( pEncParam->Othr2Format, "JPG" ) == 0 )
			{
				vArgs.push_back( "-jpeg" );	// �o�̓t�H�[�}�b�g
				::PathRenameExtensionA( outputFileName, ".jpg" );

				vArgs.push_back( "-q" );	// ���k quality
				char sQuality[ 64 ] = { 0 };
				_itoa_s( pEncParam->Webp2JpegQuality, sQuality, 10 );
				vArgs.push_back( sQuality );
			}
			else if( strcmp( pEncParam->Othr2Format, "BMP" ) == 0 )
			{
				vArgs.push_back( "-bmp" );	// �o�̓t�H�[�}�b�g
				::PathRenameExtensionA( outputFileName, ".bmp" );
			}
			else
			{
				vArgs.push_back( "-png" );	// �o�̓t�H�[�}�b�g
				::PathRenameExtensionA( outputFileName, ".png" );
			}

			vArgs.push_back( "-o" );
			vArgs.push_back( outputFileName );

			int argc = (int)vArgs.size();
			auto argv = (char**)malloc( sizeof( char* ) * argc );
			for( int i = 0; i < argc; i++ )
			{
				argv[ i ] = (char*)vArgs[ i ].c_str();
			}
			auto ret = cmodmod_main( argc, argv );

			free( argv );
		}
	}
	else if( strcmp( extention, ".webp" ) == 0 )
	{
		// �����̍쐬
		std::vector<std::string> vArgs;
		vArgs.push_back( "main" );  // �ʂɂȂ�ł��悢

		vArgs.push_back( sourceFileFullPath ); // ���̓t�@�C��

		if( strcmp( pEncParam->Webp2Format, "JPG" ) == 0 )
		{
			vArgs.push_back( "-jpeg" );	// �o�̓t�H�[�}�b�g
			::PathRenameExtensionA( outputFileName, ".jpg" );

			vArgs.push_back( "-q" );	// ���k quality
			char sQuality[ 64 ] = { 0 };
			_itoa_s( pEncParam->Webp2JpegQuality, sQuality, 10 );
			vArgs.push_back( sQuality );
		}
		else if( strcmp( pEncParam->Webp2Format, "BMP" ) == 0 )
		{
			vArgs.push_back( "-bmp" );	// �o�̓t�H�[�}�b�g
			::PathRenameExtensionA( outputFileName, ".bmp" );
		}
		else
		{
			::PathRenameExtensionA( outputFileName, ".png" );
		}

		vArgs.push_back( "-o" );
		vArgs.push_back( (LPSTR)outputFileName );	// �o�̓t�@�C��

		int argc = (int)vArgs.size();
		auto argv = (char**)malloc( sizeof( char* ) * argc );

		for( int i = 0; i < argc; i++ )
		{
			argv[ i ] = (char*)vArgs[ i ].c_str();
		}

		return_value = dwebp_main( argc, argv );

		free( argv );
	}
	else
	{
		char message[ 64 ] = { 0 };
		sprintf_s( message, 64, "Format(Extention) '%s' is not Supported.", extention );
		MessageBoxA( NULL, message, "Error", MB_ICONERROR );
	}

	if( return_value == 0 )
	{
		// ���̃t�@�C�����S�~���ɓ����
		MoveToRecycleBin( _sourceFileFullPath );
	}


}