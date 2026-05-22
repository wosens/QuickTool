#pragma once
#include "afxwin.h"


// CDlgPathManage 对话框

class CDlgPathManage : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgPathManage)

public:
	CDlgPathManage(CStringArray* a_pPath,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgPathManage();

    CStringArray*    m_pPath;


// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_PATH_MANAGE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedBtnDel();
    virtual BOOL OnInitDialog();
    CListBox m_lstPath;
};
