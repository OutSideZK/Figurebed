
// MatroxCamDemoDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MatroxCamDemo.h"
#include "MatroxCamDemoDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMatroxCamDemoDlg �Ի���


CMatroxCamDemoDlg::CMatroxCamDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MATROXCAMDEMO_DIALOG, pParent)
	, m_iFrameCount(10)
	, m_dLinePeriod(0.0)
	, m_dExposure(0.0)
	, m_bIsHigh(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMatroxCamDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PICTURE, m_mcDisplay);
	DDX_Text(pDX, IDC_EDIT_FRAMECONNUT, m_iFrameCount);
	DDX_Control(pDX, IDC_COMBO_GRABMODE, m_cbGrabeMode);
	DDX_Control(pDX, IDC_COMBO_TRIGGER, m_cbTrigger);
	DDX_Text(pDX, IDC_EDIT_LINEPERIOD, m_dLinePeriod);
	DDX_Text(pDX, IDC_EDIT_EXPOSURE, m_dExposure);
	DDX_Control(pDX, IDC_COMBO_PREAMPGAIN, m_cbPreampGain);
	DDX_Text(pDX, IDC_EDIT_GAIN, m_mdGain);
	DDX_Text(pDX, IDC_EDIT_DIGITALGAIN, m_mdDigitalGain);
	DDX_Control(pDX, IDC_STATIC_TEMPERATURE, m_static_Temperature);
}

BEGIN_MESSAGE_MAP(CMatroxCamDemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_OPEN, &CMatroxCamDemoDlg::OnBnClickedBtnOpen)
	ON_BN_CLICKED(IDC_BTN_CLOSE, &CMatroxCamDemoDlg::OnBnClickedBtnClose)
	ON_BN_CLICKED(IDC_BTN_SIGNAL_GRAB, &CMatroxCamDemoDlg::OnBnClickedBtnSignalGrab)
	ON_BN_CLICKED(IDC_BTN_COUTINUE_GRAB, &CMatroxCamDemoDlg::OnBnClickedBtnCoutinueGrab)
	ON_BN_CLICKED(IDC_BTN_STOP, &CMatroxCamDemoDlg::OnBnClickedBtnStop)
	ON_BN_CLICKED(IDC_BTN_SAVEIMAGE, &CMatroxCamDemoDlg::OnBnClickedBtnSaveimage)
	ON_MESSAGE(WM_DISPLAYMESSAGE, &CMatroxCamDemoDlg::OnDisplayMessage)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_SET, &CMatroxCamDemoDlg::OnBnClickedButtonSet)
	ON_BN_CLICKED(IDC_BUTTON2, &CMatroxCamDemoDlg::OnBnClickedButton2)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_SET_GAIN, &CMatroxCamDemoDlg::OnBnClickedButtonSetGain)
	ON_BN_CLICKED(IDC_BUTTON_READSTATUS, &CMatroxCamDemoDlg::OnBnClickedButtonReadstatus)
	ON_BN_CLICKED(IDC_BUTTON_SETSTATUS, &CMatroxCamDemoDlg::OnBnClickedButtonSetstatus)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_SOFTTRIGGER, &CMatroxCamDemoDlg::OnBnClickedButtonSofttrigger)
END_MESSAGE_MAP()


// CMatroxCamDemoDlg ��Ϣ�������

BOOL CMatroxCamDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_mvOverlay = m_mcDisplay.CreateOverlay();
	m_mcDisplay.FitSize();

	//��ʼ��combo Box
	m_cbGrabeMode.AddString(_T("�����ɼ�"));
	m_cbGrabeMode.AddString(_T("�ⴥ��"));
	m_cbGrabeMode.SetCurSel(0);

	m_cbTrigger.AddString(_T("Line0"));
	m_cbTrigger.AddString(_T("Line1"));
	m_cbTrigger.AddString(_T("Line2"));
	m_cbTrigger.AddString(_T("Line3"));
	m_cbTrigger.SetCurSel(1);

	m_cbPreampGain.AddString(_T("X1"));
	m_cbPreampGain.AddString(_T("X2"));
	m_cbPreampGain.AddString(_T("X4"));
	m_cbPreampGain.SetCurSel(0);

	//�����ʱ��ֻ�д������ť����
	GetDlgItem(IDC_BTN_CLOSE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_COUTINUE_GRAB)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_SIGNAL_GRAB)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_BOARDINDEX)->SetWindowText(_T("0"));
	GetDlgItem(IDC_STATIC_TEMPERATURE)->SetWindowText(_T("0��"));

	Get_control_proportion();

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CMatroxCamDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMatroxCamDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CMatroxCamDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMatroxCamDemoDlg::OnBnClickedBtnOpen()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_mCam.InitCam(GetDlgItemInt(IDC_EDIT_BOARDINDEX)))
	{
		//MessageBox(_T("�򿪳ɹ�"), _T("��ʾ"), MB_OK);
		GetDlgItem(IDC_BTN_CLOSE)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_COUTINUE_GRAB)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_SIGNAL_GRAB)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_OPEN)->EnableWindow(FALSE);

		m_mCam.SetParam();
		//m_mCam.SpecifyIOState();

		m_mCam.GetLinePeriod(m_dLinePeriod);
		m_mCam.GetExposureTime(m_dExposure);
		m_mCam.GetGain(m_mdGain);
		m_mCam.GetDigitalGain(m_mdDigitalGain);

		//SetTimer(2, 1000, NULL);

		UpdateData(false);
	}
}


