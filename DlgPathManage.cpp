// DlgPathManage.cpp : ¨º¦Ì?????t
//

#include "stdafx.h"
#include "QuickTool.h"
#include "DlgPathManage.h"
#include "afxdialogex.h"


// CDlgPathManage ???¡ã?¨°

IMPLEMENT_DYNAMIC(CDlgPathManage, CDialogEx)

CDlgPathManage::CDlgPathManage(CStringArray* a_pPath, CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLG_PATH_MANAGE, pParent)
{
    m_pPath=a_pPath;

}

CDlgPathManage::~CDlgPathManage()
{
}

void CDlgPathManage::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX,IDC_LST_PATH,m_lstPath);
}


BEGIN_MESSAGE_MAP(CDlgPathManage, CDialogEx)
    ON_BN_CLICKED(IDOK, &CDlgPathManage::OnBnClickedOk)
    ON_BN_CLICKED(IDC_BTN_DEL, &CDlgPathManage::OnBnClickedBtnDel)
END_MESSAGE_MAP()


// CDlgPathManage ???¡é¡ä|¨¤¨ª3¨¬D¨°
void CDlgPathManage::OnBnClickedOk()
{
    m_pPath->RemoveAll();
    for(int i=0;i<m_lstPath.GetCount();i++){
        CString sText;
        m_lstPath.GetText(i,sText);
        m_pPath->Add(sText);
    }

    CDialogEx::OnOK();
}


void CDlgPathManage::OnBnClickedBtnDel()
{
    if(m_lstPath.GetSelCount()>0){
        if(AfxMessageBox(_T("ÕæµÄÒªÉ¾³ýÂð£¿"),MB_ICONQUESTION|MB_YESNO)==IDYES){
            for(int i=m_lstPath.GetCount()-1;i>=0;i--){
                if(m_lstPath.GetSel(i)){
                    m_lstPath.DeleteString(i);
                }
            }
        }
    }
}

BOOL CDlgPathManage::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_lstPath.ResetContent();
    for(int i=0;i<m_pPath->GetSize();i++){
        m_lstPath.AddString(m_pPath->GetAt(i));
    }
    ::SendMessage(m_lstPath.GetSafeHwnd(),LB_SETHORIZONTALEXTENT,255,0);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // ¨°¨¬3¡ê: OCX ¨º?D?¨°3¨®|¡¤¦Ì?? FALSE
}
