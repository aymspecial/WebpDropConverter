// WebpDropConverterDlg.h : ヘッダー ファイル
//
#pragma once

// CWebpDropConverterDlg ダイアログ
class CWebpDropConverterDlg : public CDialogEx
{
	// コンストラクション
public:
	CWebpDropConverterDlg( CWnd* pParent = nullptr );	// 標準コンストラクター
	virtual ~CWebpDropConverterDlg();

	// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WEBPDROPCONVERTER_DIALOG };
#endif
protected:
	virtual void DoDataExchange( CDataExchange* pDX );	// DDX/DDV サポート

	// 実装
protected:
	HICON m_hIcon;

	XmlIni* appIni = nullptr;
	PropertyParameter* pEncParam = nullptr;
	std::vector<std::wstring> droppedFiles;
	void uTF8ToShiftJis( LPSTR bufShiftJis, LPWSTR bufUTF8 );

	ConvertThread* worker = nullptr;
	std::thread* th = nullptr;

	// 生成された、メッセージ割り当て関数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand( UINT nID, LPARAM lParam );
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTcnSelchangeTab1( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void OnDropFiles( HDROP hDropInfo );
	afx_msg LRESULT OnGetProgressMessage( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnGetStringMessage( WPARAM wParam, LPARAM lParam );
	DECLARE_MESSAGE_MAP()

public:
	CTabCtrl tab;
	DialogDropped DroppedDlg;
	DialogFmtWebp  FmtWebpDlg;
	DialogFmtOther FmtOthrDlg;
	afx_msg void OnMove( int x, int y );
	afx_msg void OnClose();
};
