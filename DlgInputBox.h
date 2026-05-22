#if !defined(AFX_INPUTBOX_H__88BC0BB6_F42B_4A3A_AF16_840F06FABDDF__INCLUDED_)
#define AFX_INPUTBOX_H__88BC0BB6_F42B_4A3A_AF16_840F06FABDDF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InputBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInputBox dialog

class CInputBox : public CDialog
{
private:
    BOOL        m_bOK;
    CString     m_sCaption;
    CString     m_sTitle;
    CString*    m_pInputData;
public:
	CInputBox(CString ATitle,CString ACaption,CString* AInputData,CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CInputBox)
	enum { IDD = IDD_DLG_INPUT_BOX };
	CString	m_sInputData;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInputBox)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInputBox)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INPUTBOX_H__88BC0BB6_F42B_4A3A_AF16_840F06FABDDF__INCLUDED_)
