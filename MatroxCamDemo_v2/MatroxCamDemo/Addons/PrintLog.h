#pragma once
#include "XListBox.h"

typedef enum {
    e_clr_err = 0,
    e_clr_succeed,
    e_clr_prompt,
    e_clr_para_prompt,
    e_clr_warning,
    e_clr_default,
}TYP_MSG_CLR;

class CPrintLog
{
public:
    CPrintLog();
    ~CPrintLog();

public:
    static void SetListCtrlHandle(CXListBox* pList);

    static void AddMsg(CString strMsg, TYP_MSG_CLR clr /*= e_clr_prompt*/);

private:
    static CXListBox* m_pList;
    static int m_iListRow;
};

