#include "stdafx.h"
#include "EdgeBarWnd.h"

// ==================== 贴边收缩边条窗口 2026.10.21 ====================

#define QT_EDGEBAR_W    6       // 边条宽（更纤细 2026.10.21）
#define QT_EDGE_HOT     6       // 唤出热区外扩像素

BEGIN_MESSAGE_MAP(CEdgeBarWnd, CWnd)
    ON_WM_PAINT()
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

CEdgeBarWnd::CEdgeBarWnd(){
    m_iScreenHeight=0;
    m_bLeft=FALSE;
}

CEdgeBarWnd::~CEdgeBarWnd(){
}

// 创建边条窗口（置顶、工具窗、无任务栏；默认贴右缘，具体位置由 AlignToWindow 按主窗口对齐）
BOOL CEdgeBarWnd::CreateEdgeBar(CWnd* a_pOwner, int a_iScreenHeight, int a_iBarHeight){
    m_iScreenHeight=a_iScreenHeight;
    int iBarH=a_iBarHeight;
    if(iBarH<60) iBarH=120;                     // 异常兜底
    int iTop=(m_iScreenHeight-iBarH)/2;
    if(iTop<0) iTop=0;
    BOOL bOk=CreateEx(WS_EX_TOPMOST|WS_EX_TOOLWINDOW,_T("STATIC"),_T("QTEdgeBar"),
        WS_POPUP,GetScreenWidth()-QT_EDGEBAR_W,iTop,QT_EDGEBAR_W,iBarH,
        a_pOwner?a_pOwner->GetSafeHwnd():NULL,NULL);
    if(bOk) ShowWindow(SW_SHOWNOACTIVATE);
    return bOk;
}

// 收缩时把边条与主窗口对齐 2026.10.22：
// 贴哪侧随主窗口（贴左缘→边条贴左缘，贴右缘→贴右缘），
// 高度=主窗口高、top=主窗口 top，保证把手与主窗口位置一一对应
void CEdgeBarWnd::AlignToWindow(CWnd* a_pOwner){
    if(!GetSafeHwnd() || !a_pOwner || !a_pOwner->GetSafeHwnd()) return;
    CRect rcWin;
    a_pOwner->GetWindowRect(&rcWin);
    int iScreenWidth=GetScreenWidth();
    int iScreenHeight=GetScreenHeight();
    // 主窗口左右缘哪个更贴近屏幕边缘就贴哪侧（容差 8 像素，与贴边判定一致）
    BOOL bNearLeft =(rcWin.left<=8);
    BOOL bNearRight=(rcWin.right>=iScreenWidth-8);
    m_bLeft=bNearLeft;                          // 两缘都不贴时保持原侧（此时主窗口不会收缩）
    int iX;
    if(bNearLeft){
        iX=0;                                   // 左缘：边条紧贴左缘，热区向屏幕内
    }else{
        iX=iScreenWidth-QT_EDGEBAR_W;           // 右缘：紧贴右缘
    }
    int iY=rcWin.top;
    if(iY<0) iY=0;
    if(iY>iScreenHeight-QT_EDGEBAR_W) iY=iScreenHeight-QT_EDGEBAR_W;
    int iH=rcWin.Height();
    if(iH<10) iH=10;
    if(iY>iScreenHeight-iH) iY=iScreenHeight-iH;
    SetWindowPos(NULL,iX,iY,QT_EDGEBAR_W,iH,SWP_NOZORDER|SWP_NOACTIVATE);
}

// 前台是否有独占全屏程序 2026.10.22：前台窗口矩形完全覆盖主屏且不是桌面/排除窗口
// （DX 独占全屏时系统不合成普通窗口，边条需让出 TOPMOST，退出全屏后由周期维护恢复）
BOOL CEdgeBarWnd::IsForegroundFullScreen(HWND a_hExclude){
    HWND hFore=::GetForegroundWindow();
    if(!hFore || hFore==a_hExclude) return FALSE;
    if(hFore==GetShellWindow()) return FALSE;                   // 桌面/Shell 不算全屏程序
    CRect rcFore;
    if(!::GetWindowRect(hFore,rcFore)) return FALSE;            // 高权限窗口取不到矩形：保守视为非全屏
    return rcFore.left<=0 && rcFore.top<=0
        && rcFore.right>=GetSystemMetrics(SM_CXSCREEN)
        && rcFore.bottom>=GetSystemMetrics(SM_CYSCREEN);
}

