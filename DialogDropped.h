#pragma once


// DialogDropped ダイアログ

class DialogDropped : public CDialogEx
{
	DECLARE_DYNAMIC( DialogDropped )

public:
	DialogDropped( CWnd* pParent = nullptr );   // 標準コンストラクター
	virtual ~DialogDropped();

	RECT OrigRect;
	ConvertThread* ConvertWorker;

	// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROP_Dropped };
#endif

protected:
	virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	CProgressCtrl ProgBar;
	CMFCButton ButtonProgressStop;
	virtual BOOL OnInitDialog();
	afx_msg void OnDropFiles( HDROP hDropInfo );
	afx_msg void OnBnClickedStopconvert();
	CStatic ThreadLabel;
};
