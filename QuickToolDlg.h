
// QuickToolDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include <afxole.h>
#include "TrayIcon.h"
#include "CAutoStick.h"
#include "EdgeBarWnd.h"


class CQuickToolDlg;

#define ICON_PANEL_DEF_WIDTH    76              // 图标区缺省宽度（16x16 图标一排多个）2026.10.20
#define ICON_PANEL_MIN_WIDTH    42              // 图标区最小宽度 2026.10.20
#define ICON_SPLITTER_WIDTH     3               // 分隔条宽度（像素）2026.10.20
#define ICON_DIR_NAME           _T("Icons\\")   // 图标快捷方式存放目录（exe 同目录）2026.10.20

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
    CPoint      m_pntDragPoint;     // 拖动过程中的最近鼠标客户区坐标
    int         m_iScrollDir;       // 边缘自动滚动方向：0 不滚 / -1 向上 / 1 向下 2026.8.27

    afx_msg void OnTvnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnNMCustomdraw(NMHDR* pNMHDR, LRESULT* pResult);   // 暗主题文字/底色自绘 2026.10.22
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);                            // 暗主题背景 2026.10.22
    void UpdateDropTarget(CPoint point);        // 更新拖放目标检测与高亮
    void EndDrag(BOOL bDrop);

    DECLARE_MESSAGE_MAP()
};

// OLE拖放目标 - 拖动文件到文件夹时高亮显示
class CMyIconDropTarget;                                // 前置声明 2026.10.20

// 图标显示区（大图标列表）2026.10.20
class CMyIconListCtrl : public CListCtrl {
public:
    CMyIconListCtrl();
    void SetOwner(CQuickToolDlg* a_pOwner);
    void SetHoverItem(int a_iItem);                     // 更新悬停项并重绘 2026.10.20
    void ResetHover();                                  // 清除悬停状态（重建列表后调用）2026.10.20
    void ApplyTinyFont();                               // 应用 1pt 极小字体压缩标签命中区 2026.10.20
    void ApplyTipFont();                                // Tooltip 放大到 11pt 并限宽换行 2026.10.20
    int  IconHitTest(CPoint a_pnt);                     // 按图标矩形精确命中（避开空标签区）2026.10.20
protected:
    CQuickToolDlg*  m_pOwner;
    BOOL            m_bDragging;                        // 内部拖动排序进行中
    int             m_iDragItem;                        // 被拖动项索引
    int             m_iDropIndex;                       // 悬停目标项索引（-1 无效）
    CImageList*     m_pDragImage;
    CPoint          m_pntDragPoint;                     // 按下时的鼠标客户区坐标
    int             m_iHoverItem;                       // 鼠标悬停项索引（-1 无）2026.10.20
    BOOL            m_bHoverTracked;                    // 是否已注册 WM_MOUSELEAVE 跟踪 2026.10.20
    CFont           m_ftTiny;                           // 1pt 极小字体：压缩空标签命中区，防点击串行 2026.10.20
    CFont           m_ftTip;                            // Tooltip 11pt 字体 2026.10.20
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseLeave();                        // 鼠标离开清除悬停 2026.10.20
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);                // 暗主题底色 2026.10.22
    afx_msg void OnNMCustomdraw(NMHDR* pNMHDR, LRESULT* pResult);   // 悬停高亮自绘 2026.10.20
    afx_msg void OnGetInfoTip(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDropFiles(HDROP hDropInfo);          // 经典拖放兜底：加到图标区 2026.10.20
    afx_msg UINT OnGetDlgCode();
    DECLARE_MESSAGE_MAP()
};

// 图标区 OLE 拖放目标 2026.10.20
class CMyIconDropTarget : public COleDropTarget {
public:
    CMyIconDropTarget();
    void SetOwner(CQuickToolDlg* a_pOwner);
protected:
    virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
    virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
    virtual void OnDragLeave(CWnd* pWnd);
    virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
private:
    CQuickToolDlg* m_pOwner;
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
friend CMyIconListCtrl;                                 // 图标区访问内部成员 2026.10.20
friend CMyIconDropTarget;                               // 2026.10.20
protected:                           
    void LoadConfig();
    void SaveConfig();
    HTREEITEM AddTreeNode(HTREEITEM a_hParentNode,CString a_sFileName,CString a_sName,int a_iIcon);
    void LoadFolder(CString a_sPath,HTREEITEM a_hParentNode);
    CString GetFirstPath();

