
// QuickToolDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "QuickTool.h"
#include "QuickToolDlg.h"
#include "afxdialogex.h"
#include "BrowseForFolder.h"
#include "DlgPathManage.h"
#include "DlgInputBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_ICON_NOTIFY  WM_USER+10 


// CAboutDlg 对话框用于应用程序“关于”菜单项

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

// 对话框数据

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 数据交换


// 实现

protected:
    DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX){
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX){
    CDialogEx::DoDataExchange(pDX);
}

BOOL CAboutDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 读取帮助信息并且显示在文本框中
    CString sHelpFile=GetAppExePath()+_T("Help.txt");
    CConfigFile ccFile;
    if(ccFile.Load(sHelpFile)){
        CString sText;
        ccFile.GetAllText(sText);
        SetDlgItemText(IDC_EDT_HELP,sText);
    }

    return TRUE;  // return TRUE unless you set the focus to a control
    // 异常: OCX 属性页应返回 FALSE
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


BEGIN_MESSAGE_MAP(CMyTreeCtrl, CTreeCtrl)
    ON_WM_DROPFILES()
    ON_NOTIFY_REFLECT(TVN_BEGINDRAG, &CMyTreeCtrl::OnTvnBegindrag)
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

CMyTreeCtrl::CMyTreeCtrl() {
    m_pOwner = NULL;
    m_bDragging = FALSE;
    m_hDragItem = NULL;
    m_hDropTarget = NULL;
    m_pDragImage = NULL;
}

void CMyTreeCtrl::SetOwner(CQuickToolDlg* a_pOwner) {
    m_pOwner = a_pOwner;
}

void CMyTreeCtrl::OnDropFiles(HDROP hDropInfo) {
    CTreeCtrl::OnDropFiles(hDropInfo);
    m_pOwner->OnDropFiles(hDropInfo);
}

// ==================== 内部拖放 ====================
void CMyTreeCtrl::OnTvnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) {
    LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
    m_hDragItem = pNMTreeView->itemNew.hItem;

    // 只允许拖动文件节点（不允许拖动文件夹）
    CString sPath = *(CString*)GetItemData(m_hDragItem);
    if (DirectoryExists(sPath)) {
        *pResult = 0;
        return;
    }

    m_pDragImage = CreateDragImage(m_hDragItem);
    if (m_pDragImage) {
        m_pDragImage->BeginDrag(0, CPoint(0, 0));
        CPoint pt;
        GetCursorPos(&pt);
        m_pDragImage->DragEnter(this, pt);
        SetCapture();
        m_bDragging = TRUE;
    }
    *pResult = 0;
}

void CMyTreeCtrl::OnMouseMove(UINT nFlags, CPoint point) {
    if (m_bDragging && m_pDragImage) {
        CPoint pt;
        GetCursorPos(&pt);
        m_pDragImage->DragMove(pt);

        // 清除上一个高亮
        if (m_hDropTarget) {
            SetItemState(m_hDropTarget, 0, TVIS_DROPHILITED);
            m_hDropTarget = NULL;
        }

        // 检测目标是否为文件夹
        CPoint clientPt = point;
        UINT uFlags = 0;
        HTREEITEM hTarget = HitTest(clientPt, &uFlags);
        if (hTarget && hTarget != m_hDragItem) {
            CString sPath = *(CString*)GetItemData(hTarget);
            if (DirectoryExists(sPath)) {
                SetItemState(hTarget, TVIS_DROPHILITED, TVIS_DROPHILITED);
                EnsureVisible(hTarget);
                m_hDropTarget = hTarget;
            }
        }

        // 设置鼠标形状：有效目标显示箭头，无效目标显示禁止
        SetCursor(LoadCursor(NULL, m_hDropTarget ? IDC_ARROW : IDC_NO));
    }
    CTreeCtrl::OnMouseMove(nFlags, point);
}

void CMyTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point) {
    if (m_bDragging) {
        EndDrag(m_hDropTarget != NULL);
    }
    CTreeCtrl::OnLButtonUp(nFlags, point);
}

void CMyTreeCtrl::EndDrag(BOOL bDrop) {
    if (m_pDragImage) {
        m_pDragImage->DragLeave(this);
        m_pDragImage->EndDrag();
        delete m_pDragImage;
        m_pDragImage = NULL;
    }

    if (m_hDropTarget) {
        SetItemState(m_hDropTarget, 0, TVIS_DROPHILITED);
    }

    ReleaseCapture();
    m_bDragging = FALSE;
    SetCursor(LoadCursor(NULL, IDC_ARROW));

    if (bDrop && m_pOwner && m_hDragItem && m_hDropTarget) {
        CString sSource = *(CString*)GetItemData(m_hDragItem);
        CString sTargetDir = *(CString*)GetItemData(m_hDropTarget);
        m_pOwner->MoveShortcutToFolder(sSource, sTargetDir);
    }

    m_hDragItem = NULL;
    m_hDropTarget = NULL;
}

// ==================== CMyDropTarget ====================
CMyDropTarget::CMyDropTarget() : m_pOwner(NULL), m_hHighlight(NULL) {}

void CMyDropTarget::SetOwner(CQuickToolDlg* pOwner) {
    m_pOwner = pOwner;
}

DROPEFFECT CMyDropTarget::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) {
    if (!pDataObject->IsDataAvailable(CF_HDROP)) return DROPEFFECT_NONE;
    return UpdateHighlight(pWnd, point);
}

DROPEFFECT CMyDropTarget::OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) {
    if (!pDataObject->IsDataAvailable(CF_HDROP)) return DROPEFFECT_NONE;
    return UpdateHighlight(pWnd, point);
}

void CMyDropTarget::OnDragLeave(CWnd* pWnd) {
    ClearHighlight();
}

