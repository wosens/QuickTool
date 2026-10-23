
// QuickToolDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "QuickTool.h"
#include "QuickToolDlg.h"
#include "afxdialogex.h"
#include "BrowseForFolder.h"
#include "DlgPathManage.h"
#include "DlgInputBox.h"
#include <tlhelp32.h>       // CreateToolhelp32Snapshot：识别目标程序新旧进程 2026.10.21

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_ICON_NOTIFY  WM_USER+10
#define WM_ICONBAR_LAUNCH   (WM_USER+21)    // 图标单击启动（从列表 WM_LBUTTONUP 延迟到对话框执行，避免 ShellExecute 泵消息重入原生点击状态机）2026.10.20

BOOL CreateShortcut(CString sPath, CString sLinkPath);  // 创建快捷方式（实现见后文）2026.10.20

// 图标区配置节 2026.10.20
#define ICON_INI_SECTION     _T("IconBar")
#define ICON_SPLITTER_CLASS  _T("QTIconSplitter")   // 分隔条窗口类名 2026.10.20

// ==================== 界面语言/主题（"选择语言/选择主题"菜单实现）2026.10.22 ====================
QT_LANG  g_eQtLang  = QT_LANG_ZH;      // 运行时语言状态（stdafx.h 声明，LoadConfig/SaveConfig 持久化）
QT_THEME g_eQtTheme = QT_THEME_LIGHT;  // 运行时主题状态

// 双语文本表（索引见 stdafx.h 的 QT_TXT，顺序必须与枚举一致；带 %s 的条目供 Format 格式化）
static const LPCTSTR g_aQtTxtZh[QT_TXT_COUNT]={
    _T("功能"),_T("帮助"),_T("拖动移动窗口 · 拖边缘调整大小"),
    _T("管理目录(&M)"),_T("选择语言"),_T("English"),_T("简体中文"),_T("选择主题"),_T("亮主题"),_T("暗主题"),
    _T("关于(&A)"),_T("退出(&X)"),
    _T("添加文件(&A)"),_T("添加文件夹(&F)"),_T("创建文件夹"),_T("删除(&D)"),_T("改名"),_T("复制路径"),_T("复制文件名"),_T("刷新(&R)"),
    _T("打开文件所在位置"),_T("打开快捷键所指向的位置"),
    _T("打开图标目标所在位置"),_T("修改属性(&P)..."),_T("从图标区删除(&D)"),
    _T("确定要从图标区删除 \"%s\" 吗？（不删除原文件）"),_T("确定要删除 \"%s\" 吗？"),
    _T("打开属性对话框失败！"),_T("该文件已存在！"),_T("该文件夹已存在！"),_T("创建文件夹失败！"),
    _T("给文件名分配暂存空间时出错!"),_T("错误信息"),_T("目标文件夹已存在同名文件！"),_T("移动文件失败！"),_T("真的要删除吗？"),
    _T("修改文件名称"),_T("修改文件夹名称"),_T("修改"),_T("创建文件夹"),_T("文件夹名称"),
    _T("请选择目录"),_T("快速启动工具 · 轻量高效"),_T("版本"),_T("使 用 说 明"),_T("确 定"),_T("（未找到 Help.txt）"),
    _T("确定(&O)"),_T("取消(&C)"),_T("删除(&D)"),_T("路径管理"),
};
static const LPCTSTR g_aQtTxtEn[QT_TXT_COUNT]={
    _T("Menu"),_T("Help"),_T("Drag to move · Drag edges to resize"),
    _T("Manage Folders(&M)"),_T("Language"),_T("English"),_T("简体中文"),_T("Theme"),_T("Light"),_T("Dark"),
    _T("About(&A)"),_T("Exit(&X)"),
    _T("Add File(&A)"),_T("Add Folder(&F)"),_T("New Folder"),_T("Delete(&D)"),_T("Rename"),_T("Copy Path"),_T("Copy File Name"),_T("Refresh(&R)"),
    _T("Open File Location"),_T("Open Shortcut Target"),
    _T("Open Target Location"),_T("Properties(&P)..."),_T("Remove from Icon Bar(&D)"),
    _T("Delete \"%s\" from the icon bar? (original file is kept)"),_T("Delete \"%s\"?"),
    _T("Failed to open the properties dialog!"),_T("File already exists!"),_T("Folder already exists!"),_T("Failed to create the folder!"),
    _T("Failed to allocate buffer for the file name!"),_T("Error"),_T("A file with the same name already exists in the target folder!"),_T("Failed to move the file!"),_T("Delete the selected items?"),
    _T("Rename File"),_T("Rename Folder"),_T("Rename"),_T("New Folder"),_T("Folder name"),
    _T("Please select a folder"),_T("Fast launcher · Light & efficient"),_T("Version"),_T("U S A G E"),_T("OK"),_T("(Help.txt not found)"),
    _T("OK"),_T("Cancel"),_T("Delete(&D)"),_T("Manage Folders"),
};
static_assert(sizeof(g_aQtTxtZh)/sizeof(g_aQtTxtZh[0])==QT_TXT_COUNT,"g_aQtTxtZh 条目数须与 QT_TXT 枚举一致");
static_assert(sizeof(g_aQtTxtEn)/sizeof(g_aQtTxtEn[0])==QT_TXT_COUNT,"g_aQtTxtEn 条目数须与 QT_TXT 枚举一致");

// 按当前语言取词（索引越界兜底中文首条）
LPCTSTR QtLS(QT_TXT a_eIdx){
    if(a_eIdx<0||a_eIdx>=QT_TXT_COUNT) return g_aQtTxtZh[0];
    return (g_eQtLang==QT_LANG_EN)?g_aQtTxtEn[a_eIdx]:g_aQtTxtZh[a_eIdx];
}

// 软件名随界面语言：英文模式显示"QuickTool"（与 exe 同名，不露中文），
// 中文模式显示"闪现"；主窗标题（自绘标题栏文字取自窗口标题）/tooltip/关于窗产品名共用。
// 中文来源仍为 DEF_APPLICATION_TITLE 单一出处 2026.10.22
CString QtAppTitle(){
    return (g_eQtLang==QT_LANG_EN)?CString(_T("QuickTool")):DEF_APPLICATION_TITLE;
}

// 主题配色表（亮=2026.10.21 以来现行配色；暗=深灰蓝系列）2026.10.22
struct QT_COLORS{
    COLORREF crCap1,crCap2;                            // 标题栏/关于窗头部渐变两端
    COLORREF crCapText,crCapTextIdle,crCapMenuText;    // 标题文字（激活/非激活）与菜单钮文字
    COLORREF crCapHot;                                 // 标题栏按钮悬停底色
    COLORREF crClient;                                 // 主窗客户区底色
    COLORREF crCtrlBk;                                 // 树/图标区控件底色（亮=系统白，暗=深灰蓝）
    COLORREF crTreeText;                               // 暗主题树控件文字色（亮主题走系统绘制不用）
    COLORREF crIconHot;                                // 图标区悬停高亮
    COLORREF crAboutTitle,crAboutText,crAboutLine;     // 关于窗节标题/正文/分隔线
    COLORREF crScrollTrack,crScrollBar;                // 关于窗滚动条底/滑块
    COLORREF crSplit;                                  // 图标区分隔条
};
static const QT_COLORS g_aQtColors[2]={
    {   // 亮主题（保持既有观感）
        RGB(0x23,0x2e,0x4a),RGB(0x2e,0x51,0x8e),
        RGB(255,255,255),RGB(200,210,230),RGB(200,212,235),
        RGB(88,110,168),
        RGB(246,248,251),
        RGB(255,255,255),
        RGB(0,0,0),
        RGB(204,228,247),
        RGB(52,84,140),RGB(70,80,96),RGB(70,120,210),
        RGB(224,230,238),RGB(150,170,200),
        RGB(214,220,230),
    },
    {   // 暗主题
        RGB(0x1f,0x22,0x2e),RGB(0x36,0x3c,0x52),
        RGB(235,238,245),RGB(150,158,178),RGB(188,196,214),
        RGB(74,90,140),
        RGB(43,46,58),
        RGB(43,46,58),
        RGB(222,226,234),
        RGB(70,86,132),
        RGB(158,176,210),RGB(200,206,218),RGB(90,110,160),
        RGB(58,62,76),RGB(96,106,128),
        RGB(70,76,92),
    },
};
// 当前主题配色
static const QT_COLORS& QtColors(){
    return g_aQtColors[(g_eQtTheme==QT_THEME_DARK)?1:0];
}

// 水平渐变填充（TRIVERTEX 需 16 位分量；标题栏/关于窗头部/收缩边条共用）2026.10.22
void QtGradientH(CDC* a_pDC,const CRect& a_rc,COLORREF a_cr1,COLORREF a_cr2){
    TRIVERTEX tv[2]={0};
    tv[0].x=a_rc.left;  tv[0].y=a_rc.top;    tv[0].Red=GetRValue(a_cr1)<<8; tv[0].Green=GetGValue(a_cr1)<<8; tv[0].Blue=GetBValue(a_cr1)<<8; tv[0].Alpha=0xff<<8;
    tv[1].x=a_rc.right; tv[1].y=a_rc.bottom; tv[1].Red=GetRValue(a_cr2)<<8; tv[1].Green=GetGValue(a_cr2)<<8; tv[1].Blue=GetBValue(a_cr2)<<8; tv[1].Alpha=0xff<<8;
    GRADIENT_RECT g[1]={{0,1}};
    a_pDC->GradientFill(tv,2,g,1,GRADIENT_FILL_RECT_H);
}

// 菜单是否含指定命令项（用于识别"选择语言/选择主题"弹出项）
static BOOL QtMenuHasCmd(CMenu* a_pMenu,UINT a_uCmd){
    return (a_pMenu && a_pMenu->GetSafeHmenu() && a_pMenu->GetMenuState(a_uCmd,MF_BYCOMMAND)!=0xFFFFFFFF);
}

// 菜单文本按当前语言替换：LoadMenu 得到的是运行时副本，每次弹出前调用即可切换语言，
// 资源文件保持中文不变；弹出项标题按其子项命令识别（语言/主题两个子菜单）2026.10.22
static void QtLocalizeMenu(CMenu* a_pMenu){
    if(!a_pMenu || !a_pMenu->GetSafeHmenu()) return;
    struct TagMenuTxt{ UINT uCmd; QT_TXT eTxt; };
    static const TagMenuTxt aMap[]={
        { ID_MENU_PATH_MANAGE,        QT_TXT_MNU_PATH_MGR },
        { ID_MENU_LANG_ENGLISH,       QT_TXT_MNU_LANG_EN },
        { ID_MENU_LANG_CHINESE,       QT_TXT_MNU_LANG_ZH },
        { ID_MENU_THEME_LIGHT,        QT_TXT_MNU_THEME_LIGHT },
        { ID_MENU_THEME_DARK,         QT_TXT_MNU_THEME_DARK },
        { ID_MENU_ABOUT,              QT_TXT_MNU_ABOUT },
        { ID_EXIT,                    QT_TXT_MNU_EXIT },
        { ID_POPMENU_ADD,             QT_TXT_POP_ADD_FILE },
        { ID_POPMENU_ADD_FOLDER,      QT_TXT_POP_ADD_FOLDER },
        { ID_POPMENU_CREATE_FOLDER,   QT_TXT_POP_CREATE_FOLDER },
        { ID_POPMENU_DELETE,          QT_TXT_POP_DELETE },
        { ID_POPMENU_RENAME,          QT_TXT_POP_RENAME },
        { ID_POPMENU_COPY_PATH,       QT_TXT_POP_COPY_PATH },
        { ID_POPMENU_COPY_NAME,       QT_TXT_POP_COPY_NAME },
        { ID_POPMENU_32785,           QT_TXT_POP_REFRESH },
        { ID_POPMENU_OPEN_FOLDER,     QT_TXT_POP_OPEN_LOCATION },
        { ID_POPMENU_OPEN_FOLDER_WITH_SHORTCUT, QT_TXT_POP_OPEN_SHORTCUT },
    };
    int iCount=a_pMenu->GetMenuItemCount();
    for(int i=0;i<iCount;i++){
        UINT uCmd=a_pMenu->GetMenuItemID(i);
        if(uCmd==(UINT)-1){
            // 弹出项：递归本地化子菜单，标题按子项命令识别后替换
            CMenu* pSub=a_pMenu->GetSubMenu(i);
            if(!pSub) continue;
            QtLocalizeMenu(pSub);
            QT_TXT eTitle=QT_TXT_COUNT;
            if(QtMenuHasCmd(pSub,ID_MENU_LANG_ENGLISH))        eTitle=QT_TXT_MNU_LANG;
            else if(QtMenuHasCmd(pSub,ID_MENU_THEME_LIGHT))    eTitle=QT_TXT_MNU_THEME;
            if(eTitle!=QT_TXT_COUNT){
                a_pMenu->ModifyMenu(i,MF_BYPOSITION|MF_POPUP,(UINT_PTR)pSub->GetSafeHmenu(),QtLS(eTitle));
            }
        }else{
            for(int k=0;k<(int)(sizeof(aMap)/sizeof(aMap[0]));k++){
                if(uCmd==aMap[k].uCmd){
                    a_pMenu->ModifyMenu(i,MF_BYPOSITION|MF_STRING,uCmd,QtLS(aMap[k].eTxt));
                    break;
                }
            }
        }
    }
}

// 语言/主题子菜单打单选圆点（弹出前调用，菜单上显示当前选择）2026.10.22
static void QtCheckLangTheme(CMenu* a_pMenu){
    if(!a_pMenu || !a_pMenu->GetSafeHmenu()) return;
    for(int i=0;i<a_pMenu->GetMenuItemCount();i++){
        CMenu* pSub=a_pMenu->GetSubMenu(i);
        if(!pSub) continue;
        if(QtMenuHasCmd(pSub,ID_MENU_LANG_ENGLISH)){
            pSub->CheckMenuRadioItem(ID_MENU_LANG_ENGLISH,ID_MENU_LANG_CHINESE,
                (g_eQtLang==QT_LANG_EN)?ID_MENU_LANG_ENGLISH:ID_MENU_LANG_CHINESE,MF_BYCOMMAND);
        }
        if(QtMenuHasCmd(pSub,ID_MENU_THEME_LIGHT)){
            pSub->CheckMenuRadioItem(ID_MENU_THEME_LIGHT,ID_MENU_THEME_DARK,
                (g_eQtTheme==QT_THEME_DARK)?ID_MENU_THEME_DARK:ID_MENU_THEME_LIGHT,MF_BYCOMMAND);
        }
    }
}

// ==================== CMyIconListCtrl 图标显示区 ====================
BEGIN_MESSAGE_MAP(CMyIconListCtrl, CListCtrl)
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSELEAVE()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_DROPFILES()
    ON_WM_ERASEBKGND()                                              // 暗主题底色 2026.10.22
    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CMyIconListCtrl::OnNMCustomdraw)
    ON_NOTIFY_REFLECT(LVN_GETINFOTIP, &CMyIconListCtrl::OnGetInfoTip)
    ON_NOTIFY_REFLECT(NM_RCLICK, &CMyIconListCtrl::OnRclick)
END_MESSAGE_MAP()

CMyIconListCtrl::CMyIconListCtrl() {
    m_pOwner = NULL;
    m_bDragging = FALSE;
    m_iDragItem = -1;
    m_iDropIndex = -1;
    m_pDragImage = NULL;
    m_iHoverItem = -1;
    m_bHoverTracked = FALSE;
}

void CMyIconListCtrl::SetOwner(CQuickToolDlg* a_pOwner) {
    m_pOwner = a_pOwner;
}