    void AddFileToShort(CString a_sFilePath, CString a_sTargetDir = _T(""));  // 添加一个文件到快捷方式
    CString GetCurrFilePath();                              // 获得当前选择的文件的完整路径 2023.4.25
    CString GetShortcutTargetPath(CString a_sFilePath);     // 获得快捷方式所指向的路径，非快捷方式返回原路径
    void CopyTextToClipboard(const CString& a_sText);       // 复制文本到剪贴板
    void ClearIconList();                                   // 清空图标列表 2024.1.29
    void DealMouseMove();                                   // 处理鼠标移动 2024.1.30
    void RemoveSubtreePaths(HTREEITEM hNode);               // 递归移除子树路径
    void RefreshTree(HTREEITEM hNode = NULL, const CString& sSelectPath = _T("")); // 刷新树（支持节点级）

    // 图标区 2026.10.20
    void InitIconPanel();                                   // 初始化图标区控件与图像列表
    void LayoutIconPanel();                                 // 按保存的宽度摆放图标区/分隔条/树控件
    void LoadIconBar();                                     // 从 Icons 目录加载全部图标项
    void AddIconItem(const CString& a_sLnkPath);            // 添加一个图标项（lnk 路径，图标无快捷方式箭头）
    BOOL GetShortcutIconLocation(const CString& a_sLnkPath, CString& a_sIconPath, int& a_iIconIndex); // 解析 lnk 图标位置 2026.10.21
    static HICON ExtractOneIcon(const CString& a_sFile, int a_iIconIndex);  // 按索引提取无叠加小图标 2026.10.21
    void RemoveIconItem(int a_iIndex);                      // 删除一个图标项（含删除 .lnk）
    void AddIconFromDrop(CStringArray& a_aFiles);           // 处理拖到图标区的文件（转快捷方式）
    void MoveIconItem(int a_iFrom, int a_iTo);              // 图标项位置调整（数据+界面换位）
    void SaveIconSort();                                    // 按当前显示顺序重命名 .lnk 持久化
    void LaunchIcon(int a_iIndex);                          // 运行图标项指向的程序
    static HWND FindMainWindowOfPid(DWORD a_dwPid);         // 取进程主可见顶层窗口 2026.10.21
    static void ForceWindowForeground(HWND a_hWnd);         // 强制目标窗口到前台 2026.10.21
    static UINT CALLBACK LaunchWatchThread(LPVOID a_pParam); // 后台轮询把目标主窗口置前 2026.10.21
    LRESULT OnIconBarLaunch(WPARAM wParam, LPARAM lParam);  // 图标单击启动（延迟消息处理）2026.10.20
    CString GetIconDir();                                   // 图标快捷方式目录（exe 同目录 Icons）
    CString GetIconTip(int a_iIndex);                       // 图标项 Tooltip 文本（原目标信息）
    void SaveIconWidth();                                   // 保存图标区宽度
    BOOL IsStickRight();                                    // 窗口是否吸附在屏幕右缘（决定图标区在左还是在右）
    int GetStickEdge();                                     // 贴边判定：-1=贴左缘 1=贴右缘 0=未贴（收缩/唤出方向用）2026.10.22
    void ShowIconMenu(int a_iIndex, CPoint a_pntClient);    // 图标区右键菜单 2026.10.20
    void RebuildIconItems();                                // 按数据数组重建图标列表项 2026.10.20
    static LRESULT CALLBACK IconSplitterProc(HWND a_hWnd, UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam); // 分隔条窗口过程
    int                 m_iIconMouseItem;                   // 图标区右键所在项索引 2026.10.20
    BOOL                m_bIconRight;                       // 图标区当前是否在右侧（镜像布局）2026.10.20
    CUIntArray          m_aIconImgs;                        // 图标项在图像列表中的索引（与 m_aIconLnks 对应）2026.10.20
    CPoint              m_pntSplitLast;                     // 分隔条拖动的上一次鼠标屏幕坐标 2026.10.20

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
    CEdgeBarWnd     m_edgeBar;               // 贴边收缩边条（主窗口隐藏后的唤出把手）2026.10.21


    CMyTreeCtrl         m_tvwFolder;
    CMyDropTarget        m_dropTarget;       // OLE拖放目标
    CMyIconListCtrl     m_lstIcons;         // 图标显示区 2026.10.20
    CMyIconDropTarget   m_iconDropTarget;   // 图标区 OLE 拖放目标 2026.10.20
    CImageList          m_ilIconImages;     // 图标区 32x32 图像列表 2026.10.20
    HWND                m_hSplitter;        // 可拖动分隔条 2026.10.20
    int                 m_iIconWidth;       // 图标区宽度（像素）2026.10.20
    BOOL                m_bSplitDrag;       // 分隔条拖动中 2026.10.20
    CStringArray        m_aIconLnks;        // 图标项对应的 .lnk 路径（与列表项一一对应）2026.10.20
    //CImageList          m_imgList;      // 图标列表
    CStringArray        m_aViewPath;    // 需要显示的目录 2022.3.12
    CImageList          m_ilTreeImages;             // 树控件的图标
    bool                m_bStop;
    CListString         m_aAllPath;     // 所有的路径
    CPoint              m_pntOldPoint;  // 鼠标的旧位置
    CTrayIcon           m_tiIcon;       // 托盘图标
    CFont               m_ftCaption;    // 标题栏自绘文字字体 2026.10.21
    int                 m_iCapMenuW;    // 标题栏"功能/帮助"文字钮宽(px)：文本实测宽+12；构造兜底52 2026.10.22
    CToolTipCtrl        m_tipCaption;   // 标题栏 tooltip 2026.10.21
    bool                m_bFirstShow;   // 第一次显示 2024.1.30
    HTREEITEM           m_hRightMouseItem;  // 鼠标右键选择的节点 2024.3.4


