
// QuickToolDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include <afxole.h>
#include "TrayIcon.h"
#include "CAutoStick.h"


class CQuickToolDlg;

class CMyTreeCtrl:public CTreeCtrl{
public:
    CMyTreeCtrl();
    void SetOwner(CQuickToolDlg* a_pOwner);
    afx_msg virtual void OnDropFiles(HDROP hDropInfo);
protected:
    CQuickToolDlg* m_pOwner;

    // 内部拖放
    BOOL        m_bDragging;
    HTREEITEM   m_hDragItem;
    HTREEITEM   m_hDropTarget;
    CImageList* m_pDragImage;

    afx_msg void OnTvnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    void EndDrag(BOOL bDrop);

    DECLARE_MESSAGE_MAP()
};

// OLE拖放目标 - 拖动文件到文件夹时高亮显示
class CMyDropTarget : public COleDropTarget
{
public:
    CMyDropTarget();
    void SetOwner(CQuickToolDlg* pOwner);
protected:
    virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
    virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
    virtual void OnDragLeave(CWnd* pWnd);
    virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
private:
    DROPEFFECT UpdateHighlight(CWnd* pWnd, CPoint point);
    void ClearHighlight();
    CQuickToolDlg* m_pOwner;
    HTREEITEM m_hHighlight;
};


// CQuickToolDlg 对话框
class CQuickToolDlg : public CDialogEx
{
friend CMyDropTarget;
protected:                           
    void LoadConfig();
    void SaveConfig();
    HTREEITEM AddTreeNode(HTREEITEM a_hParentNode,CString a_sFileName,CString a_sName,int a_iIcon);
    void LoadFolder(CString a_sPath,HTREEITEM a_hParentNode);
    CString GetFirstPath();

    void AddFileToShort(CString a_sFilePath, CString a_sTargetDir = _T(""));  // 添加一个文件到快捷方式
    CString GetCurrFilePath();                              // 获得当前选择的文件的完整路径 2023.4.25
    void ClearIconList();                                   // 清空图标列表 2024.1.29
    void DealMouseMove();                                   // 处理鼠标移动 2024.1.30
    void RemoveSubtreePaths(HTREEITEM hNode);               // 递归移除子树路径
    void RefreshTree(HTREEITEM hNode = NULL, const CString& sSelectPath = _T("")); // 刷新树（支持节点级）

// 构造
public:
    CQuickToolDlg(CWnd* pParent = NULL);	// 标准构造函数
    virtual ~CQuickToolDlg();
    void HandleDropFiles(HDROP hDropInfo, CString sTargetDir);    // 处理拖放文件 2026.4.7
    HTREEITEM FindTreeItemByPath(CString sPath, HTREEITEM hParent = NULL);
    void MoveShortcutToFolder(CString sSourceFile, CString sTargetDir);  // 移动快捷方式到指定文件夹

// 对话框数据
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_QUICKTOOL_DIALOG };
#endif

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
    LRESULT OnTrayNotification(WPARAM wParam,LPARAM lParam);

    
// 实现
protected:
    HICON m_hIcon;
    CAutoStick      m_asStick;               // 自动贴向屏幕边缘 2024.7.9


    CMyTreeCtrl         m_tvwFolder;
    CMyDropTarget        m_dropTarget;       // OLE拖放目标
    //CImageList          m_imgList;      // 图标列表
    CStringArray        m_aViewPath;    // 需要显示的目录 2022.3.12
    CImageList          m_ilTreeImages;             // 树控件的图标
    bool                m_bStop;
    CListString         m_aAllPath;     // 所有的路径
    CPoint              m_pntOldPoint;  // 鼠标的旧位置
    CTrayIcon           m_tiIcon;       // 托盘图标
    bool                m_bFirstShow;   // 第一次显示 2024.1.30
    HTREEITEM           m_hRightMouseItem;  // 鼠标右键选择的节点 2024.3.4


    // 生成的消息映射函数
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnClose();
    afx_msg void OnShowWindow(BOOL bShow,UINT nStatus);
    afx_msg void OnMove(int x,int y);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnMenuAddPath();
    afx_msg void OnSize(UINT nType,int cx,int cy);
    afx_msg void OnMoving(UINT fwSide,LPRECT pRect);
    afx_msg void OnExit();
    //afx_msg void OnUpdateUIState(UINT /*nAction*/,UINT /*nUIElement*/);
    afx_msg void OnDestroy();
    afx_msg void OnRefresh();
    afx_msg void OnClickTvwFolder(NMHDR *pNMHDR,LRESULT *pResult);
    afx_msg void OnMenuPathManage();
    afx_msg void OnRclickTvwFolder(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnPopmenuDeleteFile();
    afx_msg void OnPopmenuAdd();
    afx_msg void OnPopmenuAddFolder();
    afx_msg void OnPopmenuCreateFolder();
    afx_msg void OnPopmenu32785();
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnPopmenuRename();
    afx_msg void OnPopmenuOpenFolder();
    afx_msg void OnAbout();
    afx_msg void OnTvnSelchangedTvwFolder(NMHDR* pNMHDR,LRESULT* pResult);
    afx_msg void OnPopmenuOpenFolderWithShortcut();
    //afx_msg void OnEnterMenuLoop(BOOL bIsTrackPopupMenu);
    //afx_msg void OnExitMenuLoop(BOOL bIsTrackPopupMenu);
};
