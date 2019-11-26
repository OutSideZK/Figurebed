#include "stdafx.h"
#include "PrintLog.h"
#include <atltime.h>

#define LIST_MAX_ROW_COUNT 512

CXListBox* CPrintLog::m_pList;

int CPrintLog::m_iListRow = 0;

CPrintLog::CPrintLog()
{
}


CPrintLog::~CPrintLog()
{
}

void CPrintLog::SetListCtrlHandle(CXListBox* pList)
{
    m_pList = pList;
}

void CPrintLog::AddMsg(CString strMsg, TYP_MSG_CLR clr /*= e_clr_prompt*/)
{
    CXListBox::Color clrTc;
    switch (clr)
    {
    case e_clr_err:
        clrTc = CXListBox::Red;
        break;
    case e_clr_succeed:
        clrTc = CXListBox::Green;
        break;
    case e_clr_prompt:
        clrTc = CXListBox::Blue;
        break;
    case e_clr_para_prompt:
        clrTc = CXListBox::Teal;
        break;
    case e_clr_warning:
        clrTc = CXListBox::Yellow;
        break;
    case e_clr_default:
    default:
        clrTc = CXListBox::Black;
        break;
    }
    CString strLine = _T("");
    CTime time = CTime::GetCurrentTime();
    strLine.Format(_T("%.2d:%.2d:%.2d  %s"), time.GetHour(), time.GetMinute(), time.GetSecond(), strMsg);
    m_pList->AddLine(clrTc, CXListBox::White, strLine);
    m_iListRow++;
    if (m_iListRow > LIST_MAX_ROW_COUNT)
    {
        m_pList->DeleteString(0);
        m_iListRow--;
    }
}
