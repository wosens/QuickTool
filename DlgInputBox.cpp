// InputBox.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "DlgInputBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInputBox dialog


CInputBox::CInputBox(CString ATitle,CString ACaption,CString* AInputData,CWnd* pParent /*=NULL*/)
	: CDialog(CInputBox::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInputBox)
	m_sInputData = _T("");
	//}}AFX_DATA_INIT

    m_sTitle    =ATitle;
    m_sCaption  =ACaption;
    m_pInputData=AInputData;

    m_sInputData=*m_pInputData;

    m_bOK   =FALSE;
}


void CInputBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInputBox)
	DDX_Text(pDX, IDC_EDT_INPUT_DATA_INP, m_sInputData);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInputBox, CDialog)
	//{{AFX_MSG_MAP(CInputBox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInputBox message handlers

BOOL CInputBox::OnInitDialog() 
{
	CDialog::OnInitDialog();

    SetWindowText(m_sCaption);
    GetDlgItem(IDC_LBL_TITLE_INP)->SetWindowText(m_sTitle);
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInputBox::OnOK() 
{
	m_bOK=TRUE;

    UpdateData(TRUE);
    *m_pInputData=m_sInputData;
	
	CDialog::OnOK();
}

void CInputBox::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}
