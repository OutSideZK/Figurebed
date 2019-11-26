
// MatroxCamDemoDlg.h : 头文件
//

#pragma once

#include "MatroxCamera.h"
#include "afxwin.h"
#include <list>

#define WM_DISPLAYMESSAGE WM_USER+100
#define MAX_SIZE 100

typedef struct Rect
{
public:
	int Id;
	double scale[4];

	Rect()
	{
		Id = -2;
		scale[0] = 0;
		scale[1] = 0;
		scale[2] = 0;
		scale[3] = 0;
	}

	Rect(const Rect& c)
	{
		*this = c;
	}
}control;

// CMatroxCamDemoDlg 对话框
class CMatroxCamDemoDlg : public CDialogEx
{
// 构造
public:
	CMatroxCamDemoDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MATROXCAMDEMO_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CMcDisplay m_mcDisplay;
	CMvOverlay *m_mvOverlay;
	CMvImage *m_mvImage;

	CMatroxCamera m_mCam;

	afx_msg void OnBnClickedBtnOpen();
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedBtnSignalGrab();
	afx_msg void OnBnClickedBtnCoutinueGrab();
	afx_msg void OnBnClickedBtnStop();
	afx_msg void OnBnClickedBtnSaveimage();
	afx_msg void OnBnClickedButtonReadstatus();
	afx_msg void OnBnClickedButtonSetstatus();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonSetGain();
	afx_msg void OnBnClickedButtonSet();
	afx_msg void OnBnClickedButton2();
	
protected:
	afx_msg LRESULT OnDisplayMessage(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);

private:

	/*改变窗口大小*/
	CRect m_rect;
	control m_control[MAX_SIZE];

	bool IsContinue;

public:
	
	int m_iFrameCount;
	CComboBox m_cbGrabeMode;
	CComboBox m_cbTrigger;

	// 行频
	double m_dLinePeriod;
	// 曝光时间
	double m_dExposure;

	void Get_control_proportion();
	CComboBox m_cbPreampGain;
	// 模拟增益
	MIL_DOUBLE m_mdGain;
	MIL_DOUBLE m_mdDigitalGain;

	std::list<control*> m_con_list;
	CStatic m_static_Temperature;
	BOOL m_bIsHigh;  //用于控制static字体颜色
	afx_msg void OnBnClickedButtonSofttrigger();
};
