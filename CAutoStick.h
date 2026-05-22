#pragma once

//////////////////////////////////////////////////////////////////////////
// 功能：贴边窗口类
// 作者：梅文海
// 日期：2024.7.10
// 使用说明：
// 先在窗口初始化函数创建一个 CStickWindow 对象，然后调用 SetWindow 函数设置窗口句柄，一定要确保窗口已经创建
// 然后调用 AddWindow 函数添加到 CAutoStick 对象中
// 如下：
// CStickWindow* pWindows=new CStickWindow();
// pWindows->SetWindow(m_hWnd);
// m_asStick.AddWindow(pWindows);
// SetTimer(1,200,NULL); 
// 然后窗口显示函数中调用 LoadConfig 函数。注意：最好只调用一次不要多次调用
// 在窗口关闭函数中调用 SaveConfig 函数
// 在窗口的 OnTimer 函数中调用 CAutoStick 对象的 OnTimer 函数
// 在窗口的 OnMove 函数中调用 CAutoStick 对象的 OnMove 函数
// 在窗口的 OnSize 函数中调用 CAutoStick 对象的 OnSize 函数
//////////////////////////////////////////////////////////////////////////


// 贴边窗口类 2024.7.9
class CStickWindow:public CIDItem{
public:
    CStickWindow();
    virtual ~CStickWindow(){};
    virtual bool Assign(const CIDItemBase& AOther){     // 拷贝函数(不能设为纯虚方法，否则 ID 不能被设值)
        CStickWindow* pOther=(CStickWindow*)&AOther;
        m_rOrig=pOther->m_rOrig;
        m_rCurr=pOther->m_rCurr;
        m_bShink=pOther->m_bShink;
        m_bShinked=pOther->m_bShinked;
        m_bMoveing=pOther->m_bMoveing;
        m_bFirst=pOther->m_bFirst;
        m_szOrig=pOther->m_szOrig;
        m_hWnd=pOther->m_hWnd;
        return true;
    }
    virtual CIDItemBase* CreateInstance(){ return new CStickWindow; };// 创建该类的实例
    virtual LPCTSTR GetClassName(){ GET_ITEM_CLASS_NAME(CStickWindow); };   // 获得类名

    virtual void OnTimer();
    virtual void OnMove(int a_iX,int a_iY);
    virtual void OnSize(UINT nType,int cx,int cy);
    virtual void LoadConfig(CIniFile* a_pConfig);
    virtual void SaveConfig(CIniFile* a_pConfig);
    virtual void SetWindow(HWND a_hWnd);
protected:
    CRect m_rOrig;      // 原始的大小和位置(只能是正常状态) 2022.3.10
    CRect m_rCurr;      // 当前的大小和位置(有可能是缩小后的状态，也有可能是正常状态) 2022.3.10
    bool  m_bShink;     // 是否处于缩小状态 2022.3.10
    bool  m_bShinked;   // 是否已经缩小 2022.3.10
    bool  m_bMoveing;   // 正在移动窗口 2022.3.10
    bool  m_bFirst;     // 第一次调整窗口 2022.3.12
    CSize m_szOrig;     // 原始窗口大小  2022.3.12
    HWND  m_hWnd;       // 窗口句柄
};

// 自动贴向屏幕边缘类 2024.7.9
// 使用方法
class CAutoStick{
public:
    CAutoStick();
    virtual ~CAutoStick();

    virtual void AddWindow(CStickWindow* a_pWindow);
    virtual void OnTimer();
    virtual void OnMove(int a_iX,int a_iY);
    virtual void OnSize(UINT nType,int cx,int cy);
    virtual void LoadConfig(CString a_sConfigFile);
    virtual void SaveConfig(CString a_sConfigFile);
protected:
    CItemList m_aWindows;       // StickWindow 对象列表
};

