#pragma once

// ==================== 贴边收缩边条窗口 2026.10.21 ====================
// 贴边收缩时主窗口整体隐藏，另建一个独立小窗口贴在屏幕边缘做唤出把手，
// 主窗口不再变形/半透明残留，零干扰；鼠标进入边条热区唤出主窗口。
class CEdgeBarWnd : public CWnd
{
public:
    CEdgeBarWnd();
    virtual ~CEdgeBarWnd();

    BOOL CreateEdgeBar(CWnd* a_pOwner, int a_iScreenHeight, int a_iBarHeight); // 创建边条（位置与主窗口对齐，默认贴右缘，主窗口贴左缘时贴左）2026.10.22
    void ShowBar();                                             // 显示边条（主窗口将隐藏），并按主窗口当前位置重新对齐/选边
    void HideBar();                                             // 隐藏边条
    BOOL IsBarVisible() const { return GetSafeHwnd() && IsWindowVisible(); }
    BOOL IsLeftSide() const { return m_bLeft; }                 // 边条当前贴在哪一侧（TRUE=左缘）2026.10.22
    static BOOL IsForegroundFullScreen(HWND a_hExclude);        // 前台是否有独占全屏程序（窗口盖满主屏）2026.10.22
    void EnsureTopmost(BOOL a_bAllow);                          // 永久置顶维护：TOPMOST 丢失即补回；DX 独占全屏时让位 2026.10.22
    // 鼠标是否落在边条唤出热区（边条向外扩 4 像素）
    BOOL HitBar(CPoint a_pntScreen) const;

protected:
    afx_msg void OnPaint();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg LRESULT OnNcHitTest(CPoint point);
    DECLARE_MESSAGE_MAP()

protected:
    int m_iScreenHeight;    // 屏幕高度（边条垂直居中）
    BOOL m_bLeft;           // 边条贴左缘还是右缘（随主窗口贴边侧）2026.10.22

public:
    // 收缩时把边条与主窗口对齐：贴主窗口同侧边缘、高度=主窗口高、top=主窗口 top
    void AlignToWindow(CWnd* a_pOwner);
};
