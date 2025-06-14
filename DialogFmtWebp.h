#pragma once

#include "XmlIni.h"
//#pragma warning ( disable : 26495 )

// DialogProperty ダイアログ
class DialogFmtWebp : public CDialogEx
{
	DECLARE_DYNAMIC( DialogFmtWebp )

public:
	DialogFmtWebp( CWnd* pParent, PropertyParameter* _pEncParam );   // 標準コンストラクター
	virtual ~DialogFmtWebp();

	RECT OrigRect;

	PropertyParameter* pEncParam = nullptr;
	void FlushParameter();
	void EnableControls( bool bWorking );

	// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FMTWEBP };
#endif

protected:
	CSliderCtrl jpegQualitySlider;
	CStatic labelWebpQualityValue;
	CStatic labelJpegQualityValue;
	CButton radioPNG, radioJPG, radioBMP, radioTIF;



	virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	void getCtrlsValue();
	void setCtrlsValue();
	void getIniParamEnc();
	void writeIniParamEnc();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnReleasedJpegqualitySlider( NMHDR* pNMHDR, LRESULT* pResult );

	DialogFmtWebp() = default;
};