BOOL CMyDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) {
    if (!pDataObject->IsDataAvailable(CF_HDROP)) return FALSE;

    // 获取目标目录
    CString sTargetDir;
    if (m_hHighlight) {
        CTreeCtrl* pTree = (CTreeCtrl*)pWnd;
        sTargetDir = *(CString*)pTree->GetItemData(m_hHighlight);
    }
    ClearHighlight();

    // 提取文件列表
    STGMEDIUM stg;
    if (!pDataObject->GetData(CF_HDROP, &stg)) return FALSE;
    HDROP hDrop = (HDROP)GlobalLock(stg.hGlobal);
    if (hDrop && m_pOwner) {
        m_pOwner->HandleDropFiles(hDrop, sTargetDir);
    }
    GlobalUnlock(stg.hGlobal);
    ReleaseStgMedium(&stg);
    return TRUE;
}

DROPEFFECT CMyDropTarget::UpdateHighlight(CWnd* pWnd, CPoint point) {
    CTreeCtrl* pTree = (CTreeCtrl*)pWnd;
    UINT uFlags = 0;
    HTREEITEM hItem = pTree->HitTest(point, &uFlags);

    HTREEITEM hNewHighlight = NULL;
    if (hItem) {
        CString sPath = *(CString*)pTree->GetItemData(hItem);
        if (DirectoryExists(sPath)) {
            hNewHighlight = hItem;
        }
    }

    if (hNewHighlight != m_hHighlight) {
        ClearHighlight();
        if (hNewHighlight) {
            pTree->SetItemState(hNewHighlight, TVIS_DROPHILITED, TVIS_DROPHILITED);
            pTree->EnsureVisible(hNewHighlight);
            m_hHighlight = hNewHighlight;
        }
    }

    return hNewHighlight ? DROPEFFECT_COPY : DROPEFFECT_NONE;
}

void CMyDropTarget::ClearHighlight() {
    if (m_hHighlight && m_pOwner) {
        m_pOwner->m_tvwFolder.SetItemState(m_hHighlight, 0, TVIS_DROPHILITED);
        m_hHighlight = NULL;
    }
}

// CQuickToolDlg 对话框
void CQuickToolDlg::LoadConfig(){
    m_aViewPath.RemoveAll();
    CIniFile iniFile;
    if(iniFile.Load(GetAppExePath()+_T("Config.ini"))){
        int iPathCount=iniFile.GetInt(_T("General"), _T("PathCount"),0);
        for(int i=0;i<iPathCount;i++){
            CString sGroup=Format(_T("Path%d"),i+1);
            CString sPath=iniFile.GetStr(sGroup, _T("Path"), _T(""));
            m_aViewPath.Add(sPath);
        }
    }
    // 当配置的文件夹为空时，需要新增一个本地文件夹，并且将之加入到文件夹列表 2023.4.26
    if(m_aViewPath.GetCount()<=0){
        CString sOrigFolder=GetAppExePath()+_T("Shortcut\\");
        MakeSureDirectoryFileExist(sOrigFolder);
        m_aViewPath.Add(sOrigFolder);
    }
}

inline void CQuickToolDlg::SaveConfig(){
    CIniFile iniFile;
    if(iniFile.Load(GetAppExePath()+ _T("Config.ini"))){
        int iPathCount=(int)m_aViewPath.GetSize();
        iniFile.SetVarInt(_T("General"),_T("PathCount"),iPathCount);
        for(int i=0;i<iPathCount;i++){
            CString sGroup=Format(_T("Path%d"),i+1);
            iniFile.SetVarStr(sGroup, _T("Path"),m_aViewPath[i]);
        }
    }
    iniFile.Save();
}

CQuickToolDlg::CQuickToolDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(IDD_QUICKTOOL_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_bStop=false;
    m_bFirstShow=true;
    m_hRightMouseItem=NULL;
}

CQuickToolDlg::~CQuickToolDlg(){
    // 删除托盘图标
    // m_tiIcon.RemoveIcon();

}

void CQuickToolDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX,IDC_TVW_FOLDER,m_tvwFolder);
}