    // 生成的消息映射函数
    virtual BOOL OnInitDialog();
    afx_msg void OnNcPaint();                       // NC 边框绘制（标题栏已并入客户区）
    afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp); // 去系统标题栏 2026.10.21
    afx_msg LRESULT OnNcHitTest(CPoint point);      // 自绘标题栏拖动/按钮命中 2026.10.21
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);  // 标题栏关闭钮点击
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);    // 关闭钮悬停
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);    // resize 防闪烁：擦除时即画标题栏渐变 2026.10.21
    afx_msg HCURSOR OnQueryDragIcon();
    // 自绘标题栏 2026.10.21
    CRect GetCaptionRect() const;
    CRect GetCaptionCloseRect() const;
    CRect GetCaptionMenuRect(int a_iIdx) const;     // 功能/帮助/退出 三个文字钮
    void DrawCaptionBar(CDC* pDC);
    void OnCaptionMenu(int a_iIdx);                 // 标题栏菜单项点击（弹菜单/退出）
    void ApplyLanguage(QT_LANG a_eLang);            // 切换界面语言并立即生效（重测按钮宽/tooltip/重绘）2026.10.22
    void ApplyTheme(QT_THEME a_eTheme);             // 切换界面主题并立即生效（整窗+边条换肤）2026.10.22
    void MeasureCapMenuWidth();                     // 量取当前语言菜单钮文本宽（初始化/语言切换）2026.10.22
    void UpdateCaptionTooltip();                    // 标题栏 tooltip 按当前语言换词 2026.10.22
    BOOL    m_bCapCloseHot;         // 关闭钮悬停
    int     m_iCapMenuHot;          // 标题栏菜单项悬停（-1 无）
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnClose();
    afx_msg void OnShowWindow(BOOL bShow,UINT nStatus);
    afx_msg void OnMove(int x,int y);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    void UpdateEdgeShrink();            // 贴边收缩状态机（隐藏主窗口+边条把手）2026.10.21
    LRESULT OnEdgeBarNotify(WPARAM wParam, LPARAM lParam);  // 边条唤出通知（同步）2026.10.21
    void ExpandAtEdge(int a_iX,int a_iY,int a_iW,int a_iH); // 贴边唤出展开：TOPMOST 置前不激活（全屏让位）2026.10.23
    void EnsureMainTopmost(BOOL a_bAllow);  // 展开期主窗置顶维护（状态不符才动窗口）2026.10.23
    void TrackMouseLeave();             // 注册主窗口 WM_MOUSELEAVE 2026.10.21
    afx_msg void OnMouseLeave();        // 鼠标离窗→收缩 2026.10.21
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
    afx_msg void OnPopmenuCopyPath();
    afx_msg void OnPopmenuCopyName();
    afx_msg void OnPopmenuOpenFolder();
    afx_msg void OnAbout();
    afx_msg void OnTvnSelchangedTvwFolder(NMHDR* pNMHDR,LRESULT* pResult);
    afx_msg void OnPopmenuOpenFolderWithShortcut();
    afx_msg void OnPopmenuIconDelete();                     // 图标区右键删除 2026.10.20
    afx_msg void OnPopmenuIconOpenFolder();                 // 图标区右键打开目标位置 2026.10.20
    afx_msg void OnPopmenuIconProperty();                   // 图标区右键修改属性（目标/路径/图标）2026.10.21
    afx_msg void OnMenuLangEnglish();   // 选择语言：English（切换界面语言+持久化）2026.10.22
    afx_msg void OnMenuLangChinese();   // 选择语言：简体中文（切换界面语言+持久化）
    afx_msg void OnMenuThemeLight();    // 选择主题：亮主题（切换界面主题+持久化）
    afx_msg void OnMenuThemeDark();     // 选择主题：暗主题（切换界面主题+持久化）
    //afx_msg void OnEnterMenuLoop(BOOL bIsTrackPopupMenu);
    //afx_msg void OnExitMenuLoop(BOOL bIsTrackPopupMenu);
};