// 永久置顶维护 2026.10.22：TOPMOST 被系统/其它窗口操作丢掉时立即补回；
// a_bAllow=FALSE（DX 独占全屏）摘除 TOPMOST 让位全屏，全屏结束后自动恢复。
// 仅在状态与目标不符时才动窗口，供 200ms 周期调用不闪烁
void CEdgeBarWnd::EnsureTopmost(BOOL a_bAllow){
    if(!GetSafeHwnd()) return;
    BOOL bTop=(GetExStyle()&WS_EX_TOPMOST)!=0;
    if(a_bAllow && !bTop) SetWindowPos(&wndTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
    else if(!a_bAllow && bTop) SetWindowPos(&wndNoTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
}

void CEdgeBarWnd::ShowBar(){
    if(GetSafeHwnd()){
        AlignToWindow(GetOwner());              // 每次显示前按主窗口当前位置对齐（top 一致+自动选边）2026.10.22
        EnsureTopmost(TRUE);                    // 显示时重申置顶，防 TOPMOST 被外部丢掉 2026.10.22
        ShowWindow(SW_SHOWNOACTIVATE);
    }
}

void CEdgeBarWnd::HideBar(){
    if(GetSafeHwnd()) ShowWindow(SW_HIDE);
}

// 鼠标是否进入边条唤出热区（边条矩形外扩 QT_EDGE_HOT）
BOOL CEdgeBarWnd::HitBar(CPoint a_pntScreen) const{
    if(!GetSafeHwnd()) return FALSE;
    CRect rc;
    GetWindowRect(&rc);
    rc.InflateRect(QT_EDGE_HOT,QT_EDGE_HOT);
    return rc.PtInRect(a_pntScreen);
}

// 边条外观：渐变小把手 + 内缘高亮线 + 三个小点（配色随界面主题 g_eQtTheme）2026.10.21/10.22
void CEdgeBarWnd::OnPaint()
{
    CPaintDC dc(this);
    CRect rc;
    GetClientRect(&rc);
    BOOL bDark=(g_eQtTheme==QT_THEME_DARK);
    // 把手渐变（与主窗标题栏同色系，随主题取色）2026.10.22
    QtGradientH(&dc,rc,
        bDark?RGB(0x1f,0x22,0x2e):RGB(0x23,0x2e,0x4a),
        bDark?RGB(0x36,0x3c,0x52):RGB(0x2e,0x51,0x8e));
    // 高亮线画在朝屏幕内侧的一缘（贴右缘时在左边，贴左缘时在右边）2026.10.22
    if(m_bLeft) dc.FillSolidRect(rc.right-1,0,1,rc.Height(),bDark?RGB(120,140,190):RGB(90,140,220));
    else        dc.FillSolidRect(0,0,1,rc.Height(),bDark?RGB(120,140,190):RGB(90,140,220));
    // 三个小点提示可唤出（宽 6px 时点为 2x2，间距 10）
    int iCx=rc.Width()/2;
    int iCy=rc.Height()/2;
    for(int i=-1;i<=1;i++){
        CRect rcDot(iCx-1,iCy+i*10-1,iCx+1,iCy+i*10+1);
        dc.FillSolidRect(rcDot,bDark?RGB(190,200,220):RGB(220,232,250));
    }
}

void CEdgeBarWnd::OnMouseMove(UINT nFlags, CPoint point)
{
    // 鼠标进入边条：立即直接唤出主窗口（事件驱动，不依赖属主 Timer 轮询）2026.10.21
    CWnd* pOwner=GetOwner();
    if(pOwner) pOwner->SendMessage(WM_APP+301,0,0);   // 同步调用主窗口唤出处理
    CWnd::OnMouseMove(nFlags, point);
}