BEGIN_MESSAGE_MAP(CQuickToolDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_CLOSE()
    ON_WM_SHOWWINDOW()
    ON_WM_MOVE()
    ON_WM_TIMER()
    ON_COMMAND(ID_MENU_ADD_PATH,&CQuickToolDlg::OnMenuAddPath)
    ON_WM_SIZE()
    ON_WM_MOVING()
    ON_COMMAND(ID_EXIT,&CQuickToolDlg::OnExit)
    ON_WM_DESTROY()
    ON_COMMAND(ID_REFRESH,&CQuickToolDlg::OnRefresh)
    ON_NOTIFY(NM_CLICK,IDC_TVW_FOLDER,&CQuickToolDlg::OnClickTvwFolder)
    ON_COMMAND(ID_MENU_PATH_MANAGE,&CQuickToolDlg::OnMenuPathManage)
    ON_NOTIFY(NM_RCLICK, IDC_TVW_FOLDER, &CQuickToolDlg::OnRclickTvwFolder)
    ON_COMMAND(ID_POPMENU_DELETE, &CQuickToolDlg::OnPopmenuDeleteFile)
    ON_COMMAND(ID_POPMENU_ADD, &CQuickToolDlg::OnPopmenuAdd)
    ON_COMMAND(ID_POPMENU_ADD_FOLDER, &CQuickToolDlg::OnPopmenuAddFolder)
    ON_COMMAND(ID_POPMENU_CREATE_FOLDER, &CQuickToolDlg::OnPopmenuCreateFolder)
    ON_COMMAND(ID_POPMENU_32785, &CQuickToolDlg::OnPopmenu32785)
    ON_WM_DROPFILES()
    ON_COMMAND(ID_POPMENU_RENAME,&CQuickToolDlg::OnPopmenuRename)
    ON_COMMAND(ID_POPMENU_OPEN_FOLDER,&CQuickToolDlg::OnPopmenuOpenFolder)
    ON_COMMAND(ID_MENU_ABOUT,CQuickToolDlg::OnAbout)
    ON_NOTIFY(TVN_SELCHANGED,IDC_TVW_FOLDER,&CQuickToolDlg::OnTvnSelchangedTvwFolder)
    ON_MESSAGE(WM_ICON_NOTIFY, OnTrayNotification)
    ON_COMMAND(ID_POPMENU_OPEN_FOLDER_WITH_SHORTCUT,&CQuickToolDlg::OnPopmenuOpenFolderWithShortcut)
    ON_WM_ENTERMENULOOP()
    ON_WM_EXITMENULOOP()
END_MESSAGE_MAP()


LRESULT CQuickToolDlg::OnTrayNotification(WPARAM wParam,LPARAM lParam)
{
    if(LOWORD(lParam) == WM_LBUTTONDBLCLK){
        BOOL bShow=this->IsWindowVisible();
        if(!bShow) {
            //this->ShowWindow(SW_SHOW);
            AfxGetApp()->GetMainWnd()->ShowWindow(SW_SHOW);
            ::SetForegroundWindow(AfxGetApp()->GetMainWnd()->GetSafeHwnd());
        }
        else{
            AfxGetApp()->GetMainWnd()->ShowWindow(SW_HIDE);
        }
        return 0;
    }
    return m_tiIcon.OnTrayNotification(wParam,lParam);
}

BOOL CQuickToolDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // IDM_ABOUTBOX 系统命令在应用程序菜单的范围内。

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

    AfxOleInit();
    // 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动

    //  执行此操作

    SetIcon(m_hIcon, TRUE);			// 设置大图标
    SetIcon(m_hIcon, FALSE);		// 设置小图标

    LoadConfig();
    SetTimer(1,200,NULL);
    m_tvwFolder.SetOwner(this);
    m_dropTarget.SetOwner(this);
    m_dropTarget.Register(&m_tvwFolder);

    ModifyStyleEx(WS_EX_APPWINDOW,WS_EX_TOOLWINDOW);

    CStickWindow* pWindows=new CStickWindow();
    pWindows->SetWindow(m_hWnd);
    m_asStick.AddWindow(pWindows);

    //m_tiIcon.Create(this,WM_ICON_NOTIFY,Format("%s %s",DEF_APPLICATION_TITLE,DEF_APPLICATION_VER),m_hIcon,IDR_MENU_POP);

    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE

}

void CQuickToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码

//  来绘制图标。对于使用文档/视图模型的 MFC 应用程序，

//  这将由框架自动完成。


void CQuickToolDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // 用于绘制的设备上下文

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // 使图标在工作区中居中
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // 绘制图标
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

//当主对话框是最小化时系统调用此函数获取光标
// TODO: 在此添加消息处理程序代码
HCURSOR CQuickToolDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CQuickToolDlg::OnDestroy()
{
    if (!IsIconic()) {
    }
    m_asStick.SaveConfig(GetAppExePath()+_T("QuickPos.ini"));
    SaveConfig();

    CDialogEx::OnDestroy();
}

void CQuickToolDlg::OnClose()
{
    CDialogEx::OnClose();
}

void CQuickToolDlg::OnShowWindow(BOOL bShow,UINT nStatus)
{
    CDialogEx::OnShowWindow(bShow,nStatus);
    if(!m_bFirstShow) return;
    m_bFirstShow=false;
    m_asStick.LoadConfig(GetAppExePath()+_T("QuickPos.ini"));

    OnRefresh();
}

void CQuickToolDlg::DealMouseMove(){
}

void CQuickToolDlg::OnMove(int x,int y){
    m_asStick.OnMove(x,y);
    //     CDialogEx::OnMove(x,y);
}

void CQuickToolDlg::OnMoving(UINT fwSide,LPRECT pRect)
{
    CDialogEx::OnMoving(fwSide,pRect);
    
}

void CQuickToolDlg::OnTimer(UINT_PTR nIDEvent){
    m_asStick.OnTimer();
    CDialogEx::OnTimer(nIDEvent);
}

void CQuickToolDlg::OnMenuAddPath(){
    CBrowseForFolder dlg(this->GetSafeHwnd(),_T("请选择目录"),_T(""));
    if(dlg.SelectFolder()){
        m_aViewPath.Add(dlg.GetSelectedFolder());
        SaveConfig();
        OnRefresh();
    }
}

void CQuickToolDlg::OnSize(UINT nType,int cx,int cy)
{
    m_asStick.OnSize(nType,cx,cy);
    CRect r,rMain;
    GetClientRect(r);
    rMain.left=5;
    rMain.top=5;
    rMain.right=r.Width()-5;
    rMain.bottom=r.Height()-5;
    m_tvwFolder.MoveWindow(rMain);
    CDialogEx::OnSize(nType,cx,cy);
}

void CQuickToolDlg::OnExit(){
    //PostQuitMessage(0);
    EndDialog(0);
}

void CQuickToolDlg::OnAbout(){
    CAboutDlg dlg;
    dlg.DoModal();
}

// void CQuickToolDlg::OnEnterMenuLoop(BOOL bIsTrackPopupMenu){
//     KillTimer(1);
//     CDialogEx::OnEnterMenuLoop(bIsTrackPopupMenu);
// }
// 
// void CQuickToolDlg::OnExitMenuLoop(BOOL bIsTrackPopupMenu){
//     SetTimer(1, 200, NULL);
//     CDialogEx::OnExitMenuLoop(bIsTrackPopupMenu);
// }