void CMatroxCamDemoDlg::OnBnClickedBtnClose()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//KillTimer(2);
	m_mCam.CloseCam();
	GetDlgItem(IDC_BTN_CLOSE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_COUTINUE_GRAB)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_SIGNAL_GRAB)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_OPEN)->EnableWindow(TRUE);
}


void CMatroxCamDemoDlg::OnBnClickedBtnSignalGrab()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_mCam.GrabSignalImage(GetDlgItemInt(IDC_EDIT_FRAMECONNUT));
	IsContinue = false;
}


void CMatroxCamDemoDlg::OnBnClickedBtnCoutinueGrab()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int nIndex = m_cbGrabeMode.GetCurSel();
	CString strCBText;
	m_cbGrabeMode.GetLBText(nIndex, strCBText);

	UpdateData(true);

	m_mCam.SetFrameCount(m_iFrameCount);

	if (_T("�����ɼ�") == strCBText)
	{
		m_mCam.IsTrigger = FALSE;
		m_mCam.SetTriggerMode();
		m_mCam.GrabImageContinue();
		IsContinue = true;
		GetDlgItem(IDC_BTN_COUTINUE_GRAB)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_STOP)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_CLOSE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_SIGNAL_GRAB)->EnableWindow(FALSE);
	}
	else if (_T("�ⴥ��") == strCBText)
	{
		int nIndex = m_cbTrigger.GetCurSel();
		m_mCam.IsTrigger = TRUE;

		m_mCam.SetTriggerMode(nIndex);
		m_mCam.GrabImageContinue();
		IsContinue = true;
		GetDlgItem(IDC_BTN_COUTINUE_GRAB)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_STOP)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_CLOSE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_SIGNAL_GRAB)->EnableWindow(FALSE);
	}
	//SetTimer(1, 3000, NULL);
}


void CMatroxCamDemoDlg::OnBnClickedBtnStop()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	m_mCam.StopGrabImage();

	GetDlgItem(IDC_BTN_COUTINUE_GRAB)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_CLOSE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_SIGNAL_GRAB)->EnableWindow(TRUE);

	//KillTimer(1);
	
}


void CMatroxCamDemoDlg::OnBnClickedBtnSaveimage()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (NULL == m_mvImage)
	{
		MessageBox(_T("û��ͼƬ"), _T("����"));
		return;
	}

	CString strImageName;
	SYSTEMTIME sysTime;
	CString strImagePath;

	GetLocalTime(&sysTime);
	strImageName.Format(_T("%04d%02d%02d%02d%2d%2d"), sysTime.wYear, sysTime.wMonth, sysTime.wDay,
		sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

	strImagePath.Format(_T("D:\\CXP\\%s.bmp"), strImageName);

	m_mvImage->SaveFile(strImagePath);
}


afx_msg LRESULT CMatroxCamDemoDlg::OnDisplayMessage(WPARAM wParam, LPARAM lParam)
{
	CMvImage* mvImage = (CMvImage*)wParam;
	m_mcDisplay.SetImage(mvImage);

	m_mvOverlay->Clear();
	m_mcDisplay.Redraw();

	CString strImageName;
	SYSTEMTIME sysTime;
	CString strImagePath;

	GetLocalTime(&sysTime);
	strImageName.Format(_T("%04d%02d%02d%02d%2d%2d"), sysTime.wYear, sysTime.wMonth, sysTime.wDay,
		sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

	strImagePath.Format(_T("D:\\CXP\\%s.bmp"), strImageName);

	mvImage->SaveFile(strImagePath);

	//m_mvImage = mvImage;

	return 0;
}

void CMatroxCamDemoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������

	if (1 == nType)
	{
		return;
	}
	else
	{
		CRect rect;
		for (std::list<control*>::iterator it = m_con_list.begin(); it != m_con_list.end(); it++)
		{
			CWnd* pwnd = GetDlgItem((*it)->Id);
			pwnd->GetWindowRect(&rect);
			ScreenToClient(&rect);
			rect.left = (*it)->scale[0] * cx;
			rect.right = (*it)->scale[1] * cx;
			rect.top = (*it)->scale[2] * cy;
			rect.bottom = (*it)->scale[3] * cy;
			pwnd->MoveWindow(rect);
		}
	}
	GetClientRect(&m_rect);
}