// 按图标矩形精确命中：LVM_SETICONSPACING 压缩网格后，项的空标签命中区
// 会盖住下一行图标导致点击串行，故只在图标自身矩形（含 4 像素容差）内判定 2026.10.20
int CMyIconListCtrl::IconHitTest(CPoint a_pnt) {
    if (!GetSafeHwnd()) return -1;
    CRect rcList;
    GetClientRect(&rcList);
    if (!rcList.PtInRect(a_pnt)) return -1;
    int iBest = -1;
    int iBestDist = INT_MAX;
    int iCount = GetItemCount();
    for (int i = 0; i < iCount; i++) {
        CRect rcIcon;
        if (!GetItemRect(i, &rcIcon, LVIR_ICON)) continue;
        if (rcIcon.PtInRect(a_pnt)) return i;       // 在图标矩形内直接命中
        // 矩形外的横/纵距离（在矩形内为 0）
        int iDx = (a_pnt.x < rcIcon.left) ? rcIcon.left - a_pnt.x : (a_pnt.x > rcIcon.right - 1 ? a_pnt.x - (rcIcon.right - 1) : 0);
        int iDy = (a_pnt.y < rcIcon.top) ? rcIcon.top - a_pnt.y : (a_pnt.y > rcIcon.bottom - 1 ? a_pnt.y - (rcIcon.bottom - 1) : 0);
        if (iDx > 4 || iDy > 4) continue;           // 距图标 4 像素外视为空白
        int iDist = iDx + iDy;
        if (iDist < iBestDist) {
            iBestDist = iDist;
            iBest = i;
        }
    }
    return iBest;
}

// 更新悬停项：索引变化时重绘新旧两项（浅蓝高亮由 NM_CUSTOMDRAW 绘制）；
// 重绘必须带擦除（TRUE），否则自绘的浅蓝背景像素不被清除导致移出后残留；
// 移出图标（a_iItem<0）时同时清除选中框，避免高亮框滞留 2026.10.20
void CMyIconListCtrl::SetHoverItem(int a_iItem) {
    if (m_iHoverItem == a_iItem) return;
    int iOld = m_iHoverItem;
    m_iHoverItem = a_iItem;
    if (m_iHoverItem < 0) SetItemState(-1, 0, LVIS_SELECTED);
    if (iOld >= 0) {
        CRect rcOld;
        if (GetItemRect(iOld, &rcOld, LVIR_BOUNDS)) InvalidateRect(&rcOld, TRUE);
    }
    if (m_iHoverItem >= 0) {
        CRect rcNew;
        if (GetItemRect(m_iHoverItem, &rcNew, LVIR_BOUNDS)) InvalidateRect(&rcNew, TRUE);
    }
}

// 清除悬停状态（列表重建后索引失效）2026.10.20
void CMyIconListCtrl::ResetHover() {
    m_iHoverItem = -1;
    m_bHoverTracked = FALSE;
}

// 应用 1pt 极小字体：列表不显示文字，仅为压缩空标签命中区高度，
// 防止上一项标签命中区盖住下一行图标导致点击串行 2026.10.20
void CMyIconListCtrl::ApplyTinyFont() {
    if (!GetSafeHwnd()) return;
    if (m_ftTiny.GetSafeHandle() == NULL) {
        NONCLIENTMETRICS ncm = { 0 };
        ncm.cbSize = sizeof(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
        LOGFONT lf = ncm.lfMessageFont;
        CClientDC dc(this);
        lf.lfHeight = -MulDiv(1, dc.GetDeviceCaps(LOGPIXELSY), 72);     // 1pt
        m_ftTiny.CreateFontIndirect(&lf);
    }
    SetFont(&m_ftTiny);
}

// Tooltip 放大到 11pt 系统消息字体并限宽换行（默认 infotip 字号太小看不清）2026.10.20
void CMyIconListCtrl::ApplyTipFont() {
    if (!GetSafeHwnd()) return;
    CToolTipCtrl* pTip = GetToolTips();
    if (!pTip || !pTip->GetSafeHwnd()) return;
    if (m_ftTip.GetSafeHandle() == NULL) {
        NONCLIENTMETRICS ncm = { 0 };
        ncm.cbSize = sizeof(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
        LOGFONT lf = ncm.lfMessageFont;
        CClientDC dc(this);
        lf.lfHeight = -MulDiv(11, dc.GetDeviceCaps(LOGPIXELSY), 72);    // 11pt
        m_ftTip.CreateFontIndirect(&lf);
    }
    pTip->SetFont(&m_ftTip);
    pTip->SetMaxTipWidth(420);          // 长路径自动换行，不超出屏幕 2026.10.20
    pTip->ModifyStyle(0, TTS_ALWAYSTIP);    // 窗口未激活时也能弹出 2026.10.20
}

// 鼠标离开图标区：清除悬停高亮与选中残留（SetHoverItem(-1) 内一并清选中）2026.10.20
void CMyIconListCtrl::OnMouseLeave() {
    m_bHoverTracked = FALSE;
    SetHoverItem(-1);
}

// 暗主题图标区底色为深灰蓝（系统默认白底刺眼且与主窗不符）；亮主题走系统白底 2026.10.22
BOOL CMyIconListCtrl::OnEraseBkgnd(CDC* pDC) {
    if (g_eQtTheme == QT_THEME_DARK) {
        CRect rc;
        GetClientRect(&rc);
        pDC->FillSolidRect(rc, QtColors().crCtrlBk);
        return TRUE;
    }
    return CListCtrl::OnEraseBkgnd(pDC);
}

// 悬停高亮自绘：悬停项图标下画圆角背景（颜色随主题：亮=浅蓝，暗=蓝灰）2026.10.20/10.22
void CMyIconListCtrl::OnNMCustomdraw(NMHDR* pNMHDR, LRESULT* pResult) {
    LPNMLVCUSTOMDRAW pLVCD = (LPNMLVCUSTOMDRAW)pNMHDR;
    *pResult = 0;
    if (pLVCD->nmcd.dwDrawStage == CDDS_PREPAINT) {
        *pResult = CDRF_NOTIFYITEMDRAW;
        return;
    }
    if (pLVCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
        int iItem = (int)pLVCD->nmcd.dwItemSpec;
        if (iItem == m_iHoverItem && m_iHoverItem >= 0) {
            CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
            CRect rcIcon;
            if (GetItemRect(iItem, &rcIcon, LVIR_ICON)) {
                rcIcon.InflateRect(3, 3);
                CBrush brush(QtColors().crIconHot);
                CBrush* pOld = pDC->SelectObject(&brush);
                CPen pen(PS_NULL, 0, RGB(0, 0, 0));
                CPen* pOldPen = pDC->SelectObject(&pen);
                pDC->RoundRect(&rcIcon, CPoint(4, 4));
                pDC->SelectObject(pOld);
                pDC->SelectObject(pOldPen);
            }
            // 图标之后仍需系统画图标本身
            *pResult = CDRF_NOTIFYPOSTPAINT;
            return;
        }
        *pResult = CDRF_DODEFAULT;
        return;
    }
    if (pLVCD->nmcd.dwDrawStage == CDDS_ITEMPOSTPAINT) {
        *pResult = CDRF_DODEFAULT;
        return;
    }
}

// 记录按下位置与项索引（供拖动排序判定）；命中按图标矩形判定 2026.10.20
// 不调基类：原生 OnLButtonDown 内部 DragDetect 会泵走原地抬起时的 WM_LBUTTONUP，
// 导致单击启动收不到抬起消息；选中/焦点由下面 SetItemState 全权完成 2026.10.21
void CMyIconListCtrl::OnLButtonDown(UINT nFlags, CPoint point) {
    m_iDragItem = -1;
    m_iDropIndex = -1;
    int iIndex = IconHitTest(point);
    if (iIndex >= 0) {
        m_iDragItem = iIndex;
        m_pntDragPoint = point;
        // 先整体清除再精确选中图标命中的项，保证唯一（基类按空标签区命中可能选错相邻项）2026.10.20
        SetItemState(-1, 0, LVIS_SELECTED);
        SetItemState(iIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    } else {
        // 点在空白处清除系统选中，避免残留选中态误导
        SetItemState(-1, 0, LVIS_SELECTED);
    }
}

// 拖动排序：按下后移动超过 4 像素开始拖动；非拖动时跟踪悬停项 2026.10.20
void CMyIconListCtrl::OnMouseMove(UINT nFlags, CPoint point) {
    if (!m_bDragging) {
        // 悬停跟踪：注册 WM_MOUSELEAVE，命中测试更新高亮 2026.10.20
        if (!m_bHoverTracked) {
            TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, m_hWnd, 0 };
            if (_TrackMouseEvent(&tme)) m_bHoverTracked = TRUE;
        }
        SetHoverItem(IconHitTest(point));
        if ((nFlags & MK_LBUTTON) && m_iDragItem >= 0) {
            CSize szDrag = point - m_pntDragPoint;
            if (abs(szDrag.cx) > 4 || abs(szDrag.cy) > 4) {
                POINT pntHot = { point.x, point.y };
                m_pDragImage = CreateDragImage(m_iDragItem, &pntHot);
                if (m_pDragImage) {
                    m_pDragImage->BeginDrag(0, CPoint(0, 0));
                    CPoint pntScreen = point;
                    ClientToScreen(&pntScreen);
                    m_pDragImage->DragEnter(NULL, pntScreen);
                    SetCapture();
                    m_bDragging = TRUE;
                    m_iDropIndex = -1;
                }
            }
        }
        CListCtrl::OnMouseMove(nFlags, point);
        return;
    }

    // 拖动中：跟随鼠标并检测悬停项
    if (m_pDragImage) {
        CPoint pntScreen = point;
        ClientToScreen(&pntScreen);
        m_pDragImage->DragMove(pntScreen);
        int iHover = IconHitTest(point);            // 与点击命中一致，按图标矩形判定 2026.10.20
        m_iDropIndex = (iHover >= 0 && iHover != m_iDragItem) ? iHover : -1;
        SetCursor(LoadCursor(NULL, m_iDropIndex >= 0 ? IDC_ARROW : IDC_NO));
    }
    CListCtrl::OnMouseMove(nFlags, point);
}

// 单击打开：按下后未构成拖动（移动 <4 像素）且抬起仍命中同一图标时运行；
// 启动放在抬起而非按下，避免想拖动排序时误启动 2026.10.20
void CMyIconListCtrl::OnLButtonUp(UINT nFlags, CPoint point) {
    if (m_bDragging) {
        if (m_pDragImage) {
            m_pDragImage->DragLeave(NULL);
            m_pDragImage->EndDrag();
            delete m_pDragImage;
            m_pDragImage = NULL;
        }
        if (GetCapture() == this) ReleaseCapture();
        m_bDragging = FALSE;
        int iFrom = m_iDragItem;
        int iDrop = m_iDropIndex;
        m_iDragItem = -1;
        m_iDropIndex = -1;
        if (m_pOwner && iFrom >= 0 && iDrop >= 0 && iDrop != iFrom) {
            m_pOwner->MoveIconItem(iFrom, iDrop);
        }
        return;
    }
    // 快速二次按下会以 WM_LBUTTONDBLCLK 到达（不经过 OnLButtonDown），m_iDragItem 已清，不会二次启动；
    // 启动延迟到对话框消息处理（PostMessage），先清 m_iDragItem 防重入二次启动 2026.10.20
    int iDown = m_iDragItem;
    int iUpHit = IconHitTest(point);
    if (iDown >= 0 && m_pOwner && iUpHit == iDown) {
        m_pOwner->PostMessage(WM_ICONBAR_LAUNCH, (WPARAM)iUpHit, 0);
    }
    m_iDragItem = -1;
}

// Tooltip：显示原目标信息（文件名+路径）2026.10.20
void CMyIconListCtrl::OnGetInfoTip(NMHDR* pNMHDR, LRESULT* pResult) {
    NMLVGETINFOTIP* pTip = (NMLVGETINFOTIP*)pNMHDR;
    if (m_pOwner) {
        CString sTip = m_pOwner->GetIconTip(pTip->iItem);
        if (!sTip.IsEmpty()) {
            lstrcpyn(pTip->pszText, sTip, pTip->cchTextMax);
        }
    }
    *pResult = 0;
}

// 经典拖放兜底：拖到图标区的文件直接加入图标区（不走树）2026.10.20
void CMyIconListCtrl::OnDropFiles(HDROP hDropInfo) {
    if (m_pOwner) {
        CStringArray aFiles;
        int iCount = DragQueryFile(hDropInfo, -1, NULL, 0);
        for (int i = 0; i < iCount; i++) {
            int iSize = DragQueryFile(hDropInfo, i, NULL, 0) + 1;
            CString sName;
            DragQueryFile(hDropInfo, i, sName.GetBuffer(iSize), iSize);
            sName.ReleaseBuffer();
            aFiles.Add(sName);
        }
        m_pOwner->AddIconFromDrop(aFiles);
    }
}

// 右键弹出图标项菜单 2026.10.20
void CMyIconListCtrl::OnRclick(NMHDR* pNMHDR, LRESULT* pResult) {
    // 从 NMITEMACTIVATE 取项索引与点击坐标（不依赖真实光标位置）；
    // iItem 依赖系统命中（受空标签区影响），故再用图标矩形精确复核 2026.10.20
    LPNMITEMACTIVATE pItem = (LPNMITEMACTIVATE)pNMHDR;
    int iIndex = IconHitTest(pItem->ptAction);
    if (m_pOwner && iIndex >= 0) {
        m_pOwner->ShowIconMenu(iIndex, pItem->ptAction);
    }
    *pResult = 0;
}

// ==================== CMyIconDropTarget 图标区 OLE 拖放 ====================
CMyIconDropTarget::CMyIconDropTarget() {
    m_pOwner = NULL;
}

void CMyIconDropTarget::SetOwner(CQuickToolDlg* a_pOwner) {
    m_pOwner = a_pOwner;
}

DROPEFFECT CMyIconDropTarget::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) {
    if (!pDataObject->IsDataAvailable(CF_HDROP)) return DROPEFFECT_NONE;
    return DROPEFFECT_COPY;
}

DROPEFFECT CMyIconDropTarget::OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) {
    if (!pDataObject->IsDataAvailable(CF_HDROP)) return DROPEFFECT_NONE;
    return DROPEFFECT_COPY;
}

void CMyIconDropTarget::OnDragLeave(CWnd* pWnd) {
}

BOOL CMyIconDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) {
    if (!pDataObject->IsDataAvailable(CF_HDROP)) return FALSE;

    STGMEDIUM stg;
    if (!pDataObject->GetData(CF_HDROP, &stg)) return FALSE;
    HDROP hDrop = (HDROP)GlobalLock(stg.hGlobal);
    if (hDrop && m_pOwner) {
        int iCount = DragQueryFile(hDrop, -1, NULL, 0);
        CStringArray aFiles;
        for (int i = 0; i < iCount; i++) {
            int iSize = DragQueryFile(hDrop, i, NULL, 0) + 1;
            CString sName;
            DragQueryFile(hDrop, i, sName.GetBuffer(iSize), iSize);
            sName.ReleaseBuffer();
            aFiles.Add(sName);
        }
        m_pOwner->AddIconFromDrop(aFiles);
    }
    GlobalUnlock(stg.hGlobal);
    ReleaseStgMedium(&stg);
    return TRUE;
}


// 英文帮助兜底文本（与 Execute\Help_en.txt 内容保持一致；英文模式下该文件缺失时
// 关于窗"使用说明"直接使用，保证英文界面不露中文）2026.10.22
static LPCTSTR g_aQtHelpEn[]={
    _T("Welcome to QuickTool"),
    _T(""),
    _T("1. Open Windows File Explorer"),
    _T(""),
    _T("2. Drag the programs you want to launch quickly from File Explorer onto the QuickTool window, and QuickTool will automatically turn them into shortcuts"),
    _T(""),
    _T("3. You can also add a folder to QuickTool, so that all programs inside that folder can be launched quickly"),
    _T(""),
    _T(""),
    _T("Suggested usage:"),
    _T("1. Create a dedicated folder on your computer to hold software shortcuts, sorted into categories."),
    _T("For example: create a folder named \"Shortcuts\" on drive D, then create subfolders inside it, such as:"),
    _T("      Common apps, Image processing, Network, Development tools, Multimedia, etc."),
    _T("2. Drag the newly created folder onto the QuickTool window"),
    _T("3. Avoid putting files directly into that folder; make sure it contains only shortcuts, otherwise some items may fail to run"),
};
static const int g_iQtHelpEnCount=sizeof(g_aQtHelpEn)/sizeof(g_aQtHelpEn[0]);