void CQuickToolDlg::ClearIconList(){
    m_ilTreeImages.DeleteImageList();
    m_ilTreeImages.Create(16,16,ILC_COLOR24,5,5);
    m_ilTreeImages.SetBkColor(RGB(255,255,255));

    m_tvwFolder.SetImageList(&m_ilTreeImages,TVSIL_NORMAL);
}

HTREEITEM CQuickToolDlg::AddTreeNode(HTREEITEM a_hParentNode,CString a_sFileName,CString a_sName,int a_iIcon){
    // 新建节点
    TVINSERTSTRUCT tvBoot;
    tvBoot.hParent=a_hParentNode;
    tvBoot.hInsertAfter=NULL;
    tvBoot.item.mask=TVIF_TEXT;
    tvBoot.item.pszText=a_sName.GetBuffer();
    tvBoot.hInsertAfter=m_tvwFolder.InsertItem(&tvBoot);                      // 新建节点

//             m_tvwFolder.SetItemData(tvBoot.hInsertAfter,(DWORD_PTR)a_pPack);
    m_aAllPath.push_back(a_sFileName);
    m_tvwFolder.SetItemData(tvBoot.hInsertAfter,(DWORD_PTR)&*m_aAllPath.rbegin());
    m_tvwFolder.SetItemImage(tvBoot.hInsertAfter,a_iIcon,a_iIcon);
    return tvBoot.hInsertAfter;
}

// 加载文件夹的所有文件 2022.3.17
// void CQuickToolDlg::LoadFolder(CString a_sPath,HTREEITEM a_hParentNode){
//     CFileFind ff;
//     CString szDir=a_sPath;
//     if (szDir.Right(1)!=_T("\\")) szDir+=_T("\\");
//     szDir+=_T("*.*");
// 
//     BOOL bfile=ff.FindFile(szDir);
//     while (bfile)
//     {
//         if (m_bStop) return;
//         bfile=ff.FindNextFile();
//         CString sFileName=ff.GetFilePath();
//         CString sName=GetFileName(sFileName);
//         // 得到文件升图标
//         SHFILEINFO sfi;
//         SHGetFileInfo(sFileName, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_SMALLICON);
//         // 添加文件图标到 m_ilTreeImages
//         int iIconIndex = m_ilTreeImages.Add(sfi.hIcon);
//         DestroyIcon(sfi.hIcon);
//         if (ff.IsDirectory()&&!ff.IsDots()) {
//             HTREEITEM hItem=AddTreeNode(a_hParentNode,sFileName,sName,iIconIndex);
//             LoadFolder(ff.GetFilePath(),hItem);
//         }
//         if (!ff.IsDots()&&!ff.IsDirectory()) {
//             HTREEITEM hItem=AddTreeNode(a_hParentNode,sFileName,sName,iIconIndex);
//         }
//         //else if(!ff.IsDots()&&!ff.IsDirectory())  FindFile(ff.GetFilePath());
//     }
//     ff.Close();
// }

// 加载文件夹的所有文件 【修复完整版】
void CQuickToolDlg::LoadFolder(CString a_sPath, HTREEITEM a_hParentNode)
{
    // 停止标记
    if (m_bStop) return;

    CFileFind ff;
    CString szDir = a_sPath;

    // 路径格式化
    if (szDir.Right(1) != _T("\\"))
        szDir += _T("\\");
    szDir += _T("*.*");

    // 开始查找
    BOOL bfile = ff.FindFile(szDir);
    while (bfile)    {
        if (m_bStop)        {
            ff.Close();
            return;
        }
        bfile = ff.FindNextFile();

        CString sFileName = ff.GetFilePath();
        CString sName = ff.GetFileName();
        // 跳过 . 和 ..
        if (ff.IsDots()) {
            continue;
        }

        int iIconIndex = 0;
        // ==========================
        // 获取图标（安全版）
        // ==========================
        SHFILEINFO sfi = { 0 };
        if (SHGetFileInfo(sFileName, 0, &sfi, sizeof(sfi),
            SHGFI_ICON | SHGFI_SMALLICON))        {
            iIconIndex = m_ilTreeImages.Add(sfi.hIcon);
            DestroyIcon(sfi.hIcon); // 用完立刻销毁
        }

        // ==========================
        // 目录：递归
        // ==========================
        if (ff.IsDirectory())        {
            HTREEITEM hItem = AddTreeNode(a_hParentNode, sFileName, sName, iIconIndex);
            LoadFolder(sFileName, hItem); // 递归
        }
        // ==========================
        // 文件：添加节点
        // ==========================
        else        {
            AddTreeNode(a_hParentNode, sFileName, sName, iIconIndex);
        }
    }

    ff.Close();
}

void CQuickToolDlg::RemoveSubtreePaths(HTREEITEM hNode) {
    HTREEITEM hChild = m_tvwFolder.GetChildItem(hNode);
    while (hChild) {
        RemoveSubtreePaths(hChild);
        CString* pPath = (CString*)m_tvwFolder.GetItemData(hChild);
        if (pPath) m_aAllPath.remove(*pPath);
        HTREEITEM hNext = m_tvwFolder.GetNextSiblingItem(hChild);
        m_tvwFolder.DeleteItem(hChild);
        hChild = hNext;
    }
}