void CMatroxCamDemoDlg::OnBnClickedButtonSet()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
	m_mCam.SetLinePeriod(m_dLinePeriod);
	
	m_mCam.SetExposureTime(m_dExposure);
}


void CMatroxCamDemoDlg::OnBnClickedButton2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_mCam.SaveCameraParam();
}


void CMatroxCamDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	switch (nIDEvent)
	{
	case 1:
	{
		srand((unsigned)time(NULL));

		m_dLinePeriod = rand() % 300 + 10;
		m_dExposure = m_dLinePeriod - 3;

		m_mCam.SetLinePeriod(m_dLinePeriod);
		m_mCam.SetExposureTime(m_dExposure);
		UpdateData(false);
		break;
	}
	case 2:
		double dTemp = m_mCam.GetCameraTemperature();
		if (dTemp >= 55.00)
		{
			m_bIsHigh = TRUE;
		}
		else
		{
			m_bIsHigh = FALSE;
		}
		CString strTemp;
		strTemp.Format(_T("%.2f��"), dTemp);
		GetDlgItem(IDC_STATIC_TEMPERATURE)->SetWindowText(strTemp);
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CMatroxCamDemoDlg::OnBnClickedButtonSetGain()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	UpdateData(true);

	INT gain;
	int index = m_cbPreampGain.GetCurSel();
	
	switch (index)
	{
	case 0:
		gain = 1;
		break;
	case 1:
		gain = 2;
		break;
	case 2:
		gain = 4;
		break;
	default:
		break;
	}
	m_mCam.SetPreampGain(gain);

	m_mCam.SetGain(m_mdGain);
	m_mCam.SetDigitalGain(m_mdDigitalGain);
}

void CMatroxCamDemoDlg::Get_control_proportion()
{
	HWND hwndChild = ::GetWindow(m_hWnd, GW_CHILD);
	while (hwndChild)
	{
		CRect rect;
		control* tempcon = new control;
		CWnd* pWnd = GetDlgItem(::GetDlgCtrlID(hwndChild));
		pWnd->GetWindowRect(&rect);
		ScreenToClient(&rect);
		tempcon->Id = ::GetDlgCtrlID(hwndChild);
		tempcon->scale[0] = (double)rect.left / m_rect.Width();
		tempcon->scale[1] = (double)rect.right / m_rect.Width();
		tempcon->scale[2] = (double)rect.top / m_rect.Height();
		tempcon->scale[3] = (double)rect.bottom / m_rect.Height();
		m_con_list.push_back(tempcon);
		hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
	}
}

void CMatroxCamDemoDlg::OnBnClickedButtonReadstatus()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	MIL_INT status = m_mCam.GetIOState(GetDlgItemInt(IDC_EDIT_IOSOURCE));

	if (M_INVALID == status)
	{
		GetDlgItem(IDC_STATIC_STATUS)->SetWindowText(_T("INVALID"));
	}
	else if (M_OFF == status)
	{
		GetDlgItem(IDC_STATIC_STATUS)->SetWindowText(_T("OFF"));
	}
	else if (M_ON == status)
	{
		GetDlgItem(IDC_STATIC_STATUS)->SetWindowText(_T("ON"));
	}
	else if (M_UNKNOWN == status)
	{
		GetDlgItem(IDC_STATIC_STATUS)->SetWindowText(_T("UNKNOWN"));
	}
}


void CMatroxCamDemoDlg::OnBnClickedButtonSetstatus()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	INT itemp = GetDlgItemInt(IDC_EDIT_STATE);

	if (0 == itemp)
	{
		m_mCam.SetOutputState(GetDlgItemInt(IDC_EDIT_OUTPUT_SOURCE), M_OFF);
	}
	else if (1 == itemp)
	{
		m_mCam.SetOutputState(GetDlgItemInt(IDC_EDIT_OUTPUT_SOURCE), M_ON);
	}
}


HBRUSH CMatroxCamDemoDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����

	if (IDC_STATIC_TEMPERATURE == pWnd->GetDlgCtrlID())
	{
		if (m_bIsHigh)
		{
			pDC->SetTextColor(RGB(255, 0, 0));
		}
		else
		{
			pDC->SetTextColor(RGB(0, 0, 0));
		}
	}

	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}



void CMatroxCamDemoDlg::OnBnClickedButtonSofttrigger()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//AfxMessageBox(_T("11"));
	m_mCam.SoftTrigger();
}
