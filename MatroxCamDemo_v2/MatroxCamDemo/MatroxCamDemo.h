
// MatroxCamDemo.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CMatroxCamDemoApp: 
// �йش����ʵ�֣������ MatroxCamDemo.cpp
//

class CMatroxCamDemoApp : public CWinApp
{
public:
	CMatroxCamDemoApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CMatroxCamDemoApp theApp;