void CQuickToolDlg::RefreshTree(HTREEITEM hNode, const CString& sSelectPath) {
    // 保存水平滚动位置，避免 EnsureVisible 导致横向滚动
    SCROLLINFO si = { sizeof(SCROLLINFO) };
    si.fMask = SIF_POS;
    m_tvwFolder.GetScrollInfo(SB_HORZ, &si);
    int iHScroll = si.nPos;

    if (hNode == NULL) {
        // 全量刷新
        CString sSavedPath;
        HTREEITEM hSel = m_tvwFolder.GetSelectedItem();
        if (hSel) sSavedPath = *(CString*)m_tvwFolder.GetItemData(hSel);
        CString sSelect = sSelectPath.IsEmpty() ? sSavedPath : sSelectPath;

        m_tvwFolder.DeleteAllItems();
        m_aAllPath.clear();
        ClearIconList();
        for (int i = 0; i < m_aViewPath.GetCount(); i++) {
            LoadFolder(m_aViewPath[i], NULL);
        }

        if (!sSelect.IsEmpty()) {
            HTREEITEM hItem = FindTreeItemByPath(sSelect);
            if (hItem) {
                m_tvwFolder.EnsureVisible(hItem);
                m_tvwFolder.SelectItem(hItem);
            }
        }
    } else {
        // 节点级刷新：刷新 hNode 的子节点
        CString sDirPath = *(CString*)m_tvwFolder.GetItemData(hNode);
        RemoveSubtreePaths(hNode);
        LoadFolder(sDirPath, hNode);
        m_tvwFolder.Expand(hNode, TVE_EXPAND);

        if (!sSelectPath.IsEmpty()) {
            HTREEITEM hItem = FindTreeItemByPath(sSelectPath);
            if (hItem) {
                m_tvwFolder.EnsureVisible(hItem);
                m_tvwFolder.SelectItem(hItem);
            }
        }
    }

    // 恢复水平滚动位置
    m_tvwFolder.SetScrollPos(SB_HORZ, iHScroll);
}

void CQuickToolDlg::OnRefresh(){
    RefreshTree();
}

// 根据路径查找树节点
HTREEITEM CQuickToolDlg::FindTreeItemByPath(CString sPath, HTREEITEM hParent) {
    HTREEITEM hChild = hParent ? m_tvwFolder.GetChildItem(hParent) : m_tvwFolder.GetRootItem();
    while(hChild){
        CString sItemPath = *(CString*)m_tvwFolder.GetItemData(hChild);
        if(sItemPath.CompareNoCase(sPath) == 0) return hChild;
        HTREEITEM hFound = FindTreeItemByPath(sPath, hChild);
        if(hFound) return hFound;
        hChild = m_tvwFolder.GetNextSiblingItem(hChild);
    }
    return NULL;
}

// 运行程序并且等待返回 2021.9.3
bool ExecuteCommand(CString a_sProgram,CString a_sParams,CString a_sPath) {
    SHELLEXECUTEINFO sei;
    memset(&sei,0,sizeof(SHELLEXECUTEINFO));
    sei.cbSize=sizeof(SHELLEXECUTEINFO);
    sei.fMask=SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb=_T("open");
    sei.lpFile=(a_sProgram.GetBuffer(0));

    sei.lpParameters=a_sParams;
    sei.lpDirectory=a_sPath;
    sei.nShow=SW_HIDE;
    if (!ShellExecuteEx(&sei)) {
//         printf(_T("HaoZipC 程序启动失败,确认%s是存在的可执行程序。"),a_sProgram));

        return false;
    }
    WaitForSingleObject(sei.hProcess,INFINITE);
    CloseHandle(sei.hProcess);
    return true;
}

/*
函数功能：创建一个快捷方式文件到指定路径
函数2参数：
lpszFileName    目标文件，aNULL表示当前进程的EXE文件。
lpszLnkFileDir  快捷方式目录，可为NULL。
lpszLnkFileName 快捷方式名，aNULL表示EXE文件名。
wHotkey         为0表示不设置快捷键
pszDescription  备注
iShowCmd        显示方式，默认为3，最大化窗口
*/
BOOL CreateFileShortcut(LPCTSTR lpszFileName, LPCTSTR lpszLnkFileDir, LPCTSTR lpszLnkFileName, LPCTSTR lpszWorkDir,WORD wHotkey,LPCTSTR lpszDescription,int iShowCmd = SW_SHOWNORMAL){
    if (lpszLnkFileDir == NULL)
        return FALSE;
    HRESULT hr;
    IShellLink     *pLink;  //IShellLink接口指针  
    IPersistFile   *ppf; //IPersisFil接口指针  

    //创建IShellLink接口  
    hr = CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void**)&pLink);
    if (FAILED(hr))
        return FALSE;

    //从IShellLink接口得到IPersistFile接口  
    hr = pLink->QueryInterface(IID_IPersistFile,(void**)&ppf);
    if (FAILED(hr)){
        pLink->Release();
        return FALSE;
    }

    //设置图标  
    pLink->SetPath(lpszFileName);

    //工作目录  
    if (lpszWorkDir != NULL)
        pLink->SetPath(lpszWorkDir);

    //快捷键  
    if (wHotkey != 0)
        pLink->SetHotkey(wHotkey);

    //备注  
    if (lpszDescription != NULL)
        pLink->SetDescription(lpszDescription);

    //设置快捷键  
    pLink->SetShowCmd(iShowCmd);


    //快捷方式的目标路径 + 参数  
//     char szBuffer[MAX_PATH];
//     if (lpszLnkFileName != NULL) //如果指定了快捷方式名

//         sprintf(szBuffer,"%s\\%s",lpszLnkFileDir,lpszLnkFileName);
//     else{
//         //从第二个参数取路径名作为快捷方式的目标文件  

//         char *pstr;
//         if (lpszFileName != NULL)
//             pstr = strrchr(lpszFileName,'\\');
//         else
//             pstr = strrchr(_pgmptr,'\\');
// 
//         if (pstr == NULL){
//             ppf->Release();
//             pLink->Release();
//             return FALSE;
//         }
//         //保存为*.lnk  

//         sprintf(szBuffer,"%s\\%s",lpszLnkFileDir,pstr);
//         int nLen = strlen(szBuffer);
//         szBuffer[nLen - 3] = 'l';
//         szBuffer[nLen - 2] = 'n';
//         szBuffer[nLen - 1] = 'k';
//     }
    
    //保存快捷方式的目标路径  

