
// MatroxCamDemoDlg.h : ͷ�ļ�
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

// CMatroxCamDemoDlg �Ի���
class CMatroxCamDemoDlg : public CDialogEx
{
// ����
public:
	CMatroxCamDemoDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MATROXCAMDEMO_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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

	/*�ı䴰�ڴ�С*/
	CRect m_rect;
	control m_control[MAX_SIZE];

	bool IsContinue;

public:
	
	int m_iFrameCount;
	CComboBox m_cbGrabeMode;
	CComboBox m_cbTrigger;

	// ��Ƶ
	double m_dLinePeriod;
	// �ع�ʱ��
	double m_dExposure;

	void Get_control_proportion();
	CComboBox m_cbPreampGain;
	// ģ������
	MIL_DOUBLE m_mdGain;
	MIL_DOUBLE m_mdDigitalGain;

	std::list<control*> m_con_list;
	CStatic m_static_Temperature;
	BOOL m_bIsHigh;  //���ڿ���static������ɫ
	afx_msg void OnBnClickedButtonSofttrigger();
};