// 关于窗圆角半径：OnPaint 卡片圆角裁剪与 OnInitDialog 窗口外框顶部圆角共用，
// 两处必须同值，头部圆角与外框圆角才能严格贴合 2026.10.23
static const int s_iAboutRound=12;

// CAboutDlg 对话框用于应用程序“关于”菜单项
// 视觉：无系统标题栏的自绘卡片——深色渐变头部（图标+产品名+版本徽章）+
// 浅色帮助区（Help.txt 自绘文本）+ 底部圆角确定按钮 2026.10.21
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
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg HCURSOR OnQueryDragIcon();
private:
    void DrawButton(CDC* pDC);
    CRect GetButtonRect() const;
    CRect GetCloseRect() const;
    BOOL  m_bBtnHot;            // 确定按钮悬停
    BOOL  m_bCloseHot;          // 关闭钮悬停
    CFont m_ftTitle;            // 产品名 16pt Bold
    CFont m_ftVer;              // 版本徽章 9pt
    CFont m_ftHelp;             // 帮助正文 10pt
    CFont m_ftBtn;              // 按钮 10.5pt
    CString m_sHelp;            // Help.txt 内容
    int     m_iScroll;          // 帮助正文滚动偏移（滚轮）2026.10.21
    int     m_iTextHeight;      // 帮助正文完整高度（超出视口可滚动）2026.10.21
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX){
    m_bBtnHot=FALSE;
    m_bCloseHot=FALSE;
    m_iScroll=0;
    m_iTextHeight=0;
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX){
    CDialogEx::DoDataExchange(pDX);
}

BOOL CAboutDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 去掉系统标题栏与边框，自绘整卡
    ModifyStyle(WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX,WS_POPUP);
    SetWindowPos(NULL,0,0,366,640,SWP_NOMOVE|SWP_NOZORDER);

    // 窗口外形随自绘卡片：模板 DS_MODALFRAME 残留的 WS_EX_DLGMODALFRAME 直角外框
    // 沿圆弧被系统裁掉，顶部两角与圆角头部同半径贴合；底部两角保持直角外框 2026.10.23
    CRect rcWin;
    GetWindowRect(&rcWin);
    CRgn rgnFrame;
    rgnFrame.CreateRoundRectRgn(0,0,rcWin.Width(),rcWin.Height()+s_iAboutRound,s_iAboutRound,s_iAboutRound);    // 高度外扩一个半径，底部圆弧移出窗外即成直角
    SetWindowRgn((HRGN)rgnFrame.Detach(),TRUE); // 区域句柄所有权移交系统，不得再 DeleteObject
    CenterWindow();

    // 模板里的旧控件（图标/静态/帮助编辑框/确定钮）全部隐藏，画面完全自绘 2026.10.21
    CWnd* pChild=GetWindow(GW_CHILD);
    while(pChild){
        pChild->ShowWindow(SW_HIDE);
        pChild=pChild->GetNextWindow();
    }

    // 字体（帮助 9.5pt、按钮 10.5pt 以 0.5pt 精度存为十倍值，其余整数 pt）2026.10.21
    CClientDC dc(this);
    int iDpiY=dc.GetDeviceCaps(LOGPIXELSY);
    struct TagFontDef{ CFont* pFont; int iPtX10; int iWeight; };    // 字号=十倍pt
    TagFontDef aDefs[]={ {&m_ftTitle,160,FW_SEMIBOLD}, {&m_ftVer,90,FW_NORMAL}, {&m_ftHelp,110,FW_NORMAL}, {&m_ftBtn,110,FW_SEMIBOLD} };
    for(int i=0;i<4;i++){
        LOGFONT lf={0};
        NONCLIENTMETRICS ncm={0};
        ncm.cbSize=sizeof(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof(ncm),&ncm,0);
        lf=ncm.lfMessageFont;
        lf.lfWeight=aDefs[i].iWeight;
        lf.lfHeight=-MulDiv(aDefs[i].iPtX10,iDpiY,720);     // 十倍pt → 像素
        aDefs[i].pFont->CreateFontIndirect(&lf);
    }

    // 读取帮助信息（随界面语言：英文优先读 Help_en.txt，中文读 Help.txt——Sun 库
    // CConfigFile 按 UTF-8 解码，两文件均存 UTF-8；英文文件缺失时用内置英文兜底）2026.10.22
    CString sHelpFile=GetAppExePath()+((g_eQtLang==QT_LANG_EN)?_T("Help_en.txt"):_T("Help.txt"));
    CConfigFile ccFile;
    if(ccFile.Load(sHelpFile)){
        ccFile.GetAllText(m_sHelp);
    }
    if(m_sHelp.IsEmpty() && g_eQtLang==QT_LANG_EN){
        for(int i=0;i<g_iQtHelpEnCount;i++){
            if(i>0) m_sHelp+=_T("\r\n");
            m_sHelp+=g_aQtHelpEn[i];
        }
    }
    if(m_sHelp.IsEmpty()) m_sHelp=QtLS(QT_TXT_ABOUT_NO_HELP);

    return TRUE;
}

// 版本徽章文本（前缀随界面语言：版本/Version）
static CString AboutVersionBadge(){
    CString s=DEF_APPLICATION_VER;          // v1.0.40
    s.TrimLeft(_T("vV"));
    return CString(QtLS(QT_TXT_ABOUT_VER))+_T(" ")+s;
}

// 圆角矩形填充+描边
static void AboutRoundRect(CDC* pDC,const CRect& rc,int iRadius,COLORREF crFill,COLORREF crLine,int iLineW=0){
    CBrush br(crFill);
    CBrush* pOldBr=pDC->SelectObject(&br);
    CPen pen(iLineW>0?PS_SOLID:PS_NULL,iLineW,crLine);
    CPen* pOldPen=pDC->SelectObject(&pen);
    pDC->RoundRect(rc,CPoint(iRadius,iRadius));
    pDC->SelectObject(pOldBr);
    pDC->SelectObject(pOldPen);
}

// 底部确定按钮矩形
CRect CAboutDlg::GetButtonRect() const{
    CRect rc;
    GetClientRect(&rc);
    CRect rcBtn(rc.Width()/2-70,rc.bottom-52,rc.Width()/2+70,rc.bottom-16);
    return rcBtn;
}

// 右上角关闭钮矩形
CRect CAboutDlg::GetCloseRect() const{
    CRect rc;
    GetClientRect(&rc);
    return CRect(rc.right-38,10,rc.right-12,36);
}

// 绘制确定按钮（悬停/普通两态）
void CAboutDlg::DrawButton(CDC* pDC){
    CRect rc=GetButtonRect();
    COLORREF crFill=m_bBtnHot?RGB(68,138,255):RGB(45,110,230);
    AboutRoundRect(pDC,rc,8,crFill,crFill);
    CFont* pOld=pDC->SelectObject(&m_ftBtn);
    pDC->SetBkMode(TRANSPARENT);
    pDC->SetTextColor(RGB(255,255,255));
    pDC->DrawText(QtLS(QT_TXT_ABOUT_OK),&rc,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    pDC->SelectObject(pOld);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_SETCURSOR()
    ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

// 自绘整个窗口：头部渐变+图标+产品名+版本徽章+关闭钮，中部帮助文本，底部按钮
void CAboutDlg::OnPaint()
{
    CPaintDC dc(this);
    CRect rc;
    GetClientRect(&rc);

    // 整卡底色与圆角外形（区域背景透明化四角）
    CRgn rgn;
    rgn.CreateRoundRectRgn(0,0,rc.Width()+1,rc.Height()+1,s_iAboutRound,s_iAboutRound);
    dc.SelectClipRgn(&rgn);

    // 头部深色渐变（藏蓝→靛蓝，随主题色表）
    const int iHeadH=118;
    CRect rcHead(0,0,rc.Width(),iHeadH);
    CRect rcBody(0,iHeadH,rc.Width(),rc.Height());
    QtGradientH(&dc,rcHead,QtColors().crCap1,QtColors().crCap2);    // 2026.10.22 取色走主题表
    // 头部底部 1px 高光分隔线
    dc.FillSolidRect(0,iHeadH-1,rc.Width(),1,QtColors().crAboutLine);

    // 身体浅色（暗主题为深底）
    dc.FillSolidRect(rcBody,QtColors().crClient);

    // 应用图标 40x40 于头部左侧：按显示尺寸精确提取（LoadIcon 固定 32px 拉伸会发虚）2026.10.21
    HICON hIcon=(HICON)LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME),IMAGE_ICON,40,40,LR_DEFAULTCOLOR);
    if(!hIcon) hIcon=AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    if(hIcon){
        ::DrawIconEx(dc.GetSafeHdc(),22,26,hIcon,40,40,0,NULL,DI_NORMAL);
        DestroyIcon(hIcon);
    }

    // 产品名
    CFont* pOldFont=dc.SelectObject(&m_ftTitle);
    dc.SetBkMode(TRANSPARENT);
    dc.SetTextColor(RGB(255,255,255));
    CString sTitle=QtAppTitle();    // 产品名随界面语言（英文 QuickTool / 中文 闪现）2026.10.22
    dc.DrawText(sTitle,CRect(76,24,rc.Width()-48,54),DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);

    // 副标题（随界面语言）
    dc.SelectObject(&m_ftVer);
    dc.SetTextColor(RGB(190,208,235));
    dc.DrawText(QtLS(QT_TXT_ABOUT_SUBTITLE),CRect(76,54,rc.Width()-48,74),DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    dc.SelectObject(pOldFont);

    // 版本徽章（圆角胶囊）
    {
        dc.SelectObject(&m_ftVer);
        CString sVer=AboutVersionBadge();
        CSize sz=dc.GetTextExtent(sVer);
        CRect rcBadge(76,78,76+sz.cx+22,78+sz.cy+8);
        AboutRoundRect(&dc,rcBadge,sz.cy/2+4,RGB(70,120,210),RGB(120,165,240),1);
        dc.SetTextColor(RGB(225,238,255));
        dc.DrawText(sVer,&rcBadge,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    }

    // 关闭钮（头部右上 ×）
    CRect rcClose=GetCloseRect();
    dc.SelectObject(&m_ftVer);
    dc.SetTextColor(m_bCloseHot?RGB(255,255,255):RGB(170,190,220));
    if(m_bCloseHot) AboutRoundRect(&dc,rcClose,6,RGB(226,74,74),RGB(226,74,74));
    dc.DrawText(_T("✕"),&rcClose,DT_CENTER|DT_VCENTER|DT_SINGLELINE);

    // “使用说明”小节标题（随界面语言与主题）
    pOldFont=dc.SelectObject(&m_ftBtn);
    dc.SetTextColor(QtColors().crAboutTitle);
    dc.DrawText(QtLS(QT_TXT_ABOUT_SECTION),CRect(22,iHeadH+14,rc.Width()-22,iHeadH+34),DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    dc.FillSolidRect(22,iHeadH+38,44,2,QtColors().crAboutLine);     // 标题下短线
    dc.SelectObject(pOldFont);

    // 帮助正文（自绘多行文本，滚轮可滚动查看超长内容）2026.10.21
    CRect rcText(22,iHeadH+50,rc.Width()-22,GetButtonRect().top-14);
    dc.SelectObject(&m_ftHelp);
    dc.SetTextColor(QtColors().crAboutText);
    // 先在内存测量完整高度
    {
        CDC dcMem;
        dcMem.CreateCompatibleDC(&dc);
        dcMem.SelectObject(&m_ftHelp);
        CRect rcMeasure(0,0,rcText.Width(),0);
        int iH=dcMem.DrawText(m_sHelp,&rcMeasure,DT_LEFT|DT_WORDBREAK|DT_CALCRECT);
        m_iTextHeight=iH;
    }
    int iMaxScroll=max(0,m_iTextHeight-rcText.Height());
    if(m_iScroll>iMaxScroll) m_iScroll=iMaxScroll;
    if(m_iScroll<0) m_iScroll=0;
    dc.SaveDC();
    dc.IntersectClipRect(rcText);
    dc.OffsetWindowOrg(0,m_iScroll);    // 上移内容实现滚动
    CRect rcDraw(rcText.left,rcText.top,rcText.right,rcText.top+m_iTextHeight);
    dc.DrawText(m_sHelp,&rcDraw,DT_LEFT|DT_WORDBREAK);
    dc.RestoreDC(-1);
    // 内容未满时隐藏滚动指示；有更多内容时在右缘画细滚动条
    if(iMaxScroll>0){
        CRect rcTrack(rc.right-6,rcText.top,rc.right-3,rcText.bottom);
        dc.FillSolidRect(rcTrack,QtColors().crScrollTrack);
        int iBarH=max(24,(int)((__int64)rcText.Height()*rcText.Height()/m_iTextHeight));
        int iBarY=rcText.top+(rcText.Height()-iBarH)*m_iScroll/iMaxScroll;
        dc.FillSolidRect(rc.right-6,iBarY,3,iBarH,QtColors().crScrollBar);
    }

    // 底部确定按钮
    DrawButton(&dc);
    dc.SelectClipRgn(NULL);
}

// 滚轮滚动帮助正文 2026.10.21
BOOL CAboutDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    if(m_iTextHeight>0){
        m_iScroll -= zDelta/2;      // 一格约 2.4 行的 1/2
        Invalidate(FALSE);
        return TRUE;
    }
    return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}

BOOL CAboutDlg::OnEraseBkgnd(CDC* /*pDC*/)
{
    return TRUE;        // 全部在 OnPaint 绘制，防闪烁
}

// 点击：按钮=关窗；头部=拖动窗口；右上=关闭
void CAboutDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    if(GetButtonRect().PtInRect(point)){
        OnOK();
        return;
    }
    if(GetCloseRect().PtInRect(point)){
        OnOK();
        return;
    }
    CRect rcHead;
    GetClientRect(&rcHead);
    if(point.y<118){                        // 头部区域拖动
        PostMessage(WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x,point.y));
        return;
    }
    CDialogEx::OnLButtonDown(nFlags, point);
}

void CAboutDlg::OnMouseMove(UINT nFlags, CPoint point)
{
    BOOL bBtn=GetButtonRect().PtInRect(point);
    BOOL bCls=GetCloseRect().PtInRect(point);
    if(bBtn!=m_bBtnHot || bCls!=m_bCloseHot){
        m_bBtnHot=bBtn;
        m_bCloseHot=bCls;
        Invalidate(FALSE);
    }
    CDialogEx::OnMouseMove(nFlags, point);
}

// 头部区域显示手型光标提示可拖动 2026.10.21
BOOL CAboutDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    if(nHitTest==HTCLIENT){
        CPoint pnt;
        GetCursorPos(&pnt);
        ScreenToClient(&pnt);
        if(pnt.y<118 && !GetCloseRect().PtInRect(pnt)){
            SetCursor(LoadCursor(NULL,IDC_HAND));
            return TRUE;
        }
    }
    return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}

HCURSOR CAboutDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(AfxGetApp()->LoadIcon(IDR_MAINFRAME));
}


BEGIN_MESSAGE_MAP(CMyTreeCtrl, CTreeCtrl)
    ON_WM_DROPFILES()
    ON_NOTIFY_REFLECT(TVN_BEGINDRAG, &CMyTreeCtrl::OnTvnBegindrag)
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_WM_TIMER()
    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CMyTreeCtrl::OnNMCustomdraw)  // 暗主题文字/底色自绘 2026.10.22
    ON_WM_ERASEBKGND()                                              // 暗主题背景 2026.10.22