//     WCHAR  wsz[MAX_PATH];  //调用Unicode版本  

//     MultiByteToWideChar(CP_ACP,0,szBuffer,-1,wsz,MAX_PATH);
    CString sFileName=lpszFileName;
    sFileName+=_T("\\");
    hr = ppf->Save(sFileName.GetBuffer(0),TRUE);
    ppf->Release();
    pLink->Release();
    return SUCCEEDED(hr);
}  

struct tagShortcutInfo{
    CString sExecuteFile;
    CString sPath;
    int iCmd;
    CString sDesc;
};

// 获取快捷方式文件的地址 2022.3.18

BOOL GetShortcutFileInfo(LPCTSTR lpszFileName,OUT tagShortcutInfo* a_pShortcutInfo){
    HRESULT hr;
    IShellLink     *pLink;  //IShellLink接口指针  
    IPersistFile   *ppf; //IPersisFil接口指针  
    //创建IShellLink接口  

    hr = CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void**)&pLink);
    if (FAILED(hr))
        return FALSE;

    //从IShellLink接口得到IPersistFile接口  
    hr = pLink->QueryInterface(IID_IPersistFile,(void**)&ppf);
    if (FAILED(hr)) {
        pLink->Release();
        return FALSE;
    }

    hr = ppf->Load(lpszFileName,TRUE);
    TCHAR sTemp[MAX_PATH];
    pLink->GetDescription(sTemp,MAX_PATH);
    a_pShortcutInfo->sDesc=sTemp;
    pLink->GetShowCmd(&a_pShortcutInfo->iCmd);
    memset(sTemp,0,MAX_PATH);
    WIN32_FIND_DATA fdFindData;
    pLink->GetPath(sTemp,MAX_PATH,&fdFindData,0);
    a_pShortcutInfo->sPath=sTemp;

    ppf->Release();
    pLink->Release();
    return SUCCEEDED(hr);
}

void CQuickToolDlg::OnClickTvwFolder(NMHDR *pNMHDR,LRESULT *pResult)
{
    CPoint point;
    GetCursorPos(&point);
    m_tvwFolder.ScreenToClient(&point);
    UINT uFlags;
    HTREEITEM CurrentItem;
    CurrentItem=m_tvwFolder.HitTest(point,&uFlags);
    if(!CurrentItem) return;
    CString sFileName = *(CString*)m_tvwFolder.GetItemData(CurrentItem);
    if(DirectoryExists(sFileName) && !(uFlags & TVHT_ONITEMBUTTON)){
        m_tvwFolder.Expand(CurrentItem, TVE_TOGGLE);
    }
    else if(FileExists(sFileName)){
        //AfxMessageBox(s);
        sFileName.MakeLower();
        if(sFileName.Right(4)==_T(".lnk")){
            tagShortcutInfo siInfo;
            GetShortcutFileInfo(sFileName.GetBuffer(0),&siInfo);
            sFileName =siInfo.sPath;
        }
        CString sPath=GetFilePath(sFileName);
        // 改变当前目录
        //SetCurrentDirectory(sPath);
        ShellExecute(NULL,_T("open"), sFileName.GetBuffer(),_T(""),sPath.GetBuffer(),SW_SHOWNORMAL);
    }
    *pResult = 0;
}

void CQuickToolDlg::OnMenuPathManage()
{
    CDlgPathManage dlg(&m_aViewPath);
    if(dlg.DoModal()==IDOK){
        SaveConfig();
        OnRefresh();
    }
}

void CQuickToolDlg::OnRclickTvwFolder(NMHDR* pNMHDR, LRESULT* pResult){
    CPoint pntPoint;
    GetCursorPos(&pntPoint);
    CMenu menu;
    VERIFY(menu.LoadMenu(IDR_MENU_POP));

    // 记录右键选择的节点 2024.3.4
    UINT uFlags;
    CPoint pntRightFolder=pntPoint;
    m_tvwFolder.ScreenToClient(&pntRightFolder);
    m_hRightMouseItem = m_tvwFolder.HitTest(pntRightFolder,&uFlags);

    CMenu* pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup != NULL);
    CWnd* pWndPopupOwner = this;

    while (pWndPopupOwner->GetStyle() & WS_CHILD)
        pWndPopupOwner = pWndPopupOwner->GetParent();

    KillTimer(1);
    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pntPoint.x, pntPoint.y, pWndPopupOwner);
    SetTimer(1, 200, NULL);
    m_pntOldPoint=pntPoint;

    *pResult = 0;
}

// 获得右键选择的节点的完整路径 2023.4.25
CString CQuickToolDlg::GetCurrFilePath() {
    if(!m_hRightMouseItem) return _T("");
    return *(CString*)m_tvwFolder.GetItemData(m_hRightMouseItem);
}

// 删除快捷键
void CQuickToolDlg::OnPopmenuDeleteFile(){
    CString sFilePath = GetCurrFilePath();
    if(sFilePath.IsEmpty()) return;
    if(!FileExists(sFilePath) && !DirectoryExists(sFilePath)) return;

    if (AfxMessageBox(Format(_T("确定要删除 \"%s\" 吗？"),sFilePath), MB_YESNO) == IDYES) {
        SHFILEOPSTRUCT shfo = {0};
        shfo.wFunc = FO_DELETE;
        shfo.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION;
        // SHFILEOPSTRUCT 需要双 null 结尾的字符串
        CString sPath = sFilePath;
        int nLen = sPath.GetLength() + 2;
        TCHAR* pszPath = new TCHAR[nLen];
        memset(pszPath, 0, nLen * sizeof(TCHAR));
        _tcscpy_s(pszPath, nLen, sPath);
        shfo.pFrom = pszPath;
        SHFileOperation(&shfo);
        delete[] pszPath;
        HTREEITEM hParent = m_tvwFolder.GetParentItem(m_hRightMouseItem);
        RefreshTree(hParent);
    }
}

