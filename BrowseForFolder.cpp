#include "stdafx.h"
#include "BrowseForFolder.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
//
// Construction/Destruction
//

CBrowseForFolder::CBrowseForFolder(const HWND a_hParent, LPCTSTR a_lpszTitle, LPCTSTR a_lpszPath)
{
	ZeroMemory(&m_bi, sizeof(m_bi));
	m_szSelected[0] = _T('\0');
	m_hwnd = NULL;
	SetOwner(a_hParent);
	SetRoot(NULL);
	SetTitle(a_lpszTitle);
	if (a_lpszPath != NULL)
	{
		m_strPath = a_lpszPath;
	}
	m_bi.lpfn = BrowseCallbackProc;
	m_bi.lParam = (DWORD_PTR)this;
	m_bi.pszDisplayName = m_szSelected;
	m_bi.ulFlags=BIF_RETURNONLYFSDIRS;
}

CBrowseForFolder::~CBrowseForFolder()
{

}

//////////////////////////////////////////////////////////////////////
//
// Implementation
//

void CBrowseForFolder::SetOwner(const HWND hwndOwner)
{
	if (m_hwnd != NULL)
		return;

	m_bi.hwndOwner = hwndOwner;
}

void CBrowseForFolder::SetRoot(LPCITEMIDLIST a_pidl)
{
	if (m_hwnd != NULL)
		return;

	m_bi.pidlRoot = a_pidl;
}

CString CBrowseForFolder::GetTitle() const
{
	return m_strTitle;
}

void CBrowseForFolder::SetTitle(const CString& strTitle)
{
	if (m_hwnd != NULL)
		return;

	m_strTitle = strTitle;
	m_bi.lpszTitle = m_strTitle.GetString();
}

bool CBrowseForFolder::SetTitle(const int nTitle)
{
	if (nTitle <= 0)
		return false;

	CString strTitle;
	if(!strTitle.LoadString(nTitle))
	{
		return false;
	}
	SetTitle(strTitle);
	return true;
}

void CBrowseForFolder::SetFlags(const UINT ulFlags)
{
	if (m_hwnd != NULL)
		return;

	m_bi.ulFlags = ulFlags;
}

CString CBrowseForFolder::GetSelectedFolder() const
{
	return m_szSelected;
}

bool CBrowseForFolder::SelectFolder()
{
	bool bRet = false;

	LPITEMIDLIST pidl;
	if ((pidl = ::SHBrowseForFolder(&m_bi)) != NULL)
	{
		m_strPath.Empty();
		if (SUCCEEDED(::SHGetPathFromIDList(pidl, m_szSelected)))
		{
			bRet = true;
			m_strPath = m_szSelected;
		}
		else
		{
			// Path conversion failed: drop the stale buffer so
			// GetSelectedFolder does not report an old selection.
			m_szSelected[0] = _T('\0');
		}

		LPMALLOC pMalloc;
		//Retrieve a pointer to the shell's IMalloc interface
		if (SUCCEEDED(SHGetMalloc(&pMalloc)))
		{
			// free the PIDL that SHBrowseForFolder returned to us.
			pMalloc->Free(pidl);
			// release the shell's IMalloc interface
			(void)pMalloc->Release();
		}
	}
	else
	{
		// User cancelled: clear stale state so GetSelectedFolder
		// does not return the previous successful selection.
		m_szSelected[0] = _T('\0');
		m_strPath.Empty();
	}
	m_hwnd = NULL;

	return bRet;
}

void CBrowseForFolder::OnInit() const
{
	if (!m_strPath.IsEmpty())
	{
		SetSelection(m_strPath);
	}
}

void CBrowseForFolder::OnSelChanged(LPCITEMIDLIST a_pidl) const
{
	(void)a_pidl;
}

void CBrowseForFolder::EnableOK(const bool bEnable) const
{
	if (m_hwnd == NULL)
		return;

	(void)SendMessage(m_hwnd, BFFM_ENABLEOK, bEnable, NULL);
}

void CBrowseForFolder::SetSelection(LPCITEMIDLIST a_pidl) const
{
	if (m_hwnd == NULL)
		return;

	(void)SendMessage(m_hwnd, BFFM_SETSELECTION, FALSE, (DWORD_PTR)a_pidl);
}

void CBrowseForFolder::SetSelection(const CString& strPath) const
{
	if (m_hwnd == NULL)
		return;

	(void)SendMessage(m_hwnd, BFFM_SETSELECTION, TRUE, (DWORD_PTR)(LPCTSTR)strPath);
}

void CBrowseForFolder::SetStatusText(const CString& strText) const
{
	if (m_hwnd == NULL)
		return;

	(void)SendMessage(m_hwnd, BFFM_SETSTATUSTEXT, NULL, (DWORD_PTR)(LPCTSTR)strText);
}

int __stdcall CBrowseForFolder::BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	CBrowseForFolder* pbff = (CBrowseForFolder*)lpData;
	pbff->m_hwnd = hwnd;
	if (uMsg == BFFM_INITIALIZED)
		pbff->OnInit();
	else if (uMsg == BFFM_SELCHANGED)
		pbff->OnSelChanged((LPCITEMIDLIST)lParam);
	
	return 0;
}
