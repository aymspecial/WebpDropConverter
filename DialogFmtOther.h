#pragma once

#include "XmlIni.h"
//#pragma warning ( disable : 26495 )

struct FmtOtherDecodeParameter
{};

// DialogProperty ダイアログ
class DialogFmtOther : public CDialogEx
{
	DECLARE_DYNAMIC( DialogFmtOther )

public:
	PropertyParameter* pEncParam = nullptr;
	DialogFmtOther( CWnd* pParent = nullptr );   // 標準コンストラクター
	virtual ~DialogFmtOther();

	RECT OrigRect;

	void FlushParameter();
	void EnableControls( bool bWorking );

	// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FMTOTHER };
#endif

protected:
	CSliderCtrl webpQualitySlider;
	CSliderCtrl jpegQualitySlider;
	CStatic labelWebpQualityValue;
	CStatic labelJpegQualityValue;
	CButton radioWbp, radioPNG, radioJPG, radioBMP;

	virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	void getCtrlsValue();
	void setCtrlsValue();
	void getIniParamEnc();
	void writeIniParamEnc();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnReleasedWebpQualitySlider( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void OnReleasedJpegqualitySlider( NMHDR* pNMHDR, LRESULT* pResult );
};