// 修改文件或文件夹名称
void CQuickToolDlg::OnPopmenuRename() {
    CString sFilePath = GetCurrFilePath();
    if (sFilePath.IsEmpty()) return;

    bool bIsFile = FileExists(sFilePath);
    bool bIsDir = !bIsFile && DirectoryExists(sFilePath);
    if (!bIsFile && !bIsDir) return;

    // 提取父路径和旧名称
    CString sParentDir, sOldName, sExt;
    if (bIsFile) {
        sParentDir = GetFilePath(sFilePath);
        CString sFile = GetFileName(sFilePath);
        sOldName = GetFileNameExtend(sFile, false);
        sExt = GetFileNameExtend(sFile, true);
    } else {
        // GetFilePath对目录返回自身路径，需手动提取父目录
        int iPos = sFilePath.ReverseFind(_T('\\'));
        sParentDir = (iPos != -1) ? sFilePath.Left(iPos + 1) : _T("");
        sOldName = (iPos != -1) ? sFilePath.Mid(iPos + 1) : sFilePath;
    }

    CString sInput = sOldName;
    LPCTSTR lpszTitle = bIsFile ? _T("修改文件名称") : _T("修改文件夹名称");
    CInputBox dlg(lpszTitle, _T("修改"), &sInput);
    if (dlg.DoModal() != IDOK) return;

    CString sNewName = sInput;
    if (sNewName == sOldName) return;

    if (bIsFile) sNewName += _T(".") + sExt;
    CString sNewPath = sParentDir + sNewName;

    bool bExists = bIsFile ? FileExists(sNewPath) : DirectoryExists(sNewPath);
    if (bExists) {
        AfxMessageBox(bIsFile ? _T("该文件已存在！") : _T("该文件夹已存在！"));
    } else {
        MoveFile(sFilePath, sNewPath);
        HTREEITEM hParent = m_tvwFolder.GetParentItem(m_hRightMouseItem);
        RefreshTree(hParent, sNewPath);
    }
}

// 根据文件路径创建快捷方式
BOOL CreateShortcut(CString sPath, CString sLinkPath) {
    // 如果 sPath 是快捷方式，则直接复制一个快捷方式
    if(sPath.Right(4).CompareNoCase(_T(".lnk")) == 0){
        return CopyFile(sPath, sLinkPath, FALSE);
    }
    HRESULT hres;
    IShellLink* psl;
    // Get a pointer to the IShellLink interface.
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
    if (SUCCEEDED(hres)) {
        IPersistFile* ppf;
        // Set the path to the shortcut target, and add the description.
        psl->SetPath(sPath);
        //psl->SetDescription(L"My Shortcut");
        // Query IShellLink for the IPersistFile interface, used for saving the
        // shortcut in persistent storage.
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
        if (SUCCEEDED(hres)) {
            // Save the link by calling IPersistFile::Save.
            // CString 转换为 LPCOLESTR
            USES_CONVERSION;
            hres = ppf->Save(T2COLE(sLinkPath), TRUE);
            ppf->Release();
        }
        psl->Release();
    }
    return SUCCEEDED(hres);
}

// 获得第一个路径
CString CQuickToolDlg::GetFirstPath() {
    CString sPath;
    if (m_aViewPath.GetSize() > 0) {
        sPath = m_aViewPath[0];
        if (sPath.GetLength() > 0) {
            if(sPath.Right(1) != _T("\\")) sPath += _T("\\");
        }
    }
    return sPath;
}

// 添加一个文件到快捷方式
void CQuickToolDlg::AddFileToShort(CString a_sFilePath, CString a_sTargetDir) {
    CString sDir = a_sTargetDir.IsEmpty() ? GetFirstPath() : a_sTargetDir;
    if(sDir.Right(1) != _T("\\")) sDir += _T("\\");
    CString sShortName = GetFileNameExtend(a_sFilePath, false);
    if(sShortName.IsEmpty()) sShortName = GetFileName(a_sFilePath);
    CString sLinkPath = sDir + sShortName + _T(".lnk");
    CreateShortcut(a_sFilePath, sLinkPath);
}

// 添加一个快捷键
void CQuickToolDlg::OnPopmenuAdd(){
    if(GetFirstPath().IsEmpty()) return;
    CString sFile;
    CFileDialog dlg(TRUE);
    if (dlg.DoModal() == IDOK) {
        sFile = dlg.GetPathName();
        AddFileToShort(sFile);
        SaveConfig();
        HTREEITEM hRoot = FindTreeItemByPath(GetFirstPath());
        RefreshTree(hRoot);
    }
}
// 添加一个文件夹
void CQuickToolDlg::OnPopmenuAddFolder(){
    OnMenuAddPath();
}

// 创建文件夹 2026.4.8
void CQuickToolDlg::OnPopmenuCreateFolder(){
    CString sDir;

    if(m_hRightMouseItem){
        CString sPath = *(CString*)m_tvwFolder.GetItemData(m_hRightMouseItem);
        // 确定目标目录：右键的是文件则在同目录创建，是文件夹则在其中创建
        if(DirectoryExists(sPath)){
            sDir = sPath;
        } else if(FileExists(sPath)){
            sDir = GetFilePath(sPath);
        }
    }

    // 空白区域右键则使用第一个根目录
    if(sDir.IsEmpty()) sDir = GetFirstPath();
    if(sDir.IsEmpty()) return;
    if(sDir.Right(1) != _T("\\")) sDir += _T("\\");

    CString sFolderName;
    CInputBox dlg(_T("创建文件夹"), _T("文件夹名称"), &sFolderName);
    if (dlg.DoModal() == IDOK && !sFolderName.IsEmpty()) {
        CString sNewDir = sDir + sFolderName;
        if(DirectoryExists(sNewDir)){
            AfxMessageBox(_T("该文件夹已存在！"));
            return;
        }
        if(CreateDirectory(sNewDir, NULL)){
            HTREEITEM hParent = FindTreeItemByPath(sDir);
            RefreshTree(hParent, sNewDir);
        } else {
            AfxMessageBox(_T("创建文件夹失败！"));
        }
    }
}