END_MESSAGE_MAP()

CMyTreeCtrl::CMyTreeCtrl() {
    m_pOwner = NULL;
    m_bDragging = FALSE;
    m_hDragItem = NULL;
    m_hDropTarget = NULL;
    m_pDragImage = NULL;
    m_iScrollDir = 0;
}

void CMyTreeCtrl::SetOwner(CQuickToolDlg* a_pOwner) {
    m_pOwner = a_pOwner;
}

// 暗主题树控件自绘：系统画法为 COLOR_WINDOW 白底黑字，暗主题改为深底浅字；
// 亮主题直接走系统绘制（返回 CDRF_DODEFAULT 保持既有观感）2026.10.22
void CMyTreeCtrl::OnNMCustomdraw(NMHDR* pNMHDR, LRESULT* pResult) {
    LPNMTVCUSTOMDRAW pDraw = (LPNMTVCUSTOMDRAW)pNMHDR;
    *pResult = CDRF_DODEFAULT;
    if (g_eQtTheme != QT_THEME_DARK) return;
    switch (pDraw->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
        *pResult = CDRF_NOTIFYITEMDRAW;
        return;
    case CDDS_ITEMPREPAINT:
        pDraw->clrText = QtColors().crTreeText;
        pDraw->clrTextBk = QtColors().crCtrlBk;
        *pResult = CDRF_DODEFAULT;
        return;
    }
}

// 暗主题擦除为深底色（系统擦除用 COLOR_WINDOW 白色）；亮主题走系统擦除 2026.10.22
BOOL CMyTreeCtrl::OnEraseBkgnd(CDC* pDC) {
    if (g_eQtTheme == QT_THEME_DARK) {
        CRect rc;
        GetClientRect(&rc);
        pDC->FillSolidRect(rc, QtColors().crCtrlBk);
        return TRUE;
    }
    return CTreeCtrl::OnEraseBkgnd(pDC);
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

// 更新拖放目标检测与高亮（拖动与自动滚动后共用）
void CMyTreeCtrl::UpdateDropTarget(CPoint point) {
    // 清除上一个高亮
    if (m_hDropTarget) {
        SetItemState(m_hDropTarget, 0, TVIS_DROPHILITED | TVIS_SELECTED);
        m_hDropTarget = NULL;
    }

    // 检测目标是否为文件夹
    UINT uFlags = 0;
    HTREEITEM hTarget = HitTest(point, &uFlags);
    if (hTarget && hTarget != m_hDragItem) {
        CString sPath = *(CString*)GetItemData(hTarget);
        if (DirectoryExists(sPath)) {
            SetItemState(hTarget, TVIS_SELECTED, TVIS_SELECTED);   // 以选择状态显示目标文件夹 2026.8.27
            EnsureVisible(hTarget);
            m_hDropTarget = hTarget;
        }
    }
}

void CMyTreeCtrl::OnMouseMove(UINT nFlags, CPoint point) {
    if (m_bDragging && m_pDragImage) {
        CPoint pt;
        GetCursorPos(&pt);
        m_pDragImage->DragMove(pt);
        m_pntDragPoint = point;

        UpdateDropTarget(point);

        // 靠近控件上/下边缘时启动自动滚动，否则无法拖到较远的文件夹 2026.8.27
        CRect rcClient;
        GetClientRect(&rcClient);
        const int iEdge = 16;                   // 边缘判定像素
        if (point.y <= iEdge) {
            m_iScrollDir = -1;
            SetTimer(2, 60, NULL);
        } else if (point.y >= rcClient.bottom - iEdge) {
            m_iScrollDir = 1;
            SetTimer(2, 60, NULL);
        } else {
            m_iScrollDir = 0;
            KillTimer(2);
        }

        // 设置鼠标形状：有效目标显示箭头，无效目标显示禁止
        SetCursor(LoadCursor(NULL, m_hDropTarget ? IDC_ARROW : IDC_NO));
    }
    CTreeCtrl::OnMouseMove(nFlags, point);
}

// 拖动靠近边缘时的自动滚动定时器 2026.8.27
void CMyTreeCtrl::OnTimer(UINT_PTR nIDEvent) {
    if (nIDEvent == 2 && m_bDragging && m_pDragImage && m_iScrollDir != 0) {
        m_pDragImage->DragShowNolock(FALSE);
        SendMessage(WM_VSCROLL, MAKEWPARAM(m_iScrollDir < 0 ? SB_LINEUP : SB_LINEDOWN, 0));
        m_pDragImage->DragShowNolock(TRUE);
        UpdateDropTarget(m_pntDragPoint);
        return;
    }
    CTreeCtrl::OnTimer(nIDEvent);
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
        SetItemState(m_hDropTarget, 0, TVIS_DROPHILITED | TVIS_SELECTED);
    }

    ReleaseCapture();
    m_bDragging = FALSE;
    KillTimer(2);                  // 停止边缘自动滚动 2026.8.27
    m_iScrollDir = 0;
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
        g_eQtLang =(QT_LANG)iniFile.GetInt(_T("General"),_T("Lang"),QT_LANG_ZH);       // 界面语言 2026.10.22
        g_eQtTheme=(QT_THEME)iniFile.GetInt(_T("General"),_T("Theme"),QT_THEME_LIGHT); // 界面主题 2026.10.22
    }
    // 非法配置值兜底为默认（首帧绘制前读取，界面直接以保存的语言/主题出现）
    if(g_eQtLang!=QT_LANG_EN) g_eQtLang=QT_LANG_ZH;
    if(g_eQtTheme!=QT_THEME_DARK) g_eQtTheme=QT_THEME_LIGHT;
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
        iniFile.SetVarInt(_T("General"),_T("Lang"),(int)g_eQtLang);       // 语言持久化 2026.10.22
        iniFile.SetVarInt(_T("General"),_T("Theme"),(int)g_eQtTheme);     // 主题持久化 2026.10.22
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
    m_hSplitter=NULL;                   // 2026.10.20
    m_iIconWidth=ICON_PANEL_DEF_WIDTH;  // 2026.10.20
    m_bSplitDrag=FALSE;                 // 2026.10.20
    m_bIconRight=FALSE;                 // 2026.10.20
    m_iIconMouseItem=-1;                // 2026.10.20
    m_bCapCloseHot=FALSE;               // 自绘标题栏关闭钮悬停 2026.10.21
    m_iCapMenuHot=-1;                   // 自绘标题栏菜单项悬停 2026.10.21
    m_iCapMenuW=52;                     // 兜底，OnInitDialog 实测后覆盖（防测量异常退化现状）2026.10.22
}

CQuickToolDlg::~CQuickToolDlg(){
    // 删除托盘图标
    // m_tiIcon.RemoveIcon();

}

void CQuickToolDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX,IDC_TVW_FOLDER,m_tvwFolder);
    DDX_Control(pDX,IDC_LST_ICONS,m_lstIcons);      // 图标显示区 2026.10.20
}

BEGIN_MESSAGE_MAP(CQuickToolDlg, CDialogEx)
    ON_WM_NCCALCSIZE()
    ON_WM_NCHITTEST()
    ON_WM_NCPAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSELEAVE()
    ON_MESSAGE(WM_APP+301,OnEdgeBarNotify)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
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
    ON_COMMAND(ID_POPMENU_COPY_PATH,&CQuickToolDlg::OnPopmenuCopyPath)
    ON_COMMAND(ID_POPMENU_COPY_NAME,&CQuickToolDlg::OnPopmenuCopyName)
    ON_COMMAND(ID_POPMENU_OPEN_FOLDER,&CQuickToolDlg::OnPopmenuOpenFolder)
    ON_COMMAND(ID_MENU_ABOUT,CQuickToolDlg::OnAbout)
    ON_NOTIFY(TVN_SELCHANGED,IDC_TVW_FOLDER,&CQuickToolDlg::OnTvnSelchangedTvwFolder)
    ON_MESSAGE(WM_ICON_NOTIFY, OnTrayNotification)
    ON_MESSAGE(WM_ICONBAR_LAUNCH, OnIconBarLaunch)
    ON_COMMAND(ID_POPMENU_OPEN_FOLDER_WITH_SHORTCUT,&CQuickToolDlg::OnPopmenuOpenFolderWithShortcut)
    ON_COMMAND(ID_POPMENU_ICON_DELETE,&CQuickToolDlg::OnPopmenuIconDelete)          // 2026.10.20
    ON_COMMAND(ID_POPMENU_ICON_OPENFOLDER,&CQuickToolDlg::OnPopmenuIconOpenFolder)  // 2026.10.20
    ON_COMMAND(ID_POPMENU_ICON_PROPERTY,&CQuickToolDlg::OnPopmenuIconProperty)      // 修改属性 2026.10.21
    ON_COMMAND(ID_MENU_LANG_ENGLISH,&CQuickToolDlg::OnMenuLangEnglish)              // 2026.10.22
    ON_COMMAND(ID_MENU_LANG_CHINESE,&CQuickToolDlg::OnMenuLangChinese)
    ON_COMMAND(ID_MENU_THEME_LIGHT,&CQuickToolDlg::OnMenuThemeLight)
    ON_COMMAND(ID_MENU_THEME_DARK,&CQuickToolDlg::OnMenuThemeDark)
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
    // 工具窗口标题栏矮，32px 大图当小图标会被裁剪不显；从 ico 提取 16px 档专作小图标 2026.10.21
    {
        HICON hSmall=(HICON)LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME),IMAGE_ICON,
            GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),LR_DEFAULTCOLOR);
        if(hSmall){
            SetIcon(hSmall, FALSE);
            DestroyIcon(hSmall);
        }else{
            SetIcon(m_hIcon, FALSE);
        }
    }

    LoadConfig();
    SetWindowText(Format(_T("%s %s"), QtAppTitle(), DEF_APPLICATION_VER));   // 软件名随界面语言（须在 LoadConfig 读取语言后设置，自绘标题栏文字取自窗口标题）2026.10.22
    SetTimer(1,200,NULL);
    m_tvwFolder.SetOwner(this);
    m_dropTarget.SetOwner(this);
    m_dropTarget.Register(&m_tvwFolder);
    InitIconPanel();                                    // 初始化图标区 2026.10.20

    ModifyStyleEx(WS_EX_APPWINDOW,WS_EX_TOOLWINDOW);
    // MFC 对话框即使模板无 WS_CAPTION 也会强制补上；标题栏已并入客户区自绘，
    // 此处强制移除系统标题栏（含其文字/关闭钮，DWM 下不重画）2026.10.21
    ModifyStyle(WS_CAPTION|WS_SYSMENU,0,SWP_FRAMECHANGED);

    CStickWindow* pWindows=new CStickWindow();
    pWindows->SetWindow(m_hWnd);
    m_asStick.AddWindow(pWindows);

    // 贴边收缩边条：主窗口隐藏后由独立边条窗口做唤出把手 2026.10.21
    m_edgeBar.CreateEdgeBar(this,GetScreenHeight(),480);    // 初建；OnShowWindow 位置恢复后按主窗口实际高重建

    //m_tiIcon.Create(this,WM_ICON_NOTIFY,Format("%s %s",DEF_APPLICATION_TITLE,DEF_APPLICATION_VER),m_hIcon,IDR_MENU_POP);

    // 标题栏 tooltip：悬停标题栏空白区显示当前版本提示（第二行随界面语言）2026.10.21/10.22
    if(m_tipCaption.Create(this,TTS_ALWAYSTIP|TTS_NOPREFIX)){
        m_tipCaption.SetMaxTipWidth(300);
        m_tipCaption.AddTool(this,Format(_T("%s %s\r\n%s"),QtAppTitle(),DEF_APPLICATION_VER,QtLS(QT_TXT_CAP_TIP)));
        m_tipCaption.SetDelayTime(TTDT_INITIAL,400);
    }

    // 量取当前语言"功能/帮助"文本宽 → 菜单钮宽（LoadConfig 已读语言；切换时 ApplyLanguage 重测）
    // （FR-1 实测+12 方案，2026.10.22 提取为 MeasureCapMenuWidth 支持语言切换复测）
    MeasureCapMenuWidth();

    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE

}

// ==================== 自绘标题栏 ====================
// 系统 NC 标题栏在 WS_POPUP+TOOLWINDOW 组合下不绘制图标（DWM 合成 NC 时不走应用
// 绘制），NC 打补丁不可靠；改为标准 borderless 做法：WM_NCCALCSIZE 把系统标题栏
// 高度归零（客户区从窗口顶开始），标题栏画在客户区顶部：
// 图标+标题文字+关闭钮（悬停变红），拖动区 NCHITTEST 返回 HTCAPTION 2026.10.21
#define QT_CAPTION_H 30                             // 自绘标题栏高度

// 标题栏区（客户区坐标）
CRect CQuickToolDlg::GetCaptionRect() const{
    CRect rc;
    GetClientRect(&rc);
    return CRect(0,0,rc.right,QT_CAPTION_H);
}
// 关闭钮（客户区坐标）
CRect CQuickToolDlg::GetCaptionCloseRect() const{
    CRect rc=GetCaptionRect();
    return CRect(rc.right-34,4,rc.right-8,QT_CAPTION_H-4);
}
// 标题栏菜单项矩形（功能/帮助；退出钮已由 × 承担 2026.10.21）
CRect CQuickToolDlg::GetCaptionMenuRect(int a_iIdx) const{
    CRect rc=GetCaptionRect();
    int iRight=rc.right-40;                         // 关闭钮左侧
    int iW=m_iCapMenuW>0?m_iCapMenuW:52;            // 实测宽；异常时退化为原 52px 方案 2026.10.22
    return CRect(iRight-iW-(1-a_iIdx)*iW,6,iRight-(1-a_iIdx)*iW,QT_CAPTION_H-6);
}

// NCCALCSIZE：完全 borderless——窗口无 NC 区域，客户区=整个窗口矩形，
// 标题栏画在客户区顶部，边缘缩放由 NCHITTEST 手工命中实现 2026.10.21
void CQuickToolDlg::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
    // 不调基类：客户区=窗口矩形（无标题栏/无边框），一切界面元素都在客户区内自绘
}

