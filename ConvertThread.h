#pragma once
#include <atomic>
#include <mutex>
#include <queue>

using MutexLock = std::lock_guard<std::mutex>;

enum class MessageType
{
	MM_QUIT = WM_USER,
	MM_STRING,
	MM_PROGRESS,
};

struct MyMessage
{
	MessageType Type;
	void* Data;
};

class ConvertThread
{
	CWnd* parentWindow;
	UINT messageID;
	bool endFlag;
	PropertyParameter* pEncParam;

	std::queue<MyMessage> messageQueue;
	std::mutex mtx;
	std::condition_variable cv;
	bool is_ready = false; // for spurious wakeup
	char messString[ MAX_PATH ] { 0 };

	std::vector<std::wstring> droppedFiles;

	void convertFile( const wchar_t* fileName );
	void uTF8ToShiftJis( LPSTR bufShiftJis, LPWSTR bufUTF8 );

public:
	ConvertThread( CWnd* _w, int _messageID, std::vector<std::wstring> _droppedFiles, PropertyParameter* _pEncParam );
	virtual ~ConvertThread();

	void operator()();

	void PushMessage( MyMessage msg );
	void PushMessage( enum MessageType _type, void* _data );
	void GetMessage( MyMessage* msg );

	void stringMessage( const wchar_t* mess );
	void progressMessage( int it );

	int fileNamePos( const wchar_t* fullPath );
};