// 刷新
void CQuickToolDlg::OnPopmenu32785(){
    OnRefresh();
}

// 处理拖放文件（OLE和WM_DROPFILES共用） 2026.4.7
void CQuickToolDlg::HandleDropFiles(HDROP hDropInfo, CString sTargetDir){
    int DropCount = DragQueryFile(hDropInfo, -1, NULL, 0);
    for (int i = 0; i < DropCount; i++){
        int NameSize = DragQueryFile(hDropInfo, i, NULL, 0);
        TCHAR* sName = new TCHAR[NameSize + 1];
        memset(sName, 0, NameSize + 1);
        if (sName == NULL){
            MessageBox(_T("给文件名分配暂存空间时出错!"), _T("错误信息"), MB_ICONERROR);
            return;
        }
        DragQueryFile(hDropInfo, i, sName, NameSize + 1);

        if(FileExists(sName)) AddFileToShort(sName, sTargetDir);
        if(DirectoryExists(sName)){
            if(!sTargetDir.IsEmpty()){
                AddFileToShort(sName, sTargetDir);
            } else if(m_aViewPath.GetSize()>0){
                AddFileToShort(sName, GetFirstPath());
            } else {
                m_aViewPath.Add(sName);
            }
        }
        delete[] sName;
    }
    SaveConfig();
    if (!sTargetDir.IsEmpty()) {
        HTREEITEM hTarget = FindTreeItemByPath(sTargetDir);
        RefreshTree(hTarget);
    } else {
        RefreshTree();
    }
}

// 移动快捷方式到指定文件夹 2026.4.7
void CQuickToolDlg::MoveShortcutToFolder(CString sSourceFile, CString sTargetDir) {
    if(sTargetDir.Right(1) != _T("\\")) sTargetDir += _T("\\");
    CString sDestFile = sTargetDir + GetFileName(sSourceFile);

    // 检查是否已在目标文件夹中
    CString sSourceDir = GetFilePath(sSourceFile);
    if (sSourceDir == sTargetDir) return;

    // 检查目标是否已有同名文件
    if (FileExists(sDestFile)) {
        AfxMessageBox(_T("目标文件夹已存在同名文件！"));
        return;
    }

    if (MoveFile(sSourceFile, sDestFile)) {
        SaveConfig();
        HTREEITEM hTarget = FindTreeItemByPath(sTargetDir);
        RefreshTree(hTarget, sDestFile);
    } else {
        AfxMessageBox(_T("移动文件失败！"));
    }
}

void CQuickToolDlg::OnDropFiles(HDROP hDropInfo){
    // WM_DROPFILES备用路径（拖到对话框非树控件区域时触发）
    CString sDropTargetDir;
    POINT dropPoint;
    if(DragQueryPoint(hDropInfo, &dropPoint)){
        m_tvwFolder.ScreenToClient(&dropPoint);
        UINT uFlags = 0;
        HTREEITEM hTarget = m_tvwFolder.HitTest(dropPoint, &uFlags);
        if(hTarget){
            CString sTargetPath = *(CString*)m_tvwFolder.GetItemData(hTarget);
            if(DirectoryExists(sTargetPath)){
                sDropTargetDir = sTargetPath;
            }
        }
    }
    HandleDropFiles(hDropInfo, sDropTargetDir);
    CDialogEx::OnDropFiles(hDropInfo);
}

void CQuickToolDlg::OnTvnSelchangedTvwFolder(NMHDR *pNMHDR,LRESULT *pResult)
{
    LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
    // TODO: 在此添加控件通知处理程序代码
    *pResult = 0;
}

// 打开快捷键所在的文件夹
void CQuickToolDlg::OnPopmenuOpenFolder(){
    CString sFilePath = GetCurrFilePath();
    if(FileExists(sFilePath)){
        //CString sPath = GetFilePath(sFilePath);
        ShellExecute(NULL,_T("open"),_T("explorer.exe"),_T("/select,") + sFilePath,_T(""),SW_SHOWNORMAL);
    } else if(DirectoryExists(sFilePath)){
        ShellExecute(NULL,_T("open"),sFilePath,NULL,NULL,SW_SHOWNORMAL);
    }
}

// 如果是快捷方式，则返回该快捷方式指向的文件名 2025.4.28
CString GetShortcutTarget(CString sFilePath){
    IShellLink* psl;
    HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
    if (SUCCEEDED(hres)) {
        IPersistFile* ppf;
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
        if (SUCCEEDED(hres)) {
            // Save the link by calling IPersistFile::Save.
            // CString 转换为 LPCOLESTR
            USES_CONVERSION;
            hres = ppf->Load(T2COLE(sFilePath), STGM_READ);
            if (SUCCEEDED(hres)) {
                TCHAR sPath[MAX_PATH];
                psl->GetPath(sPath, MAX_PATH, NULL, 0);
                return sPath;
            }
                
        }
        psl->Release();
    }
    return _T("");
}

void CQuickToolDlg::OnPopmenuOpenFolderWithShortcut(){
    CString sFilePath = GetShortcutTarget(GetCurrFilePath());
    if(FileExists(sFilePath)){
        ShellExecute(NULL,_T("open"),_T("explorer.exe"),_T("/select,") + sFilePath,_T(""),SW_SHOWNORMAL);
    } else if(DirectoryExists(sFilePath)){
        ShellExecute(NULL,_T("open"),sFilePath,NULL,NULL,SW_SHOWNORMAL);
    }
}