// 自绘标题栏（客户区顶部 QT_CAPTION_H 条带，配色随主题、按钮文字随语言）2026.10.21/10.22
void CQuickToolDlg::DrawCaptionBar(CDC* pDC)
{
    CRect rcCap=GetCaptionRect();
    BOOL bActive=(::GetForegroundWindow()==GetSafeHwnd());
    // 渐变条（与关于窗口头部一致，取色走主题表）
    QtGradientH(pDC,rcCap,QtColors().crCap1,QtColors().crCap2);
    // 16px 应用图标
    HICON hIcon=(HICON)LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
    if(hIcon){
        int iX=6, iY=(QT_CAPTION_H-16)/2;
        ::DrawIconEx(pDC->GetSafeHdc(),iX,iY,hIcon,16,16,0,NULL,DI_NORMAL);
        DestroyIcon(hIcon);
    }
    // 标题文字
    if(!m_ftCaption.GetSafeHandle()){
        NONCLIENTMETRICS ncm={0};
        ncm.cbSize=sizeof(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof(ncm),&ncm,0);
        ncm.lfCaptionFont.lfHeight=-12;
        m_ftCaption.CreateFontIndirect(&ncm.lfCaptionFont);
    }
    CFont* pOld=pDC->SelectObject(&m_ftCaption);
    pDC->SetBkMode(TRANSPARENT);
    pDC->SetTextColor(bActive?QtColors().crCapText:QtColors().crCapTextIdle);
    CString sTitle;
    GetWindowText(sTitle);
    // 标题文字区动态计算：图标右侧(28) 到 最左菜单钮左侧，按当前宽度自适应，
    // 窄窗口才出现省略号 2026.10.21
    {
        CRect rcFirstMenu=GetCaptionMenuRect(0);
        int iTextRight=max(rcFirstMenu.left-8, 40);     // 至少留 40px，避免负宽
        pDC->DrawText(sTitle,CRect(28,0,iTextRight,QT_CAPTION_H),DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
    }
    // 标题栏菜单项（功能/帮助），替代原 NC 菜单栏；退出由 × 关闭钮承担 2026.10.21
    const LPCTSTR aMenus[]={QtLS(QT_TXT_CAP_MENU),QtLS(QT_TXT_CAP_HELP)};
    for(int i=0;i<2;i++){
        CRect rcMenu=GetCaptionMenuRect(i);
        // 悬停高亮：圆角块（色随主题），高亮时文字白色保持清晰 2026.10.21/10.22
        if(m_iCapMenuHot==i) AboutRoundRect(pDC,rcMenu,4,QtColors().crCapHot,QtColors().crCapHot);
        pDC->SetTextColor(m_iCapMenuHot==i?RGB(255,255,255):QtColors().crCapMenuText);
        pDC->DrawText(aMenus[i],&rcMenu,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    }
    pDC->SelectObject(pOld);
    // 关闭钮（悬停变红）
    CRect rcClose=GetCaptionCloseRect();
    if(m_bCapCloseHot) AboutRoundRect(pDC,rcClose,5,RGB(226,74,74),RGB(226,74,74));
    CFont* pOld2=pDC->SelectObject(&m_ftCaption);
    pDC->SetTextColor(m_bCapCloseHot?RGB(255,255,255):RGB(200,212,235));
    pDC->DrawText(_T("✕"),&rcClose,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    pDC->SelectObject(pOld2);
}

void CQuickToolDlg::OnNcPaint()
{
    CDialogEx::OnNcPaint();
    // 标题栏已并入客户区自绘（DrawCaptionBar），NC 无需补画
}

// 标题栏菜单项点击：功能/帮助弹出对应子菜单，退出关闭 2026.10.21
void CQuickToolDlg::OnCaptionMenu(int a_iIdx){
    CMenu menu;
    if(menu.LoadMenu(IDR_MENU1)){
        CMenu* pSub=NULL;
        if(a_iIdx==0) pSub=menu.GetSubMenu(0);          // 功能
        else if(a_iIdx==1) pSub=menu.GetSubMenu(1);     // 帮助
        if(pSub){
            QtLocalizeMenu(pSub);                       // 弹出前按当前语言取词（资源保持中文）2026.10.22
            if(a_iIdx==0) QtCheckLangTheme(pSub);       // 功能菜单：语言/主题子菜单打单选圆点显示当前选择 2026.10.22
            CRect rc=GetCaptionMenuRect(a_iIdx);
            ClientToScreen(&rc);
            // TrackPopupMenu 要求属主窗口在前台，否则菜单弹出不接收点击、
            // 点外部也不消失，会一直挂在屏幕上（实测残留屏幕中间）2026.10.21
            SetForegroundWindow();
            KillTimer(1);
            pSub->TrackPopupMenu(TPM_LEFTALIGN|TPM_BOTTOMALIGN|TPM_RIGHTBUTTON,rc.left,rc.top,this);
            // 菜单关闭后补一次置前（TrackPopupMenu 消息泵结束后窗口可能失活）
            PostMessage(WM_NULL);
            SetForegroundWindow();
            SetTimer(1,200,NULL);
            return;
        }
    }
}

// ==================== 选择语言/选择主题（菜单命令实现）2026.10.22 ====================
// 切换语言：更新运行时语言并立即生效——重测标题栏按钮文本宽度、tooltip 换词、
// 整窗重绘；菜单每次弹出经 QtLocalizeMenu 按当前语言取词，无需重建；选择持久化 Config.ini
void CQuickToolDlg::ApplyLanguage(QT_LANG a_eLang){
    if(g_eQtLang==a_eLang) return;
    g_eQtLang=a_eLang;
    SaveConfig();
    SetWindowText(Format(_T("%s %s"),QtAppTitle(),DEF_APPLICATION_VER));    // 软件名随语言即时切换（DrawCaptionBar 取窗口标题绘制）2026.10.22
    MeasureCapMenuWidth();      // 英文按钮文本与中文宽度不同，重测 m_iCapMenuW
    UpdateCaptionTooltip();
    RedrawWindow(NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN|RDW_UPDATENOW);
}

// 切换主题：亮=2026.10.21 以来现行配色，暗=深灰蓝配色。标题栏/客户区/树/图标区/
// 分隔条/关于窗/收缩边条取色均走 QtColors()（或 g_eQtTheme），切换后整窗重绘即完成换肤
void CQuickToolDlg::ApplyTheme(QT_THEME a_eTheme){
    if(g_eQtTheme==a_eTheme) return;
    g_eQtTheme=a_eTheme;
    SaveConfig();
    m_ilTreeImages.SetBkColor(QtColors().crCtrlBk); // 树图标底随主题（暗主题保持白底会露白块）
    RedrawWindow(NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN|RDW_UPDATENOW);
    if(m_edgeBar.GetSafeHwnd() && m_edgeBar.IsWindowVisible()) m_edgeBar.Invalidate(FALSE); // 边条把手同色系
}

// 量取当前语言"功能/帮助"按钮文本宽 → 菜单钮宽=实测+12（初始化与语言切换时调用）2026.10.22
void CQuickToolDlg::MeasureCapMenuWidth(){
    if(!m_ftCaption.GetSafeHandle()){
        NONCLIENTMETRICS ncm={0};
        ncm.cbSize=sizeof(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof(ncm),&ncm,0);
        ncm.lfCaptionFont.lfHeight=-12;             // 与 DrawCaptionBar 同参
        m_ftCaption.CreateFontIndirect(&ncm.lfCaptionFont);
    }
    CClientDC dc(this);
    CFont* pOldFt=dc.SelectObject(&m_ftCaption);
    int iMaxW=0;
    const LPCTSTR aCapMenus[]={QtLS(QT_TXT_CAP_MENU),QtLS(QT_TXT_CAP_HELP)};
    for(int i=0;i<2;i++){
        CRect rcTxt(0,0,0,0);
        dc.DrawText(aCapMenus[i],&rcTxt,DT_CALCRECT|DT_SINGLELINE);
        if(rcTxt.Width()>iMaxW) iMaxW=rcTxt.Width();
    }
    dc.SelectObject(pOldFt);
    m_iCapMenuW=iMaxW+12;                           // 测量异常时构造兜底 52 生效（异常时此值≈12+0）
}

// 标题栏 tooltip 按当前语言换词（标题+版本+操作提示）2026.10.22
void CQuickToolDlg::UpdateCaptionTooltip(){
    if(!m_tipCaption.GetSafeHwnd()) return;
    m_tipCaption.UpdateTipText(Format(_T("%s %s\r\n%s"),QtAppTitle(),DEF_APPLICATION_VER,QtLS(QT_TXT_CAP_TIP)),this);
}

// 选择语言：English / 简体中文（命令路由自语言子菜单）2026.10.22
void CQuickToolDlg::OnMenuLangEnglish(){
    ApplyLanguage(QT_LANG_EN);
}
void CQuickToolDlg::OnMenuLangChinese(){
    ApplyLanguage(QT_LANG_ZH);
}
// 选择主题：亮 / 暗（命令路由自主题子菜单）2026.10.22
void CQuickToolDlg::OnMenuThemeLight(){
    ApplyTheme(QT_THEME_LIGHT);
}
void CQuickToolDlg::OnMenuThemeDark(){
    ApplyTheme(QT_THEME_DARK);
}

// 命中测试：四边/四角 6px 内返回缩放命中（borderless 手工缩放），
// 自绘标题栏区域返回 HTCAPTION（拖动），关闭钮/菜单钮返回 HTCLIENT
LRESULT CQuickToolDlg::OnNcHitTest(CPoint point)
{
    // 边缘缩放：窗口四周 6 像素热区（贴边收缩态不启用，避免与贴边吸附冲突）
    CRect rcWin;
    GetWindowRect(&rcWin);
    BOOL bShrinked=IsStickRight();                  // 收缩态窗口探出屏幕，缩放无意义
    const int iEdge=6;
    BOOL bL=(point.x-rcWin.left)<iEdge;
    BOOL bR=(rcWin.right-point.x)<=iEdge;
    BOOL bT=(point.y-rcWin.top)<iEdge;
    BOOL bB=(rcWin.bottom-point.y)<=iEdge;
    if(!bShrinked){
        if(bT && bL) return HTTOPLEFT;
        if(bT && bR) return HTTOPRIGHT;
        if(bB && bL) return HTBOTTOMLEFT;
        if(bB && bR) return HTBOTTOMRIGHT;
        if(bL) return HTLEFT;
        if(bR) return HTRIGHT;
        if(bT) return HTTOP;
        if(bB) return HTBOTTOM;
    }
    CPoint pnt=point;
    ScreenToClient(&pnt);
    CRect rcCap=GetCaptionRect();
    if(rcCap.PtInRect(pnt)){
        if(GetCaptionCloseRect().PtInRect(pnt)) return HTCLIENT;    // 关闭钮按客户区点击处理
        for(int i=0;i<2;i++){                                       // 菜单项按客户区点击处理
            if(GetCaptionMenuRect(i).PtInRect(pnt)) return HTCLIENT;
        }
        return HTCAPTION;                                           // 其余标题栏区域=拖动
    }
    return HTCLIENT;                                    // 全客户区窗口，无其余 NC 区域
}

void CQuickToolDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    if(GetCaptionCloseRect().PtInRect(point)){
        PostMessage(WM_CLOSE);
        return;
    }
    for(int i=0;i<2;i++){
        if(GetCaptionMenuRect(i).PtInRect(point)){
            OnCaptionMenu(i);
            return;
        }
    }
    CDialogEx::OnLButtonDown(nFlags, point);
}

void CQuickToolDlg::OnMouseMove(UINT nFlags, CPoint point)
{
    BOOL bHot=GetCaptionCloseRect().PtInRect(point);
    int iMenuHot=-1;
    int iMenuCount=2;                       // 功能/帮助（退出已由 × 承担）
    for(int i=0;i<iMenuCount;i++){
        if(GetCaptionMenuRect(i).PtInRect(point)) iMenuHot=i;
    }
    if(bHot!=m_bCapCloseHot || iMenuHot!=m_iCapMenuHot){
        m_bCapCloseHot=bHot;
        m_iCapMenuHot=iMenuHot;
        InvalidateRect(GetCaptionRect(),FALSE);
    }
    // 标题栏 tooltip：仅在标题栏空白区（非按钮）时转发鼠标消息 2026.10.21
    if(GetCaptionRect().PtInRect(point) && !bHot && iMenuHot<0){
        MSG msg={m_hWnd,WM_MOUSEMOVE,nFlags,(point.y<<16)|point.x,0,0};
        m_tipCaption.RelayEvent(&msg);
    }
    CDialogEx::OnMouseMove(nFlags, point);
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
        // 不调基类（其内部 BeginPaint 一次后再次构造 CPaintDC 裁剪不可靠）：
        // 对话框背景交给 WM_ERASEBKGND，这里只画自绘标题栏 2026.10.21
        CPaintDC dc(this);
        DrawCaptionBar(&dc);
    }
}

// 擦除阶段即画标题栏渐变与客户区底色（resize 时先于 OnPaint 上屏，消除残影/白闪）
BOOL CQuickToolDlg::OnEraseBkgnd(CDC* pDC)
{
    CRect rc;
    GetClientRect(&rc);
    // 客户区整体底色（标题栏以下部分，接近控件区背景；随主题取色 2026.10.22）
    pDC->FillSolidRect(rc,QtColors().crClient);
    // 标题栏渐变直接画上（DrawCaptionBar 后续在 OnPaint 叠加按钮与文字）
    DrawCaptionBar(pDC);
    return TRUE;                            // 已自行擦除，不再走系统白底
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
    SaveIconWidth();            // 图标区宽度持久化 2026.10.20

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

    // 位置已恢复：按主窗口实际高度重建边条（高=主窗口高，垂直居中）2026.10.21
    {
        CRect rc;
        GetWindowRect(&rc);
        m_edgeBar.DestroyWindow();
        m_edgeBar.CreateEdgeBar(this,GetScreenHeight(),rc.Height());
    }

    OnRefresh();
}

void CQuickToolDlg::DealMouseMove(){
}

void CQuickToolDlg::OnMove(int x,int y){
    // 贴边收缩已改为"隐藏主窗口+边条把手"方案，主窗口不再变形移动（OnMove 仅记位置）2026.10.21
    BOOL bRight=IsStickRight()?TRUE:FALSE;
    if(bRight!=m_bIconRight){
        LayoutIconPanel();
    }
    //     CDialogEx::OnMove(x,y);
}

void CQuickToolDlg::OnMoving(UINT fwSide,LPRECT pRect)
{
    CDialogEx::OnMoving(fwSide,pRect);
    
}

// 贴边收缩状态机（新方案）：收缩=主窗口整体隐藏+边条窗口显示；
// 鼠标进入边条热区=唤出主窗口+隐藏边条；唤出后鼠标移出主窗口矩形才再次收缩。
// 支持贴左缘与贴右缘（边条把手随主窗口同侧）2026.10.21/10.22
// 贴边判定：主窗口贴近哪一缘返回哪侧（-1=左，1=右，0=未贴）2026.10.22
int CQuickToolDlg::GetStickEdge(){
    CRect rcWin;
    GetWindowRect(&rcWin);
    int iScreenWidth=GetScreenWidth();
    BOOL bNearLeft =(rcWin.left<=8);                    // 与原贴边判据同容差
    BOOL bNearRight=(rcWin.right>=iScreenWidth-8);
    if(bNearLeft) return -1;
    if(bNearRight) return 1;
    return 0;
}

// 贴边唤出展开 2026.10.23：定位+显示+整窗立即重绘（关重绘防白闪，唤出路径共用）。
// 置前策略 2026.10.24：唤出即激活。实测本机存在窗口管控，对"不激活的纯 Z 序提升"
// （SWP_NOACTIVATE 的 SetWindowPos）会间歇性静默拦截——返回成功却既不进 TOPMOST 也不动
// Z 序，主窗停在普通层深处被普通窗口遮挡（用户两次实测复现）；四组对照实验证实仅
// "显示+置顶"原子组合偶被放行、纯提升稳定被拒。激活是系统级置前保证（激活窗口必进
// 前台 Z 序顶部），且鼠标碰把手即使用意图（边条刚收到真实鼠标输入，激活自身合法），
// 故非全屏唤出一律激活置前，普通窗口不可能再遮挡。
// 首选升 TOPMOST 层最前；若 TOPMOST 被环境静默拒绝（样式位不亮），兜底 HWND_TOP 压到
// 普通层最前；前台有 DX 独占全屏程序时仅普通层显示让位（不激活不打断全屏），与边条同策略
void CQuickToolDlg::ExpandAtEdge(int a_iX,int a_iY,int a_iW,int a_iH){
    BOOL bFull=CEdgeBarWnd::IsForegroundFullScreen(m_hWnd);
    SetRedraw(FALSE);
    if(bFull){
        SetWindowPos(&wndTop,a_iX,a_iY,a_iW,a_iH,SWP_NOACTIVATE|SWP_SHOWWINDOW|SWP_NOREDRAW);
    }else{
        SetWindowPos(&wndTopMost,a_iX,a_iY,a_iW,a_iH,SWP_SHOWWINDOW|SWP_NOREDRAW);
        if(!(GetExStyle()&WS_EX_TOPMOST))
            SetWindowPos(&wndTop,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);   // TOPMOST 被拒→普通层最前兜底
        SetForegroundWindow();          // 激活置前：唤出即使用的系统级保证 2026.10.24
    }
    SetRedraw(TRUE);
    RedrawWindow(NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN|RDW_UPDATENOW);
}

// 主窗口展开期置前维护 2026.10.23：贴边展开期间窗口被系统/其它窗口压下去即顶回，
// 防止唤出后又被别的软件窗口盖住；a_bAllow=FALSE（DX 独占全屏/收缩）摘除 TOPMOST
// 让位还原常态。仅在状态与目标不符时才动窗口，供 200ms 周期调用不闪烁
void CQuickToolDlg::EnsureMainTopmost(BOOL a_bAllow){
    BOOL bTop=(GetExStyle()&WS_EX_TOPMOST)!=0;
    if(a_bAllow && !bTop){
        SetWindowPos(&wndTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
        if(!(GetExStyle()&WS_EX_TOPMOST))
            SetWindowPos(&wndTop,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE); // TOPMOST 被拒→普通层最前兜底
    }
    else if(!a_bAllow && bTop) SetWindowPos(&wndNoTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
}

void CQuickToolDlg::UpdateEdgeShrink(){
    if(!m_edgeBar.GetSafeHwnd()) return;
    CRect rcWin;
    GetWindowRect(&rcWin);
    int iScreenWidth=GetScreenWidth();
    int iEdge=GetStickEdge();                           // -1=贴左缘，1=贴右缘，0=未贴
    if(iEdge==0){
        // 未贴边：确保边条隐藏、主窗口可见，并还原非置顶（唤出期 TOPMOST 在此兜底还原）2026.10.23
        EnsureMainTopmost(FALSE);
        if(m_edgeBar.IsBarVisible()) m_edgeBar.HideBar();
        if(!IsWindowVisible()) ShowWindow(SW_SHOW);
        return;
    }
    POINT pnt;
    GetCursorPos(&pnt);
    // 唤出保持：主窗口可见时，鼠标在窗口矩形内或在边条热区（贴边侧外 6px 条带）
    // 都算"还在使用"，保持显示；鼠标真正离开贴边侧区域才收缩 2026.10.21/10.22
    if(IsWindowVisible()){
        CRect rcSelf;
        GetWindowRect(&rcSelf);
        rcSelf.InflateRect(0,8);                        // 上下各放 8px 容差
        if(iEdge<0) rcSelf.left-=12;                    // 左缘：左缘放 12px（覆盖边条热区）
        else        rcSelf.right+=12;                   // 右缘：右缘放 12px
        if(rcSelf.PtInRect(pnt)) return;                // 鼠标还在窗口/边条区域，保持
        // 鼠标已离开：收缩（还原非置顶，不把唤出期的 TOPMOST 带进常态）2026.10.23
        EnsureMainTopmost(FALSE);
        ShowWindow(SW_HIDE);
        m_edgeBar.ShowBar();
        return;
    }
    // 主窗口隐藏中：鼠标进入边条热区→唤出
    if(m_edgeBar.HitBar(pnt)){
        int iW=rcWin.Width();
        int iH=rcWin.Height();
        int iX=(iEdge<0)?0:iScreenWidth-iW;             // 展开到对应贴边位置
        int iY=rcWin.top;
        if(iY<0) iY=0;
        int iSH=GetScreenHeight();
        if(iY>iSH-iH) iY=iSH-iH;
        // 丝滑唤出（关重绘→原子定位+显示→重开重绘并整窗立即重画）+ 置顶唤出 2026.10.23
        ExpandAtEdge(iX,iY,iW,iH);
        m_edgeBar.HideBar();
    }
}

void CQuickToolDlg::OnTimer(UINT_PTR nIDEvent){
    UpdateEdgeShrink();
    // 唤出保持期主窗置顶维护 2026.10.23：贴边展开期间 TOPMOST 被丢/被其它置顶窗口
    // 压住即补回；前台出现 DX 独占全屏时摘 TOPMOST 让位（与边条同策略）
    if(IsWindowVisible() && GetStickEdge()!=0)
        EnsureMainTopmost(!CEdgeBarWnd::IsForegroundFullScreen(m_hWnd));
    // 边条永久置顶维护：TOPMOST 被丢即补回；前台出现 DX 独占全屏时让位（退出全屏自动恢复）2026.10.22
    m_edgeBar.EnsureTopmost(!CEdgeBarWnd::IsForegroundFullScreen(m_hWnd));
    // 自愈：应用自身从不使用点击穿透/分层透明/禁止激活，若窗口被外部工具注入
    // WS_EX_TRANSPARENT（看得见点不着）或 WS_EX_LAYERED(alpha≈0)/WS_EX_NOACTIVATE
    // （表现为"边条唤出后主窗口肉眼不可见"，2026.10.22 实测曾被外部自动化工具注入，
    // 且注入可发生在收缩隐藏期间），周期性摘除——隐藏态也摘，唤出必然干净 2026.10.21/10.23
    {
        LONG_PTR lEx=GetWindowLongPtr(m_hWnd,GWL_EXSTYLE);
        LONG_PTR lBad=WS_EX_TRANSPARENT|WS_EX_LAYERED|WS_EX_NOACTIVATE;
        if(lEx & lBad){
            SetWindowLongPtr(m_hWnd,GWL_EXSTYLE,lEx & ~lBad);
        }
    }
    CDialogEx::OnTimer(nIDEvent);
}

// 边条 OnMouseMove 同步通知：直接唤出主窗口（事件驱动，Timer 轮询做兜底）2026.10.21
LRESULT CQuickToolDlg::OnEdgeBarNotify(WPARAM wParam, LPARAM lParam){
    if(m_edgeBar.GetSafeHwnd() && !IsWindowVisible()){
        // 先摘外部注入的隐形样式再唤出，保证本次唤出即刻可见可交互 2026.10.23
        LONG_PTR lEx=GetWindowLongPtr(m_hWnd,GWL_EXSTYLE);
        LONG_PTR lBad=WS_EX_TRANSPARENT|WS_EX_LAYERED|WS_EX_NOACTIVATE;
        if(lEx & lBad){
            SetWindowLongPtr(m_hWnd,GWL_EXSTYLE,lEx & ~lBad);
        }
        UpdateEdgeShrink();                 // 内部按 GetCursorPos 判定并唤出
        // 双保险：判定因坐标差失败时强制唤出（边条上有真实鼠标事件，必然该唤出）
        if(!IsWindowVisible()){
            CRect rcWin;
            GetWindowRect(&rcWin);
            int iX=m_edgeBar.IsLeftSide()?0:GetScreenWidth()-rcWin.Width();   // 展开到边条同侧 2026.10.22
            int iY=rcWin.top;
            if(iY<0) iY=0;
            int iSH=GetScreenHeight();
            if(iY>iSH-rcWin.Height()) iY=iSH-rcWin.Height();
            ExpandAtEdge(iX,iY,rcWin.Width(),rcWin.Height());   // 置顶唤出（与状态机同路径，内含防闪）2026.10.23
        }
        m_edgeBar.HideBar();
        TrackMouseLeave();                  // 注册离窗检测，鼠标离开后自动收缩
    }
    return 0;
}

// 注册主窗口 WM_MOUSELEAVE（唤出后鼠标离开即收缩）2026.10.21
void CQuickToolDlg::TrackMouseLeave(){
    TRACKMOUSEEVENT tme={sizeof(tme),TME_LEAVE,m_hWnd,0};
    _TrackMouseEvent(&tme);
}

// 鼠标离开主窗口：贴边状态下收缩为边条（左/右缘均可）2026.10.21/10.22
void CQuickToolDlg::OnMouseLeave()
{
    if(IsWindowVisible()){
        if(GetStickEdge()!=0){                          // 仅贴边状态收缩（左/右缘）2026.10.22
            EnsureMainTopmost(FALSE);                   // 还原非置顶，不把唤出期的 TOPMOST 带进常态 2026.10.23
            ShowWindow(SW_HIDE);
            m_edgeBar.ShowBar();
        }
    }
}

void CQuickToolDlg::OnMenuAddPath(){
    CBrowseForFolder dlg(this->GetSafeHwnd(),QtLS(QT_TXT_SELECT_DIR),_T(""));
    if(dlg.SelectFolder()){
        m_aViewPath.Add(dlg.GetSelectedFolder());
        SaveConfig();
        OnRefresh();
    }
}

void CQuickToolDlg::OnSize(UINT nType,int cx,int cy)
{
    LayoutIconPanel();
    CDialogEx::OnSize(nType,cx,cy);
    // borderless resize：标题栏自绘渐变与子控件重绘不同步会产生旧尺寸残影，
    // 尺寸变化后立即整窗失效并同帧重画（含子控件）2026.10.21
    if(cx>0 && cy>0){
        RedrawWindow(NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN|RDW_UPDATENOW);
    }
}

// ==================== 图标区实现 2026.10.20 ====================

// 分隔条窗口过程：拖动调节图标区宽度 2026.10.20
LRESULT CALLBACK CQuickToolDlg::IconSplitterProc(HWND a_hWnd,UINT a_uMsg,WPARAM a_wParam,LPARAM a_lParam){
    CQuickToolDlg* pOwner=(CQuickToolDlg*)::GetWindowLongPtr(a_hWnd,GWLP_USERDATA);
    if(a_uMsg==WM_LBUTTONDOWN){
        if(pOwner){
            pOwner->m_bSplitDrag=TRUE;
            CPoint pnt((short)LOWORD(a_lParam),(short)HIWORD(a_lParam));
            ::ClientToScreen(a_hWnd,&pnt);      // 按下点在分隔条客户区，转屏幕坐标作基准
            pOwner->m_pntSplitLast=pnt;
            ::SetCapture(a_hWnd);
        }
        return 0;
    }
    if(a_uMsg==WM_MOUSEMOVE){
        if(pOwner && pOwner->m_bSplitDrag && ::GetCapture()==a_hWnd){
            CRect rcDlg;
            pOwner->GetClientRect(&rcDlg);
            // 捕获期间鼠标消息坐标是相对分隔条客户区（3 像素宽），
            // 转屏幕坐标后取帧间位移即可，不依赖真实光标位置
            CPoint pntCurr((short)LOWORD(a_lParam),(short)HIWORD(a_lParam));
            ::ClientToScreen(a_hWnd,&pntCurr);
            // 帧间位移量，按图标区方向换算：图标在左向右拖加宽，图标在右向左拖加宽
            int iDelta=pntCurr.x-pOwner->m_pntSplitLast.x;
            pOwner->m_pntSplitLast=pntCurr;
            if(iDelta==0) return 0;
            int iNew=pOwner->m_bIconRight ? pOwner->m_iIconWidth-iDelta : pOwner->m_iIconWidth+iDelta;
            if(iNew<ICON_PANEL_MIN_WIDTH) iNew=ICON_PANEL_MIN_WIDTH;
            if(iNew>rcDlg.Width()/2) iNew=rcDlg.Width()/2;
            if(iNew!=pOwner->m_iIconWidth){
                pOwner->m_iIconWidth=iNew;
                pOwner->LayoutIconPanel();
            }
        }
        return 0;
    }
    if(a_uMsg==WM_LBUTTONUP){
        if(pOwner && pOwner->m_bSplitDrag){
            pOwner->m_bSplitDrag=FALSE;
            if(::GetCapture()==a_hWnd) ::ReleaseCapture();
            pOwner->SaveIconWidth();
        }
        return 0;
    }
    if(a_uMsg==WM_PAINT){
        // 分隔条自绘底色随主题（窗口类无背景刷，避免残留杂物/暗主题刺眼白条）2026.10.22
        PAINTSTRUCT ps;
        HDC hDC=::BeginPaint(a_hWnd,&ps);
        RECT rc;
        ::GetClientRect(a_hWnd,&rc);
        HBRUSH hBr=::CreateSolidBrush(QtColors().crSplit);
        if(hBr){
            ::FillRect(hDC,&rc,hBr);
            ::DeleteObject(hBr);
        }
        ::EndPaint(a_hWnd,&ps);
        return 0;
    }
    if(a_uMsg==WM_SETCURSOR){
        ::SetCursor(LoadCursor(NULL,IDC_SIZEWE));
        return TRUE;
    }
    return ::DefWindowProc(a_hWnd,a_uMsg,a_wParam,a_lParam);
}

// 初始化图标区控件与图像列表 2026.10.20
void CQuickToolDlg::InitIconPanel(){
    m_lstIcons.SetOwner(this);

    // 16x16 图像列表 2026.10.20
    m_ilIconImages.DeleteImageList();
    m_ilIconImages.Create(16,16,ILC_COLOR32,5,5);
    m_ilIconImages.SetBkColor(CLR_NONE);
    m_lstIcons.SetImageList(&m_ilIconImages,LVSIL_NORMAL);
    // 1pt 极小字体：列表不显示文字，仅用于压缩项的空标签命中区，
    // 否则默认字体标签区（约 17px）超过 24px 网格，会盖住下一行图标导致点击串行 2026.10.20
    m_lstIcons.ApplyTinyFont();
    // 紧凑图标间距（16 图标 + 8 空隙），配合只显示图标可排多列 2026.10.20
    m_lstIcons.SendMessage(LVM_SETICONSPACING,0,(LPARAM)MAKELPARAM(24,24));

    // 逐项 Tooltip（LVN_GETINFOTIP）
    m_lstIcons.SetExtendedStyle(LVS_EX_INFOTIP);
    m_lstIcons.ApplyTipFont();          // Tooltip 放大到 11pt 2026.10.20

    // 图标区拖放：OLE 目标（资源管理器拖入）+ 经典 WM_DROPFILES 兜底 2026.10.20
    m_iconDropTarget.SetOwner(this);
    m_iconDropTarget.Register(&m_lstIcons);
    m_lstIcons.ModifyStyleEx(0,WS_EX_ACCEPTFILES);

    // 读取保存的宽度
    CIniFile iniFile;
    if(iniFile.Load(GetAppExePath()+_T("Config.ini"))){
        m_iIconWidth=iniFile.GetInt(ICON_INI_SECTION,_T("PanelWidth"),ICON_PANEL_DEF_WIDTH);
    }
    if(m_iIconWidth<ICON_PANEL_MIN_WIDTH) m_iIconWidth=ICON_PANEL_MIN_WIDTH;
    m_bIconRight=FALSE;

    // 创建 3 像素分隔条
    if(m_hSplitter==NULL){
        WNDCLASSEX wc={0};
        wc.cbSize=sizeof(wc);
        wc.lpfnWndProc=IconSplitterProc;
        wc.hInstance=AfxGetInstanceHandle();
        wc.lpszClassName=ICON_SPLITTER_CLASS;
        wc.hCursor=LoadCursor(NULL,IDC_SIZEWE);
        if(!::GetClassInfoEx(AfxGetInstanceHandle(),ICON_SPLITTER_CLASS,&wc)){
            ::RegisterClassEx(&wc);
        }
        m_hSplitter=::CreateWindowEx(0,ICON_SPLITTER_CLASS,_T(""),WS_CHILD|WS_VISIBLE,0,0,ICON_SPLITTER_WIDTH,10,m_hWnd,NULL,AfxGetInstanceHandle(),NULL);
        ::SetWindowLongPtr(m_hSplitter,GWLP_USERDATA,(LONG_PTR)this);
    }

    // 加载图标项
    LoadIconBar();

    // 初始布局
    LayoutIconPanel();
}

// 窗口是否吸附在屏幕右缘（决定图标区在左还是在右）2026.10.20
BOOL CQuickToolDlg::IsStickRight(){
    CRect rcWin;
    GetWindowRect(&rcWin);
    int iScreenWidth=GetScreenWidth();
    // 收缩态窗口右缘会超出屏幕，因此用贴近期望判定
    return rcWin.right>=iScreenWidth-8;
}

// 按客户区大小摆放图标区/分隔条/树控件 2026.10.20
// 图标区缺省在左；窗口吸附到屏幕右缘时镜像：图标区在右，树控件在左
void CQuickToolDlg::LayoutIconPanel(){
    CRect rc;
    GetClientRect(rc);
    int iClientWidth=rc.Width();
    int iHeight=rc.Height();
    if(iHeight<=0 || iClientWidth<=0) return;
    m_bIconRight=IsStickRight()?TRUE:FALSE;

    // 限制宽度范围
    int iWidth=m_iIconWidth;
    if(iWidth<ICON_PANEL_MIN_WIDTH) iWidth=ICON_PANEL_MIN_WIDTH;
    if(iWidth>iClientWidth/2) iWidth=iClientWidth/2;

    int iInner=iHeight-QT_CAPTION_H-5;  // 顶部留自绘标题栏，底部留 5 像素 2026.10.21
    if(iInner<0) iInner=0;
    const int iTop=QT_CAPTION_H+3;      // 控件起始 y（标题栏下 3 像素）

    CRect rcIcons,rcSplit,rcTree;
    if(!m_bIconRight){
        // 图标区在左
        rcIcons=CRect(5,iTop,5+iWidth,iTop+iInner);
        rcSplit=CRect(5+iWidth,iTop,5+iWidth+ICON_SPLITTER_WIDTH,iTop+iInner);
        rcTree =CRect(5+iWidth+ICON_SPLITTER_WIDTH,iTop,iClientWidth-5,iTop+iInner);
    } else {
        // 图标区在右
        rcTree =CRect(5,iTop,iClientWidth-5-iWidth-ICON_SPLITTER_WIDTH,iTop+iInner);
        rcSplit=CRect(iClientWidth-5-iWidth-ICON_SPLITTER_WIDTH,iTop,iClientWidth-5-iWidth,iTop+iInner);
        rcIcons=CRect(iClientWidth-5-iWidth,iTop,iClientWidth-5,iTop+iInner);
    }
    if(rcTree.Width()<10) rcTree.right=rcTree.left+10;
    if(m_hSplitter) ::MoveWindow(m_hSplitter,rcSplit.left,rcSplit.top,rcSplit.Width(),rcSplit.Height(),TRUE);
    if(m_tvwFolder.GetSafeHwnd()) m_tvwFolder.MoveWindow(rcTree);
    if(m_lstIcons.GetSafeHwnd()) m_lstIcons.MoveWindow(rcIcons);
}

// 图标快捷方式目录（exe 同目录 Icons\）2026.10.20
CString CQuickToolDlg::GetIconDir(){
    CString sDir=GetAppExePath()+ICON_DIR_NAME;
    MakeSureDirectoryFileExist(sDir);
    return sDir;
}

// 按数据数组重建图标列表项 2026.10.20
void CQuickToolDlg::RebuildIconItems(){
    if(!m_lstIcons.GetSafeHwnd()) return;
    m_lstIcons.ResetHover();
    m_lstIcons.DeleteAllItems();
    // 只显示图标不显示文字（名称与路径由 Tooltip 展示），窄面板可排多列 2026.10.20
    for(int i=0;i<(int)m_aIconLnks.GetSize();i++){
        UINT uImage=(i<(int)m_aIconImgs.GetSize())?m_aIconImgs[i]:0;
        LVITEM lv={0};
        lv.mask=LVIF_IMAGE;
        lv.iItem=i;
        lv.iImage=(int)uImage;
        m_lstIcons.InsertItem(&lv);
    }
}

// 从 Icons 目录加载全部图标项 2026.10.20
void CQuickToolDlg::LoadIconBar(){
    m_lstIcons.DeleteAllItems();
    m_ilIconImages.DeleteImageList();
    m_ilIconImages.Create(16,16,ILC_COLOR32,5,5);       // 16x16 2026.10.20
    m_ilIconImages.SetBkColor(CLR_NONE);
    m_lstIcons.SetImageList(&m_ilIconImages,LVSIL_NORMAL);
    m_lstIcons.SendMessage(LVM_SETICONSPACING,0,(LPARAM)MAKELPARAM(24,24)); // 紧凑间距 2026.10.20
    m_aIconLnks.RemoveAll();
    m_aIconImgs.RemoveAll();

    // 文件名形如 001.lnk、002.lnk...，按名称排序即持久化的顺序
    CStringArray aNames;
    CFileFind ff;
    BOOL bFound=ff.FindFile(GetIconDir()+_T("*.lnk"));
    while(bFound){
        bFound=ff.FindNextFile();
        if(ff.IsDots()) continue;
        aNames.Add(ff.GetFileName());
    }
    ff.Close();
    // 插入排序按文件名升序（CStringArray 无 Sort 成员）
    for(int i=1;i<aNames.GetSize();i++){
        CString sKey=aNames[i];
        int j=i-1;
        while(j>=0 && aNames[j].CompareNoCase(sKey)>0){
            aNames[j+1]=aNames[j];
            j--;
        }
        aNames[j+1]=sKey;
    }
    for(int i=0;i<aNames.GetSize();i++){
        AddIconItem(GetIconDir()+aNames[i]);
    }
    RebuildIconItems();
    m_lstIcons.Arrange(LVA_ALIGNTOP);
}

// 添加一个图标项（a_sLnkPath 为 .lnk 文件路径）2026.10.20
// 图标不带"快捷方式"小箭头：SHGetFileInfo 直接取 lnk 图标会被系统叠加箭头标识，
// 故改为解析 lnk 的图标位置（尊重"更改图标"自定义；未自定义则用目标文件图标），
// 再按图标索引从原图标源提取干净图标 2026.10.21
void CQuickToolDlg::AddIconItem(const CString& a_sLnkPath){
    HICON hIcon=NULL;
    CString sIconPath;
    int iIconIndex=0;
    if(GetShortcutIconLocation(a_sLnkPath,sIconPath,iIconIndex)){
        hIcon=ExtractOneIcon(sIconPath,iIconIndex);
    }
    if(!hIcon){
        // 解析失败退回系统方式（带箭头总比空图标好）2026.10.21
        SHFILEINFO sfi={0};
        if(SHGetFileInfo(a_sLnkPath,0,&sfi,sizeof(sfi),SHGFI_ICON|SHGFI_SMALLICON)){
            hIcon=sfi.hIcon;
        }
    }
    UINT uImage=0;
    if(hIcon){
        uImage=(UINT)(INT_PTR)m_ilIconImages.Add(hIcon);
        DestroyIcon(hIcon);
    }
    m_aIconLnks.Add(a_sLnkPath);
    m_aIconImgs.Add(uImage);
}

// 取快捷方式的图标位置（文件路径+索引）：优先 lnk 自定义图标，
// 未设置则回退到目标路径（系统对 lnk 默认显示目标图标）2026.10.21
BOOL CQuickToolDlg::GetShortcutIconLocation(const CString& a_sLnkPath,CString& a_sIconPath,int& a_iIconIndex){
    BOOL bGot=FALSE;
    IShellLink* psl=NULL;
    HRESULT hr=CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(LPVOID*)&psl);
    if(SUCCEEDED(hr)){
        IPersistFile* ppf=NULL;
        if(SUCCEEDED(psl->QueryInterface(IID_IPersistFile,(LPVOID*)&ppf))){
            USES_CONVERSION;
            if(SUCCEEDED(ppf->Load(T2COLE(a_sLnkPath),STGM_READ))){
                TCHAR szIcon[MAX_PATH]={0};
                int iId=0;
                if(SUCCEEDED(psl->GetIconLocation(szIcon,MAX_PATH,&iId)) && szIcon[0]!=_T('\0')){
                    a_sIconPath=szIcon;
                    a_iIconIndex=iId;
                    bGot=TRUE;
                }else{
                    // lnk 未自定义图标：默认显示目标文件图标（无箭头）
                    a_sIconPath=GetShortcutTargetPath(a_sLnkPath);
                    a_iIconIndex=0;
                    bGot=!a_sIconPath.IsEmpty();
                }
            }
            ppf->Release();
        }
        psl->Release();
    }
    return bGot;
}

// 从指定文件提取第 a_iIconIndex 个小图标（独立句柄，调用方 DestroyIcon）2026.10.21
HICON CQuickToolDlg::ExtractOneIcon(const CString& a_sFile,int a_iIconIndex){
    if(a_sFile.IsEmpty()) return NULL;
    HICON hIcon=NULL;
    UINT uCount=ExtractIconEx(a_sFile,-a_iIconIndex-1,NULL,&hIcon,1);   // 负序号=按索引取小图标
    if(uCount==0 || !hIcon){
        // 该索引没有小图标则取大图标缩用
        HICON hLarge=NULL;
        ExtractIconEx(a_sFile,a_iIconIndex,&hLarge,NULL,1);
        if(hLarge){
            hIcon=hLarge;       // 大图标直接用（显示时列表自缩放）
        }
    }
    return hIcon;
}

// 删除一个图标项（同时删除 .lnk 文件）2026.10.20
void CQuickToolDlg::RemoveIconItem(int a_iIndex){
    if(a_iIndex<0||a_iIndex>=m_aIconLnks.GetSize()) return;
    DeleteFile(m_aIconLnks[a_iIndex]);
    m_aIconLnks.RemoveAt(a_iIndex);
    m_aIconImgs.RemoveAt(a_iIndex);
    RebuildIconItems();
    m_lstIcons.Arrange(LVA_ALIGNTOP);
    SaveIconSort();
}

// 处理拖到图标区的文件（非快捷方式自动转换为快捷方式，保留原图标）2026.10.20
void CQuickToolDlg::AddIconFromDrop(CStringArray& a_aFiles){
    BOOL bAdded=FALSE;
    CString sDir=GetIconDir();
    if(sDir.Right(1)!=_T("\\")) sDir+=_T("\\");
    for(int i=0;i<a_aFiles.GetSize();i++){
        CString sFile=a_aFiles[i];

        // 忽略已存在的同目标图标项
        CString sNewTarget=GetShortcutTargetPath(sFile);
        BOOL bExist=FALSE;
        for(int k=0;k<m_aIconLnks.GetSize();k++){
            if(GetShortcutTargetPath(m_aIconLnks[k]).CompareNoCase(sNewTarget)==0){
                bExist=TRUE;
                break;
            }
        }
        if(bExist) continue;

        CString sLnkPath;
        if(sFile.Right(4).CompareNoCase(_T(".lnk"))==0){
            // 已是快捷方式：复制过来（保留原有图标）
            sLnkPath=sDir+GetFileName(sFile);
            if(!CopyFile(sFile,sLnkPath,FALSE)) continue;
        } else if(FileExists(sFile)||DirectoryExists(sFile)){
            // 非快捷方式：自动创建快捷方式（图标与原文件一致）
            CString sName=GetFileNameExtend(sFile,false);
            if(sName.IsEmpty()) sName=GetFileName(sFile);
            sLnkPath=sDir+sName+_T(".lnk");
            if(!CreateShortcut(sFile,sLnkPath)) continue;
        } else {
            continue;
        }
        AddIconItem(sLnkPath);
        bAdded=TRUE;
    }
    if(bAdded){
        RebuildIconItems();
        m_lstIcons.Arrange(LVA_ALIGNTOP);
        SaveIconSort();
    }
}

// 按当前显示顺序重命名 .lnk 持久化（001.lnk、002.lnk...）2026.10.20
void CQuickToolDlg::SaveIconSort(){
    CString sDir=GetIconDir();
    // 先全部改成临时名，避免互相重名冲突
    for(int i=0;i<m_aIconLnks.GetSize();i++){
        MoveFile(m_aIconLnks[i],sDir+Format(_T("__tmp_%03d.lnk"),i));
    }
    for(int i=0;i<m_aIconLnks.GetSize();i++){
        CString sNew=sDir+Format(_T("%03d.lnk"),i+1);
        MoveFile(sDir+Format(_T("__tmp_%03d.lnk"),i),sNew);
        m_aIconLnks[i]=sNew;
    }
}

// 图标项位置调整（数据换位后重建列表并持久化）2026.10.20
void CQuickToolDlg::MoveIconItem(int a_iFrom,int a_iTo){
    int iCount=(int)m_aIconLnks.GetSize();
    if(a_iFrom<0||a_iFrom>=iCount) return;
    if(a_iTo<0||a_iTo>=iCount) return;
    if(a_iFrom==a_iTo) return;

    CString sLnk=m_aIconLnks[a_iFrom];
    UINT uImg=m_aIconImgs[a_iFrom];
    m_aIconLnks.RemoveAt(a_iFrom);
    m_aIconImgs.RemoveAt(a_iFrom);
    m_aIconLnks.InsertAt(a_iTo,sLnk);
    m_aIconImgs.InsertAt(a_iTo,uImg);

    RebuildIconItems();
    m_lstIcons.Arrange(LVA_ALIGNTOP);
    SaveIconSort();
}

// 运行图标项指向的程序（由 WM_ICONBAR_LAUNCH 延迟触发）2026.10.20
LRESULT CQuickToolDlg::OnIconBarLaunch(WPARAM wParam, LPARAM lParam) {
    LaunchIcon((int)wParam);
    return 0;
}

// 运行图标项指向的程序 2026.10.20
// 目标程序已在运行时，ShellExecute 建的新进程会把已有实例置前后退出，但该切换
// 常被系统前台锁拦截（表现为"进程在、窗口不弹"）；启动后由后台线程轮询目标
// 主窗口并强制提到前台（线程化避免主线程轮询冻结界面）2026.10.21
struct tagLaunchWatch{
    CString sTargetExe;                         // 目标 exe 文件名（小写）
    CMap<DWORD,DWORD,DWORD,DWORD> mapOld;       // 启动前已存在的同名进程 ID
};

void CQuickToolDlg::LaunchIcon(int a_iIndex){
    if(a_iIndex<0||a_iIndex>=m_aIconLnks.GetSize()) return;
    CString sLnk=m_aIconLnks[a_iIndex];
    CString sTarget=GetShortcutTargetPath(sLnk);
    CString sWorkDir=GetFilePath(sTarget);

    // 记录启动前已存在的进程 ID 集合（按目标 exe 名匹配），用于识别新进程
    CString sTargetExe=sTarget;
    sTargetExe.MakeLower();
    int iExePos=sTargetExe.ReverseFind(_T('\\'));
    if(iExePos>=0) sTargetExe=sTargetExe.Mid(iExePos+1);
    tagLaunchWatch* pWatch=new tagLaunchWatch;
    if(!sTargetExe.IsEmpty()){
        HANDLE hSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
        if(hSnap!=INVALID_HANDLE_VALUE){
            PROCESSENTRY32 pe={0};
            pe.dwSize=sizeof(pe);
            if(Process32First(hSnap,&pe)){
                do{
                    CString sName=pe.szExeFile;
                    sName.MakeLower();
                    if(sName==sTargetExe) pWatch->mapOld[pe.th32ProcessID]=pe.th32ProcessID;
                }while(Process32Next(hSnap,&pe));
            }
            CloseHandle(hSnap);
        }
    }

    HINSTANCE hInst=ShellExecute(m_hWnd,_T("open"),sLnk,_T(""),sWorkDir,SW_SHOWNORMAL);
    if((INT_PTR)hInst<=32){ delete pWatch; return; }    // 启动失败

    // 首次启动（此前无同名进程）：系统自己会把新窗口置前，无需接管
    if(pWatch->mapOld.GetCount()==0){ delete pWatch; return; }

    pWatch->sTargetExe=sTargetExe;

    // 轮询提升放到工作线程：主线程同步 Sleep 轮询 3 秒会冻结界面 2026.10.21
    UINT uTid=0;
    HANDLE hThread=(HANDLE)_beginthreadex(NULL,0,LaunchWatchThread,pWatch,0,&uTid);
    if(hThread) CloseHandle(hThread);           // 线程句柄即交，自我管理生命周期
    else delete pWatch;
}

// 后台轮询：新进程主窗口出现即强制置前；3 秒未见新进程则把旧实例主窗口置前 2026.10.21
// 脚本/文档类目标（.bat/.cmd 等）由 shell 代理打开，不存在"单实例旧窗口"，
// 乱提升会把无关同名进程窗口拽到前台、挤掉新开的脚本窗口 2026.10.21
UINT CALLBACK CQuickToolDlg::LaunchWatchThread(LPVOID a_pParam){
    tagLaunchWatch* pWatch=(tagLaunchWatch*)a_pParam;
    CString sTargetExe=pWatch->sTargetExe;
    // 非 exe 目标（脚本/文档）：只做"新进程置前"，不做旧实例提升
    BOOL bIsExe=(sTargetExe.Right(4)==_T(".exe"));

    for(int i=0;i<30;i++){
        Sleep(100);
        HANDLE hSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
        if(hSnap==INVALID_HANDLE_VALUE){ delete pWatch; return 0; }
        DWORD dwNewPid=0;
        PROCESSENTRY32 pe={0};
        pe.dwSize=sizeof(pe);
        if(Process32First(hSnap,&pe)){
            do{
                CString sName=pe.szExeFile;
                sName.MakeLower();
                if(sName==sTargetExe){
                    BOOL bOld=pWatch->mapOld.Lookup(pe.th32ProcessID,dwNewPid);
                    if(!bOld){
                        dwNewPid=pe.th32ProcessID;
                        break;
                    }
                }
            }while(Process32Next(hSnap,&pe));
        }
        CloseHandle(hSnap);
        if(dwNewPid==0) continue;       // 暂未见新进程
        HWND hNew=FindMainWindowOfPid(dwNewPid);
        if(hNew){
            ForceWindowForeground(hNew);
            delete pWatch;
            return 0;
        }
        // 新进程在但还没建主窗口，继续等
    }

    // 未见新进程：仅当目标是 exe（单实例 GUI 程序场景）才提升旧实例主窗口
    if(bIsExe){
        HANDLE hSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
        if(hSnap!=INVALID_HANDLE_VALUE){
            PROCESSENTRY32 pe={0};
            pe.dwSize=sizeof(pe);
            if(Process32First(hSnap,&pe)){
                do{
                    CString sName=pe.szExeFile;
                    sName.MakeLower();
                    if(sName==sTargetExe){
                        HWND hOld=FindMainWindowOfPid(pe.th32ProcessID);
                        if(hOld){
                            ForceWindowForeground(hOld);
                            break;
                        }
                    }
                }while(Process32Next(hSnap,&pe));
            }
            CloseHandle(hSnap);
        }
    }
    delete pWatch;
    return 0;
}

// 取进程的主可见顶层窗口（无属主、非工具窗）2026.10.21
HWND CQuickToolDlg::FindMainWindowOfPid(DWORD a_dwPid){
    HWND hFound=NULL;
    EnumWindows([](HWND hWnd,LPARAM lParam)->BOOL{
        DWORD dwPid=0;
        ::GetWindowThreadProcessId(hWnd,&dwPid);
        if(dwPid!=(DWORD)lParam) return TRUE;
        if(!::IsWindowVisible(hWnd)) return TRUE;
        if(::GetWindow(hWnd,GW_OWNER)) return TRUE;
        LONG_PTR lEx=::GetWindowLongPtr(hWnd,GWL_EXSTYLE);
        if(lEx & WS_EX_TOOLWINDOW) return TRUE;
        LONG_PTR lStyle=::GetWindowLongPtr(hWnd,GWL_STYLE);
        if(!(lStyle & WS_CAPTION)) return TRUE;     // 无标题栏的多为隐藏辅助窗
        *(HWND*)lParam=hWnd;                        // 复用 lParam 传出
        return FALSE;
    },(LPARAM)&hFound);
    return hFound;
}

// 强制把目标窗口提到前台（后台进程调用受前台锁限制，组合拳：
// ①AttachThreadInput 共享输入队列 ②SwitchToThisWindow（系统切窗 API，Alt+Tab 同款，对
// 后台调用方放行）③TOPMOST→NOTOPMOST 翻转兜底 z 序）2026.10.21
void CQuickToolDlg::ForceWindowForeground(HWND a_hWnd){
    if(!a_hWnd || !::IsWindow(a_hWnd)) return;
    HWND hFore=::GetForegroundWindow();
    if(hFore==a_hWnd) return;
    DWORD dwForeTid=::GetWindowThreadProcessId(hFore,NULL);
    DWORD dwCurTid=::GetCurrentThreadId();
    DWORD dwTgtTid=::GetWindowThreadProcessId(a_hWnd,NULL);
    ::AttachThreadInput(dwCurTid,dwForeTid,TRUE);
    ::AttachThreadInput(dwCurTid,dwTgtTid,TRUE);
    ::ShowWindow(a_hWnd,SW_RESTORE);
    ::BringWindowToTop(a_hWnd);
    ::SetForegroundWindow(a_hWnd);
    // SwitchToThisWindow：系统内部切窗路径，绕过后台进程前台锁
    HMODULE hUser32=::GetModuleHandle(_T("user32.dll"));
    if(hUser32){
        typedef void (WINAPI *PFNSwitchToThisWindow)(HWND,BOOL);
        PFNSwitchToThisWindow pSwitch=(PFNSwitchToThisWindow)::GetProcAddress(hUser32,"SwitchToThisWindow");
        if(pSwitch) pSwitch(a_hWnd,TRUE);
    }
    // SetForegroundWindow 被拒时 z 序翻转兜底（瞬时 TOPMOST 再还原）
    ::SetWindowPos(a_hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
    ::SetWindowPos(a_hWnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
    ::SetForegroundWindow(a_hWnd);
    ::AttachThreadInput(dwCurTid,dwTgtTid,FALSE);
    ::AttachThreadInput(dwCurTid,dwForeTid,FALSE);
}

// 图标项 Tooltip 文本（原目标信息：文件名+路径）2026.10.20
CString CQuickToolDlg::GetIconTip(int a_iIndex){
    if(a_iIndex<0||a_iIndex>=m_aIconLnks.GetSize()) return _T("");
    CString sTarget=GetShortcutTargetPath(m_aIconLnks[a_iIndex]);
    CString sTip=GetFileName(sTarget);
    if(sTip.IsEmpty()) sTip=GetFileName(m_aIconLnks[a_iIndex]);
    CString sPath=GetFilePath(sTarget);
    if(!sPath.IsEmpty()) sTip+=_T("\r\n")+sPath;
    return sTip;
}

// 图标区右键菜单 2026.10.20
void CQuickToolDlg::ShowIconMenu(int a_iIndex,CPoint a_pntClient){
    m_iIconMouseItem=a_iIndex;
    CMenu menu;
    menu.CreatePopupMenu();
    menu.AppendMenu(MF_STRING,ID_POPMENU_ICON_OPENFOLDER,QtLS(QT_TXT_ICON_OPEN));
    menu.AppendMenu(MF_STRING,ID_POPMENU_ICON_PROPERTY,QtLS(QT_TXT_ICON_PROPERTY));     // 修改目标/路径/图标 2026.10.21
    menu.AppendMenu(MF_STRING,ID_POPMENU_ICON_DELETE,QtLS(QT_TXT_ICON_DELETE));
    ClientToScreen(&a_pntClient);
    menu.TrackPopupMenu(TPM_LEFTALIGN,a_pntClient.x,a_pntClient.y,this);
}

// 保存图标区宽度 2026.10.20
void CQuickToolDlg::SaveIconWidth(){
    CIniFile iniFile;
    iniFile.Load(GetAppExePath()+_T("Config.ini"));
    iniFile.SetVarInt(ICON_INI_SECTION,_T("PanelWidth"),m_iIconWidth);
    iniFile.Save();
}

// 从图标区删除（右键菜单）2026.10.20
void CQuickToolDlg::OnPopmenuIconDelete(){
    if(m_iIconMouseItem<0) return;
    if(AfxMessageBox(Format(QtLS(QT_TXT_MSG_DEL_ICON),GetIconTip(m_iIconMouseItem)),MB_YESNO)==IDYES){
        RemoveIconItem(m_iIconMouseItem);
    }
    m_iIconMouseItem=-1;
}

// 打开图标目标所在位置（右键菜单）2026.10.20
void CQuickToolDlg::OnPopmenuIconOpenFolder(){
    if(m_iIconMouseItem<0) return;
    CString sTarget=GetShortcutTargetPath(m_aIconLnks[m_iIconMouseItem]);
    if(FileExists(sTarget)){
        ShellExecute(NULL,_T("open"),_T("explorer.exe"),_T("/select,")+sTarget,_T(""),SW_SHOWNORMAL);
    } else if(DirectoryExists(sTarget)){
        ShellExecute(NULL,_T("open"),sTarget,NULL,NULL,SW_SHOWNORMAL);
    }
    m_iIconMouseItem=-1;
}

// 修改图标项属性（右键菜单）：系统快捷方式属性页可改目标/起始位置/图标 2026.10.21
// 取消（.lnk 未变）则不动列表；真修改则整体重建——不能用 CImageList::Remove 删旧图，
// Remove 会把后续图标整体前移而 m_aIconImgs 索引不同步，导致全部图标错位/清空 2026.10.21
void CQuickToolDlg::OnPopmenuIconProperty(){
    if(m_iIconMouseItem<0) return;
    int iItem=m_iIconMouseItem;
    m_iIconMouseItem=-1;
    if(iItem<0||iItem>=m_aIconLnks.GetSize()) return;
    CString sLnk=m_aIconLnks[iItem];

    // 记录属性页打开前的 .lnk 修改时间，用于识别"取消/未改动" 2026.10.21
    FILETIME ftOld={0};
    WIN32_FIND_DATA fd={0};
    HANDLE hFind=FindFirstFile(sLnk,&fd);
    if(hFind!=INVALID_HANDLE_VALUE){
        ftOld=fd.ftLastWriteTime;
        FindClose(hFind);
    }

    // SHObjectProperties 以模态方式打开 .lnk 的属性页（目标/起始位置/更改图标），
    // 返回即表示页已关闭（不管是否修改）2026.10.21
    BOOL bOk=SHObjectProperties(m_hWnd,SHOP_FILEPATH,sLnk,NULL);
    if(!bOk){
        AfxMessageBox(QtLS(QT_TXT_MSG_PROP_FAIL));
        return;
    }

    // 修改时间未变说明点了取消（或未实际保存），保持原样；
    // 系统落盘可能有短暂延迟，最多等 1 秒 2026.10.21
    BOOL bChanged=FALSE;
    for(int i=0;i<10;i++){
        hFind=FindFirstFile(sLnk,&fd);
        if(hFind!=INVALID_HANDLE_VALUE){
            bChanged=(CompareFileTime(&fd.ftLastWriteTime,&ftOld)!=0);
            FindClose(hFind);
        }
        if(bChanged) break;
        Sleep(100);
    }
    if(!bChanged) return;

    // 属性确实被修改：整体重建图标区（LoadIconBar 按文件名序号重载，
    // 图像列表与数据数组索引天然一致，Tooltip 随 m_aIconLnks 同步）2026.10.21
    LoadIconBar();
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
    m_ilTreeImages.SetBkColor(QtColors().crCtrlBk);     // 图标底色随主题（暗主题保持白底会露白块）2026.10.22

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
    QtLocalizeMenu(pPopup);                                 // 弹出前按当前语言取词 2026.10.22
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

    if (AfxMessageBox(Format(QtLS(QT_TXT_MSG_DEL_FILE),sFilePath), MB_YESNO) == IDYES) {
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
    LPCTSTR lpszTitle = bIsFile ? QtLS(QT_TXT_INP_RENAME_FILE) : QtLS(QT_TXT_INP_RENAME_DIR);
    CInputBox dlg(lpszTitle, QtLS(QT_TXT_INP_MODIFY), &sInput);
    if (dlg.DoModal() != IDOK) return;

    CString sNewName = sInput;
    if (sNewName == sOldName) return;

    if (bIsFile) sNewName += _T(".") + sExt;
    CString sNewPath = sParentDir + sNewName;

    bool bExists = bIsFile ? FileExists(sNewPath) : DirectoryExists(sNewPath);
    if (bExists) {
        AfxMessageBox(bIsFile ? QtLS(QT_TXT_MSG_FILE_EXISTS) : QtLS(QT_TXT_MSG_DIR_EXISTS));
    } else {
        MoveFile(sFilePath, sNewPath);
        HTREEITEM hParent = m_tvwFolder.GetParentItem(m_hRightMouseItem);
        RefreshTree(hParent, sNewPath);
    }
}

// 复制文本到剪贴板
void CQuickToolDlg::CopyTextToClipboard(const CString& a_sText) {
    if (a_sText.IsEmpty()) return;
    if (!::OpenClipboard(GetSafeHwnd())) return;
    ::EmptyClipboard();
    int iCount = a_sText.GetLength() + 1;
    HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, iCount * sizeof(TCHAR));
    if (hGlobal != NULL) {
        LPTSTR lpszData = (LPTSTR)::GlobalLock(hGlobal);
        if (lpszData != NULL) {
            _tcscpy_s(lpszData, iCount, (LPCTSTR)a_sText);
            ::GlobalUnlock(hGlobal);
            UINT uFormat = (sizeof(TCHAR) == sizeof(WCHAR)) ? CF_UNICODETEXT : CF_TEXT;
            if (::SetClipboardData(uFormat, hGlobal) == NULL) {
                ::GlobalFree(hGlobal);    // 系统接管失败时释放
            }
        } else {
            ::GlobalFree(hGlobal);
        }
    }
    ::CloseClipboard();
}

// 获得快捷方式所指向的路径，非快捷方式则返回原路径
CString CQuickToolDlg::GetShortcutTargetPath(CString a_sFilePath) {
    CString sLower = a_sFilePath;
    sLower.MakeLower();
    if (sLower.Right(4) == _T(".lnk")) {
        tagShortcutInfo siInfo;
        if (GetShortcutFileInfo((LPCTSTR)a_sFilePath, &siInfo) && !siInfo.sPath.IsEmpty()) {
            a_sFilePath = siInfo.sPath;
        }
    }
    return a_sFilePath;
}

// 复制完整路径到剪贴板（快捷方式则复制其指向的目标路径）
void CQuickToolDlg::OnPopmenuCopyPath() {
    CString sFilePath = GetCurrFilePath();
    if (sFilePath.IsEmpty()) return;
    CopyTextToClipboard(GetShortcutTargetPath(sFilePath));
}

// 复制文件名到剪贴板（快捷方式则复制其指向的目标文件名）
void CQuickToolDlg::OnPopmenuCopyName() {
    CString sFilePath = GetCurrFilePath();
    if (sFilePath.IsEmpty()) return;
    CopyTextToClipboard(GetFileName(GetShortcutTargetPath(sFilePath)));
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
    CInputBox dlg(QtLS(QT_TXT_INP_NEW_DIR), QtLS(QT_TXT_INP_DIR_NAME), &sFolderName);
    if (dlg.DoModal() == IDOK && !sFolderName.IsEmpty()) {
        CString sNewDir = sDir + sFolderName;
        if(DirectoryExists(sNewDir)){
            AfxMessageBox(QtLS(QT_TXT_MSG_DIR_EXISTS));
            return;
        }
        if(CreateDirectory(sNewDir, NULL)){
            HTREEITEM hParent = FindTreeItemByPath(sDir);
            RefreshTree(hParent, sNewDir);
        } else {
            AfxMessageBox(QtLS(QT_TXT_MSG_MKDIR_FAIL));
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
            MessageBox(QtLS(QT_TXT_MSG_ALLOC_FAIL), QtLS(QT_TXT_MSG_ERR_TITLE), MB_ICONERROR);
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
        AfxMessageBox(QtLS(QT_TXT_MSG_SAME_NAME));
        return;
    }

    if (MoveFile(sSourceFile, sDestFile)) {
        SaveConfig();
        HTREEITEM hTarget = FindTreeItemByPath(sTargetDir);
        RefreshTree(hTarget, sDestFile);
    } else {
        AfxMessageBox(QtLS(QT_TXT_MSG_MOVE_FAIL));
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
