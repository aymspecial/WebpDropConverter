#pragma once

#include "XmlIni.h"
//#pragma warning ( disable : 26495 )

struct PropertyParameter
{
	int webpQuality = 80;
	char format[ 16 ] = "PNG";
	int jpegQuality = 80;
};

struct DecodeParameter
{};

// DialogProperty ダイアログ
class DialogProperty : public CDialogEx
{
	DECLARE_DYNAMIC( DialogProperty )

public:
	DialogProperty( CWnd* pParent = nullptr );   // 標準コンストラクター
	virtual ~DialogProperty();

	RECT OrigRect;

	PropertyParameter GetEncodeParameter();

	// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROPERTY };
#endif

protected:
	XmlIni *iniFile;
	CSliderCtrl webpQualitySlider;
	CSliderCtrl jpegQualitySlider;
	CStatic labelWebpQualityValue;
	CStatic labelJpegQualityValue;
	CButton radioPNG, radioJPG, radioBMP, radioTIF;

	PropertyParameter propParam { 0 }, oldPropParam { 0 };

	virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	void getCtrlsValue();
	void setCtrlsValue();
	void getIniParamEnc();
	void writeIniParamEnc();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOK();
	afx_msg void OnPaint();
	afx_msg void OnReleasedWebpQualitySlider( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void OnReleasedJpegqualityslider( NMHDR* pNMHDR, LRESULT* pResult );
};
